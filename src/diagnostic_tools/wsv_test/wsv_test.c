/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: wsv_test.c 4022 2010-09-01 17:05:53Z kohler $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.2  2007/02/26 14:57:30  paulf
 *     made sure time_t are casted to long for heartbeat sprintf()
 *
 *     Revision 1.1  2006/06/02 19:17:58  stefan
 *     *** empty log message ***
 *
 *     Revision 1.2  2005/09/27 22:59:49  dietz
 *     Added two more arguments (host note) to the Waveserver command for
 *     more human-friendly error reporting.
 *
 */

      /**********************************************************************
       *                         wsv_test program                           *
       *                                                                    *
       *  Report to the status manager if any wave server process dies.     *
       **********************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <earthworm.h>
#include <ws_clientII.h>
#include <kom.h>
#include <transport.h>

#define ERR_WSV_NOT_RESPONDING 0
#define ERR_WSV_RESPONDING     1

#define FALSE             0
#define TRUE              1
#define WSV_TEST_SUCCESS  0
#define WSV_TEST_FAILURE -1
#define MAX_WSV          60
#define MAX_ADRLEN       20         /* Size of waveserver address arrays */
 
typedef struct                      /* Waveserver processes being tested */
{
   char ip[MAX_ADRLEN];
   char port[MAX_ADRLEN];
   char host[MAX_ADRLEN];
   char note[MAX_ADRLEN];
   int  alive;
} WSV;
 
typedef struct                      /* IP addresses of computers tested */
{
   char ip[MAX_ADRLEN];
   char host[MAX_ADRLEN];
   int  nDead;                      /* Number of dead waveservers at this IP address */
} IP;

/* Function prototypes
 *********************/
void GetConfig( char * );
void LogConfig( void );
int  SendStatus( unsigned char, short, char * );
int  TestWaveServer( char *, char *, char * );
void GetIp( WSV [], int, IP [], int * );

/* Things to read or derive from config file
   *****************************************/
static long          RingKey;            /* key of transport ring to write to */
static unsigned char MyModId;            /* wsv_test's module id               */
static int           LogFile;            /* If 0, don't log to disk */
static int           LogAllTests;        /* If 0, don't log all tests */
static int           HeartBeatInterval;  /* Send heartbeats to statmgr this often (sec) */
static int           TestInterval;       /* Test all waveservers this often (sec) */
static int           wsTimeout;          /* Waveserver timeout in milliseconds */
static int           nWsv = 0;           /* Number of waveservers we know about */
static WSV           Wsv[MAX_WSV];       /* Waveserver IP address and port numbers */
static int           nIp;                /* Number of computers we know about */
static IP            Ip[MAX_WSV];        /* Unique IP addresses */

/* Things to look up in the earthworm.h tables with getutil.c functions
   ********************************************************************/
static unsigned char LocalInstId;
static unsigned char TypeHeartBeat;
static unsigned char TypeError;

SHM_INFO region;
static pid_t myPid;


