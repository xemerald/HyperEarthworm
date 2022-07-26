// mutableserverbase.h: interface for the MutableServerBase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(MUTABLESERVERBASE_H)
#define MUTABLESERVERBASE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// microsoft pragma to avoid warnings relating to the container
// template names being too long
#pragma warning(disable:4786)

#include "serverbase.h"
#include <mutableservermessage.h>
#include <mutableserverrequest.h>
#include <mutableserverresult.h>
#include <result_status.h>
#include <string>
#include <vector>
#include <queue>


// Descriptive strings for these are
// held in static class variable MSB_MODE_NAME[],
// defined in mutableserverbase.cpp.
//
enum MUTABLE_MODE_TYPE
{
   MMT_NONE       = 0  // for validation
 , MMT_STANDALONE = 1  // command from args/stdin, data access and calculations internally
 , MMT_MODULE     = 2  // command from ring, data access and calculations internally [MT]
 , MMT_SERVER     = 3  // command from socket, calculations internally [MT]
 , MMT_CLIENT     = 4  // command from atgs/stdin, socket call to client for calculations
};


typedef struct _PROVIDER_ADDR
{
   std::string  IPAddr;
   std::string  Port;
} PROVIDER_ADDR;

typedef std::vector<PROVIDER_ADDR> PROVIDER_VECTOR;



#define SERVE_MAX_LOGOS  20 // maximum incoming logo ids


// sacred definitions for message types to report to heartbeat/status ring
//
// 
#define  MSB_ERR_MISSMSG  0 /* message missed in transport ring   */
#define  MSB_ERR_TOOBIG   1 /* retrieved msg too large for buffer */
#define  MSB_ERR_NOTRACK  2 /* severe weirdness in the cellar     */
#define  MSB_ERR_QUEUE    3 /* error queueing message for sending */
#define  MSG_ERR_BASE    10 /* base value for derivative classes to start
                            ** declaring their own error codes
                            */


// ----------------------------------------------------------------------
class MutableServerBase : public WormServerBase 
{
private:
   int           MaxMessageLength;
   char *        MessageBuffer;    // buffer for arriving messages
   

   // - - - - - - - - - - - - - - - - - - - - - - - - -
   // Client (sending to Server across socket, await response)
   //
   WORM_STATUS_CODE TransmitRequest( MutableServerRequest * p_request
                                   , MutableServerResult  * r_result
                                   );


   // - - - - - - - - - - - - - - - - - - - - - - - - -
   // Server (socket) or Module (ring)
   //
   // if p_result is NULL, this will transmit a message or the appropriate type
   // with status = MSB_RESULT_ERROR (system failure)
   //
   WORM_STATUS_CODE TransmitResult(       MutableServerResult * p_result
                                  , const SOCKET              * p_socketdescriptor = NULL
                                  );

protected:

   // =======================================================================
   //                  from TConfigurable
   // =======================================================================

   /* CheckConfig() -- allows derivative classes to report the status of their
   **                  the lookup values.
   **
   ** From within any deriving class, or further derivation, ALWAYS contain a call to
   ** <super_class>::CheckConfig() in their own CheckConfig() method...
   ** except that MutableServerBase::CheckConfig() should NOT call
   ** WormServerBase::CheckConfig().
   ** This ensures that all classes in the heirarchy get their chance to report status.
   ** Furthermore, any derivative classes that need to have the Mode set before
   ** doing their own CheckConfig(), should call MutableServerBase::CheckConfig()
   ** at the top of theirs.
   **
   ** All implementations should set ConfigStatus value to WORM_STAT_BADSTATE if there
   ** is a configuration problem, otherwise leave it alone.
   **
   */
   void CheckConfig();

   // =======================================================================
   //                  WormServerBase
   // =======================================================================

   /*
   ** PrepareToRun() -- sets LoggingLevel
   */
   virtual bool PrepareToRun() { return WormServerBase::PrepareToRun(); }


   /* ClientServicer() -- method to perform the work of servicing a client
   **
   ** note: THREAD_RETURN is a kind of void, so just return or leave the method.
   */
   THREAD_RETURN ClientServicer( void * p_socketdescriptor );


   // =======================================================================
   //                  MutableServerBase
   // =======================================================================


   // Array of lookup names
   //
   static const char * MSB_MODE_NAME[];

   // Mode is set in 
   //
   MUTABLE_MODE_TYPE  Mode;

   // - - - - - - - - - - - - - - - - - - - - - - - - -
   // Server
   //
   // see WormServerBase()

   unsigned short MaxClients;


   // - - - - - - - - - - - - - - - - - - - - - - - - -
   // Client
   //
   PROVIDER_VECTOR Providers;

   // Attempt connection to providers in a round-robin
   // fashion until connection achieved or until this
   // number of seconds has passed
   //
   int             ConnectFailureSec;

   // max times that each server will be queried before
   // client finally throws in the towel.
   int             MaxServerTryLoopCount;

