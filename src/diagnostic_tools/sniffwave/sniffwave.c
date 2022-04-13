/*
 *    Revision history:
 *     $Log$
 *     Revision 1.38  2010/03/25 16:57:00  paulf
 *     okay, made it only show SCNL matches if L is specified
 *
 *     Revision 1.37  2010/03/25 16:48:50  paulf
 *     now will search for SCN or SNCL again, this is version 2.4
 *
 *     Revision 1.36  2010/03/25 15:29:47  paulf
 *     added logit_init() back in for WaveMsg2MakeLocal() but set disk logging to 0
 *
 *     Revision 1.35  2010/03/25 14:27:29  paulf
 *     removed logit() calls, untested as yet
 *
 *     Revision 1.34  2010/03/19 19:50:48  scott
 *     Made reporting of WaveMsg2MakeLocal error more robust
 *
 *     Revision 1.33  2010/03/19 17:46:36  scott
 *     Added check of return from WaveMsg2MakeLocal
 *
 *     Revision 1.32  2010/03/15 19:45:16  paulf
 *     okay, made the new feature allow sniffwave ringname and optionally y or s for data flag
 *
 *     Revision 1.31  2010/03/15 19:14:36  paulf
 *     sniffwave fixed to echo usage message if operator mistakenly does sniffwave WAVE_RING y mode, it is intended only for just the ring name
 *
 *     Revision 1.30  2009/09/24 21:21:23  stefan
 *     flush on each output line
 *
 *     Revision 1.29  2009/06/18 18:43:38  stefan
 *     formatting change only
 *
 *     Revision 1.28  2009/06/16 13:18:33  paulf
 *     fixed a number of loose ends from the new features, namely when the filter turns up no packets over the timeout period
 *
 *     Revision 1.27  2009/06/15 19:21:19  paulf
 *     upped the version number
 *
 *     Revision 1.26  2009/06/15 18:56:18  paulf
 *     many changes from the EW Class #7, latency, gap checking, and statistics at end of run with time out
 *
 *     Revision 1.25  2009/06/12 17:40:28  paulf
 *     fixed for windows
 *
 *     Revision 1.24  2009/06/12 15:29:49  paulf
 *     minor change to fix verbosity setting reading, during EW Class #7
 *
 *     Revision 1.23  2009/06/10 02:36:44  quintiliani
 *     Minor bug fix: removed unuseful lines about latency computation
 *
 *     Revision 1.22  2009/06/09 15:22:19  quintiliani
 *     Added the computation of latency (seconds) of the current tracebuf message
 *
 *     Revision 1.21  2008/01/19 19:55:14  paulf
 *     added a new command line allowed for sniffwave, just ring name
 *
 *     Revision 1.20  2007/12/16 06:02:02  paulf
 *     fixed sniffwave to handle systems where long could be 8 bytes
 *
 *     Revision 1.19  2007/03/23 15:53:15  stefan
 *     Earthworm Class March 2007, added statistics flag to sniffwave, y, n, or s options are now handled, tport fix made
 *
 *     Revision 1.18  2007/02/07 00:35:16  stefan
 *     help text revision
 *
 *     Revision 1.17  2007/02/02 19:06:59  stefan
 *     removed module verbosity for non-local modules
 *
 *     Revision 1.16  2006/12/28 23:07:54  lombard
 *     Added version number, printed with usage.
 *     Changed to specifically look for TYPE_TRACEBUF2 and TYPE_TRACE2_COMP_UA
 *     packets if SCNL given on command line, or to look for TYPE_TRACEBUF and
 *     TYPE_TRACE_COMP_UA packets if SCN is given on command line. Thus you cannot
 *     sniff for both SCNL and SCN packets at once with this command.
 *
 *     Revision 1.15  2006/12/01 02:12:58  stefan
 *     Added "verbose" option to print module, installation and type names along
 *     with the numbers. Without this extra parameter it works as before.
 *
 *     Revision 1.14  2006/07/26 20:23:00  stefan
 *     additional significant digits for sample rates < 1hz
 *
 *     Revision 1.13  2006/03/27 16:57:27  davek
 *     Added version and quality info to the sniffwave output for each tracebuf.
 *
 *     Revision 1.12  2005/03/30 23:13:26  dietz
 *     Now printing seconds to 4 decimal places.
 *
 *     Revision 1.11  2004/04/13 22:57:32  dietz
 *     modified to work with TYPE_TRACEBUF2 and TYPE_TRACE2_COMP_UA msgs
 *
 *     Revision 1.10  2003/10/31 17:20:43  dietz
 *     Will now sniff both TYPE_TRACEBUF and TYPE_TRACE_COMP_UA msg headers.
 *     Will accept strings "wild", "WILD", and "*" as wildcards in SCN.
 *
 *     Revision 1.9  2002/10/24 22:11:32  dietz
 *     Added message length to printed information
 *
 *     Revision 1.8  2002/07/12 17:49:57  dietz
 *     added #include <time_ew.h>
 *
 *     Revision 1.7  2002/02/02 00:28:58  dietz *     Changed to print instid and module on the tracebuf header summary line
 *
 *     Revision 1.6  2001/03/30 01:17:36  dietz
 *     *** empty log message ***
 *
 *     Revision 1.5  2001/03/30 01:10:18  dietz
 *     added newline after every 10 data samples, and an fflush(stdout)
 *     after each tracebuf message.
 *
 *     Revision 1.4  2001/03/29 23:51:03  dietz
 *     Changed ring flush statemnet to while( tport_get... != GET_NONE );
 *
 *     Revision 1.3  2000/08/08 18:18:28  lucky
 *     Lint cleanup
 *
 *     Revision 1.2  2000/05/31 18:29:10  dietz
 *     Fixed to print 4-byte data properly.
 *
 *     Revision 1.1  2000/02/14 19:36:09  lucky
 *     Initial revision
 */

  /*****************************************************************
   *                            sniffwave.c                        *
   *                                                               *
   * Program to read waveform messages from shared memory and      *
   * write to a disk file.                                         *
   *                                                               *
   * Modified by Lucky Vidmar Tue May 11 11:27:35 MDT 1999         *
   *   allows for more arguments which specify                     *
   *     Ring to read from                                         *
   *     SCN of the messages to print                              *
   *     Whether or not trace data should be printed               *
   *                                                               *
   *                                                               *
   * Usage: sniffwave  <Ring> <Sta> <Comp> <Net> <Loc> <Data y/n>  *
   *    or: sniffwave  <Ring> <Sta> <Comp> <Net> <Data y/n>        *
   *    or: sniffwave  <Ring>                                      *
   *                                                               *
   *****************************************************************/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "earthworm.h"
