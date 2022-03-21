
  /********************************************************************
   *                                                       10/1998    *
   *                           transport.c                            *
   *                                                                  *
   *   Transport layer functions to access shared memory regions.     *
   *                                                                  *
   *   written by Lynn Dietz, Will Kohler with inspiration from       *
   *       Carl Johnson, Alex Bittenbinder, Barbara Bogaert           *
   *                                                                  *
   ********************************************************************/

/* For semtimedop() in sys/sem.h */
#ifndef _USE_POSIX_SHM
# undef  _GNU_SOURCE
# define _GNU_SOURCE 1
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <time.h>

#ifdef _USE_POSIX_SHM
# include <fcntl.h>
# include <sys/mman.h>
# include <sys/stat.h>
# include <semaphore.h>
#else
# include <sys/ipc.h>
# include <sys/shm.h>
# include <sys/sem.h>
#endif

#ifdef _USE_PTHREADS
# include <pthread.h>
#else
# include <thread.h>
#endif

#include <earthworm.h>
#include "platform.h"
#include "transport.h"

static short Copyfrom_Init=1;      /* initialization flag */
static short Flag_Init=1;          /* initialization flag */
/* */
static SHM_INFO smf_region;
static long shm_flag_key;
/* */
#define SHM_DEFAULT_MASK     0664
/* */
#define SHM_FLAG_DEFAULT_KEY 9999
#define SHM_FLAG_RING "FLAG_RING"
/* */
#define FF_REMOVE 1
#define FF_FLAG2ADD 2
#define FF_FLAG2DIE 3
#define FF_GETFLAG 4
#define FF_CLASSIFY 5
/* */
#define FLAG_LOCK_TRIES 3
#define RING_LOCK_TRIES 3

/* These functions are for internal use by transport functions only */
static SHM_HEAD *create_shm_region( int *, const long, const long );
static void close_shm_region( SHM_INFO * );
static void destroy_semaphore( SHM_INFO * );
static SHM_HEAD *attach_shm_region( int *, const long );
static void detach_shm_region( SHM_INFO * );
#ifdef _USE_POSIX_SHM
static sem_t *create_semaphore( const long );
static sem_t *get_semaphore( const long );
#else
static int   create_semaphore( const long );
static int   get_semaphore( const long );
#endif
static MSG_TRACK *search_track_in_list( MSG_TRACK [], int *, const int, const MSG_TRACK * );
static RING_INDEX_T find_latest_keyget(
	const SHM_HEAD *, MSG_TRACK [], const long , const MSG_LOGO [], const long, const long, RING_INDEX_T *
);
static void update_track_list_keyout(
	MSG_TRACK [], const long, const MSG_LOGO [], const long, const long, const RING_INDEX_T
);
/* */
static int  move_keyold_2_nextmsg( SHM_HEAD * );
static RING_INDEX_T copy_msg_2_shm( SHM_HEAD *, RING_INDEX_T, const void *, const size_t );
static RING_INDEX_T copy_shmmsg_2_buf( const SHM_HEAD *, RING_INDEX_T, void *, const size_t );
static void reset_key_shm( SHM_HEAD * );
static int  track_getmsg_seq( const MSG_TRACK *, MSG_TRACK [], int *, int );
static int  compare_msg_track( const void *, const void * );
static int  compare_msg_track_wildlogo( const void *, const void * );

static void tport_syserr( const char *, const long );
static void tport_buferror( short, char * );
static int  tport_doFlagOp( SHM_INFO* region, int pid, int op );

#define COMPARE_MSG_TRACK(TRACK_A, TRACK_B) \
		(!((TRACK_A)->memkey == (TRACK_B)->memkey && \
		(TRACK_A)->logo.type == (TRACK_B)->logo.type && \
		(TRACK_A)->logo.mod == (TRACK_B)->logo.mod && \
		(TRACK_A)->logo.instid == (TRACK_B)->logo.instid))

#define COMPARE_MSG_LOGO(LOGO_A, LOGO_B) \
		(!(((LOGO_A)->type == (LOGO_B)->type || (LOGO_A)->type == WILD) && \
		((LOGO_A)->mod == (LOGO_B)->mod || (LOGO_A)->mod == WILD) && \
		((LOGO_A)->instid == (LOGO_B)->instid || (LOGO_A)->instid == WILD)))

/*
 * These statements and variables are required by the functions of
 * the input-buffering thread
 */
static volatile SHM_INFO *PubRegion;      /* transport public ring      */
static volatile SHM_INFO *BufRegion;      /* pointer to private ring    */
static volatile MSG_LOGO *Getlogo;        /* array of logos to copy     */
static volatile int16_t   Nget;           /* number of logos in getlogo */
static volatile uint32_t  MaxMsgSize;     /* size of message buffer     */
static volatile char     *Message;        /* message buffer             */
static uint8_t            MyModuleId;     /* module id of main thread   */
static uint8_t            MyInstid;       /* inst id of main thread     */
static uint8_t            TypeError;      /* type for error messages    */

/*
 * tport_create() - Create a shared memory region & its semaphore,
 *                  attach to it and initialize header values.
 * Arguments:
 *   info structure for memory region
 *   size of shared memory region
 *   key to shared memory region
 */
void tport_create( SHM_INFO *region, long nbytes, long memkey )
{
	int       regid = 0;                                            /* shared memory region identifier   */
	SHM_HEAD *shm   = create_shm_region( &regid, nbytes, memkey );  /* pointer to start of memory region */

/* Initialize shared memory region header */
   shm->nbytes = nbytes;
   shm->keymax = nbytes - sizeof(SHM_HEAD);
   shm->keyin  = 0;   /* cosmetic change 980428:ldd */
   shm->keyold = shm->keyin;
   shm->flag   = 0;
/* set values in the shared memory information structure */
   region->addr = shm;
   region->mid  = regid;
   region->sid  = create_semaphore( memkey );
   region->key  = memkey;

   return;
}

/*
 * tport_destroy() - Destroy a shared memory region.
 * Arguments:
 */
void tport_destroy( SHM_INFO *region )
{
/* Close and delete shared memory region */
	close_shm_region( region );
/* Close and delete semaphore */
	destroy_semaphore( region );

	return;
}


/*
 * tport_attach() - Map to an existing shared memory region.
 * Arguments:
 *   info structure for memory region
 *   key to shared memory region
 */
void tport_attach( SHM_INFO *region, long memkey )
{
	int       regid;                                      /* shared memory file descriptor   */
	SHM_HEAD *shm = attach_shm_region( &regid, memkey );  /* pointer to start of memory region */

/* Set values in the shared memory information structure */
   region->addr = shm;
   region->mid  = regid;
   region->sid  = get_semaphore( memkey );
   region->key  = memkey;
/* Attach to flag if necessary; add our pid to flag */
	if ( Flag_Init ) {
		Flag_Init = 0;
		shm_flag_key = GetKeyWithDefault( SHM_FLAG_RING, SHM_FLAG_DEFAULT_KEY );
		tport_attach( &smf_region, shm_flag_key );
	}
	tport_addToFlag( region, getpid() );

   return;
}

/*
 * tport_detach() - Detach from a shared memory region.
 */
void tport_detach( SHM_INFO *region )
{
	detach_shm_region( region );
	return;
}

/*
 * tport_putmsg() - Put a message into a shared memory region.
 *                  Assigns a transport-layer sequence number.
 * Arguments:
 *   info structure for memory region
 *   type,module,instid of incoming msg
 *   size of incoming message
 *   pointer to incoming message
 */
