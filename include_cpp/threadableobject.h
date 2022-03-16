/*
** ThreadableObject is a base class for objects that are intended to
**                  internally spawn processing threads to execute
**                  specific member methods....a common example would be
**                  a multi-threaded server.
**                  This is needed because C++ member functions do not
**                  have the signature required by thread-spawning
**                  library functions.
**
**
**   Derive the object class from ThreadableObject, and implement
**     the class's StartThreadFunc method (this is the method through
**     which a newly spawned thread reenters the class code).
**   At the locations in the new class where a handler thread is to be
**     spawned, set any appropriate variables (see discussion below),
**     then call StartThreadWithArg() or StartThread().
**
**   Any method to be executed by a spawned thread should generally
**     have the signature:  THR_RTR func(void *).  That is, it returns void
**     and takes one parameter that is a void pointer.  The parameter is
**     not strictly required, but it can simplify coding under one set
**     of circumstances.
**   The spawning process allows a single void pointer parameter to be
**     passed to a new thread.  Even if a method to be executed by a
**     spawned thread needs no parameters, the void pointer must still
**     be passed, even as NULL.  Keep in mind that
**   If there will be more than one function that might be executed by
**     a spawned thread, then there must be some way for a thread
**     to determine which method to enter.  One way to do this is
**     by using a member variable (possibly with a static mutex).
**     The method might also be indicated as part of a structure
**     passed to the spawning code through the void pointer parameter.
**   In some cases, the simplest routing code might result with an
**     implementation that uses a pointer to class member function:
**     <return_type>(<class_name>::*<pointer_type_name>)(<argument_list>)
**     All of the methods pointed to by this pointer type must have the
**     same signature.  If they all really need different parameters,
**     then designing them all to accept a single void pointer parameter
**     will make them look the same.  Using this method, just before calling
**     StartThreadWithArg, set the static method pointer to
**     point at the desired method:
**       <my_member_pointer> = &<class_name>::<method_name>
**     and in the StartThreadFunc, dereference the member pointer:
**       (this->*<my_member_pointer>)(p_arg)
**     note that in this call, p_arg is the void pointer passed through
**     the spawn process -- just that some of the methods might use it and
**     some might not.
**     The reason why this is a simplification is that the code in
**     the StartThreadFunc method does not have to decode the correct
**     method to call based upon the void pointer contents, or any
**     other variables -- it is implicit within the member method pointer.
**     So, this implementation would be appropriate if the logic for
**     determining which method a new thread should execute is complex.
*/
//---------------------------------------------------------------------------
#ifndef threadableobjectH
#define threadableobjectH
//---------------------------------------------------------------------------
#include <worm_statuscode.h>
#include <mutex.h>


//---------------------------------------------------------------------------

#if defined(_WINNT) || defined(_Windows)  // - - - - - - - - - - - - - - - - - - - - - - -


#include <windows.h>
#include <process.h>

#include <stddef.h>

typedef unsigned int TO_STACK_SIZE;
typedef long         TO_THREAD_ID;


#define THREAD_RETURN void         //     Thread functions return this



#elif defined(_SOLARIS)  // - - - - - - - - - - - - - - - - - - - - - - -


#include <stdio.h>

#include <sys/types.h>
#include <thread.h>

typedef size_t       TO_STACK_SIZE;  // or is it unsigned int?
typedef thread_t     TO_THREAD_ID;


typedef void THREAD_RETURN;         //     Thread functions return this


#else                   // - - - - - - - - - - - - - - - - - - - - - - -

#error threadableobject.h not complete for this O/S

#endif  // - - - - - - - - - - - - - - - - - - - - - - -

//---------------------------------------------------------------------------



class ThreadableObject
{
protected:

