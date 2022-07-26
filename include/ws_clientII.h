
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: ws_clientII.h 7606 2018-11-26 20:47:34Z alexander $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.6  2006/06/19 20:58:17  davek
 *     Upped the max menu str len (as defined by wsREPLEN) so that it is large
 *     enough to handle a 5000+ SCNL menu from a single wave_server.
 *     Downside is that it must temporarily allocate 500kb instead of 80kb when
 *     parsing a menu.  This seemed like a reasonable tradeoff.
 *
 *     Revision 1.5  2005/11/28 16:53:19  friberg
 *     modified wsREQLEN and wsREPLEN to allow 1024 SNCL's as per Winston Servers
 *
 *     Revision 1.4  2004/07/02 01:03:58  lombard
 *     Added non-location-code versions of several functions.
 *     Added `L' to the name of location-code-enabled functions
 *     and to structures with SCN in the name.
 *
 *     Revision 1.3  2004/05/18 22:28:47  lombard
 *     Modified for location code; no structure names have changed here.
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
#include <trace_buf.h>
#include <socket_ew.h>

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


/* Trace Request structure 
*************************/
/* the structure below is the 'form' which must be filled out to request a 
   snippet of trace data. The requester fills out the first part, and the
   client routines fill out the second part. */
typedef struct
{
    /* the request portion */
    char    sta[TRACE2_STA_LEN];	/* Site name */
    char    chan[TRACE2_CHAN_LEN];	/* Component/channel code */
    char    net[TRACE2_NET_LEN];	/* Network name */
    char    loc[TRACE2_LOC_LEN];	/* Location code */
    short   pinno;          	/* Pin number that maps to sta/net/chan */
    double  reqStarttime;	/* requested starttime - seconds since 1970 */
    double  reqEndtime;	/* requested endtime - seconds since 1970 */
    int	    partial;		/* 1=> I'll accept partial data;
				   0=> all or nothing 
				   *** NOT IMPLEMENTED ***		  */
    char*   pBuf;		/* pointer to user supplied buffer for trace */
    unsigned long	bufLen;	/* length of above */
    long    timeout;	/* seconds after which we must return */
    long    fill;		/* ASCII only: fill value for gaps in trace */

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
#define wsREQLEN   1024    /* length for MENU request */
#define wsREPLEN  2000000   /* increased to 2Megs to allow HUGE menu requests */


typedef struct _WS_PSCNL_REC *WS_PSCNL;
typedef struct _WS_PSCNL_REC
{
    char    sta[TRACE2_STA_LEN];	/* Site name */
    char    chan[TRACE2_CHAN_LEN];	/* Component/channel code */
    char    net[TRACE2_NET_LEN];	/* Network name */
    char    loc[TRACE2_LOC_LEN];	/* Location code */
    short   pinno;          /* Pin number that maps to sta/net/chan */
    double  tankStarttime;  /* starttime for this SCNL */	
    double  tankEndtime;    /* endtime for this SCNL */	
    WS_PSCNL next;
} WS_PSCNL_REC;

typedef struct _WS_MENU_REC *WS_MENU;
typedef struct _WS_MENU_REC
{
    char    addr[wsADRLEN];
    char    port[wsADRLEN]; 
    SOCKET  sock;

    WS_PSCNL pscnl;
    WS_MENU next;
} WS_MENU_REC;

typedef volatile struct
{
    WS_MENU head;
    WS_MENU tail;
} WS_MENU_QUEUE_REC;

/* to setup and connect to a list of servers, and get SCNL menu of each server
****************************************************************************/
int wsAppendMenu( char* , char*, WS_MENU_QUEUE_REC*, int );

/* to close all server sockets and deallocate the menus
******************************************************/
void wsKillMenu( WS_MENU_QUEUE_REC* );

/* to deallocate a PSCNL list
**************************************/
void wsKillPSCNL( WS_PSCNL );

/* To open a connection to a server
***********************************/
int wsAttachServer( WS_MENU, int );

/* to close a single server socket after an error
************************************************/
void wsDetachServer( WS_MENU );

/* wsGetServerPSCNL: Return the pscn list for this server from the menu queue *
*****************************************************************************/
int wsGetServerPSCN( char*, char*, WS_PSCNL*, WS_MENU_QUEUE_REC* );
int wsGetServerPSCNL( char*, char*, WS_PSCNL*, WS_MENU_QUEUE_REC* );

/* to retrieve the binary trace snippet specified in the structure TRACE_REQ 
***************************************************************************/
int wsGetTraceBin( TRACE_REQ*, WS_MENU_QUEUE_REC*, int );
int wsGetTraceBinL( TRACE_REQ*, WS_MENU_QUEUE_REC*, int );

/* to retrieve the ASCII trace snippet specified in the structure TRACE_REQ 
**************************************************************************/
int wsGetTraceAscii( TRACE_REQ*, WS_MENU_QUEUE_REC*, int );
int wsGetTraceAsciiL( TRACE_REQ*, WS_MENU_QUEUE_REC*, int );

/* return the PSCNL list for this server
***************************************/
int wsSearchSCN( TRACE_REQ*, WS_MENU*, WS_PSCNL*, WS_MENU_QUEUE_REC* );
int wsSearchSCNL( TRACE_REQ*, WS_MENU*, WS_PSCNL*, WS_MENU_QUEUE_REC* );

/* turn debugging on or off for the ws_clientII routines.
**********************************************************/
int setWsClient_ewDebug(int debug);


#endif /* WS_CLIENT */
