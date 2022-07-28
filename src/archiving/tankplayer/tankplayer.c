/*********************************************************************
 *                           tankplayer.c                            *
 *                                                                   *
 *  Program to read a waveform file & play it into a transport ring. *
 *  Reads files containing TYPE_ADBUF, TYPE_TRACEBUF, or             *
 *  TYPE_TRACEBUF2 msgs.                                             *
 *  Places messages into ring in pseudo-real-time, using the         *
 *  timestamps in the message headers to determine the temporal      *
 *  spacing of messages.                                             *
 *    For TYPE_ADBUF msgs, tankplayer spaces msgs by the delta-t     *
 *                    between the starttimes in the headers,         *
 *    For TYPE_TRACEBUF & TYPE_TRACEBUF2, tankplayer spaces msgs by  *
 *                    the delta-t between the endtimes in headers.   *
 *                                                                   *
 *  Command line arguments:                                          *
 *     arvg[1] = tankplayer's configuration file                     *
 *********************************************************************/

/* Added VERSION number 1.0.2  on Nov 5, 2014 */
#define VERSION "1.0.3 2016.07.01"

/* */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
/* */
#include <data_buf.h>
#include <trace_buf.h>
#include <earthworm.h>
#include <kom.h>
#include <swap.h>
#include <time_ew.h>
#include <transport.h>
#include <mem_circ_queue.h>

/* */
#define MAX_BUFSIZ           51740
#define MAX_WAVEFILE         1000    /* max # waveform files to play   */
#define MAX_LEN              512     /* length of full directory name  */
#define FILE_INDICATOR_LOGO  0

/* Function prototypes */
static void tankplayer_config( char * );
static void tankplayer_lookup( void );
static void tankplayer_status( unsigned char, short, char * );

static int fetch_adbuf( void *, const int, unsigned char *, int * );
static int fetch_trbuf( void *, const int, unsigned char *, int * );
static double get_time( const void *, const unsigned char );
static double set_time( void *, const unsigned char, const double );

static SHM_INFO Region;         /* Info structure for memory region  */
pid_t           MyPID = 0;      /* to use in heartbeat               */

/* Things read from configuration file */
static long          RingKey;        /* transport ring to attach to       */
static int           LogSwitch;      /* Log-to-disk switch                */
static unsigned char MyModId;        /* play as this module id            */
static long          HeartBeatInt;   /* seconds between heartbeats        */
static unsigned char PlayType;       /* read this msgtype from wavefile   */
static int           Pause;          /* # sec to sleep between wave files */
static long          StartUpDelay;   /* seconds to wait before playing    */
static int           ScreenMsg = 0;  /* =1 write info to screen, =0 don't */
static int           Debug = 0;
static int           AdjTime = 0;         /* =1 to adjust packet time, =0 don't                                     */
static double        LateTime = 0.0;      /* seconds earlier than present to put in packet timestamp.               */
static int           bBeSuperLenient = 0; /* be lenient about putting out tracebufs with incorrect version numbers. */
/*
 * the flag and number of ms to sleep between stuffing in tbufs,
 * regardless of header time stamp! If this is set to 0,
 * then all other delay settings apply
 */
static int       InterMessageDelayMillisecs = 0;
static char      GetFromDir[MAX_LEN]; /* directory to monitor for data      */
static unsigned  CheckPeriod = 1;     /* secs between looking for new files */
static int       OpenTries = 5;
static int       OpenWait = 200;      /* milliseconds between open tries                  */
static int       SaveDataFiles = 1;   /* if non-zero, move to SaveSubdir, 0, remove files */
static char     *TroubleSubdir = "trouble"; /* subdir for problem files    */
static char     *SaveSubdir = "save";       /* subdir for processed files  */
static char      WaveFile[MAX_WAVEFILE][MAX_LEN];  /* list of waveform files to play */
static int       nWaveFile = 0;                    /* # waveform files to play       */
/* Look up from earthworm.h */
static unsigned char InstId;         /* local installation id             */
static unsigned char TypeHeartBeat;
static unsigned char TypeError;
static unsigned char TypeADBuf;
static unsigned char TypeTraceBuf;
static unsigned char TypeTraceBuf2;
/* Define global some other variables */
static char           *ProgName = NULL;
static pthread_mutex_t QueueMutex;
static QUEUE           MsgQueue;    /* from queue.h, queue.c; sets up linked */
/* Thread things */
#define THREAD_STACK  8388608       /* 8388608 Byte = 8192 Kilobyte = 8 Megabyte */
#define THREAD_OFF    0             /* thread has not been started      */
#define THREAD_ALIVE  1             /* thread alive and well            */
#define THREAD_ERR   -1             /* thread encountered error quit    */
static volatile int   ReadFileThreadStatus = THREAD_OFF;
static volatile _Bool Finish = 0;

