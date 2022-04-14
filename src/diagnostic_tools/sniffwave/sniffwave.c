/*
 *                            sniffwave.c
 *
 * Program to read waveform messages from shared memory and
 * write to a disk file.
 *
 * Modified by Lucky Vidmar Tue May 11 11:27:35 MDT 1999
 *   allows for more arguments which specify
 *     Ring to read from
 *     SCN of the messages to print
 *     Whether or not trace data should be printed
 *
 *
 * Usage: sniffwave  <Ring> <Sta> <Comp> <Net> <Loc> <Data y/n>
 *    or: sniffwave  <Ring> <Sta> <Comp> <Net> <Data y/n>
 *    or: sniffwave  <Ring>
 *
 */
#define _GNU_SOURCE
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <search.h>

#include <earthworm.h>
#include <transport.h>
#include <swap.h>
#include <time_ew.h>
#include <trace_buf.h>
#include <libmseed.h>

/* */
#define PROC_VERSION  "HyEW 2022-04-14 0.0.1"
/* */
#define DEF_WILD_STR  "wild"
#define NLOGO          5
#define TYPE_NOTUSED   254

/*
 * Definition for checking SCNL gaps and latencies
 */
/* Structure for storing channel information, last scnl time and last packet time */
typedef struct {
	char sta[TRACE2_STA_LEN];
	char chan[TRACE2_CHAN_LEN];
	char net[TRACE2_NET_LEN];
	char loc[TRACE2_LOC_LEN];
	double last_scnl_time;       /* end time of last packet   */
	double last_scnl_starttime;  /* start time of last packet */
	double last_packet_time;
} SCNL_LAST_PACKET_TIME;

/* */
static void usage( int, char ** );
/* Return index from array scnl_time for SCNL (sta, cha, net, loc). If the SCNL is not present yet, it is inserted. */
static SCNL_LAST_PACKET_TIME *search_scnl_lptime( void **, const char *, const char *, const char *, const char * );
static SCNL_LAST_PACKET_TIME *find_scnl_lptime( void **, const char *, const char *, const char *, const char * );
static void proc_reject_trace( const TRACE2_HEADER * );
/* Check if there is a gap from the last packet and the current one in trh.
 * A message is printed to stdout if a gap or an overlap occurs.  */
static void proc_gap_and_overlap( const TRACE2_HEADER *, SCNL_LAST_PACKET_TIME * );
/* Print to stdout latencies for
 *      Data (now - last_scnl_time)
 *      Feed (now - last_packet_time) */
static void proc_feed_latencies( const TRACE2_HEADER *, SCNL_LAST_PACKET_TIME * );
static void proc_trace_data( const unsigned char, const TRACE2_HEADER *, const int, const int );
/* */
static void free_node( void * );
static int  compare_scnl( const void *, const void * );
static void end_process_signal( int );
static void handle_signal( void );

/* */
static int Verbose  = 0; /* if verbose =1 print name and number for logo ids */
static int DataFlag = 0;
static int StatFlag = 0;
static int LiveSec  = 0;
static int Terminate = 0; /* Local terminating flag for signal handler */
/* Arguments from command line */
static char *InRingName = NULL;
static char *GetSta     = NULL;
static char *GetComp    = NULL;
static char *GetNet     = NULL;
static char *GetLoc     = NULL;
static int   nLogo      = NLOGO; /* if nLogo = 5 then include TraceBuf2s */
/* Message logo related */
static unsigned char InstId;        /* local installation id              */
static unsigned char Type_Mseed;
static unsigned char Type_TraceBuf;
static unsigned char Type_TraceBuf2;
static unsigned char Type_TraceComp;
static unsigned char Type_TraceComp2;
static unsigned char InstWildcard;
static unsigned char ModWildcard;

/* */
#define SWAP_NULL_BY_WILD( SCNL ) \
		((SCNL) ? (SCNL) : DEF_WILD_STR)
/* Cast quality fields to unsigned char when you want convert to hexadecimal. */
#define CAST_QUALITY( q ) \
		((unsigned char)q)
#define SCNL_IS_WILD( SCNL ) \
		(!(SCNL) || (!strcmp((SCNL), DEF_WILD_STR) || !strcmp((SCNL), "WILD") || !strcmp((SCNL), "*")))
