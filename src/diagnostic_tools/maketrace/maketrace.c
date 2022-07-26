#define _ISOC99_SOURCE
#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED 1        /* For Solaris */

/**********************************************************************
 *                             makeTrace.c                            *
 *                                                                    *
 *                       For data load testing.                       *
 **********************************************************************/

/*                                     Story

   This is derived from adsend. It's purpose is to execute a test ordered by
   Ray Buland: To have one machine producing trace_buf's, and broadcasting
   them onto a dedicated wire. To have two machines listening: one running
   WaveServer, the other an export. To then crank up the rate of trace_buf's
   until something breaks. Thus I was directed to do. Alex 11/12/02
*/

/*
   V2 is pretty much a complete rewrite.  The Windows specific code has
   been removed.  Support was added for TRACEBUF2 SCNLS and heartbeats.
   Only the location code part of the SCNL is sequenced.  The user can
   specify a SCNL template and the start time.  Data rate statistics can
   be disabled or the update interval changed by the user.  The code is
   now so fast that statmgr cannot keep up.  L. Baker 12/21/2018
*/

#include <ctype.h>              /* isdigit(), isupper() */
#include <inttypes.h>           /* uint32_t */
#include <math.h>               /* lround()                      */
                                /* Linux requires _ISOC99_SOURCE */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>             /* size_t */
#include <string.h>
#include <time.h>               /* time_t, strftime(), strptime() */
                                /* Linux requires _XOPEN_SOURCE   */
                                /* Windows has no strptime()      */

#include "earthworm.h"
#include "kom.h"
#include "transport.h"
#include "trace_buf.h"
#include "time_ew.h"            /* gmtime_ew(), hrtime_ew(), timegm_ew() */

#define MAKETRACE_VERSION "2.1 2019-01-04"

/* https://www.fdsn.org/webservices/FDSN-WS-Specifications-1.1.pdf */
#define FDSNTimeFormat "%Y-%m-%dT%H:%M:%S"

/* This is a TRACE2_HEADER SCNL.  The fields are longer than required */
/* by SEED.  You can specify the full TRACE2_HEADER SCNL in the       */
/* optional SCNL parameter.  See trace_buf.h.                         */
typedef struct {
   char sta[TRACE2_STA_LEN];    /* Site name (NUL-terminated) */
   char net[TRACE2_NET_LEN];    /* Network name (NUL-terminated) */
   char chan[TRACE2_CHAN_LEN];  /* Component/channel code (NUL-terminated)*/
   char loc[TRACE2_LOC_LEN];    /* Location code (NUL-terminated) */
} SCNL;

/* Functions prototypes
***********************/
static int  makeTrace_config( const char * );
static void Heartbeat( void );
static int  incSCNL( SCNL * );

/* Global parameters
********************/
static char *Argv0;             /* Pointer to executable name */

/* Configuration parameters - Required
**************************************/
static unsigned char ModuleId;  /* Data source ID placed in the trace header */
static int      HeartbeatInt;   /* Heartbeat interval, in seconds */
static double   ChanRate;       /* Rate in samples per second per channel */
static int      ChanMsgSize;    /* Message size in samples per channel */
static long     OutKey;         /* Key to ring where traces will live */
static int      nChan;          /* Number of channels to send */

/* The SCNL location code is incremented for each channel.  The default SCNL   */
/* template is <STA.CHA.NN.00>.  The channels will have location codes 00, 01, */
/* etc., up to 99.  By defaut, nChan cannot be more than 100.  You can supply  */
/* your own SCNL template.  If the last character is a digit, the location     */
/* codes will be digits.  If the last character is upper case alpha, the       */
/* location codes will be upper case alpha.  The sequence will "carry" into    */
/* the next character, as long as the template for that character is from the  */
/* same character set.  If you specify AA for the template SCNL location code, */
/* nChan can be up to 676 (26^2).  Use -- for the blank location code.  For    */
/* that case, nChan must be 1.                                                 */

/* Configuration parameters - Optional
**************************************/
static SCNL     seedName;       /* SCNL template, default=<STA.CHA.NN.00> */
static time_t   StartTime;      /* Header start time, time_t */
static double   Interval=10.;   /* Seconds between data rate statistics */
                                /* output, 0.=none                      */
static int      Debug=0;        /* >0 for debugging output, >1 for more */

/* Global variables
*******************/
static SHM_INFO OutRegion;      /* Info structure for output region */
static pid_t    MyPid;          /* Process id, sent with heartbeat */