int main( int argc, char *argv[] )
{
   int    i;
   time_t t_heart_prev;
   time_t t_prev;

/* Check command line arguments
   ****************************/
   if ( argc != 2 )
   {
        printf( "Usage: wsv_test <configfile>\n" );
        return 0;
   }

/* Initialize name of log file & open it
   *************************************/
   logit_init( argv[1], 0, 256, 1 );

/* Read configuration file
   ***********************/
   GetConfig( argv[1] );

/* Look up local installation id
   *****************************/
   if ( GetLocalInst( &LocalInstId ) != 0 )
   {
      logit( "e", "wsv_test: Error getting local installation id. Exiting.\n" );
      return -1;
   }

/* Look up message types from earthworm.h tables
   *********************************************/
   if ( GetType( "TYPE_HEARTBEAT", &TypeHeartBeat ) != 0 )
   {
      logit( "e", "wsv_test: Invalid message type <TYPE_HEARTBEAT>. Exiting.\n" );
      return -1;
   }

   if ( GetType( "TYPE_ERROR", &TypeError ) != 0 )
   {
      logit( "e", "wsv_test: Invalid message type <TYPE_ERROR>. Exiting.\n" );
      return -1;
   }

/* Get list of unique IP addresses of computers monitored
   ******************************************************/
   GetIp( Wsv, nWsv, Ip, &nIp );

/* Re-init logging with log-level from config
   ******************************************/
   logit_init( argv[1], 0, 256, LogFile );
   logit( "" , "wsv_test: Read command file <%s>\n", argv[1] );

/* Log the config file
   *******************/
   LogConfig();

/* Get our own pid for restart purposes
   ************************************/
   myPid = getpid();
   if ( myPid == -1 )
   {
      logit( "e", "pick_ew: Can't get my pid. Exiting.\n" );
      return -1;
   }

/* Attach to shared memory ring
   ****************************/
   tport_attach( &region, RingKey );

/* Assume all waveservers are alive at startup
   *******************************************/
   time( &t_prev );

   for ( i = 0; i < nWsv; i++ )
      Wsv[i].alive = TRUE;

   for ( i = 0; i < nIp; i++ )
      Ip[i].nDead = 0;

/* Send the first heartbeat
   ************************/
   time( &t_heart_prev );

   if ( SendStatus( TypeHeartBeat, 0, "" ) != PUT_OK )
   {
      logit( "t", "wsv_test:  Error sending heartbeat to ring. Exiting.\n");
      return -1;
   }

/* Loop until kill flag is set
   ***************************/
   while ( 1 )
   {
      time_t t_heart;
      time_t t_test;

/* Check kill flag
   ***************/
      if ( tport_getflag( &region ) == TERMINATE  ||
           tport_getflag( &region ) == myPid )
      {
         tport_detach( &region );
         logit( "et", "wsv_test: Termination requested. Exiting.\n" );
         return 0;
      }

/* Send heartbeat every HeartBeatInterval seconds
   **********************************************/
      time( &t_heart );
      if ( t_heart - t_heart_prev > HeartBeatInterval )
      {
         t_heart_prev = t_heart;

         if ( SendStatus( TypeHeartBeat, 0, "" ) != PUT_OK )
            logit( "et", "wsv_test:  Error sending heartbeat to ring.\n");
      }

/* Test waveservers every TestInterval seconds
   *******************************************/
      time( &t_test );
      if ( t_test - t_prev > TestInterval )
      {
         for ( i = 0; i < nWsv; i++ )
         {
            static char msg_tws[80];
            static char errmsg[180];
            int changeState = 0;

/* Test the state of a waveserver process by attempting to get its menu.
   TestWaveServer() returns either WSV_TEST_FAILURE or WSV_TEST_SUCCESS.
   A message string is returned in msg_tws.
   ********************************************************************/
            int rc_tws = TestWaveServer( Wsv[i].ip, Wsv[i].port, msg_tws );

            sprintf( errmsg, "%s %s (%s %s) %s", 
                     Wsv[i].ip, Wsv[i].port, Wsv[i].host, Wsv[i].note, msg_tws );

/* Log changes of state of each waveserver process.  A waveserver
   changes state if it goes from alive to dead, or from dead to alive.
   ******************************************************************/
            if ( Wsv[i].alive == TRUE )          /* Waveserver was previously alive */
            {
               if ( rc_tws == WSV_TEST_FAILURE ) /* Waveserver is now dead */
               {
                  Wsv[i].alive = FALSE;
                  changeState  = 1;
               }
            }
            else                                 /* Waveserver was previously dead */
            {
               if ( rc_tws == WSV_TEST_SUCCESS ) /* Waveserver is now alive */
               {
                  Wsv[i].alive = TRUE;
                  changeState  = 1;
               }
            }

            if ( changeState )
               logit( "et", "%s\n", errmsg );      /* Log to screen and file */
            else
               if ( LogAllTests )
                  logit( "t", "%s\n", errmsg );    /* Log to file only */
         }

/* Send a message to statmgr if any waveservers at an IP address
   have died, or if all waveservers at the address have recovered.
   **************************************************************/
         for ( i = 0; i < nIp; i++ )
         {
            int j;
            int nDead = 0;
            char statmsg[128];                     /* Message to statmgr */

/* Count the dead waveservers at this IP address
   *********************************************/
            for ( j = 0; j < nWsv; j++ )
               if ( (strcmp( Wsv[j].ip, Ip[i].ip) == 0) &&
                    (Wsv[j].alive == FALSE) ) nDead++;

/* Send message to statmgr if there are some dead
   waveservers and there weren't before
   **********************************************/
            if ( (Ip[i].nDead == 0) && (nDead > 0) )
            {
               if ( nDead > 1 )
                  sprintf( statmsg, "(%d) waveserver processes on %s (%s) are not responding.",
                     nDead, Ip[i].ip, Ip[i].host );
               else
                  sprintf( statmsg, "One waveserver process on %s (%s) is not responding.",
                     Ip[i].ip, Ip[i].host );

               if ( SendStatus( TypeError, ERR_WSV_NOT_RESPONDING, statmsg ) != PUT_OK )
                  logit( "et", "wsv_test:  Error sending error to ring.\n");
            }

/* Send message to statmgr if there are no dead
   waveservers and there were some before
   ********************************************/
            if ( (Ip[i].nDead > 0) && (nDead == 0) )
            {
               sprintf( statmsg, " All waveserver processes on %s (%s) are now responding.",
                  Ip[i].ip, Ip[i].host );

               if ( SendStatus( TypeError, ERR_WSV_RESPONDING, statmsg ) != PUT_OK )
                  logit( "et", "wsv_test:  Error sending error to ring.\n");
            }

            Ip[i].nDead = nDead;
         }
         time( &t_prev );
      }

      sleep_ew( 1000 );
   }
}


       /*******************************************************
        *                   TestWaveServer                    *
        *                                                     *
        *  Returns WSV_TEST_SUCCESS if the waveserver is ok   *
        *          WSV_TEST_FAILURE if the waveserver is bad  *
        *******************************************************/