#define WAVE_IS_TRACEBUF( WAVE_TYPE ) \
		((WAVE_TYPE) == Type_TraceBuf || (WAVE_TYPE) == Type_TraceBuf2)
#define WAVE_IS_VERSION1( WAVE_TYPE ) \
		((WAVE_TYPE) == Type_TraceBuf || (WAVE_TYPE) == Type_TraceComp)
#define WAVE_IS_VERSION2( WAVE_TYPE ) \
		((WAVE_TYPE) == Type_TraceBuf2 || (WAVE_TYPE) == Type_TraceComp2)
#define SCNL_WILD_FILTER( FILTER, SCNL_IN ) \
		(!(FILTER) || !strcmp((FILTER), (SCNL_IN)))
#define WAVE_SCNL_FILTER( TRH, TYPE ) \
		(SCNL_WILD_FILTER( GetSta, (TRH)->sta ) && \
		SCNL_WILD_FILTER( GetComp, (TRH)->chan ) && \
		SCNL_WILD_FILTER( GetNet, (TRH)->net ) && \
		((WAVE_IS_VERSION2( (TYPE) ) && SCNL_WILD_FILTER( GetLoc, (TRH)->loc )) || WAVE_IS_VERSION1( (TYPE) )))

/*
 *
 */
int main( int argc, char **argv )
{
/* */
	SHM_INFO      region;
	long          ringkey;         /* Key to the transport ring to read from */
	MSG_LOGO      getlogo[NLOGO];
	MSG_LOGO      gotlogo;
	long          gotsize;
	unsigned char seq = 0;
/* */
	char           buffer[MAX_TRACEBUF_SIZ];
	TRACE2_HEADER *trh = (TRACE2_HEADER *)buffer;
	char           orig_datatype[3];
	char           stime[256];
	char           etime[256];
	char           mod_name[MAX_MOD_STR];
	char          *modid_name;
	MSRecord      *msr = NULL; /* mseed record */
	int            i, rc;
/* */
	double        start_time = -1.0;
	double        end_time   = -1.0;
	time_t        monitor_start_time = 0;
	unsigned long packet_total = 0;
	unsigned long packet_total_size = 0;
/* */
	void                  *root = NULL;
	SCNL_LAST_PACKET_TIME *slpt = NULL; /* Pointer to the last scnl and packet */

/* */
	usage( argc, argv );
	handle_signal();
/* Initialize pointers */
	trh = (TRACE2_HEADER *)buffer;
/* logit_init but do NOT WRITE to disk, this is needed for WaveMsg2MakeLocal() which logit()s */
	logit_init("sniffwave", 200, 200, 0);

/* Attach to ring */
	if ( (ringkey = GetKey(InRingName)) == -1 ) {
		fprintf(stderr, "Invalid RingName; exiting!\n");
		return -1;
	}
	tport_attach(&region, ringkey);

/* Look up local installation id */
	if ( GetLocalInst(&InstId) != 0 ) {
		fprintf(stderr, "sniffwave: error getting local installation id; exiting!\n");
		return -1;
	}
/* Specify logos to get */
	if ( GetType("TYPE_MSEED", &Type_Mseed) != 0 ) {
		fprintf(stderr, "%s: WARNING: Invalid message type <TYPE_MSEED>! Missing from earthworm.d or earthworm_global.d\n", argv[0]);
		Type_Mseed = TYPE_NOTUSED;
	}
	if ( GetType("TYPE_TRACEBUF", &Type_TraceBuf) != 0 ) {
		fprintf(stderr, "%s: Invalid message type <TYPE_TRACEBUF>! Missing from earthworm.d or earthworm_global.d\n", argv[0]);
		return -1;
	}
	if ( GetType("TYPE_TRACE_COMP_UA", &Type_TraceComp) != 0 ) {
		fprintf(stderr, "%s: Invalid message type <TYPE_TRACE_COMP_UA>! Missing from earthworm.d or earthworm_global.d\n", argv[0]);
		return -1;
	}
	if ( GetType("TYPE_TRACEBUF2", &Type_TraceBuf2) != 0 ) {
		fprintf(stderr, "%s: Invalid message type <TYPE_TRACEBUF2>! Missing from earthworm.d or earthworm_global.d\n", argv[0]);
		return -1;
	}
	if ( GetType("TYPE_TRACE2_COMP_UA", &Type_TraceComp2) != 0 ) {
		fprintf(stderr,"%s: Invalid message type <TYPE_TRACE2_COMP_UA>! Missing from earthworm.d or earthworm_global.d\n", argv[0]);
		return -1;
	}
	if ( GetModId("MOD_WILDCARD", &ModWildcard) != 0) {
		fprintf(stderr, "%s: Invalid moduleid <MOD_WILDCARD>! Missing from earthworm.d or earthworm_global.d\n", argv[0]);
		return -1;
	}
	if ( GetInst("INST_WILDCARD", &InstWildcard) != 0 ) {
		fprintf(stderr, "%s: Invalid instid <INST_WILDCARD>! Missing from earthworm.d or earthworm_global.d\n", argv[0]);
		return -1;
	}
/* */
	for ( i = 0; i < nLogo; i++ ) {
		getlogo[i].instid = InstWildcard;
		getlogo[i].mod    = ModWildcard;
	}
	getlogo[0].type = Type_Mseed;
	getlogo[1].type = Type_TraceBuf;
	getlogo[2].type = Type_TraceComp;
	getlogo[3].type = Type_TraceBuf2;
	getlogo[4].type = Type_TraceComp2;

/* Flush the ring */
	while (
		tport_copyfrom(
			&region, getlogo, nLogo, &gotlogo, &gotsize, (char *)&buffer, MAX_TRACEBUF_SIZ, &seq
		) != GET_NONE
	) {
		packet_total++;
		packet_total_size += gotsize;
	}
	fprintf(stderr, "sniffwave: inRing flushed %ld packets of %ld bytes total.\n", packet_total, packet_total_size);
/* */
	if ( LiveSec > 0 ) {
		monitor_start_time = time(0);
		packet_total = packet_total_size = 0;
		start_time = 0.0;
	}
/* */
	while (
		!Terminate &&
		tport_getflag(&region) != TERMINATE &&
		(!(LiveSec > 0) || (time(0) - monitor_start_time < LiveSec))
	) {
	/* */
		rc = tport_copyfrom(&region, getlogo, nLogo, &gotlogo, &gotsize, buffer, MAX_TRACEBUF_SIZ, &seq);
	/* */
		if ( rc == GET_NONE ) {
			sleep_ew(200);
			continue;
		}
		else if ( rc == GET_TOOBIG ) {
			fprintf(stderr, "sniffwave: retrieved message too big (%ld) for msg\n", gotsize);
			continue;
		}
		else if ( rc == GET_NOTRACK ) {
			fprintf(stderr, "sniffwave: Tracking error.\n");
		}
		else if ( rc == GET_MISS_LAPPED ) {
			fprintf(stderr, "sniffwave: Got lapped on the ring.\n");
		}
		else if ( rc == GET_MISS_SEQGAP ){
			fprintf(stderr, "sniffwave: Gap in sequence numbers\n");
		}
		else if ( rc == GET_MISS ) {
			fprintf( stderr, "sniffwave: Missed messages\n");
		}

	/* Check SCNL of the retrieved message */
		if ( Type_Mseed != TYPE_NOTUSED && gotlogo.type == Type_Mseed ) {
		/* Unpack record header and not data samples */
			if ( msr_unpack(buffer, gotsize, &msr, DataFlag | StatFlag, Verbose) != MS_NOERROR ) {
				fprintf(stderr, "Error parsing mseed record\n");
				continue;
			}
		/* Print record information */
			msr_print(msr, Verbose);
			msr_free(&msr);
			continue;
		}

		if ( WAVE_SCNL_FILTER( trh, gotlogo.type ) ) {
		/* */
			strcpy(orig_datatype, trh->datatype);
		/* */
			if ( WaveMsg2MakeLocal(trh) < 0 ) {
				proc_reject_trace( trh );
				continue;
			}
		/* */
			if ( LiveSec > 0 ) {
				if ( start_time < 0.0 )
					start_time = trh->starttime;
				end_time = trh->endtime;
				packet_total++;
				packet_total_size += gotsize;
			}

		/* */
			slpt = search_scnl_lptime( &root, trh->sta, trh->chan, trh->net, trh->loc );
		/* Check for gap and in case output message */
			proc_gap_and_overlap( trh, slpt );
		/* */
			datestr23(trh->starttime, stime, 256);
			datestr23(trh->endtime,   etime, 256);
			if ( WAVE_IS_VERSION2( gotlogo.type ) ) {
				fprintf(
					stdout, "%5s.%s.%s.%s (0x%x 0x%x) ",
					trh->sta, trh->chan, trh->net, trh->loc, trh->version[0], trh->version[1]
				);
			}
			else {
				fprintf(stdout, "%5s.%s.%s ", trh->sta, trh->chan, trh->net);
			}

			fprintf(stdout, "%d %s %3d ", trh->pinno, orig_datatype, trh->nsamp);
		/* more decimal places for slower sample rates */
			fprintf(stdout, trh->samprate < 1.0 ? "%6.4f" : "%5.1f", trh->samprate);
			fprintf(stdout, " %s (%.4f) %s (%.4f) ", stime, trh->starttime, etime, trh->endtime);
			switch (trh->version[1]) {
			case TRACE2_VERSION1:
				fprintf(stdout, "0x%02x 0x%02x ", CAST_QUALITY(trh->quality[0]), CAST_QUALITY(trh->quality[1]));
				break;
			case TRACE2_VERSION11:
				fprintf(stdout, "%6.4f ", ((TRACE2X_HEADER *)trh)->x.v21.conversion_factor);
				break;
			}
		/* */
			if ( Verbose ) {
				if ( InstId == gotlogo.instid ) {
					modid_name = GetModIdName(gotlogo.mod);
					if ( modid_name != NULL )
						sprintf(mod_name, "%s", modid_name);
					else
						sprintf(mod_name, "UnknownLocalMod");
				}
				else {
					sprintf(mod_name, "UnknownRemoteMod");
				}

				fprintf(
					stdout, "i%d=%s m%d=%s t%d=%s len%4ld",
					(int)gotlogo.instid, GetInstName(gotlogo.instid), (int)gotlogo.mod,
					mod_name, (int)gotlogo.type, GetTypeName(gotlogo.type), gotsize
				);
			}
			else {
				fprintf(stdout, "i%d m%d t%d len%4ld", (int)gotlogo.instid, (int)gotlogo.mod, (int)gotlogo.type, gotsize);
			}
		/* Compute and print latency for data and packet */
			proc_feed_latencies( trh, slpt );
		/* */
			proc_trace_data( gotlogo.type, trh, DataFlag, StatFlag );
		} /* end of process this tracebuf if */
	} /* end of while loop */
/* */
	tdestroy(root, free_node);
	tport_detach(&region);

	if ( LiveSec > 0 && start_time > 0.0 ) {
		datestr23(start_time, stime, 256);
		datestr23(end_time,   etime, 256);
		fprintf(stdout, "Sniffed %s for %d seconds:\n", argv[1], LiveSec);
		fprintf(stdout, "\tStart Time of first packet:  %s\n", stime);
		fprintf(stdout, "\t   End Time of last packet:  %s\n", etime);
		fprintf(stdout, "\t           Seconds of data:  %f\n", end_time - start_time);
		fprintf(stdout, "\t             Bytes of data:  %ld\n", packet_total_size);
		fprintf(stdout, "\t Number of Packets of data:  %ld\n", packet_total);
	}
	if ( LiveSec > 0 && start_time < 0.0 )  {
		fprintf(
			stdout, "Sniffed %s for %d seconds and found no packets matching desired SCN[L] filter.\n",
			argv[1], LiveSec
		);
	}

	return 0;
}