int main( int argc, char *argv[] ) {

   int      ret;                /* maketrace return code */
   double   packetDt;
   unsigned int traceBufSize;   /* Size of the trace buffer, in bytes */
   char    *traceBuf=NULL;      /* Where the trace message is assembled */
   TRACE2_HEADER *traceHead;    /* Where the trace header is stored */
   int32_t *traceDat;           /* Where the data points are stored in the trace msg */
   TRACE2_HEADER *chanHead=NULL;/* Array with preloaded channel trace headers */
   unsigned char InstId;        /* Installation id placed in the trace header */
   MSG_LOGO logo;               /* Logo of message to put out */
   struct tm tm;
   char     startTimeBuf[]="YYYY-MM-DDTHH:MM:SS";
   double   startTime, endTime; /* Header start and end time, (EW) double */
   double   t0, tnow, twait;
   double   tnextSend;
   double   tShowNext;
   size_t   nSweep;             /* Current sweep through the SCNLs */
   int      rc;                 /* tport_putmsg() return code */
   int      printHeader;        /* Print data rate header when =0 */
   double   packetsPerSec;      /* Data rate statistics */
   double   samplesPerSec;
   double   bytesPerSec, KBytesPerSec;
   size_t   totalBytes, totalKBytes;
   double   totalMBytes;
   int      i, j;               /* Loop indexes */

   /* Check command line arguments
   ******************************/
   Argv0 = argv[0];
   if ( argc != 2 ) {
      fprintf( stderr, "Usage: %s <configfile>\n", Argv0 );
      fprintf( stderr, "Version %s\n", MAKETRACE_VERSION );
      return 0;
   }

   /* Initialize name of log file & open it
   ****************************************/
   logit_init( argv[1], 0, 512, 1 );
   logit( "et", "%s: Version %s\n", Argv0, MAKETRACE_VERSION);

   /* Assume failure
   *****************/
   ret = -1;

   /* Read the configuration file(s)
   *********************************/
   hrtime_ew( &t0 );            /* Start time */
   StartTime = (time_t) t0;     /* Start time of first sweep */
   strcpy( seedName.sta,  "STA" );
   strcpy( seedName.chan, "CHA" );
   strcpy( seedName.net,  "NN"  );
   strcpy( seedName.loc,  "00"  );
   if ( makeTrace_config( argv[1] ) < 0 ) {
      logit( "e", "%s: Error reading configuration file. Exiting.\n", Argv0 );
      goto quit;
   }

   logit( "e", "%s: ChanRate:                 %f samples/sec\n", Argv0, ChanRate );
   logit( "e", "%s: ChanMsgSize:              %d samples\n", Argv0, ChanMsgSize );
   logit( "e", "%s: nChan:                    %d\n", Argv0, nChan );
   packetDt = ChanMsgSize / ChanRate;
   logit( "e", "%s: Inter packet time:        %f sec\n", Argv0, packetDt );
   logit( "e", "%s: SCNL:                     <%s.%s.%s.%s>\n",
               Argv0, seedName.sta, seedName.chan, seedName.net, seedName.loc );

   /* Allocate some array space
   ****************************/
   traceBufSize = sizeof( *traceHead ) + ( ChanMsgSize * sizeof( *traceDat ) );
   if ( traceBufSize > MAX_TRACEBUF_SIZ ) {
      logit( "e", "%s: Trace Buffer too large: %d\n", Argv0, traceBufSize );
      logit( "e", "%s: Max legal size: %d\n", Argv0, MAX_TRACEBUF_SIZ );
      goto quit;
   }
   logit( "e", "%s: Trace buffer size:        %d bytes\n", Argv0, traceBufSize );
   logit( "e", "%s: Trace buffer header size: %zu bytes\n", Argv0, sizeof( TRACE2_HEADER ) );
   traceBuf = malloc( traceBufSize );
   if ( traceBuf == NULL ) {
      logit( "e", "%s: Cannot allocate the trace buffer\n", Argv0 );
      goto quit;
   }
   traceHead = (TRACE2_HEADER *) &traceBuf[0];
   traceDat  = (int32_t *) &traceBuf[sizeof( TRACE2_HEADER )];

   chanHead = calloc( nChan, sizeof( *chanHead ) );
   if ( chanHead == NULL ) {
      logit( "e", "%s: Cannot allocate the channel trace headers\n", Argv0 );
      goto quit;
   }

   /* Preload the invariant parts of the trace buffer headers
   **********************************************************/
   traceHead->nsamp      = ChanMsgSize;           /* Number of samples in message */
   traceHead->samprate   = ChanRate;              /* Sample rate; nominal */
   traceHead->version[0] = TRACE2_VERSION0;       /* Header version number */
   traceHead->version[1] = TRACE2_VERSION1;       /* Header version number */
   traceHead->quality[0] = 0;                     /* One bit per condition */
   traceHead->quality[1] = 0;                     /* One bit per condition */

#ifdef _SPARC
   strcpy( traceHead->datatype, "s4" );           /* Data format code */
#else /* assume _INTEL */
   strcpy( traceHead->datatype, "i4" );           /* Data format code */
#endif

   /* Preload the channel trace headers and add their SCNLs
   ********************************************************/
   for ( i = 0; i < nChan; i++ ) {
      if ( i > 0 ) {
         if ( incSCNL( &seedName ) < 0 ) {
            logit( "e", "%s: incSCNL( <%s.%s.%s.%s> ) failed\n", Argv0,
                        seedName.sta, seedName.chan, seedName.net, seedName.loc );
            goto quit;
         }
      }
      memcpy( &chanHead[i], traceHead, sizeof( *chanHead ) );
      strcpy( chanHead[i].sta,  seedName.sta  );
      strcpy( chanHead[i].chan, seedName.chan );
      strcpy( chanHead[i].net,  seedName.net  );
      strcpy( chanHead[i].loc,  seedName.loc  );
      chanHead[i].pinno = i;
      if ( Debug )
         logit( "e", "%s:                    [%3d]: <%s.%s.%s.%s>\n", Argv0,
                    chanHead[i].pinno,
                    chanHead[i].sta, chanHead[i].chan, chanHead[i].net, chanHead[i].loc);
   }

   /* Preload the invariant ChanMsgSize samples to traceBuf (0123456789...)
   ************************************************************************/
   for ( j = 0; j < ChanMsgSize; ++j )
      traceDat[j] = j % 10;

   /* Get our own PID for the heartbeat
   ************************************/
   MyPid = getpid();
   if( MyPid == -1 ) {
      logit( "e", "%s: Cannot get pid. Exiting.\n", Argv0 );
      goto quit;
   }

   /* Set up the logo of outgoing waveform messages
   ************************************************/
   if ( GetLocalInst( &InstId ) < 0 ) {
      logit( "e", "%s: Error getting the local installation id. Exiting.\n", Argv0 );
      goto quit;
   }
   logit( "e", "%s: Local InstId:        %8u\n", Argv0, InstId );

   logo.instid = InstId;
   logo.mod    = ModuleId;
   GetType( "TYPE_TRACEBUF2", &logo.type );

   /* Attach to existing transport ring and send first heartbeat
   *************************************************************/
   tport_attach( &OutRegion, OutKey );
   logit( "e", "%s: Attached to transport ring: %ld\n", Argv0, OutKey );
   Heartbeat();

/************************* The main program loop   *********************/

   tShowNext = t0 + Interval;   /* Interval for data rate display (0=no display) */
   printHeader = 0;             /* Print the data rate display header */

   /* Set initial time for the header time stamps
   **********************************************/
   startTime = (double) StartTime;/* Start time of first sweep */
   gmtime_ew( &StartTime, &tm );
   if ( strftime( startTimeBuf, sizeof( startTimeBuf ), FDSNTimeFormat, &tm ) == 0 ) {
      logit( "e", "%s: Error formatting startTime. Exiting.\n", Argv0 );
      goto quit;
   }
   logit( "e", "%s: startTime:                %lf (%s)\n",
               Argv0, startTime, startTimeBuf );

   nSweep = 0;                  /* Sweep no. through the SCNLs */

   while ( tport_getflag( &OutRegion ) != TERMINATE  &&
           tport_getflag( &OutRegion ) != MyPid         ) {

      ++nSweep;

      /* Sleep a bit if necessary
      ***************************/
      tnextSend = ( nSweep * packetDt ) + t0;
      hrtime_ew( &tnow );
      twait = tnextSend - tnow;
      if ( Debug > 1 )
         logit( "e", "%s: [%10zu]  now: %lf  wait: %lf\n", Argv0, nSweep, tnow, twait );
      if ( twait > 0 )
         sleep_ew( (unsigned int) lround( twait * 1000 ) );

      /* Beat the heart
      *****************/
      Heartbeat();

      /* Set the trace start and end times.
         Times are in seconds since midnight 1/1/1970.
         The first start time is set to t0, above,
         and incremented at the end of the channel loop.
      **************************************************/
      endTime = startTime + ( ( ChanMsgSize - 1 ) / ChanRate );

      /* Loop through the trace messages to be sent out
      *************************************************/
      for ( i = 0; i < nChan; i++ ) {

         /* Fill in the invariant parts of the trace buffer header
         *********************************************************/
         memcpy( traceHead, &chanHead[i], sizeof( *traceHead ) );

         /* Fill in the variable parts of the trace buffer header
         ********************************************************/
         traceHead->starttime = startTime;
         traceHead->endtime   = endTime;

         /* Send the message
         *******************/
         rc = tport_putmsg( &OutRegion, &logo, traceBufSize, traceBuf );
         if ( rc == PUT_TOOBIG ) {
           logit( "et", "%s: Trace message for channel %d too big\n", Argv0, i );
           goto quit;
         }
         if ( rc == PUT_NOTRACK ) {
            logit( "et", "%s: Tracking error while sending channel %d\n", Argv0, i );
            goto quit;
         }

      }
      startTime = startTime + ( ChanMsgSize / ChanRate );

      /* Show data rates if Interval > 0.
      ***********************************/
      if ( Interval > 0. ) {
         hrtime_ew( &tnow );
         if ( tnow >= tShowNext ) {
            if ( printHeader == 0 ) {
               logit( "e", "\nPackets/Sec  Samples/Sec  Hdr+Data/Sec  Total Hdr+Data\n"
                             "-----------  -----------  ------------  --------------\n" );
               printHeader = 20;
            }
            --printHeader;
            packetsPerSec = nSweep * nChan / ( tnow - t0 );
            samplesPerSec = ChanMsgSize * packetsPerSec;
            bytesPerSec   = ( sizeof( TRACE2_HEADER ) * packetsPerSec ) +
                            ( sizeof( *traceDat )     * samplesPerSec );
            KBytesPerSec  = ( bytesPerSec + 512 ) / 1024;
            totalBytes    = nSweep * nChan * traceBufSize;
            totalKBytes = ( totalBytes + 512 ) >> 10;
            totalMBytes = (double) totalKBytes / 1024.;
            logit( "e", "  %5.lf       %6.lf        %6.1lf KB      %6.1lf MB\n",
                        packetsPerSec, samplesPerSec, KBytesPerSec, totalMBytes );
            tShowNext += Interval;
         }
      }

   }

   /* Normal exit
   **************/
   logit( "et", "%s: Termination requested; exiting!\n", Argv0 );
   ret = 0;

   /* Clean up and exit
   ********************/
quit:
   free( traceBuf );
   free( chanHead );

   return ret;
}