/* */
int main( int argc, char *argv[] )
{
	uint8_t        msg[MAX_BUFSIZ];   /* waveform data buffer read from file   */
	TRACE2_HEADER *wfhead;            /* pntr to header of TYPE_TRACEBUF2 msg  */
	MSG_LOGO       putlogo;           /* logo to attach to waveform msgs       */
	MSG_LOGO       getlogo;           /* logo fetch from the waveform files    */
	double         current_time;      /* current system time                   */
	double         pac_time = 0.0;    /* original time stamp on packet         */
	double         offset_time = 0.0; /* Difference between pac_time and current_time */
	double         wait;              /* Seconds to wait before sending pkt    */
	time_t         itime;             /* integer version of double times       */
	time_t         timeNow;           /* current system time                   */
	time_t         timeLastBeat;      /* system time of last heartbeat sent    */
	long           size = 0;
	int            first;             /* flag 1st waveform message of file     */
	int            i;
	char           lo[2];  /* logit arg1: ""  if ScreenMsg=0; "o"  otherwise  */
	char           lot[3]; /* logit arg1: "t" if ScreenMsg=0; "ot" otherwise  */
	char           current_file[MAX_LEN];		/* a pointer to the currently named tank file */
	int            res; 		/* processing result  -1 == failure */
	int            ret;
	ew_thread_t    tid;            /* Thread ID */

/* */
	wfhead = (TRACE2_HEADER *)msg;
/* Check arguments */
	if ( argc != 2 ) {
		fprintf(stdout, "Usage: tankplayer configfile\n");
		fprintf(stdout, "Version: %s\n", VERSION);
		return 0;
	}
/* */
	Finish = 1;
/* Initialize name of log-file & open it */
	logit_init(argv[1], 0, 512, 1);
	GetFromDir[0] = '\0';
	ProgName = argv[0];

/* Read configuration file */
	tankplayer_config( argv[1] );
	logit_init(argv[1], 0, 512, LogSwitch);
	logit("" , "tankplayer: Read command file <%s>\n", argv[1]);
	logit("" , "tankplayer: Version %s\n", VERSION);

	if ( nWaveFile > 0 && GetFromDir[0] != 0 ) {
		logit(
			"e", "%s: You can only use GetFromDir OR WaveFile modes individually, not together; "
			"exiting!\n", ProgName
		);
		return -1;
	}

/* Look up important info from earthworm.h tables */
	tankplayer_lookup();

/* Store my own processid */
	MyPID = getpid();

/* Set up out-going logo */
	if (
		PlayType != TypeADBuf &&
		PlayType != TypeTraceBuf &&
		PlayType != TypeTraceBuf2
	) {
		logit(
			"e", "tankplayer: Not programmed to process msg type <%d>",
			(int) PlayType
		);
		logit("e", "; exiting!\n");
		return 1;
	}
	putlogo.instid = InstId;
	putlogo.mod    = MyModId;
	putlogo.type   = PlayType;

/* Attach to transport ring */
	tport_attach(&Region, RingKey);
	logit("", "tankplayer: Attached to public memory region: %ld\n", RingKey);
/* Create a Mutex to control access to queue */
	CreateSpecificMutex(&QueueMutex);
/* Initialize the message queue */
	initqueue(&MsgQueue, 1024, (unsigned long)MAX_BUFSIZ);

/* Force a heartbeat to be issued in first pass thru main loop */
	timeLastBeat = time(&timeNow) - HeartBeatInt - 1;

/* Set up logit's first argument based on value of ScreenMsg */
	if ( ScreenMsg == 0 ) {
		strcpy(lo, "");
		strcpy(lot, "t");
	}
	else {
		strcpy(lo, "o");
		strcpy(lot, "ot");
	}

/* Hold off sending data for a specified bit while system staggers up */
	logit(lo, "%s: startup. Waiting %ld seconds\n", ProgName, StartUpDelay);
	for ( i = 0; i < StartUpDelay; i++ ) {
		sleep_ew(1000);
		if ( time(&timeNow) - timeLastBeat >= HeartBeatInt ) {
			timeLastBeat = timeNow;
			tankplayer_status( TypeHeartBeat, 0, "" );
		}
		res = tport_getflag(&Region);
		if ( res == TERMINATE || res == MyPID ) {
			logit(lot, "%s: Termination requested; exiting!\n", ProgName);
			goto exit_procedure;
		}
	}
/* */
	if ( nWaveFile == 0 && chdir_ew(GetFromDir) == -1 ) {
		logit("e", "%s: GetFromDir directory <%s> not found; exiting!\n", ProgName, GetFromDir);
		return -1;
	}
	if ( GetFromDir[0] != 0 && Debug )
		logit("et", "%s: changed to directory <%s>\n", ProgName, GetFromDir);
/* */
	if ( nWaveFile == 0 ) {
	/* Make sure trouble subdirectory exists */
		if ( CreateDir(TroubleSubdir) != EW_SUCCESS ) {
			logit("e", "%s: trouble creating trouble directory: %s/%s\n", ProgName, GetFromDir, TroubleSubdir);
			return -1;
		}
	/* Make sure save subdirectory exists (if it will be used) */
		if ( SaveDataFiles ) {
			if ( CreateDir(SaveSubdir) != EW_SUCCESS ) {
				logit("e", "%s: trouble creating save directory: %s/%s\n", ProgName, GetFromDir, SaveSubdir);
				return -1;
			}
		}
	}

/*--------- Main Loop: over list of waveform files to play----------------*/
	while ( 1 ) {
	/* Send the heartbeat */
		if ( time(&timeNow) - timeLastBeat >= HeartBeatInt ) {
			timeLastBeat = timeNow;
			tankplayer_status(TypeHeartBeat, 0, "");
		}
	/* See if it's time to stop */
		res = tport_getflag(&Region);
		if ( res == TERMINATE || res == MyPID ) {
			logit(lot, "%s: Termination requested; exiting!\n", ProgName);
			goto exit_procedure;
		}
	/* */
		if ( ReadFileThreadStatus == THREAD_OFF ) {
			if ( StartThread( thread_read_files, (unsigned)THREAD_STACK, &tid ) == -1 ) {
				logit("e", "%s: Error starting thread(read_files); exiting!\n", ProgName);
				goto exit_procedure;
			}
			ReadFileThreadStatus = THREAD_ALIVE;
		}
		else if ( ReadFileThreadStatus == THREAD_ERR ) {
			goto exit_procedure;
		}

	/* */
		RequestSpecificMutex(&QueueMutex);
		res = dequeue(&MsgQueue, (char *)msg, &size, &getlogo);
		ReleaseSpecificMutex(&QueueMutex);
		if ( res < 0 ) {
			sleep_ew(100);
		}
		else {
		/* */
			if ( getlogo.type == FILE_INDICATOR_LOGO ) {
			/* */
				itime = (time_t)pac_time;
				logit(lot, "last header time-stamp: UTC %s", asctime(gmtime(&itime)));
			/* */
				if ( strlen(msg) ) {
					first = 1;
					strcpy(current_file, msg);
					continue;
				}
				else {
					logit(lot, "%s: No more wavefiles to play; exiting!\n", ProgName);
					goto exit_procedure;
				}
			}
		/* Sleep until it's time to send this message */
			pac_time = get_time( msg, PlayType );
			hrtime_ew( &current_time );
		/* First trace... */
			if ( first ) {
				offset_time = ceil(current_time - pac_time);
				first = 0;
				logit(lo, "\n");
				logit(
					lo, "%s:  Reading    type:%3d  mod:%3d  from <%s>\n", ProgName, (int)PlayType, (int)getlogo.mod, current_file
				);
				logit(
					lo, "             Playing as type:%3d  mod:%3d  inst:%3d\n", (int)PlayType, (int)MyModId, (int)InstId
				);
				if ( AdjTime ) {
					logit(lot, "time shifted by %lf seconds from original\n", offset_time - LateTime);
					itime = (time_t)(pac_time + offset_time - LateTime);
				}
				else {
					itime = (time_t)pac_time;
				}

				logit(lot, " 1st header time-stamp: UTC %s", asctime(gmtime(&itime)));
			}
		/* */
			pac_time += offset_time;
		/* */
			if ( InterMessageDelayMillisecs > 0 ) {
				sleep_ew((unsigned)(InterMessageDelayMillisecs));
			}
			else {
				if ( (pac_time - current_time) > 120.0 ) {
					logit(
						"e", "WARNING:  waiting %d seconds for packet: <%s.%s.%s> %15.2lf\n",
						(int)(pac_time - current_time), wfhead->sta, wfhead->chan, wfhead->net, get_time( msg, PlayType )
					);
				}
				while ( pac_time > current_time ) {
					sleep_ew(100);
					hrtime_ew( &current_time );
					if ( time(&timeNow) - timeLastBeat >= HeartBeatInt ) {
						timeLastBeat = timeNow;
						tankplayer_status(TypeHeartBeat, 0, "");
					}
				}
			}
		/* */
			if ( AdjTime )
				pac_time = set_time( msg, PlayType, pac_time - LateTime );
		/*
			fprintf(
				stdout, "%s: current time-stamp:%15.2lf  dt:%5.0lf ms  dtsys:%5.0lf ms\r",
				ProgName, tcurr, dt, dtsys
			);
		*/ /* DEBUG */
			if ( tport_putmsg(&Region, &putlogo, (long)size, msg) != PUT_OK )
				logit("e", "%s: tport_putmsg error.\n", ProgName);
			if ( Debug )
				logit("t", "packet: <%s.%s.%s> %15.2lf\n", wfhead->sta, wfhead->chan, wfhead->net, get_time( msg, PlayType ));
		}
	} /*end-while */
exit_procedure:
	Finish = 0;
	tport_detach(&Region);
	freequeue(&MsgQueue);
	CloseSpecificMutex(&QueueMutex);
	fflush(stdout);

	return 0;
}