int tport_putmsg( SHM_INFO *region, MSG_LOGO *putlogo, long length, char *msg )
{
	static MSG_TRACK trak[NTRACK_PUT] = { { 0, { 0 }, 0, 0, 0 } };  /* sequence number keeper   */
	static int       ntrak = 0;                   /* # of logos seen so far   */
#ifndef _USE_POSIX_SHM
	struct sembuf sops;                           /* semaphore operations; changed to non-static 980424:ldd */
	int           tries_left;
	int           res;
#endif
	SHM_HEAD    *shm      = region->addr;           /* pointer to start of memory region   */
	MSG_TRACK    trak_in  = { 0, { 0 }, 0, 0, 0 };  /* transport layer header to put       */
	MSG_TRACK   *trak_ptr = NULL;                   /* transport layer header to put       */
	TPORT_HEAD   thead;                             /* transport layer header to put       */
	RING_INDEX_T _keyin   = 0;                      /* pointer to start of memory region   */

	const long size_tporth = sizeof(TPORT_HEAD);

/* First, see if the incoming message will fit in the memory region */
	if ( length + size_tporth > shm->keymax ) {
		fprintf(stdout, "ERROR: tport_putmsg; message too large (%ld) for Region %ld\n", length, region->key);
		return PUT_TOOBIG;
	}
/* Change semaphore; let others know you're using tracking structure & memory */
#ifdef _USE_POSIX_SHM
	if ( sem_wait(region->sid) == -1 )
		tport_syserr( "tport_putmsg sem_wait ->inuse", region->key );
#else
	sops.sem_num = 0;   /* moved outside Put_Init loop 980424:ldd */
	sops.sem_flg = 0;   /* moved outside Put_Init loop 980424:ldd */
	sops.sem_op  = SHM_INUSE;
	tries_left   = RING_LOCK_TRIES;
	while ( tries_left > 0 && (res = semop( region->sid, &sops, 1 )) == -1 ) {
		if ( errno != EINTR )
			break;
		tries_left--;
	}
	if ( res == -1 )
		tport_syserr( "tport_putmsg semop ->inuse", region->key );
#endif

/* Next, find incoming logo in list of combinations already seen */
	trak_in.memkey = region->key;
	trak_in.logo   = *putlogo;
	trak_ptr = search_track_in_list( trak, &ntrak, NTRACK_PUT, &trak_in );
/* */
	if ( !trak_ptr ) {
		fprintf(stdout, "ERROR: tport_putmsg; exceeded NTRACK_PUT, msg not sent\n");
		return PUT_NOTRACK;
	}
/* Store everything you need in the transport header */
	thead.start = FIRST_BYTE;
	thead.size  = length;
	thead.logo  = trak_ptr->logo;
	thead.seq   = trak_ptr->seq++;

/* In shared memory, see if keyin will wrap; if so, reset keyin and keyold */
	if ( (RING_INDEX_T)(shm->keyin + size_tporth + length) < shm->keyold ) {
		reset_key_shm( shm );
		/*
		 * fprintf(stdout, "NOTICE: tport_putmsg; keyin wrapped & reset; Region %ld\n", region->key );
		 */
	}

/*
 * Then see if there's enough room for new message in shared memory
 * If not, "delete" oldest messages until there's room
 */
	while ( (RING_INDEX_T)(shm->keyin + size_tporth + length - shm->keyold) > shm->keymax ) {
		if ( move_keyold_2_nextmsg( shm ) ) {
			fprintf(stdout, "ERROR: tport_putmsg; keyold not at FIRST_BYTE, Region %ld\n", region->key);
			exit(1);
		}
	}

/* Now copy transport header into shared memory by chunks... */
	_keyin = copy_msg_2_shm( shm, shm->keyin, &thead, size_tporth );
/* ...and copy message into shared memory by chunks */
	_keyin = copy_msg_2_shm( shm, _keyin, msg, length );
/* */
	shm->keyin += size_tporth + length;

/* Finished with shared memory, let others know via semaphore */
#ifdef _USE_POSIX_SHM
	if ( sem_post(region->sid) == -1 )
		tport_syserr( "tport_putmsg sem_post ->inuse", region->key );
#else
	sops.sem_op = SHM_FREE;
	if ( semop(region->sid, &sops, 1) == -1 )
		tport_syserr( "tport_putmsg semop ->free", region->key );
#endif

   return PUT_OK;
}



/*
 * tport_copyto() - Puts a message into a shared memory region. Preserves the sequence number
 *                  (passed as argument) as the transport layer sequence number.
 *
 * Arguments:
 *   info structure for memory region
 *   type,module,instid of incoming msg.
 *   size of incoming message
 *   pointer to incoming message.
 *   preserve as sequence# in TPORT_HEAD
 */
int tport_copyto( SHM_INFO *region, MSG_LOGO *putlogo, long length, char *msg, unsigned char seq )
{
#ifndef _USE_POSIX_SHM
	struct sembuf sops;                           /* semaphore operations; changed to non-static 980424:ldd */
	int           tries_left;
	int           res;
#endif
	SHM_HEAD    *shm    = region->addr;           /* pointer to start of memory region   */
	RING_INDEX_T _keyin = 0;                      /* pointer to start of memory region   */
	TPORT_HEAD   thead;                             /* transport layer header to put       */

	const long size_tporth = sizeof(TPORT_HEAD);

/* First, see if the incoming message will fit in the memory region */
	if ( length + size_tporth > shm->keymax ) {
		fprintf(stdout, "ERROR: tport_copyto; message too large (%ld) for Region %ld\n", length, region->key);
		return PUT_TOOBIG;
	}

/* Store everything you need in the transport header */
	thead.start = FIRST_BYTE;
	thead.size  = length;
	thead.logo  = *putlogo;
	thead.seq   = seq;

/* Change semaphore; let others know you're using tracking structure & memory */
#ifdef _USE_POSIX_SHM
	if ( sem_wait(region->sid) == -1 )
		tport_syserr( "tport_copyto sem_wait ->inuse", region->key );
#else
	sops.sem_num = 0;
	sops.sem_flg = 0;
	sops.sem_op  = SHM_INUSE;
	tries_left   = RING_LOCK_TRIES;
	while ( tries_left > 0 && (res = semop( region->sid, &sops, 1 )) == -1 ) {
		if ( errno != EINTR )
			break;
		tries_left--;
	}
	if ( res == -1 )
		tport_syserr( "tport_copyto semop ->inuse", region->key );
#endif

/* First see if keyin will wrap; if so, reset both keyin and keyold */
	if ( (RING_INDEX_T)(shm->keyin + size_tporth + length) < shm->keyold ) {
		reset_key_shm( shm );
		/*
		 * fprintf(stdout, "NOTICE: tport_copyto; keyin wrapped & reset; Region %ld\n", region->key );
		 */
	 }

/*
 * Then see if there's enough room for new message in shared memory
 * If not, "delete" oldest messages until there's room
 */
	while ( (RING_INDEX_T)(shm->keyin + size_tporth + length - shm->keyold) > shm->keymax ) {
		if ( move_keyold_2_nextmsg( shm ) ) {
			fprintf(stdout, "ERROR: tport_copyto; keyold not at FIRST_BYTE, Region %ld\n", region->key);
			exit(1);
		}
	}

/* Now copy transport header into shared memory by chunks... */
	_keyin = copy_msg_2_shm( shm, shm->keyin, &thead, size_tporth );
/* ...and copy message into shared memory by chunks */
	_keyin = copy_msg_2_shm( shm, _keyin, msg, length );
/* */
	shm->keyin += size_tporth + length;

/* Finished with shared memory, let others know via semaphore */
#ifdef _USE_POSIX_SHM
	if ( sem_post(region->sid) == -1 )
		tport_syserr( "tport_copyto sem_post ->inuse", region->key );
#else
	sops.sem_op = SHM_FREE;
	if ( semop(region->sid, &sops, 1) == -1 )
		tport_syserr( "tport_copyto semop ->free", region->key );
#endif

   return PUT_OK;
}

/*
 * tport_getmsg() - Find (and possibly get) a message out of shared memory.
 * Arguments:
 *   info structure for memory region
 *   requested logo(s)
 *   number of logos in getlogo
 *   logo of retrieved msg
 *   size of retrieved message
 *   retrieved message (may be NULL)
 *   max length for retrieved message
 */
int tport_getmsg(
	SHM_INFO *region, MSG_LOGO *getlogo, short nget, MSG_LOGO *logo, long *length, char *msg, long maxsize
) {
	static MSG_TRACK trak[NTRACK_GET] = { { 0, { 0 }, 0, 0, 0 } };  /* sequence #, outpointer keeper     */
	static int       ntrak = 0;                                     /* # modid,type,instid combos so far */
/* */
	const SHM_HEAD  *shm = region->addr;               /* pointer to start of memory region */
	TPORT_HEAD       thead;                            /* transport header from memory      */
	MSG_TRACK        trak_in = { 0, { 0 }, 0, 0, 0 };  /* sequence #, outpointer keeper     */
	RING_INDEX_T     keyin;                            /* in-pointer to shared memory       */
	RING_INDEX_T     keyold;                           /* oldest complete message in memory */
	RING_INDEX_T     keyget;                           /* pointer at which to start search  */
	int              status = GET_NONE;                /* how did retrieval go?             */
	int              i;

/* Make sure all requested logos are entered in tracking list */
	trak_in.memkey = region->key;
/* for all logos we're tracking */
	for ( i = 0; i < nget; i++ ) {
		trak_in.logo = getlogo[i];
		search_track_in_list( trak, &ntrak, NTRACK_GET, &trak_in );
	}
/* Find latest starting index and in-index to look for any of the requested logos */
	keyget = find_latest_keyget( shm, trak, ntrak, getlogo, nget, region->key, &keyin );

/* Find next message from requested type, module, instid */
	while ( keyget < keyin ) {
	/* Make sure you haven't been lapped by tport_putmsg */
		if ( keyget < shm->keyold )
			keyget = shm->keyold;
	/* Load next header; make sure you weren't lapped */
		copy_shmmsg_2_buf( shm, keyget, &thead, sizeof(TPORT_HEAD) );
	/* added 960612:ldd */
		if ( keyget < shm->keyold )
			continue;

	/* Make sure it starts at beginning of a header */
		if ( thead.start != FIRST_BYTE ) {
			fprintf(stdout, "ERROR: tport_getmsg; keyget not at FIRST_BYTE, Region %ld\n", region->key);
			exit(1);
		}

		keyget += sizeof(TPORT_HEAD);
	/* See if this msg matches any requested type */
		for ( i = 0; i < nget; i++ ) {
			if ( !COMPARE_MSG_LOGO( getlogo + i, &thead.logo ) )
				break;
		}
	/* */
		if ( i < nget ) {
		/* Found a message of requested logo; retrieve it! */
			if ( msg != NULL && thead.size > maxsize ) {
			/* Complain if retreived msg is too big */
				status = GET_TOOBIG;
				keyget += thead.size;
			}
			else {
			/* Copy message by chunks to caller's address */
				if ( msg != NULL )
					copy_shmmsg_2_buf( shm, keyget, msg, thead.size );
				keyget += thead.size;
			/*
			 * See if we got run over by tport_putmsg while copying msg
			 * if we did, go back and try to get a msg cleanly.
			 */
				keyold = shm->keyold;
				if ( keyold >= keyget ) {
					keyget = keyold;
					continue;
				}
			}
		/* Set other returned variables */
			*logo   = thead.logo;
			*length = thead.size;
		/* */
			memset(&trak_in, 0, sizeof(MSG_TRACK));
			trak_in.memkey = region->key;
			trak_in.logo   = thead.logo;
			trak_in.seq    = thead.seq;
			status = track_getmsg_seq( &trak_in, trak, &ntrak, status );
		/* If you got here, that means we got a message we want so just leave the loop */
			break;
		}
		else {
			keyget += thead.size;
		}
	} /* end while over ring */

/* Update outpointer (->msg after retrieved one) for all requested logos */
	update_track_list_keyout( trak, ntrak, getlogo, nget, region->key, keyget );

	return status;
}

