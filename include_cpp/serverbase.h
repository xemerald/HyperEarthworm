/*
**  ServerBase -- a Earthworm base class for any TCP/Stream server
*/
//---------------------------------------------------------------------------
#if !defined(EWserverbaseH)
#define EWserverbaseH
//---------------------------------------------------------------------------

#include <socket_exception.h>

#if defined(_WINNT) || defined(_Windows)
#include <winsock2.h> // prevent conflicts with winsock.h
#endif

extern "C" {
#include "platform.h"
#include "socket_ew.h"
#include "transport.h"
}
#define _USING_EW_XPORT 1 // prevent conflict between EW & MW

// microsoft pragma to avoid 157 messages in some cases
#pragma warning(disable:4786)

// following includes must follow the winsock2.h include, etc.
#include <threadableobject.h>
#include <time.h>     // time() -- for interthread heartbeats
#include <configurable.h>
#include <map>
#include <vector>
#include <worm_types.h>
#include <worm_defs.h>
#include <worm_signal.h>
#include <logger.h>
#include <globalutils.h>
#include <configsource.h>

#define SERVE_MAX_THREADS    100
#define WAIT_FOR_SERVICE_THREAD  3000    // milliseconds to wait for a service thread to come free

#define SERVICE_THREAD_SLEEP_MS  1000  // milliseconds for client to sleep between loop passes

#define SERVICE_THREAD_PULSE_MAX 10 // maximum seconds that the main thread will allow a service
                                    // thread to exist without getting a valid pulse before it
                                    // kills the service thread

#define THREAD_STACK  (unsigned int)8096

// Absolute minumum time to listen for a message if specified
#define MIN_RECV_TIMEOUT_MS  50

enum WORM_SERVER_THREAD_STATE  // Server Thread States
{
   THREAD_ERROR            = -1
 , THREAD_STARTING         =  0
 , THREAD_INITIALIZING     =  1
 , THREAD_WAITING          =  2  // waiting for something to do
 , THREAD_PROCESSING       =  3  // doing something
 , THREAD_BLOCKINGSOCKET   =  4
 , THREAD_DISCONNECTED     =  5
 , THREAD_COMPLETED        = 10
};

typedef char CLIENT_IPADDR[16];

typedef struct _ServiceThreadInfoStruct
{
   SOCKET                   descriptor; // Socket descriptors
   TO_THREAD_ID             threadid;
   CLIENT_IPADDR            ipaddr;
   struct sockaddr_in       sock_addr;
   WORM_SERVER_THREAD_STATE status;
   time_t                   lastpulse; // = time(NULL) during each pass of ClientServicer() loop
} ServiceThreadInfoStruct;

typedef std::map<SOCKET, ServiceThreadInfoStruct> SERVICETHREAD_MAP;
typedef SERVICETHREAD_MAP::iterator SERVICETHREAD_MAP_ITERATOR;

typedef std::vector<SOCKET> SERVICETHREADID_VECTOR;

//---------------------------------------------------------------------------

