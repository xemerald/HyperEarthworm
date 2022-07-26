/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: gaplist.c 7247 2018-04-06 20:47:06Z baker $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.22  2007/03/13 14:54:59  paulf
 *     more time_t printings
 *
 *     Revision 1.21  2007/01/10 19:01:53  dietz
 *     Changed gap binning boundaries from 1.0 to 1.1 sec so that gaps from
 *     1 second timing errors will show up in a different bin from gaps caused
 *     by 1 missed packet (for most digital dataloggers). LDD
 *
 *     Revision 1.20  2005/04/11 17:56:32  dietz
 *       Added new optional command: GetLogo <instid> <modid> <msgtype>
 *     If no GetLogo cmds are given, gaplist will process all TYPE_TRACEBUF
 *     and TYPE_TRACEBUF2 messages (previous behavior).
 *       Modified to allow processing of TYPE_TRACE_COMP_UA and TYPE_TRACE2_COMP_UA
 *     messages (headers are NOT compressed!).
 *
 *     Revision 1.19  2004/05/19 00:03:20  kohler
 *     Fixed typo that was causing the program to crash when the day rolled over.
 *
 *     Revision 1.18  2004/05/17 18:01:34  kohler
 *     Fixed dangling-else error in two places.  WMK
 *
 *     Revision 1.17  2004/05/14 22:44:54  kohler
 *     Yet more defensive programming. WMK
 *
 *     Revision 1.16  2004/05/14 20:25:03  kohler
 *     Defensive programming to prevent buffer overflow.  WMK
 *
 *     Revision 1.15  2004/05/13 17:45:13  kohler
 *     Cosmetic changes.  WMK
 *
 *     Revision 1.14  2004/05/11 17:04:24  kohler
 *     Now, the scnl array is allocated using realloc, so any number of Scnl and
 *     Label lines are permitted in the config file.
 *
 *     Fixed two bugs:
 *     Label strings longer than 31 characters weren't null-terminated in the
 *     program.  Now, they are, so long labels are correctly truncated.
 *
 *     The label array, defined in gaplist.h, had a length of 31 characters,
 *     which caused some values in the scnl array to get clobbered.  This resulted
 *     in continuous dead/alive messages being sent to statmgr.
 *
 *     Revision 1.13  2004/05/07 19:10:50  dietz
 *     added system-clock timestamp to gap/overlap logging
 *
 *     Revision 1.12  2004/04/30 18:36:39  kohler
 *     gaplist now accepts both TYPE_TRACEBUF and TYPE_TRACEBUF2 messages.
 *     WMK 4/30/04
 *
 *     Revision 1.11  2004/04/21 19:00:56  kohler
 *     gaplist now looks for gaps in TYPE_TRACEBUF2 message, ie trace messages
 *     that contain location codes.  The program ignores TYPE_TRACEBUF messages.
 *     WMK 4/21/04
 *
 *     Revision 1.10  2004/01/20 22:40:03  kohler
 *     Changed table format slightly
 *
 *     Revision 1.9  2004/01/09 00:51:32  kohler
 *     Changed gap tables format
 *
 *     Revision 1.8  2002/05/15 16:45:14  patton
 *     Made logit changes.
 *
 *     Revision 1.7  2001/12/07 22:46:41  dietz
 *     added optional Label command
 *
 *     Revision 1.6  2001/05/09 17:23:51  dietz
 *     Changed to shut down gracefully if the transport flag is
 *     set to TERMINATE or myPid..
 *
 *     Revision 1.5  2001/04/19 19:15:41  kohler
 *     Boundaries of bins changed.  Two config parameters removed.
 *
 *     Revision 1.4  2000/11/20 19:23:30  kohler
 *     Program now waits ReportDeadChan minutes before reporting
 *     dead channels.  Two stars are then printed in the gaplist
 *     table.
 *
 *     Revision 1.3  2000/08/08 18:16:21  lucky
 *     Lint cleanup
 *
 *     Revision 1.2  2000/04/18 23:29:17  dietz
 *     1. module id is now configurable (defaults to MOD_GAPLIST)
 *     2. logs and sends msg to statmgr when it receives data from a
 *        previously dead channel.
 *
 *     Revision 1.1  2000/02/14 17:39:05  lucky
 *     Initial revision
 *
 *
 */

  /*****************************************************************
   *                            gaplist.c                          *
   *                                                               *
   *  Program to search for gaps in tracebuf messages and print    *
   *  tables.                                                      *
   *****************************************************************/

