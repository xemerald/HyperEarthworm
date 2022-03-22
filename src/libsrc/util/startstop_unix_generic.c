#if !(defined(_SOLARIS) || defined(_MACOSX))
#define _XOPEN_SOURCE 500
#endif

 /********************************************************************
  *                     Startstop_Unix_Generic                       *
  *                                                                  *
  *  Unlike most of Earthworm where "unix" means "Linux", in this    *
  *  case "unix" means "unix", which covers both Linux and Solaris.  *
  *  This basically was startstop main() for Linux and Solaris.      *
  *  Dropped in this library here so that it can be easily accessed  *
  *  by all unix platforms. If you need to make changes, please      *
  *  put them in here with #ifdefs rather than creating new code     *
  *  branches.                                                       *

  ********************************************************************/

#include <ctype.h>
#include <inttypes.h>
#include <limits.h>
#include <signal.h>
#include <string.h>
#include <strings.h>

#include "startstop_version.h"
#include "startstop_unix_generic.h"
#include "earthworm_complex_funcs.h"
#include "ew_nevent_message.h"

#define PROGRAM_NAME "startstop"

#define MAX_STATUS_LINE_LEN 80 /* number of chars a status line can be */

#ifndef MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#endif

#define NUM_CAUGHT_SIGNALS 6
int signals_to_catch[NUM_CAUGHT_SIGNALS] = {
    SIGFPE,
    SIGILL,
    SIGSEGV,
    SIGBUS,
    SIGABRT,
    SIGTERM };
char *signal_names[NUM_CAUGHT_SIGNALS] = {
    "Floating point exception",
    "Illegal instruction",
    "Segment fault",
    "Bus error",
    "Abort",
    "Termiation request" };


/* Prototypes for local functions
 *********************************/
int StopChild ( char *restartmsg, int *ich );
thr_ret StopChildThread( void *restartmsg );
thr_ret RestartChildThread( void *restartmsg );

/* noexec FIFO file
 ******************/
char noexec_fname[256];
int  noexec_fd;

/* Child's stderr status:
   0 = normal
   1 = redirect to /dev/null
   2 = redirect to another file in log directory
 ***********************************************/
char childStderr[MAX_CHILD] = {0};

static int 	done;
static int 	nChild;
static CHILD 	*child;
static METARING	metaring;

int RunEarthworm ( int argc, char *argv[] )
{
   /* Allot space for status message
    ********************************/
   int  i, j, rc, placeholder;
   char *runPath;
   char *logPath;
   char      msg[512];
   long      msgsize = 0L;
   MSG_LOGO  getLogo[4]; /* if you change this '3' here, you need to change the parameter to tport_getmsg as well */
   MSG_LOGO  msglogo;
#ifdef _USE_POSIX_SIGNALS
   struct sigaction act;
#endif

   metaring.LogLocation = metaring.ParamLocation = metaring.BinLocation = NULL;
   done=0;

   strcpy ( metaring.Version, STARTSTOP_VERSION );
   strcat ( metaring.Version, VERSION_APPEND_STR ); /* add "64 bit" indicator string */
/* Set name of configuration file
   ******************************/
   strcpy ( metaring.ConfigFile, DEF_CONFIG );
   if ( argc == 2 ) {
         if ((strlen(argv[1]) == 2) /* checking for /v or -v or /h or -h */
                  && ((argv[1][0] == '/')
                  || (argv[1][0] == '-'))) {
            if ((argv[1][1] == 'v') || (argv[1][1] == 'V')) {
                printf("%s %s%s\n",PROGRAM_NAME, STARTSTOP_VERSION, VERSION_APPEND_STR);
            } else if ((argv[1][1] == 'h') || (argv[1][1] == 'H')) {
                printf("%s %s%s\n",PROGRAM_NAME, STARTSTOP_VERSION, VERSION_APPEND_STR);
                printf("usage: %s <config file name>\n", PROGRAM_NAME);
                printf("       If no config file name is used, then the default config file is used,\n");
                printf("       in this case %s.\n", DEF_CONFIG);
            }
            exit (0);
        } else {
            strcpy ( metaring.ConfigFile, argv[1] );
        }
    }

   logit_init( metaring.ConfigFile, (short) metaring.MyModId, MAXLINE*3, 1 );
   fprintf( stderr, "startstop: Using config file %s\n", metaring.ConfigFile );

/* Change working directory to environment variable EW_PARAMS value
   ***************************************************************/
   runPath = getenv( "EW_PARAMS" );

   if ( runPath == NULL )
   {
      printf( "startstop: Environment variable EW_PARAMS not defined." );
      printf( " Exiting.\n" );
      exit (-1);
   }

   if ( *runPath == '\0' )
   {
      printf( "startstop: Environment variable EW_PARAMS " );
      printf( "defined, but has no value. Exiting.\n" );
      exit (-1);
   }

   if ( chdir( runPath ) == -1 )
   {
      printf( "startstop: Params directory not found: %s\n", runPath );
      printf( "startstop: Reset environment variable EW_PARAMS." );
      printf( " Exiting.\n" );
      exit (-1);
   }

   metaring.ParamLocation = (char *)malloc( strlen(runPath)+1 );
   if ( metaring.ParamLocation != NULL )
       strcpy( metaring.ParamLocation, runPath );

   LockStartstop(metaring.ConfigFile, argv[0]);

 /* Get EW_LOG environment variable for metaring.LogLocation */

   logPath = getenv( "EW_LOG" );
   metaring.LogLocation = (char *)malloc( strlen(logPath)+1 );
   if ( metaring.LogLocation != NULL )
       strcpy ( metaring.LogLocation, logPath );

 /* Parse the PATH environment variable, and check each element to see if
    startstop is an executable file in that directory; if it is, we'll assume
    that's our bin directory. */
   {
       char *filename = argv[0];
       int save_bytes = strlen(filename)+2;
       char * dir_path = malloc( save_bytes );
       char * path_env = getenv("PATH");
       char * dir_start = path_env;
       char * dir_end = path_env;
       struct stat filestat;
       int dir_path_len;
       char * new_dir_path;
       while ( dir_path != NULL ) {
          /* Find the end of the path element */
          while ( *dir_end != 0 && *dir_end != ':' )
             dir_end++;
          dir_path_len = dir_end - dir_start;
          /* Build our proposed full path to startstop in dir_path */
          new_dir_path = realloc( dir_path, dir_path_len + save_bytes );
          if ( new_dir_path != NULL ) {
              dir_path = new_dir_path;
              strncpy( dir_path, dir_start, dir_path_len );
              dir_path[dir_path_len] = '/';
              strcpy( dir_path + dir_path_len + 1, filename );
              /* If the file exists there & is exeutable, BINGO */
              if(stat(dir_path, &filestat) == 0) {
                 if(filestat.st_mode & S_IXUSR) {
                    dir_path[dir_path_len] = 0;
                    /* This should never fail, since we're shrinking the string */
                    dir_path = realloc( dir_path, dir_path_len+1 );
                    metaring.BinLocation = dir_path;
                    break;
                 }
              }
          }
          if ( *dir_end == 0 )
          {
            break;
          }
          dir_start = dir_end = (dir_end+1);
       }

   }

/* Set our "nice" value back to the default value.
 * This does nothing if you started at the default.
 * Requested by D. Chavez, University of Utah.
 *************************************************/
 nice( -nice( 0 ) );

/* Catch process termination signals
   *********************************/
#ifdef _USE_POSIX_SIGNALS
   act.sa_flags = SA_SIGINFO; sigemptyset(&act.sa_mask);
   act.sa_sigaction = SigtermHandler;
   for ( i=0; i<NUM_CAUGHT_SIGNALS; i++ )
       sigaction(signals_to_catch[i], &act, (struct sigaction *)NULL);
   act.sa_flags = 0;
   act.sa_handler = SIG_IGN;
   sigaction(SIGINT, &act, (struct sigaction *)NULL);
   sigaction(SIGQUIT, &act, (struct sigaction *)NULL);
   sigaction(SIGTTIN, &act, (struct sigaction *)NULL);
#else
   sigignore( SIGINT );             /* Control-c */
   sigignore( SIGQUIT );            /* Control-\ */

   for ( i=0; i<NUM_CAUGHT_SIGNALS; i++ )
       signal( signals_to_catch[i], SigtermHandler );

/* Catch tty input signal, in case we are in background */
   sigignore( SIGTTIN );
#endif

/* Allocate array of children
   **************************/
   child = (CHILD *) calloc( MAX_CHILD, sizeof(CHILD) );
   for (i = 0; i < MAX_CHILD; i++) {
     for (j = 0; j < MAX_ARG; j++)
       child[i].argv[j] = NULL;
     child[i].use_uname[0] = '\0';
     child[i].use_gname[0] = '\0';
   }


/* Read configuration parameters
   *****************************/
   nChild    = 0;
   if ( GetConfig() == EXIT )
   {  /* error flagged during config read; abort program startup */
      return -1;
   }

/* Allocate one region structure per ring
   **************************************/
   metaring.Region = (SHM_INFO *) calloc( (size_t)MAX_RING, sizeof(SHM_INFO) );
   if ( metaring.Region == NULL )
   {
      printf( "startstop: Error allocating region structures. Exiting.\n" );
      return -1;
   }

/* Look up this installation id
   ****************************/
   if ( GetLocalInst ( &(metaring.InstId) ) != 0 )
   {
      printf( "startstop: Error getting local installation id. Exiting.\n" );
      return -1;
   }

/* Look up module id & message types of interest
   *********************************************/
   if ( GetModId( metaring.MyModName, &(metaring.MyModId) ) != 0 )
   {
      printf( "startstop: Invalid module name <%s>. Exiting.\n", metaring.MyModName );
      return -1;
   }

   if ( GetModId( "MOD_WILDCARD", &(metaring.ModWildcard) ) != 0 )
   {
      printf( "startstop: Invalid module name <MOD_WILDCARD>. Exiting.\n" );
      return -1;
   }

   if ( GetType( "TYPE_HEARTBEAT", &(metaring.TypeHeartBeat) ) != 0 )
   {
      printf( "startstop: Unknown message type <TYPE_HEARTBEAT>. Exiting.\n" );
      return -1;
   }

   if ( GetType( "TYPE_ERROR", &(metaring.TypeError) ) != 0 )
   {
      printf( "startstop: Unknown message type <TYPE_ERROR>. Exiting.\n" );
      return -1;
   }

   if ( GetType( "TYPE_RESTART", &(metaring.TypeRestart) ) != 0 )
   {
      printf( "startstop: Unknown message type <TYPE_RESTART>. Exiting.\n" );
      return -1;
   }

   if ( GetType( "TYPE_STOP", &(metaring.TypeStop) ) != 0 )
   {
      printf( "startstop: Unknown message type <TYPE_STOP>. Exiting.\n" );
      return -1;
   }
   if ( GetType( "TYPE_REQSTATUS", &(metaring.TypeReqStatus) ) != 0 )
   {
      printf( "startstop: Unknown message type <TYPE_REQSTATUS>. Exiting.\n" );
      return -1;
   }

   if ( GetType( "TYPE_STATUS", &(metaring.TypeStatus) ) != 0 )
   {
      printf( "startstop: Unknown message type <TYPE_STATUS>. Exiting.\n" );
      return -1;
   }

   if ( GetType( "TYPE_RECONFIG", &(metaring.TypeReconfig) ) != 0 )
   {
      printf( "startstop: Unknown message type <TYPE_RECONFIG>. Exiting.\n" );
      return -1;
   }

   parent.pid = getpid();
   parent.processName = argv[0];
   parent.args = (argc == 2 ? argv[1] : (char *)NULL);

/* Initialize name of log-file & open it
   *************************************/
   if ( argc == 2 )
     logit_init( argv[1], (short) metaring.MyModId, MAXLINE*3, metaring.LogSwitch );
   else
     logit_init( argv[0], (short) metaring.MyModId, MAXLINE*3, metaring.LogSwitch );

   logit( "" , "startstop: Read command file <%s>\n", metaring.ConfigFile );

/* Make startstop be the process group leader */
   if ( setpgid( parent.pid, parent.pid ) != 0 )
     logit( "e", "startstop: failed to set process group ID\n" );

/* Set the priority of startstop
   *****************************/
   SetPriority( P_MYID, parent.className, parent.priority );

/* Create and open FIFO for modules to report status into
   ******************************************************/
   sprintf( noexec_fname, "%s/.noexec_fifo.txt", metaring.LogLocation );
   mknod( noexec_fname, S_IFIFO | 0666, 0);
   noexec_fd = open( noexec_fname, O_RDONLY | O_NDELAY );
   if ( noexec_fd == -1 )
      logit( "e", "startstop: couldn't open noexec FIFO\n" );

/* Start everything
   ****************/
   StartEarthworm( argv[0] );
/* Watch transport rings for kill flags, status requests & restart requests
   ************************************************************************/

   for (i = 0; i < 4; i++) {
       getLogo[i].instid = metaring.InstId;
       getLogo[i].mod    = metaring.ModWildcard;
   }

   getLogo[0].type   = metaring.TypeRestart;
   getLogo[1].type   = metaring.TypeReqStatus;
   getLogo[2].type   = metaring.TypeReconfig;
   getLogo[3].type   = metaring.TypeStop;


   while ( !done )
   {
  /* Is "kill flag" set?
   ***********************/
	if ( tport_getflag( NULL ) == TERMINATE )
	 {
		StopEarthworm();
		return 0;
	 }

   /* Send hearbeat, if it's time
    *****************************/
      Heartbeat( &metaring );
      for ( i = 0; i < metaring.nRing; i++ )
      {

      /* Look for any interesting new messages
       ***************************************/
         rc = tport_getmsg( &(metaring.Region[i]), getLogo, 4,
                            &msglogo, &msgsize, msg, sizeof(msg)-1 );
         if( rc==GET_NONE || rc==GET_TOOBIG ) continue;
      /* Process a new message
       ***********************/
         msg[msgsize]='\0';

         if     ( msglogo.type == metaring.TypeRestart   ){
            ew_thread_t tid;
            if ( StartThreadWithArg( &RestartChildThread, msg, 0, &tid ) == -1 ) {
                logit( "et", "startstop: couldn't do restart in background\n");
                RestartChild( msg );
            }
         } else if( msglogo.type == metaring.TypeStatus )  {
            msg[msgsize]=0;
            fprintf(stderr,"%s\n",msg);
         } else if( msglogo.type == metaring.TypeReqStatus ) {
            SendStatus( i );
         } else if( msglogo.type == metaring.TypeReconfig ) {
            int rv;
            logit( "et" , "startstop: Reconfigure: Re-reading command file <%s>\n", metaring.ConfigFile );
            /* We're re-reading the startstop*.d file here, and adding any new modules or rings */
            /* Even if this new GetConfig returns an error, we don't want to bail out on our running earthworm */
            oldNChild = nChild;
            rv = GetUtil_LoadTableCore(0); /* reread earthworm.d and earthworm_global.d */
            if ( rv == -1 )
                logit( "et", "startstop: reconfigure rejected due to error\n" );
            else {
                GetConfig(); /* reread startstop*.d*/
                for ( j = metaring.nRing; j < (newNRing); j++ ) {
                    logit( "et" , "tport_create: creating ring number <%d>\n", j );
                    tport_create( &(metaring.Region[j]),
                        1024 * metaring.ringSize[j],
                        metaring.ringKey[j] );
                    metaring.nRing ++;
                }
                if (nChild > oldNChild ) {
                    logit( "et" ,
                        "startstop: Adding child, oldNChild=%d, nChild=%d, statMgrLoc=%d\n",
                             oldNChild, nChild, metaring.statmgr_location );
                } else {
                    logit( "et" ,
                        "startstop: Not adding child, oldNChild=%d, nChild=%d, statMgrLoc=%d\n",
                             oldNChild, nChild, metaring.statmgr_location );
                }
                SpawnChildren();
                if (metaring.statmgr_location != -1) {
                    logit( "et" , "startstop: Final reconfigure step: Restart statmgr\n" );
                    sprintf (msg, "%d", child[metaring.statmgr_location].pid);
                    RestartChild( msg );
                }
            }
         } else if( msglogo.type == metaring.TypeStop   ) {
            ew_thread_t tid;
            if ( StartThreadWithArg( &StopChildThread, msg, 0, &tid ) == -1 ) {
                logit( "et", "startstop: couldn't do stop in background\n");
                StopChild( msg, &placeholder );
            }
         } else {
            logit("et","Got unexpected msg type %d from transport region: %s\n",
                        (int) msglogo.type, metaring.ringName[i] );
         }
      }
      sleep_ew( 1000 );
    }
   StopEarthworm();
   return 0;
} /* end RunEarthworm */

 /********************************************************************
  *                            GetConfig()                           *
  *                                                                  *
  *  Processes command file using kom.c functions.                   *
  *  Returns 'EXIT' if any errors are encountered.                   *
  *  Commands are expected in a given order in startstop.cnf         *
  ********************************************************************/