/**********************************************************************
 *                             GetConfig()                            *
 *            Processes command file using kom.c functions.           *
 *              Returns -1 if any errors are encountered.             *
 *                                                                    *
 * Required parameters: ModuleId                                      *
 *                      OutRing                                       *
 *                      HeartbeatInt                                  *
 *                      ChanRate                                      *
 *                      ChanMsgSize                                   *
 *                      nChan                                         *
 * Optional parameters: SCNL                                          *
 *                      Interval                                      *
 *                      Debug                                         *
 *                                                                    *
 **********************************************************************/

#define NCOMMAND 6              /* Number of mandatory commands in the config file */

static int makeTrace_config( const char *configfile ) {

   char     init[NCOMMAND];     /* Flags, one for each required command */
   int      nmiss;              /* Number of commands that were missed */
   char    *com;
   int      nfiles;
   int      success;
   int      i;
   char    *str;

   /* Set to zero one init flag for each required command
   ******************************************************/
   for ( i = 0; i < NCOMMAND; i++ )
      init[i] = 0;

   /* Open the main configuration file
   ***********************************/
   nfiles = k_open( configfile );
   if ( nfiles == 0 ) {
      logit( "et", "%s: Error opening configuration file <%s>\n", Argv0, configfile );
      return -1;
   }

   /* Process all nested configuration files
   *****************************************/
   while ( nfiles > 0 ) {       /* While there are config files open */

      while ( k_rd() ) {        /* Read next line from active file  */

         com = k_str();         /* Get the first token from line */

         if ( !com ) continue;             /* Ignore blank lines */
         if ( com[0] == '#' ) continue;    /* Ignore comments */

   /* Open another configuration file
   **********************************/
         if ( com[0] == '@' ) {
            success = nfiles + 1;
            nfiles  = k_open( &com[1] );
            if ( nfiles != success ) {
               logit( "et", "%s: Error opening command file <%s>.\n", Argv0, &com[1] );
               return -1;
            }
            continue;
         }

   /* Required configuration parameters (update init[])
   ****************************************************/
         else if ( k_its( "ModuleId" ) ) {
            str = k_str();
            if ( str != NULL ) {
               if ( GetModId(str, &ModuleId) == -1 ) {
                  logit( "et", "%s: Invalid ModuleId <%s>. Exiting.\n", Argv0, str );
                  return -1;
               }
            }
            init[0] = 1;
         }
         else if ( k_its( "OutRing" ) ) {
            str = k_str();
            if ( str != NULL ) {
               if ( (OutKey = GetKey(str)) == -1 ) {
                  logit( "et", "%s: Invalid OutRing <%s>. Exiting.\n", Argv0, str );
                  return -1;
               }
            }
            init[1] = 1;
         }
         else if ( k_its( "HeartbeatInt" ) ) {
            HeartbeatInt = k_int();
            init[2] = 1;
         }
         else if ( k_its( "ChanRate" ) ) {
            ChanRate = k_val();
            init[3] = 1;
         }
         else if ( k_its( "ChanMsgSize" ) ) {
            ChanMsgSize = k_int();
            init[4] = 1;
         }
         else if ( k_its( "nChan" ) ) {
            nChan = k_int();
            init[5] = 1;
         }

   /* Optional configuration parameters (DO NOT update init[])
   ***********************************************************/
         else if ( k_its( "SCNL" ) ) {
            char *s, *c, *n, *l;
            str = "";
            s = k_str();
            if ( s == NULL ) s = str;
            c = k_str();
            if ( c == NULL ) c = str;
            n = k_str();
            if ( n == NULL ) n = str;
            l = k_str();
            if ( l == NULL ) l = str;
            if ( ( *s == '\0' ) || ( *c == '\0' ) || ( *n == '\0' ) || ( *l == '\0' ) ) {
               logit( "et", "%s: Missing SCNL component(s): <%s.%s.%s.%s>. Exiting.\n",
                            Argv0, s, c, n, l );
               return -1;
            }
            if ( ( strlen( s ) >= sizeof( seedName.sta  ) ) ||
                 ( strlen( c ) >= sizeof( seedName.chan ) ) ||
                 ( strlen( n ) >= sizeof( seedName.net  ) ) ||
                 ( strlen( l ) >= sizeof( seedName.loc  ) ) ) {
               logit( "et", "%s: SCNL component(s) are too long: <%s.%s.%s.%s>. Exiting.\n",
                            Argv0, s, c, n, l );
               return -1;
            }
            if ( strcmp( l, "--" ) == 0 ) l = "  ";
            strcpy( seedName.sta,  s );
            strcpy( seedName.chan, c );
            strcpy( seedName.net,  n );
            strcpy( seedName.loc,  l );
         }
         else if ( k_its( "StartTime" ) ) {
            str = k_str();
            if ( ( str == NULL ) || ( *str == '\0' ) ) {
               logit( "et", "%s: Missing StartTime. Exiting.\n", Argv0 );
               return -1;
            }
#ifdef _WINNT
            logit( "et", "%s: The StartTime parameter is ignored on Windows.\n", Argv0 );
#else
            {
               struct tm tm;
               char *p;
               p = strptime( str, FDSNTimeFormat, &tm );
               if ( p == NULL ) {
                  logit( "et", "%s: StartTime is not a legal time: \"%s\". Exiting.\n",
                               Argv0, str );
                  return -1;
               }
               if ( *p != '\0' ) {
                  logit( "et", "%s: StartTime is not FDSN format: \"%s\". Exiting.\n",
                               Argv0, str );
                  return -1;
               }
               StartTime = timegm_ew( &tm );
            }
#endif
         }
         else if ( k_its( "Interval" ) ) {
            Interval = k_val();
            if ( Interval < 0. )
               Interval = 0.;
         }
         else if ( k_its( "Debug" ) ) {
            Debug = k_int();
            if ( Debug < 0 )
               Debug = 0;
         }

   /* An unknown parameter was encountered
   ***************************************/
         else
         {
            logit( "et", "%s: <%s> unknown parameter in <%s>\n", Argv0, com, configfile );
            return -1;
         }

   /* See if there were any errors processing the command
   ******************************************************/
         if ( k_err() ) {
            logit( "et", "%s: Bad <%s> command in <%s>.\n", Argv0, com, configfile );
            return -1;
         }

         str = k_str();
         if ( ( str != NULL ) && ( *str != '#' ) ) {
            logit( "et", "%s: Extraneous characters in <%s> command. Exiting.\n", Argv0, com );
            return -1;
         }

      }
      nfiles = k_close();
   }

   /* After all files are closed, check flags for missed commands
   **************************************************************/
   nmiss = 0;
   for ( i = 0; i < NCOMMAND; i++ )
      if ( !init[i] )
         nmiss++;

   if ( nmiss > 0 ) {
      logit( "et", "%s: ERROR, no ", Argv0 );
      if ( !init[0]  ) logit( "", "<ModuleId> " );
      if ( !init[1]  ) logit( "", "<OutRing> " );
      if ( !init[2]  ) logit( "", "<HeartbeatInt> " );
      if ( !init[3]  ) logit( "", "<ChanRate> " );
      if ( !init[4]  ) logit( "", "<ChanMsgSize> " );
      if ( !init[5]  ) logit( "", "<nChan> " );
      logit( "et", "command(s) in <%s>.\n", configfile );
      return -1;
   }

   return 0;
}