int TestWaveServer( char *ip_address, char *port, char *msg_tws )
{
   WS_MENU_QUEUE_REC queue;                /* Points to the list of menus */
   int rc;
   int got_connection = WSV_TEST_FAILURE;

   queue.head = queue.tail = NULL;

   rc = wsAppendMenu( ip_address, port, &queue, wsTimeout );
 
   if ( rc == WS_ERR_NO_CONNECTION )
      strcpy( msg_tws, "Could not connect." );
 
   else if ( rc == WS_ERR_SOCKET )
      strcpy( msg_tws, "Could not create a socket." );
 
   else if ( rc == WS_ERR_BROKEN_CONNECTION )
      strcpy( msg_tws, "Connection broke during wsAppendMenu" );
 
   else if ( rc == WS_ERR_TIMEOUT )
      strcpy( msg_tws, "Connection timed out while getting menu." );
 
   else if ( rc == WS_ERR_MEMORY )
      strcpy( msg_tws, "Waveserver out of memory." );
 
   else if ( rc == WS_ERR_INPUT )
      strcpy( msg_tws, "Connection input error" );
 
   else if ( rc == WS_ERR_PARSE )
      strcpy( msg_tws, "Parser failed." );
 
   else if ( rc == WS_ERR_BUFFER_OVERFLOW )
      strcpy( msg_tws, "Buffer overflowed." );
 
   else if ( rc == WS_ERR_EMPTY_MENU )
      strcpy( msg_tws, "Menu is empty." );
 
   else if ( rc == WS_ERR_NONE )
   {
      strcpy( msg_tws, "Waveserver responding." );
      got_connection = WSV_TEST_SUCCESS;
   }
 
   wsKillMenu( &queue );          /* Close connections and kill the menu queue */

   return got_connection;
}


         /**************************************************
          *                   SendStatus                   *
          *  Builds heartbeat or error msg and puts it in  *
          *  shared memory.                                *
          **************************************************/

int SendStatus( unsigned char msg_type, short ierr, char *note )
{
   MSG_LOGO    logo;
   char        msg[256];
   int         res;
   long        size;
   time_t      t;

   logo.instid = LocalInstId;
   logo.mod    = MyModId;
   logo.type   = msg_type;

   time( &t );

   if( msg_type == TypeHeartBeat )
      sprintf ( msg, "%ld %d\n", (long) t, (int) myPid );
   else if( msg_type == TypeError )
      sprintf ( msg, "%ld %d %s\n", (long) t, ierr, note);

   size = strlen( msg );           /* don't include null byte in message */
   res = tport_putmsg( &region, &logo, size, msg );

   if ( msg_type == TypeError )
      logit( "t", "%s\n", note );

   return res;
}