int GetConfig()
{
   int      state;        /* the part of the config file you are reading  */
   char     missed[30];
   char    *com;
   char    *str;
   char    *ptr;
   int      nfiles,success;
   int      i, ir, j;
   int      ichild;
   boolean  firstTimeThrough = FALSE;
   boolean  duplicate = FALSE;

/* Definitions for state while reading file
 ******************************************/
   #define READ_NRING       0
   #define READ_RINGINFO    1
   #define READ_MODID       2
   #define READ_HEARTBEAT   3
   #define READ_MYCLASS     4
   #define READ_MYPRIORITY  5
   #define READ_LOGSWITCH   6
   #define READ_KILLDELAY   7
   #define READ_OPTIONAL    8
   #define READ_PROCESS     9
   #define READ_PRIORITY   10
   #define READ_CHILD_OPT  11

/* Initialize some things
 ************************/
   state     = READ_NRING;
   missed[0] = '\0';
   metaring.maxStatusLineLen = MAX_STATUS_LINE_LEN;
   metaring.HardKillDelay = 0;
   metaring.statmgr_location = -1; 	/* some startstops (testing) may not use statmgr */

   if (nChild == 0){
       firstTimeThrough = TRUE;
       oldNChild = 0;
   }
/* Open the main configuration file
 **********************************/
   nfiles = k_open( metaring.ConfigFile );
   if ( nfiles == 0 )
   {
        printf( "startstop: Error opening command file <%s>. Exiting.\n",
                 metaring.ConfigFile );
        return EXIT;
   }

/* Process all command files
   *************************/
   while (nfiles > 0)              /* While there are command files open */
   {
        while (k_rd())             /* Read next line from active file  */
        {
            com = k_str();         /* Get the first token from line */

        /* Ignore blank lines & comments
           *****************************/
            if( !com )           continue;
            if( com[0] == '#' )  continue;
            if( com[0] == '@' ) {
                success = nfiles+1;
                nfiles  = k_open(&com[1]);
                if ( nfiles != success ) {
                    printf( "startstop: Error opening command file from @ directive <%s>. Exiting.\n", &com[1]);
                    logit( "e", "startstop: Error opening command file from @ directive <%s>; exiting!\n",  &com[1] );
                    return EXIT;
                }
                continue;
            }

        /* Process anything else as a command;
           Expect commands in a certain order!
           ***********************************/
            switch (state)
            {

            /* Read the number of transport rings
               **********************************/
            case READ_NRING:
                if ( !k_its( "nRing" ) )
                {
                    strcpy( missed, "nRing" );
                    break;
                }
                if (firstTimeThrough) {/* Only if we're reading the config for the first time do we keep track */
                    metaring.nRing  = k_int();
                }
                state  = READ_RINGINFO;
                if (firstTimeThrough) {
                    ir = 0;
                } else {
                    ir = metaring.nRing;
                }
                break;

            /* Read the transport ring names & keys & size
               *******************************************/
            case READ_RINGINFO:
                if ( !k_its( "Ring" ) )
                {
                    if ((k_its( "MyModuleId" )) && (!firstTimeThrough)) {
                        /* OK to skip MyModuleId on 2nd time through; we already know it */
                        state = READ_HEARTBEAT;
                        /* if this isn't the first time through we want to add the rings we just found to nRing */
                        newNRing = ir;

                    } else {
                       strcpy( missed, "Ring" );
                    }
                    break;
                }
                if ( ir == MAX_RING )
                {
                    logit( "e" , "startstop: Too many Ring commands, max=%d;"
                            " exiting!\n", MAX_RING );
                    return EXIT;
                }
                str = k_str();
                if( !str ) break;
                if ( strlen( str ) > 19 )
                {
                  printf( "startstop: Ring name <%s> too long; exiting!\n",
                      str );
                  return EXIT;
                }
                for ( i = 0; i < ir; i++ ) {
                    if ( strcmp (str, metaring.ringName[i]) == 0 ){
                        duplicate = TRUE;
                    }
                }
                if (duplicate) {
                    if (firstTimeThrough) {
                        logit( "e", "Duplicate ring name <%s>; exiting!\n", str);
                    }
                    duplicate = FALSE;
                } else {
                    strcpy( metaring.ringName[ir], str );
                    metaring.ringSize[ir] = k_int();
                    if ( (metaring.ringKey[ir]= GetKey(metaring.ringName[ir])) == -1 )
                    {
                       logit( "e" , "startstop: Invalid Ring name <%s>; exiting!\n",
                               metaring.ringName[ir] );
                       return EXIT;
                    }
                    if ( ++ir == metaring.nRing )  state = READ_MODID;
                }
                break;

            /* Read stuff concerning startstop itself
               **************************************/
            case READ_MODID:
                if ( !k_its("MyModuleId") )
                {
                    strcpy( missed, "MyModuleId" );
                    break;
                }
                str = k_str();
                if ( !str ) break;
                if ( strlen( str ) > 39 )
                {
                  logit( "e" , "startstop: MyModuleId name <%s> too long;"
                                  " exiting!\n", str );
                  return EXIT;
                }
                strcpy( metaring.MyModName, str );
                state = READ_HEARTBEAT;
                break;

            case READ_HEARTBEAT:
                if ( !k_its("HeartbeatInt") )
                {
                    strcpy( missed, "HeartbeatInt" );
                    break;
                }
                metaring.HeartbeatInt = k_int();
                state = READ_MYCLASS;
                break;

            case READ_MYCLASS:
                if ( !k_its("MyClassName") )
                {
                    strcpy( missed, "MyClassName" );
                    break;
                }
                str = k_str();
                if ( !str ) break;
                if ( strlen( str ) > MAX_CLASSNAME_SIZE )
                {
                  logit( "e" , "startstop: MyClassName <%s> too long; exiting!\n",
                      str );
                  return EXIT;
                }
                strcpy( parent.className, str );
                state = READ_MYPRIORITY;
                break;

            case READ_MYPRIORITY:
                if ( !k_its("MyPriority") )
                {
                    strcpy( missed, "MyPriority" );
                    break;
                }
                parent.priority = k_int();
                state  = READ_LOGSWITCH;
                break;

            case READ_LOGSWITCH:
                if ( !k_its("LogFile") )
                {
                    strcpy( missed, "LogFile" );
                    break;
                }
                metaring.LogSwitch = k_int();
                state     = READ_KILLDELAY;
                break;

        case READ_KILLDELAY:
          if ( !k_its("KillDelay") )
          {
        strcpy( missed, "KillDelay" );
        break;
          }
          metaring.KillDelay = k_int();
          state     = READ_OPTIONAL;
          break;

        /* Optional command to tell startstop to wait a number
         of seconds after starting statmgr John Patton*/
        case READ_OPTIONAL:
          if ( k_its("statmgrDelay") )
          {
            metaring.statmgr_sleeptime = (k_int() * 1000);
          }
          else if ( k_its("maxStatusLineLen") )
          {
            int len = k_int();
            metaring.maxStatusLineLen = (80 > len ? 80 : len);
          }
          else if ( k_its("HardKillDelay") )
          {
            metaring.HardKillDelay = k_int();
          }
          else if ( k_its( "Stderr" ) ) {
               int val = -1;
               fprintf( stderr, "Global Stderr\n" );
               str = k_str();
               if ( strcmp( str, "Console" ) == 0 )
                  val = 0;
               else if ( strcmp( str, "None" ) == 0 )
                  val = 1;
               else if ( strcmp( str, "File" ) == 0 )
                  val = 2;
               else
                  logit( "e", "startstop: Illegal argument to Stderr: '%s'\n", str );
               fprintf( stderr, "Global Stderr: %s (%d)\n", str, val );
               if ( val != -1 )
                  memset( childStderr+1, val, MAX_CHILD-1 );
               break;
            }
          else
          {
            state     = READ_PROCESS;

            /* since current command isn't optional, jump to the next */
            /*  command in line instead of declaring an error */
            goto not_optional;
          }
          break;

           /* Read a command to start a child
              *******************************/
            case READ_PROCESS:
                not_optional:
                if ( !k_its("Process") )
                {
                    strcpy( missed, "Process" );
                    break;
                }
                Get_Process:  /* back door from optional "Agent" (below) */
                if ( nChild == MAX_CHILD )
                {
                    logit( "e" , "startstop: Too many child processes in file %s, max=%d;"
                            " exiting!\n", metaring.ConfigFile, MAX_CHILD );
                    return EXIT;
                }
                str = k_str();
                if ( !str ) break;
                if ( strlen( str ) > MAXLINE - 1 )
                {
                    logit( "e" , "startstop: Process command line <%s> too long in file %s,"
                            " max=%d; exiting!\n", str, metaring.ConfigFile, MAXLINE - 1 );
                    return EXIT;
                }
               duplicate = FALSE;
               for ( ichild = 0; ichild < nChild; ichild++ ) {
                   if ( strcmp (str, child[ichild].commandLine) == 0 ) {
                       /* logit ( "", "Skipping twin, no duplicate children allowed: %s\n", str); */
                       duplicate = TRUE;
                   }
               }
               if (duplicate) {
                   state = READ_PRIORITY;
                   break;
               }
                strcpy( child[nChild].commandLine, str );
                strcpy( child[nChild].parm, str );


                /* Cut the command line into tokens
                ********************************/
                j = 0;

                ptr = strtok( child[nChild].parm, " " );
                child[nChild].argv[j++] = ptr;

                do
                {
                    ptr = child[nChild].argv[j++] = strtok( NULL , " " );
                }
                while ( ptr != NULL );

                child[nChild].processName = child[nChild].argv[0];

                if (strcmp( child[nChild].processName, "statmgr") == 0)
                {
                    /* Store statmgr's location in the child array so that we can start it first */
                    metaring.statmgr_location = nChild;
                }

                state = READ_PRIORITY;
                break;

            /* Read the child's priority
               *************************/
            case READ_PRIORITY:

                if ( !k_its("Class/Priority") )
                {
                    strcpy( missed, "Class/Priority" );
                    break;
                }
                str = k_str();
                if ( !str ) break;
                if (duplicate) {
                   state = READ_CHILD_OPT;
                   break;
                }
                if ( strlen ( str ) > MAX_CLASSNAME_SIZE )
                {
                    logit( "e" , "startstop: Class name <%s> too long; exiting!\n",
                            str );
                    return EXIT;
                }
                strcpy( child[nChild].className, str );
                child[nChild].priority = k_int();

                CreateSpecificMutex( &(child[nChild].mutex) );

                (nChild)++;
                state = READ_CHILD_OPT;
                break;

        /* Read the child's agent
           **********************/
        case READ_CHILD_OPT:
            if ( k_its( "Stderr" ) ) {
               str = k_str();
               if ( strcmp( str, "Console" ) == 0 )
                  childStderr[nChild-1] = 0;
               else if ( strcmp( str, "None" ) == 0 )
                  childStderr[nChild-1] = 1;
               else if ( strcmp( str, "File" ) == 0 )
                  childStderr[nChild-1] = 2;
               else
                  logit( "e", "startstop: Illegal argument to Stderr: '%s'\n", str );
               break;
            }
            if ( !k_its("Agent") )
            {   /* "Agent" is optional; if it's missing then the
                 * the command should be another "Process" */
                if ( k_its("Process") ) goto Get_Process;
                break;
            }
            str = k_str();
            if ( !str ) break;
            if (duplicate) {
               state = READ_PROCESS;
               break;
            }
            if ( strlen( str ) > LOGNAME_MAX )
            {
                logit( "e" , "startstop: Agent user <%s> too long; exiting!\n",
                        str );
                return EXIT;
            }
            strcpy( child[nChild - 1].use_uname, str );
            str = k_str();
           if (( !str ) || duplicate) break;
            if ( strlen( str ) > LOGNAME_MAX )
            {
                logit( "e" , "startstop: Agent group <%s> too long; exiting!\n",
                        str );
                return EXIT;
            }
            strcpy( child[nChild - 1].use_gname, str );
            state = READ_PROCESS;
            break;
        } /*end switch*/

        /* Complain if we got an unexpected command
           ****************************************/
            if( missed[0] )
            {
                logit( "e" , "startstop:  Expected: <%s>  Found: <%s>\n",
                         missed, com );
                logit( "e" , "startstop:  Incorrect command order in <%s>;",
                         metaring.ConfigFile );
                logit( "e" , " exiting!\n" );
                return EXIT;
            }

        /* See if there were any errors processing this command
           ****************************************************/
            if( k_err() )
            {
               logit( "e" , "startstop: Bad <%s> command in <%s>; exiting!\n",
                        com, metaring.ConfigFile );
               logit( "e" , "Offending line: %s\n", k_com() );
               return EXIT;
            }
        }
        nfiles = k_close();
   }
   return SUCCESS;
}

 /******************************************************************
  *                             Threads()                          *
  ******************************************************************/

