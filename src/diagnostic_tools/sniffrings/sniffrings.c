
/*****************************************************************************
 *                    sniffrings - a debugging tool
 *
 * sniffrings displays the content of the messages read from multiple rings.
 * It prints the logo, sequence # and length of the message, followed by the
 * message itself (except for binary waveform messages) to the screen.
 *
 * In addition to being an useful debugging tool, this module has been
 * developed also for:
 * 1) testing the functions that implement the semaphore operations:
 *     - DECLARE_SPECIFIC_SEMAPHORE_EW(semname)
 *     - CREATE_SPECIFIC_SEMAPHORE_EW(semname, count)
 *     - POST_SPECIFIC_SEMAPHORE_EW(semname)
 *     - WAIT_SPECIFIC_SEMAPHORE_EW(semname)
 *     - DESTROY_SPECIFIC_SEMAPHORE_EW(semname)
 *    within "sema_ew.c" for the different platforms: "solaris", "unix" and
 *    "winnt".
 * 2) defining a template for the development of an Earthworm module that
 *    requires to read and process efficiently messages coming from multiple
 *    rings simultaneously. It is based on the solution of the
 *    "producers/consumers"-problem which uses a FIFO-queue and two semaphores.
 *    This module can miss messages only when a 'stacker' is lapped on the ring
 *    it is reading. This can be due to the local FIFO-queue size and the speed
 *    of the 'consumer' to process the messages.
 *
 * Matteo Quintiliani - Istituto Nazionale di Geofisica e Vulcanologia - Italy
 * Mail bug reports and suggestions to <matteo.quintiliani@ingv.it>
 *
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <earthworm.h>
#include <transport.h>
#include <mem_circ_queue.h>

#define VERSION  "1.0.1 2016.10.10"

#define MAX_SIZE MAX_BYTES_PER_EQ   /* Largest message size in characters */

/* Thread things
 ***************/
#define THREAD_STACK 8192

/* Error messages used by sniffrings
 ***********************************/
#define  ERR_MISSMSG       0   /* message missed in transport ring        */
#define  ERR_TOOBIG        1   /* retreived msg too large for buffer      */
#define  ERR_NOTRACK       2   /* msg retreived; tracking limit exceeded  */
#define  ERR_QUEUE         3   /* queue error                             */

/* maximum length for error message */
#define STR_LEN_ERRTEXT 2048

DECLARE_SPECIFIC_SEMAPHORE_EW(sema_CountAvail);
DECLARE_SPECIFIC_SEMAPHORE_EW(sema_CountBusy);

void       inc_nStackerThread();
void       dec_nStackerThread();
mutex_t    mutex_nStackerThread;
int        nStackerRunning = 0;

mutex_t   mutex_OutQueue;            /* mutex for variable OutQueue */
QUEUE     OutQueue; 		     /* queue for saving messages read from ring */
long      MaxMsgSize = MAX_BYTES_PER_EQ; /* max size for input/output msgs           */
const int QueueSize = 1000;           /* max messages in output circular buffer   */

#define   FLAG_TERM_NULL   0            /* flag value for null info */
#define   FLAG_TERM_REQ    1            /* flag value for termination requested */
#define   FLAG_TERM_THR_PRINTER 2       /* flag value Printer thread is terminated */
#define   FLAG_TERM_THR_STACKER 4       /* flag value Stacker thread is terminated */
mutex_t   mutex_flags_term;             /* mutex for variable  flags_term */
int       flags_term = FLAG_TERM_NULL;          /* flags variable for handling termination */
void      setlck_flags_term(int flags_bitmap);  /* set flags_bitmap by lock mechanism */
int       testlck_flags_term(int flags_bitmap); /* test flags_bitmap by lock mechanism */

thr_ret MessageStacker( void *p);    /* thread: messages from ring to queue */
thr_ret MessagePrinter( void *p );        /* thread: messages from queue to database */

/* Things to look up in the earthworm.h tables with getutil.c functions
 **********************************************************************/