/***********************************************************************
 * GetConfig()  processes command file using kom.c functions           *
 *                      exits if any errors are encountered            *
 ***********************************************************************/

void GetConfig(char *configfile)
{
   int  ncommand;     /* # of required commands you expect to process   */
   char init[10];     /* init flags, one byte for each required command */
   int  nmiss;        /* number of required commands that were missed   */
   char *com;
   char *str;
   char blank = ' ';
   int  nfiles;
   int  success;
   int  i;

/* Set to zero one init flag for each required command
 *****************************************************/
   ncommand = 8;
   for ( i = 0; i < ncommand; i++ )
      init[i] = 0;

/* Open the main configuration file
   ********************************/
   nfiles = k_open( configfile );
   if ( nfiles == 0 )
   {
      logit( "e", "wsv_test: Error opening command file <%s>. Exiting.\n",
               configfile );
      exit( -1 );
   }

/* Process all command files
   *************************/
   while ( nfiles > 0 )   /* While there are command files open */
   {
      while ( k_rd() )        /* Read next line from active file  */
      {
         com = k_str();         /* Get the first token from line */

/* Ignore blank lines & comments
   *****************************/
         if ( !com )          continue;
         if ( com[0] == '#' ) continue;

/* Open a nested configuration file
   ********************************/
         if ( com[0] == '@' )
         {
            success = nfiles+1;
            nfiles  = k_open(&com[1]);
            if ( nfiles != success )
            {
               logit( "e", "wsv_test: Error opening command file <%s>. Exiting.\n",
                        &com[1] );
               exit( -1 );
            }
            continue;
         }

/* Read module id for this program
   *******************************/
/*0*/    if ( k_its( "MyModuleId" ) )
         {
             str = k_str();
             if ( str )
             {
                if ( GetModId( str, &MyModId ) < 0 )
                {
                   logit( "e", "wsv_test: Invalid MyModuleId <%s> in <%s>. Exiting.\n",
                            str, configfile );
                   exit( -1 );
                }
             }
             init[0] = 1;
         }

/* Name of transport ring to write to
   **********************************/
/*1*/    else if( k_its( "RingName" ) )
         {
             str = k_str();
             if ( str )
             {
                if ( ( RingKey = GetKey(str) ) == -1 )
                {
                   logit( "e", "wsv_test: Invalid RingName <%s> in <%s>. Exiting.\n",
                            str, configfile );
                   exit( -1 );
                }
             }
             init[1] = 1;
         }

/* Set Logfile switch
   ******************/
/*2*/    else if( k_its( "LogFile" ) )
         {
             LogFile = k_int();
             init[2] = 1;
         }

/* Set heartbeat interval
   **********************/
/*3*/    else if( k_its( "HeartBeatInterval" ) )
         {
             HeartBeatInterval = k_int();
             init[3] = 1;
         }

/* Set waveserver test interval
   ****************************/
/*4*/    else if( k_its( "TestInterval" ) )
         {
             TestInterval = k_int();
             init[4] = 1;
         }

/* Waveserver time out in msec
   ***************************/
/*5*/    else if( k_its( "wsTimeout" ) )
         {
             wsTimeout = k_int();
             init[5] = 1;
         }

/*6*/    else if ( k_its("Waveserver") )
         {
            if ( nWsv+1 > MAX_WSV )
            {
               logit( "e", "wsv_test: Too many <Waveserver> commands in <%s>", configfile );
               logit( "e", "; max=%d. Exiting.\n", (int) MAX_WSV );
               exit( -1 );
            }
            str = k_str();
            if ( str  &&  strlen(str) < MAX_ADRLEN )
               strcpy( (char *)Wsv[nWsv].ip, str );
            else  
            {   
               if( !str ) str = &blank;
               logit( "e", "wsv_test: Bad <Waveserver> IP address: %s. Exiting.\n", str );
               exit( -1 );
            }
            str = k_str();
            if ( str  &&  strlen(str) < MAX_ADRLEN )
               strcpy( (char *)Wsv[nWsv].port, str );
            else  
            {   
               if( !str ) str = &blank;
               logit( "e", "wsv_test: Bad <Waveserver> port number: %s. Exiting.\n", str );
               exit( -1 );
            }
            str = k_str();
            if ( str  &&  strlen(str) < MAX_ADRLEN )
               strcpy( (char *)Wsv[nWsv].host, str );
            else  
            {   
               if( !str ) str = &blank;
               logit( "e", "wsv_test: Bad <Waveserver> hostname: %s. Exiting.\n", str );
               exit( -1 );
            }
            str = k_str();
            if ( str  &&  strlen(str) < MAX_ADRLEN )
               strcpy( (char *)Wsv[nWsv].note, str );
            else  
            {   
               if( !str ) str = &blank;
               logit( "e", "wsv_test: Bad <Waveserver> note: %s. Exiting.\n", str );
               exit( -1 );
            }
            nWsv++;
            init[6] = 1;
         }

/* Switch for controlling logging of connections.
   If LogAllTests is 0, tests are logged only when there
   is a change of status, ie a waveserver stops responding
   or starts responding.
   *******************************************************/
/*7*/    else if( k_its( "LogAllTests" ) )
         {
             LogAllTests = k_int();
             init[7] = 1;
         }

         else
         {
             logit( "e", "wsv_test: <%s> unknown command in <%s>.\n",
                     com, configfile );
             continue;
         }

        /* See if there were any errors processing the command
         *****************************************************/
         if ( k_err() )
         {
            logit( "e", "wsv_test: Bad <%s> command in <%s>; \n",
                     com, configfile );
            exit( -1 );
         }
      }
      nfiles = k_close();
   }

/* After all files are closed, check init flags for missed commands
 ******************************************************************/
   nmiss = 0;
   for ( i = 0; i < ncommand; i++ )
      if ( !init[i] ) nmiss++;

   if ( nmiss )
   {
       logit( "e", "wsv_test: ERROR, no " );
       if ( !init[0] ) logit( "e", "<MyModuleId> " );
       if ( !init[1] ) logit( "e", "<RingName> "   );
       if ( !init[2] ) logit( "e", "<LogFile> "    );
       if ( !init[3] ) logit( "e", "<HeartBeatInterval> " );
       if ( !init[4] ) logit( "e", "<TestInterval> " );
       if ( !init[5] ) logit( "e", "<wsTimeout> " );
       if ( !init[6] ) logit( "e", "<Waveserver> " );
       if ( !init[7] ) logit( "e", "<LogAllTests> " );
       logit( "e", "command(s) in <%s>. Exiting.\n", configfile );
       exit( -1 );
   }
   return;
}


