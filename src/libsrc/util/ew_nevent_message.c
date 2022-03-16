


/* 
   Copyright Paul Friberg ISTI 2012,  Free for anyone to use and modify as long 
   as credit is given to ISTI. No guarantees or warrantees implied or given for this code.
   See full text of COPYRIGHT at end of this intro.

   This API is to duplicate the Windows Event Handling inter-process communication
   interface known as Named Events (not provided in UNIX semaphores in quite the same way.

   The idea here is to introduce an _ew version of each of the following Windoze funcs:

   CreateEvent()
   SetEvent()
   WaitForSingleObject()
   CloseHandle()

   and use a hidden reserved earthworm ring to pass messages. This API will only be valid 
   for startstop 7.6c and higher and will not work otherwise.
 
   The startstop generic call to create the hidden reserved earthworm ring is:

   tport_createNamedEventRing() and it succeeds or fails and exits 

   By default the NAMED_EVENT_RING will be 100kb in size, and have a
   predefined RING ID number, but the RING ID number can be overwritten
   in the earthworm.d (like is done for FLAG_RING) if there are multiple
   earthworm instances on the same host (as can happen for testers and
   really advanced users).

   The design is simply:

   CreateEvent() creates a struct to contain a NamedEvent and mostly just contains the name string.

   SetEvent() sends a NAMED_EVENT_MSG message into the NAMED_EVENT_RING containing
   just the epoch of the time the named event was created and the name (space separated)

   WaitForSingleObject() blocks waiting for a NAMED_EVENT_MSG to appear of the given named type

   CloseHandle() deletes the struct created by CreateEvent()

   This is far more powerful than the Windoze implementation since if an export/import is lashed up,
   then a program on a remote host could fire an action on a local earthworm AND it allows for 
   cross language interfaces for the same reason. A Java GUI could fire a message into a import socket
   and control a C processing module.

   some example code taken from Mm of EarlyBird:

   HANDLE hEventMomDone;

   hEventMomDone = CreateEvent( NULL,        // Default security attributes
                                FALSE,       // Auto-reset event
                                FALSE,       // Initially not set
                                "DoneMm" );  // Share with LOCATE
   CloseHandle(hEventMomDone);

   WaitForSingleObject( hEventMom, INFINITE );

   SetEvent( hEventMomDone );       // Respond to calling program

******************************************************************************/

#include <string.h>
#include <earthworm.h>
#include <transport.h>
#include <ew_nevent_message.h>
#include <time.h>

#define EXTRA_MESSAGE_SPACE 64

static SHM_INFO _named_event_region;
static int attached_to_name_ring = 0;

void SetEvent_ew(struct NamedEventHandle *nh);


/**********************************************************************************/
/*  tport_createNamedEventRing() succeeds in creating the named event ring or exits
    if there are any problems (as all tport_ funcs do that encounter fatal errors).
 
    this function only to be called ONE time by startstop, ring size is fixed, for now 
*/
void tport_createNamedEventRing()
{

long memkey;

        memkey =  GetKeyWithDefault( NAMED_EVENT_RING_STR, NAMED_EVENT_DEFAULT_KEY );

	tport_create( &_named_event_region,   /* info structure for memory region  */
                   (long) NAMED_EVENT_RING_SIZE,   /* size of shared memory region      */
                   memkey );  /* key to shared memory region       */ 
}
void tport_destroyNamedEventRing()
{
	tport_destroy( &_named_event_region );
}
/**********************************************************************************/
/* _attachToNameRing() is an internal function called by CreateEvent_ew() to connect one
	time to the NAME RING.
*/
void _attachToNameRing() {
long memkey;

	if (! attached_to_name_ring ) {
        	memkey =  GetKeyWithDefault( NAMED_EVENT_RING_STR, NAMED_EVENT_DEFAULT_KEY );
		tport_attach(&_named_event_region, memkey);
	}
	/* for now do not flush it */
	attached_to_name_ring++;
}

/**********************************************************************************/
/*  CreateEvent_ew() creates the named event struct, and sends it if requested.
	Returns a HANDLE to the created struct, which must be destroyed with CloseHandle_ew()

	Returns NULL if any problem occurs (memory not avail, ring not created, etc).
		and writes to logit() the error encountered.
*/

HANDLE CreateEvent_ew(
                   void *security, /* not used */
		   int auto_reset, /* not used */
		   int set_on_init, /* create the event and set it if TRUE (non-zero) */
		   char * name_of_event) 
{

struct NamedEventHandle  *nh;


	if (name_of_event == NULL) {
		logit("et", "CreateEvent_ew(): fatal programmer error no name provided for named event\n");
		return(NULL);
	}

	if (strlen(name_of_event) > MAX_NAMED_STRING_SIZE) {
		logit("et", "CreateEvent_ew(): fatal programmer error string of name provided longer (%zu) than allowed (%d)\n",
			strlen(name_of_event), MAX_NAMED_STRING_SIZE);
		return(NULL);
	}

	if ( (nh = (struct NamedEventHandle *) calloc(1, sizeof(struct NamedEventHandle))) == NULL ) {
		logit("et", "CreateEvent_ew(): fatal error allocating memory for Handle for named event %s\n", name_of_event);
		return(NULL);
	}

 	nh->creation_epoch = time(NULL);
 	nh->name = strdup(name_of_event);
        nh->security = security;
        nh->auto_reset = auto_reset;
        nh->set_on_init = set_on_init;

	_attachToNameRing();	/* we will use it or why create it */

        if (nh->set_on_init) {
		SetEvent_ew(nh);
	}

	return (nh);
}