void Threads( void *fun( void * ), ew_thread_t *tid ) /* *tid is pointer to thread id */
{
   const size_t stackSize = 0;          /* Use default values */
#ifdef _USE_PTHREADS
   /* Just use the ew library call (since POSIX doesn't do daemons) */
   if ( StartThread(fun,stackSize,(ew_thread_t*)tid) == -1 ) {
    fprintf(stderr, "startstop: can't create thread\n");
    exit( -1 );
   }
#else
   int rc;  /* used in non-P_THREADS situations */
   rc = thr_create( (void *)0, stackSize, fun, (void *)0,
                    THR_DETACHED|THR_NEW_LWP|THR_DAEMON, tid );
   if ( rc != 0 )
   {
    perror( "startstop: thr_create" );
    exit( -1 );
   }
#endif
}

  /******************************************************************
   *                         SigtermHandler()                       *
   *                                                                *
   *             Stop the whole system based on SIGTERM             *
   ******************************************************************/

#ifdef _USE_POSIX_SIGNALS
void SigtermHandler( int sig, siginfo_t *sip, void *up )
#else
void SigtermHandler( int sig )
#endif
{
    int i;
    for ( i=0; i<NUM_CAUGHT_SIGNALS; i++ )
        if ( signals_to_catch[i] == sig ) {
            fprintf( stderr, "%s: Stopping earthworm\n", signal_names[i] );
            break;
        }
    if ( i==NUM_CAUGHT_SIGNALS )
        fprintf( stderr, "Unexpected signal #%d; Stopping earthworm\n", sig );
    StopEarthworm();
    exit(0);
}


  /******************************************************************
   *                         StopEarthworm()                        *
   *                                                                *
   *                      Stop the whole system                     *
   ******************************************************************/