#include "transport.h"
#include "swap.h"
#include "time_ew.h"
#include "trace_buf.h"
#include "libmseed.h"

#define NLOGO 5
#define VERSION "2018-03-15 2.5.7"

#define TYPE_NOTUSED 254

int IsWild( char *str )
{
  if( (strcmp(str,"wild")==0)  ) return 1;
  if( (strcmp(str,"WILD")==0)  ) return 1;
  if( (strcmp(str,"*")   ==0)  ) return 1;
  return 0;
}

/************************************************************************************/
/* Definition for checking SCNL gaps and latencies                                  */
/************************************************************************************/

/* Max length for a SCNL string */
#define MAX_LEN_STR_SCNL 30

/* Structure for storing channel information, last scnl time and last packet time */
typedef struct {
    char scnl[MAX_LEN_STR_SCNL];
    double last_scnl_time; /* end time of last packet */
    double last_scnl_starttime; /* start time of last packet */
    double last_packet_time;
} SCNL_LAST_SCNL_PACKET_TIME;
/* Max number of channels to check */
#define MAX_NUM_SCNL 6000

/* Cast quality fields to unsigned char when you want convert to hexadecimal. */
#define CAST_QUALITY( q ) ( (unsigned char) q )

/* Return index from array scnl_time for SCNL (sta, cha, net, loc).
 * If the SCNL is not present yet, it is inserted. */
int search_scnl_time(SCNL_LAST_SCNL_PACKET_TIME *scnl_time, int *n_scnl_time, char *sta, char *cha, char *net, char *loc);
/* Check if there is a gap from the last packet and the current one in trh.
 * A message is printed to stdout if a gap or an overlap occurs.  */
int  check_for_gap_overlap_and_output_message(TRACE2_HEADER  *trh, SCNL_LAST_SCNL_PACKET_TIME *scnl_time, int *n_scnl_time);
/* Print to stdout latencies for
 *      Data (now - last_scnl_time)
 *      Feed (now - last_packet_time) */
void  print_data_feed_latencies(TRACE2_HEADER  *trh, SCNL_LAST_SCNL_PACKET_TIME *scnl_time, int *n_scnl_time);
/* */
static void end_process_signal( int );
static void handle_signal( void );

/************************************************************************************/
SHM_INFO Region;