typedef struct {
  unsigned char InstId;        /* local installation id              */
  unsigned char InstWildCard;  /* wildcard for installations         */
  unsigned char ModWildCard;   /* wildcard for module                */
  unsigned char TypeWildCard;  /* wildcard for message type          */
  unsigned char TypeTraceBuf;  /* binary 1-channel waveform msg      */
  unsigned char TypeTraceBuf2; /* binary 1-channel waveform msg      */
  unsigned char TypeCompUA;    /* binary compressed waveform msg     */
  unsigned char TypeCompUA2;   /* binary compressed waveform msg     */
  unsigned char InstToGet;
  unsigned char TypeToGet;
  unsigned char ModToGet;
} TYPE_GETUTIL_INFO;

typedef struct {
  char        *cmdname;      /* pointer to executable name */
  char        *InRing;         /* Name of input ring           */
  MSG_LOGO    GetLogo[1];        /* array for requesting module,type,instid  */
  short       nLogo;           /* number of logos to get                   */
  int         no_flush;	       /*  flush ring buffer by default */
  TYPE_GETUTIL_INFO *getutil_info;
} TYPE_ARGS_STACKER;

typedef struct {
  char        *cmdname;      /* pointer to executable name */
  char        verbose;            /* verbose will print names     */
  TYPE_GETUTIL_INFO *getutil_info;
} TYPE_ARGS_PRINTER;


#define MAX_NUM_OF_RINGS 30
int explode_ringlist(char *s, char ringNames[MAX_NUM_OF_RINGS][MAX_RING_STR]);
int remove_duplicated_ringname(char ringNames[MAX_NUM_OF_RINGS][MAX_RING_STR], int *nRings);