void StopEarthworm()
{
   int    i;
   int    status;
   int    ret;
   char   tstr[TIMESTR_LEN];
   int    noChildAlive;
#ifdef _USE_POSIX_SIGNALS
   struct    sigaction act;
#endif

/* Say goodbye
   ***********/
   GetCurrentUTC( tstr );
   logit( "t", "Earthworm stopping at local time: %s\n", tstr );

/* Set kill flag, and wait for the children to terminate.
   If the children don't die in KillDelay seconds, send them a SIGTERM signal.
   ******************************************************************/
   logit( "t", "Earthworm setting kill flag in each ring\n" );
   for ( i = 0; i < metaring.nRing; i++ )
      tport_putflag( &(metaring.Region[i]), TERMINATE );

   printf( "Earthworm will stop in %d seconds...\n", metaring.KillDelay );
   noChildAlive = 0;    // Assume there is a child process still running
   for ( i = 0; i < metaring.KillDelay; i++ )
   {
#ifdef _LINUX
      if ( (ret=waitpid( (pid_t)0, &status, WNOHANG |__WALL )) == -1 && errno == ECHILD )
#else
      if ( (ret=waitpid( (pid_t)0, &status, WNOHANG )) == -1 && errno == ECHILD )
#endif
      {
         noChildAlive = 1;
         break;
      }
      sleep_ew( 1000 );
   }
#ifdef _LINUX
  if ( (ret=waitpid( (pid_t)0, &status, WNOHANG |__WALL )) == -1 && errno == ECHILD )
#else
  if ( (ret=waitpid( (pid_t)0, &status, WNOHANG )) == -1 && errno == ECHILD )
#endif
      {
         noChildAlive = 1;
      }

   if ( !noChildAlive ) {
/* Catch process termination signals.
   Send a kill signal to the current process group.
   Wait for all children to die.
   ************************************************/
#ifdef _USE_POSIX_SIGNALS
       act.sa_flags = 0; sigemptyset(&act.sa_mask);
       act.sa_handler = SIG_IGN;
       for ( i=0; i<6; i++ )
           sigaction(signals_to_catch[i], &act, (struct sigaction *)NULL);
#else
       for ( i=0; i<6; i++ )
           sigignore(signals_to_catch[i]);
#endif
       logit( "t", "Earthworm sending SIGTERM to all processes\n" );
       kill( (pid_t)0, SIGTERM );

       printf( "Earthworm will REALLY stop in %d more seconds...\n", metaring.HardKillDelay );
       for ( i = 0; i < metaring.HardKillDelay; i++ )
       {
#ifdef _LINUX
          if ( (ret=waitpid( (pid_t)0, &status, WNOHANG |__WALL )) == -1 && errno == ECHILD )
#else
          if ( (ret=waitpid( (pid_t)0, &status, WNOHANG )) == -1 && errno == ECHILD )
#endif
          {
             noChildAlive = 1;
             break;
          }
          sleep_ew( 1000 );
       }
   }

#ifdef _LINUX
  if ( (ret=waitpid( (pid_t)0, &status, WNOHANG |__WALL )) == -1 && errno == ECHILD )
#else
  if ( (ret=waitpid( (pid_t)0, &status, WNOHANG )) == -1 && errno == ECHILD )
#endif
      {
         noChildAlive = 1;
      }

   if ( !noChildAlive ) {
       logit( "t", "Earthworm sending SIGKILL to all child processes\n" );
       for ( i = 0; i < nChild; i++ )
           kill( (pid_t)(child[i].pid), SIGKILL );
       while ( waitpid( (pid_t)0, &status, 0 ) > 0 );
   }

/* Close & remove the noexec FIFO
   ******************************/
   logit( "t", "Earthworm removing noexec FIFO\n" );
   if ( noexec_fd != -1 )
       close( noexec_fd );
   unlink( noexec_fname );

/* Destroy shared memory regions
   *****************************/
   logit( "t", "Earthworm destroying Rings\n" );
   for( i = 0; i < metaring.nRing; i++ )
     tport_destroy( &(metaring.Region[i]) );
   tport_destroyFlag();
   tport_destroyNamedEventRing();

/* Destroy the mutexen
   ********************/
   logit( "t", "Earthworm releasing mutexes\n" );
   for( i = 0; i < nChild; i++ )
        CloseSpecificMutex( &(child[i].mutex) );

/* Free allocated space
   ********************/
   logit( "t", "Earthworm freeing memory\n" );
   if ( metaring.LogLocation != NULL )
       free( metaring.LogLocation );
   if ( metaring.ParamLocation != NULL )
       free( metaring.ParamLocation );
   if ( metaring.BinLocation != NULL )
       free( metaring.BinLocation );
   free( metaring.Region );
   free( child  );

   logit( "t", "Earthworm shutdown complete\n" );
   printf( "Earthworm shutdown complete\n" );
}

/******************************************************************
 *                          delayinggetc()                        *
 *                                                                *
 *   Version of getc for a non-blocking file which progressively  *
 *   waits from 0.375 to 3 seconds before giving up               *
 ******************************************************************/
static int delayinggetc( FILE *fp )
{
    int delay = 375;
    int ch;
    while ( delay <= 3000 )
    {
        ch = getc( fp );
        if ( (ch != EOF) || feof(fp) )
            return ch;
        ferror( fp );
        if ( (errno == EWOULDBLOCK) || (errno == EAGAIN) )
        {
            sleep_ew( delay );
            delay += delay;
        } else {
            return EOF;
        }
    }
    return EOF;
}


 /******************************************************************
 *                   AddArgsToStatusLine()                        *
 *                                                                *
 *  Add elements of args to line, using truncation as necessary   *
 *  to make sure line is no longer than metaring.maxStatusLineLen *
 *  chars long                                                    *
 ******************************************************************/
void AddArgsToStatusLine( char* args[], char* line, int hardcap )
{
  int j;
  int line_len = strlen(line);
  for (j = 1; j < MAX_ARG && args[j] != NULL; j++) {
    int arg_len = strlen( args[j] );
    int notlast_arg = ( args[j+1] != NULL ) ? 1 : 0;
    int cap = hardcap - notlast_arg;
    int d_arg = (arg_len>2 && args[j][arg_len-1]=='d' && args[j][arg_len-2]=='.') ? 1 : 0;
    int main_len = arg_len;
    int over = (line_len + main_len + 1) - cap;

    while ( over > 0 )
    {
	  if ( d_arg ) {
        /* Knock off trailing .d */
	    main_len -= 2;
	    over = (line_len + main_len + 1) - cap;
	    d_arg = 0;
	    continue;
      }
      else if (main_len-over-3 > 1-notlast_arg && main_len > 3)
      {
        /* Trim arg from front with < */
        line[line_len] = ' ';
        strncpy( line+line_len+1, args[j]+over, main_len-over );
        if ( over!=0 )
          line[line_len+1] = '<';
        if ( notlast_arg )
          line[hardcap-1] = '>';
        line[hardcap] = 0;
      }
      else
      {
        /* Can't fit enough; just tack on a > */
        int shift = (line_len < hardcap-1) ? 1 : 0;
        line[line_len] = ' ';
        line[line_len+shift] = '>';
        line[line_len+1+shift] = 0;
      }
      return;
    }
    line[line_len] = ' ';
    strncpy( line+line_len+1, args[j], main_len );
    line[line_len+1+main_len] = 0;
    line_len += (main_len+1);
  }
}


/* truncate a line longer than len, at len chars with a newline */
void _truncate_with_newline(char *buffer, int len) {
   if (strlen(buffer) > len) {
        buffer[len]='\n';
        buffer[len+1]=0;
   }
}
 /******************************************************************
 *                          EncodeStatus()                        *
 *                                                                *
 *                    Encode the status message                   *
 ******************************************************************/

