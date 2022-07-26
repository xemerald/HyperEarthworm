// mfc_dlog_modl_base.h: interface for the CMFCDialogModuleBase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MFC_DLOG_MODL_BASE_H__5F18DF8E_C017_46E1_8328_7C2D2BBAA0C1__INCLUDED_)
#define AFX_MFC_DLOG_MODL_BASE_H__5F18DF8E_C017_46E1_8328_7C2D2BBAA0C1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _USING_EW_XPORT 1 // avoid earthworm<-->magworm conflict

#include "mfc_dlog_app_base.h"

extern "C" {
#include <transport.h>
}

// sacred definitions for message types to report to heartbeat/status ring
//
#define  ERR_MISSMSG  0 /* message missed in transport ring   */
#define  ERR_TOOBIG   1 /* retreived msg too large for buffer */
#define  ERR_NOTRACK  2 /* severe weirdness in the cellar     */
#define  ERR_QUEUE    3 /* error queueing message for sending */


#define SERVE_MAX_LOGOS  20 // maximum incoming logo ids

/////////////////////////////////////////////////////////////////////////////////

class CMFCDialogModuleBase : public CMFCDialogAppBase  
{
public:
   // ------------------------  FOR  CMFCDialogAppBase  ---------------------
   //
   /*
   ** StartWorkerThread -- The point at which worker threads reenter
   **                      derived classes.
   **                      The derived class should have class/instance
   **                      variable(s) which are set (to indicate work
   **                      to be done) prior to calling AfxBeginThread()
   **                      and a sleep should imposed between such a
   **                      call and any subsequent changes to the variables
   **                      (to give the spawned thread a chance to read them). 
   **
   **
   ** OVERRIDE This method in derived classes as needed.
   */
	UINT StartWorkerThread();

   // ------------------------  FOR  CMFCDialogModuleBase  ---------------------
   //

   CMFCDialogModuleBase();

	virtual ~CMFCDialogModuleBase();

protected:

   // ------------------------  FOR  TConfigurable  ---------------------
   //
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
   **  OVERRIDE for child classes to handle command (.d) lines
   **          always call <super_class>::HandleConfigLine() when overridden
   **          (except when super class is CMFCDialogAppBase).
   **
   */
   HANDLE_STATUS HandleConfigLine( ConfigSource * p_parser );

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

   // ------------------------  FOR  CMFCDialogAppBase  ---------------------
   //

   /*
   ** PrepApp -- actions to be taken to prepare a specific application
   **            for parsing the command file.
   **
   ** OVERRIDE in deriving classes as needed,
   **          MUST always call <super_class>::PrepApp() when overridden
   **          (Except when parent is CMFCDialogAppBase).
   **
   **  For CMFCDialogAppBase, this: instantiates MyGlobalUtils,
   **                               sets the logging level
   **                               changes the working directory
   */
   bool PrepApp(const char * p_configfilename);


   /*
   ** InitApp -- actions to be taken to initialize a specific application
   **            including the creation of the main form
   **
   **  For CMFCDialogAppBase: starts the worker thread that loops in
   **                         StatusAndReadLoop() to:
   **                         1. check for shutdown flag in command ring
   **                         2. call CheckForFatal()
   **                         3. if input ring specified, checks for message
   **                            and passes any with accepted logo
   **                            to HandleMessage()
   **
   ** OVERRIDE in deriving classes as needed,
   **          always call <super_class>::InitApp() when overridden.
   **          IT IS GENERALLY APPROPRIATE TO call <super_class>::InitApp()
   **          at end of deriviative classe's implementation.
   ** 
   */
	bool InitApp();

   /*
   ** BeforeMessage -- actions before the first message is handled
   */
   virtual bool BeforeMessage();



   // ------------------------  FOR  CMFCDialogModuleBase  ---------------------
   //

   TGlobalUtils * MyGlobalUtils;

   WORM_MSGTYPE_ID  TYPE_ERROR
                 ,  TYPE_HEARTBEAT
                 ;

private:
   int           MaxMessageLength;
   char *        MessageBuffer;    // buffer for arriving messages

protected:

   // Ring to check for shutdown flags, to post heartbeats, etc.
   WORM_RING_NAME CommandRingName;	// name of transport ring
   WORM_RING_ID   CommandRingKey;   // key to transport ring
   SHM_INFO       CommandRegion;    // Info structure for shared memory (command ring)

   // Ring for incoming messages (may be the same as command ring)
   WORM_RING_NAME InputRingName;	// name of transport ring
   WORM_RING_ID   InputRingKey;  // key to transport ring
   SHM_INFO       InputRegion;   // Info structure for shared memory (input ring)

   MSG_LOGO		  AcceptLogo[SERVE_MAX_LOGOS];
   short         AcceptLogoCount;

   /*
   **  HandleMessage() -- method for derivative classes to use to
   **                     perform the actual handling of message
   **                     as they arrive on the ring.
   **
   ** OVERRIDE in derivative classes to as needed.
   */
   virtual bool HandleMessage( const MSG_LOGO   p_msglogo
                             , const char     * p_msg
                             ) { return true; }


   CWinThread * StatusThread;

	void SendStatus( WORM_MSGTYPE_ID   p_type
                  , short             p_ierr = 0
                  , const char      * p_text = NULL
                  );

	void HeartBeat();

   /*
   **  StatusAndReadLoop -- A worker thread enters this method to loop, looking
   **                       for shutdown flag on the command ring, calling
   **                       CheckForFatal() to check for fatal errors on other
   **                       threads, and (if the input ring has been created)
   **                       check for messages to pass to HandleMessage()
   */
	UINT StatusAndReadLoop();


   /*
   ** CheckForFatal -- check any other threads and states for fatal error
   **
   ** RETURN: true on fatal error, false otherwise.
   **
   ** OVERRIDE in deriving classes as needed.
   */
   virtual bool CheckForFatal() { return false; }

};

#endif // !defined(AFX_MFC_DLOG_MODL_BASE_H__5F18DF8E_C017_46E1_8328_7C2D2BBAA0C1__INCLUDED_)
