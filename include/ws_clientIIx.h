/* This file  ws_clientIIx.h
   contains a set of alternate routines to ws_clientII.h for accessing 
   wave_server data.
   These routines focus on simplifying client interaction with wave_servers
   and streamlining the process of retrieving a list of requests from
   a set of wave_servers.
   The ws_clientII routines(ws_clientII.h/ws_clientII.c) are still the
   standard for accessing wave_server data.
   The ws_clientIIx routines are not scheduled to be SCNL'd at this time.
   David 072004
 *********************************************************************/

/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: ws_clientIIx.h 7606 2018-11-26 20:47:34Z alexander $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.9  2007/02/23 15:55:04  paulf
 *     fixed warning for int tNextRefresh
 *
 *     Revision 1.8  2005/04/25 17:39:26  davidk
 *     SCNLized ws_clientIIx.h header file (modeled after SCNL changes
 *     to ws_clientII.h.
 *
 *     Revision 1.7  2004/07/20 18:32:41  davidk
 *     Added comment to top of file indicating that the ws_clientIIx(name changed from
 *     ws_clientIII) routines do not outdate the ws_clientII routines.
 *
 *     Revision 1.6  2004/07/19 20:23:42  davidk
 *     Added include of time.h
 *
 *     Revision 1.4  2003/02/04 17:59:06  davidk
 *     Added debug-level constants.
 *     Changed debug from a simple switch to an overly complex
 *     debug level matrix.
 *
 *     Revision 1.3  2002/02/18 18:35:58  davidk
 *     added iNumSnippetsTimedOut variable to the stats structure.
 *
 *     Revision 1.2  2002/02/12 04:42:02  davidk
 *     Added connection statistics, and other params.
 *
 *     Revision 1.1  2001/01/18 17:15:03  davidk
 *     Initial revision
 *
 *     Revision 1.2  2000/09/29 19:25:45  alex
 *     fixed comment re endtime=0 in getAscii call. Alex
 *
 *     Revision 1.1  2000/02/14 20:05:54  lucky
 *     Initial revision
 *
 *
 */


/* This is the include file for the WaveServerIV clientII routines
and associated structures */

/* 5/17/98: increased wsREPLEN; PNL */

#ifndef WS_CLIENT
#define WS_CLIENT

#include <time.h>
#include <platform.h>
#include <trace_buf.h>

/* Return codes used by routines in ws_client.c
 * Errors (negative values) indicate a problem that will affect other
 * transactions at least with the same server.
 * Warnings (positive values) indicate some problem with a transaction that
 * should not affect other transactions.
*********************************************/
#define WS_WRN_FLAGGED               1   /* reply flagged by server */
#define WS_ERR_NONE                  0   /* All ok */
#define WS_ERR_INPUT                -1   /* Faulty or missing input */
#define WS_ERR_EMPTY_MENU           -2   /* Unexpected empty menu */
#define WS_ERR_SERVER_NOT_IN_MENU   -3   /* Server should have been in menu */
#define WS_ERR_SCNL_NOT_IN_MENU     -4   /* SCNL not found in menu */
#define WS_ERR_BUFFER_OVERFLOW      -5   /* reply truncated at buffer limit */
#define WS_ERR_MEMORY               -6   /* Couldn't allocate memory */
#define WS_ERR_PARSE                -7   /* Couldn't parse server's reply */
/* socket related errors: */
#define WS_ERR_TIMEOUT             -10   /* Socket transaction timed out */
#define WS_ERR_BROKEN_CONNECTION   -11   /* An open connection was broken */
#define WS_ERR_SOCKET              -12   /* problem setting up socket */
#define WS_ERR_NO_CONNECTION       -13   /* Could not make connection */
#define WS_ERR_UNKNOWN             -14   /* Unknown error occurred */
#define WS_ERR_MENU_NOT_FOUND      -15   /* Menu not found in PSCNL list */
#define WS_ERR_LIST_FINISHED       -16   /* All items list already processed */
#define WS_ERR_TEMPORARY           -17   /* Uh, there was a problem, retry */
/* Trace format types */
#define WS_TRACE_BINARY              2
#define WS_TRACE_ASCII               3

/* Wave server status types */
#define WS_SERVER_STATUS_HEALTHY     0
#define WS_SERVER_STATUS_ERROR      -1


/* WS_CLIENT DEBUG LEVELS */
#define WS_DEBUG_NONE               0x00
#define WS_DEBUG_SERVER_INFO        0x01
#define WS_DEBUG_SERVER_WARNINGS    0x02
#define WS_DEBUG_SERVER_ERRORS      0x04
#define WS_DEBUG_SERVER_STATISTICS  0x08
#define WS_DEBUG_DATA_INFO          0x10
#define WS_DEBUG_DATA_WARNINGS      0x20
#define WS_DEBUG_DATA_ERRORS        0x40
#define WS_DEBUG_OVERKILL           0x80
#define WS_DEBUG_ALL                0xFFFFFFFF


/* string buffer lengths */
#define wsADRLEN    64    /* length for ip and port adrress strings */
#define wsREQLEN   256    /* length for MENU request */
#define wsREPLEN  20480   /* length for reply to MENU request(up to 256 SCNLs) */


/* typedef pointers to structures, so that they can be used inside
   the structures when the structures are declared. */
typedef struct _WS_PSCNL_REC *WS_PSCNL;
typedef struct _WS_MENU_REC *WS_MENU;
typedef struct _wsEnvironmentStruct * wsHANDLE;

/* Trace Request structure 
 *************************/
/* the structure below is the 'form' which must be filled out to request a 
   snippet of trace data. The requester fills out the first part, and the
   client routines fill out the second part. */
typedef struct
{
	/* the request portion */
  char    sta[TRACE2_STA_LEN];         /* Site name */
  char    chan[TRACE2_CHAN_LEN];       /* Component/channel code */
  char    net[TRACE2_NET_LEN];         /* Network name */
  char    loc[TRACE2_LOC_LEN];         /* Location code */
  short   pinno;          /* Pin number that maps to sta/net/chan */
	double	reqStarttime;	/* requested starttime - seconds since 1970 */	
	double	reqEndtime;	/* requested endtime - seconds since 1970 */
	int	partial;	/* 1=> I'll accept partial data;
                                   0=> all or nothing 
				*** NOT IMPLEMENTED ***			*/	
	char*	pBuf;		/* pointer to user supplied buffer for trace */
	unsigned long	bufLen;	/* length of above */
	long 	timeout;	/* seconds after which we must return */
	long	fill;		/* ASCII only: fill value for gaps in trace */

	/* the reply portion */
	char	retFlag;	/* return flag (if any) from wave server */
	double	waitSec;	/* seconds to wait until requested data will be
				   available in the wave server.
                                   *** NOT IMPLEMENTED */
	double actStarttime;	/* actual start time of supplied trace */
	double actEndtime;	/* actual end   time of supplied trace 
				   *** NOT IMPLEMENTED in wsGetTraceAscii() */
	long   actLen;		/* actual length in bytes */
	double samprate;	/* nominal sampling rate - samples per second */
	
  WS_MENU menu;
  void *  pClientData;
  int     wsError;
}
TRACE_REQ;


typedef struct _WS_PSCNL_REC
{
  char    sta[TRACE2_STA_LEN];         /* Site name */
  char    chan[TRACE2_CHAN_LEN];       /* Component/channel code */
  char    net[TRACE2_NET_LEN];         /* Network name */
  char    loc[TRACE2_LOC_LEN];         /* Location code */
  short   pinno;          /* Pin number that maps to sta/net/chan/loc */
  double  tankStarttime;  /* starttime for this SCNL */	
  double  tankEndtime;    /* endtime for this SCNL */	

  /* added for ws_clientIII DK III */
  int     iNumMenus;
  WS_MENU MenuList[4]; /* list of menus that contain this SCNL */
 
  WS_PSCNL next;
} WS_PSCNL_REC;


typedef struct _WS_SERVER_CONNECTION_STATS_REC
{
  int    iNumTotalConnectionsAttempted;
  int    iNumTotalConnections;
  double dTotalTimeConnected;
  double dTotalTimeOverhead;
  double dConnectedSince;
  int    bConnected;
  time_t tServerAdded;
  int    iNumErrors;
  int    iNumSnippetsAttempted;
  int    iNumSnippetsRetrieved;
  int    iNumSnippetsFlagged;
  int    iNumSnippetsTimedOut;
} WS_SERVER_CONNECTION_STATS_REC;

typedef struct _WS_MENU_REC
{
  char    addr[wsADRLEN];
  char    port[wsADRLEN]; 
  SOCKET  sock;
  int     menunum;   /* menu number DK III*/
                     /* used for comparing menus and determining
                        precedence among menus */
  int     serverstatus;
  WS_SERVER_CONNECTION_STATS_REC stats;
  time_t     tNextRefresh;
  int     tRefreshInterval;
  WS_PSCNL pscnl;
  WS_MENU next;
} WS_MENU_REC;

/* DK III  environment struct */
typedef struct _wsEnvironmentStruct
{
  WS_MENU_REC** MenuList;
  int           iNumMenusInList;
  int           iNextMenuNum;
  int           iMenuListSize;
  WS_PSCNL      pPSCNLBuffer;
  int           iNumPSCNLs;
  int           iCurrentTraceReq;
  int           iNumTraceReqsInList;
  int           iTraceTypeForList;
  int           iTimeoutMsec;
  WS_MENU       CurrentMenu;
  char *        bSnippetBuffer;
  int           iSnippetBufferSize;
} wsEnvironmentStruct;


/* Unused in wsclientIII */
typedef volatile struct
{
  WS_MENU head;
  WS_MENU tail;
} WS_MENU_QUEUE_REC;


/***************************************************
* Creating and Destroying the wsclient environment.*
***************************************************/

/*  Call wsInitialize() to setup the wsclient environment,
   it must be called prior to any other wsclient calls.
   Calls wsDestroy() to tear down the wsclient environment,
   and free any remaining resources, it should be called
   when the wsclient environment is no longer needed.
************************************************************/

/* to initialize the wsclient environment and allocate
   resources for the wsclient routines to operate.
 ******************************************************/
int wsInitialize(wsHANDLE* ppEnv, int iMaxSnippetSize);

/* to destroy the wsclient environment and free any allocated
   resources, including closing all server sockets 
   and deallocating the menus
 ******************************************************/
void wsDestroy(wsHANDLE * ppEnv );
/* REPLACES wsclientII
void wsKillMenu( WS_MENU_QUEUE_REC* );
****************************************************************************/

/***************************************************
* Adding and removing wave servers                 *
***************************************************/

/* to add a server to the list of servers available for trace requests
 ****************************************************************************/
int wsAddServer(wsHANDLE pEnv, char* ipAdr, char*  port, 
                int timeout, int refreshtime );
/* REPLACES wsclientII 
int wsAppendMenu( char* , char*, WS_MENU_QUEUE_REC*, int );
****************************************************************************/

/* to remove a server from the list of servers available for trace requests
**************************************/
int wsRemoveServer(wsHANDLE pEnv, char * szIPAddr, char * szPort);
/* REPLACES wsclientII
void wsKillPSCNL( WS_PSCNL );
****************************************************************************/


/***************************************************
* Getting trace                                    *
***************************************************/

/* to retrieve the binary trace snippet specified in the structure TRACE_REQ 
 ***************************************************************************/
int wsGetTrace(TRACE_REQ* getThis, wsHANDLE pEnv, 
               int iTraceType, int timeout_msec);
/* REPLACES wsclientII
int wsGetTraceBin( TRACE_REQ*, WS_MENU_QUEUE_REC*, int );
int wsGetTraceAscii( TRACE_REQ*, WS_MENU_QUEUE_REC*, int );
****************************************************************************/

int wsGetTraceFromServer(TRACE_REQ* getThis, WS_MENU menu, int iTraceType, 
                         int timeout_msec);
/****************************************************************************/


/*******************************************************
* Debug                                                *
*******************************************************/

/* set debugging level for the ws_clientIII routines:
   can be:
     WS_DEBUG_NONE
   or any combination of:
     WS_DEBUG_SERVER_INFO 
     WS_DEBUG_SERVER_WARNINGS 
     WS_DEBUG_SERVER_ERRORS 
     WS_DEBUG_SERVER_STATISTICS  
     WS_DEBUG_DATA_INFO       
     WS_DEBUG_DATA_WARNINGS     
     WS_DEBUG_DATA_ERRORS       
     WS_DEBUG_OVERKILL        
   or
     WS_DEBUG_ALL

**********************************************************/
int setWsClient_ewDebug(int debug);




/*******************************************************
* Other functions that should not be used at this time *
*******************************************************/

/***** DO NOT USE wsAttachServer() or wsDetachServer *********/
/* To open a connection to a server
***********************************/
int wsAttachServer( WS_MENU, int );

/* to close a single server socket after an error
 ************************************************/
void wsDetachServer( WS_MENU, int iError );


/* delete and free a linked list of PSCNLs
*****************************************/
void wsKillPSCNL( WS_PSCNL );


/* Return the pscnl list for this server from the menu queue *
 *****************************************************************************/
int wsGetServerPSCNL( char* addr, char* port, WS_PSCNL* pscnlp, wsHANDLE pEnv );
/* REPLACES wsclientII
int wsGetServerPSCNL( char* , char*, WS_PSCNL*, WS_MENU_QUEUE_REC* );
****************************************************************************/


/* finds a PSCNL.  returns the menu that contained the PSCNL.
   if menup is NULL then it starts at the beginning of the
   list of menus.  If menup is NOT NULL, then it starts searching
   at the menu immediately after menup in the list.
*******************************************************************/
int wsSearchSCNL( TRACE_REQ* getThis, WS_MENU* menup, wsHANDLE pEnv);
/* REPLACES wsclientII
int wsSearchSCNL( TRACE_REQ*, WS_MENU*, WS_PSCNL*, WS_MENU_QUEUE_REC* );
****************************************************************************/

/***************************************/
int wsPrepRequestList(TRACE_REQ* RequestList, int iNumReqsInList, 
                      int iTraceType, int timeout_sec, wsHANDLE pEnv);

/***************************************/
int wsGetNextTraceFromRequestList(TRACE_REQ* RequestList, wsHANDLE pEnv, 
                           TRACE_REQ** ppResult);

/***************************************/
int wsEndRequestList(TRACE_REQ* RequestList, wsHANDLE pEnv);


/*******************************************************
* Other                                                *
*******************************************************/
void wsCloseAllSockets(wsHANDLE pEnv);

int wsPrintServerStats(wsHANDLE pEnv);

/* still to write ########################*/
int wsRefreshMenu(WS_MENU menu);

int wsRefreshAllMenus(wsHANDLE pEnv);

#endif /* WS_CLIENT */