void EncodeStatus( char *statusMsg )
{
   FILE *fp;
   char phrase[MAXLINE];
   char *line;
   char string[20];
   char tty[20], tim[20];
   char tstr[TIMESTR_LEN];
   int i;
   int dummy;
   struct statvfs buf;
   struct utsname uts;
   pid_t status, pid;
   int ch, line_len, heading;
   int pn_width, excess;

#ifdef _USE_SCHED
#ifdef _MACOSX
#else
   struct sched_param sc_prm;
   int sc_pol;
#endif
#else
   int j;
   union
   {
      tsparms_t tsp;
      rtparms_t rtp;
      long      clp[PC_CLPARMSZ];
   } u;

   pcinfo_t  cid;
   pcparms_t pcp;
#endif

#if defined(_LINUX) /* Tested on RedHat and Fedora Core */
   gid_t groupid; /* need <unistd.h> and <sys/types.h> */

/* Get cpu used for the parent and each child
   ******************************************/
   groupid = getgid(); /* gets group ID for current process */
   sprintf( phrase, "/bin/ps -G %d %c", groupid, 0 );
#elif defined(_MACOSX)
   sprintf( phrase, "/bin/ps -o pid,state,time,command -g %d%c", parent.pid, 0 );
#else /* below line works for Solaris */
   sprintf( phrase, "/usr/bin/ps -g %d%c", parent.pid, 0 );
#endif
   i = metaring.maxStatusLineLen + 80;
   line = malloc(MAXLINE>i ? MAXLINE : i);
   if ( line == NULL )
   {
      printf( "startstop/EncodeStatus: couldn't allocate line.\n" );
      return;
   }

   fp = popen( phrase, "r" );
   if ( fp == NULL )
   {
      printf( "startstop/EncodeStatus: popen failed.\n" );
      free( line );
      return;
   }

   strcpy( parent.tcum, "0:00" );             /* Initialize to zero cpu used */
   for ( i = 0; i < nChild; i++ )
      strcpy( child[i].tcum, "0:00" );

   fcntl(fileno(fp), F_SETFL, O_NONBLOCK);    /* Make read non-blocking */

   line_len = 0;
   heading = 1;
   for ( ch = delayinggetc( fp ); 1; ch = delayinggetc( fp ) )
   {
     if ( (ch==EOF) && !feof(fp) )
     {
       /* delaying getc failed; give up */
       printf( "startstop/EncodeStatus: Error reading process information\n" );
       break;
     }
     if ( ch == EOF || ch == 10 || ch == 13 )
     {
       /* end-of line */
       if ( line_len > 0 && heading == 0 )
       {
         line[line_len] = 0;
         sscanf( line, "%d%s%s", &pid, tty, tim );
         if ( parent.pid == pid )
           strcpy( parent.tcum, tim );
         for ( i = 0; i < nChild; i++ )
           if ( child[i].pid == pid )
             strcpy( child[i].tcum, tim );
       }
       line_len = heading = 0;
       if ( ch == EOF )
         break;
     } else {
       line[line_len++] = ch;
     }
   }
   pclose( fp );

/* Get system information
   **********************/
   if ( uname( &uts ) == -1 )
      printf( "startstop/EncodeStatus: Error getting system information\n" );

   sprintf( statusMsg, "                    HyEARTHWORM-%zu SYSTEM STATUS\n\n",
                       CHAR_BIT * sizeof( void * ) );

   sprintf( line, "        Hostname-OS:            %s - %s %s\n",
                          uts.nodename, uts.sysname, uts.release );
   strcat( statusMsg, line );
   sprintf( line, "        Start time (UTC):       %s", metaring.tstart );
   strcat( statusMsg, line );

   GetCurrentUTC( tstr );
   sprintf( line, "        Current time (UTC):     %s", tstr );
   strcat( statusMsg, line );

   if ( statvfs( ".", &buf ) == -1 )
      printf( "startstop: Error getting file system info\n" );
   else {
      sprintf( line, "        Disk space avail:       %jd kb\n",
        (intmax_t) ( buf.f_frsize ? (buf.f_bavail * buf.f_frsize) / 1024 :
                     buf.f_bsize ? (buf.f_bavail * buf.f_bsize) / 1024 : buf.f_bavail ) );
      strcat( statusMsg, line );
   }

   for ( i = 0; i < metaring.nRing; i++ )
   {
      sprintf( line, "        Ring %2d name/key/size:  %s / %d / %ld kb\n",
               i+1, metaring.ringName[i], metaring.ringKey[i], metaring.ringSize[i] );
      strcat( statusMsg, line );
   }

   if ( metaring.LogLocation != NULL )
   {
       sprintf( line,    "        Startstop's Log Dir:    %s\n", metaring.LogLocation );
       _truncate_with_newline(line, MAX_STATUS_LINE_LEN);
       strcat( statusMsg, line );
   }
   if ( metaring.ParamLocation != NULL )
   {
       sprintf( line,    "        Startstop's Params Dir: %s\n", metaring.ParamLocation );
       _truncate_with_newline(line, MAX_STATUS_LINE_LEN);
       strcat( statusMsg, line );
   }
   if ( metaring.BinLocation != NULL )
   {
       sprintf( line,    "        Startstop's Bin Dir:    %s\n", metaring.BinLocation );
       _truncate_with_newline(line, MAX_STATUS_LINE_LEN);
       strcat( statusMsg, line );
   }

   sprintf( line,        "        Startstop Version:      %s\n", metaring.Version );
   strcat( statusMsg, line );

   if ( metaring.maxStatusLineLen == MAX_STATUS_LINE_LEN )
       pn_width = 16;
   else {
       int width, arg_width = 8;
	   pn_width = MAX( 7, strlen( parent.processName ) );
	   if ( parent.args != (char *)NULL )
		  arg_width = MAX( arg_width, strlen( parent.args ) );
	   for ( i = 0; i < nChild; i++ ) {
		  int j;
		  width = strlen( child[i].processName );
		  if ( width > pn_width )
			 pn_width = width;
	      width = -1;
		  for ( j=1; child[i].argv[j]; j++ )
		     width += ( 1 + strlen(child[i].argv[j]) );
		  arg_width = MAX( arg_width, width );
	   }
	   width = pn_width + 36 + arg_width;
	   if ( pn_width > 16 && width > metaring.maxStatusLineLen )
	   	  pn_width = MAX( 16, pn_width - (width + 16 - metaring.maxStatusLineLen)/2 );
   }


/* Get and print status of the parent process
   ******************************************/
   sprintf( line, "\n%*s  Process               Class/    CPU\n", pn_width, "Process" );
   strcat( statusMsg, line );
   sprintf( line,   "%*s    Id      Status     Priority   Used    Argument\n", pn_width, " Name  " );
   strcat( statusMsg, line );
   sprintf( line,   "%*s  -------   ------     --------   ----    --------\n", pn_width, "-------" );
   strcat( statusMsg, line );

   sprintf( phrase, "%*s", pn_width, parent.processName );
   excess = strlen(parent.processName) - pn_width;
   if ( excess <= 0 )
      strcpy( line, phrase );
   else {
      line[0] = '<';
      strcpy( line+1, parent.processName+excess+1 );
   }
   sprintf( phrase, " %7d", parent.pid );
   strcat( line, phrase );
   sprintf( phrase, "    Alive    " );
   strcat( line, phrase );

#ifdef _USE_SCHED
   /* get POSIX scheduling policy */
#ifdef _MACOSX
   strcat( line, "   **/**" );
#else /* not _MACOSX */
   if ( (sc_pol = sched_getscheduler(parent.pid)) == -1 ) {
      printf( "startstop/EncodeStatus: parent sched_getscheduler error: %s\n",strerror(errno));
      strcat( line, "    **" );
   } else {
      sprintf( phrase, "   %s",
#if !defined(_LINUX)
      sc_pol == SCHED_NP ? "  NP" :
      sc_pol == SCHED_TS ? "  TS" :
#endif /* ndef _LINUX */
      sc_pol == SCHED_FIFO ? "FIFO" :
      sc_pol == SCHED_RR ? "  RR" : "  ??");
      strcat( line, phrase );
   }
   /* get POSIX scheduling priority */
   if ( sched_getparam(parent.pid, &sc_prm) ) {
      printf( "startstop/EncodeStatus: parent sched_getparam error: %s\n",strerror(errno));
      strcat( line, "/**" );
   } else {
      sprintf( phrase, "/%2d", sc_prm.sched_priority );
      strcat( line, phrase );
   }
#endif /* _MACOSX */
#else  /* _USE_SCHED */
/* Get and print class of parent
   *****************************/
   pcp.pc_cid = PC_CLNULL;
   if ( priocntl( P_PID, parent.pid, PC_GETPARMS,
                  (caddr_t)&pcp ) == -1 )
      printf( "startstop/EncodeStatus: Error getting parent parameters\n" );

   cid.pc_cid = pcp.pc_cid;
   if ( priocntl( (idtype_t)0, (id_t)0, PC_GETCLINFO,
                  (caddr_t)&cid ) == -1 )
      printf( "startstop/EncodeStatus: priocntl getclinfo error\n" );
   sprintf( phrase, "   %s", cid.pc_clname );
   strcat( line, phrase );

/* Get and print priority of parent
   ********************************/
   for ( j = 0; j < PC_CLPARMSZ; j++ )
      u.clp[j] = pcp.pc_clparms[j];

   if ( strcmp( cid.pc_clname, "RT" ) == 0 )
   {
      sprintf( phrase, "/%2d", u.rtp.rt_pri );
      strcat( line, phrase );
   }

   else if ( strcmp( cid.pc_clname, "TS" ) == 0 )
   {
      sprintf( phrase, "/%2d", u.tsp.ts_upri );
      strcat( line, phrase );
   }
   else
   	  strcat( line, "/**" );
#endif  /* _USE_SCHED */

/* Print cumulative cpu time used by parent
   ****************************************/
   sprintf( phrase, "%9s", parent.tcum );
   strcat( line, phrase );

/* Print the parent argument list without the command itself */
   if ( parent.args != (char *)NULL ) {
    sprintf( phrase, "  %s\n", parent.args );
    strcat( line, phrase );
    if ( strlen(line) > metaring.maxStatusLineLen ) {
        line[metaring.maxStatusLineLen-1] = '\n';
        line[metaring.maxStatusLineLen] = '\0';
    }
   } else
    strcat(line,"  -\n");
   strcat( statusMsg, line );

/* Get and print status of each child
   **********************************/
   /* See if any children have reported failure to execute */
   if ( noexec_fd != -1 ) {
   	   unsigned char ich;
       while ( read( noexec_fd, &ich, 1 ) == 1 ) {
           strcpy( child[ich].status, "NoExec" );
       }
   }

   /* build a line in `line[]', to check its length */
   for ( i = 0; i < nChild; i++ )
   {
	   sprintf( phrase, "%*s", pn_width, child[i].processName );
	   excess = strlen(child[i].processName) - pn_width;
	   if ( excess <= 0 )
		  strcpy( line, phrase );
	   else {
		  line[0] = '<';
		  strcpy( line+1, child[i].processName+excess+1 );
	   }
      sprintf( phrase, " %7d", child[i].pid );
      strcat( line, phrase );
      //XXX XXX XXX
      //Adding in restarting condition here:
      if ( strcmp( child[i].status, "Restarting") == 0 ) {
         strcpy(string,        "    Restarting" );
      } else  if ( strcmp( child[i].status, "NoExec" ) == 0 ) {
      	strcpy( string,       "    NoExec   " );
      	status = -1;
      } else {
        /* Assume its a zombie until we know otherwise */
      	strcpy( string,       "    Zombie      " );
#ifdef _LINUX /* Linux-only __WALL wait-all */
        status = waitpid( child[i].pid, &dummy, WNOHANG | __WALL );
/*
      fprintf(stderr, "SLATE DEBUG: waitpid( WNOHANG|__WALL) for %d returns %d errno=%d \n", child[i].pid, status, errno);
*/
#else /*_LINUX */
        status = waitpid( child[i].pid, &dummy, WNOHANG );
#endif /*_LINUX */
        if ( status == 0 ) {
            if ( strcmp( child[i].status, "Start" ) == 0 )
                /* We now know it is alive */
                strcpy( child[i].status, "Alive" );
            strcpy( string,      "    Alive     " );
        }
        if ( (status == -1) && (errno == ECHILD) ) {
            if ( strcmp( child[i].status, "Stopped" ) == 0 ) {
                strcpy( string, "    Stop        " );
            } else if ( noexec_fd == -1 && strcmp( child[i].status, "Start" ) == 0 ) {
                /* It started, but our FIFO is missing and there is no process:
                	assume it failed to actually run */
                strcpy( child[i].status, "NoExec" );
                strcpy( string, "    NoExec      " );
            } else {
                strcpy( string, "    Dead        " );
            }
        }
      }
      sprintf( phrase, "%s", string );
      strcat( line, phrase );

/* Get and print class of each child
   *********************************/
      if (status == 0) {
#ifdef _USE_SCHED
        /* get POSIX scheduling policy */
#ifdef _MACOSX
        strcat( line, "  **/**" );
#else /* _MACOSX */
        if ( (sc_pol = sched_getscheduler(child[i].pid)) == -1 ) {
           printf( "startstop/EncodeStatus: child (%d) sched_getscheduler error: %s\n",i,strerror(errno));
           strcat( line, "   **" );
        } else {
           sprintf( phrase, "  %s",
#if !defined(_LINUX)
        sc_pol == SCHED_NP ? "  NP" :
        sc_pol == SCHED_TS ? "  TS" :
#endif /* ndef _LINUX */
        sc_pol == SCHED_FIFO ? "FIFO" :
        sc_pol == SCHED_RR ? "  RR" : "  ??");
           strcat( line, phrase );
        }
        /* get POSIX scheduling priority */
        if ( sched_getparam(child[i].pid, &sc_prm) ) {
           printf( "startstop/EncodeStatus: child (%d) sched_getparam error: %s\n",i,strerror(errno));
           strcat( line, "/**" );
        } else {
           sprintf( phrase, "/%2d", sc_prm.sched_priority );
           strcat( line, phrase );
        }
#endif /* _MACOSX */
#else /* _USE_SCHED */
        pcp.pc_cid = PC_CLNULL;
        if ( priocntl( P_PID, child[i].pid, PC_GETPARMS,
                       (caddr_t)&pcp ) == -1 )
          printf( "startstop/EncodeStatus: Error getting child parameters\n" );

        cid.pc_cid = pcp.pc_cid;
        if ( priocntl( (idtype_t)0, (id_t)0, PC_GETCLINFO,
                       (caddr_t)&cid ) == -1 )
          printf( "startstop/EncodeStatus: priocntl getclinfo error\n" );
        sprintf( phrase, "  %s", cid.pc_clname );
        strcat( line, phrase );

        /* Get and print priority of each child
        ************************************/
        for ( j = 0; j < PC_CLPARMSZ; j++ )
          u.clp[j] = pcp.pc_clparms[j];

        if ( strcmp( cid.pc_clname, "RT" ) == 0 ) {
          sprintf( phrase, "/%2d", u.rtp.rt_pri );
        } else if ( strcmp( cid.pc_clname, "TS" ) == 0 ) {
          sprintf( phrase, "/%2d", u.tsp.ts_upri );
        } else
          strcpy( phrase, "/**" );
        strcat( line, phrase );
#endif /* _USE_SCHED */
        /* Print cumulative cpu time used by each child
        ********************************************/
        sprintf( phrase, "%9s ", child[i].tcum );
        strcat( line, phrase );
      } else {  /* status != 0 */
        sprintf( phrase, "                 " );
        strcat( line, phrase );
      }

      /* Print the argument list without the command itself
       ****************************************************/
      AddArgsToStatusLine( child[i].argv, line, metaring.maxStatusLineLen );
      strcat( statusMsg, line );

/* Attach a newline to the end of the line
   ***************************************/
      strcat( statusMsg, "\n" );
   }
   free( line );
   return;
} /* end EncodeStatus */


  /******************************************************************
   *                          ConstructIds                         *
   *     Look up user id and group id numbers for Agents            *
   ******************************************************************/