/* changes:
  Lombard: 11/19/98: V4.0 changes:
     0) no Y2k dates
     1) changed argument of logit_init to the config file name.
     2) process ID in heartbeat message
     3) flush input transport ring
     4) add `restartMe' to .desc file
     5) multi-threaded logit: not applicable

  Dietz: 11/23/98: added changes to chron3 function calls.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "earthworm.h"
#include "transport.h"
#include "trace_buf.h"
#include "chron3.h"
#include "kom.h"
#include "swap.h"
#include "trheadconv.h"
#include "gaplist.h"

/* Windows provides a non-ANSI version of snprintf,
   name _snprintf.  The Windows version has a
   different return value than ANSI snprintf.
   ***********************************************/
#ifdef _WINNT
#define snprintf _snprintf
#endif

/* Function prototypes
   *******************/
void GetConfig( char *, GPARM * );
void LogConfig( GPARM * );
void LogGapSize( int, double, double );
void IncrementGapCount( int, double );
void PrintTable( void );
void LogTable( void );
int  GetDay( time_t * );

/* Global variables
   ****************/
extern int           nScnl;         /* # of scnl's to monitor */
extern SCNL          *scnl;         /* Array of scnl parameters and labels */
static char          *TraceBuf;     /* The tracebuf buffer */
static TRACE_HEADER  *TraceHead;    /* The tracebuf header */
static TRACE2_HEADER *Trace2Head;   /* The tracebuf2 header */
static GPARM         Gparm;         /* Configuration file parameters */
static char          t_begin[26];   /* Begin time of table */
pid_t                myPid;         /* for restarts by startstop */
static SHM_INFO      region;        /* Shared memory region */
static MSG_LOGO      errlogo;       /* Logo of outgoing errors */

/* Error number definitions
   ************************/
#define ERR_CHAN_DEAD  0  /* no data from chan in TablePrintInterval sec */
#define ERR_CHAN_ALIVE 1  /* data received previously dead channel       */


         /*************************************************
          *        The main program starts here.          *
          *************************************************/