/*
 * tankplayer_config()  processes command file using kom.c functions
 *                      exits if any errors are encountered
 */
void tankplayer_config(char *configfile)
{
	int ncommand;  /* # of required commands you expect to process   */
	char init[10]; /* init flags, one byte for each required command */
	int nmiss;	   /* number of required commands that were missed   */
	char *comm;
	char *str;
	int nfiles;
	int success;
	int i;

/* Set to zero one init flag for each required command */
	ncommand = 8;
	for ( i = 0; i < ncommand; i++ )
		init[i] = 0;

/* Open the main configuration file */
	nfiles = k_open(configfile);
	if ( nfiles == 0 ) {
		logit("e", "tankplayer: Error opening command file <%s>; exiting!\n", configfile);
		exit(-1);
	}

/* Process all command files */
/* While there are command files open */
	while ( nfiles > 0 ) {
	/* Read next line from active file  */
		while ( k_rd() ) {
			comm = k_str(); /* Get the first token from line */

		/* Ignore blank lines & comments */
			if ( !comm )
				continue;
			if ( comm[0] == '#' )
				continue;

		/* Open a nested configuration file */
			if ( comm[0] == '@' ) {
				success = nfiles + 1;
				nfiles = k_open(&comm[1]);
				if ( nfiles != success ) {
					logit("e", "tankplayer: Error opening command file <%s>; exiting!\n", &comm[1]);
					exit(-1);
				}
				continue;
			}

		/* Process anything else as a command */
		/* Read the transport ring name */
		/* 0 */
			if ( k_its("RingName") ) {
				if ( (str = k_str()) != NULL ) {
					if ( (RingKey = GetKey(str)) == -1 ) {
						logit("e", "tankplayer: Invalid ring name <%s>; exiting!\n", str);
						exit(-1);
					}
				}
				init[0] = 1;
			}
		/* Read the log file switch */
		/* 1 */
			else if ( k_its("LogFile") ) {
				LogSwitch = k_int();
				init[1] = 1;
			}
		/* Read tankplayer's module id */
		/* 2 */
			else if ( k_its("MyModuleId") ) {
				if ( (str = k_str()) != NULL ) {
					if ( GetModId(str, &MyModId) != 0 ) {
						logit("e", "tankplayer: Invalid module name <%s>; exiting!\n", str);
						exit(-1);
					}
				}
				init[2] = 1;
			}
		/* Read type of waveform msg to read from tankfile */
		/* 3 */
			else if ( k_its("PlayMsgType") ) {
				if ( (str = k_str()) != NULL ) {
					if ( GetType(str, &PlayType) != 0 ) {
						logit("e", "tankplayer: Invalid message type <%s>", str);
						logit("e", " in <PlayMsgType> cmd; exiting!\n");
						exit(-1);
					}
				}
				init[3] = 1;
			}
		/* Read heartbeat interval (seconds) */
		/* 4 */
			else if ( k_its("HeartBeatInt") ) {
				HeartBeatInt = k_long();
				init[4] = 1;
			}
		/* Read a list of wave files to play */
		/* 5 */
			else if ( k_its("WaveFile") ) {
				if ( nWaveFile + 1 >= MAX_WAVEFILE ) {
					logit("e", "tankplayer: Too many <WaveFile> commands in <%s>", configfile);
					logit("e", "; max=%d; exiting!\n", MAX_WAVEFILE);
					exit(-1);
				}
				if ( (str = k_str()) != NULL ) {
					if ( strlen(str) > (size_t)MAX_LEN - 1 ) {
						logit("e", "tankplayer: Filename <%s> too long in <WaveFile>", str);
						logit("e", " cmd; max=%d; exiting!\n", MAX_LEN - 1);
						exit(-1);
					}
					strcpy(WaveFile[nWaveFile], str);
				}
				nWaveFile++;
				init[5] = 1;
			}
			else if ( k_its("GetFromDir") ) {
				str = k_str();
				if ( str )
					strncpy(GetFromDir, str, MAX_LEN);
				init[5] = 1;
			}
			else if ( k_its("CheckPeriod") ) {
				CheckPeriod = k_int();
			}
			else if ( k_its("OpenTries") ) {
				OpenTries = k_int();
			}
			else if ( k_its("OpenWait") ) {
				OpenWait = k_int();
			}
			else if ( k_its("SaveDataFiles") ) {
				SaveDataFiles = k_int();
			}
		/* Read #seconds to pause between playing wave files */
		/* 6 */
			else if ( k_its("Pause") ) {
				Pause = k_int();
				init[6] = 1;
			}
		/* Read #seconds to wait for system to come up before playing data */
		/* 7 */
			else if ( k_its("StartUpDelay") ) {
				StartUpDelay = k_int();
				init[7] = 1;
			}
		/* Flag for writing info to screen */
		/* Optional command */
			else if ( k_its("ScreenMsg") ) {
				ScreenMsg = k_int();
			}
		/* Optional packet time adjustment */
			else if ( k_its("SendLate") ) {
				AdjTime = 1;
				LateTime = k_val();
			}
			else if ( k_its("InterMessageDelayMillisecs") ) {
				InterMessageDelayMillisecs = (int)k_val();
			}
		/* Optional debug command */
			else if ( k_its("Debug") ) {
				Debug = k_int();
			}
			else if ( k_its("IgnoreTBVersionNumbers") ) {
				bBeSuperLenient = k_int();
			}
		/* Command is not recognized */
			else {
				logit("e", "tankplayer: <%s> unknown command in <%s>.\n", comm, configfile);
				continue;
			}

		/* See if there were any errors processing the command */
			if ( k_err() ) {
				logit("e", "tankplayer: Bad <%s> command in <%s>; exiting!\n", comm, configfile);
				exit(-1);
			}
		}
		nfiles = k_close();
	}

/* After all files are closed, check init flags for missed commands */
	nmiss = 0;
	for ( i = 0; i < ncommand; i++ ) {
		if (!init[i])
			nmiss++;
	}

	if ( nmiss ) {
		logit("e", "tankplayer: ERROR, no ");
		if (! init[0] ) logit("e", "<RingName> "     );
		if (! init[1] ) logit("e", "<LogFile> "      );
		if (! init[2] ) logit("e", "<MyModuleId> "   );
		if (! init[3] ) logit("e", "<PlayMsgType> "  );
		if (! init[4] ) logit("e", "<HeartBeatInt> " );
		if (! init[5] ) logit("e", "<WaveFile> "     );
		if (! init[6] ) logit("e", "<Pause> "        );
		if (! init[7] ) logit("e", "<StartUpDelay> " );
		logit("e", "command(s) in <%s>; exiting!\n", configfile);
		exit(-1);
	}

	return;
}