void ConstructIds( char *user, char *group, uid_t *uid, gid_t *gid )
{
    struct group grp;
    struct passwd passwd;
    char    grbuffer[BUFSIZ];
    char    pwbuffer[BUFSIZ];
    uid_t   my_uid;
    struct passwd my_passwd;
    char   mypwbuffer[BUFSIZ];

#if _USE_PTHREADS
    struct passwd pwe, *pwptr;
    struct group gre, *grptr;
#endif

    my_uid = getuid();
#if _USE_PTHREADS
    pwptr = & my_passwd;
    getpwuid_r( my_uid, &pwe, mypwbuffer, BUFSIZ, &pwptr );
#else
    getpwuid_r( my_uid, &my_passwd, mypwbuffer, BUFSIZ );
#endif

    if( !strcmp( user, "" ) )
    {   /* Use the real userID if user hasn't been specified */
        *uid = getuid();
    }

    /* Don't allow "root" as the Agent user; use the real uid instead */
    else if ( !strcmp( user, "root") && my_uid != 0 )
    {
      fprintf( stderr, "`root' user Agent not permitted; using `%s'\n",
           my_passwd.pw_name );
      *uid = my_uid;
    }
#if _USE_PTHREADS
    else if( ! (pwptr = &passwd, getpwnam_r( user, &pwe, &pwbuffer[0], BUFSIZ, &pwptr )) )
#else
    else if( ! getpwnam_r( user, &passwd, &pwbuffer[0], BUFSIZ ) )
#endif
    {
        fprintf( stderr, "Failed to find password entry for user %s\n",
             user );
        *uid = my_uid;
    }
    else
    {
        *uid = passwd.pw_uid;
    }

    if( !strcmp( group, "" ) )
    {   /* Use the real groupIP if group hasn't been specified */
        *gid = getgid();
    }
#if _USE_PTHREADS
    else if( ! (grptr = &grp, getgrnam_r( group, &gre, &grbuffer[0], BUFSIZ, &grptr )) )
#else
    else if( ! getgrnam_r( group, &grp, &grbuffer[0], BUFSIZ ) )
#endif
    {
        fprintf( stderr, "Failed to find entry for group %s\n", group );
        *gid = getgid();
    }
    else
    {
        *gid = grp.gr_gid;
    }

    return;
} /* end ConstructIds */



  /******************************************************************
   *                           RestartChild                         *
   *                                                                *
   *      Restart a specific child, given a TYPE_RESTART msg        *
   ******************************************************************/

void RestartChild( char *restartmsg )
{
    boolean NotInitialStartup = TRUE;

    int ich = 0, ret;

    /* stop */
    ret = StopChild( restartmsg, &ich );
    if (ret == EXIT) {
       return;
    }

    /* and now start */
    ConstructIds( child[ich].use_uname,
                  child[ich].use_gname,
                  &child[ich].use_uid,
                  &child[ich].use_gid );
    switch ( StartChild( ich, NotInitialStartup ) )
    {
        case -1:
            logit("et","startstop: failed to restart <%s>\n",
                  child[ich].processName);
            break;
        case 0:
            logit("et","startstop: successfully restarted <%s> <pid: %d>\n",
                  child[ich].processName, child[ich].pid);
            break;
        default:
            break;
    }

    /* Release the mutex that StopChild acquired but didn't release */
    ReleaseSpecificMutex( &(child[ich].mutex) );

} /* end RestartChild */

  /******************************************************************
   *                     RestartChildThread                         *
   *                                                                *
   *  Wrapper around RestartChild so it can be called in a thread   *
   ******************************************************************/

thr_ret RestartChildThread( void *restartmsg )
{
    RestartChild( (char*)restartmsg );
    return(NULL);
}


  /******************************************************************
   *                           StopChild                            *
   *                                                                *
   *     Stop a specific child given a TYPE_RESTART or              *
   *     TYPE_STOP message                                          *
   ******************************************************************/
int StopChild ( char *restartmsg, int *ich )
{
   int ir, childNum;
   int status  = 0;
   int procId  = 0;
   int nsec    = 0;
   char ErrText[MAXLINE];
   int delay;
   int stage;
   char *stageName;
   int notDeadYet;

/* Definitions for state while reading file
 ******************************************/
   #define STOP_BY_FLAG       0
   #define STOP_BY_SIGTERM    1
   #define STOP_BY_SIGKILL    2
   #define STOPPED            3
   #define STOP_ABORT         4

   /* Find this process id in the list of children
    **********************************************/
      procId = atoi( restartmsg );

      for ( childNum = 0; childNum < nChild; childNum++ )
      {
         if ( child[childNum].pid == procId ) break;
      }

      if( childNum==nChild )
      {
         sprintf( ErrText, "Cannot restart pid=%d; it is not my child!\n",
                  procId);
         ReportError( ERR_STARTCHILD, ErrText, &metaring );
         return EXIT;
      }

   /* Get exclusive access to this child
    *************************************/
      RequestSpecificMutex( &(child[childNum].mutex) );
      strcpy( child[childNum].status, "Restarting" );
   /* Kill the child in stages:
    *   First, try flagging it to off itself (stage==STOP_BY_FLAG)
    *   Next, try signalling it with SIGTERM (stage==STOP_BY_SIGTERM)
    *   Finally, if HardKillDelay>0, send SIGKILL (stage==STOP_BY_SIGKILL)
    ************************************************/
      if ( tport_newModule( child[childNum].pid ) )
	      tport_putflag( NULL, child[childNum].pid );
	  else
		  for( ir=0; ir<metaring.nRing; ir++ )
			  tport_putflag( &(metaring.Region[ir]), child[childNum].pid );

      nsec = 0;
      notDeadYet = 1;
      delay = metaring.KillDelay;
      stage = STOP_BY_FLAG;

      while( stage != STOPPED )
      {
          notDeadYet = (waitpid((pid_t)child[childNum].pid, &status, WNOHANG) == 0);
          if ( notDeadYet )
          {
             if (nsec > delay )
             {
                switch ( stage ) {
                case STOP_BY_FLAG:
                    logit( "et", "startstop: <%s> (pid=%d) did not self-terminate in %d sec; terminating it!\n",
                        child[childNum].processName, procId, delay );
#ifdef _UNIX
                    kill( (pid_t)child[childNum].pid, SIGTERM );
#else
                    sigsend( P_PID, (pid_t)child[childNum].pid, SIGTERM );
#endif
                    stage = STOP_BY_SIGTERM;
                    stageName = "terminat";
                    nsec = -1;
                    break;
                case STOP_BY_SIGTERM:
                    if ( metaring.HardKillDelay == 0 )
                    {
                        stage = STOP_ABORT;
                        break;
                    }
                    logit( "et", "startstop: <%s> (pid=%d) did not respond to SIGTERM in %d sec; killing it!\n",
                        child[childNum].processName, procId, delay );
#ifdef _UNIX
                    kill( (pid_t)child[childNum].pid, SIGKILL );
#else
                    sigsend( P_PID, (pid_t)child[childNum].pid, SIGKILL );
#endif
                    stage = STOP_BY_SIGKILL;
                    stageName = "kill";
                    delay = metaring.HardKillDelay;
                    nsec = -1;
                    break;
                 case STOP_BY_SIGKILL:
                    stage = STOP_ABORT;
                }
                if ( stage == STOP_ABORT )
                {
                   sprintf( ErrText, "%sing <%s> (pid=%d) in %d sec failed%s\n",
                             stageName, child[childNum].processName, procId, nsec,
                             ich==NULL ? "." : "; cannot restart!" );
                   ReportError( ERR_STARTCHILD, ErrText, &metaring );

                   /* Because we failed, caller won't release mutex for us */
                   ReleaseSpecificMutex( &(child[childNum].mutex) );
                   return EXIT;
                }
             }
             if ( nsec > -1 )
                 sleep_ew(1000);
             nsec++;
          } else
                break;
      }
      strcpy( child[childNum].status, "Stopped" );
      for( ir=0; ir<metaring.nRing; ir++ )
      	  tport_detachFromFlag( &(metaring.Region[ir]), child[childNum].pid );
      if ( ich != NULL )
          /* Caller will handle release of mutex */
          *ich = childNum;
      else
          ReleaseSpecificMutex( &(child[childNum].mutex) );
      return SUCCESS;
}

  /******************************************************************
   *                     StopChildThread                            *
   *                                                                *
   *  Wrapper around StopChild so it can be called in a thread      *
   ******************************************************************/

thr_ret StopChildThread( void *restartmsg )
{
    StopChild( (char*)restartmsg, NULL );
    return(NULL);
}



  /******************************************************************
   *                         IORedirect                             *
   *                                                                *
   * Redirect stdin & stderr as specified in config file            *
   ******************************************************************/

static void IORedirect ( int ich ) {
     /* freopen( "/dev/null", "r", stdin ); */
     int fd = open("/dev/null", O_RDONLY);
     dup2(fd, 0);
     close(fd);

	 if ( childStderr[ich] == 1 ) {
		/* freopen( "/dev/null", "w", stderr ); */
        fd = open("/dev/null", O_WRONLY);
        dup2(fd, 1);
        close(fd);
	 } else if ( childStderr[ich] == 2 ) {
		char progName[50], *str = child[ich].argv[1];
		if ( str == NULL )
			str = child[ich].processName;

		if (get_prog_name2(str, progName, sizeof(progName)) == EW_SUCCESS)
		{
			char logPath[256], path[256], date[10];
			time_t now;
			struct tm res;

			strcpy ( logPath, metaring.LogLocation );
			ew_addslash (logPath);

			time( &now );
			gmtime_ew( &now, &res );
			sprintf( date, "%04d%02d%02d",
				(res.tm_year + TM_YEAR_CORR), (res.tm_mon + 1), res.tm_mday );

			sprintf( path, "%s%s_%s.err", logPath, progName, date );
			/* freopen( path, "a", stderr ); */
            fd = open(path, O_WRONLY);
            dup2(fd, 2);
            close(fd);

			fprintf( stderr, "\n-------------------------------------------------------\n" );
			fprintf( stderr, "stderr for %s %s %s\n",child[ich].processName,
				child[ich].argv[0], child[ich].argv[1] );
			fprintf( stderr, "startup at UTC_%s_%02d:%02d:%02d\n",
						date, res.tm_hour, res.tm_min, res.tm_sec );
			fprintf( stderr, "-------------------------------------------------------\n" );
			/* freopen( path, "a", stderr ); */
		} else {
			logit( "e", "startstop: Call to get_prog_name failed; stderr unredirected.\n");
		}
	 }
}


  /******************************************************************
   *                         SignalNoExec                           *
   *                                                                *
   * Tell startstop that a module failed to load                    *
   ******************************************************************/