int main( int argc, char *argv[] )
{
   MSG_LOGO logo;
   MSG_LOGO hrtlogo;              /* Logo of outgoing heartbeats */
   long     gotsize;
   int      i;
   int      res;
   int      day;
   time_t   startTime;
   time_t   prevHeartTime;
   time_t   prevTableTime;
   unsigned char inst_wildcard;
   unsigned char inst_local;
   unsigned char type_tracebuf;
   unsigned char type_tracebuf2;
   unsigned char type_tracecomp;
   unsigned char type_trace2comp;
   unsigned char type_heartbeat;
   unsigned char type_error;
   unsigned char mod_wildcard;
   unsigned char mod_gaplist;
   unsigned char seq;

/* Check command line arguments
   ****************************/
   if ( argc != 2 )
   {
      printf( "Usage: gaplist <configfile>\n" );
      return -1;
   }

/* Open log file
   *************/
   logit_init( argv[1], 0, 256, 1 );

/* Read things from earthworm*d files
   **********************************/
   if ( GetInst( "INST_WILDCARD", &inst_wildcard ) != 0 )
   {
      logit( "et", "gaplist: Error getting INST_WILDCARD. Exiting.\n" );
      return -1;
   }
   if ( GetLocalInst( &inst_local ) != 0 )
   {
      logit( "et", "gaplist: Error getting MyInstId.\n" );
      return -1;
   }
   if ( GetType( "TYPE_TRACEBUF", &type_tracebuf ) != 0 )
   {
      logit( "et", "gaplist: Error getting <TYPE_TRACEBUF>. Exiting.\n" );
      return -1;
   }
   if ( GetType( "TYPE_TRACEBUF2", &type_tracebuf2 ) != 0 )
   {
      logit( "et", "gaplist: Error getting <TYPE_TRACEBUF2>. Exiting.\n" );
      return -1;
   }
   if ( GetType( "TYPE_TRACE_COMP_UA", &type_tracecomp ) != 0 )
   {
      logit( "et", "gaplist: Error getting <TYPE_TRACE_COMP_UA>. Exiting.\n" );
      return -1;
   }
   if ( GetType( "TYPE_TRACE2_COMP_UA", &type_trace2comp ) != 0 )
   {
      logit( "et", "gaplist: Error getting <TYPE_TRACE2_COMP_UA>. Exiting.\n" );
      return -1;
   }
   if ( GetType( "TYPE_HEARTBEAT", &type_heartbeat ) != 0 )
   {
      logit( "et", "gaplist: Error getting <TYPE_HEARTBEAT>. Exiting.\n" );
      return -1;
   }
   if ( GetType( "TYPE_ERROR", &type_error ) != 0 )
   {
      logit( "et", "gaplist: Error getting <TYPE_ERROR>. Exiting.\n" );
      return -1;
   }
   if ( GetModId( "MOD_WILDCARD", &mod_wildcard ) != 0 )
   {
      logit( "et", "gaplist: Error getting MOD_WILDCARD. Exiting.\n" );
      return -1;
   }

/* Get parameters from the configuration files.
   GetConfig() exits on error.
   *******************************************/
   GetConfig( argv[1], &Gparm );

/* Get the module id for this process.
   Beware dangling else statement. First set of {} is required.
   ***********************************************************/
   if ( strlen( Gparm.MyModName ) != 0 )     /* Use config file MyModuleId */
   {
      if ( GetModId( Gparm.MyModName, &mod_gaplist ) != 0 )
      {
         logit("", "gaplist: Error getting %s. Exiting.\n", Gparm.MyModName );
         free( Gparm.GetLogo );
         return -1;
      }
   } else {                                  /* Not configured, use default */
      if ( GetModId( "MOD_GAPLIST", &mod_gaplist ) != 0 )
      {
         logit("", "gaplist: Error getting MOD_GAPLIST. Exiting.\n" );
         free( Gparm.GetLogo );
         return -1;
      }
   }

/* Specify logos of incoming waveforms, outgoing heartbeats, errors
   ****************************************************************/
   if( Gparm.nGetLogo == 0 )
   {
      Gparm.nGetLogo = 2;
      Gparm.GetLogo  = (MSG_LOGO *) calloc( Gparm.nGetLogo, sizeof(MSG_LOGO) );
      if( Gparm.GetLogo == NULL ) {
         logit( "e", "gaplist: Error allocating space for GetLogo. Exiting\n" );
         return -1;
      }
      Gparm.GetLogo[0].instid = inst_wildcard;
      Gparm.GetLogo[0].mod    = mod_wildcard;
      Gparm.GetLogo[0].type   = type_tracebuf2;

      Gparm.GetLogo[1].instid = inst_wildcard;
      Gparm.GetLogo[1].mod    = mod_wildcard;
      Gparm.GetLogo[1].type   = type_tracebuf;
   }

   hrtlogo.instid = inst_local;
   hrtlogo.type   = type_heartbeat;
   hrtlogo.mod    = mod_gaplist;

   errlogo.instid = inst_local;
   errlogo.type   = type_error;
   errlogo.mod    = mod_gaplist;

/* Log the configuration parameters
   ********************************/
   LogConfig( &Gparm );

/* Get process ID for heartbeat messages
   *************************************/
   myPid = getpid();
   if ( myPid == -1 )
   {
     logit( "e","gaplist: Cannot get pid. Exiting.\n" );
     free( Gparm.GetLogo );
     return -1;
   }

/* Allocate the trace buffer
   *************************/
   TraceBuf = (char *) malloc( MAX_TRACEBUF_SIZ );
   if ( TraceBuf == NULL )
   {
      logit( "t", "Error allocating trace buffer.  Exiting.\n" );
      free( Gparm.GetLogo );
      return -1;
   }
   TraceHead  = (TRACE_HEADER *)TraceBuf;
   Trace2Head = (TRACE2_HEADER *)TraceBuf;

/* Attach to a transport ring
   **************************/
   tport_attach( &region, Gparm.InKey );

/* Flush transport ring on startup
   *******************************/
   while (tport_copyfrom( &region, Gparm.GetLogo, Gparm.nGetLogo, 
                          &logo, &gotsize,
                          TraceBuf, MAX_TRACEBUF_SIZ, &seq ) != GET_NONE);

/* Get the time when we start reading messages
   *******************************************/
   time( &startTime );
   prevHeartTime = startTime;
   prevTableTime = startTime;

/* Get the day of month.
   Table will be logged when the day rolls over.
   ********************************************/
   day = GetDay( &startTime );

/* Get the table start time as an ascii string,
   to be written to the log file
   *******************************************/
   strcpy( t_begin, asctime(gmtime(&startTime)) );

/* See if termination flag has been set
   ************************************/
   while ( 1 )
   {
      time_t now;              /* Current time */
      int    new_day;

      sleep_ew( 200 );

      if ( tport_getflag( &region ) == TERMINATE ||
           tport_getflag( &region ) == myPid )
      {
         tport_detach( &region );
         LogTable();
         logit( "t", "Termination flag detected. Program stopping.\n" );
         free( Gparm.GetLogo );
         return 0;
      }

/* Send a heartbeat to the transport ring
   **************************************/
      time( &now );
      if ( Gparm.HeartbeatInt > 0 )
      {
         if ( (now - prevHeartTime) >= Gparm.HeartbeatInt )
         {
            int  lineLen;
            char line[40];

            prevHeartTime = now;

            snprintf( line, sizeof(line), "%ld %ld\n", (long) now, (long) myPid );
            line[sizeof(line)-1] = 0;
            lineLen = strlen( line );

            if ( tport_putmsg( &region, &hrtlogo, lineLen, line ) !=
                 PUT_OK )
            {
               logit( "et", "gaplist: Error sending heartbeat. Exiting." );
               free( Gparm.GetLogo );
               return -1;
            }
         }
      }

/* Print a gap table on the screen
   *******************************/
      if ( (now - prevTableTime) >= Gparm.TablePrintInterval )
      {
         prevTableTime = now;
         PrintTable();
      }

/* If the day has changed, dump the count
   table and start refilling it
   **************************************/
      new_day = GetDay( &now );
      if ( new_day != day )
      {
         int k, j;

         LogTable();
         for ( k = 0; k < nScnl; k++ )
            if ( !scnl[k].isLabel )
            {
               for ( j = 0; j < NWINDOW; j++ )
                  scnl[k].count[j] = 0;
               scnl[k].totalGap = 0.0;
            }
         strcpy( t_begin, asctime(gmtime(&now)) );
         day = new_day;
      }

/* Get all available tracebuf and tracebuf2 messages
   *************************************************/
      while ( 1 )
      {
         double GapBegin;
         double GapEnd;
         double GapSize;
         int    p;            /* The scnl index number */

         res = tport_copyfrom( &region, Gparm.GetLogo, Gparm.nGetLogo, 
                               &logo, &gotsize,
                               TraceBuf, MAX_TRACEBUF_SIZ, &seq );

         if ( res == GET_NONE )
            break;

         if ( res == GET_TOOBIG )
         {
            logit( "et", "gaplist: Retrieved message is too big (%ld)\n", gotsize );
            break;
         }

         if ( res == GET_NOTRACK )
            logit( "et", "gaplist: NTRACK_GET exceeded.\n" );

         if ( res == GET_MISS_LAPPED )
            logit( "et", "gaplist: GET_MISS_LAPPED error.\n" );

         if ( res == GET_MISS_SEQGAP ) {}     /* Do nothing */

/* If necessary, swap bytes in tracebuf message.
   Beware dangling else statement. First set of {} is required.
   ***********************************************************/
         if ( logo.type == type_tracebuf ||
              logo.type == type_tracecomp   )
         {
            if ( WaveMsgMakeLocal( TraceHead ) < 0 )
            {
               logit( "et", "gaplist: WaveMsgMakeLocal() error.\n" );
               continue;
            }
         } else {
            if ( WaveMsg2MakeLocal( Trace2Head ) < 0 )
            {
               logit( "et", "gaplist: WaveMsg2MakeLocal() error.\n" );
               continue;
            }
         }

/* Convert TYPE_TRACEBUF messages to TYPE_TRACEBUF2
   Convert TYPE_TRACE_COMP_UA msgs to TYPE_TRACE2_COMP_UA
   ******************************************************/
         if ( logo.type == type_tracebuf || 
              logo.type == type_tracecomp   )
            Trace2Head = TrHeadConv( TraceHead );

/* Get the index number of this scnl in the scnl array
   ***************************************************/
         p = -1;

         for ( i = 0; i < nScnl; i++ )
            if ( !scnl[i].isLabel )
               if ( !strcmp(scnl[i].sta,  Trace2Head->sta)  &&
                    !strcmp(scnl[i].chan, Trace2Head->chan) &&
                    !strcmp(scnl[i].net,  Trace2Head->net)  &&
                    !strcmp(scnl[i].loc,  Trace2Head->loc) )
               {
                  p = i;
                  break;
               }
         if ( p == -1 )       /* We're not interested in this scnl */
            continue;

/* Compute the gap size
   ********************/
         GapBegin = scnl[p].EndTimePrev;
         GapEnd   = Trace2Head->starttime;
         GapSize  = GapEnd - GapBegin;

         scnl[p].EndTimePrev = Trace2Head->endtime;
         scnl[p].alive       = 1;                  /* Declare this scnl alive */

/* This is the first message for this scnl
   ***************************************/
         if ( GapBegin < 0.0  ) continue;

/* Don't log if the gap is only one sample (ie no gap)
   ***************************************************/
         {
            double sampleGap = GapSize * Trace2Head->samprate;
            if ( (sampleGap > 0.0) && (sampleGap < 1.5) ) continue;
         }

/* Log the gap/overlap size based on this message
   **********************************************/
         if ( GapSize >= Gparm.MinGapToLog  ||  GapSize < 0.0 )
            LogGapSize( p, GapBegin, GapSize );

/* Log to the count table at regular intervals
   *******************************************/
         IncrementGapCount( p, GapSize );
      }
   }
}


      /**********************************************************
       *                      LogGapSize()                      *
       **********************************************************/