/*
 *
 */
static void usage( int argc, char **argv )
{
	int i;

/* check the verbosity level before ANY other tests on the command line */
	if ( argc > 2 && (!strcmp(argv[argc-1], "verbose") || !strcmp(argv[argc-1], "v")) ) {
		argc--;
		Verbose = 1;
	}
/* check if help parameter given */
	if ( argc >= 2 ) {
	/* skip any 1 or 2 leading switch characters */
		i = 0;
		if ( argv[1][i] == '-' || argv[1][i] == '/' )
			++i;
		if ( argv[1][i] == '-' || argv[1][i] == '/' )
			++i;
	/* parameter is "help" or "h" */
		if ( !strcmp(&argv[1][i], "help") || !strcmp(&argv[1][i], "h") )
		/* set arg count to make help screen show below */
			argc = 1;
	}

/* check if last parameter is y/n/s or time value */
	if ( argc > 2 ) {
		if ( !strcmp(argv[argc - 1], "y") ) {
			DataFlag = 1;
			argc--;
		}
		else if ( !strcmp(argv[argc - 1], "n") ) {
			DataFlag = 0;
			argc--;
		}
		else if ( !strcmp(argv[argc - 1], "s") ) {
			StatFlag = 1;
			argc--;
		}
		else if ( argv[argc - 1][0] != '0' && (i = atoi(argv[argc - 1])) > 0 ) {
		/* not leading zero (i.e., "00" location) and is positive value */
			LiveSec = i;
			argc--;
		}
		else if ( argc == 7 ) {
		/* seventh param should have been processed above */
			fprintf(stderr, "sniffwave: Bad parameter: %s\n", argv[argc - 1]);
		/* set arg count to make help screen show below */
			argc = 1;
		}
	}

/* Check command line argument */
	if ( argc < 2 || argc > 6 ) {
		if ( argc > 6 )
			fprintf(stderr, "sniffwave: Too many parameters\n");
		fprintf(stderr, "%s version %s\n", argv[0], PROC_VERSION);
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
		exit(0);
	}

/* process given parameters, the first parameter is ring name */
	InRingName = argv[1];
/* any parameters not given are set as wildcards */
	if ( argc > 2 ) {
	/* at least station parameter given */
		GetSta = SCNL_IS_WILD( argv[2] ) ? NULL : argv[2];
		if ( argc > 3 ) {
		/* channel (component) parameter given */
			GetComp = SCNL_IS_WILD( argv[3] ) ? NULL : argv[3];
			if ( argc > 4 ) {
			/* network parameter given */
				GetNet = SCNL_IS_WILD( argv[4] ) ? NULL : argv[4];
			/* location parameter given (SCNL) */
				if ( argc > 5 )
					GetLoc = SCNL_IS_WILD( argv[5] ) ? NULL : argv[5];
			/* SCN without location parameter given, do not include tracebuf2s in search */
				else
					nLogo = 3;
			}
		}
	}
/* */
	if ( nLogo > 3 ) {
		fprintf(
			stdout, "Sniffing %s for %s.%s.%s.%s\n", InRingName,
			SWAP_NULL_BY_WILD( GetSta ), SWAP_NULL_BY_WILD( GetComp ),
			SWAP_NULL_BY_WILD( GetNet ), SWAP_NULL_BY_WILD( GetLoc )
		);
	}
	else {
		fprintf(
			stdout, "Sniffing %s for %s.%s.%s (excluding TraceBuf2 packets)\n", InRingName,
			SWAP_NULL_BY_WILD( GetSta ), SWAP_NULL_BY_WILD( GetComp ), SWAP_NULL_BY_WILD( GetNet )
		);
	}

	return;
}