static void SignalNoExec ( int ich ) {
    int noexec_fd;

	 noexec_fd = open( noexec_fname, O_WRONLY | O_NDELAY );
	 if ( noexec_fd != -1 ) {
		unsigned char uich = ich;
		if ( write( noexec_fd, &uich, 1 ) != 1 )
		   logit( "e", "startstop: couldn't write to FIFO failing start of %s\n", child[ich].processName );
		 close( noexec_fd );
	 } else {
		logit( "e", "startstop: couldn't open FIFO for writing failing start of %s\n", child[ich].processName );
	 }
}

  /******************************************************************
   *                           StartChild                           *
   *                                                                *
   * Start a specific child given by the index in the child array   *
   ******************************************************************/

int StartChild ( int ich, boolean NotInitialStartup ) {
    char ErrText[512];
    /*
     *     From fork(2) man page:
     *     In applications that use the Solaris threads API rather than
     *     the POSIX threads API (applications linked with -lthread but
     *     not -lpthread),fork() duplicates in the  child  process  all
     *     threads  (see  thr_create(3THR)) and LWPs in the parent pro-
     *     cess. The  fork1()  function  duplicates  only  the  calling
     *     thread (LWP) in the child process.
     *
     *     Thus, we use fork1 for restarts to take fewer resources
     */
    /* Mark status as having started the child; let EncodeStatus change to
       Alive when it sees the process alive */
    strcpy( child[ich].status, "Start" );
    if ( NotInitialStartup ) { /* not initial startup, so use fork1 */
        child[ich].pid = fork1();
    } else {
        child[ich].pid = fork();
    }
    switch ( child[ich].pid )
    {
      case -1:
         perror( "startstop: fork" );
         sprintf( ErrText, "fork failed; cannot start <%s>\n",
                  child[ich].processName );
         ReportError( ERR_STARTCHILD, ErrText, &metaring );
    	 strcpy( child[ich].status, "NoExec" );
         sleep_ew( 500 );
         return(-1);
         break;
      case 0:   /* the child */
         setgid( child[ich].use_gid );
         setuid( child[ich].use_uid );
          /* fprintf(stderr,"PIDS at exec %d %d\n",getpid(),getppid()); */
         fprintf(stderr,"%s %s %s\n",child[ich].processName,child[ich].argv[0],child[ich].argv[1] );
		 IORedirect( ich );
         execvp( child[ich].processName, child[ich].argv );
         /* logit("e", "debug: error with: child[ich].processName = %s\n", child[ich].processName); */
         perror( "startstop: execvp" );
         sprintf( ErrText, "execvp failed; cannot restart <%s>\n",
            child[ich].processName );
         ReportError( ERR_STARTCHILD, ErrText, &metaring );
		 SignalNoExec( ich );
         StartError( ich, child[ich].processName, &metaring, &nChild );
         /* StartError ends the child process completely */
         logit("e", "debug2: error with: child[ich].processName = %s %d\n", child[ich].processName, child[ich].pid);
         sleep_ew( 500 );
         return(1);
         break;
      default:  /* the parent */
         break;
    }
    SetPriority( child[ich].pid, child[ich].className, child[ich].priority );
    /* Sleep for 0.5 second to allow fork to complete it's business.
    This was motivated by message:
    "startstop: fork: Resource temporarily unavailable" on Sparc5 - newt
    ********************************************************************/
    return(0);

} /* end StartChild */

/******************************************************************
 *                         StartEarthworm()                       *
 *                                                                *
 *                      Start the whole system                    *
 ******************************************************************/

void StartEarthworm( char *ProgName )
{
   int      i;
   ew_thread_t tid;
   boolean RestartStatus = FALSE;

   /* log startstop version string (with 64-bit indicator) */
   logit("t", "Startstop Version:  %s%s\n", STARTSTOP_VERSION, VERSION_APPEND_STR);

   /* Print start time
   ****************/
   GetCurrentUTC( metaring.tstart );         /* Get UTC as a 26 char string */
   logit( "t", " Earthworm starting at local time: %s\n", metaring.tstart );

/* Create the transport rings
   **************************/
   tport_createFlag();
   tport_createNamedEventRing();
   for ( i = 0; i < metaring.nRing; i++ )
      tport_create( &(metaring.Region[i]), 1024 * metaring.ringSize[i], metaring.ringKey[i] );

/* Spawn the child processes.  Results are
   unpredictable if other threads start before this loop.
   *****************************************************/
#ifdef _SOLARIS

/* Start Statmgr first if present */

    /* Why would we use MAX_CHILD + 1 when we initialize this to -1??
     * Changed to -1 stefan 20121024  */
    /*      if (metaring.statmgr_location != (MAX_CHILD + 1)) */
    if (metaring.statmgr_location != -1)
        /* aka if we did find a statmgr in GetConfig */
    {

        ConstructIds( child[metaring.statmgr_location].use_uname,
                      child[metaring.statmgr_location].use_gname,
                      &child[metaring.statmgr_location].use_uid,
                      &child[metaring.statmgr_location].use_gid );
        strcpy( child[metaring.statmgr_location].status, "Start" );
        switch( child[metaring.statmgr_location].pid = fork() )
        {
            case -1:
                perror( "startstop: fork" );
                strcpy( child[metaring.statmgr_location].status, "NoExec" );
                exit( 1 );
                break;
            case 0: /* the child */
                setgid( child[metaring.statmgr_location].use_gid );
                setuid( child[metaring.statmgr_location].use_uid );
                IORedirect( metaring.statmgr_location );
                execvp( child[metaring.statmgr_location].processName, child[metaring.statmgr_location].argv );
                perror( "startstop: execvp" );
				SignalNoExec( metaring.statmgr_location );
                StartError( metaring.statmgr_location, child[metaring.statmgr_location].processName, &metaring, &nChild );
                break;
            default:    /* the parent */
                break;
        }
        SetPriority( child[metaring.statmgr_location].pid, child[metaring.statmgr_location].className, child[metaring.statmgr_location].priority );

        /* Tell the user why it's taking so long to start up */
        logit("et","startstop: sleeping <%d> second(s) for statmgr startup.\n",
              (metaring.statmgr_sleeptime/1000) );

        /* Sleep after starting statmgr to allow statmgr to come up first */
        sleep_ew(metaring.statmgr_sleeptime);
    }
#endif /* _SOLARIS */

   for ( i = 0; i < nChild; i++ )
   {
#ifdef _SOLARIS
        /* To prevent starting statmgr a second time, we'll just skip over it
         If it's not there, then the index to skip defaults to one past MAXCHILD, so
         no other modules will be skipped. */
         if (i != metaring.statmgr_location)
         {
#endif /* _SOLARIS */
            ConstructIds( child[i].use_uname,
                          child[i].use_gname,
                          &child[i].use_uid,
                          &child[i].use_gid );
            if ( StartChild ( i, RestartStatus ) != 0 ){
                logit("et","startstop: process <%s> <%d> failed to start.\n",
                                  child[i].parm, child[i].pid );
            } else {
                logit("et","startstop: process <%s> <%d> started.\n",
                                  child[i].parm, child[i].pid );
            }

#ifdef _SOLARIS
         }
#endif /* _SOLARIS */
   }


/* Start the interactive thread
   ****************************/
    Threads( Interactive, &tid );
   return;
} /* end StartEarthworm */


 /********************************************************************
  *                            SetPriority()                         *
  *                                                                  *
  *        Set the priorities of all threads in this process.        *
  *         Valid TS/RT priority ranges depend on O/S and POSIX      *
  *         compliance (RT 0 to 59 for solaris w/o POSIX).           *
  ********************************************************************/

void SetPriority( pid_t pid, char *ClassName, int Priority )
{

#ifdef _USE_SCHED
   struct sched_param sc_prm;
   int sc_pol, sc_pri_min, sc_pri_max;

   /* get POSIX scheduling policy for requested class */
   /* use NONE class to request no POSIX RT scheduling */
   sc_pol =
    strcmp( ClassName, "FIFO" ) == 0 ? SCHED_FIFO : /* FIFO (RR with no time out) */
    strcmp( ClassName, "RR" ) == 0 ? SCHED_RR : /* Round robbin */
    strcmp( ClassName, "RT" ) == 0 ? SCHED_RR : /* SOLARIS compatibility = RR */
#if !defined(_UNIX)
    strcmp( ClassName, "TS" ) == 0 ? SCHED_TS : /* Time share */
    strcmp( ClassName, "NP" ) == 0 ? SCHED_NP : /* who knows */
#endif
    strcmp( ClassName, "OTHER" ) == 0 ? SCHED_OTHER : -1;   /* OTHER -> revert to TS */
   if ( sc_pol == -1 ) {
      if ( strcmp( ClassName, "NONE" ) != 0 )
          printf( "startstop: ignoring unknown class: %s; converted to NONE class- no priority set\n",ClassName);
      return;
   }
   /* get POSIX min and max priority for requested policy */
   if ( (sc_pri_min = sched_get_priority_min(sc_pol)) == -1 ) {
      printf( "startstop: sched_get_priority_min error: %s\n",strerror(errno));
      return;
   }
   if ( (sc_pri_max = sched_get_priority_max(sc_pol)) == -1 ) {
      printf( "startstop: sched_get_priority_max error: %s\n",strerror(errno));
      return;
   }
   /* keep requested priority within allowed min and max priority */
   sc_prm.sched_priority = MIN(MAX(Priority,sc_pri_min),sc_pri_max);
   if ( sc_prm.sched_priority != Priority )
       printf( "startstop: requested %s priority (%d) not between %d and %d: reset to %d\n",
                ClassName,Priority,sc_pri_min,sc_pri_max,sc_prm.sched_priority);
   /* for safety, keep real-time priorities lower than all system and device processes */
# ifdef __sgi
#  define MAX_USR_PRI 89
   if ( (sc_pol == SCHED_FIFO || sc_pol == SCHED_RR) && sc_prm.sched_priority > MAX_USR_PRI ) {
      sc_prm.sched_priority = MAX_USR_PRI;
      printf( "startstop: requested priority (%d) too high: set to %d\n",
                Priority,sc_prm.sched_priority);
   }
#  undef MAX_USR_PRI
# endif
   /* set POSIX scheduling policy and priority */
#ifndef _MACOSX
   if ( sched_setscheduler(pid, sc_pol, &sc_prm) == -1 ) {
#if !defined(_UNIX)
      printf( "startstop: sched_setscheduler error: PID %d (policy/priority %s/%d):\n\t%s\n",
            pid, (sc_pol == SCHED_FIFO ? "FIFO" : sc_pol == SCHED_RR ? "RR" :
            sc_pol == SCHED_TS ? "TS" : sc_pol == SCHED_NP ? "NP" : "??"),
        sc_prm.sched_priority, strerror(errno));
#else /* _UNIX */
      printf( "startstop: sched_setscheduler error: PID %d (policy/priority %s/%d):\n\t%s\n",
            pid, (sc_pol == SCHED_FIFO ? "FIFO" : sc_pol == SCHED_RR ? "RR" :
             "??"),
        sc_prm.sched_priority, strerror(errno));
#endif /* _UNIX */
      printf( "\tCheck that startstop is setuid root!!\n");
      return;
   }
#endif /* ndef _MACOSX */
#else
   int i;
   union{
      tsparms_t tsp;
      rtparms_t rtp;
      long      clp[PC_CLPARMSZ];
   } u;

   pcinfo_t  cid;
   pcparms_t pcp;
   rtparms_t rtp;
   strcpy( cid.pc_clname, ClassName );
   if ( priocntl( P_PID, (id_t)pid, PC_GETCID, (caddr_t)&cid ) == -1 )
      perror( "startstop: priocntl getcid" );

   pcp.pc_cid = cid.pc_cid;

   if ( strcmp( ClassName, "TS" ) == 0 )
   {
      u.tsp.ts_uprilim = TS_NOCHANGE;            /* Use the default */
      u.tsp.ts_upri    = Priority;               /* Desired priority */
   }

   if ( strcmp( ClassName, "RT" ) == 0 )
   {
      u.rtp.rt_pri     = Priority;               /* Desired priority */
      u.rtp.rt_tqsecs  = 0;
      u.rtp.rt_tqnsecs = RT_TQDEF;               /* Use default time quantum */
   }
   for ( i = 0; i < PC_CLPARMSZ; i++ )
      pcp.pc_clparms[i] = u.clp[i];

   if ( priocntl( P_PID, (id_t)pid, PC_SETPARMS, (caddr_t)&pcp ) == -1 )
      perror( "startstop: priocntl setparms" );
#endif
} /* end SetPriority */

