
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: ws_clientII.c 7800 2019-02-15 17:04:21Z alexander $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.16  2008/09/21 05:12:27  rwg
 *     fix null pointer dereference found by llvm/clang's static analyzer
 *
 *     Revision 1.15  2007/03/28 14:17:36  paulf
 *     fixed MACOSX dependency
 *
 *     Revision 1.14  2006/06/19 20:53:11  davek
 *     Added protection against oversized SCNL fields in wave server menus:
 *     increased the size of the buffers used to capture SCNL values from the
 *     menu, in order to reduce the chance of clobbering memory if a channel
 *     with SCNL longer than the spec'd tracebuf2 sizes is found in a menu.
 *
 *     Fixed a bug where wsGetTraceAsciiL() was returning WS_ERR_NONE even
 *     though there was a warning flag (with a WS_WRN_FLAGGED) return code
 *     from wsParseAsciiHeaderReplyL().
 *     Added a work around for handling flagged wave_server returns where the
 *     datatype does not get properly filled in.
 *
 *     Revision 1.13  2005/11/03 17:41:58  luetgert
 *     .
 *
 *     Revision 1.12  2005/09/12 17:28:06  davidk
 *     Added code to wsGetTraceAsciiL() that causes the socket connection to be
 *     closed and WS_ERR_BUFFER_OVERFLOW to be returned when an overflow
 *     occurs.  The previous behavior was to leave the socket buffer full, in which
 *     case successive calls could fail with random results.
 *
 *     Revision 1.11  2005/04/21 22:54:23  davidk
 *     Added "SCNL" token to the end of wave_server menu request, to identify
 *     this client as an SCNL client, per wave_serverV protocol change (v5.1.24).
 *
 *     Revision 1.10  2004/07/02 01:03:27  lombard
 *     Added non-location-code versions of several functions.
 *     Added `L' to the name of location-code-enabled functions
 *     and to structures with SCN in the name.
 *
 *     Revision 1.9  2004/06/04 00:16:35  lombard
 *     Change to wsParseMenuReply: if no location code in reply, leave the pscn->loc
 *     empty instead of filling in the default location code of "--". That way
 *     clients can tell if they are talking to an SCN wave_server vs an SCNL
 *     wave_server.
 *
 *     Revision 1.8  2004/05/18 22:38:12  lombard
 *     Modified for location code
 *
 *     Revision 1.7  2003/03/11 12:31:44  friberg
 *     changed the fix of yesterday to match other logit messages
 *
 *     Revision 1.6  2003/03/10 18:33:47  friberg
 *     bug in logit message found by Ilya Dricker
 *
 *     Revision 1.5  2002/03/19 22:15:15  davidk
 *     Fixed bug reported by IlyaDricker in wsGetTraceBin() where the function
 *     was not correctly handling the return code from wsParseBinHeaderReply().
 *
 *     Revision 1.4  2001/04/17 17:31:41  davidk
 *     Added explicit (SOCKET) typecasts in FD_SET to get rid of compiler warnings on NT.
 *
 *     Revision 1.3  2000/09/17 18:37:57  lombard
 *     wsSearchSCN now really returns the first menu with matching scn.
 *     wsGetTraceBin will now try another server if the first returns a gap.
 *
 *     Revision 1.2  2000/03/08 18:14:06  luetgert
 *     fixed memmove problem in wsParseAsciiHeaderReply.
 *
 *     Revision 1.1  2000/02/14 18:51:48  lucky
 *     Initial revision
 *
 *
 */

/* Version II of WaveServerIV client utility routines. This file contains
 * various routines useful for dealing with WaveServerIV and beyond. */

/* 5/17/98: Fixed bug in wsWaitAscii: it would fill reply buffer but never
 * report overflow. PNL */

/* The philosophy behind the version II changes is:
 * - Leave sockets open unless there is an error on the socket; calling 
 *   progam should ensure all sockets are closed before it quits.
 * - Routines are thread-safe. However, the calling thread must ensure its
 *   socket descriptors are unique or mutexed so that requests and replies
 *   can't overlap.
 * - Routines are layered on top of Dave Kragness's socket wrappers and thus
 *   do no timing themselves. Exceptions to this are wsWaitAscii and 
 *   wsWaitBinHeader which need special timing of recv().
 * Pete Lombard, University of Washington Geophysics; 1/4/98
 */

/* NOTE: Location code additions:
 * Location code has been added to station-network-channel to make SCNL
 * All ws_clientII structures with SCN in their name have been changed
 * to have SCNL in the name.
 * Most functions that care about SCNs now have counterparts that care
 * about SCNL.
 * The exceptions are wsAppendMenu and wsParseMenuReply which handle the
 * presence or absence of location codes.
 * Each of the function descriptions below indicate whether the particular
 * function handles location codes or not.
 * In this way, a single client can communicate with both 
 * location-code-enabled and location-code-challenged wave_serverVs.
 * Pete Lombard, UC Berkeley Seismology Lab; 7/1/04
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <earthworm.h>
#include <ws_clientII.h>
#include <socket_ew.h>
#include <time_ew.h>

#define WS_MAX_RECV_BUF_LEN 4096

static int menu_reqid = 0;
int WS_CL_DEBUG = 0;

extern int recv_all (SOCKET, char FAR*,int,int,int);


/* Protoypes for internal functions */
static int  wsWaitAscii( WS_MENU, char*, int, int );
static int  wsWaitBinHeader( WS_MENU, char*, int, int );
static int  wsParseMenuReply( WS_MENU, char* );
static int  wsParseBinHeaderReply( TRACE_REQ*, char* );
static int  wsParseBinHeaderReplyL( TRACE_REQ*, char* );
static int  wsParseAsciiHeaderReply( TRACE_REQ*, char* );
static int  wsParseAsciiHeaderReplyL( TRACE_REQ*, char* );
static void wsSkipN( char*, int, int* );
struct timeval FAR * resetTimeout(struct timeval FAR *);
Time_ew adjustTimeoutLength(int timeout_msec);

/**************************************************************************
 *      wsAppendMenu: builds a combined menu from many waveservers.       *
 *      Called with the address and port of one waveserver.               *
 *      On the first call it creates a linked list to store the 'menu'    *
 *      reply from the indicated waveserver.                              *
 *      On subsequent calls, it appends the menu replies to the list.     *
 *  Location code:                                                        *
 *      If querying an SCN (non-location-code) wave_server, the loc field *
 *      of the *WS_PSCN record will be the null, zero-length string.      *
 *      Otherwise, a non-zero-length location code will be present.       *
 *      It is up to the client program to check for the presence of       *
 *      location codes and make appropriate subsequent ws_clientII calls. *
 **************************************************************************/

int wsAppendMenu( char* ipAdr, char* port, WS_MENU_QUEUE_REC* menu_queue, 
		  int timeout )
     /*
       Arguments:
       ipAdr:  is the dot form of the IP address as a char string.
       port:  TCP port number as a char string.
       menu_queue:  caller-supplied pointer to the list of menus.
       timeout:  timeout interval in milliseconds,
       return:  WS_ERR_NONE:  if all went well.
       WS_ERR_NO_CONNECTION: if we could not get a connection.
       WS_ERR_SOCKET: if we could not create a socket.
       WS_ERR_BROKEN_CONNECTION:  if the connection broke.
       WS_ERR_TIMEOUT: if a timeout occured.
       WS_ERR_MEMORY: if out of memory.
       WS_ERR_INPUT: if bad input parameters.
       WS_ERR_PARSE: if parser failed.
     */
{
    int ret, len, err;
    WS_MENU menu = NULL;
    char request[wsREQLEN];
    char* reply = NULL;

    if ( !ipAdr || !port ||
	 (strlen(ipAdr) >= wsADRLEN) || (strlen(port) >= wsADRLEN) ) {
	ret = WS_ERR_INPUT;
	if (WS_CL_DEBUG) logit("e","wsAppendMenu: bad address: %s port: %s\n",
			       ipAdr, port);
	goto Abort;
    }

    menu = ( WS_MENU_REC* )calloc(sizeof(WS_MENU_REC),1);
    reply = ( char* )malloc( wsREPLEN );
    if ( menu == NULL || reply == NULL ) {
	ret = WS_ERR_MEMORY;
	if (WS_CL_DEBUG) logit("e", "wsAppendMenu: memory allocation error\n");
	goto Abort;
    }

    strncpy( menu->addr, ipAdr, wsADRLEN );
    strncpy( menu->port, port, wsADRLEN );
    if ( wsAttachServer( menu, timeout ) != WS_ERR_NONE ) {
	    ret = WS_ERR_NO_CONNECTION;
	    goto Abort;
    }

    if (WS_CL_DEBUG) logit("t","Attempting to retrieve menu from %s:%s\n", menu->addr, menu->port);

    sprintf( request, "MENU: %d SCNL\n", menu_reqid++ );
    len = (int)strlen(request);
    if ( ( ret =  send_ew( menu->sock, request, len, 0, timeout ) ) != len ) {
	if (ret < 0 ) {
	    if (WS_CL_DEBUG) logit("e", "wsAppendMenu: connection broke to server %s:%s\n",
				   ipAdr, port);
	    ret = WS_ERR_BROKEN_CONNECTION;
	} else {
	    if (WS_CL_DEBUG) logit("e", "wsAppendMenu: server %s:%s timed out\n",
				   ipAdr, port);
	    ret = WS_ERR_TIMEOUT;
	}
	goto Abort;
    }
    
    ret = wsWaitAscii( menu, reply, wsREPLEN, timeout );
    if ( ret != WS_ERR_NONE && ret != WS_ERR_BUFFER_OVERFLOW ) {
	/* we might have received some useful data in spite of the overflow */
	
	goto Abort;
    }
    
    if ( ( err = wsParseMenuReply( menu, reply ) ) != WS_ERR_NONE ) {
	    if ( ret == WS_ERR_BUFFER_OVERFLOW && err == WS_ERR_PARSE ) {
	        if (WS_CL_DEBUG) logit("e", "wsAppendMenu: buffer overflow; parse failure\n");
	    } else {
	        ret = err;
	    }
	    goto Abort;
    }
    
    if ( menu->pscnl == NULL ) {
	    if (WS_CL_DEBUG) logit("e", "wsAppendMenu: no SCNL at server %s:%s\n",
		    	       menu->addr, menu->port);
	    ret = WS_ERR_EMPTY_MENU;
	    goto Abort;
    }
  
    /* Add the menu to the possibly empty menu queue */
    if ( !menu_queue->head ) {
	menu_queue->head = menu_queue->tail = menu;
    } else {
	menu_queue->tail->next = menu;
	menu_queue->tail = menu;
    }
    if ( reply ) free( reply );
    return WS_ERR_NONE;
    
 Abort:
    /* An error occured, so clean up the mess */
    wsDetachServer( menu );
    if ( reply ) free( reply );
    if ( menu ) {
	wsKillPSCNL( menu->pscnl );
	free( menu );
    }
    return ret;
}


/************************************************************************** 
 * wsKillMenu: Gracefully closes all the server sockets and releases the  *
 *             linked list of menus created by wsAppendMenu               *
 *  This function is not concerned with location codes.                   *
 **************************************************************************/
void wsKillMenu( WS_MENU_QUEUE_REC* menu_queue )
     /*
       Arguments:
       menu_queue:  caller-supplied pointer to the list of menus.
     */
{
    WS_MENU menu = menu_queue->head;
    
    while ( menu ) {
	WS_MENU next = menu->next;
	
	if ( menu->sock != -1 )
	    closesocket_ew( menu->sock, SOCKET_CLOSE_GRACEFULLY_EW );
	wsKillPSCNL( menu->pscnl );
	free( menu );
	menu = next;
    }
    menu_queue->head = menu_queue->tail = NULL;
    menu_reqid = 0;

    return;
}


/**********************************************************************
 * wsGetTraceBinL: retrieves the piece of raw trace data specified in *
 * the structure 'getThis': The current menu list, as built by the    *
 * routines above will be searched for a matching SCNL. If a match    *
 * is found, the associated wave server will be contacted, and a      *
 * request for the trace snippet will be made.                        *
 * This function requires location codes at the wave_server.          *
 **********************************************************************/
int wsGetTraceBinL( TRACE_REQ* getThis, WS_MENU_QUEUE_REC* menu_queue,
		   int timeout )
     /*
       Arguments:
       getThis:   a TRACE_REQ structure (see ws_client.h), with the
       request portion filled in.
       menu_queue:   pointer to the list of server menus.
       timeout:   Time in milliseconds to wait for reply
       return:   WS_ERR_NONE: all went well.
       WS_WRN_FLAGGED: wave server returned error flag instead
       of trace data.
       WS_ERR_EMPTY_MENU: No menu list found.
       WS_ERR_SCNL_NOT_IN_MENU: SCNL not found in menu list.
       WS_ERR_NO_CONNECTION: if socket was already closed
       The socket will be closed for the following:
       WS_ERR_BUFFER_OVERFLOW: trace buffer supplied too small.
       WS_ERR_TIMEOUT: if a timeout occured
       WS_ERR_BROKEN_CONNECTION: if a connection broke.
       WS_ERR_SOCKET: problem changing socket options.
       WS_ERR_PARSE: couldn't parse server's reply.
     */
{
    int ret, len, err = WS_ERR_NONE;
    WS_MENU menu = NULL;
    WS_PSCNL pscnl = NULL;
    char request[wsREQLEN];

    if ( menu_queue->head == NULL ) {
	    if (WS_CL_DEBUG) logit("e", "wsGetTraceBinL: empty menu\n");
	    return WS_ERR_EMPTY_MENU;
    }
    
 next_menu:
    if ( ( ret = wsSearchSCNL( getThis, &menu, &pscnl, menu_queue ))
	 != WS_ERR_NONE ) {
	    if (err != WS_ERR_NONE)
	        return err;
	    else
	        return ret;  /* WS_ERR_SCNL_NOT_IN_MENU */
    }
    
    if (menu->sock < 0) {
	    if (WS_CL_DEBUG) logit("e", "wsGetTraceBinL: no socket for %s:%s\n", 
		    	       menu->addr, menu->port );
	    return WS_ERR_NO_CONNECTION;
    }
    
    sprintf( request, "GETSCNLRAW: %d %s %s %s %s %lf %lf\n",
	     menu_reqid++, pscnl->sta, pscnl->chan, pscnl->net, pscnl->loc,
	     getThis->reqStarttime, getThis->reqEndtime );

    len = (int)strlen(request);
    if ( (ret = send_ew( menu->sock, request, len, 0, timeout )) != len ) {
	    if (ret < 0 ) {
	        ret = WS_ERR_BROKEN_CONNECTION;
	    } else {
	        ret = WS_ERR_TIMEOUT;
	    }
	    goto Abort;
    }

    ret = wsWaitBinHeader( menu, getThis->pBuf, getThis->bufLen, timeout );
    switch (ret) {
        case WS_ERR_INPUT:
            return ret;
        break;
        case WS_ERR_NONE:
        break;
        default:
	        /* buffer overflow, timeout, socket error, or broken connection */
	        goto Abort;
    }

    err = wsParseBinHeaderReplyL( getThis, getThis->pBuf );
    switch (err) {
        case WS_ERR_INPUT:
            if (WS_CL_DEBUG) logit("e", "wsGetTraceBinL: input error\n");
            goto next_menu;
        break;
        case WS_WRN_FLAGGED:  /* no trace data to get */
            if (WS_CL_DEBUG) logit("e", "wsGetTraceBinL: no trace data to get\n");
	        goto next_menu;
	    break;
        case WS_ERR_PARSE:  /* can't get pending trace data, so close socket */
	        /* DK 03192002 setting ret due to bug reported by Ilya Dricker */
	        if(ret == WS_ERR_NONE)
	            ret = err;
	        /* end DK 0319 change */
	        goto Abort;
	    break;
        default:  /* no error, so continue */
	    break;
    }
  
    if ( getThis->actLen ) {
	    long int binLen = getThis->actLen;
	
	    if ( (int)getThis->bufLen < binLen )
	            binLen = (int)getThis->bufLen;
	    ret = recv_all( menu->sock, getThis->pBuf, binLen, 0, timeout );
	
	    if ( ret != binLen ) {
	        if ( ret < 0 ) {
		        ret = WS_ERR_BROKEN_CONNECTION;
	        } else {
		        ret = WS_ERR_TIMEOUT;
	        }
	        goto Abort;
	    }
	
	    if ( binLen < getThis->actLen ) {
	        getThis->actLen = binLen;
	        ret = WS_ERR_BUFFER_OVERFLOW;
	        if (WS_CL_DEBUG) 
		        logit ( "e", "wsGetTraceBinL(): buffer full, trace truncated\n" );
	        goto Abort;
	    }
    }
    return WS_ERR_NONE;
  
 Abort:
    wsDetachServer( menu );
    if (WS_CL_DEBUG) {
	    if  ( ret == WS_ERR_TIMEOUT ) {
	        logit( "e","wsGetTraceBinL(): server %s:%s timed out\n", menu->addr,
		            menu->port );
	    } else if ( ret == WS_ERR_BROKEN_CONNECTION ) {
	        logit( "e","wsGetTraceBinL(): broken connection to server %s:%s\n",
		            menu->addr, menu->port);
	    }
    }
  
    return ret;
}

/*********************************************************************
 * wsGetTraceBin: retrieves the piece of raw trace data specified in *
 * the structure 'getThis': The current menu list, as built by the   *
 * routines above will be searched for a matching SCN. If a match    *
 * is found, the associated wave server will be contacted, and a     *
 * request for the trace snippet will be made.                       *
 * This function does not handle location codes at the wave_server.  *
 *********************************************************************/
int wsGetTraceBin( TRACE_REQ* getThis, WS_MENU_QUEUE_REC* menu_queue,
		   int timeout )
     /*
       Arguments:
       getThis:   a TRACE_REQ structure (see ws_client.h), with the
       request portion filled in.
       menu_queue:   pointer to the list of server menus.
       timeout:   Time in milliseconds to wait for reply
       return:   WS_ERR_NONE: all went well.
       WS_WRN_FLAGGED: wave server returned error flag instead
       of trace data.
       WS_ERR_EMPTY_MENU: No menu list found.
       WS_ERR_SCNL_NOT_IN_MENU: SCNL not found in menu list.
       WS_ERR_NO_CONNECTION: if socket was already closed
       The socket will be closed for the following:
       WS_ERR_BUFFER_OVERFLOW: trace buffer supplied too small.
       WS_ERR_TIMEOUT: if a timeout occured
       WS_ERR_BROKEN_CONNECTION: if a connection broke.
       WS_ERR_SOCKET: problem changing socket options.
       WS_ERR_PARSE: couldn't parse server's reply.
     */
{
    int ret, len, err = WS_ERR_NONE;
    WS_MENU menu = NULL;
    WS_PSCNL pscnl = NULL;
    char request[wsREQLEN];

    if ( menu_queue->head == NULL ) {
	if (WS_CL_DEBUG) logit("e", "wsGetTraceBin: empty menu\n");
	return WS_ERR_EMPTY_MENU;
    }
    
 next_menu:
    if ( ( ret = wsSearchSCN( getThis, &menu, &pscnl, menu_queue ))
	 != WS_ERR_NONE ) {
	if (err != WS_ERR_NONE)
	    return err;
	else
	    return ret;  /* WS_ERR_SCNL_NOT_IN_MENU */
    }
    
    if (menu->sock < 0) {
	if (WS_CL_DEBUG) logit("e", "wsGetTraceBin: no socket for %s:%s\n", 
			       menu->addr, menu->port );
	return WS_ERR_NO_CONNECTION;
    }
    
    sprintf( request, "GETSCNRAW: %d %s %s %s %lf %lf\n",
	     menu_reqid++, pscnl->sta, pscnl->chan, pscnl->net,
	     getThis->reqStarttime, getThis->reqEndtime );

    len = (int)strlen(request);
    if ( (ret = send_ew( menu->sock, request, len, 0, timeout )) != len ) {
	if (ret < 0 ) {
	    ret = WS_ERR_BROKEN_CONNECTION;
	} else {
	    ret = WS_ERR_TIMEOUT;
	}
	goto Abort;
    }

    ret = wsWaitBinHeader( menu, getThis->pBuf, getThis->bufLen, timeout );
    switch (ret) {
    case WS_ERR_INPUT:
	return ret;
	break;
    case WS_ERR_NONE:
	break;
    default:
	/* buffer overflow, timeout, socket error, or broken connection */
	goto Abort;
    }

    err = wsParseBinHeaderReply( getThis, getThis->pBuf );
    switch (err) {
    case WS_ERR_INPUT:
    case WS_WRN_FLAGGED:  /* no trace data to get */
	goto next_menu;
	break;
    case WS_ERR_PARSE:  /* can't get pending trace data, so close socket */
	/* DK 03192002 setting ret due to bug reported by Ilya Dricker */
	if(ret == WS_ERR_NONE)
	    ret = err;
	/* end DK 0319 change */
	goto Abort;
	break;
    default:  /* no error, so continue */
	break;
    }
  
    if ( getThis->actLen ) {
	long int binLen = getThis->actLen;
	
	if ( (int)getThis->bufLen < binLen )
	    binLen = (int)getThis->bufLen;
	ret = recv_all( menu->sock, getThis->pBuf, binLen, 0, timeout );
	
	if ( ret != binLen ) {
	    if ( ret < 0 ) {
		ret = WS_ERR_BROKEN_CONNECTION;
	    } else {
		ret = WS_ERR_TIMEOUT;
	    }
	    goto Abort;
	}
	
	if ( binLen < getThis->actLen ) {
	    getThis->actLen = binLen;
	    ret = WS_ERR_BUFFER_OVERFLOW;
	    if (WS_CL_DEBUG) 
		logit ( "e", "wsGetTraceBin(): buffer full, trace truncated\n" );
	    goto Abort;
	}
    }
    return WS_ERR_NONE;
  
 Abort:
    wsDetachServer( menu );
    if (WS_CL_DEBUG) {
	if  ( ret == WS_ERR_TIMEOUT ) {
	    logit( "e","wsGetTraceBin(): server %s:%s timed out\n", menu->addr,
		   menu->port );
	} else if ( ret == WS_ERR_BROKEN_CONNECTION ) {
	    logit( "e","wsGetTraceBin(): broken connection to server %s:%s\n",
		   menu->addr, menu->port);
	}
    }
  
    return ret;
}

/**************************************************************************
 *      wsGetTraceAsciiL: retrieves the ascii trace data specified in     *
 *      the structure 'getThis': The current menu list, as buit by the    *
 *      routines above will be searched for a matching SCNL. If a match   *
 *      is found, the associated wave server will be contacted, and a     *
 *      request for the trace snippet will be made.                       *
 *      This function requires location code at the wave_server           *
 **************************************************************************/

int wsGetTraceAsciiL( TRACE_REQ* getThis, WS_MENU_QUEUE_REC* menu_queue, 
		     int timeout )
     /*
       Arguments:
       getThis:   a TRACE_REQ structure (see ws_client.h), with the
       request portion filled in.
       menu_queue:   pointer to list of menues.
       timeout:   timeout interval in milliseconds.
       return:    WS_ERR_NONE: all went well.
       WS_ERR_EMPTY_MENU: No menu list found.
       WS_ERR_SCNL_NOT_IN_MENU: SCNL not found in menu list.
       WS_ERR_BUFFER_OVERFLOW: buffer too small for reply
       WS_ERR_BROKEN_CONNECTION: couldn't talk to server
       WS_ERR_TIMEOUT: if a timeout occured
       WS_ERR_NO_CONNECTION: There was no connection for the server
       WS_WRN_FLAGGED: wave server returned flag warning.
       the flag character is in the TRACE_REQ structure.
     */
{
    int ret, len, err;
    WS_MENU menu = NULL;
    WS_PSCNL pscnl = NULL;
    char request[wsREQLEN];
  
    if ( menu_queue->head == NULL ) {
	    if (WS_CL_DEBUG) logit("e", "wsGetTraceAsciiL: empty menu\n");
	    return WS_ERR_EMPTY_MENU;
    }
    
    if ( (ret = wsSearchSCNL( getThis, &menu, &pscnl, menu_queue )) 
	     != WS_ERR_NONE ) {
	    return ret; /* WS_ERR_SCNL_NOT_IN_MENU */
    }
    
    if ( menu->sock < 0 ) {
	    if (WS_CL_DEBUG) logit("e", "wsGetTraceAsciiL: no socket for %s:%s\n",
		        	       menu->addr, menu->port );
	    return WS_ERR_NO_CONNECTION;
    }
    
    sprintf( request, "GETSCNL: %d %s %s %s %s %lf %lf %ld\n",
	     menu_reqid++, pscnl->sta, pscnl->chan, pscnl->net, pscnl->loc,
	     getThis->reqStarttime, getThis->reqEndtime, getThis->fill );
    len = (int)strlen( request );
  
    if ( ( ret = send_ew( menu->sock, request, len, 0, timeout )) != len ) {
	    if (ret < 0 ) {
	        if (WS_CL_DEBUG) 
                logit("e", "wsGetTraceAsciiL: connection broke to server %s:%s\n", 
		                menu->addr, menu->port );
	        ret = WS_ERR_BROKEN_CONNECTION;
	    } else {
	        if (WS_CL_DEBUG)
		        logit("e", "wsGetTraceAsciiL: server %s:%s timed out\n", 
		            menu->addr, menu->port );
	        ret = WS_ERR_TIMEOUT;
	    }
	    goto Abort;
    }
    
    ret = wsWaitAscii( menu, getThis->pBuf, getThis->bufLen, timeout );
    if ( ret != WS_ERR_NONE && ret != WS_ERR_BUFFER_OVERFLOW ) {
	    goto Abort;
    } else {
	err = wsParseAsciiHeaderReplyL( getThis, getThis->pBuf );
	if ( err < WS_ERR_NONE ) {
	    if ( ( ret == WS_ERR_BUFFER_OVERFLOW ) && WS_CL_DEBUG )
	    	logit ( "e", "wsGetTraceAsciiL(): buffer overflow; parse error\n" );
	        return err;
	    }
    else if(ret == WS_ERR_BUFFER_OVERFLOW ) 
    {
        goto Abort;
    }
    }
    /* wsParseAsciiReplyL puts the trace data into getThis, so now we're done */
    return err;  /* return whatever code was returned by wsParseAsciiHeaderReplyL(), 
                    it should either be WS_ERR_NONE or WS_WRN_FLAGGED */
  
 Abort:
    wsDetachServer( menu );
    return ret;
}

/**************************************************************************
 *      wsGetTraceAscii: retrieves the ascii trace data specified in      *
 *      the structure 'getThis': The current menu list, as buit by the    *
 *      routines above will be searched for a matching SCN. If a match    *
 *      is found, the associated wave server will be contacted, and a     *
 *      request for the trace snippet will be made.                       *
 *      This function does not handle location codes at the wave_server.  *
 **************************************************************************/

int wsGetTraceAscii( TRACE_REQ* getThis, WS_MENU_QUEUE_REC* menu_queue, 
		     int timeout )
     /*
       Arguments:
       getThis:   a TRACE_REQ structure (see ws_client.h), with the
       request portion filled in.
       menu_queue:   pointer to list of menues.
       timeout:   timeout interval in milliseconds.
       return:    WS_ERR_NONE: all went well.
       WS_ERR_EMPTY_MENU: No menu list found.
       WS_ERR_SCNL_NOT_IN_MENU: SCNL not found in menu list.
       WS_ERR_BUFFER_OVERFLOW: buffer too small for reply
       WS_ERR_BROKEN_CONNECTION: couldn't talk to server
       WS_ERR_TIMEOUT: if a timeout occured
       WS_ERR_NO_CONNECTION: There was no connection for the server
       WS_WRN_FLAGGED: wave server returned flag warning.
       the flag character is in the TRACE_REQ structure.
     */
{
    int ret, len, err;
    WS_MENU menu = NULL;
    WS_PSCNL pscnl = NULL;
    char request[wsREQLEN];
  
    if ( menu_queue->head == NULL ) {
	if (WS_CL_DEBUG) logit("e", "wsGetTraceAscii: empty menu\n");
	return WS_ERR_EMPTY_MENU;
    }
    
    if ( (ret = wsSearchSCN( getThis, &menu, &pscnl, menu_queue )) 
	 != WS_ERR_NONE ) {
	return ret; /* WS_ERR_SCNL_NOT_IN_MENU */
    }
    
    if ( menu->sock < 0 ) {
	if (WS_CL_DEBUG) logit("e", "wsGetTraceAscii: no socket for %s:%s\n",
			       menu->addr, menu->port );
	return WS_ERR_NO_CONNECTION;
    }
    
    sprintf( request, "GETSCN: %d %s %s %s %lf %lf %ld\n",
	     menu_reqid++, pscnl->sta, pscnl->chan, pscnl->net,
	     getThis->reqStarttime, getThis->reqEndtime, getThis->fill );
    len = (int)strlen( request );
  
    if ( ( ret = send_ew( menu->sock, request, len, 0, timeout )) != len ) {
	if (ret < 0 ) {
	    if (WS_CL_DEBUG) 
		logit("e", "wsGetTraceAscii: connection broke to server %s:%s\n", 
		      menu->addr, menu->port );
	    ret = WS_ERR_BROKEN_CONNECTION;
	} else {
	    if (WS_CL_DEBUG)
		logit("e", "wsGetTraceAscii: server %s:%s timed out\n", 
		      menu->addr, menu->port );
	    ret = WS_ERR_TIMEOUT;
	}
	goto Abort;
    }
    
    ret = wsWaitAscii( menu, getThis->pBuf, getThis->bufLen, timeout );
    if ( ret != WS_ERR_NONE && ret != WS_ERR_BUFFER_OVERFLOW ) {
	goto Abort;
    } else {
	err = wsParseAsciiHeaderReply( getThis, getThis->pBuf );
	if ( err < WS_ERR_NONE ) {
	    if ( ( ret == WS_ERR_BUFFER_OVERFLOW ) && WS_CL_DEBUG )
		logit ( "e", "wsGetTraceAscii(): buffer overflow; parse error\n" );
	    return err;
	}
    }
    /* wsParseAsciiReply puts the trace data into getThis, so now we're done */
    return WS_ERR_NONE;
  
 Abort:
    wsDetachServer( menu );
    return ret;
}

/********************************************************
 *  wsKillPSCNL: Deallocates the PSCNL list             *
 *  This function is not concerned with location codes. *
 ********************************************************/
void wsKillPSCNL( WS_PSCNL pscnl )
     /*
       Arguments:
       pscnl: pointer to a list of scnl structures
     */
{
    while ( pscnl ) {
	WS_PSCNL next = pscnl->next;
	
	free( pscnl );
	pscnl = next;
    }
    return;
}

/********************************************************************
 * wsGetServerPSCNL: Return the pscnl list for this server from the *
 *       menu queue                                                 *
 *  This function is not concerned with location codes.             *
 ********************************************************************/
int wsGetServerPSCNL( char* addr, char* port, WS_PSCNL* pscnlp,
		     WS_MENU_QUEUE_REC* menu_queue )
     /*
       Arguments: 
       addr: IP address of the server
       port: port number of the server
       pscnlp: pointer to the pscnl list to be returned
       menu_queue: pointer to list of menus.
       return: WS_ERR_NONE: all went well
       WS_ERR_EMPTY_MENU: no menu in the queue
       WS_ERR_SERVER_NOT_IN_MENU: server's menu not in the queue
     */
{
    int ret = menu_queue->head ? WS_ERR_SERVER_NOT_IN_MENU : WS_ERR_EMPTY_MENU;
    WS_MENU menu = menu_queue->head;

    *pscnlp = NULL;
    while ( menu ) {
	if ( strncmp( addr, menu->addr, wsADRLEN ) == 0 &&
	     strncmp( port, menu->port, wsADRLEN ) == 0 ) {
	    ret = WS_ERR_NONE;
	    *pscnlp = menu->pscnl;
	    break;
	}
	menu = menu->next;
    }
    
    if (WS_CL_DEBUG) {
	if ( ret == WS_ERR_SERVER_NOT_IN_MENU )
	    logit( "e","wsGetServerPSCNL(): WS_ERR_SERVER_NOT_IN_MENU\n" );
	else if ( ret == WS_ERR_EMPTY_MENU )
	    logit( "e","wsGetServerPSCNL(): WS_ERR_EMPTY_MENU\n" );
    }
    
    return ret;
}


/*************************************************************************
 * wsSearchSCNL: Find menu and PSCNL in queue which will serve this scnl *
 *              If menup points to a menu in the menu_queue, search      *
 *              starts at the next menu after *menup; otherwise,         *
 *              search starts at menu_queue->head.                       *
 *              If the SCNL is listed more than once in the queue, only  *
 *              the first menu and PSCNL will be returned.               *
 *  This function assumes the presence of location code in menus.        *
 *************************************************************************/
int wsSearchSCNL( TRACE_REQ* getThis, WS_MENU* menup, WS_PSCNL* pscnlp,
		 WS_MENU_QUEUE_REC* menu_queue )
     /*
       Arguments:
       getThis: a TRACE_REQ structure with the SCNL to search for.
       menup: pointer to the menu to return.
       pscnlp: pointer to the pscnl list to return.
       returns: WS_ERR_NONE: if all went well
       WS_ERR_EMPTY_MENU: no menus in the queue
       WS_ERR_SERVER_NOT_IN_MENU: scnl not in the queue
     */
{
    int ret;
    WS_MENU menu;

    if ( *menup != NULL)
    	menu = (*menup)->next;
    else
	    menu  = menu_queue->head;
    ret = menu ? WS_ERR_SCNL_NOT_IN_MENU : WS_ERR_EMPTY_MENU;
    *pscnlp = NULL;

    while ( menu ) {
	    WS_PSCNL pscnl = menu->pscnl;
	    while ( pscnl ) {
	        if ( strncmp( getThis->sta, pscnl->sta, TRACE2_STA_LEN ) == 0 &&
		        strncmp( getThis->chan, pscnl->chan, TRACE2_CHAN_LEN ) == 0 &&
		        strncmp( getThis->net, pscnl->net, TRACE2_NET_LEN ) == 0 &&
		        strncmp( getThis->loc, pscnl->loc, TRACE2_LOC_LEN ) == 0 ) {
		            ret = WS_ERR_NONE;
		            *menup = menu;
		            *pscnlp = pscnl;
		            goto exit;
	        }
	        pscnl = pscnl->next;
	    }
	    menu = menu->next;
    }
    
 exit:
    if (WS_CL_DEBUG) {
	    if ( ret == WS_ERR_SCNL_NOT_IN_MENU )
	        logit( "e","wsSearchSCNL(): WS_ERR_SCNL_NOT_IN_MENU\n" );
	    else if ( ret == WS_ERR_EMPTY_MENU )
	        logit( "e","wsSearchSCNL(): WS_ERR_EMPTY_MENU\n" );
    }
    
    return ret;
}

/***********************************************************************
 * wsSearchSCN: Find menu and PSCNL in queue which will serve this scn *
 *              If menup points to a menu in the menu_queue, search    *
 *              starts at the next menu after *menup; otherwise,       *
 *              search starts at menu_queue->head.                     *
 *              If the SCN is listed more than once in the queue, only *
 *              the first menu and PSCNL will be returned.             *
 *  This function ignores the presence of location code in menus.      *
 ***********************************************************************/
int wsSearchSCN( TRACE_REQ* getThis, WS_MENU* menup, WS_PSCNL* pscnlp,
		 WS_MENU_QUEUE_REC* menu_queue )
     /*
       Arguments:
       getThis: a TRACE_REQ structure with the SCNL to search for.
       menup: pointer to the menu to return.
       pscnlp: pointer to the pscnl list to return.
       returns: WS_ERR_NONE: if all went well
       WS_ERR_EMPTY_MENU: no menus in the queue
       WS_ERR_SERVER_NOT_IN_MENU: scnl not in the queue
     */
{
    int ret;
    WS_MENU menu;

    if ( *menup != NULL)
	menu = (*menup)->next;
    else
	menu  = menu_queue->head;
    ret = menu ? WS_ERR_SCNL_NOT_IN_MENU : WS_ERR_EMPTY_MENU;
    *pscnlp = NULL;

    while ( menu ) {
	WS_PSCNL pscnl = menu->pscnl;
	while ( pscnl ) {
	    if ( strncmp( getThis->sta, pscnl->sta, TRACE2_STA_LEN ) == 0 &&
		 strncmp( getThis->chan, pscnl->chan, TRACE2_CHAN_LEN ) == 0 &&
		 strncmp( getThis->net, pscnl->net, TRACE2_NET_LEN ) == 0 ) {
		ret = WS_ERR_NONE;
		*menup = menu;
		*pscnlp = pscnl;
		goto exit;
	    }
	    pscnl = pscnl->next;
	}
	menu = menu->next;
    }
    
 exit:
    if (WS_CL_DEBUG) {
	if ( ret == WS_ERR_SCNL_NOT_IN_MENU )
	    logit( "e","wsSearchSCN(): WS_ERR_SCNL_NOT_IN_MENU\n" );
	else if ( ret == WS_ERR_EMPTY_MENU )
	    logit( "e","wsSearchSCN(): WS_ERR_EMPTY_MENU\n" );
    }
    
    return ret;
}

/************************************************************************
 *  wsAttachServer: Open a connection to a server. The timeout starts   *
 *    when connect() is called by connect_ew() via setuptcpclient_ew() *
 *  This function is not concerned with location codes.                 *
 ***********************************************************************/
int wsAttachServer( WS_MENU menu, int timeout )
     /*
       Arguemnts:
       menu: pointer to the menu of the server
       timeout: time interval in milliseconds; use -1 for no timeout.
       returns: WS_ERR_NONE: if all went well.
       WS_ERR_INPUT: if menu is missing.
       WS_ERR_SOCKET: if a socket error occurred.
       WS_ERR_NO_CONNECTION: if a connection could not be established
     */
{
    int                ret = WS_ERR_NONE;
    SOCKET             sock = 0;   /* Socket descriptor                  */

    if ( !menu ) {
	ret = WS_ERR_INPUT;
	if (WS_CL_DEBUG) logit( "e", "wsAttachServer(): WS_ERR_INPUT\n");
	goto Abort;
    }
    if ( menu->sock > 0 ) { /* maybe already connected, so disconnect first */ 
	    wsDetachServer( menu );
    }

    if( ( sock = setuptcpclient_ew(menu->addr, menu->port, timeout)) < 0) {
      if(sock == -2) {
         ret = WS_ERR_SOCKET;
         if (WS_CL_DEBUG) logit("e", "wsAttachServer(): socket call failed\n");
      }  else {
         ret = WS_ERR_NO_CONNECTION;
         if (WS_CL_DEBUG) logit("e", "wsAttachServer(): connection failed\n");
      }
      goto Abort;
    }

    menu->sock = sock;

    ret = WS_ERR_NONE;
    return ret;

    /* An error occured;
     * don't blab about here since we already did earlier. */
 Abort:
    if (menu) menu->sock = (SOCKET)-1; /* mark the socket as dead */
    return ret;
}


/*********************************************************************
 * wsDetachServer: Immediately disconnect from a socket if it's open *
 *  This function is not concerned with location codes.              *
 *********************************************************************/
void wsDetachServer( WS_MENU menu )
     /*  
	 Arguments:
	 menu: menu of server to be detached
     */
{
    if ( !menu || menu->sock == -1 )
	return;
    closesocket_ew( menu->sock, SOCKET_CLOSE_IMMEDIATELY_EW );
    menu->sock = (SOCKET)-1;
}


/*********************************************************************
 * wsWaitBinHeader: Retrieve the ASCII header of a binary message.   *
 * The header will be terminated by a newline, but binary characters *
 * will follow in the same message. Thus this routine must read one  *
 * character at a time. Since the header is relatively short, this   *
 * should not be much a performance hit.                             *
 * Returns after newline is read, when timeout expires if set,       *
 * or on error.                                                      *
 * This function is not concerned with location codes.               *
 *********************************************************************/
static int wsWaitBinHeader( WS_MENU menu, char* buf, int buflen,
			    int timeout_msec )
     /*  
	 Arguments:
	 menu: menu of server from which message is received
	 buf: buffer in which to place the message, terminated by null.
         buflen: number of bytes in the buffer.
	 timeout_msec: timout interval in milliseconds. 
         return: WS_ERR_NONE: all went well.
	 WS_ERR_BUFFER_OVERFLOW: ran out of space before the message
	 end; calling program will have to decide how serious this is.
	 WS_ERR_INPUT: missing input parameters.
	 WS_ERR_SOCKET: error setting socket options.
	 WS_ERR_TIMEOUT: time expired before we read everything.
	 WS_ERR_BROKEN_CONNECTION: if the connection failed.
     */
{
    int ir = 0;
    int nr = 0;
    char c = '\0';
    int ret, ioctl_ret;
    fd_set ReadableSockets;
    Time_ew StartTime;
    struct timeval SelectTimeout;
    Time_ew timeout = adjustTimeoutLength(timeout_msec);
    long lOnOff;
    
    if ( !buf || buflen <= 0 ) {
	if (WS_CL_DEBUG) logit( "e", "wsWaitBinHeader(): no input buffer\n");
	return WS_ERR_INPUT;
    }
    
    /* If there is no timeout, make the socket blocking */
    if (timeout_msec == -1) {
	timeout = 0;
	lOnOff = 0;
	ioctl_ret = ioctlsocket(menu->sock, FIONBIO, (u_long *)&lOnOff);
	if (ioctl_ret == SOCKET_ERROR) {
	    ret = WS_ERR_SOCKET;
	    if (WS_CL_DEBUG) 
		logit("et", "wsWaitBinHeader: error %d occurred during change to blocking\n",
		      socketGetError_ew());
	    goto Done;
	}
    }
    
    StartTime = GetTime_ew(); /* the timer starts here */
    /* Start reading the socket, one character at a time */
    while ( c != '\n' ) {
	if ((timeout) && (GetTime_ew() - timeout) > StartTime ) {
	    ret = WS_ERR_TIMEOUT;
	    if (WS_CL_DEBUG) logit("et", "wsWaitBinHeader timed out\n");
	    goto Done;
	}
	if ( ir == buflen - 1 ) {
	    /* stop if there's no more room  */
	    if (WS_CL_DEBUG) 
		logit( "e", "wsWaitBinHeader(): reply buffer overflows\n" );
	    ret = WS_ERR_BUFFER_OVERFLOW;
	    goto Done;
	}
	
	/* try to get a char from socket */
	nr = recv( menu->sock, &c, 1, 0 );
	if ( nr == -1 && socketGetError_ew() == WOULDBLOCK_EW ) {
	    FD_ZERO( &ReadableSockets );
	    FD_SET( (SOCKET)(menu->sock), &ReadableSockets );
	    while (( !select(menu->sock + 1, &ReadableSockets, 0, 0,
			     resetTimeout( &SelectTimeout)))) {
		if ((timeout) && (GetTime_ew() - timeout) > StartTime ) {
		    ret = WS_ERR_TIMEOUT;
		    if (WS_CL_DEBUG) logit("et", "wsWaitBinHeader timed out\n");
		    goto Done;
		}
		FD_ZERO( &ReadableSockets );
		FD_SET( (SOCKET)(menu->sock), &ReadableSockets );
		sleep_ew(100); /* wait a little and try again */
	    }
	    /* select() says won't block */
	    nr = recv( menu->sock, &c, 1, 0 ); 
	}
	if ( nr == 1 && c != 0 ) {
	    /* got a character; save it      */
	    buf[ir++] = c;
	}
	else if ( nr == -1 ) {
	    /* trouble reading socket        */
	    ret = WS_ERR_BROKEN_CONNECTION;
	    if (WS_CL_DEBUG) 
		logit( "e", "wsWaitBinHeader(): Error on socket recv()\n" );
	    goto Done;
	}
    }
    buf[ir] = '\0';                   /* null-terminate the buf      */
    
    ret = WS_ERR_NONE;

 Done:
    buf[ir] = '\0';                 /* null-terminate the buf      */
    /* If there was no timeout, then change the socket back to non-blocking */
    if (timeout_msec == -1) {
	lOnOff = 1;
	ioctl_ret = ioctlsocket( menu->sock, FIONBIO, (u_long *)&lOnOff);
	if (ioctl_ret == SOCKET_ERROR) {
	    
	    if (WS_CL_DEBUG) 
		logit("et","wsWaitBinHeader: error %d occurred during change to non-blocking\n", 
		      socketGetError_ew() );
	    ret = WS_ERR_SOCKET;
	}
    }
    return ret;
}


/*******************************************************************
 * wsWaitAscii: Retrieve an ASCII message.                         *
 * The message will be terminated by a newline; nothing else is    *
 * expected after the newline, so we can read several characters   *
 * at a time without fear of reading past the newline.             *
 * This message may have internal nulls which will be converted to *
 * spaces.                                                         *
 * Returns after newline is read, when timeout expires if set,     *
 * or on error.                                                    *
 * This function is not concerned with location codes.             *
 *******************************************************************/
static int wsWaitAscii( WS_MENU menu, char* buf, int buflen, int timeout_msec )
     /*
       Arguments:
       menu: menu of server from which message is received
       buf: buffer in which to place the message, terminated by null.
       buflen: number of bytes in the buffer.
       timeout_msec: timout interval in milliseconds. 
       return: WS_ERR_NONE: all went well.
       WS_ERR_BUFFER_OVERFLOW: ran out of space before the message
       end; calling program will have to decide how serious this is.
       WS_ERR_INPUT: missing input parameters.
       WS_ERR_SOCKET: error setting socket options.
       WS_ERR_TIMEOUT: time expired before we read everything.
       WS_ERR_BROKEN_CONNECTION: if the connection failed.
     */
{
    int ii, ir = 0;  /* character counters */
    int nr = 0;
    char c = '\0';
    int len = 0;
    int ret, ioctl_ret;
    fd_set ReadableSockets;
    Time_ew StartTime;
    struct timeval SelectTimeout;
    Time_ew timeout = adjustTimeoutLength(timeout_msec);
    long lOnOff;
  
    if ( !buf || buflen <= 0 ) {
	if (WS_CL_DEBUG) logit( "e", "wsWaitAscii(): no input buffer\n");
	return WS_ERR_INPUT;
    }
    
    /* If there is no timeout, make the socket blocking */
    if (timeout_msec == -1) {
	timeout = 0;
	lOnOff = 0;
	ioctl_ret = ioctlsocket(menu->sock, FIONBIO, (u_long *)&lOnOff);
	if (ioctl_ret == SOCKET_ERROR) {
	    ret = WS_ERR_SOCKET;
	    if (WS_CL_DEBUG) logit("et",
				   "wsWaitAscii: error %d occurred during change to blocking\n",
				   socketGetError_ew());
	    goto Done;
	}
    }
    
    StartTime = GetTime_ew(); /* the timer starts here */
    while ( c != '\n' ) {
	if ((timeout) && (GetTime_ew() - timeout) > StartTime ) {
	    ret = WS_ERR_TIMEOUT;
	    if (WS_CL_DEBUG) logit("et", "wsWaitAscii timed out\n");
	    goto Done;
	}
	if ( ir >= buflen - 2 ) {
	    if (WS_CL_DEBUG)
		logit( "e", "wsWaitAscii(): reply buffer overflows\n" );
	    ret = WS_ERR_BUFFER_OVERFLOW;
	    goto Done;
	}
	len = WS_MAX_RECV_BUF_LEN;
	if ( ir + len >= buflen - 1 )
	    len = buflen - ir - 2; /* leave room for the terminating null */
	nr = recv( menu->sock, &buf[ir], len, 0 );
	if ( nr == -1 && socketGetError_ew() == WOULDBLOCK_EW ) {
	    FD_ZERO( &ReadableSockets );
	    FD_SET( (SOCKET)(menu->sock), &ReadableSockets );
	    while (( !select(menu->sock + 1, &ReadableSockets, 0, 0,
			     resetTimeout( &SelectTimeout)))) {
		if ((timeout) && (GetTime_ew() - timeout) > StartTime ) {
		    ret = WS_ERR_TIMEOUT;
		    if (WS_CL_DEBUG) logit("et", "wsWaitAscii timed out\n");
		    goto Done;
		}
		FD_ZERO( &ReadableSockets );
		FD_SET( (SOCKET)(menu->sock), &ReadableSockets );
		sleep_ew(100); /* wait a little and try again */
	    }
	    /* poll() says won't block */
	    nr = recv( menu->sock, &buf[ir], len, 0 ); 
	}
	
	if ( nr == -1 || nr > len ) {
	    /* trouble reading socket        */
	    ret = WS_ERR_BROKEN_CONNECTION;
	    if (WS_CL_DEBUG) logit( "e", "wsWaitAscii(): Error on socket recv()\n" );
	    goto Done;
	}
	if ( nr > 0 ) {
	    ii = 0;
	    /* got something, adjust ir and c  */
	    ir += nr;
	    c = buf[ir-1];
	    
	    /* replace NULL char in ascii string with SPACE char */
	    while ( ii < nr ) {
		if ( !buf[ir-nr+ii] ) buf[ir-nr+ii] = ' ';
		++ii;
	    }
	}
    }
    
    ret = WS_ERR_NONE;
 Done:
    buf[ir] = '\0';                 /* null-terminate the reply      */
    /* If there was no timeout, then change the socket back to non-blocking */
    if (timeout_msec == -1) {
	lOnOff = 1;
	ioctl_ret = ioctlsocket( menu->sock, FIONBIO, (u_long *)&lOnOff);
	if (ioctl_ret == SOCKET_ERROR) {
	    
	    if (WS_CL_DEBUG) logit("et", "wsWaitAScii: error %d occurred during change to non-blocking\n", 
				   socketGetError_ew() );
	    ret = WS_ERR_SOCKET;
	}
    }
    return ret;
}

/************************************************************************
 * wsParseMenuReply: parse the reply we got from the waveserver into    *
 * a menu list. Handles replies to MENU, MENUPIN and MENUSCNL requests. *
 * This function handles the presence or absence of location codes.     *
 ************************************************************************/
static int wsParseMenuReply( WS_MENU menu, char* reply )
{
    /* Arguments:
     *       menu: pointer to menu structure to be allocated and filled in.
     *      reply: pointer to reply to be parsed.
     *   Returns: WS_ERR_NONE:  if all went well
     *            WS_ERR_INPUT: if bad input parameters
     *            WS_ERR_PARSE: if we couldn't parse the reply
     *            WS_ERR_MEMORY: if out of memory
     */
    int reqid = 0;
    int pinno = 0;
   /* allow extra room in these input buffer fields,  so that we are unlikely to clobber
      memory if the data coming in is larger than the spec'd definition. */
    char    sta[TRACE2_STA_LEN*3];         /* Site name */
    char    chan[TRACE2_CHAN_LEN*3];       /* Component/channel code */
    char    net[TRACE2_NET_LEN*3];         /* Network name */
    char    loc[TRACE2_LOC_LEN*3];         /* Location code */

    double tankStarttime = 0.0, tankEndtime = 0.0;
    char datatype[3];
    int scnl_pos = 0;
    int skip;
    
    if ( !reply || !menu ) {
	if (WS_CL_DEBUG) logit("e", "wsParseMenuReply: WS_ERR_INPUT\n");
	return WS_ERR_INPUT;
    }
    
    if ( sscanf( &reply[scnl_pos], "%d", &reqid ) < 1 ) {
	if (WS_CL_DEBUG)
	    logit( "e","wsParseMenuReply(): error parsing reqid\n" );
	return WS_ERR_PARSE;
    }
    wsSkipN( reply, 1, &scnl_pos );
    while ( reply[scnl_pos] && reply[scnl_pos] != '\n' ) {
	WS_PSCNL pscnl = NULL;
	skip = 8;
	if ( sscanf( &reply[scnl_pos], "%d %s %s %s %s %lf %lf %s",
		     &pinno, sta, chan, net, loc,
		     &tankStarttime, &tankEndtime, datatype ) < 8 ) {
	    /* Try parsing for SCN instead of SCNL */
	    if ( sscanf( &reply[scnl_pos], "%d %s %s %s %lf %lf %s",
			 &pinno, sta, chan, net, 
			 &tankStarttime, &tankEndtime, datatype ) < 7 ) {
		if (WS_CL_DEBUG)
		    logit( "e","wsParseMenuReply(): error decoding reply<%s>\n",
			   &reply[scnl_pos] );
		return WS_ERR_PARSE;
	    }
	    strcpy(loc, "");
	    skip = 7;
	}
	pscnl = ( WS_PSCNL_REC* )calloc(sizeof(WS_PSCNL_REC),1);
	if ( pscnl == NULL ) {
	    if (WS_CL_DEBUG)
		logit("e", "wsParseMenuReply(): error allocating memory\n");
	    return WS_ERR_MEMORY;
	}
	
	pscnl->next = menu->pscnl;
	pscnl->pinno = (short)pinno;

	/* truncate the SCNL input buffers at the spec'd Tracebuf2 length */
	sta[TRACE2_STA_LEN-1]=0x00;
	chan[TRACE2_CHAN_LEN-1]=0x00;
	net[TRACE2_NET_LEN-1]=0x00;
	loc[TRACE2_LOC_LEN-1]=0x00;
  
	strncpy( pscnl->sta, sta, TRACE2_STA_LEN );
	strncpy( pscnl->chan, chan, TRACE2_CHAN_LEN );
	strncpy( pscnl->net, net, TRACE2_NET_LEN );
	strncpy( pscnl->loc, loc, TRACE2_LOC_LEN );
	pscnl->tankStarttime = tankStarttime;
	pscnl->tankEndtime = tankEndtime;
	menu->pscnl = pscnl;
	wsSkipN( reply, skip, &scnl_pos );
    }
    
    return WS_ERR_NONE;
}

/***********************************************************************
 * wsParseBinHeaderReply: parse the reply we got from the waveserver   *
 * into a TRACE_REQ structure. Handles the header for replies reply to *
 * GETSCNLRAW requests.                                                *
 * This function requires the presence of location code.               *
 ***********************************************************************/
static int wsParseBinHeaderReplyL( TRACE_REQ* getThis, char* reply )
{
    /* Arguments:
     *    getThis: pointer to TRACE_REQ structure to be filled with reply info
     *      reply: pointer to reply to be parsed.
     *   Returns: WS_ERR_NONE:  if all went well
     *            WS_ERR_INPUT: if bad input parameters
     *            WS_ERR_PARSE: if we couldn't parse part of the reply
     *            WS_WRN_FLAGGED: server sent us a no-data flag
     */
    int reqid = 0;
    int pinno = 0;
    char sta[TRACE2_STA_LEN];
    char chan[TRACE2_CHAN_LEN];
    char net[TRACE2_NET_LEN];
    char loc[TRACE2_LOC_LEN];
    char flag[9];
    char datatype[3];
    double tankStarttime = 0.0, tankEndtime = 0.0;
    int bin_len = 0;
    int scnl_pos = 0;

    if ( !reply || !getThis ) {
	    if (WS_CL_DEBUG)
	        logit( "e", "wsParseBinHeaderReplyL(): bad input parameters\n");
	    return WS_ERR_INPUT;
    }
    
    if ( sscanf( &reply[scnl_pos], "%d %d", &reqid, &pinno ) < 2 ) {
	    if (WS_CL_DEBUG)
	        logit( "e","wsParseBinHeaderReplyL(): error parsing reqid/pinno\n" );
	    return WS_ERR_PARSE;
    }
    wsSkipN( reply, 2, &scnl_pos );
    
    if ( sscanf( &reply[scnl_pos], "%s %s %s %s", sta, chan, net, loc ) < 4 ) {
	if (WS_CL_DEBUG) logit( "e","wsParseBinHeaderReplyL(): error parsing SCNL\n" );
	return WS_ERR_PARSE;
    }
    wsSkipN( reply, 4, &scnl_pos );
    
    if ( sscanf( &reply[scnl_pos], "%s %s", flag, datatype ) < 2 ) {
	    if (WS_CL_DEBUG)
	        logit( "e","wsParseBinHeaderReplyL(): error parsing flag/datatype\n" );
        if(strlen(flag) == 2)
            strcpy(datatype, "XX");  /* work around a bug in wave_server where it doesn't issue correct reply
                                        on some odd flags */
        else
	        return WS_ERR_PARSE;
    }
    wsSkipN( reply, 2, &scnl_pos );
    
    if ( strlen(flag) == 1 ) {
	    if ( sscanf( &reply[scnl_pos], "%lf %lf", &tankStarttime, 
		         &tankEndtime ) < 2 ) {
	        if (WS_CL_DEBUG) 
		        logit( "e","wsParseBinHeaderReplyL(): error parsing starttime/endtime\n" );
	        return WS_ERR_PARSE;
	    }
	    wsSkipN( reply, 2, &scnl_pos );
	
    	if ( sscanf( &reply[scnl_pos], "%d", &bin_len ) < 1 ) {
	        if (WS_CL_DEBUG) 
		        logit( "e","wsParseBinHeaderReplyL(): error parsing bin_len\n" );
	        return WS_ERR_PARSE;
	    }
	    wsSkipN( reply, 1, &scnl_pos );
    }
    else if ( strlen(flag) == 2 ) {
	    tankStarttime = 0.0;
	    tankEndtime = 0.0;
	    bin_len = 0;
	    if ( strcmp(flag,"FL") == 0 ) {
	        if ( sscanf( &reply[scnl_pos], "%lf", &tankStarttime) < 1 ) {
		        if (WS_CL_DEBUG) 
		            logit( "e","wsParseBinHeaderReplyL(): error parsing starttime\n" );
		        return WS_ERR_PARSE;
	        }
	        wsSkipN( reply, 1, &scnl_pos );
	    }
	    else if ( strcmp(flag,"FR") == 0 ) {
	        if ( sscanf( &reply[scnl_pos], "%lf", &tankEndtime) < 1 ) {
		        if (WS_CL_DEBUG) 
		            logit( "e","wsParseBinHeaderReplyL(): error parsing endtime\n" );
		        return WS_ERR_PARSE;
	        }
	        wsSkipN( reply, 1, &scnl_pos );
	    }
    } else {
	    if (WS_CL_DEBUG) 
	        logit( "e","wsParseBinHeaderReplyL(): bad flag[%s]\n", flag );
	    return WS_ERR_PARSE;
    }
    
    getThis->pinno = (short)pinno;
    getThis->actStarttime = tankStarttime;
    getThis->actEndtime = tankEndtime;
    getThis->samprate = (double) 0.0; /* server doesn't send this */
    getThis->actLen = bin_len;
    if ( strlen( flag ) >= 2 ) {
	    getThis->retFlag = flag[1];
	    return WS_WRN_FLAGGED;
    } else {
	    getThis->retFlag = '\0';
	    return WS_ERR_NONE;
    }
}


/***********************************************************************
 * wsParseBinHeaderReply: parse the reply we got from the waveserver   *
 * into a TRACE_REQ structure. Handles the header for replies reply to *
 * GETSCNRAW requests.                                                 *
 * This function requires the absence of location codes.               *
 ***********************************************************************/
static int wsParseBinHeaderReply( TRACE_REQ* getThis, char* reply )
{
    /* Arguments:
     *    getThis: pointer to TRACE_REQ structure to be filled with reply info
     *      reply: pointer to reply to be parsed.
     *   Returns: WS_ERR_NONE:  if all went well
     *            WS_ERR_INPUT: if bad input parameters
     *            WS_ERR_PARSE: if we couldn't parse part of the reply
     *            WS_WRN_FLAGGED: server sent us a no-data flag
     */
    int reqid = 0;
    int pinno = 0;
    char sta[TRACE2_STA_LEN];
    char chan[TRACE2_CHAN_LEN];
    char net[TRACE2_NET_LEN];
    char flag[9];
    char datatype[3];
    double tankStarttime = 0.0, tankEndtime = 0.0;
    int bin_len = 0;
    int scnl_pos = 0;

    if ( !reply || !getThis ) {
	if (WS_CL_DEBUG)
	    logit( "e", "wsParseBinHeaderReply(): bad input parameters\n");
	return WS_ERR_INPUT;
    }
    
    if ( sscanf( &reply[scnl_pos], "%d %d", &reqid, &pinno ) < 2 ) {
	if (WS_CL_DEBUG)
	    logit( "e","wsParseBinHeaderReply(): error parsing reqid/pinno\n" );
	return WS_ERR_PARSE;
    }
    wsSkipN( reply, 2, &scnl_pos );
    
    if ( sscanf( &reply[scnl_pos], "%s %s %s", sta, chan, net ) < 3 ) {
	if (WS_CL_DEBUG) logit( "e","wsParseBinHeaderReply(): error parsing SCN\n" );
	return WS_ERR_PARSE;
    }
    wsSkipN( reply, 3, &scnl_pos );
    
    if ( sscanf( &reply[scnl_pos], "%s %s", flag, datatype ) < 2 ) {
	if (WS_CL_DEBUG)
	    logit( "e","wsParseBinHeaderReply(): error parsing flag/datatype\n" );
	return WS_ERR_PARSE;
    }
    wsSkipN( reply, 2, &scnl_pos );
    
    if ( strlen(flag) == 1 ) {
	if ( sscanf( &reply[scnl_pos], "%lf %lf", &tankStarttime, 
		     &tankEndtime ) < 2 ) {
	    if (WS_CL_DEBUG) 
		logit( "e","wsParseBinHeaderReply(): error parsing starttime/endtime\n" );
	    return WS_ERR_PARSE;
	}
	wsSkipN( reply, 2, &scnl_pos );
	
	if ( sscanf( &reply[scnl_pos], "%d", &bin_len ) < 1 ) {
	    if (WS_CL_DEBUG) 
		logit( "e","wsParseBinHeaderReply(): error parsing bin_len\n" );
	    return WS_ERR_PARSE;
	}
	wsSkipN( reply, 1, &scnl_pos );
	
    }
    else if ( strlen(flag) == 2 ) {
	tankStarttime = 0.0;
	tankEndtime = 0.0;
	bin_len = 0;
	if ( strcmp(flag,"FL") == 0 ) {
	    if ( sscanf( &reply[scnl_pos], "%lf", &tankStarttime) < 1 ) {
		if (WS_CL_DEBUG) 
		    logit( "e","wsParseBinHeaderReply(): error parsing starttime\n" );
		return WS_ERR_PARSE;
	    }
	    wsSkipN( reply, 1, &scnl_pos );
	}
	else if ( strcmp(flag,"FR") == 0 ) {
	    if ( sscanf( &reply[scnl_pos], "%lf", &tankEndtime) < 1 ) {
		if (WS_CL_DEBUG) 
		    logit( "e","wsParseBinHeaderReply(): error parsing endtime\n" );
		return WS_ERR_PARSE;
	    }
	    wsSkipN( reply, 1, &scnl_pos );
	}
    } else {
	if (WS_CL_DEBUG) 
	    logit( "e","wsParseBinHeaderReply(): bad flag[%s]\n", flag );
	return WS_ERR_PARSE;
    }
    
    getThis->pinno = (short)pinno;
    getThis->actStarttime = tankStarttime;
    getThis->actEndtime = tankEndtime;
    getThis->samprate = (double) 0.0; /* server doesn't send this */
    getThis->actLen = bin_len;
    if ( strlen( flag ) >= 2 ) {
	getThis->retFlag = flag[1];
	return WS_WRN_FLAGGED;
    } else {
	getThis->retFlag = '\0';
	return WS_ERR_NONE;
    }
}


/************************************************************************
 * wsParseAsciiHeaderReplyL: parse the reply we got from the waveserver *
 * into a TRACE_REQ structure. Handles the header for replies reply to  *
 * GETSCNL and GETPIN requests.                                         *
 * This function requires the presence of location code.                *
 ************************************************************************/
static int wsParseAsciiHeaderReplyL( TRACE_REQ* getThis, char* reply )
{
    /* Arguments:
     *    getThis: pointer to TRACE_REQ structure to be filled with reply info
     *      reply: pointer to reply to be parsed.
     *   Returns: WS_ERR_NONE:  if all went well
     *            WS_ERR_INPUT: if bad input parameters
     *            WS_ERR_PARSE: if we couldn't parse part of the reply
     *            WS_WRN_FLAGGED: server sent us a no-data flag
     */
    int reqid = 0;
    int pinno = 0;
    char sta[TRACE2_STA_LEN];
    char chan[TRACE2_CHAN_LEN];
    char net[TRACE2_NET_LEN];
    char loc[TRACE2_LOC_LEN];
    char flag[9];
    char datatype[3];
    double tankStarttime = 0.0, samprate = 0.0;
    int scnl_pos = 0;

    if ( !reply ) {
	    if (WS_CL_DEBUG) 
	        logit( "e", "wsParseAsciiHeaderReplyL(): bad input parameters\n");
	    return WS_ERR_INPUT;
    }
    
    if ( sscanf( &reply[scnl_pos], "%d %d", &reqid, &pinno ) < 2 ) {
	    if (WS_CL_DEBUG) 
	        logit( "e","wsParseAsciiHeaderReplyL(): error parsing reqid/pinno\n" );
	    return WS_ERR_PARSE;
    }
    wsSkipN( reply, 2, &scnl_pos );
    
    if ( sscanf( &reply[scnl_pos], "%s %s %s %s", sta, chan, net, loc ) < 4 ) {
	    if (WS_CL_DEBUG) 
	        logit( "e","wsParseAsciiHeaderReplyL(): error parsing SCNL\n" );
	    return WS_ERR_PARSE;
    }
    wsSkipN( reply, 4, &scnl_pos );
    
    if ( sscanf( &reply[scnl_pos], "%s %s", flag, datatype ) < 2 ) {
	    if (WS_CL_DEBUG) 
	        logit( "e","wsParseAsciiHeaderReplyL(): error parsing flag/datatype\n" );
	    return WS_ERR_PARSE;
    }
    wsSkipN( reply, 2, &scnl_pos );
    
    if ( strlen(flag) == 1 || strcmp(flag,"FG") == 0 ) {
	    if ( sscanf( &reply[scnl_pos], "%lf %lf", &tankStarttime, &samprate ) 
	         < 2 ) {
	        if (WS_CL_DEBUG)
		        logit( "e","wsParseAsciiHeaderReplyL(): error parsing startT/samprate\n" );
	        return WS_ERR_PARSE;
	    }
	wsSkipN( reply, 2, &scnl_pos );
    }
    else if ( strlen(flag) == 2 ) {
	tankStarttime = 0.0;
	samprate = 0.0;
	if ( strcmp(flag,"FL") == 0 ) {
	    if ( sscanf( &reply[scnl_pos], "%lf", &tankStarttime) < 1 ) {
		if (WS_CL_DEBUG)
		    logit( "e","wsParseAsciiHeaderReplyL(): error parsing startTime\n" );
		return WS_ERR_PARSE;
	    }
	    wsSkipN( reply, 1, &scnl_pos );
	}
	else if ( strcmp(flag,"FR") == 0 ) {
	    if ( sscanf( &reply[scnl_pos], "%lf", &samprate) < 1 ) {
		if (WS_CL_DEBUG)
		    logit( "e","wsParseAsciiHeaderReplyL(): error parsing samprate\n" );
		return WS_ERR_PARSE;
	    }
	    wsSkipN( reply, 1, &scnl_pos );
	}
    }
    
    getThis->pinno = (short)pinno;
    getThis->actStarttime = tankStarttime;
    getThis->actEndtime = (double) 0.0;
    getThis->samprate = samprate;
    getThis->actLen = (long)(strlen( reply ) - scnl_pos);
    memmove(reply, &reply[scnl_pos], getThis->actLen);
    reply[getThis->actLen] = 0;
  
    if ( strlen( flag ) >= 2 ) {
	    getThis->retFlag = flag[1];
	    return WS_WRN_FLAGGED;
    } else {
	    getThis->retFlag = '\0';
	    return WS_ERR_NONE;
    }
}


/***********************************************************************
 * wsParseAsciiHeaderReply: parse the reply we got from the waveserver *
 * into a TRACE_REQ structure. Handles the header for replies reply to *
 * GETSCN and GETPIN requests.                                         *
 * This function requires the absence of location code.                *
 ***********************************************************************/
static int wsParseAsciiHeaderReply( TRACE_REQ* getThis, char* reply )
{
    /* Arguments:
     *    getThis: pointer to TRACE_REQ structure to be filled with reply info
     *      reply: pointer to reply to be parsed.
     *   Returns: WS_ERR_NONE:  if all went well
     *            WS_ERR_INPUT: if bad input parameters
     *            WS_ERR_PARSE: if we couldn't parse part of the reply
     *            WS_WRN_FLAGGED: server sent us a no-data flag
     */
    int reqid = 0;
    int pinno = 0;
    char sta[TRACE2_STA_LEN];
    char chan[TRACE2_CHAN_LEN];
    char net[TRACE2_NET_LEN];
    char flag[9];
    char datatype[3];
    double tankStarttime = 0.0, samprate = 0.0;
    int scnl_pos = 0;

    if ( !reply ) {
	if (WS_CL_DEBUG) 
	    logit( "e", "wsParseAsciiHeaderReply(): bad input parameters\n");
	return WS_ERR_INPUT;
    }
    
    if ( sscanf( &reply[scnl_pos], "%d %d", &reqid, &pinno ) < 2 ) {
	if (WS_CL_DEBUG) 
	    logit( "e","wsParseAsciiHeaderReply(): error parsing reqid/pinno\n" );
	return WS_ERR_PARSE;
    }
    wsSkipN( reply, 2, &scnl_pos );
    
    if ( sscanf( &reply[scnl_pos], "%s %s %s", sta, chan, net ) < 3 ) {
	if (WS_CL_DEBUG) 
	    logit( "e","wsParseAsciiHeaderReply(): error parsing SCNL\n" );
	return WS_ERR_PARSE;
    }
    wsSkipN( reply, 3, &scnl_pos );
    
    if ( sscanf( &reply[scnl_pos], "%s %s", flag, datatype ) < 2 ) {
	if (WS_CL_DEBUG) 
	    logit( "e","wsParseAsciiHeaderReply(): error parsing flag/datatype\n" );
	return WS_ERR_PARSE;
    }
    wsSkipN( reply, 2, &scnl_pos );
    
    if ( strlen(flag) == 1 || strcmp(flag,"FG") == 0 ) {
	if ( sscanf( &reply[scnl_pos], "%lf %lf", &tankStarttime, &samprate ) 
	     < 2 ) {
	    if (WS_CL_DEBUG)
		logit( "e","wsParseAsciiHeaderReply(): error parsing startT/samprate\n" );
	    return WS_ERR_PARSE;
	}
	wsSkipN( reply, 2, &scnl_pos );
    }
    else if ( strlen(flag) == 2 ) {
	tankStarttime = 0.0;
	samprate = 0.0;
	if ( strcmp(flag,"FL") == 0 ) {
	    if ( sscanf( &reply[scnl_pos], "%lf", &tankStarttime) < 1 ) {
		if (WS_CL_DEBUG)
		    logit( "e","wsParseAsciiHeaderReply(): error parsing startTime\n" );
		return WS_ERR_PARSE;
	    }
	    wsSkipN( reply, 1, &scnl_pos );
	}
	else if ( strcmp(flag,"FR") == 0 ) {
	    if ( sscanf( &reply[scnl_pos], "%lf", &samprate) < 1 ) {
		if (WS_CL_DEBUG)
		    logit( "e","wsParseAsciiHeaderReply(): error parsing samprate\n" );
		return WS_ERR_PARSE;
	    }
	    wsSkipN( reply, 1, &scnl_pos );
	}
    }
    
    getThis->pinno = (short)pinno;
    getThis->actStarttime = tankStarttime;
    getThis->actEndtime = (double) 0.0;
    getThis->samprate = samprate;
    getThis->actLen = (long)(strlen( reply ) - scnl_pos);
    memmove(reply, &reply[scnl_pos], getThis->actLen);
    reply[getThis->actLen] = 0;
  
    if ( strlen( flag ) >= 2 ) {
	getThis->retFlag = flag[1];
	return WS_WRN_FLAGGED;
    } else {
	getThis->retFlag = '\0';
	return WS_ERR_NONE;
    }
}


/**************************************************************************
 *      wsSkipN: moves forward the pointer *posp in buf by moving forward *
 *      cnt words.  Words are delimited by either space or horizontal     *
 *      tabs; newline marks the end of the buffer.                        *
 *  This function is not concerned with location codes.                   *
 **************************************************************************/
static void wsSkipN( char* buf, int cnt, int* posp )
{
    int pos = *posp;

    while ( cnt ) {
	while ( buf[pos] != ' ' && buf[pos] != '\t' ) {
	    if ( !buf[pos] ) {
		goto done;
	    }
	    if ( buf[pos] == '\n' ) {
		++pos;
		goto done;
	    }
	    ++pos;
	}
	--cnt;
	while ( buf[pos] == ' ' || buf[pos] == '\t' ) {
	    ++pos;
	}
    }
 done:
    *posp = pos;
}

int setWsClient_ewDebug(int debug)
{
    /* setWsClient_ewDebug() turns debugging on or off for 
       the ws_clientII routines
    */
    WS_CL_DEBUG=debug;
    return(0);
}