/*
 * tport_copyfrom() - Get a message out of public shared memory; save the sequence number
 *                    from the transport layer, with the intent of copying it to a private
 *                    (buffering) memory ring
 *
 * Arguments:
 *
 */
int tport_copyfrom( SHM_INFO  *region,   /* info structure for memory region */
		    MSG_LOGO  *getlogo,  /* requested logo(s)                */
		    short      nget,     /* number of logos in getlogo       */
		    MSG_LOGO  *logo,     /* logo of retrieved message 	     */
		    long      *length,   /* size of retrieved message        */
		    char      *msg,      /* retrieved message                */
		    long       maxsize,  /* max length for retrieved message */
		    unsigned char *seq ) /* TPORT_HEAD seq# of retrieved msg */
{
   static MSG_TRACK  trak[NTRACK_GET]; /* sequence #, outpointer keeper     */
   static int        nlogo;            /* # modid,type,instid combos so far */
   int               it;               /* index into trak                   */
   SHM_HEAD         *shm;              /* pointer to start of memory region */
   char             *ring;             /* pointer to ring part of memory    */
   TPORT_HEAD       *tmphd;            /* temp pointer into shared memory   */
   long              ir;               /* index into the ring               */
   long              nfill;            /* bytes from ir to ring's last-byte */
   long              nwrap;            /* bytes to grab from front of ring  */
   TPORT_HEAD        hd;               /* transport header from memory      */
   char             *h;                /* pointer to transport layer header */
   int               ih;               /* index into the transport header   */
   unsigned long     keyin;            /* in-pointer to shared memory       */
   unsigned long     keyold;           /* oldest complete message in memory */
   unsigned long     keyget;           /* pointer at which to start search  */
   int               status = GET_OK;  /* how did retrieval go?             */
   int               lapped = 0;       /* = 1 if memory ring has been over- */
 				       /* written since last tport_copyfrom */
   int               trakked;          /* flag for trakking list entries    */
   int               i,j;

/**** Get the pointers set up ****/

   shm  = region->addr;
   ring = (char *) shm + sizeof(SHM_HEAD);
   h    = (char *) (&hd);

/**** First time around, initialize sequence counters, outpointers ****/

   if (Copyfrom_Init)
   {
       nlogo = 0;

       for( i=0 ; i < NTRACK_GET ; i++ )
       {
          trak[i].memkey      = 0;
          trak[i].logo.type   = 0;
          trak[i].logo.mod    = 0;
          trak[i].logo.instid = 0;
          trak[i].seq         = 0;
          trak[i].keyout      = 0;
          trak[i].active      = 0; /*960618:ldd*/
       }
       Copyfrom_Init = 0;
   }

/**** make sure all requested logos are entered in tracking list ****/

   for ( j=0 ; j < nget ; j++ )  /* for all requested message logos */
   {
       trakked = 0;  /* assume it's not being trakked */
       for( it=0 ; it < nlogo ; it++ )  /* for all logos we're tracking */
       {
          if( region->key       != trak[it].memkey      ) continue;
          if( getlogo[j].type   != trak[it].logo.type   ) continue;
          if( getlogo[j].mod    != trak[it].logo.mod    ) continue;
          if( getlogo[j].instid != trak[it].logo.instid ) continue;
          trakked = 1;  /* found it in the trakking list! */
          break;
       }
       if( trakked ) continue;
    /* Make an entry in trak for this logo; if there's room */
       if ( nlogo < NTRACK_GET )
       {
          it = nlogo;
          trak[it].memkey = region->key;
          trak[it].logo   = getlogo[j];
          nlogo++;
       }
   }

/**** find latest starting index to look for any of the requested logos ****/

findkey:

   keyget = 0;

   for ( it=0 ; it < nlogo ; it++ )  /* for all message logos we're tracking */
   {
       if ( trak[it].memkey != region->key ) continue;
       for ( j=0 ; j < nget ; j++ )  /* for all requested message logos */
       {
          if((getlogo[j].type   == trak[it].logo.type   || getlogo[j].type==WILD) &&
             (getlogo[j].mod    == trak[it].logo.mod    || getlogo[j].mod==WILD)  &&
             (getlogo[j].instid == trak[it].logo.instid || getlogo[j].instid==WILD) )
          {
             if ( trak[it].keyout > keyget )  keyget = trak[it].keyout;
          }
       }
   }

/**** make sure you haven't been lapped by tport_copyto or tport_putmsg ****/
   if ( keyget < shm->keyold ) {
      keyget = shm->keyold;
      lapped = 1;
   }

/**** See if keyin and keyold were wrapped and reset by tport_putmsg; ****/
/****       If so, reset trak[xx].keyout and go back to findkey       ****/

   keyin = shm->keyin;
   if ( keyget > keyin )
   {
      keyold = shm->keyold;
      for ( it=0 ; it < nlogo ; it++ )
      {
         if( trak[it].memkey == region->key )
         {
          /* reset keyout */
/*DEBUG*/    /*printf("tport_copyfrom: Pre-reset:  keyout=%10u    keyold=%10u  keyin=%10u\n",
                     trak[it].keyout, keyold, keyin );*/
             trak[it].keyout = trak[it].keyout % shm->keymax;
/*DEBUG*/    /*printf("tport_copyfrom:  Intermed:  keyout=%10u    keyold=%10u  keyin=%10u\n",
                     trak[it].keyout, keyold, keyin );*/

          /* make sure new keyout points to keyin or to a msg's first-byte; */
          /* if not, we've been lapped, so set keyout to keyold             */
             ir    = trak[it].keyout;
             tmphd = (TPORT_HEAD *) &ring[ir];
             if ( trak[it].keyout == keyin   ||
                  (keyin-trak[it].keyout)%shm->keymax == 0 )
             {
/*DEBUG*/       /*printf("tport_copyfrom:  Intermed:  keyout=%10u  same as keyin\n",
                        trak[it].keyout );*/
                trak[it].keyout = keyin;
             }
             else if( tmphd->start != FIRST_BYTE )
             {
/*DEBUG*/       /*printf("tport_copyfrom:  Intermed:  keyout=%10u  does not point to FIRST_BYTE\n",
                        trak[it].keyout );*/
                trak[it].keyout = keyold;
                lapped = 1;
             }

          /* else, make sure keyout's value is between keyold and keyin */
             else if ( trak[it].keyout < keyold )
             {
                do {
                    trak[it].keyout += shm->keymax;
                } while ( trak[it].keyout < keyold );
             }
/*DEBUG*/    /*printf("tport_copyfrom:     Reset:  keyout=%10u    keyold=%10u  keyin=%10u\n",
                     trak[it].keyout, keyold, keyin );*/
         }
      }
    /*fprintf( stdout,
          "NOTICE: tport_copyfrom; keyin wrapped, keyout(s) reset; Region %ld\n",
           region->key );*/

      goto findkey;
   }


/**** Find next message from requested type, module, instid ****/

nextmsg:

   while ( keyget < keyin )
   {
   /* make sure you haven't been lapped by tport_copyto or tport_putmsg */
       if ( keyget < shm->keyold ) {
	  keyget = shm->keyold;
          lapped = 1;
       }

   /* load next header; make sure you weren't lapped */
       ir = keyget % shm->keymax;
       for ( ih=0 ; ih < sizeof(TPORT_HEAD) ; ih++ )
       {
          if ( ir >= shm->keymax )  ir -= shm->keymax;
          h[ih] = ring[ir++];
       }
       if ( keyget < shm->keyold ) continue;  /*added 960612:ldd*/

   /* make sure it starts at beginning of a header */
       if ( hd.start != FIRST_BYTE )
       {
          fprintf( stdout,
                  "ERROR: tport_copyfrom; keyget not at FIRST_BYTE, Region %ld\n",
                   region->key );
          exit( 1 );
       }
       keyget += sizeof(TPORT_HEAD) + hd.size;

   /* see if this msg matches any requested type */
       for ( j=0 ; j < nget ; j++ )
       {
          if((getlogo[j].type   == hd.logo.type   || getlogo[j].type == WILD) &&
             (getlogo[j].mod    == hd.logo.mod    || getlogo[j].mod  == WILD) &&
             (getlogo[j].instid == hd.logo.instid || getlogo[j].instid == WILD) )
          {

/**** Found a message of requested logo; retrieve it! ****/
        /* complain if retreived msg is too big */
             if ( hd.size > maxsize )
             {
               *logo   = hd.logo;
               *length = hd.size;
	       *seq    = hd.seq;
                status = GET_TOOBIG;
                goto trackit;    /*changed 960612:ldd*/
             }
        /* copy message by chunks to caller's address */
             nwrap = ir + hd.size - shm->keymax;
             if ( nwrap <= 0 )
             {
                memcpy( (void *) msg, (void *) &ring[ir], hd.size );
             }
             else
             {
                nfill = hd.size - nwrap;
                memcpy( (void *) &msg[0],     (void *) &ring[ir], nfill );
                memcpy( (void *) &msg[nfill], (void *) &ring[0],  nwrap );
             }
        /* see if we got lapped by tport_copyto or tport_putmsg while copying msg */
        /* if we did, go back and try to get a msg cleanly          */
             keyold = shm->keyold;
             if ( keyold >= keyget )
             {
                keyget = keyold;
		lapped = 1;
                goto nextmsg;
             }

        /* set other returned variables */
            *logo   = hd.logo;
            *length = hd.size;
            *seq    = hd.seq;

trackit:
        /* find logo in tracked list */
             for ( it=0 ; it < nlogo ; it++ )
             {
                if ( region->key    != trak[it].memkey      )  continue;
                if ( hd.logo.type   != trak[it].logo.type   )  continue;
                if ( hd.logo.mod    != trak[it].logo.mod    )  continue;
                if ( hd.logo.instid != trak[it].logo.instid )  continue;
                /* activate sequence tracking if 1st msg */
                if ( !trak[it].active )
                {
                    trak[it].seq    = hd.seq;
                    trak[it].active = 1;
                }
                goto sequence;
             }
        /* new logo, track it if there's room */
             if ( nlogo == NTRACK_GET )
             {
                fprintf( stdout,
                     "ERROR: tport_copyfrom; exceeded NTRACK_GET\n");
                if( status != GET_TOOBIG ) status = GET_NOTRACK; /*changed 960612:ldd*/
                goto wrapup;
             }
             it = nlogo;
             trak[it].memkey = region->key;
             trak[it].logo   = hd.logo;
             trak[it].seq    = hd.seq;
             trak[it].active = 1;      /*960618:ldd*/
             nlogo++;

sequence:
        /* check if sequence #'s match; update sequence # */
             if ( status == GET_TOOBIG   )  goto wrapup; /*added 960612:ldd*/
             if ( hd.seq != trak[it].seq )
             {
       		if (lapped)  status = GET_MISS_LAPPED;
                else         status = GET_MISS_SEQGAP;
                trak[it].seq = hd.seq;
             }
             trak[it].seq++;

        /* Ok, we're finished grabbing this one */
             goto wrapup;

          } /* end if of logo & getlogo match */
       }    /* end for over getlogo */
   }        /* end while over ring */

/**** If you got here, there were no messages of requested logo(s) ****/

   status = GET_NONE;

/**** update outpointer (->msg after retrieved one) for all requested logos ****/

wrapup:
   for ( it=0 ; it < nlogo ; it++ )  /* for all message logos we're tracking */
   {
       if ( trak[it].memkey != region->key ) continue;
       for ( j=0 ; j < nget ; j++ )  /* for all requested message logos */
       {
          if((getlogo[j].type   == trak[it].logo.type   || getlogo[j].type==WILD) &&
             (getlogo[j].mod    == trak[it].logo.mod    || getlogo[j].mod==WILD)  &&
             (getlogo[j].instid == trak[it].logo.instid || getlogo[j].instid==WILD) )
          {
             trak[it].keyout = keyget;
          }
       }
    }

   return( status );

}