/*
 * tankplayer_lookup( )   Look up important info from earthworm.h tables
 */
void tankplayer_lookup( void )
{
	if ( GetLocalInst(&InstId) != 0 ) {
		fprintf(stderr, "tankplayer: error getting local installation id; exiting!\n");
		exit(-1);
	}
	if ( GetType("TYPE_ADBUF", &TypeADBuf) != 0 ) {
		fprintf(stderr, "tankplayer: Invalid message type <TYPE_ADBUF>; exiting!\n");
		exit(-1);
	}
	if ( GetType("TYPE_TRACEBUF", &TypeTraceBuf) != 0 ) {
		fprintf(stderr, "tankplayer: Invalid message type <TYPE_TRACEBUF>; exiting!\n");
		exit(-1);
	}
	if ( GetType("TYPE_TRACEBUF2", &TypeTraceBuf2) != 0 ) {
		fprintf(stderr, "tankplayer: Invalid message type <TYPE_TRACEBUF2>; exiting!\n");
		exit(-1);
	}
	if ( GetType("TYPE_HEARTBEAT", &TypeHeartBeat) != 0 ) {
		fprintf(stderr, "tankplayer: Invalid message type <TYPE_HEARTBEAT>; exiting!\n");
		exit(-1);
	}
	if ( GetType("TYPE_ERROR", &TypeError) != 0 ) {
		fprintf(stderr, "tankplayer: Invalid message type <TYPE_ERROR>; exiting!\n");
		exit(-1);
	}
	return;
}

