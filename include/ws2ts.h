/******************************************************************************
 *
 *	File:			ws2ts.h
 *
 *	Function:		Routines for getting data from waveserver(s) and
 *                  converting it to timeseries data
 *
 *	Author(s):		Scott Hunter, ISTI
 *
 *	Source:			Started anew.
 *
 *	Notes:			
 *
 *	Change History:
 *			4/26/11	Started source
 *	
 *****************************************************************************/

#include <ew_timeseries.h>

#define MAX_WAVESERVERS   20  /* Maximum number of Waveservers            */
#define MAX_ADRLEN        20  /* Size of waveserver address arrays        */
#define NAMELEN           50  /* Length of some comment strings           */


typedef struct {
    int     WSDebug;         /* debugging on for wavserver work           */
    long    wsTimeout;       /* seconds to wait for reply from ws         */
    int     nServer;         /* number of wave servers we know about      */
    long    RetryCount;      /* Retry count for waveserver errors.        */
    int     got_a_menu;
    WS_MENU_QUEUE_REC menu_queue[MAX_WAVESERVERS];

    int     inmenu[MAX_WAVESERVERS];
                             /* List of waveServers, from config. file:   */
    char    wsIp[MAX_WAVESERVERS][MAX_ADRLEN];
    char    wsPort[MAX_WAVESERVERS][MAX_ADRLEN];
    char    wsComment[MAX_WAVESERVERS][NAMELEN];
} WShandle;


/*************************************************************************
 *   init_ws()                                                           *
 *      Reads config file that should contain at least one Waveserver    *
 *          directive (IP and port and optional comment)                 *
 *      callback called for each command not understood                  *
 *      Build & return pointer to WShandle of waveserver info            *
 *************************************************************************/
WShandle * init_ws(char * config, char * program_name, int(*callback)(void));

/*************************************************************************
 *   getTraceBufsFromWS()                                                *
 *      Fill TraceReq w/ tracebuf(s) from servers in wsh for SCNL        *
 *      Return WS_ERR_NONE on success                                    *
 *************************************************************************/
int getTraceBufsFromWS(WShandle *wsh, char *S, char *C, char *N, char* L, 
	TRACE_REQ * TraceReq);

/*************************************************************************
 *   convertTraceBufReq2WEW()                                            *
 *      Return a timeseries computed from  from tracebufs                *
 *************************************************************************/
EW_TIME_SERIES * convertTraceBufReq2WEW ( TRACE_REQ * tr, double gapFillValue );

/*************************************************************************
 *   ws2ts_purge()                                                       *
 *      Free datastructures allocated by this library                    *
 *      Any of the arguments that are NULL are ignored                   *
 *************************************************************************/
void ws2ts_purge( WShandle *wsh, TRACE_REQ *trh, EW_TIME_SERIES *ewtsh );