/*
 * tport_flush() - Flush messages out of shared memory.
 * Arguments:
 *   info structure for memory region
 *   requested logo(s)
 *   number of logos in getlogo
 *   logo of retrieved msg
 */

int tport_flush( SHM_INFO *region, MSG_LOGO *getlogo, short nget, MSG_LOGO *logo )
{
	int  res;
	long length;

	do {
		res = tport_getmsg( region, getlogo, nget, logo, &length, NULL, 0 );
	} while ( res != GET_NONE );

	return res;
}



/*
 * tport_doFlagOp() - Perform operation op on the flag
 */
static int tport_doFlagOp( SHM_INFO* region, int pid, int op )
{
   int i;
   SHM_FLAG  *smf;
   int start, stop;
   int		 ret_val = 0;
#ifndef _USE_POSIX_SHM
   struct sembuf sops;             /* semaphore operations; changed to non-static 980424:ldd */
   int res;
#ifdef _MACOSX
   int triesLeft;
#else
    struct timespec timeout;
#endif
#endif

   if ( Flag_Init )
   	  tport_createFlag();
   smf = (SHM_FLAG *)smf_region.addr;
   if ( smf == NULL )
   	  return 0;

#ifdef _USE_POSIX_SHM
   if ( sem_wait(smf_region.sid) == -1 )
      tport_syserr( "tport_doFlagOp sem_wait ->inuse", smf_region.key );
#else
   sops.sem_num = 0;
   sops.sem_flg = 0;
   sops.sem_op = SHM_INUSE;
#ifdef _MACOSX
   triesLeft = FLAG_LOCK_TRIES;
   while ( triesLeft > 0 && (res = semop( smf_region.sid, &sops, 1 )) == -1 ) {
      if ( errno != EINTR )
          break;
      triesLeft--;
   }
#else
/* This part is indeed a bottle neck for highthrough system */
    timeout.tv_sec = 2;
    timeout.tv_nsec = 0;
    res = semtimedop( smf_region.sid, &sops, 1, &timeout );
    if (res==-1 && errno==EAGAIN) {
        /* assume that process that acquired lock has died and proceed */
        fprintf( stdout, "tport_doFlagOp semop/wait timed out; proceeding\n" );
        res = 0;
    }
#endif
   if (res == -1)
      tport_syserr( "tport_doFlagOp semop ->inuse", smf_region.key );
#endif

   switch ( op ) {
   case FF_REMOVE:
   	start = 0;
   	stop = smf->nPids;
   	break;

   case FF_FLAG2ADD:
   	start = 0;
   	stop = smf->nPidsToDie;
   	break;

   case FF_FLAG2DIE:
    if ( pid == TERMINATE ) {
	   	/* Terminate supercedes all individial process termination requests */
	   	if ( smf->nPidsToDie == 0 || smf->pid[0] != TERMINATE ) {
			smf->pid[smf->nPids] = smf->pid[0];
			smf->pid[0] = TERMINATE;
			smf->nPidsToDie++;
			smf->nPids++;
		}
		if ( region != NULL )
			(region->addr)->flag = TERMINATE;
	   	ret_val = TERMINATE;
	   	start = 0;
	   	stop = 0;
	} else if ( smf->nPidsToDie > 0 && smf->pid[0] == TERMINATE ) {
	   	/* We've already been told to terminate everybody */
	   	ret_val = pid;
	   	start = 0;
	   	stop = 0;
	} else {
	   	start = smf->nPidsToDie;
   		stop = smf->nPids;
   	}
   	break;

   case FF_GETFLAG:
  	if ( smf->nPidsToDie>0 && smf->pid[0] == TERMINATE ) {
   		/* All modules being terminated */
   		ret_val = TERMINATE;
   	}
   	start = 0;
   	stop = smf->nPidsToDie;
   	break;

   case FF_CLASSIFY:
   	start = 0;
   	stop = smf->nPids;
   	break;
   }

   for ( i=start; i<stop; i++ )
		if ( smf->pid[i] == pid )
			break;

   switch ( op ) {
   case FF_REMOVE:
	if ( i >= stop ) {
		ret_val = 0;
		break;
	}
	if ( i < smf->nPidsToDie ) {
		smf->nPidsToDie--;
		smf->pid[i] = smf->pid[smf->nPidsToDie];
		i = smf->nPidsToDie;
	}
	smf->nPids--;
	smf->pid[smf->nPidsToDie] = smf->pid[smf->nPids];
	ret_val = pid;
	break;

   case FF_FLAG2ADD:
	if ( i >= stop ) {
		/* If no room, we'll have to treat as old-style */
		if ( smf->nPids < MAX_NEWTPROC ) {
			smf->pid[smf->nPids] = pid;
			smf->nPids++;
		}
	}
	ret_val = pid;
	break;

   case FF_FLAG2DIE:
   	if ( ret_val == 0 ) {
		if ( i >= stop ) {
			if ( region != NULL )
				(region->addr)->flag = pid;
		} else if ( i > smf->nPidsToDie ) {
			smf->pid[i] = smf->pid[smf->nPidsToDie];
			smf->pid[smf->nPidsToDie] = pid;
			smf->nPidsToDie++;
		}
		ret_val = pid;
	}
	break;

   case FF_GETFLAG:
    if ( ret_val != TERMINATE ) {
		if ( i >= stop ) {
			stop = smf->nPids;
			for ( ; i<stop; i++ )
				if ( smf->pid[i] == pid )
					break;
			/* If pid unknown, treat as an old-style module */
			ret_val = ( i >= stop && region != NULL) ? (region->addr)->flag : 0;
		} else {
			ret_val = ( i >= stop ) ? 0 : pid;
		}
	}
	break;

   case FF_CLASSIFY:
	ret_val = ( i >= stop ) ? 0 : ( i >= smf->nPidsToDie ) ? 1 : 2;
   }

#ifdef _USE_POSIX_SHM
   if ( sem_post(smf_region.sid) == -1 )
      tport_syserr( "tport_doFlagOp sem_post ->inuse", smf_region.key );
#else
   sops.sem_op = SHM_FREE;
   res = semop( smf_region.sid, &sops, 1 );
   if (res == -1)
      tport_syserr( "tport_doFlagOp semop ->free", smf_region.key );
#endif

   return ret_val;
}