/*
 * tankplayer_status() builds a heartbeat or error message & puts it into
 *                     shared memory.  Writes errors to log file & screen.
 */
void tankplayer_status( unsigned char type, short ierr, char *note )
{
	MSG_LOGO logo;
	char msg[256];
	long size;
	time_t t;

/* Build the message */
	logo.instid = InstId;
	logo.mod = MyModId;
	logo.type = type;

	time(&t);

	if ( type == TypeHeartBeat ) {
		sprintf(msg, "%ld %ld\n", (long)t, (long)MyPID);
	}
	else if ( type == TypeError ) {
		sprintf(msg, "%ld %hd %s\n", (long)t, ierr, note);
		logit("et", "tankplayer: %s\n", note);
	}

	size = (long)strlen(msg); /* don't include the null byte in the message */

/* Write the message to shared memory */
	if ( tport_putmsg(&Region, &logo, size, msg) != PUT_OK ) {
		if ( type == TypeHeartBeat ) {
			logit("et", "tankplayer: Error sending heartbeat.\n");
		}
		else if ( type == TypeError ) {
			logit("et", "tankplayer: Error sending error:%d.\n", ierr);
		}
	}

	return;
}

/*
 *
 */
static thr_ret thread_read_files( void *dummy )
{
	int i;
	int iw = -1;
	int fd = 0;
	int ret = 0;
	int result = 0;
	int file_offset = 0;      /* tracking where we are in the     */
	int correct_version = 1;  /* recording match of version info  */

	double current_time;   /* current system time                   */
	double lastdot = 0.0;  /* time last dot was written to screen   */

	char *current_file = NULL;
	char  lo[2];               /* logit arg1: "" if ScreenMsg=0; "o" otherwise */
	char  fname[MAX_LEN];      /* filename grabbed from polling dir */
	char  fnew[MAX_LEN + 25];  /* filename for moving to save dir */

	uint8_t        msg[MAX_BUFSIZ];                /* waveform data buffer read from file   */
	TRACE2_HEADER *wfhead = (TRACE2_HEADER *)msg;  /* pntr to header of TYPE_TRACEBUF2 msg  */
	MSG_LOGO       logo;                           /* logo to attach to waveform msgs       */

/* */
	ReadFileThreadStatus = THREAD_ALIVE;
/* Set up logit's first argument based on value of ScreenMsg */
	strcpy(lo, ScreenMsg ? "o" : "");
/* */
	logo.instid = InstId;
/* */
	while ( Finish ) {
		if ( nWaveFile > 0 ) {
		/* we are processing via listed files */
			iw++;
		/* we have reached the end! */
			if ( iw == nWaveFile ) {
			/* Sending the file starting message to the main process */
				logo.type = FILE_INDICATOR_LOGO;
				msg[0] = '\0';
				while ( getNumOfElementsInQueue(&MsgQueue) == 1024 )
					sleep_ew(100);
				RequestSpecificMutex(&QueueMutex);
				enqueue(&MsgQueue, (char *)msg, 1, logo);
				ReleaseSpecificMutex(&QueueMutex);
			/* */
				return NULL;
			}
		/* Open a listed waveform file */
			if ( (fd = open(WaveFile[iw], O_RDONLY, 0)) <= 0 ) {
				logit("e", "%s: Cannot open tank file <%s>\n", ProgName, WaveFile[iw]);
				continue;
			}
			if ( Debug ) {
				logit("e", "%s: starting tank <%s>\n", ProgName, WaveFile[iw]);
			}
			current_file = WaveFile[iw];
		}
		else {
		/* open the file in the directory specified */
			while ( Finish ) {
			/* No files found; wait for one to appear */
				if ( GetFileName(fname) == 1 ) {
					sleep_ew(CheckPeriod * 1000);
					continue;
				}
				else {
					if ( Debug )
						logit("et", "%s: got file name <%s>\n", ProgName, fname);
					break;
				}
			}
		/* we should have a file at this point */
			current_file = fname;
		/*
		 * Open the file.
		 * We open for updating (even though we only want to read it),
		 * as that will hopefully get us an exclusive open.
		 * We don't ever want to look at a file that's being written to.
		 */
			for ( i = 0; i < OpenTries && Finish; i++ ) {
				if ( fd = open(fname, O_RDONLY, 0) > 0 )
					break;
				sleep_ew(OpenWait);
			}
		/* */
			if ( !Finish )
				return NULL;
		/* failed to open file! */
			if ( fd <= 0 ) {
				logit("et", "%s: Error: Could not open %s after %d*%d msec.", ProgName, fname, OpenTries, OpenWait);
				result = -1;
				goto file_process;
			}
			if ( i > 0 ) {
				logit("t", "Warning: %d attempts required to open file %s\n", i + 1, fname);
			}
		}

	/* Sending the file starting message to the main process */
		logo.type = FILE_INDICATOR_LOGO;
		strcpy(msg, current_file);
		while ( getNumOfElementsInQueue(&MsgQueue) == 1024 )
			sleep_ew(100);
		RequestSpecificMutex(&QueueMutex);
		enqueue(&MsgQueue, (char *)msg, strlen(msg) + 1, logo);
		ReleaseSpecificMutex(&QueueMutex);

		file_offset = 0;
	/* Loop over one file: get a msg from file, write it to ring */
		while ( Finish ) {
		/* */
			logo.type = PlayType;
		/* Read ADBuf waveform message from tank */
			if ( PlayType == TypeADBuf ) {
				if ( (ret = fetch_adbuf(msg, fd, &logo.mod, &correct_version)) < 0 ) {
					break;
				}
				file_offset += ret;
			}
		/* Read TraceBuf waveform message from file */
			else if ( PlayType == TypeTraceBuf2 || PlayType == TypeTraceBuf ) {
				if ( (ret = fetch_trbuf(msg, fd, &logo.mod, &correct_version)) < 0 ) {
					break;
				}
			/* */
				if ( !correct_version && !bBeSuperLenient ) {
					logit(
						"e", "%s: Error: packet (%s,%s,%s) at file %s offset %d is not the correct TYPE_TRACEBUF2 version (%c%c).  \n"
						"Parsing it as one and attempting to continue.  Will not output this packet!\n",
						ProgName, wfhead->sta, wfhead->chan, wfhead->net, current_file, file_offset, wfhead->version[0], wfhead->version[1]
					);
				}
			/* */
				file_offset += ret;
			}
		/* Write waveform message to transport region */
			if ( ScreenMsg && (current_time - lastdot) > 1.0 ) {
				lastdot = current_time;
				fprintf(stdout, ".");
				fflush(stdout);
			}
		/*
			fprintf(
				stdout, "%s: current time-stamp:%15.2lf  dt:%5.0lf ms  dtsys:%5.0lf ms\r",
				ProgName, tcurr, dt, dtsys
			);
		*/ /* DEBUG */
			if ( correct_version || bBeSuperLenient ) {
				while ( getNumOfElementsInQueue(&MsgQueue) == 1024 )
					sleep_ew(100);
				RequestSpecificMutex(&QueueMutex);
				enqueue(&MsgQueue, (char *)msg, ret, logo);
				ReleaseSpecificMutex(&QueueMutex);

				if ( Debug )
					logit("t", "packet: <%s.%s.%s> %15.2lf\n", wfhead->sta, wfhead->chan, wfhead->net, get_time( msg, PlayType ));
			}
		} /*end-while over one file*/

	/* Clean up; get ready for next file */
		if ( ScreenMsg )
			fprintf(stdout, "\n");
		result = 0;
		if ( ret == 0 ) {
			logit(lo, "%s:  Reached end of <%s>\n", ProgName, current_file);
		}
		else if ( ret < 0 ) {
			logit(lo, "%s:  Error reading from <%s>\n", ProgName, current_file);
			result = -1;
		}
		logit(lo, "%s:  Closing <%s>\n", ProgName, current_file);
		close(fd);
	/* Pause between playing files */
		for ( i = 0; i < Pause && Finish; i++ )
			sleep_ew(1000);
	/* jump to next file in list */
		if ( iw != -1 )
			continue;
file_process:
	/* now handle the GetFromDir case, and clean up the tank file lying around as directed */
		if ( result >= 0 ) {
		/* Keep file around */
			if ( SaveDataFiles ) {
				sprintf(fnew, "%s/%s", SaveSubdir, fname);
				if ( rename_ew(fname, fnew) != 0 ) {
					logit("e", "error moving file to ./%s\n; exiting!", fnew);
					break;
				}
				else {
					logit("e", "moved to ./%s\n", SaveSubdir);
				}
			}
		/* Delete the file */
			else {
				if ( remove(fname) != 0 ) {
					logit("e", "error deleting file; exiting!\n");
					break;
				}
				else {
					logit("e", "deleted file.\n");
				}
			}
		}
		else {
		/* ...or there was trouble (result<0)! */
			logit("e", "\n");
			sprintf(fnew, "%s/%s", TroubleSubdir, fname);
			if ( rename_ew(fname, fnew) != 0 ) {
				logit("e", " error moving file to ./%s ; exiting!\n", fnew);
				break;
			}
			else {
				logit("e", " moved to ./%s\n", fnew);
			}
		}
	}
/* File a complaint to the main thread */
	if ( Finish )
		ReadFileThreadStatus = THREAD_ERR;

	KillSelfThread();

	return NULL;
}