int main( int argc, char *argv[] )
{
  TYPE_GETUTIL_INFO getutil_info;

  ew_thread_t     tidMessagePrinter;                 /* Thread read message from queue and populate DB */
  TYPE_ARGS_PRINTER args_printer;

  ew_thread_t     tidStacker[MAX_NUM_OF_RINGS];                    /* Thread moving messages from transport to queue */
  TYPE_ARGS_STACKER args_stacker[MAX_NUM_OF_RINGS];
  int          r = 0;
  int          nRings = 0;
  char         InRings[MAX_NUM_OF_RINGS][MAX_RING_STR];

  time_t       now;	                        /* current time, used for timing heartbeats */
  int          no_flush = 0;
  char         *cmdname;      /* pointer to executable name */

  const int    wait_time_thr_slice = 500;     /* slice of waiting time for joining all threads */
  const int    wait_time_thr_tot   = 5000;    /* total of waiting time for joining all threads */
  int          wait_time_thr_count = 0;       /* counter of waiting time for joining all threads */

  time_t       expiration_date = 0;           /* if non-0, self terminate after this time */

  cmdname = argv[0];

  /* Create Semaphores
   ********************************************/
  CREATE_SPECIFIC_SEMAPHORE_EW(sema_CountAvail, QueueSize);
  CREATE_SPECIFIC_SEMAPHORE_EW(sema_CountBusy,  0 );

  /* Check program arguments
   ***********************/
  if ( (argc < 2) || (argc > 9) )
  {
    printf( "Version %s\n", VERSION );
    printf( "Usage 1:  sniffrings [-n] <ringnames> [-t secs]\n" );
    printf( "Usage 2:  sniffrings [-n] <ringnames> <instid> <mod> <type> [-t secs]\n" );
    printf( "Usage 3:  sniffrings [-n] <ringnames> verbose  [-t secs]\n" );
    printf( "Usage 4:  sniffrings [-n] <ringnames> <instid> <mod> <type> verbose [-t secs]\n" );
    printf( "<ringnames> is in the form of comma-separated-values. (i.e. PICK_RING,HYPO_RING).\n" );
    printf( "Sniffring shows full ring, inst and module names if 'v' or \n" );
    printf( "'verbose' is specified as the final parameter. Otherwise you'll\n" );
    printf( "see numeric IDs for these three, as defined in earthworm*.d files. \n" );
    printf( "(Modules from external installations are not identified other than by number.)\n" );
    printf( "A new option is -n to not flush the ring of all messages first, must be before ringname if used\n" );
    printf( "the default is to flush the ring of all messages and only show those that come in after invocation\n" );
    printf( "-t secs specifies that sniffrings self-terminates after the specified number of seconds\n" );
    return -1;
  }

  if (strcmp(argv[1], "-n") == 0)
  {
    no_flush = 1; /* DO NOT FLUSH the buffer first, show all old messages first */
    nRings = explode_ringlist(argv[2], InRings);
  } else {
    nRings = explode_ringlist(argv[1], InRings);
  }

  if(nRings <= 0) {
    printf( "sniffrings: error getting ring names; exiting!\n" );
    return -1;
  }

  if ( argc >= 4 && strcmp(argv[argc-2],"-t") == 0 ) {
    int seconds = atoi( argv[argc-1] );
    if ( seconds <= 0 ) {
        printf( "sniffrings: argument to -t must be a positive integer; exiting!\n" );
        return -1;
    }
    expiration_date = seconds;    /* so we know to set a real time once the actual processing starts */
    argc -= 2;
  }


  remove_duplicated_ringname(InRings, &nRings);

  for(r=0; r < nRings; r++) {
    args_stacker[r].cmdname = cmdname;
    args_stacker[r].nLogo = 1;
    args_stacker[r].no_flush = no_flush;
    args_stacker[r].InRing  = InRings[r];
  }

  args_printer.cmdname = cmdname;
  args_printer.verbose = 1;


  /* Look up local installation id
   *****************************/
  if ( GetLocalInst( &getutil_info.InstId ) != 0 )
  {
    printf( "sniffrings: error getting local installation id; exiting!\n" );
    return -1;
  }
  if ( GetInst( "INST_WILDCARD", &getutil_info.InstWildCard ) != 0 )
  {
    printf( "sniffrings: Invalid installation name <INST_WILDCARD>" );
    printf( "; exiting!\n" );
    return -1;
  }

  /* Look up module ids & message types earthworm.h tables
   ****************************************************/
  if ( GetModId( "MOD_WILDCARD", &getutil_info.ModWildCard ) != 0 )
  {
    printf( "sniffrings: Invalid module name <MOD_WILDCARD>; exiting!\n" );
    return -1;
  }
  if ( GetType( "TYPE_WILDCARD", &getutil_info.TypeWildCard ) != 0 )
  {
    printf( "sniffrings: Invalid message type <TYPE_WILDCARD>; exiting!\n" );
    return -1;
  }
  if ( GetType( "TYPE_TRACEBUF", &getutil_info.TypeTraceBuf ) != 0 )
  {
    printf( "sniffrings: Invalid message type <TYPE_TRACEBUF>; exiting!\n" );
    return -1;
  }
  if ( GetType( "TYPE_TRACEBUF2", &getutil_info.TypeTraceBuf2 ) != 0 )
  {
    printf( "sniffrings: Invalid message type <TYPE_TRACEBUF2>; exiting!\n" );
    return -1;
  }
  if ( GetType( "TYPE_TRACE_COMP_UA", &getutil_info.TypeCompUA ) != 0 )
  {
    printf( "sniffrings: Invalid message type <TYPE_TRACE_COMP_UA>; exiting!\n" );
    return -1;
  }
  if ( GetType( "TYPE_TRACE2_COMP_UA", &getutil_info.TypeCompUA2 ) != 0 )
  {
    printf( "sniffrings: Invalid message type <TYPE_TRACE2_COMP_UA>; exiting!\n" );
    return -1;
  }

  if ((argc == 2) || (argc == 3) || (argc == 4))
  {
    /* Initialize args_stacker.GetLogo to all wildcards (get any message)
     ******************************************************/
    for(r=0; r < nRings; r++) {
      args_stacker[r].GetLogo[0].type   = getutil_info.TypeWildCard;
      args_stacker[r].GetLogo[0].mod    = getutil_info.ModWildCard;
      args_stacker[r].GetLogo[0].instid = getutil_info.InstWildCard;
    }
  }
  if ( ( !no_flush && ((argc == 2) || (argc == 5)) ) || (no_flush && (argc == 3 || argc == 6))) {
    args_printer.verbose = 0;
  }
  if ((argc == 5+no_flush) || (argc == 6+no_flush))  {
    if (GetInst (argv[2+no_flush], &getutil_info.InstToGet) != 0)
    {
      printf( "sniffrings: Invalid installation name %s", argv[2]);
      return -1;
    }
    if (GetModId (argv[3+no_flush], &getutil_info.ModToGet) != 0)
    {
      printf( "sniffrings: Invalid module name %s", argv[3]);
      return -1;
    }
    if (GetType (argv[4+no_flush], &getutil_info.TypeToGet) != 0)
    {
      printf( "sniffrings: Invalid message type name %s", argv[4]);
      return -1;
    }
    for(r=0; r < nRings; r++) {
      args_stacker[r].GetLogo[0].type   = getutil_info.TypeToGet;
      args_stacker[r].GetLogo[0].mod    = getutil_info.ModToGet;
      args_stacker[r].GetLogo[0].instid = getutil_info.InstToGet;
    }
  }

  /* Set other variables in args_stacker
   ********************************************/
  for(r=0; r < nRings; r++) {
    args_stacker[r].getutil_info = &getutil_info;
  }

  /* Set variables in args_printer
   ********************************************/
  args_printer.getutil_info = &getutil_info;

  /* Create Mutexes
   ********************************************/
  CreateSpecificMutex(&mutex_nStackerThread);
  CreateSpecificMutex(&mutex_OutQueue);
  CreateSpecificMutex(&mutex_flags_term);

  /* Initialize the message queue
   *******************************/
  RequestSpecificMutex(&mutex_OutQueue);
  initqueue( &OutQueue, (unsigned long)QueueSize,(unsigned long)(MaxMsgSize+1));
  ReleaseSpecificMutex(&mutex_OutQueue);

  /* Start the message printer thread
   ***********************************/
  if ( StartThreadWithArg(  MessagePrinter, (void *) &args_printer, (unsigned)THREAD_STACK, &tidMessagePrinter ) == -1 )
  {
    printf("%s: Error starting MessagePrinter thread; exiting!\n",
	cmdname );
    exit( -1 );
  }

  /* Start the message stacking threads
   ****************************************************************/
  for(r=0; r < nRings; r++) {
    inc_nStackerThread();
  }
  for(r=0; r < nRings; r++) {
    if ( StartThreadWithArg(  MessageStacker, (void *) &args_stacker[r], (unsigned)THREAD_STACK, &(tidStacker[r]) ) == -1 )
    {
      printf("%s: Error starting MessageStacker thread on ring %s; skipping!\n",
	  cmdname, args_stacker[r].InRing );
      exit( -1 );
    }
  }

  /* If self-termination requested, set the time to do so
   ******************************************************/
  if ( expiration_date > 0 ) {
     time(&now);
     expiration_date += now;
  }

  /* Start main sniffrings service loop, which aimlessly beats its heart.
   **********************************/
  while ( !testlck_flags_term(FLAG_TERM_THR_PRINTER) && !testlck_flags_term(FLAG_TERM_THR_STACKER ) )
  {
    /* Beat the heart into the transport ring
     ****************************************/
    time(&now);
    /*
       if (difftime(now,MyLastBeat) > (double)HeartBeatInt )
       {
       snprintf( errText, STR_LEN_ERRTEXT-1, "%ld %ld\n%c", (long) now, (long) MyPid,0);
       sniffrings_status( &HeartLogo, errText);
       MyLastBeat = now;
       }
       */

    /* Check for self-termination
     *****************************/
    if ( expiration_date && (now > expiration_date) ) {
        printf("%s: self-termination requested.\n", cmdname);
        break;
    }

    /* take a brief nap
     ************************************/
    sleep_ew(100);
  } /*end while of monitoring loop */

  printf("%s: termination requested; waiting for all threads (max. %d sec.)!\n",
      cmdname, wait_time_thr_tot/1000 );

  /* Waiting for termination of other threads before exiting */
  setlck_flags_term(FLAG_TERM_REQ);
  wait_time_thr_count=0;
  while( wait_time_thr_count < wait_time_thr_tot
      && !testlck_flags_term(FLAG_TERM_THR_PRINTER | FLAG_TERM_THR_STACKER) ) {
		sleep_ew(wait_time_thr_slice);

	/*
	 * Unlock possible situation when MessagePrinter is waiting for new item
	 * from an empty queue. It is exexcuted only one time.
	 */
		if(wait_time_thr_count==0 && !testlck_flags_term(FLAG_TERM_THR_PRINTER)) {
		POST_SPECIFIC_SEMAPHORE_EW(sema_CountBusy);
		}

		wait_time_thr_count+=wait_time_thr_slice;
  }

  if(!testlck_flags_term(FLAG_TERM_THR_PRINTER)) {
    printf("Warning: thread MessagePrinter not terminated yet!\n");
  }
  if(!testlck_flags_term(FLAG_TERM_THR_STACKER)) {
    printf("Warning: thread MessageStacker not terminated yet!\n");
  }

  /* Destroy Semaphores
   ********************************************/
  DESTROY_SPECIFIC_SEMAPHORE_EW(sema_CountAvail);
  DESTROY_SPECIFIC_SEMAPHORE_EW(sema_CountBusy);

  /* Deallocate memory from OutQueue */
  RequestSpecificMutex(&mutex_OutQueue);
  freequeue(&OutQueue);
  ReleaseSpecificMutex(&mutex_OutQueue);

  /* Destroy Mutexes
   ********************************************/
  CloseSpecificMutex(&mutex_nStackerThread);
  CloseSpecificMutex(&mutex_OutQueue);
  CloseSpecificMutex(&mutex_flags_term);

  return 0;
}