/********************* function tport_putflag ************************/
/*           Puts the kill flag into a shared memory region.         */
/*********************************************************************/

void tport_putflag( SHM_INFO *region,  /* shared memory info structure */
       		    int     flag )   /* tells attached processes to exit */
{
	if ( smf_region.addr == NULL )
		(region->addr)->flag = flag;
	else
		tport_doFlagOp( region, flag, FF_FLAG2DIE );
}


/******************** function tport_detachFromFlag ******************/
/* Remove pid from flag; return 0 if pid not in flag, pid otherwise  */
/*********************************************************************/
int tport_detachFromFlag( SHM_INFO *region, int pid )
{
	return tport_doFlagOp( region, pid, FF_REMOVE );
}


/*********************** function tport_addToFlag ********************/
/*   Add pid from flag; return 0 if pid not in flag, pid otherwise   */
/*********************************************************************/

int tport_addToFlag( SHM_INFO *region, int pid )
{
	return tport_doFlagOp( region, pid, FF_FLAG2ADD );
}



/*********************** function tport_getflag **********************/
/*         Returns the kill flag from a shared memory region.        */
/*********************************************************************/

int tport_getflag( SHM_INFO *region )

{
	if ( smf_region.addr == NULL )
		if ( region == NULL )
			return 0;
		else
			return (region->addr)->flag;
	else
		return tport_doFlagOp( region, getpid(), FF_GETFLAG );
}


/*********************** function tport_newModule ********************/
/* Returns 0 if process w/ id pid isn't using new transport library  */
/*********************************************************************/

int tport_newModule( int pid )

{
	return (tport_doFlagOp( NULL, pid, FF_CLASSIFY ) != 0);
}



/************************** tport_bufthr ****************************/
/*     Thread to buffer input from one transport ring to another.   */
/********************************************************************/
void *tport_bufthr( void *dummy )
{
   char		 errnote[150];
   MSG_LOGO      logo;
   long          msgsize;
   unsigned char msgseq;
   int           res1, res2;
   int 		 gotmsg;

/* Flush all existing messages from the public memory region
   *********************************************************/
   while( tport_copyfrom((SHM_INFO *) PubRegion, (MSG_LOGO *) Getlogo,
                          Nget, &logo, &msgsize, (char *) Message,
                          MaxMsgSize, &msgseq )  !=  GET_NONE  );

   while ( 1 )
   {
/* If a terminate flag is found, go to sleep;
   the main thread should cause the process to exit!
   *************************************************/
      if ( tport_getflag( (SHM_INFO *) PubRegion ) == TERMINATE )
      {
         tport_putflag( (SHM_INFO *) BufRegion, TERMINATE );
         sleep_ew( 100000 );
      }

      do
      {
/* Try to copy a message from the public memory region
   ***************************************************/
 	  res1 = tport_copyfrom((SHM_INFO *) PubRegion, (MSG_LOGO *) Getlogo,
				Nget, &logo, &msgsize, (char *) Message,
			        MaxMsgSize, &msgseq );
	  gotmsg = 1;

/* Handle return values
   ********************/
          switch ( res1 )
	  {
          case GET_MISS_LAPPED:
		sprintf( errnote,
			"tport_bufthr: Missed msg(s)  c%d m%d t%d  Overwritten, region:%ld.",
			 (int) logo.instid, (int) logo.mod, (int) logo.type,
			 PubRegion->key );
		tport_buferror( ERR_LAPPED, errnote );
		break;
	  case GET_MISS_SEQGAP:
		sprintf( errnote,
			"tport_bufthr: Missed msg(s)  c%d m%d t%d  Sequence gap, region:%ld.",
			 (int) logo.instid, (int) logo.mod, (int) logo.type,
			 PubRegion->key );
		tport_buferror( ERR_SEQGAP, errnote );
		break;
          case GET_NOTRACK:
		sprintf( errnote,
			"tport_bufthr: Logo c%d m%d t%d not tracked; NTRACK_GET exceeded.",
			(int) logo.instid, (int) logo.mod, (int) logo.type );
		tport_buferror( ERR_UNTRACKED, errnote );
          case GET_OK:
		break;
          case GET_TOOBIG:
		sprintf( errnote,
			"tport_bufthr: msg[%ld] c%d m%d t%d seq%d too big; skipped in region:%ld.",
			 msgsize, (int) logo.instid, (int) logo.mod,
		         (int) logo.type, (int) msgseq, PubRegion->key );
		tport_buferror( ERR_OVERFLOW, errnote );
          case GET_NONE:
		gotmsg = 0;
		break;
          }

/* If you did get a message, copy it to private ring
   *************************************************/
	  if ( gotmsg )
	  {
	        res2 = tport_copyto( (SHM_INFO *) BufRegion, &logo,
			             msgsize, (char *) Message, msgseq );
		switch (res2)
		{
		case PUT_TOOBIG:
		   sprintf( errnote,
		       "tport_bufthr: msg[%ld] (c%d m%d t%d) too big for Region:%ld.",
			msgsize, (int) logo.instid, (int) logo.mod, (int) logo.type,
			BufRegion->key );
		   tport_buferror( ERR_OVERFLOW, errnote );
		case PUT_OK:
		   break;
		}
	  }
      } while ( res1 != GET_NONE );

      sleep_ew( 500 );

   }
}


/************************** tport_buffer ****************************/
/*       Function to initialize the input buffering thread          */
/********************************************************************/
int tport_buffer( SHM_INFO  *region1,      /* transport ring	         */
		  SHM_INFO  *region2,      /* private ring 	         */
		  MSG_LOGO  *getlogo,      /* array of logos to copy 	 */
		  short      nget,         /* number of logos in getlogo */
		  unsigned   maxMsgSize,   /* size of message buffer 	 */
		  unsigned char module,	   /* module id of main thread   */
		  unsigned char instid )   /* instid id of main thread   */
{
   int error;
#ifdef _USE_PTHREADS
   pthread_t thr;
   pthread_attr_t attr;
#endif

/* Allocate message buffer
   ***********************/
   Message = (char *) malloc( maxMsgSize );
   if ( Message == NULL )
   {
      fprintf( stdout, "tport_buffer: Error allocating message buffer\n" );
      return( -1 );
   }

/* Copy function arguments to global variables
   *******************************************/
   PubRegion   = region1;
   BufRegion   = region2;
   Getlogo     = getlogo;
   Nget        = nget;
   MaxMsgSize  = maxMsgSize;
   MyModuleId  = module;
   MyInstid    = instid;

/* Lookup message type for error messages
   **************************************/
   if ( GetType( "TYPE_ERROR", &TypeError ) != 0 ) {
      fprintf( stderr,
              "tport_buffer: Invalid message type <TYPE_ERROR>\n" );
      return( -1 );
   }

/* Start the input buffer thread
   *****************************/
#ifdef _USE_PTHREADS
   if ( (error = pthread_attr_init(&attr)) != 0 ) {
	fprintf(stderr,"tport_buffer: pthread_attr_init error: %s\n",strerror(error));
	return (-1);
   }
   if ( (error = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) != 0 ) {
	fprintf(stderr,"tport_buffer: pthread_attr_setdetachstate error: %s\n",strerror(error));
	return (-1);
   }
   if ( (error = pthread_create( &thr, &attr, tport_bufthr, (void *)NULL )) != 0 )
   {
      fprintf( stderr, "tport_buffer: pthread_create error: %s\n", strerror(error));
      return( -1 );
   }
#else
   /* Solaris defaults: 1MB stacksize, inherit parent's priority */
   if ( thr_create( NULL, NULL, tport_bufthr, NULL, THR_DETACHED,
                    NULL ) != 0 )
   {
      fprintf( stderr, "tport_buffer: thr_create error: %s\n", strerror(error));
      return( -1 );
   }
#endif

/* Yield to the buffer thread
   **************************/
#ifdef _USE_PTHREADS
  /* under POSIX.1c there is supposed to be a pthread_yield().  Additionally, the
     POSIX.1b sched_yield() function is amended to refer to the calling thread, as
     opposed to the calling process.  Some vendors didn't implement pthread_yield();
     for those cases redefine pthread_yield as sched_yield.  Old Solaris systems didn't
     implement either; for those cases redefine pthread_yield as thr_yield */
# ifdef __sgi
#  include <sched.h>
#  define pthread_yield sched_yield
# endif
# ifdef _UNIX
#  include <sched.h>
#  define pthread_yield sched_yield
# endif
# ifdef OLD_SOLARIS	/* Replace with appropriate system-defined symbol */
#  include <thread.h>
#  define pthread_yield thr_yield
# endif
   pthread_yield();
#else
   thr_yield();
#endif

   return( 0 );
}