/*
 *
 */
static int fetch_adbuf( void *buf, const int fd, unsigned char *module, int *correct_ver  )
{
	WF_HEADER *adh  = (WF_HEADER *)buf;
	uint8_t   *_buf = (uint8_t *)buf;
	int16_t    nchan;
	int16_t    nscan;
	int        size = sizeof(WF_HEADER);

/* Read the header */
	if ( read(fd, _buf, size) < size )
		return -1;
/* */
	_buf   += size;
	nchan   = adh->nchan;
	nscan   = adh->nscan;
	*module = adh->mod_id;
#ifdef _SPARC
	SwapShort(&nscan); /* Note: By definition, TYPE_ADBUF msgs */
	SwapShort(&nchan); /*       are always in INTEL byte order */
#endif
/* Relabel mod_id in msg header */
	adh->mod_id = MyModId;
/* Read pin #'s and data samples from file */
	size = sizeof(int16_t) * nchan * (nscan + 1);
	if ( (size + sizeof(WF_HEADER)) > MAX_BUFSIZ ) {
		logit("e", "%s: msg[%zu] adtype overflows internal buffer[%d]\n", ProgName, size + sizeof(WF_HEADER), MAX_BUFSIZ);
		return -2;
	}
	if ( read(fd, _buf, size) < size )
		return -1;
	size += sizeof(WF_HEADER);

	return size;
}