int main( int argc, char **argv )
{
   long            RingKey;         /* Key to the transport ring to read from */
   MSG_LOGO        getlogo[NLOGO], logo;
   long            gotsize;
   char            msg[MAX_TRACEBUF_SIZ];
   char           *getSta, *getComp, *getNet, *getLoc, *inRing;
   char            wildSta, wildComp, wildNet, wildLoc;
   unsigned char   Type_Mseed;
   unsigned char   Type_TraceBuf,Type_TraceBuf2;
   unsigned char   Type_TraceComp, Type_TraceComp2;
   unsigned char   InstWildcard, ModWildcard;
   short          *short_data;
   int            *long_data;
   TRACE2_HEADER  *trh;
   char            orig_datatype[3];
   char            stime[256];
   char            etime[256];
   int             dataflag;
   int             i;
   int             rc;
   int             verbose = 0; /* if verbose =1 print name and number for logo ids */
   int             nLogo = NLOGO;
   static unsigned char InstId;        /* local installation id              */
   char            ModName[MAX_MOD_STR];
   char		   *modid_name;
   unsigned char   sequence_number = 0;
   int             statistics_flag;
   time_t monitor_start_time = 0;
   double start_time, end_time;
   int seconds_to_live;
   unsigned long packet_total=0;
   unsigned long packet_total_size=0;
   SCNL_LAST_SCNL_PACKET_TIME scnl_time[MAX_NUM_SCNL]; /* times of last scnl and packet */
   int n_scnl_time = 0;                                  /* current number of scnl_time items */
   MSRecord *msr = NULL;	/* mseed record */

	handle_signal();
   seconds_to_live = 0;
   dataflag = 0;
   statistics_flag = 0;

  /* Initialize pointers
  **********************/
  trh  = (TRACE2_HEADER *) msg;
  long_data  =  (int *)( msg + sizeof(TRACE2_HEADER) );
  short_data = (short *)( msg + sizeof(TRACE2_HEADER) );

  /* check the verbosity level before ANY other tests on the command line */
  if (argc > 2 && (strcmp(argv[argc-1], "verbose") == 0 ||
                        strcmp(argv[argc-1], "v") == 0)) {
    argc--;
    verbose = 1;
  }

  /* check if help parameter given */
  if(argc >= 2)
  {
    i = 0;    /* skip any 1 or 2 leading switch characters */
    if(argv[1][i] == '-' || argv[1][i] == '/')
      ++i;
    if(argv[1][i] == '-' || argv[1][i] == '/')
      ++i;
    if(strcmp(&argv[1][i], "help") == 0 || strcmp(&argv[1][i], "h") == 0)
    {  /* parameter is "help" or "h" */
      argc = 1;         /* set arg count to make help screen show below */
    }
  }

  /* check if last parameter is y/n/s or time value */
  if(argc > 2)
  {
    if(strcmp(argv[argc-1], "y") == 0)
    {
      dataflag = 1;
      argc--;
    }
    else if(strcmp(argv[argc-1], "n") == 0)
    {
      dataflag = 0;
      argc--;
    }
    else if(strcmp(argv[argc-1], "s") == 0)
    {
      statistics_flag = 1;
      argc--;
    }
    else if(argv[argc-1][0] != '0' && (i=atoi(argv[argc-1])) > 0)
    {  /* not leading zero (i.e., "00" location) and is positive value */
      seconds_to_live = i;
      argc--;
    }
    else if(argc == 7)
    {  /* seventh param should have been processed above */
      fprintf(stderr, "sniffwave: Bad parameter: %s\n", argv[argc-1]);
      argc = 1;         /* set arg count to make help screen show below */
    }
  }

  /* Check command line argument
  *****************************/
  if ( argc < 2 || argc > 6 )
  {
     if(argc > 6)
       fprintf(stderr, "sniffwave: Too many parameters\n");
     fprintf(stderr, "%s version %s\n", argv[0], VERSION);
     fprintf(stderr,"\nUsage: %s <ring name> [sta] [comp] [net] [loc] [y/n/s/time] [v]\n", argv[0]);
     fprintf(stderr, "\n Note: All parameters are positional, and all but first are optional.\n");
     fprintf(stderr, "\n Appending the optional 'v' or 'verbose' argument causes module,\n");
     fprintf(stderr, " installation and type names to be printed in addition to usual ID numbers.\n");
     fprintf(stderr, "\n The <y/n/s> flag is a data flag or 'time in seconds'. If 'y' is specified\n");
     fprintf(stderr, " the full data contained in the packet is printed out.\n");
     fprintf(stderr, " If flag set to s, provides max/min/avg statistics of the trace data.\n");
     fprintf(stderr, " If flag set to numeric value, program runs only for that number of seconds.\n");
     fprintf(stderr, "\n If sta comp net (but not loc) are specified then only TraceBuf\n");
     fprintf(stderr, " packets will be fetched (not TraceBuf2); otherwise both are fetched.\n");
     fprintf(stderr, "\n sniffwave output Data and Feed latencies in this form [D: secs F: secs]\n");
     fprintf(stderr, " [D] Data latency is the difference between current time and last sample time in current packet.\n");
     fprintf(stderr, " [F] Feed latency is the difference between current time and previous packet arrival time.\n");
     fprintf(stderr, "\n Example: %s WAVE_RING PHOB wild NC wild n\n", argv[0]);
     fprintf(stderr, " Example: %s WAVE_RING wild wild wild y verbose\n", argv[0]);
     fprintf(stderr, "\n MSEED capability starting with version 2.5.1, prints mseed headers\n");
     fprintf(stderr, " of TYPE_MSEED packets (no filtering yet).\n");
     return 1;
  }

  /* process given parameters */
  inRing  = argv[1];         /* first parameter is ring name */

  /* any parameters not given are set as wildcards */
  getSta = getComp = getNet = getLoc = "";
  wildSta = wildComp = wildNet = wildLoc = 1;
  if(argc > 2)
  {  /* at least station parameter given */
    getSta = argv[2];
    wildSta  = IsWild(getSta);
    if(argc > 3)
    {  /* channel (component) parameter given */
      getComp = argv[3];
      wildComp = IsWild(getComp);
      if(argc > 4)
      {  /* network parameter given */
        getNet = argv[4];
        wildNet = IsWild(getNet);
        if(argc > 5)
        {  /* location parameter given (SCNL) */
          getLoc  = argv[5];
          wildLoc = IsWild(getLoc);
          fprintf(stdout, "Sniffing %s for %s.%s.%s.%s\n",
                                   inRing, getSta, getComp, getNet, getLoc);
        }
        else
        {  /* SCN without location parameter given */
          nLogo = 3;    /* do not include tracebuf2s in search */
          fprintf(stdout, "Sniffing %s for %s.%s.%s (excluding TraceBuf2 packets)\n",
                                           inRing, getSta, getComp, getNet);
        }
      }
      else
      {  /* SC given */
        fprintf(stdout, "Sniffing %s for %s.%s.wild.wild\n",
                                                   inRing, getSta, getComp);
      }
    }
    else  /* station name given */
      fprintf(stdout, "Sniffing %s for %s.wild.wild.wild\n", inRing, getSta);
  }
  else  /* only ring name given */
    fprintf(stdout, "Sniffing %s for wild.wild.wild.wild\n", inRing);

  /* logit_init but do NOT WRITE to disk, this is needed for WaveMsg2MakeLocal() which logit()s */
  logit_init("sniffwave", 200, 200, 0);

  /* Attach to ring
  *****************/
  if ((RingKey = GetKey( inRing )) == -1 )
  {
    fprintf( stderr, "Invalid RingName; exiting!\n" );
    return -1;
  }
  tport_attach( &Region, RingKey );

/* Look up local installation id
   *****************************/
   if ( GetLocalInst( &InstId ) != 0 )
   {
      fprintf(stderr, "sniffwave: error getting local installation id; exiting!\n" );
      return -1;
   }

  /* Specify logos to get
  ***********************/
  if ( GetType( "TYPE_MSEED", &Type_Mseed ) != 0 ) {
     fprintf(stderr, "%s: WARNING: Invalid message type <TYPE_MSEED>! Missing from earthworm.d or earthworm_global.d\n", argv[0] );
     Type_Mseed = TYPE_NOTUSED;
  }
  if ( GetType( "TYPE_TRACEBUF", &Type_TraceBuf ) != 0 ) {
     fprintf(stderr, "%s: Invalid message type <TYPE_TRACEBUF>! Missing from earthworm.d or earthworm_global.d\n", argv[0] );
     return -1;
  }
  if ( GetType( "TYPE_TRACE_COMP_UA", &Type_TraceComp ) != 0 ) {
     fprintf(stderr, "%s: Invalid message type <TYPE_TRACE_COMP_UA>! Missing from earthworm.d or earthworm_global.d\n", argv[0] );
     return -1;
  }
  if ( GetType( "TYPE_TRACEBUF2", &Type_TraceBuf2 ) != 0 ) {
     fprintf(stderr, "%s: Invalid message type <TYPE_TRACEBUF2>! Missing from earthworm.d or earthworm_global.d\n", argv[0] );
     return -1;
  }
  if ( GetType( "TYPE_TRACE2_COMP_UA", &Type_TraceComp2 ) != 0 ) {
     fprintf(stderr,"%s: Invalid message type <TYPE_TRACE2_COMP_UA>! Missing from earthworm.d or earthworm_global.d\n", argv[0] );
     return -1;
  }
  if ( GetModId( "MOD_WILDCARD", &ModWildcard ) != 0 ) {
     fprintf(stderr, "%s: Invalid moduleid <MOD_WILDCARD>! Missing from earthworm.d or earthworm_global.d\n", argv[0] );
     return -1;
  }
  if ( GetInst( "INST_WILDCARD", &InstWildcard ) != 0 ) {
     fprintf(stderr, "%s: Invalid instid <INST_WILDCARD>! Missing from earthworm.d or earthworm_global.d\n", argv[0] );
     return -1;
  }

  for( i=0; i<nLogo; i++ ) {
      getlogo[i].instid = InstWildcard;
      getlogo[i].mod    = ModWildcard;
  }
  getlogo[0].type = Type_Mseed;
  getlogo[1].type = Type_TraceBuf;
  getlogo[2].type = Type_TraceComp;
  if (nLogo >= 4) {     /* if nLogo=5 then include TraceBuf2s */
      getlogo[3].type = Type_TraceBuf2;
      getlogo[4].type = Type_TraceComp2;
  }

  /* Flush the ring
  *****************/
  while ( tport_copyfrom( &Region, getlogo, nLogo, &logo, &gotsize,
            (char *)&msg, MAX_TRACEBUF_SIZ, &sequence_number ) != GET_NONE )
  {
         packet_total++;
         packet_total_size+=gotsize;
  }
  fprintf( stderr, "sniffwave: inRing flushed %ld packets of %ld bytes total.\n",
		packet_total, packet_total_size);

  if (seconds_to_live > 0) {
        monitor_start_time = time(0);
        packet_total = packet_total_size = 0;
        start_time = 0.0;
  }
  while ( (!(seconds_to_live > 0) ||
         (time(0)-monitor_start_time < seconds_to_live)) &&
         tport_getflag( &Region ) != TERMINATE )

  {

    rc = tport_copyfrom( &Region, getlogo, nLogo,
               &logo, &gotsize, msg, MAX_TRACEBUF_SIZ, &sequence_number );

    if ( rc == GET_NONE )
    {
      sleep_ew( 200 );
      continue;
    }

    if ( rc == GET_TOOBIG )
    {
      fprintf( stderr, "sniffwave: retrieved message too big (%ld) for msg\n",
        gotsize );
      continue;
    }

    if ( rc == GET_NOTRACK )
      fprintf( stderr, "sniffwave: Tracking error.\n");

    if ( rc == GET_MISS_LAPPED )
      fprintf( stderr, "sniffwave: Got lapped on the ring.\n");

    if ( rc == GET_MISS_SEQGAP )
      fprintf( stderr, "sniffwave: Gap in sequence numbers\n");

    if ( rc == GET_MISS )
      fprintf( stderr, "sniffwave: Missed messages\n");

    /* Check SCNL of the retrieved message */


    if (Type_Mseed != TYPE_NOTUSED && logo.type == Type_Mseed) {
      /* Unpack record header and not data samples */
      if ( msr_unpack (msg, gotsize, &msr, dataflag|statistics_flag, verbose) != MS_NOERROR) {
         fprintf (stderr, "Error parsing mseed record\n");
         continue;
      }

      /* Print record information */
      msr_print (msr, verbose);

      msr_free (&msr);
      continue;
    }

    if ( (wildSta  || (strcmp(getSta,trh->sta)  ==0)) &&
         (wildComp || (strcmp(getComp,trh->chan)==0)) &&
         (wildNet  || (strcmp(getNet,trh->net)  ==0)) &&
         (((logo.type == Type_TraceBuf2 ||
            logo.type == Type_TraceComp2) &&
         (wildLoc  || (strcmp(getLoc,trh->loc) == 0))) ||
         ( (logo.type == Type_TraceBuf ||
             logo.type == Type_TraceComp))))
    {
      strcpy(orig_datatype, trh->datatype);
      if(WaveMsg2MakeLocal( trh ) < 0)
      {
        char scnl[20], dt[3];
        scnl[0] = 0;
        strcat( scnl, trh->sta);
        strcat( scnl, ".");
        strcat( scnl, trh->chan);
        strcat( scnl, ".");
        strcat( scnl, trh->net);
        strcat( scnl, ".");
        strcat( scnl, trh->loc);
        /* now put a space if there are any punctuation marks */

        for ( i=0; i<15; i++ ) {
          if ( !isalnum(scnl[i]) && !ispunct(scnl[i]))
            scnl[i] = ' ';
        }
        strncpy( dt, trh->datatype, 2 );
        for ( i=0; i<2; i++ ) {
          if ( !isalnum(dt[i]) && !ispunct(dt[i]))
            dt[i] = ' ';
        }
        dt[i] = 0;
        fprintf(stderr, "WARNING: WaveMsg2MakeLocal rejected tracebuf.  Discard (%s).\n",
        	scnl );
        fprintf(stderr, "\tdatatype=[%s]\n", dt);
        continue;
      }


      if (seconds_to_live > 0) {
           if (start_time == 0.0)
                  start_time = trh->starttime;
           end_time = trh->endtime;
           packet_total++;
           packet_total_size += gotsize;
      }

      /* TODO add a flag to output gap message or not */
      /* Check for gap and in case output message */
      check_for_gap_overlap_and_output_message(trh, scnl_time, &n_scnl_time);

      datestr23 (trh->starttime, stime, 256);
      datestr23 (trh->endtime,   etime, 256);

      if( logo.type == Type_TraceBuf2  ||
          logo.type == Type_TraceComp2    ) {
          fprintf( stdout, "%5s.%s.%s.%s (0x%x 0x%x) ",
                   trh->sta, trh->chan, trh->net, trh->loc, trh->version[0], trh->version[1] );
      } else {
          fprintf( stdout, "%5s.%s.%s ",
                   trh->sta, trh->chan, trh->net );
      }

      fprintf( stdout, "%d %s %3d ",
                         trh->pinno, orig_datatype, trh->nsamp);
      if (trh->samprate < 1.0) { /* more decimal places for slower sample rates */
          fprintf( stdout, "%6.4f", trh->samprate);
      } else {
          fprintf( stdout, "%5.1f", trh->samprate);
      }
      fprintf( stdout, " %s (%.4f) %s (%.4f) ",
                         stime, trh->starttime,
                         etime, trh->endtime);
      switch (trh->version[1]) {
      case TRACE2_VERSION1:
         fprintf( stdout, "0x%02x 0x%02x ",
                         CAST_QUALITY(trh->quality[0]), CAST_QUALITY(trh->quality[1]) );
         break;
      case TRACE2_VERSION11:
         fprintf( stdout, "%6.4f ",
                         ((TRACE2X_HEADER *)trh)->x.v21.conversion_factor);
         break;
      }
      if (verbose) {

          if (InstId == logo.instid) {
              modid_name = GetModIdName(logo.mod);
              if (modid_name != NULL)
                  sprintf(ModName, "%s", modid_name);
              else
                  sprintf(ModName, "UnknownLocalMod");
          } else {
              sprintf( ModName, "UnknownRemoteMod");
          }

          fprintf( stdout, "i%d=%s m%d=%s t%d=%s len%4ld",
                 (int)logo.instid, GetInstName(logo.instid), (int)logo.mod,
                 ModName, (int)logo.type, GetTypeName(logo.type), gotsize );
      } else {
          fprintf( stdout, "i%d m%d t%d len%4ld",
                 (int)logo.instid, (int)logo.mod, (int)logo.type, gotsize );
      }

      /* Compute and print latency for data and packet */
      print_data_feed_latencies(trh, scnl_time, &n_scnl_time);

      fprintf( stdout, "\n");
      fflush (stdout);

      if (dataflag == 1)
      {
        if( logo.type == Type_TraceBuf2 ||
            logo.type == Type_TraceBuf     )
        {
          if ( (strcmp (trh->datatype, "s2")==0) ||
               (strcmp (trh->datatype, "i2")==0)    )
          {
            for ( i = 0; i < trh->nsamp; i++ ) {
              fprintf ( stdout, "%6hd ", *(short_data+i) );
              if(i%10==9) fprintf ( stdout, "\n" );
            }
          }
          else if ( (strcmp (trh->datatype, "s4")==0) ||
                    (strcmp (trh->datatype, "i4")==0)    )
          {
            for ( i = 0; i < trh->nsamp; i++ ) {
              fprintf ( stdout, "%6d ", *(long_data+i) );
              if(i%10==9) fprintf ( stdout, "\n" );
            }
          }
          else
          {
             fprintf (stdout, "Unknown datatype %s\n", trh->datatype);
          }
        } else {
             fprintf (stdout, "Data values compressed\n");
        }
        fprintf (stdout, "\n");
        fflush (stdout);
      }
      if (statistics_flag == 1)
      {
        if( logo.type == Type_TraceBuf2 ||
            logo.type == Type_TraceBuf     )
        {
          long int max=0;
          long int min=0;
          double avg=0.0;
          short short_value;
          int long_value;

          if ( (strcmp (trh->datatype, "s2")==0) ||
               (strcmp (trh->datatype, "i2")==0)    )
          {
            for ( i = 0; i < trh->nsamp; i++ ) {
              short_value = *(short_data+i);
              if (short_value > max || max == 0) {
                max = short_value;
              }
              if (short_value < min || min == 0) {
                min = short_value;
              }
              avg += short_value;
            }
            avg = avg / trh->nsamp;
            fprintf(stdout, "Raw Data statistics max=%ld min=%ld avg=%f\n",
                       max, min, avg);
            fprintf(stdout, "DC corrected statistics max=%f min=%f spread=%ld\n\n",
                       (double)(max - avg), (double)(min - avg),
                       labs(max - min));
          }
          else if ( (strcmp (trh->datatype, "s4")==0) ||
                    (strcmp (trh->datatype, "i4")==0)    )
          {
            for ( i = 0; i < trh->nsamp; i++ ) {
              long_value = *(long_data+i);
              if (long_value > max || max == 0) {
                max=long_value;
              }
              if (long_value < min || min == 0) {
                min=long_value;
              }
              avg += long_value;
            }
            avg = avg / trh->nsamp;
            fprintf(stdout, "Raw Data statistics max=%ld min=%ld avg=%f\n",
                       max, min, avg);
            fprintf(stdout, "DC corrected statistics max=%f min=%f spread=%ld\n\n",
                       (double)(max - avg), (double)(min - avg),
                       labs(max - min));
          }
          else
          {
             fprintf (stdout, "Unknown datatype %s\n", trh->datatype);
          }
        } else {
             fprintf (stdout, "Data values compressed\n");
        }
        fflush (stdout);
      } /* end of statistics_flag if */
    } /* end of process this tracebuf if */
  } /* end of while loop */
/* */
  tport_detach( &Region );

  if (seconds_to_live > 0 && start_time > 0.0)
  {
    datestr23 (start_time, stime, 256);
    datestr23 (end_time,   etime, 256);
    fprintf(stdout, "Sniffed %s for %d seconds:\n", argv[1], seconds_to_live);
    fprintf(stdout, "\tStart Time of first packet:  %s\n", stime);
    fprintf(stdout, "\t   End Time of last packet:  %s\n", etime);
    fprintf(stdout, "\t           Seconds of data:  %f\n", end_time - start_time);
    fprintf(stdout, "\t             Bytes of data:  %ld\n", packet_total_size);
    fprintf(stdout, "\t Number of Packets of data:  %ld\n", packet_total);
  }
  if (seconds_to_live > 0 && start_time == 0.0)  {
    fprintf(stdout, "Sniffed %s for %d seconds and found no packets matching desired SCN[L] filter.\n", argv[1], seconds_to_live);
  }
  return 0;
}

