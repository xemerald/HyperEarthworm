/**********************************************************************
 *  This file contains prototypes for earthworm libsrc                *
 *  functions that require special type definitions, such as          *
 *  (semaphores, threads, mutexes, sockets, etc.).                    *
 *                                                                    *
 *  If you have functions that only use primitive types and you       *
 *  do not need any extra header files for them to compile, then      *
 *  you can put them into earthworm_simple_funcs.h.                   *
 *                                                                    *
 *  Note, please try to keep functions from the same object           *
 *  together in one section of one file.  So all of the sema_ew.c     *
 *  stuff should go together.  Thank You!                             *
 *  Davidk 2001/04/06                                                 *
 **********************************************************************/

#ifndef EARTHWORM_COMPLEX_FUNCS_H
#define EARTHWORM_COMPLEX_FUNCS_H

#include <stdarg.h>

/* System-dependent stuff goes here
   ********************************/
#include "platform.h"

#if 0 /* OBSOLETE */
void CreateSemaphore_ew( void );            /* sema_ew.c    system-dependent */
void PostSemaphore   ( void );              /* sema_ew.c    system-dependent */
void WaitSemPost     ( void );              /* sema_ew.c    system-dependent */
void DestroySemaphore( void );              /* sema_ew.c    system-dependent */
#endif

/* Use SpecificSemaphore Uppercase APIs names in order to develop cross-platform application:
 *   - DECLARE_SPECIFIC_SEMAPHORE_EW(semname)
 *   - CREATE_SPECIFIC_SEMAPHORE_EW(semname, count)
 *   - POST_SPECIFIC_SEMAPHORE_EW(semname)
 *   - WAIT_SPECIFIC_SEMAPHORE_EW(semname)
 *   - DESTROY_SPECIFIC_SEMAPHORE_EW(semname)
 */

#ifdef _WINNT
#define DECLARE_SPECIFIC_SEMAPHORE_EW(semname) sema_t semname
#else
#define DECLARE_SPECIFIC_SEMAPHORE_EW(semname) sema_t *semname
#endif

#ifdef _MACOSX
#define CREATE_SPECIFIC_SEMAPHORE_EW(semname, count) semname = CreateSpecificNamedSemaphore_ew(get_semaphore_unique_name("semname"), count)
#define DESTROY_SPECIFIC_SEMAPHORE_EW(semname) DestroySpecificNamedSemaphore_ew(get_semaphore_unique_name("semname"), semname)
#else
#ifdef _WINNT
#define CREATE_SPECIFIC_SEMAPHORE_EW(semname, count) CreateSpecificSemaphore_ew(&semname, count)
#else
#define CREATE_SPECIFIC_SEMAPHORE_EW(semname, count) semname = CreateSpecificSemaphore_ew(count)
#endif
#define DESTROY_SPECIFIC_SEMAPHORE_EW(semname) DestroySpecificSemaphore_ew(semname)
#endif

#ifdef _WINNT
#define WAIT_SPECIFIC_SEMAPHORE_EW(semname) WaitSpecificSemaphore_ew(&semname)
#define POST_SPECIFIC_SEMAPHORE_EW(semname) PostSpecificSemaphore_ew(&semname)
#else
#define WAIT_SPECIFIC_SEMAPHORE_EW(semname) WaitSpecificSemaphore_ew(semname)
#define POST_SPECIFIC_SEMAPHORE_EW(semname) PostSpecificSemaphore_ew(semname)
#endif

#ifdef _MACOSX
char *get_semaphore_unique_name(char *semname);
sema_t *CreateSpecificNamedSemaphore_ew( char *sem_name,
                      unsigned int count ); /* sema_ew.c    system-dependent */
#else
#ifdef _WINNT
void CreateSpecificSemaphore_ew(HANDLE *ms, unsigned int count );
                                            /* sema_ew.c    system-dependent */
#else
sema_t *CreateSpecificSemaphore_ew( unsigned int count ); /* sema_ew.c    system-dependent */
#endif
#endif

void PostSpecificSemaphore_ew  ( sema_t * );              /* sema_ew.c    system-dependent */
void WaitSpecificSemaphore_ew  ( sema_t * );              /* sema_ew.c    system-dependent */

#ifdef _MACOSX
void DestroySpecificNamedSemaphore_ew( char *sem_name,  sema_t *ms );
#else
void DestroySpecificSemaphore_ew  ( sema_t * );           /* sema_ew.c    system-dependent */
#endif

int logit_core( const char *, const char *, va_list );/* logit.c sys-independent */

void CreateMutex_ew  ( void );              /* sema_ew.c    system-dependent */
void RequestMutex( void );                  /* sema_ew.c    system-dependent */
void ReleaseMutex_ew( void );               /* sema_ew.c    system-dependent */
void CloseMutex( void );                    /* sema_ew.c    system-dependent */
void CreateSpecificMutex( mutex_t * );
void CloseSpecificMutex( mutex_t * );
void RequestSpecificMutex( mutex_t * );
void ReleaseSpecificMutex( mutex_t * );

                                            /* sendmail.c   system-dependent */
void SocketSysInit( void   );               /* socket_ew.c  system-dependent */
void SocketClose  ( SOCKET    );            /* socket_ew.c  system-dependent */
void SocketPerror ( char * );               /* socket_ew.c  system-dependent */
int sendall( SOCKET, const char *, long, int ); /* socket_ew.c  system-dependent */

/******************************************************************************
 *                                                                            *
 *             Function prototypes for the Earthworm threads API              *
 *   Platform-specific data types (thr_ret, ew_thread_t) are in platform.h    *
 *   Implementations are in src/libsrc/{solaris,unix,windows}/threads_ew.c    *
 *                                                                            *
 ******************************************************************************/

int WaitThread( ew_thread_t * );
int KillThread( ew_thread_t );
int KillSelfThread( void );
int StartThread( thr_ret (*fun)( void * ), unsigned int, ew_thread_t * );
int StartThreadWithArg( thr_ret (*fun)( void * ), void *, unsigned int, ew_thread_t * );

#endif /* EARTHWORM_COMPLEX_FUNCS_H */