/*
 * search_scnl_lptime() - Insert the trace to the tree.
 * Arguments:
 *
 * Returns:
 *    NULL = Didn't find the trace.
 *   !NULL = The Pointer to the trace.
 */
static SCNL_LAST_PACKET_TIME *search_scnl_lptime(
	void **root, const char *sta, const char *chan, const char *net, const char *loc
) {
	SCNL_LAST_PACKET_TIME *result = NULL;

/* Test if already in the tree */
	if ( (result = find_scnl_lptime( root, sta, chan, net, loc )) == NULL ) {
		result = (SCNL_LAST_PACKET_TIME *)calloc(1, sizeof(SCNL_LAST_PACKET_TIME));
		memcpy(result->sta, sta, TRACE2_STA_LEN);
		memcpy(result->net, net, TRACE2_NET_LEN);
		memcpy(result->loc, loc, TRACE2_LOC_LEN);
		memcpy(result->chan, chan, TRACE2_CHAN_LEN);

		if ( (result = tsearch(result, root, compare_scnl)) == NULL ) {
		/* Something error when insert into the tree */
			return NULL;
		}
	/* */
		result = *(SCNL_LAST_PACKET_TIME **)result;
		result->last_scnl_time      = -1.0;
		result->last_scnl_starttime = -1.0;
		result->last_packet_time    = -1.0;
	}

	return result;
}