void LogGapSize( int p, double GapBegin, double GapSize )
{
   struct Greg g;
   int    second;

/* Convert time format
   *******************/
   datime( GapBegin + MAGIC, &g );
   second  = (int) g.second;

/* Log the gap
   ***********/
   if( GapSize < 0.0 ) logit( "t", "overlap  " );
   else                logit( "t", "gap      " );
   logit( "", "%-5s %-3s %-2s %-2s\t%02d:%02d:%02d\t%7.4lf\t%10.2lf\n",
          Trace2Head->sta,
          Trace2Head->chan,
          Trace2Head->net,
          Trace2Head->loc,
          g.hour,
          g.minute,
          second,
          ((double)g.hour + (double)g.minute/60. + (double)second/3600.),
          GapSize );

   return;
}


      /**********************************************************
       *                   IncrementGapCount()                  *
       *             Increment the gap counter table            *
       **********************************************************/

void IncrementGapCount( int p, double GapSize )
{
   int i = 0;

/* Increment the gap counter
   *************************/
   if ( GapSize >   0.00 ) i++;
   if ( GapSize >   0.25 ) i++;
   if ( GapSize >   1.10 ) i++;
   if ( GapSize >   8.00 ) i++;
   if ( GapSize >  64.00 ) i++;

   ++scnl[p].count[i];

/* Add this gap size to the previous total gap for this channel
   ************************************************************/
   scnl[p].totalGap += GapSize;
   return;
}


      /**********************************************************
       *                      PrintTable()                      *
       *                 Print the count table.                 *
       **********************************************************/