/************************* tport_buferror ***************************/
/*  Build an error message and put it in the public memory region   */
/********************************************************************/
void tport_buferror( short  ierr, 	/* 2-byte error word       */
		     char  *note  )	/* string describing error */
{
	MSG_LOGO    logo;
	char	    msg[256];
	long	    size;
	time_t	    t;

	logo.instid = MyInstid;
        logo.mod    = MyModuleId;
        logo.type   = TypeError;

        time( &t );
	sprintf( msg, "%ld %hd %s\n", t, ierr, note );
	size = strlen( msg );   /* don't include the null byte in the message */

 	if ( tport_putmsg( (SHM_INFO *) PubRegion, &logo, size, msg ) != PUT_OK )
	{
	    printf("tport_bufthr:  Error sending error:%hd for module:%d.\n",
		    ierr, MyModuleId );
	}
	return;
}


/*
 * tport_syserr() - Print a system error and terminate.
 * Arguments:
 *   message to print (which routine had an error)
 *   identifies which memory region had the error
 */
void tport_syserr( const char *msg, const long key )
{
#if defined(_LINUX) || defined(_SOLARIS)
	extern int               sys_nerr;
	extern const char *const sys_errlist[];
#elif defined(_MACOSX)
	extern int const         sys_nerr;
	extern const char *const sys_errlist[];
#else
	extern  char *sys_errlist[];
#endif

/* */
	fprintf(stdout, "ERROR: %s (%d", msg, errno);
#ifdef _UNIX
	if ( errno > 0 )
		fprintf(stdout,"; %s) Region: %ld\n", strerror(errno), key);
#else
	if ( errno > 0 && errno < sys_nerr )
		fprintf(stdout,"; %s) Region: %ld\n", sys_errlist[errno], key);
#endif
	else
		fprintf(stdout, ") Region: %ld\n", key);

	exit(1);
}

/******************* function tport_createFlag **********************/
/*        Create the shared memory flag & its semaphore,            */
/*           attach to it and initialize header values.             */
/********************************************************************/

void tport_createFlag()
{
   SHM_FLAG *faddr;

   if ( Flag_Init == 0 )
   		return;

   shm_flag_key = GetKeyWithDefault( SHM_FLAG_RING, SHM_FLAG_DEFAULT_KEY );

   tport_create( &smf_region, sizeof(SHM_FLAG), shm_flag_key );

   faddr = (SHM_FLAG *)smf_region.addr;

   faddr->nPidsToDie = faddr->nPids = 0;

   Flag_Init = 0;
}

/****************** function tport_destroyFlag **********************/
/*                Destroy the shared memory flag.                   */
/********************************************************************/

void  tport_destroyFlag()
{
   tport_destroy( &smf_region );
}

/*
 *
 */
static int move_keyold_2_nextmsg( SHM_HEAD *shm )
{
	TPORT_HEAD           ohead;                            /* transport header of oldest msg      */
	uint8_t             *ohead_p  = (uint8_t *)&ohead;     /* pointer to oldest transport header  */
	uint8_t const       *ohsz_end = (uint8_t *)&ohead.size + sizeof(ohead.size);
	const uint8_t       *ring_p   = (uint8_t *)(shm + 1);  /* pointer to ring part of memory      */
	const uint8_t const *ring_end = ring_p + shm->keymax;  /* pointer to the end of ring          */

/* Point to the first byte of the oldest transport head */
	ring_p += shm->keyold % shm->keymax;
	if ( *ring_p != FIRST_BYTE )
		return -1;
/* Copy the oldest transport head to local memory; here we only need the size information */
	for ( ; ohead_p < ohsz_end; ohead_p++, ring_p++ ) {
		if ( ring_p >= ring_end )
			ring_p = (uint8_t *)(shm + 1);
		*ohead_p = *ring_p;
	}
/* Just move the index to the next msg */
	shm->keyold += sizeof(TPORT_HEAD) + ohead.size;

	return 0;
}

/*
 *
 */
static RING_INDEX_T find_latest_keyget(
	const SHM_HEAD *shm, MSG_TRACK trak_list[], const long ntrak,
	const MSG_LOGO getlogo[], const long nget, const long memkey,
	RING_INDEX_T *keyin
) {
	RING_INDEX_T result;    /* pointer at which to start search  */
	RING_INDEX_T keyold;    /* oldest complete message in memory */
	MSG_TRACK   *trak_ptr;  /* pointer to outpointer keeper      */
	TPORT_HEAD  *thead;     /* temp pointer into shared memory   */
	int          i, j;

/* Find latest starting index to look for any of the requested logos */
	do {
	/*  */
		result = shm->keyold;
	/* For all message logos we're tracking */
		for ( i = 0, trak_ptr = trak_list; i < ntrak; i++, trak_ptr++ ) {
			if ( trak_ptr->memkey == memkey ) {
			/* For all requested message logos */
				for ( j = 0; j < nget; j++ ) {
					if ( !COMPARE_MSG_LOGO( getlogo + j, &trak_ptr->logo ) ) {
						if ( trak_ptr->keyout > result )
							result = trak_ptr->keyout;
					}
				}
			}
		}
		*keyin = shm->keyin;
	/*
	 * See if keyin and keyold were wrapped and reset by putting msg process;
	 * If so, reset trak[xx].keyout and go back to the beginning of loop.
	 */
		if ( result > *keyin ) {
			keyold = shm->keyold;
			for ( i = 0, trak_ptr = trak_list; i < ntrak; i++, trak_ptr++ ) {
				if ( trak_ptr->memkey == memkey ) {
				/* DEBUG */
					/* printf("tport_getmsg: Pre-reset: keyout=%ld keyold=%ld keyin=%ld\n", trak_ptr->keyout, keyold, *keyin); */
				/* Reset keyout */
					trak_ptr->keyout %= shm->keymax;
				/* DEBUG */
					/* printf("tport_getmsg: Intermed: keyout=%ld keyold=%ld keyin=%ld\n", trak_ptr->keyout, keyold, *keyin); */
				/*
				 * Make sure new keyout points to keyin or to a msg's first-byte;
				 * if not, we've been lapped, so set keyout to keyold
				 */
					thead = (TPORT_HEAD *)((uint8_t *)(shm + 1) + trak_ptr->keyout);
					if ( trak_ptr->keyout == *keyin || ((*keyin - trak_ptr->keyout) % shm->keymax) == 0 ) {
					/* DEBUG */
						/* printf("tport_getmsg: Intermed: keyout=%ld same as keyin\n", trak_ptr->keyout); */
						trak_ptr->keyout = *keyin;
					}
					else if ( thead->start != FIRST_BYTE ) {
					/* DEBUG */
						/* printf("tport_getmsg: Intermed: keyout=%ld does not point to FIRST_BYTE\n", trak_ptr->keyout); */
						trak_ptr->keyout = keyold;
					}
				/* Else, make sure keyout's value is between keyold and keyin */
					else if ( trak_ptr->keyout < keyold ) {
						do {
							trak_ptr->keyout += shm->keymax;
						} while ( trak_ptr->keyout < keyold );
					}
				/* DEBUG */
					/* printf("tport_getmsg: Reset: keyout=%ld keyold=%ld keyin=%ld\n", trak_ptr->keyout, keyold, *keyin); */
				}
			}
			/*
			 * fprintf(stdout, "NOTICE: tport_getmsg; keyin wrapped, keyout(s) reset; Region %ld\n", region->key);
			 */
		}
	} while ( result > *keyin );

	return result;
}

/*
 *
 */
