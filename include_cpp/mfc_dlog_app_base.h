// mfc_dlog_app_base.h : main header file for base class of appliation that uses
//                       a dialog-based style
//

#if !defined(AFX_MFC_DLOG_APP_BASE_H__3022A762_36E0_42B6_A746_40C4AA5B66BA__INCLUDED_)
#define      AFX_MFC_DLOG_APP_BASE_H__3022A762_36E0_42B6_A746_40C4AA5B66BA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#include <configurable.h>
#include <worm_statuscode.h>
#include <worm_exceptions.h>
#include <logger.h>
#include <globalutils.h>

/*
** Declare function used by spawned threads to reenter derivative classes
*/
UINT AFX_CDECL StartMFCWorkerThread(LPVOID p_object);


/////////////////////////////////////////////////////////////////////////////
// CMFCDialogAppBase:
// See mfc_dlog_app_base.cpp for the implementation of this class
//

class CMFCDialogAppBase : public CWinApp, public TConfigurable
{
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
   // virtual HANDLE_STATUS HandleConfigLine( ConfigSource * p_parser );

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
   //virtual void CheckConfig();


   // ------------------------  FOR  CMFCDialogAppBase  ---------------------
   //

   bool Running;

   int  LoggingLevel;

   /*
   ** PrepApp -- actions to be taken to prepare a specific application
   **            for parsing the command file.
   **
   ** OVERRIDE in deriving classes as needed,
   **          always call <super_class>::PrepApp() when overridden
   **          (Except when parent is CMFCDialogAppBase, because that always
   **          returns true, below). 
   */
   virtual bool PrepApp(const char * p_configfilename) { return true; } 


   bool ParseCommandFile( LPTSTR p_filename );

   /*
   ** InitApp -- actions to be taken to initialize a specific application
   **            including the creation of the main form
   **
   ** OVERRIDE in deriving classes as needed,
   **          always call <super_class>::InitApp() when overridden
   **          (Except when parent is CMFCDialogAppBase, because that always
   **          returns true, below). 
   */
   virtual bool InitApp() { return true; } 
   
   /*
   ** GetMainWindow -- after initialization, this is used to return a pointer
   **                  to the main form, which is tied into the event system
   **                  to handle shutdowns on dialog closure.
   */
   virtual CWnd * GetMainWindow() = 0;
   

   /*
   ** OpenMainDialog -- after initialization, this is used by the
   **                   deriving class to 
   */
   virtual void OpenMainDialog() = 0;

   virtual char * GetApplicationName() = 0;

public:
	CMFCDialogAppBase();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMFCDialogAppBase)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CMFCDialogAppBase)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

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
   virtual UINT StartWorkerThread() { return 0; }


};



/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MFC_DLOG_APP_BASE_H__3022A762_36E0_42B6_A746_40C4AA5B66BA__INCLUDED_)
