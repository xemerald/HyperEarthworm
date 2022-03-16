
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: restart.c 7786 2019-02-13 05:28:18Z alexander $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.6  2019/02/13 05:19:23  alexander
 *     adding lookup for statmgr ring, defaults to first ring listed otherwise
 *
 *     Revision 1.5  2007/03/28 18:21:31  paulf
 *     added _MACOSX flag
 *
 *     Revision 1.4  2006/04/04 19:02:25  stefan
 *     removing deendency on old .h file
 *
 *     Revision 1.3  2005/07/27 17:26:09  friberg
 *     added _LINUX directive for appropriate startstop.h
 *
 *     Revision 1.2  2000/07/24 21:13:34  lucky
 *     Implemented global limits to module, installation, ring, and message type strings.
 *
 *     Revision 1.1  2000/02/14 19:10:41  lucky
 *     Initial revision
 *
 *
 */


/**********************************************************
 *                       restart                          *
 *    Program to restart an Earthworm module while        *
 *    startstop is running                                *
 **********************************************************/

/* 5/21/98: changed to allow config file to be specified on the command line.
 * If none is specified, then a default file is used depending on the OS.
 * PNL, UW Geophysics. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <kom.h>
#include <transport.h>
#include <earthworm.h>

/* include the appropriate definition of DEF_CONFIG for the OS */
#if defined(_SOLARIS) || defined(_LINUX) || defined(_MACOSX)
#include <startstop_unix_generic.h>
#endif

#ifdef _WINNT
#include <startstop_winlib.h>
#endif
#ifdef _OS2
#include <startstop_os2.h>
#endif

void SendRestartRequest( char* );
void ReadRingName( char*, char* );
//looks for statmgr process, attempts to read that ring
//returns 1 if Ringname found, 0 otherwise
int CheckForStatusRing( char *, char * );
int ReadStatusRingName( char *, char *); 

SHM_INFO   region;                /* The shared memory region   */
static unsigned char InstId;      /* local installation id      */
static unsigned char MyModId;
static unsigned char TypeRestart;
static int ringKey;

int main( int argc, char *argv[] )
{
  char *pid;
  char *runPath;
  char  ringname[MAX_RING_STR];
  char  configFile[FILENAME_MAX] = DEF_CONFIG;
  int   arg_index = 1;

/* Check the number of arguments.
   ******************************/

  if (argc < 2) 
  {
    fprintf( stderr, "Usage: restart [-c config_file] <pid> [<pid> ...]\n" );
    return -1;
  }
  if (strcmp(argv[1], "-c") == 0 ) 
  {
    strcpy(configFile, argv[2]);
    arg_index = 3;
  }


/* Change working directory to environment variable EW_PARAMS value
   ****************************************************************/
  runPath = getenv( "EW_PARAMS" );

  if ( runPath == NULL )
  {
      fprintf(stderr, "restart: Environment variable EW_PARAMS not defined." );
      fprintf(stderr, " Exiting.\n" );
      return -1;
  }

  if ( *runPath == '\0' )
  {
      fprintf(stderr, "restart: Environment variable EW_PARAMS " );
      fprintf(stderr, "defined, but has no value. Exiting.\n" );
      return -1;
  }

  if ( chdir_ew( runPath ) == -1 )
  {
      fprintf(stderr, "restart: Params directory not found: %s\n", runPath );
      fprintf(stderr, "restart: Reset environment variable EW_PARAMS." );
      fprintf(stderr, " Exiting.\n" );
      return -1;
  }

/* Look up ids in earthworm.d tables
 ***********************************/
   if ( GetLocalInst( &InstId ) != 0 )
   {
      fprintf( stderr,
               "restart: error getting local installation id; exiting!\n" );
      return -1;
   }

   if ( GetType( "TYPE_RESTART", &TypeRestart ) != 0 )
   {
      fprintf( stderr,
               "restart: Invalid message type <TYPE_RESTART> exiting!\n" );
      return -1;
   }

/* Read startstop's config file to find a valid transport ring name
 ******************************************************************/
   if (CheckForStatusRing( ringname, configFile ) == 0)
   {
      ReadRingName( ringname, configFile );
   }

   if( (ringKey = GetKey(ringname)) == -1 )
   {
       printf( "restart: Invalid ring name <%s>. Exiting.\n",
                ringname );
       return -1;
   }

/* Attach to shared memory ring
   *****************************/
   tport_attach( &region, ringKey );

   while (arg_index < argc)
   {
     pid = argv[arg_index++];
     SendRestartRequest( pid );
   }

   tport_detach( &region );
   return 0;
}

  /************************************************************
   *                   SendRestartRequest                     *
   *  To send a message requesting the restarting of a module *
   ************************************************************/
