
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: ws_client.h 15 2000-02-14 20:06:34Z lucky $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2000/02/14 20:05:54  lucky
 *     Initial revision
 *
 *
 */

/* 8/22/97
This is the include file for the WaveServerIV client routines
and associated structures */

/* Changed PNL, 11/23/97: fixed up return status codes here and in ws_client.c.
 */

#ifndef WS_CLIENT
#define WS_CLIENT

/* Error codes used by routines in ws_client.c
 *********************************************/
#define WS_ERR_NONE                  1
#define WS_ERR_NO_CONNECTION        -1
#define WS_ERR_BROKEN_CONNECTION    -3
#define WS_ERR_MEMORY               -4
#define WS_ERR_BUFFER_OVERFLOW      -5
#define WS_ERR_EMPTY_MENU           -6
#define WS_ERR_SCN_NOT_IN_MENU      -7
#define WS_ERR_SERVER_NOT_IN_MENU   -8
#define WS_ERR_INPUT                -9
#define WS_ERR_TIMEOUT             -10
#define WS_ERR_FLAGGED             -11
#define WS_ERR_PARSE               -12
#define WS_ERR_GENERIC             -99

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
                                   0=> all or nothing */	
	char*	pBuf;		/* pointer to user supplied buffer for trace */
	unsigned long	bufLen;	/* length of above */
	long 	timeout;	/* seconds after which we must return */
	long	fill;		/* ASCII only: fill value for gaps in trace */

	/* the reply portion */
	char	retFlag;	/* return flag (if any) from wave server */
	double	waitSec;	/* seconds to wait until requested data will be
				   available in the wave server.
                                   Based on times in current menu list;
                                   <0 => it's too late */
	double actStarttime;	/* actual start time of supplied trace */
	double actEndtime;	/* actual end   time of supplied trace */
	long   actLen;		/* actual length in bytes */
	double samprate;	/* nominal sampling rate - samples per second */
	
}
TRACE_REQ;

#define wsADRLEN    16    /* length for ip and port adrress strings */
#define wsREQLEN   256    /* length for MENU request */
#define wsREPLEN  8192    /* length for reply to MENU request(up to 128 SCNs) */


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
  int     sock;

  WS_PSCN pscn;
  WS_MENU next;
} WS_MENU_REC;

typedef volatile struct
{
  WS_MENU head;
  WS_MENU tail;
} WS_MENU_QUEUE_REC;

/* to setup a list of servers and SCN's of each server
 *****************************************************/
int wsAppendMenu( char* , char*, long );

/* to find out what servers the menu has 
 ***************************************/
int wsGetMenu( WS_MENU* );

/* to find out what a waveserver has;
   an example to find out what a server has is as follows:
   int FindOutWhatAServerHas( char* addr, char* port )
   {
     WS_PSCN pscn = NULL;
     int err = WS_ERR_GENERIC;

     err = wsGetServerPSCN( addr, port, &pscn );
     if ( err != WS_ERR_NONE )
       goto abort;
     if ( !pscn )
       {
       print( "Server has nothing to offer\n" );
       goto abort;
       }
     while ( pscn )
       {
         printf( "pinno[%d] s[%s] c[%s] n[%s]\n",
                 pscn->pinno, pscn->sta, pscn->chan, pscn->net );
         pscn = pscn->next;
       }
     err = WS_ERR_NONE;
    abort:
     return( err );
   }
 ***********************************/
int wsGetServerPSCN( char*, char*, WS_PSCN* );

/* to release the list created by wsAppendMenu
 *********************************************/
void wsKillMenu();

/* wsGetServerPSCN: Return the pscn list for this server from the menu queue *
 *****************************************************************************/
int wsGetServerPSCN( char*, char*, WS_PSCN*, WS_MENU_QUEUE_REC* );

/* to retrieve the binary trace snippet specified in the structure TRACE_REQ 
 ***************************************************************************/
int wsGetTraceBin( TRACE_REQ* );

/* to retrieve the ASCII trace snippet specified in the structure TRACE_REQ 
 **************************************************************************/
int wsGetTraceAscii( TRACE_REQ* );

#endif /* WS_CLIENT */