static MSG_TRACK *search_track_in_list(
	MSG_TRACK trak_list[], int *ntrak, const int max_ntrak, const MSG_TRACK *trak_in
) {
	MSG_TRACK   *result = NULL;
	const size_t size_msg_trak = sizeof(MSG_TRACK);

/* */
	result = (MSG_TRACK *)bsearch(trak_in, trak_list, *ntrak, size_msg_trak, compare_msg_track);
	if ( !result ) {
	/* Make an entry in trak for this logo; if there's room */
		if ( *ntrak < max_ntrak ) {
			trak_list[*ntrak] = *trak_in;
			(*ntrak)++;
			qsort(trak_list, *ntrak, size_msg_trak, compare_msg_track);
		/* Search again to return the new pointer of this input track inside the list */
			result = (MSG_TRACK *)bsearch(trak_in, trak_list, *ntrak, size_msg_trak, compare_msg_track);
		}
	}

	return result;
}

/*
 *
 */
static void update_track_list_keyout(
	MSG_TRACK trak_list[], const long ntrak, const MSG_LOGO getlogo[], const long nget,
	const long memkey, const RING_INDEX_T keyget
) {
	MSG_TRACK *trak_ptr;
	int        i, j;

/* For all message logos we're tracking */
	for ( i = 0, trak_ptr = trak_list; i < ntrak; i++, trak_ptr++ ) {
		if ( trak_ptr->memkey == memkey ) {
		/* For all requested message logos */
			for ( j = 0; j < nget; j++ ) {
				if ( !COMPARE_MSG_LOGO( getlogo + j, &trak_ptr->logo ) ) {
					trak_ptr->keyout = keyget;
				}
			}
		}
	}

	return;
}

/*
 *
 */
static RING_INDEX_T copy_msg_2_shm( SHM_HEAD *shm, RING_INDEX_T keyin, const void *msg, const size_t size )
{
	uint8_t *ring_p;  /* pointer to ring part of memory */
	size_t   rspace;  /* Rear space to the end of ring  */

/* */
	keyin %= shm->keymax;
	ring_p = (uint8_t *)(shm + 1) + keyin;
	rspace = shm->keymax - keyin;
/* Now copy data into shared memory by chunks... */
	if ( size <= rspace ) {
		memcpy(ring_p, msg, size);
		keyin += size;
	}
	else {
		memcpy(ring_p, msg, rspace);
		keyin = size - rspace;
		memcpy(shm + 1, (uint8_t *)msg + rspace, keyin);
	}

	return keyin;
}

/*
 *
 */
static RING_INDEX_T copy_shmmsg_2_buf(
	const SHM_HEAD *shm, RING_INDEX_T keyout, void *buf, const size_t size
) {
	uint8_t *ring_p;  /* pointer to ring part of memory */
	size_t   rspace;  /* Rear space to the end of ring  */

/* */
	keyout %= shm->keymax;
	ring_p  = (uint8_t *)(shm + 1) + keyout;
	rspace  = shm->keymax - keyout;
/* Copy message by chunks to caller's address */
	if ( size <= rspace ) {
		memcpy(buf, ring_p, size);
		keyout += size;
	}
	else {
		memcpy(buf, ring_p, rspace);
		keyout = size - rspace;
		memcpy((uint8_t *)buf + rspace, shm + 1, keyout);
	}

	return keyout;
}

/*
 *
 */
static int track_getmsg_seq(
	const MSG_TRACK *trak_in, MSG_TRACK trak_list[], int *ntrak, int status
) {
	MSG_TRACK *trak_ptr = NULL;

/* Find msg logo in tracked list */
	trak_ptr = search_track_in_list( trak_list, ntrak, NTRACK_GET, trak_in );
	if ( !trak_ptr ) {
		status = status != GET_TOOBIG ? GET_NOTRACK : status;
	}
	else {
		if ( !trak_ptr->active ) {
		/* Activate sequence tracking if 1st msg */
			trak_ptr->seq    = trak_in->seq;
			trak_ptr->active = 1;
		}
	/* */
		if ( status != GET_TOOBIG ) {
		/* Check if sequence #'s match; update sequence # */
			if ( trak_ptr->seq != trak_in->seq ) {
				trak_ptr->seq = trak_in->seq;
				status = GET_MISS;
			}
			else {
				status = GET_OK;
			}
			trak_ptr->seq++;
		}
	}

	return status;
}

/*
 *
 */
static void reset_key_shm( SHM_HEAD *shm )
{
/* */
	shm->keyin  %= shm->keymax;
	shm->keyold %= shm->keymax;
/* */
	if ( shm->keyin <= shm->keyold )
		shm->keyin += shm->keymax;

	return;
}

/*
 *
 */
static int compare_msg_track( const void *key, const void *elem )
{
	MSG_TRACK *_key = (MSG_TRACK *)key;
	MSG_TRACK *_elem = (MSG_TRACK *)elem;

/* */
	if ( _key->memkey != _elem->memkey )
		return _key->memkey > _elem->memkey ? 1 : -1;
	if ( _key->logo.type != _elem->logo.type )
		return _key->logo.type > _elem->logo.type ? 1 : -1;
	if ( _key->logo.mod != _elem->logo.mod )
		return _key->logo.mod > _elem->logo.mod ? 1 : -1;
	if ( _key->logo.instid != _elem->logo.instid )
		return _key->logo.instid > _elem->logo.instid ? 1 : -1;

	return 0;
}

/*
 *
 */
static int compare_msg_track_wildlogo( const void *key, const void *elem )
{
	MSG_TRACK *_key = (MSG_TRACK *)key;
	MSG_TRACK *_elem = (MSG_TRACK *)elem;

/* */
	if ( _key->memkey != _elem->memkey )
		return _key->memkey > _elem->memkey ? 1 : -1;
	if ( _key->logo.type != _elem->logo.type && _key->logo.type != WILD )
		return _key->logo.type > _elem->logo.type ? 1 : -1;
	if ( _key->logo.mod != _elem->logo.mod && _key->logo.mod != WILD )
		return _key->logo.mod > _elem->logo.mod ? 1 : -1;
	if ( _key->logo.instid != _elem->logo.instid && _key->logo.instid != WILD )
		return _key->logo.instid > _elem->logo.instid ? 1 : -1;

	return 0;
}

#ifdef _USE_POSIX_SHM
/*
 * Compatibility fuctions to establish conversion between IPC-style keys (used
 * by most earthworm setup files) to POSIX shared memory path identifiers.
 * We need this kludge until everyone is ready to convert their setup files
 * to use a POSIX path instead of an IPC key.  (Each  module would also
 * need to parse the POSIX path instead of an IPC key, and the SHM_INFO struct
 * would need to have POSIX path and file descriptor members instead of IPC
 * key and ID members.)  Since the goal for now is a fairly transparent migration
 * to POSIX, we'll keep the IPC-style keys and make an equivalent path from the key.
 * For those POSIX machines that actually implement POSIX shared memory paths in
 * the filesystem (e.g., IRIX), make sure the BASE macros below refer to an
 * appropriate place, or modify as needed.
 */
#define PREFIX_SHM      "ew_shm_"
#define PREFIX_SEM      "ew_sem_"
#define PREFIX_SHM_FLAG "ew_shm_flag_"
#define PREFIX_SEM_FLAG "ew_sem_flag_"

/*
 *
 */
static char *key_2_path( const long memkey, const int is_shm )
{
	static char result[256] = { 0 };

	if ( memkey == shm_flag_key )
		sprintf(result, "%s%ld", is_shm ? PREFIX_SHM_FLAG : PREFIX_SEM_FLAG, memkey);
	else
		sprintf(result, "%s%ld", is_shm ? PREFIX_SHM : PREFIX_SEM, memkey);

	return result;
}

/*
 *
 */
static int path_2_key( const char *path, const int is_shm )
{
	return atoi(path + strlen(is_shm ? PREFIX_SHM : PREFIX_SEM));
}

/*
 *
 */
static SHM_HEAD *create_shm_region( int *regid, const long nbytes, const long memkey )
{
	void *result;  /* shared memory pointer */
	int   omask;
	int   prot;
	int   flags;

/* Destroy memory region if it already exists */
	if ( (*regid = shm_open(key_2_path( memkey, 1 ), O_RDONLY, 0)) != -1 ) {
		if ( close(*regid) == -1 )
			tport_syserr( "tport_create close", memkey );
		if ( shm_unlink(key_2_path( memkey, 1 )) == -1 )
			tport_syserr( "tport_create shm_unlink", memkey );
	}
/* Temporarily clear any existing file creation mask */
	omask = umask(0);
/* Connect and map shared memory region */
	flags = O_CREAT | O_RDWR | O_EXCL;
	if ( (*regid = shm_open(key_2_path( memkey, 1 ), flags, SHM_DEFAULT_MASK)) == -1 )
		tport_syserr( "tport_create shm_open", memkey );
	ftruncate(*regid, nbytes);

	prot = PROT_READ | PROT_WRITE;
	flags = MAP_SHARED;  /* The flag MAP_AUTOGROW is not documented on every system! */
	if ( (result = mmap(0, nbytes, prot, flags, *regid, 0)) == MAP_FAILED )
		tport_syserr( "tport_create mmap", memkey );
/* Restore any existing file creation mask */
	if ( omask )
		umask(omask);

	return (SHM_HEAD *)result;
}

/*
 *
 */