/*
 *
 */
static int fetch_trbuf( void *buf, const int fd, unsigned char *module, int *correct_ver )
{
	TRACE2_HEADER *trh  = (TRACE2_HEADER *)buf;
	uint8_t       *_buf = (uint8_t *)buf;
	int32_t        nsamp;
	uint8_t        byte_order;
	int32_t        byte_per_sample;
	int16_t        nscan;
	int            size = sizeof(TRACE2_HEADER);

/* Read the header */
	size = sizeof(TRACE2_HEADER);
	if ( read(fd, _buf, size) < size )
		return -1;

	_buf           += size;
	nsamp           = trh->nsamp;
	byte_order      = trh->datatype[0];
	byte_per_sample = atoi(&trh->datatype[1]);
	*module         = 0;
#ifdef _SPARC
	if ( byte_order == 'i' || byte_order == 'f' )
		SwapInt(&nsamp);
#endif
#ifdef _INTEL
	if ( byte_order == 's' || byte_order == 't' )
		SwapInt(&nsamp);
#endif
/* Test for version; skip file if wrong type */
	if ( PlayType == TypeTraceBuf2 ) {
		if ( !TRACE2_HEADER_VERSION_IS_VALID(trh) ) {
			*correct_ver = 0;
		}
		else {
			*correct_ver = 1;
		}
	}

/* Read data samples from file */
	size = byte_per_sample * nsamp;
	if ( (size + sizeof(TRACE2_HEADER)) > MAX_TRACEBUF_SIZ ) {
		logit("e", "%s: msg[%zu] tbuf2 overflows internal buffer[%d]\n", ProgName, size + sizeof(TRACE2_HEADER), MAX_TRACEBUF_SIZ);
		return -2;
	}

	if ( read(fd, _buf, size) < size )
		return -1;
	size += sizeof(TRACE2_HEADER);

	return size;
}

