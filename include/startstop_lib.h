/*
 *    Revision history:
 *     $Log$
 *     Revision 1.17  2010/03/24 04:26:10  stefan
 *     named pipe name option
 *
 *     Revision 1.16  2010/03/10 22:39:56  scott
 *     Added missing field for new startstop module termination behavior
 *
 *     Revision 1.15  2010/03/04 16:43:55  scott
 *     Make truncation of module paramaters in status display smarter; added optional maxStatusLineLen command
 *
 *     Revision 1.14  2010/01/11 17:39:19  scott
 *     Remove path-length limitation in status display
 *
 *     Revision 1.13  2010/01/07 22:04:34  scott
 *     Fixed type of mutex field of CHILD to work for all unixes
 *
 *     Revision 1.12  2009/12/14 20:07:10  scott
 *     Fixed revision #s for startstop & statmgr
 *
 *     Revision 1.11  2009/12/12 15:00:00  scott
 *     Added threading & recovery from bad reconfigure
 *
 *     Revision 1.10  2009/04/08 19:25:36  tim
 *     Display Bin Dir, Log Dir and Params Dir in startstop
 *
 *     Revision 1.9  2007/12/16 12:20:23  paulf
 *     added MAX_CLASSNAME_SIZE #define
 *
 *     Revision 1.8  2007/12/15 20:12:39  paulf
 *     increased className array size for linux
 *
 *     Revision 1.7  2007/04/03 17:28:58  paulf
 *     modified MAX_CHILD from 100 to 200 as per user request
 *
 *     Revision 1.6  2007/03/28 14:13:39  paulf
 *     minor MACOSX #ifdefs added
 *
 *     Revision 1.5  2007/02/27 05:15:34  stefan
 *     stop/restart messages
 *
 *     Revision 1.4  2007/02/22 21:01:17  stefan
 *     lock procs
 *
 *     Revision 1.3  2007/02/20 21:58:46  stefan
 *     startstop lock, no two at once
 *
 *     Revision 1.2  2006/06/17 01:15:51  stefan
 *     remove inaccurate comment
 *
 *     Revision 1.1  2006/04/04 16:48:03  stefan
 *     startstop with reconfigure and libraries
 *
 *     Revision 1.1  20060226 20:05:54  lisowski
 *     Initial revision
 */

/*
 * startstop_unix_generic.h: startstop parameter definitions for all platforms.
 *
 */

#ifndef startstop_util_H
#define startstop_util_H

/* set string indicating 64 bit to go after STARTSTOP_VERSION */
#include <stdint.h>	/* UINTPTR_MAX */
#if   UINTPTR_MAX == 0xffffffffffffffff
#define VERSION_APPEND_STR " (64 bit)"
#elif UINTPTR_MAX == 0x00000000ffffffff
#define VERSION_APPEND_STR " (32 bit)"
#else
#define VERSION_APPEND_STR ""
#endif

  /******************************************************************
   *                         Includes                               *
   *                                                                *
   ******************************************************************/

#ifdef _WINNT
 #include <windows.h>
#endif  /* _WINNT */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <lockfile.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#ifdef __DARWIN_UNIX03
#include <sys/malloc.h>
#endif

#include "earthworm.h"      /* Earthworm main include file */
#include "kom.h"
#include "transport.h"      /* Earthworm shared-memory transport routines */
#include "time_ew.h"        /* Earthworm time conversion routines */

  /******************************************************************
   *                         #defines                               *
   *                                                                *
   ******************************************************************/

/* Below #defines snagged from startstop_x.c */
#define MAX_RING   50         /* Maximum number of transport rings  */
#define MAX_CHILD 200         /* Maximum children to this process   */
#define MAX_ARG    50         /* Maximum command line arguments     */
#define MAX_STATUS_LEN  (((MAX_RING)+(MAX_CHILD))*100)
#define ERR_STARTCHILD  1
#define TIMESTR_LEN  26       /* Length of string from GetCurrentUTC() */

#define LOGNAME_MAX 8 /* This was just assigned for linux; I didn't see where it gets set in Solaris, and neither did the compiler, so I'm breaking this out of the #else below. */
#define PIPE_NAME_MAX 127     /* Maximum length of the string for the */
                              /* name of the named pipe on Windows  */

#define SUCCESS 0             /* Return value */
#define EXIT 1                /* Return value */

/* standard TRUE/FALSE definitions */
#ifndef TRUE
#define TRUE        1
#endif
#ifndef FALSE
#define FALSE       0
#endif

  /******************************************************************
   *                         Structs                                *
   *                                                                *
   ******************************************************************/

#ifdef _WINNT
   #define MAX_CLASSNAME_SIZE 9
    typedef struct {
       char   commandLine[80];
       char   progName[40];
       char   priorityClass[MAX_CLASSNAME_SIZE];
       char   threadPriority[13];
       char   display[17];
       char   status[11]; /* stores "Stopped" "Restarting" status... vs. Alive/Dead */
       PROCESS_INFORMATION procInfo;
       HANDLE mutex; /* used by startstop to serialize operations on this child */
    } CHILD;