   /*
   ** StartThreadWithArg() -- start a new thread with argument(s)
   **
   **  Arguments:
   **     stack_size: Stack size of new thread in bytes
   **                 If 0, stack size is set to 8192.
   **                 In OS2, 4096 or 8192 is recommended.
   **                 In SOLARIS, this argument is ignored
   **                 In Windows NT, if stack_size=0, use the stack
   **                 size of the calling thread.
   **     thread_id:  Thread identification number returned to
   **                 calling program.
   **     arg:        an unsigned long (void*) passed to the thread.
   **     p_isdaemon  start the thread/process as a daemon
   **                 rather than internal thread (Solaris only)
   **
   *   Note: It is a good idea for the calling thread to sleep for
   *   250 - 500 ms after this call, or the thread might night
   *   properly initialize.
   * 
   **  Returns:
   **     WORM_STAT_FAILURE if error
   **     WORM_STAT_SUCCESS if ok
   */

   WORM_STATUS_CODE StartThreadWithArg( TO_STACK_SIZE   stack_size
                                      , TO_THREAD_ID  * thread_id
                                      , void          * arg
#ifdef _SOLARIS
                                      , bool            p_isdaemon = false
#endif
                                      );

   /*
   ** StartThread() -- start a new thread with no arguments
   **
   **  Arguments:
   **     stack_size: Stack size of new thread in bytes
   **                 If 0, stack size is set to 8192.
   **                 In OS2, 4096 or 8192 is recommended.
   **                 In SOLARIS, this argument is ignored
   **                 In Windows NT, if stack_size=0, use the stack
   **                 size of the calling thread.
   **     thread_id:  Thread identification number returned to
   **                 calling program.
   **
   **  Returns:
   **     WORM_STAT_FAILURE if error
   **     WORM_STAT_SUCCESS if ok
   */

   WORM_STATUS_CODE StartThread( TO_STACK_SIZE   stack_size
                               , TO_THREAD_ID  * thread_id
#ifdef _SOLARIS
                               , bool            p_isdaemon = false
#endif
                               )
   {
      return StartThreadWithArg( stack_size
                               , thread_id
                               , NULL
#ifdef _SOLARIS
                               , p_isdaemon
#endif
                               );
   }

   /*
   **  KillSelfThread() -- Exit thread without effecting
   **                      other threads
   */

   static void KillSelfThread();

   /*
   **  WaitForThread() -- wait for thread to terminate
   **
   **  Returns:
   **    WORM_STAT_FAILURE if error
   **    WORM_STAT_SUCCESS if ok
   */
   static int WaitForThread( TO_THREAD_ID * thread_id );

   /*
   ** KillThread() -- force immediate, unclean, thread exit
   **
   **  Windows NT documentation gives a strong warning against
   **  using TerminateThread(), since no stack cleanup, etc,
   **  is done.
   **
   ** Argument:
   **    tid = id of thread to kill
   **
   ** Returns:
   **    WORM_STAT_SUCCESS if ok
   **    WORM_STAT_FAILURE if an error
   */
   static int KillThread(  TO_THREAD_ID tid );


   // That -- Pointer to the ThreadableObject requesting a
   //         new internal thread.
   //
   static ThreadableObject * That;

   // ThatMutex -- access lock for That pointer memory
   //
   static TMutex * ThatMutex;

public:

   ThreadableObject()
   {
      if ( ThatMutex == NULL )
      {
         ThatMutex = new TMutex("ThreadMutex");
      }
   }

   // GetThat() -- Method used by the file-scope function
   //              StartObjectThread() to access the "That"
   //              pointer which is the object requesting
   //              the thread start.
   //
   static ThreadableObject * GetThat() { return That; }

   /* StartThreadFunc() -- Method through which a newly-spawned
   **                      thread returns to the "That" object
   **                      to start execution.
   **
   ** PARAMETERS:
   **      p_argument = pointer to any object containing
   **                   requisite unique start conditions to
   **                   the thread, possibly including
   **                   indication of which method to execute.
   */
   virtual void StartThreadFunc( void * p_argument ) = 0;
};

#endif

