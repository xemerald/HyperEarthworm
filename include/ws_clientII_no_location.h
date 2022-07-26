
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: ws_clientII.h,v 1.1.1.1 2005/07/14 19:57:46 paulf Exp $
 *
 *    Revision history:
 *     $Log: ws_clientII.h,v $
 *     Revision 1.1.1.1  2005/07/14 19:57:46  paulf
 *     Local ISTI CVS copy of EW v6.3
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

#include <platform.h>

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
#define WS_ERR_SCN_NOT_IN_MENU      -4   /* SCN not found in menu */
#define WS_ERR_BUFFER_OVERFLOW      -5   /* reply truncated at buffer limit */
#define WS_ERR_MEMORY               -6   /* Couldn't allocate memory */
#define WS_ERR_PARSE                -7   /* Couldn't parse server's reply */
/* socket related errors: */
#define WS_ERR_TIMEOUT             -10   /* Socket transaction timed out */
#define WS_ERR_BROKEN_CONNECTION   -11   /* An open connection was broken */
#define WS_ERR_SOCKET              -12   /* problem setting up socket */
#define WS_ERR_NO_CONNECTION       -13   /* Could not make connection */


/* Trace Request structure 
 *************************/
/* the structure below is the 'form' which must be filled out to request a 
   snippet of trace data. The requester fills out the first part, and the
   client routines fill out the second part. */
typedef struct
{
	/* the request portion */
        char    sta[7];         /* Site name */
        char    chan[9];        /* Component/channel code */
        char    net[9];         /* Network name */
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
	
}
TRACE_REQ;

#define wsADRLEN    64    /* length for ip and port adrress strings */
#define wsREQLEN   256    /* length for MENU request */
#define wsREPLEN  20480   /* length for reply to MENU request(up to 256 SCNs) */


typedef struct _WS_PSCN_REC *WS_PSCN;
typedef struct _WS_PSCN_REC
{
  char    sta[7];         /* Site name */
  char    chan[9];        /* Component/channel code */
  char    net[9];         /* Network name */
  short   pinno;          /* Pin number that maps to sta/net/chan */
  double  tankStarttime;  /* starttime for this SCN */	
  double  tankEndtime;    /* endtime for this SCN */	
  
  WS_PSCN next;
} WS_PSCN_REC;

typedef struct _WS_MENU_REC *WS_MENU;
typedef struct _WS_MENU_REC
{
  char    addr[wsADRLEN];
  char    port[wsADRLEN]; 
  SOCKET  sock;

  WS_PSCN pscn;
  WS_MENU next;
} WS_MENU_REC;

typedef volatile struct
{
  WS_MENU head;
  WS_MENU tail;
} WS_MENU_QUEUE_REC;

/* to setup and connect to a list of servers, and get SCN menu of each server
 ****************************************************************************/
int wsAppendMenu( char* , char*, WS_MENU_QUEUE_REC*, int );

/* to close all server sockets and deallocate the menus
 ******************************************************/
void wsKillMenu( WS_MENU_QUEUE_REC* );

/* to deallocate a PSCN list
**************************************/
void wsKillPSCN( WS_PSCN );

/* To open a connection to a server
***********************************/
int wsAttachServer( WS_MENU, int );

/* to close a single server socket after an error
 ************************************************/
void wsDetachServer( WS_MENU );

/* wsGetServerPSCN: Return the pscn list for this server from the menu queue *
 *****************************************************************************/
int wsGetServerPSCN( char*, char*, WS_PSCN*, WS_MENU_QUEUE_REC* );

/* to retrieve the binary trace snippet specified in the structure TRACE_REQ 
 ***************************************************************************/
int wsGetTraceBin( TRACE_REQ*, WS_MENU_QUEUE_REC*, int );

/* to retrieve the ASCII trace snippet specified in the structure TRACE_REQ 
 **************************************************************************/
int wsGetTraceAscii( TRACE_REQ*, WS_MENU_QUEUE_REC*, int );

/* return the PSCN list for this server
***************************************/
int wsSearchSCN( TRACE_REQ*, WS_MENU*, WS_PSCN*, WS_MENU_QUEUE_REC* );

/* turn debugging on or off for the ws_clientII routines.
**********************************************************/
int setWsClient_ewDebug(int debug);


#endif /* WS_CLIENT */