/*
 * get_time() - reads the header of a TYPE_ADBUF or TYPE_TRACEBUF2 msg.
 *              For TYPE_ADBUF msgs, returns the starttime as a double
 *              For TYPE_TRACEBUF2 msgs, returns the endtime as a double
 */
static double get_time( const void *msg, const unsigned char msgtype )
{
	WF_HEADER     *adh; /* Header for TYPE_ADBUF message     */
	TRACE2_HEADER *trh; /* Header for TYPE_TRACEBUF2 message  */
	int32_t tssec;
	int32_t tsmic;
	double  result = 0.0;

	if ( msgtype == TypeADBuf ) {
		adh   = (WF_HEADER *)msg;
		tssec = adh->tssec;
		tsmic = adh->tsmic;
#ifdef _SPARC
	/* Note: By definition, TYPE_ADBUF msgs are always in INTEL byte order */
		SwapInt32(&tssec);
		SwapInt32(&tsmic);
#endif
		result = (double)tssec + (0.000001 * tsmic);
	}
	else if ( msgtype == TypeTraceBuf || msgtype == TypeTraceBuf2 ) {
		trh    = (TRACE2_HEADER *)msg;
		result = trh->endtime;
#ifdef _SPARC
		if ( trh->datatype[0] == 'i' )
			SwapDouble(&result);
#endif
#ifdef _INTEL
		if ( trh->datatype[0] == 's' )
			SwapDouble(&result);
#endif
	}

	return result;
}

/*
 * set_time() - reads the header of a TYPE_ADBUF or TYPE_TRACEBUF2 msg.
 *              For TYPE_ADBUF msgs, it sets the starttime as a double
 *              For TYPE_TRACEBUF2 msgs, it sets the endtime and then calculates
 *              and sets the starttime based on origin difference between start & end.
 *              Byte swapping is done as necessary.
 */
static double set_time( void *msg, const unsigned char msgtype, const double time )
{
	WF_HEADER     *adh; /* Header for TYPE_ADBUF message     */
	TRACE2_HEADER *trh; /* Header for TYPE_TRACEBUF2 message  */
	int32_t tssec;
	int32_t tsmic;
	double starttime;
	double endtime;

	if ( msgtype == TypeADBuf ) {
		adh   = (WF_HEADER *)msg;
		tssec = (int32_t)time;
		tsmic = (int32_t)(1000000.0 * (time - (double)tssec));
#ifdef _SPARC
	/* Note: By definition, TYPE_ADBUF msgs are always in INTEL byte order */
		SwapInt32(&tssec);
		SwapInt32(&tsmic);
#endif
		adh->tssec = tssec;
		adh->tsmic = tsmic;
	}
	else if ( msgtype == TypeTraceBuf || msgtype == TypeTraceBuf2 ) {
		trh       = (TRACE2_HEADER *)msg;
		starttime = trh->starttime;
		endtime   = trh->endtime;
#ifdef _SPARC
		if ( trh->datatype[0] == 'i' || trh->datatype[0] == 'f' ) {
			SwapDouble(&starttime);
			SwapDouble(&endtime);
		}
#endif
#ifdef _INTEL
		if ( trh->datatype[0] == 's' || trh->datatype[0] == 't' ) {
			SwapDouble(&starttime);
			SwapDouble(&endtime);
		}
#endif
		starttime = time + (starttime - endtime);
		endtime   = time;
#ifdef _SPARC
		if (byte_order == 'i' || byte_order == 'f') {
			SwapDouble(&starttime);
			SwapDouble(&endtime);
		}
#endif
#ifdef _INTEL
		if (byte_order == 's' || byte_order == 't') {
			SwapDouble(&starttime);
			SwapDouble(&endtime);
		}
#endif
		trh->starttime = starttime;
		trh->endtime   = endtime;
	}

	return time;
}
