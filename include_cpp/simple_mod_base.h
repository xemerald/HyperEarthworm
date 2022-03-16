/*
 * This is a base class which serves as a template providing
 * basic Earthworm module functionality.
 * 
 * It provides the basis for parameter file parsing, logging
 * a dual-threaded message-stacker / message-handler processing
 * model, standardized message transmission.
 * 
 */
#ifndef _SMPL_CPP_MOD_BASE_H_INCLUDED
#define _SMPL_CPP_MOD_BASE_H_INCLUDED

extern "C" {
#include "platform.h"
#include "earthworm_defs.h"
#include "earthworm_simple_funcs.h"
#include "transport.h"        /* Ring stuff  */
#include "mem_circ_queue.h"
}

#include <time.h>             /* time() -- for worker thread heartbeats  */
#define _USING_EW_XPORT 1     /* prevent conflict between transport.h and worm_types.h */
#include <worm_types.h>
#include <worm_defs.h>
#include <worm_exceptions.h>
#include <configurable.h>
#include <threadableobject.h>

//#pragma warning(disable:4786)    // microsoft pragma to avoid messages from cpp stdlib
//#include <string>                // std::string
//#include <queue>                 // std::queue

enum WORKER_THREAD_STATUS
{
   THREAD_STATUS_ERROR     = -1
 , THREAD_STATUS_STARTING  =  0  // set by [main] thread starting the new one
 , THREAD_STATUS_INIT      =  1  // set by worker for any initialization
 , THREAD_STATUS_GOOD      =  2
 , THREAD_WAITING          =  3  // waiting for something to do
 , THREAD_PROCESSING       =  4  // doing something
 , THREAD_STATUS_QUIT      = 10  // set by worker
};

typedef struct _WorkThreadInfoStruct
{
   TO_THREAD_ID             threadid;
   WORKER_THREAD_STATUS     status;
   time_t                   lastpulse; // = time(NULL) during each pass of ClientServicer() loop
} WorkThreadInfoStruct;

#define THREAD_STACK  (unsigned int)8096

#define WORK_THREAD_DEAD_SEC  10  /* number of seconds without worker thread pulse before considered dead */

// Default Logging Level
#define DEF_LOGGING_LEVEL  WORM_LOG_DETAILS

// Default Heartbeat interval (seconds)
#define DEF_HEARTBEAT_INTV_SEC  10

// Default milliseconds to sleep between main loop executions
#define DEF_LOOP_SLEEP_MS  1000

// Default millisecond sleep time for Stacker thread (if no new message)
#define DEF_STACK_SLEEP_MS  500

// Default millisecond sleep time for Handler thread (if no new message)
#define DEF_HANDLE_SLEEP_MS  400

/*
 * Error codes to report for alarms
 */
#define SMB_ERR_QUEUE_WRAPPED 1
#define SMB_ERR_MSG_TOO_LONG  2

//---------------------------------------------------------------------------