int search_scnl_time(SCNL_LAST_SCNL_PACKET_TIME *scnl_time, int *n_scnl_time, char *sta, char *chan, char *net, char *loc) {
    char str_SCNL[MAX_LEN_STR_SCNL];
    int i = 0;
    int found = 0;

    /* initialize a SCNL string from current packet */
    snprintf(str_SCNL, MAX_LEN_STR_SCNL, "%s.%s.%s.%s", sta, chan, net, loc);

    /* search for SCNL string in scnl array */
    i = 0;
    while(i < *n_scnl_time &&  !found) {
	if(strcmp(str_SCNL, scnl_time[i].scnl) == 0) {
	    found = 1;
	} else {
	    i++;
	}
    }

    if(!found) {
	/* just add if space is available */
	if(*n_scnl_time < MAX_NUM_SCNL-1) {
	    strncpy(scnl_time[*n_scnl_time].scnl, str_SCNL, MAX_LEN_STR_SCNL);
	    scnl_time[*n_scnl_time].last_scnl_time = -1.0;
	    scnl_time[*n_scnl_time].last_scnl_starttime = -1.0;
	    scnl_time[*n_scnl_time].last_packet_time = -1.0;
	    (*n_scnl_time)++;
	}  else {
	    /* TODO error message */
	    i = -1;
	    fprintf(stderr, "sniffwave: error: number of channels for gaps and latencies exceeded MAX_NUM_SCNL (%d).\n                  SCNL %s will not be monitored.",
		    MAX_NUM_SCNL, str_SCNL);
	}
    }

    return i;
}

