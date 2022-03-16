/*
** RingReaderServer -- a class of server that reads messages from a Ring
**                     as input.
**                     This class remains virtual because it does not
**                     implement method ClientServicer()
*/
//---------------------------------------------------------------------------
#ifndef ringreaderserverH
#define ringreaderserverH
//---------------------------------------------------------------------------
#include "serverbase.h"

extern "C" {
#include <transport.h>
}




#define SERVE_MAX_LOGOS  20 // maximum incoming logo ids


// sacred definitions for message types to report to heartbeat/status ring
//
#define  ERR_MISSMSG  0 /* message missed in transport ring   */
#define  ERR_TOOBIG   1 /* retreived msg too large for buffer */
#define  ERR_NOTRACK  2 /* severe weirdness in the cellar     */
#define  ERR_QUEUE    3 /* error queueing message for sending */

//---------------------------------------------------------------------------
class RingReaderServer : public WormServerBase
{
private:
   int           MaxMessageLength;
   char *        MessageBuffer;    // buffer for arriving messages
   
protected:

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
   virtual void CheckConfig();

   // =======================================================================
   //                  WormServerBase
   // =======================================================================

   /* PrepareToRun() -- actions to take prior to entering main loop
   **
   **   ALWAYS call base class's PrepareToRun() at the top
   **
   ** RETURN:  true if ready
   **          false if some condition prevents proper execution
   */
   bool PrepareToRun();

   /* MainThreadActions() -- override to implements actions performed during the main
   **                        thread's loop (other than sending heartbeats, which are
   **                        handled by other code.
   **                        This is made virtual since some servers may look for input
   **                        from a ring, some may look into the database, while others
   **                        may only respond to requests from clients.
   **
   ** For DisplayServer class, purge old quakes when they expire from the history
   */
   WORM_STATUS_CODE MainThreadActions();

   /* FinishedRunning() -- actions to take after exiting main loop
   **
   **   ALWAYS call base class's FinishedRunning()
   **
   **  For DisplayServer class, just use base class's
   */
   void FinishedRunning();


   /* ClientServicer() -- method to perform the work of servicing a client
   **
   ** note: THREAD_RETURN is some kind of void, so just return or leave block
   */
   // virtual THREAD_RETURN ClientServicer( void * p_socketdescriptor );


   // =======================================================================
   //                  RingReaderServer
   // =======================================================================

   int           LoggingLevel;

   // Ring to check for shutdown messages, to post heartbeats, etc.
   WORM_RING_NAME InputRingName;	// name of transport ring for message input
   WORM_RING_ID   InputRingKey;  // key to transport ring to read messages from
   SHM_INFO       InputRegion;   // Info structure for shared memory (input ring)

   MSG_LOGO		  ServeLogo[SERVE_MAX_LOGOS];
   short         ServeLogoCount;

   // MessageMutex -- mutex to prevent collisions over the MessageBuffer,
   //                 should be used in both MessageFromRing()
   //                 and ClientServicer()
   //
   //          MessageMutex.RequestLock();
   //          MessageMutex.ReleaseLock();
   //
   TMutex  *     MessageMutex;

   /*
   **  MessageFromRing() -- method for derivative classes to use to
   **                       perform the actual handling of message
   **                       as they arrive on the ring.
   */
   virtual bool MessageFromRing( const MSG_LOGO p_msglogo, const char * p_msg ) = 0;

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
   //                  RingReaderServer
   // =======================================================================

   RingReaderServer();
   ~RingReaderServer();

};

#endif
 