class SimpleModuleBase : public ThreadableObject
                       , public TConfigurable
{
private:
   // Members in the private: area cannot be accessed by deriving classes
   // (except through accessor methods in the protected area)
   
   
   WORM_MSGTYPE_ID    TYPE_HEARTBEAT
                 ,    TYPE_ERROR
                 ;

   /*
    * HBIntervalSec -- Maximum rate (seconds) at which heartbeats will be sent.
    *                  If < 1, no heartbeats will be sent.
    */
   int                HBIntervalSec;

   unsigned int       MainLoopSleepMS;

   /*
    * At least the command ring is actually required.
    * 
    * The input ring may be the same as the command ring,
    * the output ring may be the same as the input ring.
    */
   // Ring to check for shutdown flags, to post heartbeats, etc.
   WORM_RING_NAME     CommandRingName;   // name of transport ring
   WORM_RING_ID       CommandRingKey;    // key to transport ring
   SHM_INFO           CommandRing;       // Info structure for shared memory (command ring)


   // Ring for arriving messages.
   WORM_RING_NAME     InputRingName;   // name of transport ring
   WORM_RING_ID       InputRingKey;    // key to transport ring
   SHM_INFO           InputRing;       // Only used if InputRingKey != CommandRingKey

   // Ring for created messages.
   WORM_RING_NAME     OutputRingName;   // name of transport ring
   WORM_RING_ID       OutputRingKey;    // key to transport ring
   SHM_INFO           OutputRing;       // Only used if OutputRing != ( InputRingKey or CommandRingKey )
   
   
   unsigned int       StackerSleepMS
              ,       HandlerSleepMS
              ;

   
   /* Status and pulse info for worker threads */
   WorkThreadInfoStruct   StackerInfo;
   WorkThreadInfoStruct   HandlerInfo;
   
   short                  LogoAlloc;        /* allocated element count of InputLogoList */
   short                  InputLogoCount;   /* elements used in InputLogoList */
   MSG_LOGO             * InputLogoList;    /* list of logos desired from input ring */
   
   
   unsigned long          MaxMessageSize;   /* maximum length of message */
   unsigned long          MaxQueueElements; /* max messages in internal queue */   
   mutex_t                QueueMutex;       /* mutex to keep Stacker and Handler from stomping on each other */
   QUEUE                  MessageQueue;     /* queue between stacker and handler */

   /*
    * Methods in which the worker threads run.
    * 
    * Handler() calls MessageHandling() when a message is available.
    */
   THREAD_RETURN          Stacker();
   THREAD_RETURN          Handler();

protected:

   // =======================================================================
   //                  from TConfigurable
   // =======================================================================

   /*
    *  HandleConfigLine()
    *
    *  PARMS:
    *          p_parser -- the parser being used, command string already
    *                      in the current token for comparison with Its()
    *
    * RETURN:
    *          HANDLE_INVALID --  line invalid
    *          HANDLE_UNUSED  --  line not used
    *          HANDLE_USED    --  line used okay
    *
    * Override for child classes to handle other parameters, but
    * ALWAYs call <parent_class>::HandleConfigLine() therein.
    */
   virtual HANDLE_STATUS HandleConfigLine( ConfigSource * p_parser );


   /* CheckConfig() -- allows derivative classes to report the status of their
    *                  the lookup values.
    *
    * From within any deriving class, or further derivation, ALWAYS contain a call to
    * <super_class>::CheckConfig() in their own CheckConfig() method...
    * this ensures that all classes in the heirarchy get their chance to report status.
    *
    * All implementations should set ConfigStatus value to WORM_STAT_BADSTATE if there
    * is a configuration problem, otherwise leave it alone.
    */
   virtual void CheckConfig();

      // =======================================================================
   //                   for SimpleModuleBase
   // =======================================================================

   PROGRAM_NAME  ProgramName;
   LOG_DIRECTORY HomeDirectory;
   char          Version[30];

   WORM_INSTALLATION_ID  MyInstallId;
   WORM_MODULE_ID        MyModuleId;

   /*
    * DEF_LOGGING_LEVEL until after configuration file
    */
   WORM_LOGGING_LEVEL    LoggingLevel;
   
   SHM_INFO         *    CommandRegion;   // Info structure for shared memory (command ring)
   SHM_INFO         *    InputRegion;     // Info structure for shared memory (input ring)
   SHM_INFO         *    OutputRegion;    // Info structure for shared memory (output ring)


   /* Running -- flag indicating if the main loop is running,
    *            set to false by terminate message, or various
    *            errors, to tell all threads to exit.
    */
   volatile bool          Running;
   
   /* EnsureMessageSize() -- ensure that messages of size
    *                        can be accepted.
    * 
    * @ return 0 = okay
    *         -1 = error (already running, can reset it now)
    */
   int EnsureMessageSize( unsigned long p_maxsize );
   
   
   /*
    * SendMessage() -- method to send 
    */
   void SendMessageToRing(       SHM_INFO    * p_ring
                         ,       MSG_LOGO    * p_logo
                         , const long          p_messagelength
                         ,       char        * p_messagetext
                         );

   /*
    * SendHeartbeat() -- sends a TYPE_HEARTBEAT message, checking against
    *                    HBIntervalSec (in private section, above)
    *                    to limit HB message rate.
    *                    (Does not check thread status).
    */
   void SendHeartbeat();
   
   /*
    * SendError() -- sends a TYPE_ERROR message, consisting of an error number
    *                and optional text, to the command ring.
    * 
    * @ param p_messagetext anything over 200 char, will be truncated.
    */
   void SendError( const short    p_errornumber
                 , const char   * p_messagetext /*  = NULL */
                 );

   // =======================================================================
   //  METHODS FOR DERIVATIVE CLASSES TO OVER-RIDE TO EXTEND FUNCTIONALITY
   //  AS NEEDED.
   // =======================================================================
   
   /* PrepareToRun() -- Called within the Run() method to enable derivative
    *                   classes need to take any action they need before
    *                   the worker threads are started and the main
    *                   loop is entered.
    *
    * ALWAYS call <parent_class>::PrepareToRun() at the top
    *
    * @ return  true if ready
    *          false if some condition prevents proper execution
    */
   virtual bool PrepareToRun();

   /* 
    * MainThreadActions() -- override to implements actions performed during the main
    *                        thread's loop (within the Run() method)
    *                        Don't perform time-intensive operations herein, because
    *                        that could delay module heartbeats.  Alternately, call
    *                        SendHeartbeat().
    *
    * ALWAYS call <parent_class>::MainThreadActions() at the top
    * 
    * @ return
    *     WORM_STAT_SUCCESS = good actions
    *                         The main thread will immediately repeat its loop
    *     WORM_STAT_NOMATCH = no actions to take (good return)
    *                         The main thread will sleep before repeating its loop
    *     WORM_STAT_FAILURE = actions resulted in, or encountered a fatal error
    *                         The main thread will leave its loop, set Running = false
    *                         (to terminate worker threads) and return an error
    *                         code to the Run() method.
    */
   virtual WORM_STATUS_CODE MainThreadActions() { return WORM_STAT_NOMATCH; }

   /* CheckForFatal -- allows the main loop in the Run() method to query derived
    *                  classes if a fatal error has been encountered.
    *                  (Status of the worker threads is checked elsewhere.)
    *
    * ALWAYS call <parent_class>::CheckForFatal() at the top
    *
    * @ return  true if an error occurred and must exit.
    *          false if no fatal error has occurred.
    */
   virtual bool CheckForFatal();

   /* FinishedRunning() -- actions to take after exiting main loop in the
    *                      Run() method.
    *
    *   ALWAYS call <parent_class>::FinishedRunning() at the top
    */
   virtual void FinishedRunning() { }
   
   /*
    * WantMessage() -- virtual method to all derivative classes to perform 
    *                  additional checking of message to determine if it
    *                  is desired for processing, this is performed before
    *                  the message is put onto the internal buffer by the
    *                  Stacker() method.
    *                  This is intended to support the case where the Logo
    *                  is not sufficient for such a determination (for
    *                  example, when only certain SCN[L]s are desired for
    *                  TRACE_BUF messages).
    */
   virtual bool WantMessage( const MSG_LOGO   p_logo
                           , const int        p_messagelength
                           ,       char     * p_messagebody
                           ) { return true; }    

   // =======================================================================
   //  MAIN METHOD FOR DERIVATIVE CLASSES TO PROVIDE APPLICATION-SPECIFIC
   //  MESSAGE HANDLING
   // =======================================================================

   /*
    * MessageHandler() -- method for derivative classes to override to
    *                     handle their specific message handling.
    *                     Every time a message matching the input logos
    *                     is stacked and pulled off the queue, it is
    *                     sent here to be processed.
    * 
    * @ return  true = okay
    *          false = fatal error
    */
   virtual bool MessageHandler( const MSG_LOGO   p_logo
                              , const int        p_messagelength
                              ,       char     * p_messagebody
                              ) = 0;

public:

   // =======================================================================
   //                  from ThreadableObject
   // =======================================================================

   /*
    * StartThreadFunc -- used by ThreadableObject global function to reenter
    *                    this class in a new thread.
    * 
    * Deriving classes might want to override this method if they have
    * other specialized threads.
    * 
    * NOTE: Never call this from any code, except from within ThreadableObject.cpp.
    */
   virtual void StartThreadFunc( void * p_arg )
   {
      if ( p_arg != NULL )
      {
         switch ( *((int *)p_arg) )
         {
           case 1:
logit( "o", "StartThreadFunc(): Starting stacker\n" );
                Stacker();
                break;
           case 2:
logit( "o", "StartThreadFunc(): Starting handler\n" );
                Handler();
                break;
         }
      }
   }

   // =======================================================================
   //                   for SimpleModuleBase
   // =======================================================================

   /*
    * Run() -- calls PreptoRun(), starts the stacker and handler threads,
    *          then monitors the command ring for shutdown flags,
    *          and checks the worker thread heartbeats and status
    *          for error indications.
    * 
    * Generally, do not override this method.
    */
   WORM_STATUS_CODE Run( const char * p_programName
                       , const char * p_configFilename = NULL
                       );

   /*
    * 
    * CAUTION:  logit_init() must have been called before
    *           creating an instance of this class or any
    *           derivatives thereof.
    * 
    * Can throw worm_exception.
    */
   SimpleModuleBase();
   
   ~SimpleModuleBase();

};

#endif /* _SMPL_CPP_MOD_BASE_H_INCLUDED */