/**********************************************************************************/
/* SetEvent_ew() - connects to hidden transport ring, and tport_put's the named event message into it.
	The logo of the message is hard to do since we don't know the module id of the setter in the API, so we use 0

	If the ring has not been attached to, it is initialized.
*/
void SetEvent_ew(struct NamedEventHandle *nh)
{
MSG_LOGO send_logo;

char msg_str[MAX_NAMED_STRING_SIZE+EXTRA_MESSAGE_SPACE];

	if (!nh) {
      		fprintf( stderr, "SetEvent_ew(): Fatal programmer error no NamedEventHandle provided; exiting!\n" );
		exit(-1);
	}

	
	/* build the logo */
	send_logo.mod = 0; /* maybe set this with another setter func somewhere later on */
   	if ( GetLocalInst( &send_logo.instid ) != 0 ) {
      		fprintf( stderr, "SetEvent_ew(): Fatal error getting local installation id; exiting!\n" );
      		exit( -1 );
   	}
	if( GetType( NAMED_EVENT_MESSAGE_TYPE_STR, &send_logo.type ) != 0 ) {
		fprintf( stderr, "SetEvent_ew(): Fatal error message type <%s> not found; add it to earthworm.d; exiting\n", 
			NAMED_EVENT_MESSAGE_TYPE_STR);
		exit( -1 );
	}

	/* build the message */
	sprintf(msg_str, "%ld %s", (long)time(NULL), nh->name);
	
	/* kick it */
	tport_putmsg(&_named_event_region, &send_logo, (long) strlen(msg_str), msg_str);
	/* for now ignore return */
}
/**********************************************************************************/
/*  WaitForSingleObject_ew()  blocks or waits for duration seconds for the named event to be seen 
		(or forever if INFINITE is specified). Once event is seen, func returns
*/
void   WaitForSingleObject_ew( struct NamedEventHandle *nh, int duration)
{
MSG_LOGO getlogo[1];
MSG_LOGO reclogo;
long recsize;
char msg_buffer[MAX_NAMED_STRING_SIZE+EXTRA_MESSAGE_SPACE+1];
int res; /* result of tport_getmsg */
long event_time;
char event_name[MAX_NAMED_STRING_SIZE];
int wait_ms = 100; /* TODO: this value is related to the precision time you need */
time_t       now;
time_t       start_time;

	time(&start_time);

	if (!nh) {
      		fprintf( stderr, "WaitForSingleObject_ew(): Fatal programmer error no NamedEventHandle provided; exiting!\n" );
		exit(-1);
	}
	if( GetType( NAMED_EVENT_MESSAGE_TYPE_STR, &getlogo[0].type ) != 0 ) {
		fprintf( stderr, "WaitForSingleObject_ew(): Fatal error message type <%s> not found; add it to earthworm.d; exiting\n", 
			NAMED_EVENT_MESSAGE_TYPE_STR);
		exit( -1 );
	}
	getlogo[0].instid = 0; /* wild card */
	getlogo[0].mod = 0; /* wild card */

	while(1) 
	{
           res = tport_getmsg( &_named_event_region, getlogo, 1,
                               &reclogo, &recsize, msg_buffer, 
				MAX_NAMED_STRING_SIZE+EXTRA_MESSAGE_SPACE+1 );
           switch(res)
           {
           case GET_MISS:   /* got a message; missed some */
                logit("t", "Missed msg(s)  i%u m%u t%u  region:%ld.\n",
                         reclogo.instid, reclogo.mod, reclogo.type, _named_event_region.key);
           case GET_OK:   /* got a message just fine */
                msg_buffer[recsize] = '\0';              /*null terminate the message*/
                logit( "t", "named event message recieved:'%s'", msg_buffer );  /*debug*/
		
		/* confirm its this named event we care about */
		res = sscanf(msg_buffer, "%ld %s", &event_time, event_name);
		if (res != 2) {
			logit("et", "Msg not formatted properly, ignoring\n");
			continue;
		}
		if ((time_t)event_time > nh->creation_epoch && strcmp(nh->name, event_name) ==0) {
			return; /* success */
		}
	   }
	   sleep_ew(wait_ms);
	   time(&now);
	   if (duration != INFINITE && difftime(now,start_time) >= duration) {
		return; /* unblock because timeout was reached before message showed */
	   }
	}
	/* will never reach here */
}
/**********************************************************************************/
/* CloseHandle_ew() - delete resources and detatch from transport if no more events left
*/
void CloseHandle_ew(struct NamedEventHandle *nh) 
{
	if (!nh) return;

	free(nh->name);
	free(nh);
	nh = NULL;
	attached_to_name_ring--;
	
	if (! attached_to_name_ring) {
		tport_detach(&_named_event_region);
	}
}