#else /* shared by both Solaris and Linux/unix */
    #define MAXLINE     200
    #if defined(_LINUX) || defined(__DARWIN_UNIX03)
	/* was P_MYID (-1), but I can't see how -1 was ever right. changed to 0 which man sched_setscheduler says is what will be used for the PID of the calling process stefan 20130711*/
        #define P_MYID (0)
    #endif
    #ifndef MAX_CLASSNAME_SIZE
        #define MAX_CLASSNAME_SIZE 10
    #endif
    typedef struct {
       char   parm[MAXLINE];
       char   commandLine[MAXLINE];
       char   tcum[20];          /* Cumulative run time */
       char   *argv[MAX_ARG];
       char   *processName;
       pid_t  pid;
       char   className[MAX_CLASSNAME_SIZE];
       int    priority;
       char   use_uname[LOGNAME_MAX+1];
       char   use_gname[LOGNAME_MAX+1];
       uid_t  use_uid;
       gid_t  use_gid;
       char   status[11]; /* stores "Stopped" "Restarting" status... vs. Alive/Dead */
       mutex_t mutex; /* used by startstop to serialize operations on this child */
    } CHILD;

#endif

typedef struct {
    char            ConfigFile[FILENAME_MAX];  /* config file name */
	char			*LogLocation;		       /* log directory */
	char 			*ParamLocation; 	       /* param directory */
	char 			*BinLocation;		       /* bin directory */
    unsigned char   InstId;     /* local installation id          */
    unsigned char   MyModId;        /* startstop's module id          */
    char            MyModName[MAX_MOD_STR]; /* name to use to lookup MyModId */
    unsigned char   ModWildcard;
    unsigned char   TypeError;
    unsigned char   TypeHeartBeat;
    unsigned char   TypeStatus;
    unsigned char   TypeRestart;
    unsigned char   TypeStop;
    unsigned char   TypeReqStatus;
    unsigned char   TypeReconfig;
    char            MyPriorityClass[9];
    int             HeartbeatInt;       /* Heartbeat interval in seconds */
    SHM_INFO        *Region;            /* Pointers to transport regions */
                                /* SHM_INFO definied in transport.h */
    char            ringName[MAX_RING][MAX_RING_STR]; /* Array of ring names.
    MAX_RING_STR is defined in earthworm_defs.h and at this time happens to be set to 32 */
    long            ringSize[MAX_RING]; /* Ring size in kbytes           */
    int             ringKey[MAX_RING];  /* Which rings to create         */
    int             nRing;              /* Number of transport rings     */
    int             LogSwitch;          /* logging switch                */
    int             KillDelay;          /* number of seconds to wait before
                                   killing modules on shutdown   */
    int             HardKillDelay;      /* number of seconds to wait for
                                   SIGKILL to work on a module; is 0, SIGKILL
                                   will not be used, and shutdown of module
                                   will fail */
    int     statmgr_sleeptime;  /* The amount of time that
                                   startstop sleeps after starting
                                   statmgr, default 1000
                                   John Patton                   */
    int     statmgr_location;   /* The location of statmgr in the
                                   child array or, by default, one
                                   past the last possible child so
                                   if statmgr is not present, no
                                   other modules will be affected*/
    int maxStatusLineLen;       /* Maximum length of a line printed by
                                   status; defaults to 80 */
    char tstart[TIMESTR_LEN];   /* Buffer for program start time   */
    char Version[127];          /* Release version   */
	char pipeName[PIPE_NAME_MAX]; /* name for Windows named pipe */
} METARING;

  /******************************************************************
   *                         Prototypes                             *
   *                                                                *
   ******************************************************************/

void Heartbeat( METARING *);    /* Heartbeat function              */
void SendStopReq( METARING *, char *);    /* Send stop message to the ring */
void SendRestartReq( METARING *, char *);  /* Send restart msg to the ring */
void GetCurrentUTC( char * );   /* Get UTC as a 26 char string     */
void ReportError( int, char *, METARING * ); /*  creates an error message */
int  StartError( int , char *, METARING *, int * ); /* start children err mesg */
void LockStartstop(char *, char *);/* Lock startstop so another can't start */
void UnlockStartstop();         /* Unlock once startstop quits */

  /******************************************************************
   *                         Externs                                *
   *                                                                *
   ******************************************************************/

int         oldNChild;    /* original number of children before a reconfig request */
int         newNRing;     /* new NRing total */
#ifdef _WINNT
DWORD       badProcessID; /* increment this to keep track of bad processes */
#else /* shared by both Solaris and Linux/unix */
pid_t       badProcessID; /* increment this to keep track of bad processes */
#endif
char * lockfile;          /* create a lock so only a single instance of startstop runs */
int lockfile_fd;

#endif /* not defined startstop_util_H */
