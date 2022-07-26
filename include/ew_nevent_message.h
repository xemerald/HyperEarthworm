
#include <time.h>

#define NAMED_EVENT_RING_STR "NAMED_EVENT_RING" 
#define NAMED_EVENT_MESSAGE_TYPE_STR "TYPE_NAMED_EVENT"
#define NAMED_EVENT_DEFAULT_KEY 9991
#define NAMED_EVENT_RING_SIZE 100000  /* 100kb */
#define MAX_NAMED_STRING_SIZE 128

#ifndef _WINNT
#define INFINITE            0xFFFFFFFF  // Infinite timeout
typedef void* HANDLE;
#endif

struct NamedEventHandle {
	char * name;		
        void * security;		/* not used */
        int auto_reset;		/* not used */
        int set_on_init;	/* if 0, do not send event at time of creation */
	time_t creation_epoch;	/* time of creation of event, only handle named events after this time */
};

/* function prototypes */
/* only to be used by startstop */
void tport_createNamedEventRing();
void tport_destroyNamedEventRing();

void SetEvent_ew(struct NamedEventHandle *nh);
void WaitForSingleObject_ew( struct NamedEventHandle *nh, int duration);
void CloseHandle_ew(struct NamedEventHandle *nh);
HANDLE CreateEvent_ew(void* security, /* not used */
                   int auto_reset, /* not used */
                   int set_on_init, /* create the event and set it if TRUE (non-zero) */
                   char * name_of_event);