   // - - - - - - - - - - - - - - - - - - - - - - - - -
   // Module
   //
   std::string  InputRingName
             ,  OutputRingName
             ;
   WORM_RING_ID InputRingKey
             ,  OutputRingKey
             ;
   SHM_INFO   * InputRegion      // pointer to SHM_INFO used for input
          ,   * OutputRegion     // pointer to SHM_INFO used for output
          ,     InputRegionStruct
          ,     OutputRegionStruct
          ;

   MSG_LOGO		  ServeLogo[SERVE_MAX_LOGOS];
   short         ServeLogoCount;

   char        * TransmitBuffer;
   long          TransmitBufferLength;


   // MessageMutex -- mutex to prevent collisions over the MessageBuffer,
   //                 should be used in both Stacker()
   //                 and Handler()
   //
   //          MessageMutex.RequestLock();
   //          MessageMutex.ReleaseLock();
   //
   TMutex  *     MessageMutex;

   std::queue<std::string> MessageQueue;

   TO_THREAD_ID  StackerThreadId;
   long          LastStackerPulse;

   THREAD_RETURN Stacker();

   TO_THREAD_ID  HandlerThreadId;
   long          LastHandlerPulse;

   THREAD_RETURN Handler();

   MSG_LOGO      ResultMsgLogo;

   // OutputMessageTypeKey() -- Returns the string used to identify 
   //                           result messages sent in Module mode
   //                           via the ring (e.g.:  TYPE_XXXXXXX).
   //                           This is intended to allow different
   //                           derivative servers to emit different
   //                           message types.
   //
   virtual const char * OutputMessageTypeKey() = 0;


   // - - - - - - - - - - - - - - - - - - - - - - - - -
   // Server or Module (required for Module)
   //
   // in WormServerBase

//   std::string  CommandRingName;
//   WORM_RING_ID CommandRingKey;
//   SHM_INFO     CommandRegion;

   int           LoggingOptions;


   // returns true if a processing thread died
   //         false if all still okay
   //
   bool CheckForThreadDeath();

   /*
   ** Server and Module enter this to start worker threads
   **                   and start main loop
   **                   (sending heartbeats, checking shut
   */
   WORM_STATUS_CODE MainThreadActions();


   /*
   ** GetMaxSocketBufferSize() -- Server mode uses this to allocate
   **                             a buffer of sufficient size to
   **                             parse the arriving and format
   **                             the outgoing message
   */
   virtual long GetMaxSocketBufferSize() = 0;


   /*
   ** GetRequestContainer() -- gets a container to hold the request
   **                          (e.g. passport & event info).
   **                          Since we don't know what all
   **                          possible content that can be in a
   **                          request for the various derivative
   **                          classes of servers, the container
   **                          is returned as a void pointer,
   **                          This base class code does not need
   **                          to know about the container, only
   **                          the following virtual methods do.
   **
   ** RETURNS:
   **      a pointer to the container
   **   or NULL on failure
   */
   virtual MutableServerRequest * GetRequestContainer() = 0;

   virtual MutableServerResult * GetResultContainer() = 0;


   // - - - - - - - - - - - - - - - - - - - - - - - - -
   // Standalone or Client
   //
   // Gets request parameters/instructions from command line args
   // or stdin
   //
   // r_container = a pointer to the appropriate type of
   //           MutableServerRequest derivative class
   //           (cast to the specific type herein).
   //
   virtual WORM_STATUS_CODE GetRequestFromInput( int    p_argc
                                               , char * p_argv[]
                                               , void * r_container
                                               ) = 0;
   
   // - - - - - - - - - - - - - - - - - - - - - - - - -
   // Server, Module or Standalone
   //
   virtual WORM_STATUS_CODE ProcessRequest( void * p_request
                                          , void * r_result
                                          ) = 0;


   // - - - - - - - - - - - - - - - - - - - - - - - - -
   // Client, Module or Standalone
   //
   //  p_result is a pointer to the appropriate type of
   //           MutableServerResult derivative class
   //           (cast to the specific type herein).
   //
   // MUST RETURN:
   //      WORM_STAT_SUCCESS  = good results
   //      WORM_STAT_BADSTATE = algorithmic, not system, failure (no results obtained)
   //      WORM_STAT_FAILURE  = system failure
   //
   virtual WORM_STATUS_CODE HandleResult( void * p_result ) = 0;


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
   **  Call <super_class::HandleConfigLine() within derivative classes
   **  to allow them a whack at the parameter as well.
   */
   HANDLE_STATUS HandleConfigLine( ConfigSource * p_parser );

   // =======================================================================
   //                  from ThreadableObject
   // =======================================================================

   /* StartThreadFunc -- used by ThreadableObject global function to reenter
   **                    this class in a new thread
   */
   void StartThreadFunc( void * p_arg );


   // =======================================================================
   //                  for MutableServerBase
   // =======================================================================
   
   MutableServerBase();
	~MutableServerBase();

   // Run( int argc, char* argv[] )
   //
   //    Not quite an override of the ServerBase:;Run() method.
   //    This one must switch on the mode, and start thread or
   //    call methods as appropriate.
   //
   // Included the parameters to enable derivative classes to
   // get what they need from the command line or from stdin
   // as preferred.
   //
   WORM_STATUS_CODE Run( int argc, char* argv[] );

};

#endif // !defined(MUTABLESERVERBASE_H)