/**********************************************************************
 *                             Heartbeat()                            *
 *            Send a heartbeat message to the output ring.            *
 **********************************************************************/

static void Heartbeat( void ) {

   long              msgLen;        /* Length of the heartbeat message */
   char              msg[40];       /* To hold the heartbeat message */
   static int        first = TRUE;  /* 1 the first time Heartbeat() is called */
   static time_t     time_prev;     /* When Heartbeat() was last called */
   time_t            time_now;      /* The current time */
   static MSG_LOGO   logo;          /* Logo of heartbeat messages */

   /* Initialize the heartbeat variables
   **********************************/
   if ( first ) {
      GetLocalInst( &logo.instid );
      logo.mod = ModuleId;
      GetType( "TYPE_HEARTBEAT", &logo.type );
      time_prev = 0;                /* Force heartbeat first time thru */
      first = FALSE;
   }

   /* Is it time to beat the heart?
   ********************************/
   time( &time_now );
   if ( (time_now - time_prev) < HeartbeatInt )
      return;
   time_prev = time_now;

   /* It's time to beat the heart
   ******************************/
   msgLen = sprintf( msg, "%ld %d\n", (long) time_now, (int) MyPid );

   if ( tport_putmsg( &OutRegion, &logo, msgLen, msg ) != PUT_OK )
      logit( "et", "%s: Error sending heartbeat.", Argv0 );

}