/*
 * find_scnl_lptime( ) -- Output the SCNL_LAST_PACKET_TIME that match the SCNL.
 * Arguments:
 *
 * Returns:
 *    NULL = Didn't find the trace.
 *   !NULL = The Pointer to the trace.
 */
static SCNL_LAST_PACKET_TIME *find_scnl_lptime(
	void **root, const char *sta, const char *chan, const char *net, const char *loc
) {
	SCNL_LAST_PACKET_TIME  key;
	SCNL_LAST_PACKET_TIME *result = NULL;

/* */
	memcpy(key.sta,  sta, TRACE2_STA_LEN);
	memcpy(key.net,  net, TRACE2_NET_LEN);
	memcpy(key.loc,  loc, TRACE2_LOC_LEN);
	memcpy(key.chan, chan, TRACE2_CHAN_LEN);
/* Find which SCNL */
	if ( (result = tfind(&key, root, compare_scnl)) == NULL ) {
	/* Not found in trace table */
		return NULL;
	}

	return *(SCNL_LAST_PACKET_TIME **)result;
}

/*
 *
 */
static void proc_reject_trace( const TRACE2_HEADER *trh )
{
	int  i;
	char scnl[24] = { 0 };
	char dt[4];

/* */
	strcat(scnl, trh->sta);
	strcat(scnl, ".");
	strcat(scnl, trh->chan);
	strcat(scnl, ".");
	strcat(scnl, trh->net);
	strcat(scnl, ".");
	strcat(scnl, trh->loc);
/* now put a space if there are any punctuation marks */
	for ( i = 0; i < 15; i++ ) {
		if ( !isalnum(scnl[i]) && !ispunct(scnl[i]) )
			scnl[i] = ' ';
	}
	memcpy(dt, trh->datatype, 2);
	for ( i = 0; i < 2; i++ ) {
		if ( !isalnum(dt[i]) && !ispunct(dt[i]) )
			dt[i] = ' ';
	}
	dt[i] = 0;
	fprintf(stderr, "WARNING: WaveMsg2MakeLocal rejected tracebuf.  Discard (%s).\n", scnl);
	fprintf(stderr, "\tdatatype=[%s]\n", dt);

	return;
}