/********************** Message Stacking Thread *******************
 *           Move messages from transport to memory queue         *
 ******************************************************************/
thr_ret MessageStacker( void *p )
{
  TYPE_ARGS_STACKER *args_stacker = (TYPE_ARGS_STACKER *) p;
  SHM_INFO    InRegion;
  long        inkey;             /* Key to input ring            */
  long       recsize;	/* size of retrieved message             */
  MSG_LOGO   reclogo;       /* logo of retrieved message             */
  unsigned char inseq = 0;        /* transport seq# in input ring */
  time_t     now;
  int        ret;
  int        error_occurred = 0;
  int        NumOfTimesQueueLapped= 0; /* number of messages lost due to
					  queue lap */
  /* Message Buffers to be allocated
   *********************************/
  char       *msgb = NULL;           /* msg retrieved from transport      */
  char       errText[STR_LEN_ERRTEXT]; /* string for log/error/heartbeat messages */

  /* set last character to zero */
  errText[STR_LEN_ERRTEXT-1]=0;

  /* Look up transport region keys earthworm.h tables
   ************************************************/
  if( ( inkey = GetKey(args_stacker->InRing) ) == -1 )
  {
    printf( "sniffrings: Invalid input ring name <%s>; skipping these ring!\n",
	args_stacker->InRing );
    error_occurred = 1;
  } else {
    printf( "sniffrings: reading messages from input ring name <%s>.\n",
	args_stacker->InRing );
  }

  if(!error_occurred) {

    /* Allocate space for messages buffer
     ***********************************************************/
    /* Buffer for Read thread: */
    if ( ( msgb = (char *) malloc(MaxMsgSize+1) ) ==  NULL )
    {
      printf("%s: error allocating Rawmsg; exiting!\n",
	  args_stacker->cmdname );
      exit( -1 );
    }

    /* Attach to input and output transport rings
     ******************************************/
    tport_attach( &InRegion,  inkey );

    /* Flush all old messages from the ring
     ************************************/
    if (!args_stacker->no_flush) {
      while( tport_copyfrom( &InRegion, args_stacker->GetLogo, args_stacker->nLogo, &reclogo,
	    &recsize, msgb, MaxMsgSize, &inseq ) != GET_NONE );
    }

    /* Start main service loop for current connection
     ************************************************/
    while( !testlck_flags_term(FLAG_TERM_REQ) && !error_occurred)
    {
      /* Get a message from transport ring
       ************************************/
      ret = tport_copyfrom( &InRegion, args_stacker->GetLogo, args_stacker->nLogo,
	  &reclogo, &recsize, msgb, MaxMsgSize, &inseq );

      switch (ret) {
	case GET_NONE:
	  /* Wait if no messages for us */
	  sleep_ew(100);
	  continue;
	  break;
	case GET_NOTRACK:
	  time(&now);
	  snprintf( errText, STR_LEN_ERRTEXT-1, "%ld %d no tracking for logo i%d m%d t%d in %s",
	      (long)now, ERR_NOTRACK, (int) reclogo.instid, (int) reclogo.mod,
	      (int)reclogo.type, args_stacker->InRing );
	  printf("%s\n", errText);
	  break;
	case GET_MISS_LAPPED:
	  time(&now);
	  snprintf( errText, STR_LEN_ERRTEXT-1, "%ld %d msg(s) overwritten i%d m%d t%d in %s",
	      (long)now, ERR_MISSMSG, (int) reclogo.instid,
	      (int) reclogo.mod, (int)reclogo.type, args_stacker->InRing );
	  printf("%s\n", errText);
	  break;
	case GET_MISS_SEQGAP:
	  time(&now);
	  snprintf( errText, STR_LEN_ERRTEXT-1, "%ld %d gap in msg sequence i%d m%d t%d in %s",
	      (long)now, ERR_MISSMSG, (int) reclogo.instid,
	      (int) reclogo.mod, (int)reclogo.type, args_stacker->InRing );
	  printf("%s\n", errText);
	  break;
	/* only for tport_getmsg()
	case GET_MISS:
	  time(&now);
	  snprintf( errText, STR_LEN_ERRTEXT-1, "%ld %hd missed msg(s) i%d m%d t%d in %s",
	      now, ERR_MISSMSG, (int) reclogo.instid,
	      (int) reclogo.mod, (int)reclogo.type, args_stacker->InRing );
	  printf("%s\n", errText);
	  break;
	  */
	case GET_TOOBIG:
	  time(&now);
	  snprintf( errText, STR_LEN_ERRTEXT-1, "%ld %d msg[%ld] i%d m%d t%d too long for target",
	      (long)now, ERR_TOOBIG, recsize, (int) reclogo.instid,
	      (int) reclogo.mod, (int)reclogo.type );
	  printf("%s\n", errText);
	  continue;
	  break;
      }


      /* Process retrieved msg (ret==GET_OK,GET_MISS,GET_NOTRACK)
       ***********************************************************/
      msgb[recsize] = '\0';

      /* put the message into the queue */
      WAIT_SPECIFIC_SEMAPHORE_EW(sema_CountAvail);
      RequestSpecificMutex(&mutex_OutQueue);
      ret=enqueuering( &OutQueue, msgb, recsize, reclogo, inkey, inseq );
      ReleaseSpecificMutex(&mutex_OutQueue);
      POST_SPECIFIC_SEMAPHORE_EW(sema_CountBusy);

      switch(ret) {
	case -2:
	  /* Serious: quit */
	  /* Currently, enqeueue() in mem_circ_queue.c never returns this error. */
	  time(&now);
	  snprintf(errText, STR_LEN_ERRTEXT-1,"%ld %d internal queue error. Terminating.", (long)now, ERR_QUEUE);
	  printf("%s\n", errText);
	  error_occurred = 1;
	  break;
	case -1:
	  time(&now);
	  snprintf(errText, STR_LEN_ERRTEXT-1,"%ld %d message too big for queue.", (long)now,
	      ERR_QUEUE);
	  printf("%s\n", errText);
	  continue;
	  break;
	case -3:
	  NumOfTimesQueueLapped++;
	  if (!(NumOfTimesQueueLapped % 5)) {
	    printf("%s: Circular queue lapped 5 times. Messages lost.\n",
		args_stacker->cmdname);
	    if (!(NumOfTimesQueueLapped % 100)) {
	      printf("%s: Circular queue lapped 100 times. Messages lost.\n",
		  args_stacker->cmdname);
	    }
	  }
	  continue;
      }
    } /* end of while */

    /* Detach from shared memory regions and terminate
     ***********************************************/
    tport_detach( &InRegion );

    /* Deallocate space of messages buffer
     ***********************************************************/
    free(msgb);
    msgb=NULL;

  }

  /* we're quitting
   *****************/
  dec_nStackerThread();
  KillSelfThread(); /* main thread will not restart us */
  return THR_NULL_RET; /* Should never get here */
}

