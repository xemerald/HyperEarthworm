//---------------------------------------------------------------------------
#ifndef MutexH
#define MutexH
//---------------------------------------------------------------------------
#if defined(_WINNT) || defined(_Windows)   //-------------------------------------------

#include <windows.h>
#include <stdlib.h>

#elif defined(_SOLARIS) //-------------------------------------------

#include <synch.h>

#else                  //-------------------------------------------

#endif           //-------------------------------------------

#include <worm_exceptions.h>


#define WORM_MUTEX_NAME_LEN  14
typedef char MUTEX_NAME[WORM_MUTEX_NAME_LEN+1];

class TMutex
{
private:
#if defined(_WINNT) || defined(_Windows)
   HANDLE   MutexHandle;
#elif  defined(_SOLARIS)
//   int      ShMemoryId;
//   bool     ImOwner;
   mutex_t  MutexHandle;
#else
#error TMutex not complete in mutex.h for this OS
#endif
protected:
   MUTEX_NAME     Name;
public:

   // THROWs worm_exception
   TMutex( const MUTEX_NAME p_name );

   ~TMutex();

   // THROWs worm_exception
   void RequestLock();

   // THROWs worm_exception
   void ReleaseLock();
};

#endif
 