/*
 * proc_gap_and_overlap() -
 */
static void proc_gap_and_overlap( const TRACE2_HEADER *trh, SCNL_LAST_PACKET_TIME *scnl_time )
{
	char   str1[256], str2[256];
	char   type_str[32] = { 0 };
	double diff_time;
	double tollerance;
	double t1, t2;

	if ( scnl_time ) {
	/* Skips the check the first time for the current scnl */
		if ( scnl_time->last_scnl_time > 0.0 ) {
			t1 = scnl_time->last_scnl_time;
			t2 = trh->starttime;
		/* compare to last scnl time */
			tollerance = 2.0 * (1.0 / (double) trh->samprate);
			diff_time =  t2 - t1;

			if ( fabs(diff_time) > tollerance ) {
			/* check for a gap or an overlap */
				if ( diff_time > 0.0 ) {
				/* it is a gap */
					strcpy(type_str, "gap");
				}
				else if ( trh->endtime < scnl_time->last_scnl_starttime ) {
				/* this packet ends BEFORE the last packet started! */
					strcpy(type_str, "out-of-order");
				}
				else {
					t1 = trh->starttime;
					strcpy(type_str, "overlap");
				/* it is an overlap */
					if ( scnl_time->last_scnl_time > (trh->endtime + (1.0 / (double)trh->samprate)) )
						t2 = trh->endtime + (1.0 / (double)trh->samprate);
					else
						t2 = scnl_time->last_scnl_time;
					diff_time = t1 - t2;
				}
			/* Convert double epochs to string */
				datestr23(t1, str1, 256);
				datestr23(t2, str2, 256);
			/* Output message for gap or overlap of out-of-order */
				fprintf(
					stdout, "%5s.%s.%s.%s %8s of %6.2fs        %s (%.4f) %s (%.4f)\n",
					scnl_time->sta, scnl_time->chan, scnl_time->net, scnl_time->loc,
					type_str, fabs(diff_time), str1, t1, str2, t2
				);
			}
		}
	/* Save last time for current scnl */
		scnl_time->last_scnl_time      = trh->endtime + (1.0 / (double) trh->samprate);
		scnl_time->last_scnl_starttime = trh->starttime;
	}

	return;
}

/*
 *
 */