#ifdef _LINUX
  /*********************************************************************
				_getLinuxEWStatus()

	get the status by sending a request to the main ring and
	read it back (overcomes some wacky thread permissions on
	calling waitpid() in a thread that didn't do the fork calls

	this echos the status to stdout and can only be called by
	interactive
   *********************************************************************/
void _getLinuxEWStatus() {
   int       rc;
   char      msg[5120];
   long      msgsize = 0L;
   MSG_LOGO  getLogo[1]; /* if you change this '3' here, you need to change the parameter to tport_getmsg as well */
   MSG_LOGO  msglogo;
   int  getStatus;
   char      message[512];/**/
   MSG_LOGO  logo;/**/

/* Build status request message
   ****************************/
   sprintf(message,"%d\n", metaring.MyModId );

/* Set logo values of message
   **************************/
   logo.type   = metaring.TypeReqStatus;
   logo.mod    = metaring.MyModId;
   logo.instid = metaring.InstId;

/* Send status message to transport ring
   **************************************/
   if ( tport_putmsg( &(metaring.Region[0]), &logo, strlen(message), message ) != PUT_OK )
   {
    fprintf(stderr, "status: Error sending message to transport region.\n" );
   }
   getLogo[0].type = metaring.TypeStatus;
   getLogo[0].mod    = metaring.MyModId;
   getLogo[0].instid = metaring.InstId;
   getStatus=0;
   while( getStatus == 0) {
         rc = tport_getmsg( &(metaring.Region[0]), getLogo, 1,
                        &msglogo, &msgsize, msg, sizeof(msg)-1 );
         if( rc==GET_NONE || rc==GET_TOOBIG ) {sleep_ew(1000); continue;}
         if (msglogo.type == metaring.TypeStatus) {
                msg[msgsize] ='\0';
                printf("\n%s", msg);
                getStatus=1;
         }
   }

}

#endif /* _LINUX */



  /*********************************************************************
   *                           Interactive()                           *
   *                                                                   *
   *          Thread function to get commands from keyboard            *
   *********************************************************************/

void *Interactive( void *dummy) {
   char ewstat[MAX_STATUS_LEN];
   int  i, scan_return;
   char line[MAXLINE];
   char param[MAXLINE];
   char      message[512]; 	/* used for messages sent to ring for commands*/
   MSG_LOGO  logo;		/* used to describe the logo of those messages*/
   char     *ret; 		/* return value of fgets() */

   /* _getLinuxEWStatus(); */
   int quit = 0;

  if ( tport_getflag( NULL ) == TERMINATE )
  {
	  quit = 1;
  }

   if ( quit != 1 )
   {
      EncodeStatus( ewstat );/* One free status */
      printf( "\n%s", ewstat );
   }
   do
   {
      printf( "\n   Press return to print Earthworm status, or\n" );
      printf( "   type restart nnn where nnn is proc id or name to restart, or\n" );
      printf( "   type quit<cr> to stop Earthworm.\n\n" );

/* With SIGTTIN ignored, reading from tty (stdin) when in background
 * will return with errno set to EIO. Or if startstop is run from a script,
 * errno will be set to ESPIPE.
 * If that happens, just return from this thread.
 */

      ret = fgets(line, MAXLINE, stdin );

#if defined(_LINUX) || defined(_MACOSX)
      {
      if ((errno == EIO) || (errno == ESPIPE) ||(errno == ENOTTY))
      {
        fprintf(stderr,"Interactive() exiting: errno=%d '%s'\n",errno,strerror(errno));
        thr_exit( (void *)0 );
      }
      }
#endif

      if ((strlen(line) == 0) && ((errno == EIO) || (errno == ESPIPE))) {
        fprintf(stderr,"Interactive() thread exiting: errno=%d '%s'\n",errno,strerror(errno));
        thr_exit( (void *)0 );
      }

      /* one final failsafe check is needed on some Linux systems to catch closed standard input */
      if (ret == NULL)
      {
        if (errno != 0)
        {
            fprintf(stderr,"Interactive() thread exiting: fgets returned NULL and errno=%d '%s'\n",errno,strerror(errno));
        }
        else
        {
            fprintf(stderr,"Interactive() thread exiting: fgets() returns NULL\n");
        }
        thr_exit( (void *)0 );
      }

      if ( strncmp( line, "status", 6 ) == 0 || strlen( line ) == 1 )
      {
         EncodeStatus( ewstat );
         printf( "\n%s", ewstat );
      }


     param[0] = 0;
     scan_return = sscanf(line, "%*s %s", param);
     for ( i = 0; i < (int)strlen( line ); i++ )
        line[i] = tolower( line[i] );

     if (strncmp( line, "restart", 7)==0 && scan_return == 1)
     {
       int paramAsInt = atoi( param );

       for( i = 0; i < nChild; i++ )
       {
         if( strcmp(child[i].processName, param)==0 || child[i].pid==paramAsInt)
         {
	   		/* sending it to the ring rather than starting directly so statmgr can keep track */
           sprintf(param,"%d%c", child[i].pid,0);
       	   fprintf( stderr, "sending restart message to the ring for %s\n", param);
       	   SendRestartReq(&metaring, param);
           break;
         }
       }
	   if ( i >= nChild ) {
       	fprintf( stderr, "Could not find process '%s' to restart; ignoring\n", param );
       }
     }

     if (((strncmp( line, "stop", 4)==0) || (strncmp( line, "stopmodule", 10)==0)) && scan_return == 1)
     {
	   int paramAsInt = atoi( param );
	   for( i = 0; i < nChild; i++ )
       {
         if( strcmp(child[i].processName, param)==0 || child[i].pid==paramAsInt)
         {
		   /* sending it to the ring rather than starting directly so statmgr can keep track */
           sprintf(param,"%d%c", child[i].pid,0);
		   fprintf( stderr, "sending stop message to the ring for %s\n", param);
		   SendStopReq(&metaring, param);
           break;
         }
       }
	   if ( i >= nChild ) {
       	fprintf( stderr, "Could not find process '%s' to stop; ignoring\n", param );
       }
     }

     /* reconfigure */
     if ((strncmp( line, "recon", 5)==0) || (strncmp( line, "reconfigure", 11)==0)) {
        /* Send a message requesting reconfigure */
        /* message is just MyModId
           ****************************/
           sprintf(message,"%d\n", metaring.MyModId );
        /* Set logo values of message
           **************************/
           logo.type   = metaring.TypeReconfig;
           logo.mod    = metaring.MyModId;
           logo.instid = metaring.InstId;

        /* Send status message to transport ring
           **************************************/
           if ( tport_putmsg( &(metaring.Region[0]), &logo, strlen(message), message ) != PUT_OK ) {
                logit( "e" , "status: Error sending message to transport region.\n" );
                return 0;
           }
     }


     if (strcasecmp( line, "PAU\n" ) == 0 ) break;

   } while ( strcmp( line, "quit\n" ) != 0 );

   done = 1;

   thr_exit( (void *)0 );
   return 0;
} /* end Interactive */

/******************************************************************
 *                            SendStatus()                        *
 *    Build a status message and put it in a transport ring       *
 ******************************************************************/

void SendStatus( int iring )
{
   MSG_LOGO logo;
   int length;
   char ewstat[MAX_STATUS_LEN];

   logo.instid = metaring.InstId;
   logo.mod    = metaring.MyModId;
   logo.type   = metaring.TypeStatus;

   EncodeStatus( ewstat );
   length = strlen( ewstat );

   if ( tport_putmsg( &(metaring.Region[iring]), &logo, length, ewstat ) != PUT_OK )
      logit("t", "startstop: Error sending status msg to transport region: %s\n",
             metaring.ringName[iring] );
   return;
} /* end SendStatus */

void SpawnChildren (){
    /* Start the child processes
    *************************/
    int ichild;
    boolean NotInitialStartup = TRUE;

    /* The following changes were made by John Patton to fix the
     processes dying before statmgr comes up bug */
   for ( ichild = oldNChild; ichild < nChild; ichild++ )
   {
        /* To prevemt starting statmgr a second time, we'll just skip over it
          If it's not there, then the index to skip defaults to one past MAXCHILD, so
          no other modules will be skipped. */
        if (ichild != metaring.statmgr_location)
        {
            ConstructIds( child[ichild].use_uname,
                          child[ichild].use_gname,
                          &child[ichild].use_uid,
                          &child[ichild].use_gid );
            if ( StartChild( ichild, NotInitialStartup ) == 0 )
            {
                logit("et","startstop: process <%s> <%d> started.\n",
                  child[ichild].parm, child[ichild].pid );
            } else {
                logit("et","startstop: process <%s> <%d> failed to start.\n",
                                  child[ichild].parm, child[ichild].pid );
            }
        }
   }
} /* end SpawnChildren */