void SendRestartRequest( char*  modPid ) /* pid of module as ascii string*/
{
   MSG_LOGO logo;
   char message[512];

/* Build message
   *************/
   strcpy (message,modPid);
   strcat( message, "\n\0" );

/* Set logo values of pager message
   ********************************/
   logo.type   = TypeRestart;
   logo.mod    = MyModId;
   logo.instid = InstId;

/* Send restart message to transport ring
   **************************************/
   if ( tport_putmsg( &region, &logo, (long)strlen(message), message ) != PUT_OK )
      fprintf(stderr, "restart: Error sending message to transport region.\n" );

   return;

}


  /*************************************************************
   *                      ReadStatusRingName                   *
   * Read statmgr's config for 'RingName'                      *
   * Return 1 if successful, 0 otherwise
   *************************************************************/
int ReadStatusRingName( char *ringname, char *statmgrConfigName )
{
   int        nfiles;
   char      *com, *str;

   nfiles = k_open( statmgrConfigName );
   if ( nfiles == 0 )
   {
        printf( "restart: Error opening statmgr config <%s>. Exiting.\n",
                 statmgrConfigName );
        return 0;
   }

/* Process Ring commands from startstop's command file
   ***************************************************/
   while(nfiles > 0)          /* While there are command files open */
   {
        while(k_rd())         /* Read next line from active file  */
        {
            com = k_str();    /* Get the first token from line */

        /* Ignore blank lines & comments
           *****************************/
            if( !com )           continue;
            if( com[0] == '#' )  continue;

        /* Process only 1st Ring command
           *****************************/
           if( k_its( "RingName" ) )
           {
                str = k_str();
                strcpy( ringname, str );
                nfiles = k_close();
                return 1;
           }

        /* See if there were any errors processing this command
           ****************************************************/
           if( k_err() )
           {
               printf( "restart: Bad <%s> command in <%s>. Exiting.\n",
                        com, statmgrConfigName );
               return 0;
           }
        }
        nfiles = k_close();
   }

   return 0;
}

  /*************************************************************
   *                   CheckForStatusRing                      *
   * Read startstop's configfile, looks for statmgr process    *
   * If statmgr is found, attempts to read statmgr's .d file   *
   *   and then read the RingName in statmgr.d                 *
   *************************************************************/
int CheckForStatusRing( char *ringname, char *configFile )
{
   int        nfiles;
   char      *com, *str, *statmgrConfig, *ptr;

   nfiles = k_open( configFile );
   if ( nfiles == 0 )
   {
        printf( "restart: Error opening file <%s>. Exiting.\n",
                 configFile );
        return 0;
   }

/* Process Ring commands from startstop's command file
   ***************************************************/
   while(nfiles > 0)          /* While there are command files open */
   {
        while(k_rd())         /* Read next line from active file  */
        {
            com = k_str();    /* Get the first token from line */

        /* Ignore blank lines & comments
           *****************************/
            if( !com )           continue;
            if( com[0] == '#' )  continue;

        /* Look at processes:
           *****************************/
           if( k_its( "Process" ) )
           {
                str = k_str();
                ptr = strtok(str, " ");
                if( strcmp(ptr, "statmgr" ) == 0)
                {
                  statmgrConfig = strtok( NULL, " ");
                  nfiles = k_close();
                  return ReadStatusRingName(ringname,  statmgrConfig); 
                }
           }

        /* See if there were any errors processing this command
           ****************************************************/
           if( k_err() )
           {
               printf( "restart: Bad <%s> command in <%s>. Exiting.\n",
                        com, configFile );
               return 0;
           }
        }
        nfiles = k_close();
   }
   return 0;
}

  /*************************************************************
   *                      ReadRingName                         *
   * Read startstop's configfile as far as the first ring name *
   *************************************************************/
void ReadRingName( char *ringname, char *configFile )
{
   int        nfiles;
   char      *com, *str;

   nfiles = k_open( configFile );
   if ( nfiles == 0 )
   {
        printf( "restart: Error opening file <%s>. Exiting.\n",
                 configFile );
        return;
   }

/* Process Ring commands from startstop's command file
   ***************************************************/
   while(nfiles > 0)          /* While there are command files open */
   {
        while(k_rd())         /* Read next line from active file  */
        {
            com = k_str();    /* Get the first token from line */

        /* Ignore blank lines & comments
           *****************************/
            if( !com )           continue;
            if( com[0] == '#' )  continue;

        /* Process only 1st Ring command
           *****************************/
           if( k_its( "Ring" ) )
           {
                str = k_str();
                strcpy( ringname, str );
                break;
           }

        /* See if there were any errors processing this command
           ****************************************************/
           if( k_err() )
           {
               printf( "restart: Bad <%s> command in <%s>. Exiting.\n",
                        com, configFile );
               return;
           }
        }
        nfiles = k_close();
   }

   return;
}
