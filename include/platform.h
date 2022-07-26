
              /*************************************************
               *                   platform.h                  *
               *                                               *
               *  System-dependent stuff.                      *
               *  This file is included by earthworm.h         *
               *************************************************/

#ifndef PLATFORM_H
#define PLATFORM_H

/***************************************************************** Standard C */

#include <stdint.h>			/* C99 */
#include <stdio.h>			/* C90 */
#include <stdlib.h>			/* C90 */

/********************************************************************* _WINNT */

#ifdef _WINNT
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>              /* Socket stuff */
#include <windows.h>
#include <process.h>               /* Required for getpid() */
#include <sys\types.h>

/* Thread functions return these */
typedef void thr_ret;
typedef uintptr_t ew_thread_t;
/* Value returned by thread functions; for Windows, nothing */
#define THR_NULL_RET

#define getpid _getpid
typedef int    pid_t;
typedef HANDLE sema_t;
typedef HANDLE mutex_t;
typedef HANDLE timer_t;

#ifndef EW_INT16
typedef signed __int16 EW_INT16;
#endif

#ifndef EW_INT32
typedef signed __int32 EW_INT32;
#endif

/* added so that logit.c can call vsnprintf for all platforms */
#define vsnprintf _vsnprintf
#define  snprintf  _snprintf

/* Thread priority constants.  These are based off the Win32 constants in winbase.h */
#define EW_PRIORITY_LOWEST      THREAD_PRIORITY_IDLE
#define EW_PRIORITY_LOW         THREAD_PRIORITY_BELOW_NORMAL
#define EW_PRIORITY_NORMAL      THREAD_PRIORITY_NORMAL
#define EW_PRIORITY_HIGH        THREAD_PRIORITY_ABOVE_NORMAL
#define EW_PRIORITY_CRITICAL    THREAD_PRIORITY_TIME_CRITICAL

#define DIR_SLASH   '\\'
#else
#define DIR_SLASH   '/'
#endif /* _WINNT */

/*********************************************************************** _OS2 */

#ifdef _OS2
#error OS/2 is no longer a supported platform
#endif /* _OS2 */

/********************************************************************* _LINUX */

#ifdef _LINUX
#define _UNIX
/* broke this out on 2006/03/08 - paulf */
/* note the LINUX/POSIX includes go here, mostly pthread changes */
#include <sys/types.h>
#include <netinet/in.h>            /* Socket stuff */
#include <arpa/inet.h>             /* Socket stuff */
#include <signal.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>            /* Socket stuff */
#include <netdb.h>                 /* Socket stuff */
#define mutex_t pthread_mutex_t
#define sema_t int
#define thread_t int
#define USYNC_THREAD 0
#undef SHM_INFO
#define fork1 fork

/* Thread functions return these */
typedef void *thr_ret;
typedef pthread_t ew_thread_t;
/* Value returned by thread functions; for UNIX, NULL */
#define THR_NULL_RET (NULL)

#ifndef LONG
#define LONG long
#endif
#ifndef LONG_t
#define LONG_t
#ifdef _CYGWIN
typedef unsigned long ulong ;
#endif 
typedef unsigned long ULONG ;
#endif
typedef uint32_t DWORD ;

#ifndef EW_INT16
typedef int16_t EW_INT16;
#endif

#ifndef EW_INT32
typedef int32_t EW_INT32;
#endif

#ifndef SOCKET
#define SOCKET int
#endif

#endif /* _LINUX */

/******************************************************************** _MACOSX */

#ifdef _MACOSX
#define _UNIX
#include <sys/types.h>
#include <netinet/in.h>            /* Socket stuff */
#include <arpa/inet.h>             /* Socket stuff */
#include <signal.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>            /* Socket stuff */
#include <netdb.h>                 /* Socket stuff */
#define mutex_t pthread_mutex_t
#define sema_t int
#define thread_t int
#define USYNC_THREAD 0
#undef SHM_INFO
#define fork1 fork

/* Thread functions return these */
typedef void *thr_ret;
typedef pthread_t ew_thread_t;
/* Value returned by thread functions; for MACOSX, NULL */
#define THR_NULL_RET (NULL)

#ifndef LONG
#define LONG long
#endif
#ifndef LONG_t
#define LONG_t
typedef unsigned long ULONG ;
typedef unsigned long ulong;
#endif
typedef uint32_t DWORD ;

typedef long timer_t;
#ifndef EW_INT16
typedef int16_t EW_INT16;
#endif

#ifndef EW_INT32
typedef int32_t EW_INT32;
#endif

#ifndef SOCKET
#define SOCKET int
#endif

#endif /* _MACOSX */

/******************************************************************* _SOLARIS */

#ifdef _SOLARIS
/* all SOLARIS includes now specifically go here */
#include <malloc.h>
#include <sys/types.h>
#include <netinet/in.h>            /* Socket stuff */
#include <arpa/inet.h>             /* Socket stuff */
#include <signal.h>
#include <synch.h>                 /* for mutex's */
#include <sys/ipc.h>
#include <sys/shm.h>
#include <wait.h>
#include <thread.h>
#include <unistd.h>
#include <sys/socket.h>            /* Socket stuff */
#include <netdb.h>                 /* Socket stuff */

/* Thread functions return these */
typedef void *thr_ret;
typedef thread_t ew_thread_t;
/* Value returned by thread functions; for Solaris, NULL */
#define THR_NULL_RET (NULL)

#ifndef LONG_t
#define LONG_t
typedef long LONG;
#endif
typedef uint32_t DWORD;
#ifndef EW_INT16
typedef int16_t EW_INT16;
#endif
#ifndef EW_INT32
typedef int32_t EW_INT32;
#endif

#ifndef SOCKET
#define SOCKET int
#endif

/* Thread priority constants.  These are arbitrary values chosen over the Solaris priority range
 * (0-127).  These may need to be tweaked.  (MMM 10/7/04)
 */
#define EW_PRIORITY_LOWEST      0
#define EW_PRIORITY_LOW         1
#define EW_PRIORITY_NORMAL      8
#define EW_PRIORITY_HIGH       16
#define EW_PRIORITY_CRITICAL  127

#endif /* _SOLARIS */

#endif /* PLATFORM_H */