class WormServerBase : public ThreadableObject
                     , public TConfigurable
{
private:
   // Members in the private: area cannot be accessed by deriving classes
   // (except through accessor methods in the protected area)

   // Socketing
   SOCKET         PassiveSocket;

   unsigned int MaxServiceThreads; // both max allowed and max waiting in listen queue

protected:

   bool           SocketDebug;

   char           ServerIPAddr[20]; // IP address of wave_server machine
   int            ServerPort;       // Server port for requests & replies

   int            SendTimeoutMS;    // Timeout milliseconds for send_ew() calls, blocking if -1

   // Listen() -- listens to the passive socket, starts client server threads
   //             as requests arrive.
   //
   THREAD_RETURN Listener( void * p_dummy );
   TO_THREAD_ID  ListenerThreadId;
   long          LastListenerPulse;

   static bool   DoSCNLsMatch( const char * p_s1
                             , const char * p_c1
                             , const char * p_n1
                             , const char * p_l1
                             , const char * p_s2
                             , const char * p_c2
                             , const char * p_n2
                             , const char * p_l2
                             );

   // =======================================================================
   //                  from TConfigurable
   // =======================================================================

   /* CheckConfig() -- allows derivative classes to report the status of their
   **                  the lookup values.
   **
   ** From within any deriving class, or further derivation, ALWAYS contain a call to
   ** <super_class>::CheckConfig() in their own CheckConfig() method...
   ** this ensures that all classes in the heirarchy get their chance to report status.
   **
   ** All implementations should set ConfigStatus value to WORM_STAT_BADSTATE if there
   ** is a configuration problem, otherwise leave it alone.
   */
   void CheckConfig();

   // =======================================================================
   //                  WormServerBase
   // =======================================================================

   // Accessor to private data
   //
   bool SetMaxThreads( unsigned short p_count )
   {
      // Check for a sane value
      if ( SERVE_MAX_THREADS < p_count )
      {
         return false;
      }
      MaxServiceThreads = p_count;
      return true;
   }

   // not valid until after PrepToRun()
   int            LoggingLevel;

   SERVICETHREAD_MAP ThreadsInfo;

   int            RecvTimeoutMS;    // Timeout (milliseconds) to wait for message from client in ListenForMsg(),
                                    // -1 == ignored and socket made blocking during recv_ew()

   // Ring to check for shutdown flags, to post heartbeats, etc.
   WORM_RING_NAME CommandRingName;	// name of transport ring
   WORM_RING_ID   CommandRingKey;   // key to transport ring
   SHM_INFO       CommandRegion;  // Info structure for shared memory (command ring)

   /*
   ** ListenForMsg()
   **
   **    Listens on an active socket for a message ('\n' terminated)
   **    from the client.
   **
   ** PARAMETERS:
   **          p_descriptor = socket descriptor for recv()
   **          p_rcv_buffer = buffer to receive data
   **          p_length = on call set to maximum chars to read,
   **                     on return will be actual number read
   **          p_timeoutms = used to override RecvTimeoutMS
   **                        If greater than zero, then use it for the
   **                        timeout, otherwise RecvTimeoutMS is used.
   **
   ** RETURNS  0 = read completed successfully
   **         -1 = read timed-out (check if p_length == 0 else p_rcv_buffer[p_length-1] == '\n')
   **         -2 = client closed socket (terminate service thread)
   **         -3 = socket error (other then time-out)
   **         -4 = abort to prevent buffer overrun
   **
   ** WARNING: This method will support blocking client sockets (config
   **          parm RecvTimeoutMSecs not set or -1), however, if used then
   **          can only send a single pulse to the main thread before entering
   **          the client thread a potentially blocked state.
   **          This prevents the main thread from identifying, reporting and
   **          terminating hung threads.  Therefore, use blocking sockets
   **          with caution.
   */
   int ListenForMsg( SOCKET p_descriptor
                   , char * p_rcv_buffer
                   , int *  p_length
                   , int    p_timeoutms = -1 // uses RecvTimeoutMS if not included in the function call (or -1)
                   );

   /*
   **
   ** RETURNS  WORM_STAT_SUCCESS
   **          WORM_STAT_FAILURE
   */
   WORM_STATUS_CODE SendMessage( const SOCKET p_descriptor, const char * p_msg, int * p_length );

   /* Running -- flag indicating if the main loop is running,
   **            set to false by terminate message, or various
   **            errors, to tell all threads to exit.
   **
   **     changed by main thread, read by others, may need to be volatile
   */
   bool        Running;

   void SendStatus( unsigned char type, short ierr, char *note );

   // ReleaseServiceThreadInfo() -- enables
   void ReleaseServiceThreadInfo( SOCKET p_descriptor )
   {
      if ( 0 < ThreadsInfo.count(p_descriptor) )
      {
         ThreadsInfo.erase(p_descriptor);
      }
   }

   /* PrepareToRun() -- actions to take prior to entering main loop
   **
   **   ALWAYS call base class's PrepareToRun() at the top
   **
   ** RETURN:  true if ready
   **          false if some condition prevents proper execution
   */
   virtual bool PrepareToRun()
   {
      LoggingLevel = TGlobalUtils::GetLoggingLevel();
      return true;
   }

   /* MainThreadActions() -- override to implements actions performed during the main
   **                        thread's loop (other than sending heartbeats, which are
   **                        handled by other code.
   **                        This is made virtual since some servers may look for input
   **                        from a ring, some may look into the database, while others
   **                        may only respond to requests from clients.
   */
   virtual WORM_STATUS_CODE MainThreadActions() { return WORM_STAT_SUCCESS; }

   /* FinishedRunning() -- actions to take after exiting main loop
   **
   **   ALWAYS call base class's FinishedRunning() at the top
   */
   virtual void FinishedRunning() { }

   /* ClientServicer() -- method to perform the work of servicing a client
   **
   ** note: THREAD_RETURN is some kind of void, so just return or leave block
   */
   // note: THREAD_RETURN is some kind of void, so just return or leave block
   //
   virtual THREAD_RETURN ClientServicer( void * p_socketdescriptor ) = 0;

   // MyThreadFunction is a pointer to one of the member functions of this class
   // (WormServerBase) that takes no parameters and returns THREAD_RETURN
//   THREAD_RETURN (WormServerBase::*MyThreadFunction)(void * p_argument);

public:

   // =======================================================================
   //                  from TConfigurable
   // =======================================================================

   /*
   **  HandleConfigLine()
   **
   **  PARMS:
   **          p_parser -- the parser being used, command string already
   **                      in the current token for comparison with Its()
   **
   ** RETURN:
   **          HANDLE_INVALID --  line invalid
   **          HANDLE_UNUSED  --  line not used
   **          HANDLE_USED    --  line used okay
   **
   **  Override for child classes to handle command lines
   */
   HANDLE_STATUS HandleConfigLine( ConfigSource * p_parser );

   // =======================================================================
   //                  from ThreadableObject
   // =======================================================================

   /* StartThreadFunc -- used by ThreadableObject global function to reenter
   **                    this class in a new thread
   */
   void StartThreadFunc( void * p_arg )
   {
      // Determining which method to enter for this server is simple:
      // if p_arg is NULL, then this is the call to start the
      // listener thread.
      // if p_arg is not null, then this is a call to start a
      // client service thread (p_arg is the socket descriptor,
      // used to access the thread info in the map)
      if ( p_arg == NULL )
      {
         Listener(p_arg);  // p_arg is ignored
      }
      else
      {
         ClientServicer(p_arg);
      }
   }

   // =======================================================================
   //                  WormServerBase
   // =======================================================================

   // Call this base class constructor before the constructors
   // for any derived classes, thusly:
   //
   //    <derived-class>(const char * p_filename) : WormServerBase(p_filename)
   //    {
   //       .... code for derived class constructor
   //    }
   //
   WormServerBase();

   ~WormServerBase();

   // Run() -- starts the listener thread, then parses incoming messages,
   //          shutting everything down as needed.
   WORM_STATUS_CODE Run();

};

#endif // EWserverbaseH