/***********************************************************************
 * LogConfig()  Log the configuration file parameters.                 *
 ***********************************************************************/

void LogConfig( void )
{
   int i;

   logit( "", "MyModId:           %u\n", MyModId );
   logit( "", "RingKey:           %d\n", RingKey );
   logit( "", "HeartBeatInterval: %d\n", HeartBeatInterval );
   logit( "", "TestInterval:      %d\n", TestInterval );
   logit( "", "wsTimeout:         %d\n", wsTimeout );
   logit( "", "LogFile:           %d\n", LogFile );
   logit( "", "LogAllTests:       %d\n", LogAllTests );

   logit( "", "\nThese waveservers will be tested:\n" );

   for ( i = 0; i < nWsv; i++ )
      logit( "", "%s  %s  %s %s\n", Wsv[i].ip, Wsv[i].port, 
             Wsv[i].host, Wsv[i].note );

   logit( "", "\n" );
   return;
}


/*********************************************************************
 * GetIp()  Get list of unique IP addresses of computers monitored.  *
 *********************************************************************/

void GetIp( WSV Wsv[], int nWsv, IP Ip[], int *nIp )
{
   int n = 0;                      /* Number of computers */
   int i;

   for ( i = 0; i < nWsv; i++ )
   {
      int j;
      int gotit = FALSE;

      for ( j = 0; j < n; j++ )
         if ( strcmp(Wsv[i].ip, Ip[j].ip) == 0 ) gotit = TRUE;

      if ( !gotit ) {
         strcpy( Ip[n].ip,   Wsv[i].ip   );
         strcpy( Ip[n].host, Wsv[i].host );
         n++;
      }
   }

   *nIp = n;                      /* Return number of computers */
}