static void proc_feed_latencies( const TRACE2_HEADER *trh, SCNL_LAST_PACKET_TIME *scnl_time )
{
	double d_now;
	double d_latency_second;
	double d_packet_latency_second = 0.0;

/* Get current time */
	hrtime_ew(&d_now);
/* Compute latency */
	d_latency_second = d_now - trh->endtime;

	if ( scnl_time ) {
		if ( scnl_time->last_packet_time > 0.0 )
		/* Compute packet latency */
			d_packet_latency_second = d_now - scnl_time->last_packet_time;
	/* */
		scnl_time->last_packet_time = d_now;
	}
/* Print Data and Feed latencies */
	fprintf(stdout, " [D:%4.2fs F:%4.1fs]", d_latency_second, d_packet_latency_second);
	fprintf(stdout, "\n");
	fflush(stdout);

	return;
}

/*
 *
 */
static void proc_trace_data(
	const unsigned char type, const TRACE2_HEADER *trh, const int data_flag, const int statis_flag
) {
	int      i = 0;
/* */
	int16_t *sdata = (int16_t *)(trh + 1);
	int32_t *ldata = (int32_t *)(trh + 1);
/* */
	long   max = 0;
	long   min = 0;
	double avg = 0.0;

	if ( data_flag || statis_flag ) {
		if ( WAVE_IS_TRACEBUF( type ) ) {
			if ( !strcmp(trh->datatype, "s2") || !strcmp(trh->datatype, "i2") ) {
				for ( i = 0; i < trh->nsamp; i++, sdata++ ) {
				/* */
					if ( data_flag ) {
						fprintf(stdout, "%6hd ", *sdata);
						if ( i % 10 == 9 )
							fprintf(stdout, "\n");
					}
				/* */
					if ( statis_flag ) {
						if ( *sdata > max || max == 0 )
							max = *sdata;
						if ( *sdata < min || min == 0 )
							min = *sdata;
						avg += *sdata;
					}
				}
			}
			else if ( !strcmp(trh->datatype, "s4") || !strcmp(trh->datatype, "i4") ) {
				for ( i = 0; i < trh->nsamp; i++, ldata++ ) {
				/* */
					if ( data_flag ) {
						fprintf(stdout, "%6d ", *ldata);
						if ( i % 10 == 9 )
							fprintf(stdout, "\n");
					}
				/* */
					if ( statis_flag ) {
						if ( *ldata > max || max == 0 )
							max = *ldata;
						if ( *ldata < min || min == 0 )
							min = *ldata;
						avg += *ldata;
					}
				}
			}
			else {
				fprintf(stdout, "Unknown datatype %s\n", trh->datatype);
			}
		}
		else {
			fprintf(stdout, "Data values compressed\n");
		}
	/* */
		fprintf(stdout, "\n");
	/* */
		if ( i && statis_flag ) {
			avg = avg / trh->nsamp;
			fprintf(stdout, "Raw Data statistics max=%ld min=%ld avg=%f\n", max, min, avg);
			fprintf(
				stdout, "DC corrected statistics max=%f min=%f spread=%ld\n\n",
				(double)(max - avg), (double)(min - avg), labs(max - min)
			);
		}
		fflush(stdout);
	}

	return;
}

/*
 * free_node() - free node of binary tree search
 */
static void free_node( void *node )
{
	SCNL_LAST_PACKET_TIME *slpt = (SCNL_LAST_PACKET_TIME *)node;

	free(slpt);

	return;
}

/*
 * compare_scnl() - the SCNL compare function of binary tree search
 */
static int compare_scnl( const void *a, const void *b )
{
	SCNL_LAST_PACKET_TIME *tmpa = (SCNL_LAST_PACKET_TIME *)a;
	SCNL_LAST_PACKET_TIME *tmpb = (SCNL_LAST_PACKET_TIME *)b;
	int                    rc;

	if ( (rc = strcmp(tmpa->sta, tmpb->sta)) != 0 )
		return rc;
	if ( (rc = strcmp(tmpa->chan, tmpb->chan)) != 0 )
		return rc;
	if ( (rc = strcmp(tmpa->net, tmpb->net)) != 0 )
		return rc;
	return strcmp(tmpa->loc, tmpb->loc);
}

/*
 *
 */
static void end_process_signal( int sig )
{
	Terminate = 1;
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