/**********************************************************************
 *                              incSCNL()                             *
 *                  Increment the SCNL location code.                 *
 **********************************************************************/

static int incSCNL( SCNL *name ) {

   static const char digits[] = "0123456789";
   static const char uppers[]  = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
   int   i, index;

   i = (int) strlen( name->loc ) - 1;
   /* Increment digits location code field: XXXXX.XXX.XX.++ */
   if ( isdigit( name->loc[i] ) ) {
      while ( ( i >= 0 ) && isdigit( name->loc[i] ) ) {
         index = (int) ( strchr( digits, name->loc[i] ) - digits );
         name->loc[i] = digits[( index + 1 ) % 10];
         if ( name->loc[i] != '0' )
            return 0;
         --i;
      }
   }
   /* Increment alpha location code field: XXXXX.XXX.XX.++ */
   else if ( isupper( name->loc[i] ) ) {
      while ( ( i >= 0 ) && isupper( name->loc[i] ) ) {
         index = (int) ( strchr( uppers, name->loc[i] ) - uppers );
         name->loc[i] = uppers[( index + 1 ) % 26];
         if ( name->loc[i] != 'A' )
            return 0;
         --i;
      }
   }

   /* Overflow: XXXXX.XXX.XX.** */
   for ( i = 0; i < (int) strlen( name->loc ); i++ )
      name->loc[i] = '*';
   logit( "e", "%s: Cannot generate more names\n", Argv0 );
   return -1;
}