#define MAX_LINE_OUT MAX_BYTES_PER_EQ + 1024

/************* Main Thread for populating DB from queue  ***************
 *          Pull a messsage from the queue, and call stored procedure  *
 *          for inserting information in to the database               *
 **********************************************************************/
thr_ret MessagePrinter( void *p )
{
  TYPE_ARGS_PRINTER *args_printer = (TYPE_ARGS_PRINTER *) p;
  MSG_LOGO reclogo;
  int      ret;
  long     msgSize;
  unsigned char inseq = 0;        /* transport seq# in input ring */
  long     inkey;
  char     *RingName;
  char     ModName[MAX_MOD_STR+1];
  char     InstName[MAX_INST_STR+1];
  char     TypeName[MAX_TYPE_STR+1];

  int      i;
  int      count_cr;

  char    *Wrmsg = NULL;           /* message to get from queue         */
  char    errText[STR_LEN_ERRTEXT];

  char    line_out[MAX_LINE_OUT];
  line_out[MAX_LINE_OUT-1] = 0;

  /* set last character to zero */
  errText[STR_LEN_ERRTEXT-1]=0;

  /* Allocate buffer for reading message from queue  */
  if ( ( Wrmsg = (char *) malloc(MaxMsgSize+1) ) ==  NULL )
  {
    printf("%s: error allocating Wrmsg in MessagePrinter; exiting!\n",
	args_printer->cmdname );
    exit( -1 );
  }

  while (!testlck_flags_term(FLAG_TERM_REQ)) {   /* main loop */

    /* Get message from queue
     *************************/
    WAIT_SPECIFIC_SEMAPHORE_EW(sema_CountBusy);
    RequestSpecificMutex(&mutex_OutQueue);
    ret=dequeuering( &OutQueue, Wrmsg, &msgSize, &reclogo, &inkey, &inseq);
    ReleaseSpecificMutex(&mutex_OutQueue);
    POST_SPECIFIC_SEMAPHORE_EW(sema_CountAvail);

    /* During the main cycle, it does not occur. */
    if (ret < 0 )
    { /* -1 means empty queue */
      sleep_ew(100);
      continue;
    }

    /***************************************************************************************************/

    /* Grab next message from the ring
     *******************************/
    // rc = tport_copyfrom( &InRegion, getlogo, (short)1, &reclogo, &msgSize, Wrmsg, MAX_SIZE, &inseq );

    if (args_printer->verbose) {
      if (args_printer->getutil_info->InstId == reclogo.instid && GetModIdName(reclogo.mod) != NULL) {
	strcpy(ModName, GetModIdName(reclogo.mod));
      } else {
	sprintf( ModName, "UnknownRemoteMod:%d", reclogo.mod);
      }
      if (GetInstName(reclogo.instid)==NULL) {
	sprintf(InstName, "UnknownInstID:%d", reclogo.instid);
      } else {
	strcpy(InstName, GetInstName(reclogo.instid));
      }
      if (GetTypeName(reclogo.type)==NULL) {
	sprintf(TypeName, "UnknownRemoteType:%d", reclogo.type);
      } else {
	strcpy(TypeName, GetTypeName(reclogo.type));
      }
      RingName = GetKeyName(inkey);
      /* Print message logo names, etc. to the screen
       ****************************************/
      snprintf( line_out, MAX_LINE_OUT-1, "%d %s %s %s %s <seq:%3d> <Length:%6ld>",
	  (int)time (NULL), (RingName)? RingName : "RING_UNKNOWN", InstName, ModName, TypeName, (int)inseq, msgSize );

    } else {
      /* Backward compatibility */
      /* Print message logo, etc. to the screen
       ****************************************/
      snprintf( line_out, MAX_LINE_OUT-1, "%d <ring:%ld> <inst:%3d> <mod:%3d> <type:%3d> <seq:%3d> <Length:%6ld>",
	  (int)time (NULL),  inkey, (int)reclogo.instid, (int)reclogo.mod, (int)reclogo.type, (int)inseq, msgSize );

    }

    /* Print any non-binary (non-waveform) message to screen
     *******************************************************/
    if( reclogo.type == args_printer->getutil_info->TypeTraceBuf2
	|| reclogo.type == args_printer->getutil_info->TypeCompUA2
	|| reclogo.type == args_printer->getutil_info->TypeTraceBuf
	|| reclogo.type == args_printer->getutil_info->TypeCompUA ) {

      strncat(line_out, "\n", MAX_LINE_OUT- strlen(line_out) - 1);

    } else {

      count_cr = 0;
      for(i=0; i < msgSize - 1; i++) {
	if(Wrmsg[i] == '\n') {
	  count_cr++;
	}
      }
      if(count_cr > 1) {
	strncat(line_out, "\n", MAX_LINE_OUT- strlen(line_out) - 1);
      } else {
	strncat(line_out, " ", MAX_LINE_OUT- strlen(line_out) - 1);
      }
      Wrmsg[msgSize] = '\0'; /* Null-terminate message */
      strncat(line_out, Wrmsg, MAX_LINE_OUT- strlen(line_out) - 1);
      if(Wrmsg[msgSize - 1] != '\n') {
	strncat(line_out, "\n", MAX_LINE_OUT- strlen(line_out) - 1);
      }

    }

    printf( "%s", line_out );
    fflush( stdout );

  }   /* End of main loop */


  /* we're quitting
   *****************/
  setlck_flags_term(FLAG_TERM_THR_PRINTER);
  KillSelfThread(); /* main thread will not restart us */
  return THR_NULL_RET; /* Should never get here */
}