void PrintTable( void )
{
   char   line[120];
   char   note[80];
   int    p;
   int    i;
   time_t now = time( 0 );       /* Get current time */

   printf( "\n" );
   printf( "                              Numbers of Dropouts\n" );
   printf( "\n" );
   printf( "    Begin time: UTC %s", t_begin );
   printf( "    End time:   UTC %s", asctime(gmtime(&now)) );
   printf( "                                                                       Total\n" );
   printf( "                  Dead Time                  Gap Length (sec)           gap\n"  );
   printf( "      SCNL        (dy:hr:mn)  le 0  0-.25 .25-1.1  1.1-8  8-64    >64  (sec)\n" );
   printf( "      ----        ----------  ----  -----  ------  -----  ----   ----  -----\n" );
   for ( p = 0; p < nScnl; p++ )
   {
      int new_status;                /* 0, 1, or -1 */

      if ( scnl[p].isLabel )
      {
         printf( "%s\n", scnl[p].label );
         continue;
      }

      printf( " %-5s", scnl[p].sta );
      printf( " %-3s", scnl[p].chan );
      printf( " %-2s", scnl[p].net );
      printf( " %-2s", scnl[p].loc );

/* Determine new status
   ********************/
      if ( scnl[p].alive )
         new_status = 1;
      else                                /* Channel is currently dead */
      {
         if ( scnl[p].status == 1 )       /* It just died */
         {
            new_status = 0;
            scnl[p].timeOfDeath = now;    /* Record time of death */
         }
         else                             /* It's been dead a while */
         {
            if ( (now - scnl[p].timeOfDeath) < (int)(60. * Gparm.ReportDeadChan + .5) )
               new_status = 0;
            else
               new_status = -1;
         }
      }

/* If channel is dead for a long time, send message to statmgr.
   Use snprintf() to avoid buffer overflow.
   See top of this file for note about Windows version of snprintf.
   ***************************************************************/
      if ( (scnl[p].status == 0) && (new_status == -1) )
      {
         snprintf( note, sizeof(note), "Channel %s.%s.%s.%s dead for %.1lf min",
                  scnl[p].sta, scnl[p].chan, scnl[p].net, scnl[p].loc,
                  Gparm.ReportDeadChan );
         note[sizeof(note)-1] = '\0';
         logit( "t", "gaplist: %s\n", note );

         snprintf( line, sizeof(line), "%ld %d %s\n", (long) now, ERR_CHAN_DEAD, note);
         line[sizeof(line)-1] = '\0';
         if ( tport_putmsg( &region, &errlogo, strlen(line), line ) != PUT_OK )
         {
            logit( "et", "gaplist: Error sending error to statmgr. Exiting." );
            return;
         }
      }

/* If channel just came alive, send message to statmgr.
   Use snprintf() to avoid buffer overflow.
   See top of this file for note about Windows version of snprintf.
   ***************************************************************/
      if ( (scnl[p].status == -1) && (new_status == 1) )
      {
         snprintf( note, sizeof(note), "Channel %s.%s.%s.%s alive.",
                  scnl[p].sta, scnl[p].chan, scnl[p].net, scnl[p].loc );
         note[sizeof(note)-1] = '\0';
         logit( "t", "gaplist: %s\n", note );

         snprintf( line, sizeof(line), "%ld %d %s\n", (long) now, ERR_CHAN_ALIVE, note);
         line[sizeof(line)-1] = '\0';
         if ( tport_putmsg( &region, &errlogo, strlen(line), line ) != PUT_OK )
         {
            logit( "et", "gaplist: Error sending error to statmgr. Exiting." );
            return;
         }
      }

/* Print how long the channel has been dead
   ****************************************/
      if ( new_status == 1 )              /* Channel is alive */
         printf( "           " );
      else                                /* Channel is dead */
      {
         int deadMn = (int)((now - scnl[p].timeOfDeath) / 60.0);
         int deadDy = deadMn / 1440;
         int deadHr;

         deadMn -= 1440 * deadDy;
         deadHr  = deadMn / 60;
         deadMn -= 60 * deadHr;
         printf( "  %3d:%02d:%02d", deadDy, deadHr, deadMn );
      }

/* Print gap counts and total gap
   ******************************/
      for ( i = 0; i < NWINDOW; i++ )
         printf( " %6d", scnl[p].count[i] );

      printf( " %6.0lf\n", scnl[p].totalGap );

      scnl[p].alive  = 0;
      scnl[p].status = new_status;
   }
   return;
}


      /**********************************************************
       *                       LogTable()                       *
       *            Log the count table once per day            *
       **********************************************************/