static sem_t *create_semaphore( const long memkey )
{
	sem_t *result;
	int   omask;

/* Destroy semaphore if it already exists */
	if ( (result = sem_open(key_2_path( memkey, 0 ), 0)) != SEM_FAILED ) {
		if ( sem_close(result) == -1 )
			tport_syserr( "tport_create sem_close", memkey );
		if ( sem_unlink(key_2_path( memkey, 0 )) == -1 )
			tport_syserr( "tport_create sem_unlink", memkey );
	}
/* Temporarily clear any existing file creation mask */
	omask = umask(0);
/* Make semaphore for this shared memory region & set semval = SHM_FREE */
	if ( (result = sem_open(key_2_path( memkey, 0 ), O_CREAT, SHM_DEFAULT_MASK, SHM_FREE)) == SEM_FAILED )
		tport_syserr( "tport_create semget", memkey );
/* Restore any existing file creation mask */
	if ( omask )
		umask(omask);

	return result;
}

/*
 *
 */
static void close_shm_region( SHM_INFO *region )
{
/* Close and delete shared memory region */
	if ( munmap(region->addr, region->addr->nbytes) == -1 )
		tport_syserr( "tport_destroy munmap", region->key );
	if ( close(region->mid) == -1 )
		tport_syserr( "tport_destroy close", region->key );
	if ( shm_unlink(key_2_path( region->key, 1 )) == -1 )
		tport_syserr( "tport_destroy sem_unlink", region->key );

	return;
}

/*
 *
 */
static void destroy_semaphore( SHM_INFO *region )
{
/* Close and delete semaphore */
	if ( sem_close(region->sid) == -1 )
		tport_syserr( "tport_destroy sem_close", region->key );
	if ( sem_unlink(key_2_path( region->key, 0 )) == -1 )
		tport_syserr( "tport_destroy sem_unlink", region->key );

	return;
}

/*
 *
 */
static SHM_HEAD *attach_shm_region( int *regid, const long memkey )
{
    void  *result;   /* shared memory pointer         */
	int    prot;
	int    flags;
	size_t nbytes;

/* Open and map header; find out size memory region; close */
	if ( (*regid = shm_open(key_2_path( memkey, 1 ), O_RDONLY, 0)) == -1 ) {
		if ( memkey != shm_flag_key )
			tport_syserr( "tport_attach shm_open ->header", memkey );
	/* Must have been called when using an old startstop, so no flag ring */
		smf_region.addr = NULL;
		return NULL;
	}
	if ( (result = mmap(0, sizeof(SHM_HEAD), PROT_READ, MAP_SHARED, *regid, 0)) == MAP_FAILED )
		tport_syserr( "tport_attach mmap", memkey );
/* Fetch the size of entire shared memory */
	nbytes = ((SHM_HEAD *)result)->nbytes;
/* Then just close & unmap it */
	if ( munmap(result, sizeof(SHM_HEAD)) == -1 )
		tport_syserr( "tport_attach munmap", memkey );
	if ( close(*regid) == -1 )
		tport_syserr( "tport_attach close ->header",  memkey );

/* Reopen and map entire header */
	if ( (*regid = shm_open(key_2_path( memkey, 1 ), O_RDWR, 0)) == -1 )
		tport_syserr( "tport_attach shm_open ->header", memkey );

	prot = PROT_READ | PROT_WRITE;
	flags = MAP_SHARED;  /* The flag MAP_AUTOGROW is not documented on every system! */
	if ( (result = mmap(0, nbytes, prot, flags, *regid, 0)) == MAP_FAILED )
		tport_syserr( "tport_attach mmap ->region", memkey );

    return (SHM_HEAD *)result;
}

/*
 *
 */
static sem_t *get_semaphore( const long memkey )
{
	sem_t *result;

/* Destroy semaphore if it already exists */
	if ( (result = sem_open(key_2_path( memkey, 0 ), 0)) == (sem_t *)-1 )
		tport_syserr( "tport_attach sem_open", memkey );

	return result;
}

/*
 *
 */
static void detach_shm_region( SHM_INFO *region )
{
	struct stat shm_stat;

/* Silently return if the memory region has already been closed */
	if ( fstat(region->mid, &shm_stat) == -1 )
		if ( errno == EBADF )
			return;
/*
 * Origin comment from earthworm:
 *   munmap is bad to do here because other threads will lose the region.
 * New comment:
 *   I think the attach and detach should be paired, that means every thread should have its own
 *   memory space for region or only one region in the entire process.
 */
	if ( munmap(region->addr, region->addr->nbytes) == -1 )
		tport_syserr( "tport_detach munmap", region->key );
	if ( close(region->mid) == -1 )
		tport_syserr( "tport_detach close", region->key );

	return;
}

#else /* ifdef _USE_POSIX_SHM */
/*
 *
 */
static SHM_HEAD *create_shm_region( int *regid, const long nbytes, const long memkey )
{
	SHM_HEAD       *result = NULL;
	struct shmid_ds shmbuf;    /* shared memory data structure      */
	int             res;

/* Destroy memory region if it already exists */
	if ( (*regid = shmget(memkey, nbytes, 0)) != -1 ) {
		res = shmctl(*regid, IPC_RMID, &shmbuf);
		if ( res == -1 )
			tport_syserr( "tport_create shmctl", memkey );
	}

/* Real create shared memory region */
	*regid = shmget(memkey, nbytes, IPC_CREAT | 0666);
	if ( *regid == -1 )
		tport_syserr( "tport_create shmget", memkey );

/* Attach to shared memory region */
	result = (SHM_HEAD *)shmat(*regid, 0, SHM_RND);
	if ( result == (SHM_HEAD *)-1 )
		tport_syserr( "tport_create shmat", memkey );

	return result;
}

/*
 *
 */
static int create_semaphore( const long memkey )
{
	int result;

/* Make semaphore for this shared memory region & set semval = SHM_FREE */
	result = semget(memkey, 1, IPC_CREAT | 0666);
	if ( result == -1 )
		tport_syserr( "tport_create semget", memkey );

	//semarg->val = SHM_FREE;
	//if ( semctl(result, 0, SETVAL, semarg) == -1 )
		//tport_syserr( "tport_create semctl", memkey );

	if ( semctl(result, 0, SETVAL, SHM_FREE) == -1 )
		tport_syserr( "tport_create semctl", memkey );

	return result;
}

/*
 *
 */
static void close_shm_region( SHM_INFO *region )
{
	struct shmid_ds shmbuf;  /* shared memory data structure */

/* Detach from shared memory region then destroy it */
	if ( shmdt((char *)region->addr) == -1 )
		tport_syserr( "tport_destroy shmdt", region->key );
	if ( shmctl(region->mid, IPC_RMID, &shmbuf) == -1 )
		tport_syserr( "tport_destroy shmctl", region->key );

	return;
}

/*
 *
 */
static void destroy_semaphore( SHM_INFO *region )
{
/* Destroy semaphore set for shared memory region */
	//semarg->val = 0;
	//semctl(region->sid, 0, IPC_RMID, semarg);
	if ( semctl(region->sid, 0, IPC_RMID, 0) == -1 )
		tport_syserr( "tport_destroy semctl", region->key );

	return;
}

/*
 *
 */
static SHM_HEAD *attach_shm_region( int *regid, const long memkey )
{
	SHM_HEAD *result;      /* pointer to start of memory region */
	size_t    nbytes;      /* size of memory region             */

/* Attach to header; find out size memory region; detach */
	*regid = shmget(memkey, sizeof(SHM_HEAD), 0);
	if ( *regid == -1 ) {
		if ( memkey != shm_flag_key )
			tport_syserr( "tport_attach shmget ->header", memkey );
	/* Must have been called when using an old startstop, so no flag ring */
		smf_region.addr = NULL;
		return NULL;
	}
	result = (SHM_HEAD *)shmat(*regid, 0, SHM_RND);
	if ( result == (SHM_HEAD *)-1 )
		tport_syserr( "tport_attach shmat ->header", memkey );
/* Fetch the size of entire shared memory */
	nbytes = result->nbytes;
/* Then just detach it */
	if ( shmdt((char *)result) == -1 )
		tport_syserr( "tport_attach shmdt ->header", memkey );

/* Reattach to the entire memory region */
	*regid = shmget(memkey, nbytes, 0);
	if ( *regid == -1 )
		tport_syserr( "tport_attach shmget ->region", memkey );

	result = (SHM_HEAD *)shmat(*regid, 0, SHM_RND);
	if ( result == (SHM_HEAD *)-1 )
		tport_syserr( "tport_attach shmat ->region", memkey );

    return result;
}

/*
 *
 */
static int get_semaphore( const long memkey )
{
	int result;

/* Destroy semaphore if it already exists */
	result = semget(memkey, 1, 0);
	if ( result == -1 )
		tport_syserr( "tport_attach semget", memkey );

	return result;
}

/*
 *
 */
static void detach_shm_region( SHM_INFO *region )
{
/* Detach from shared memory region */
	if ( shmdt((char *)region->addr) == -1 )
		tport_syserr( "tport_detach shmdt", region->key );

	return;
}
#endif