/* set flags_bitmap by lock mechanism
 ***************************************************/
void       setlck_flags_term(int flags_bitmap) {
  RequestSpecificMutex(&mutex_flags_term);
  flags_term = flags_term | flags_bitmap;
  ReleaseSpecificMutex(&mutex_flags_term);
}


/* test flags_bitmap by lock mechanism
 ***************************************************/
int       testlck_flags_term(int flags_bitmap) {
  int ret = 0;
  RequestSpecificMutex(&mutex_flags_term);
  if(flags_bitmap == (flags_term & flags_bitmap)) {
    ret = 1;
  }
  ReleaseSpecificMutex(&mutex_flags_term);
  return ret;
}


/* increment the number of running Stacker Threads
 ***************************************************/
void inc_nStackerThread() {
  RequestSpecificMutex(&mutex_nStackerThread);
  nStackerRunning++;
  ReleaseSpecificMutex(&mutex_nStackerThread);
}

/* decrement the number of running Stacker Threads and
 * in case they are zero set the flag FLAG_TERM_THR_STACKER
 ***************************************************/
void dec_nStackerThread() {
  RequestSpecificMutex(&mutex_nStackerThread);
  nStackerRunning--;
  if(nStackerRunning <= 0) {
    setlck_flags_term(FLAG_TERM_THR_STACKER);
  }
  ReleaseSpecificMutex(&mutex_nStackerThread);
}