void LogTable( void )
{
   int    p;
   int    i;
   time_t now = time( 0 );        /* Get current time */

   logit( "", " ----------------------------------------------------------------------\n" );
   logit( "", "                            Numbers of Dropouts\n" );
   logit( "", "\n" );
   logit( "", "   Begin time: UTC %s", t_begin );
   logit( "", "   End time:   UTC %s", asctime(gmtime(&now)) );
   logit( "", "                                                            Total\n" );
   logit( "", "                                   Gap Length (sec)          gap\n"  );
   logit( "", "      SCNL         le 0  0-.25 .25-1.1  1.1-8  8-64    >64  (sec)\n" );
   logit( "", "      ----         ----  -----  ------  -----  ----   ----  -----\n" );

   for ( p = 0; p < nScnl; p++ )
   {
      if ( scnl[p].isLabel )
      {
         logit( "", "%s\n", scnl[p].label );
         continue;
      }

      logit( "", " %-5s", scnl[p].sta );
      logit( "", " %-3s", scnl[p].chan );
      logit( "", " %-2s", scnl[p].net );
      logit( "", " %-2s", scnl[p].loc );

      for ( i = 0; i < NWINDOW; i++ )
         logit( "", " %6d", scnl[p].count[i] );

      logit( "", " %6.0lf\n", scnl[p].totalGap );
   }
   logit( "", " ----------------------------------------------------------------------\n" );
   return;
}


      /**********************************************************
       *                        GetDay()                        *
       *              Get the current day of month              *
       **********************************************************/

int GetDay( time_t *ltime )
{
   struct tm *g;

   g = gmtime( ltime );
   return g->tm_mday;
}