int  check_for_gap_overlap_and_output_message(TRACE2_HEADER  *trh, SCNL_LAST_SCNL_PACKET_TIME *scnl_time, int *n_scnl_time) {
    int ret = 0;
    int i = 0;
    char st1[256], st2[256], type_str[32];
    double diff_time;
    double SCNL_TIME_DIFF_TOLLERANCE;
    double t1, t2;

    type_str[0]=0;

    /* Get index for SCNL */
    i = search_scnl_time(scnl_time, n_scnl_time, trh->sta, trh->chan, trh->net, trh->loc);

    if(i < 0) {
	/* Error message has been printed by search_scnl_time() */
	ret = i;
    } else {

	/* Skips the check the first time for the current scnl */
	if(scnl_time[i].last_scnl_time > 0.0) {

	    t1 = scnl_time[i].last_scnl_time;
	    t2 = trh->starttime;

	    /* compare to last scnl time */
	    SCNL_TIME_DIFF_TOLLERANCE = 2.0 * (1.0 / (double) trh->samprate);
	    diff_time =  t2 - t1;

	    if(fabs(diff_time) > SCNL_TIME_DIFF_TOLLERANCE) {

		/* check for a gap or an overlap */
		if(diff_time > 0.0) {
			/* it is a gap */
			strcpy(type_str, "gap");
		}
                else if (trh->endtime < scnl_time[i].last_scnl_starttime) {
		    strcpy(type_str, "out-of-order"); /* this packet ends BEFORE the last packet started! */
                }
                else {
		    t1 = trh->starttime;
		    strcpy(type_str, "overlap");
		    /* it is an overlap */
		    if(scnl_time[i].last_scnl_time > (trh->endtime + (1.0 / (double) trh->samprate))) {
			t2 = trh->endtime + (1.0 / (double) trh->samprate);
		    } else {
			t2 = scnl_time[i].last_scnl_time;
		    }
		    diff_time = t1 - t2;
		}

		/* Convert double epochs to string */
		datestr23 (t1, st1, 256);
		datestr23 (t2, st2, 256);

		/* output message for gap or overlap of out-of-order */
		fprintf(stdout, "%15s %8s of %6.2fs        %s (%.4f) %s (%.4f)\n",
			scnl_time[i].scnl, type_str, fabs(diff_time), st1, t1, st2, t2);

	    }
	}
	/* Save last time for current scnl */
	scnl_time[i].last_scnl_time = trh->endtime + (1.0 / (double) trh->samprate);
	scnl_time[i].last_scnl_starttime = trh->starttime;
    }

    return ret;
}