/* Explode Ring list into an array and return the
 * number of ring names read
 ***************************************************/
int explode_ringlist(char *s, char ringNames[MAX_NUM_OF_RINGS][MAX_RING_STR]) {
  int ret = 0;
  int i = 0;
  int r = 0; /* Number of rings */
  int k = 0;
  int l = 0;

  if(s) {
    l = (int)strlen(s);
    i = 0;
    while(i < l  &&  r < MAX_NUM_OF_RINGS) {
      if(s[i] == ',') {
	ringNames[r][k] = 0;
	r++;
	k=0;
      } else {
	ringNames[r][k] = s[i];
	k++;
      }
      i++;
    }

    /* Have no space more than MAX_NUM_OF_RINGS */
    if(i < l  &&  r >= MAX_NUM_OF_RINGS) {
      printf( "sniffrings: warning only %d rings are allowed!\n", MAX_NUM_OF_RINGS);
    } else {
      /* NULL terminated for the last ring name */
      ringNames[r][k] = 0;
      if(k > 0) {
	r++;
      }
    }
    ret = r;
  }

  return ret;
}

/* Remove duplicated entries in ringNames
 ***************************************************/
int remove_duplicated_ringname(char ringNames[MAX_NUM_OF_RINGS][MAX_RING_STR], int *nRings) {
  int ret = 0;
  int r = 0;
  int k = 0;
  int j = 0;

  r = 0;
  while(r < *nRings) {
    k = r+1;

    while(k < *nRings) {
      if(strcmp(ringNames[r], ringNames[k]) == 0) {
	printf( "sniffrings: warning ring name %s duplicated!\n", ringNames[r]);
	/* override the string at k position */
	for(j=k; j < *nRings - 1; j++) {
	  strncpy(ringNames[j], ringNames[j+1], MAX_RING_STR-1);
	}
	*nRings = *nRings - 1;
      } else {
	k++;
      }
    }

    r++;
  }

  return ret;
}