void  print_data_feed_latencies(TRACE2_HEADER  *trh, SCNL_LAST_SCNL_PACKET_TIME *scnl_time, int *n_scnl_time) {
    double d_now;
    double d_latency_second;
    double d_packet_latency_second = 0.0;
    int i = 0;

    /* Get current time */
    hrtime_ew( &d_now );

    /* Compute latency */
    d_latency_second = d_now - trh->endtime;

    /* Get index for SCNL */
    i = search_scnl_time(scnl_time, n_scnl_time, trh->sta, trh->chan, trh->net, trh->loc);

    if(i < 0) {
	/* Error message has been printed by search_scnl_time() */
    } else {
	if(scnl_time[i].last_packet_time > 0.0) {
	    /* Compute packet latency */
	    d_packet_latency_second = d_now - scnl_time[i].last_packet_time;
	}
	scnl_time[i].last_packet_time = d_now;
    }

    /* Print Data and Feed latencies */
    fprintf( stdout, " [D:%4.2fs F:%4.1fs]", d_latency_second, d_packet_latency_second);

}

/*
 *
 */
static void end_process_signal( int sig )
{
	tport_detach( &Region );
	return;
}

/*
 * handle_signal() -
 */
static void handle_signal( void )
{
	struct sigaction act;

/* Signal handling */
	sigemptyset(&act.sa_mask);
	act.sa_sigaction = NULL;
	act.sa_flags     = 0;
	act.sa_handler   = end_process_signal;

	sigaction(SIGINT , &act, (struct sigaction *)NULL);
	sigaction(SIGQUIT, &act, (struct sigaction *)NULL);

	return;
}
