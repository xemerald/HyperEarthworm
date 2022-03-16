/******************************************************************************
 *
 *	File:			ws2ts.c
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

#include <kom.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <earthworm.h>
#include <swap.h>
#include <ws2ts.h>

/*************************************************************************
 *   ParseConfigForWS (char * configfile, WShandle * waveservers)        *
 *      Parse provided configfile to fill provided WShandle              *
 *      Return 0 if successful                                           *
 *************************************************************************/

static int ParseConfigForWS(char * configfile, 
	WShandle * waveservers, char * program_name, int(*callback)(void)) 
{
    char            *com;
    char            *str;
    int             nfiles;
    int             success;
    int             abort;
    char 			req_cmd[2] = {0,0};


    /* Open the main configuration file 
    **********************************/
    nfiles = k_open( configfile ); 
    if ( nfiles == 0 ) {
        logit( "e", "%s: Fatal Error opening command file <%s>; exiting!\n", 
        	program_name, configfile );
        return -1;
    }

    /* Process all command files
    ***************************/
    abort = 0;
    while(nfiles > 0)   /* While there are command files open */
    {
        while(!abort && k_rd())        /* Read next line from active file  */
        {  
            com = k_str();         /* Get the first token from line */

            /* Ignore blank lines & comments
            *******************************/
            if( !com )           continue;
            if( com[0] == '#' )  continue;

            /* Open a nested configuration file 
            **********************************/
            if( com[0] == '@' ) {
                success = nfiles+1;
                nfiles  = k_open(&com[1]);
                if ( nfiles != success ) {
                    logit( "e", "%s: Error opening command file <%s>; exiting!\n", 
                    	program_name, &com[1] );
                    return -1;
                }
                continue;
            }

            /* Process anything else as a command 
            ************************************/
            /* 0 */
            else if (k_its("wsTimeout")) {  /* timeout interval in
                                            * seconds */
                req_cmd[0] = 1;
                waveservers->wsTimeout = k_int() * 1000;
            }
            /* wave server addresses and port numbers to get trace snippets
            * from ***************************************************************** */
            /* 1 */

            else if (k_its("WaveServer")) {
            	req_cmd[1] = 1;
                if (waveservers->nServer >= MAX_WAVESERVERS) {
                    logit( "e", "%s: Too many <WaveServer> commands in <%s>",
                        program_name, configfile);
                    logit( "e", "; max=%d; exiting!\n", (int) MAX_WAVESERVERS);
                    return -1;
                }
                if ((str = k_str()) != NULL) {
                    if (strlen(str) > MAX_ADRLEN - 1) {
                        logit( "e", "%s: server address length too long, max is %d\n",
                            program_name, MAX_ADRLEN - 1);
                        return 1;
                    }
                    strcpy(waveservers->wsIp[waveservers->nServer], str);
                }
                if ((str = k_str()) != NULL) {
                    if (strlen(str) > MAX_ADRLEN) {
                        logit( "e", "%s: server port number too long, max is %d\n",
                            program_name, MAX_ADRLEN - 1);
                        return 1;
                    }
                    strcpy(waveservers->wsPort[waveservers->nServer], str);
                }
                str = k_str();
                if (str != NULL && str[0] != '#') {
                    if (strlen(str) > NAMELEN - 1) {
                        logit( "e", "%s: server comment too long, max is %d\n",
                            program_name, NAMELEN - 1);
                        return 1;
                    }
                    strcpy(waveservers->wsComment[waveservers->nServer], str);
                }
                waveservers->nServer ++;
            } else if (k_its("WSDebug")) {
                waveservers->WSDebug = 1;   /* optional commands */
            }
            /* Unknown command
            *****************/ 
            else if ( callback != NULL ) {
            	abort = callback();
            }

        }
        nfiles = k_close();
    }
    if ( req_cmd[0] == 0 ) {
    	logit( "e", "%s: required command 'wsTimeout' not provided\n" , program_name );
    	abort = 1;
    }
    if ( req_cmd[0] == 0 ) {
    	logit( "e", "%s: required command 'WaveServer' not provided\n" , program_name );
    	abort = 1;
    }

	return abort;
}

/*************************************************************************
 *   Build_Menu (WShandle * But, char * program_name)                                         *
 *      Builds the menus for waveservers in But                          *
 *      Each waveserver has its own menu so that we can do intelligent   *
 *         searches for data.                                            *
 *      Return 1 if all are successful, 0 if at least some are           *
 *************************************************************************/

static int Build_Menu(WShandle * But, char * program_name)
{
    char            whoami[NAMELEN], server[100], got_one;
    long            i, retry, ret, final_ret;

	if ( But->nServer < 1 )
		return -1;
    final_ret = 0;
    sprintf(whoami, " %s: %s: ", program_name, "Build_Menu");
    setWsClient_ewDebug(0);
    if (But->WSDebug)
    	setWsClient_ewDebug(1);
    But->got_a_menu = 0;
    got_one = 0;

    for (i = 0; i < But->nServer; i++) {
        retry = 0;
        But->inmenu[i] = 0;
        sprintf(server, " %s:%s <%s>", But->wsIp[i], But->wsPort[i], But->wsComment[i]);
        if (But->WSDebug) 
        	logit("et", "%s\n", server);
        Append:
        ret = wsAppendMenu(But->wsIp[i], But->wsPort[i], &But->menu_queue[i], But->wsTimeout);

        if (ret == WS_ERR_NONE) {
            But->inmenu[i] = But->got_a_menu = 1;
            got_one = 1;
        } else {
            switch (ret) {
                case WS_ERR_NO_CONNECTION:
                    if (But->WSDebug)
                        logit("e", "%s Could not get a connection to %s to get menu.\n",
                          whoami, server);
		    final_ret = -1;
                    break;
                case WS_ERR_SOCKET:
                    logit("e", "%s Could not create a socket for %s\n",
                          whoami, server);
		    		final_ret = -1;
                    break;
                case WS_ERR_BROKEN_CONNECTION:
                    logit("e", "%s Connection to %s broke during menu\n",
                          whoami, server);
                    if (retry++ < But->RetryCount)
                        goto Append;
		    		final_ret = -1;
                    break;
                case WS_ERR_TIMEOUT:
                    logit("e", "%s Connection to %s timed out during menu.\n",
                          whoami, server);
                    if (retry++ < But->RetryCount)
                        goto Append;
		    		final_ret = -1;
                    break;
                case WS_ERR_MEMORY:
                    logit("e", "%s: error allocating memory during menu.\n", whoami);
		    		final_ret = -1;
                    break;
                case WS_ERR_INPUT:
                    logit("e", "%s bad/empty inputs to menu\n", whoami);
		    		final_ret = -1;
                    break;
                case WS_ERR_PARSE:
                    logit("e", "%s Parser failed for %s\n", whoami, server);
		    		final_ret = -1;
                    break;
                case WS_ERR_BUFFER_OVERFLOW:
                    logit("e", "%s Buffer overflowed for %s\n", whoami, server);
		    		final_ret = -1;
                    break;
                case WS_ERR_EMPTY_MENU:
                    logit("e", "%s Unexpected empty menu from %s\n", whoami, server);
		    		final_ret = -1;
                    break;
                default:
                    logit("e", "%s Connection to %s returns error: %ld\n", whoami,
                      server, ret);
		    final_ret = -1;
            }
        }
    }
    if (got_one && final_ret != -1) {
 		return 1;
    }
    if (got_one) {
		return 0;
    } 
    return(final_ret);

}


WShandle * init_ws(char * config, char * program_name, int(*callback)(void)) 
{

	WShandle *waveservers = calloc( 1, sizeof( *waveservers ) );
	int ret;
	
	if ( waveservers == NULL ) {
		logit("e", "Error allocating memory for waveserver definitions.\n");
		return NULL;
	}
	
	/* Set some default values */
	waveservers->RetryCount=5;
	waveservers->wsTimeout=30;

	/* get some Waveservers from the config file */
	ret = ParseConfigForWS(config, waveservers, program_name, callback);
	if ( ret == 0 ) {
	
		/* Now initialize menus */
		ret = Build_Menu(waveservers, program_name);
		if ( ret == 0 || ret == 1 ) {
			return waveservers;
		}
	}
	
	/* Something went wrong; clean-up and exit */
	free( waveservers );
	return NULL;
}


/************************************************************************
 *    matchSCNL( char *S, char *C, char *N, char* L, WS_PSCNL pscnl)    *
 *       Return 1 if SCNL matches the menu item, 0 if not.              *
 *       Wildcards `*'allowed                                           *
 ************************************************************************/
static int matchSCNL( char *S, char *C, char *N, char* L, WS_PSCNL pscnl)
{
    int staMatch =0;
    int netMatch =0;
    int chanMatch=0;
    int locMatch = (strlen(L)==0);
    
    if (strcmp( S, "*") == 0)  staMatch =1;
    if (strcmp( C, "*") == 0)  chanMatch=1;
    if (strcmp( N, "*") == 0)  netMatch =1;
    if (strcmp( L, "*") == 0)  locMatch =1;
    if (staMatch+netMatch+chanMatch+locMatch == 4) 
	return(1);

    if ( !staMatch  && strcmp( S,  pscnl->sta  )==0 ) staMatch=1;
    if ( !chanMatch && strcmp( C, pscnl->chan )==0 ) chanMatch=1;
    if ( !netMatch  && strcmp( N,  pscnl->net  )==0 ) netMatch=1;
    if ( !locMatch  && strcmp( L,  pscnl->loc  )==0 ) locMatch=1;

    if (staMatch+netMatch+chanMatch+locMatch == 4) 
	return(1);
    else
	return(0);
}


int getTraceBufsFromWS(WShandle *wsh, char *S, char *C, char *N, char* L, 
	TRACE_REQ * TraceReq) 
{

	/* Build the trace request */
    int ret =  WS_ERR_SCNL_NOT_IN_MENU;
    WS_MENU server; 

    if ((wsh == NULL) || (S == NULL) || (C == NULL) || (N == NULL) 
    	|| (L == NULL) || (TraceReq == NULL)) {
		logit ("e", "Missing argument(s) passed in\n");
		return WS_ERR_INPUT;
    }
    

    if(wsh->WSDebug == 1) 
		logit("t","Entering getTraceBufsFromWS\n");      

    for ( server = wsh->menu_queue[0].head; server != NULL; server = server->next ) {
		WS_PSCNL tank;
	
		if(wsh->WSDebug == 1) 
			logit("t","Searching through Server %s:%s:\n",server->addr,server->port);  


		for ( tank = server->pscnl; tank != NULL; tank = tank->next ) {
			if ( matchSCNL(S,C,N,L,tank) == 1 ) {
				ret = WS_ERR_NONE;
				strcpy( TraceReq->sta, tank->sta  );
				strcpy( TraceReq->chan,tank->chan );
				strcpy( TraceReq->net, tank->net  );
				strcpy( TraceReq->loc, tank->loc );
				if (strlen(tank->loc) > 0) {
					if (strcmp(L, "--")==0 || strcmp(L, "*")==0)
						logit("t", "getTraceBufsFromWS WARNING: specific location code <%s>"
					  		"served by SCN server\n", tank->loc);
				}
				break; 
			}
		}
    }

    /*
    if ( ret == WS_ERR_EMPTY_MENU )   
		logit( "","snippet2trReq(): Empty menu\n" );
	*/

	/* Get the data */
	
	if ( ret == WS_ERR_SCNL_NOT_IN_MENU ) {
		logit ("et", "SCNL (%s %s %s %s) not found in menu list; exiting!\n",
			S, C, N, L );
		return EW_FAILURE;
	}
	
	if (strlen(TraceReq->loc) > 0) 
		ret = wsGetTraceBinL( TraceReq, wsh->menu_queue, wsh->wsTimeout );
	else
		ret = wsGetTraceBin( TraceReq, wsh->menu_queue, wsh->wsTimeout );

	
	if (ret != WS_ERR_NONE ) {
	    logit ("et", "problem retrieving %s %s %s %s ", TraceReq->sta, 
		   TraceReq->chan, TraceReq->net, TraceReq->loc); 
	    
	    switch( ret ) {
	    case WS_WRN_FLAGGED:
			logit ("e", "server has no data for period requested; exiting!\n");
			return EW_FAILURE;
			break;
		/* following errors will cause the socket to be closed - exit */
	    case WS_ERR_EMPTY_MENU:
			logit ("e", "no menu list found; exiting!.\n");
			return EW_FAILURE;
			break;
	    case WS_ERR_BUFFER_OVERFLOW:
			logit ("e", "trace buffer overflow; exiting!\n");
			return EW_FAILURE;
			break;
	    case WS_ERR_PARSE:
			logit ("e", "error parsing server's reply; exiting!\n");
			return EW_FAILURE;
			break;
	    case WS_ERR_TIMEOUT:
			logit ("e", "timeout talking to wave server; exiting!\n");
			return EW_FAILURE;
			break;
	    case WS_ERR_BROKEN_CONNECTION:
			logit ("e", "connection to wave server broken; exiting with %d!\n", EW_FAILURE);
			return EW_FAILURE;
			break;
	    case WS_ERR_SOCKET:
			logit ("e", "error changing socket options; exitting!\n");
			return EW_FAILURE;
			break;
	    case WS_ERR_NO_CONNECTION:
			logit ("e", "socket to wave server already closed; exitting!\n");
			return EW_FAILURE;
			break;
	    default:
			logit ("e", "unknown error code %d; exiting with %d!\n", ret, EW_FAILURE);
			return EW_FAILURE;
	    }
	}
	if (wsh->WSDebug == 1)
	    logit ("t",
		   "trace %s %s %s %s: went ok first time. Got %ld bytes\n", 
		   		TraceReq->sta, TraceReq->chan, TraceReq->net, TraceReq->loc, 
		   		TraceReq->actLen); 
	
	return ret;
}

EW_TIME_SERIES * convertTraceBufReq2WEW ( TRACE_REQ * tr,  double gapFillValue ) 
{
	TRACE2_HEADER *wf;
	char        *msg_p;        /* pointer into tracebuf data */
	short       *s_data;
	int32_t     *l_data;
	float       *f_data;
	double       fill = gapFillValue;
	int          j;
	int          gap_count = 0;
	int32_t      nsamp, nfill, AH_nsamp;
	int32_t      nfill_max = 0;
	int32_t      nsamp_this_scn = 0;
	double       starttime, endtime; /* times for current scn         */
	double       samprate;
	double       AH_starttime;
	char         datatype; /* 's' for short, 'l' for long */
	double		 gapTime = 0;
	double       GapThresh;
	double       TimeOfOneSample;
  	double 		 *mydata;
  	EW_TIME_SERIES *my_ewt;
  	int          eltSize = 0;
  	long         dataLen;
  	double		 gapSize;
	  
	if ( (msg_p = tr->pBuf) == NULL)   /* pointer to first message */
	{
		logit("e", "convertTraceBufReq2WEW: message buffer is NULL\n");
		return NULL;
	}
	
	wf = (TRACE2_HEADER *) msg_p;
	  
	
	if (WaveMsg2MakeLocal(wf) < 0)
	{
		logit("e", "convertTraceBufReq2WEW: unknown trace data type: %s\n",
			  wf->datatype);
		return NULL;
	}
	
	nsamp = wf->nsamp;
	starttime = wf->starttime;
	endtime = wf->endtime;
	samprate = wf->samprate;

  	/* these are the same for now, but in future GapThresh may be settable */
	TimeOfOneSample = GapThresh = 1.0 / samprate; 

	if (samprate < 0.01)
	{
		logit("et", "convertTraceBufReq2WEW: unreasonable samplerate (%f) for <%s.%s.%s.%s>\n",
			  samprate, wf->sta, wf->chan, wf->net, wf->loc);
		return NULL;
	}
	AH_starttime = starttime;
	AH_nsamp = 0;
	datatype = 'n';
	if (wf->datatype[0] == 's' || wf->datatype[0] == 'i')
	{
		if (wf->datatype[1] == '2') {
			datatype = 's';
			eltSize = sizeof(short);
		} else if (wf->datatype[1] == '4') {
			datatype = 'l';
			eltSize = sizeof(int32_t);
		}
	}
	else if (wf->datatype[0] == 't' || wf->datatype[0] == 'f')
	{
		if (wf->datatype[1] == '4') {
			datatype = 'f';
			eltSize = sizeof(float);
		}
	}
	if (datatype == 'n')
	{
		logit("et", "convertTraceBufReq2WEW: unsupported datatype: %c%c\n", 
			wf->datatype[0], wf->datatype[1]);
		return NULL;
	}

 	dataLen = (long)ceil((tr->actEndtime - tr->actStarttime + TimeOfOneSample) * samprate);
 	mydata = (double*)malloc( dataLen * sizeof(double) );
  	if ( mydata == NULL ) {
		logit( "e", "convertTraceBufReq2WEW:not enough space for <%s.%s.%s.%s>.\n",
			 wf->sta, wf->chan, wf->net, wf->loc);
		return NULL;
  	}

	/* Reset pointer to start of messages */
	msg_p = tr->pBuf;	
	wf = (TRACE2_HEADER *) msg_p;
	
	/* loop through all the messages for this s-c-n */
	while (1) 
	{
		/* advance message pointer to the data */
		msg_p += sizeof(TRACE_HEADER);
	
		/* check for sufficient memory in output buffer -- SHOULD NEVER HAPPEN!
		this_size = (nsamp_this_scn + nsamp ) * sizeof(float);
		if ( OutBufferLen < (this_size + (long)sizeof(ahhed) ))
		{
		  	logit( "e", "out of space for <%s.%s.%s.%s>; saving short trace.\n",
				 wf->sta, wf->chan, wf->net, wf->loc);
		  	break;
		}*/
	  
		switch( datatype )
		{
		case 's':
		  	s_data = (short *)msg_p;
		  	for ( j = 0; j < nsamp ; j++, nsamp_this_scn++ ) {
				mydata[nsamp_this_scn] = (float) s_data[j];
			}
		  	break;
		case 'l':
		  	l_data = (int32_t *)msg_p;
		  	for ( j = 0; j < nsamp; j++, nsamp_this_scn++ ) {
				mydata[nsamp_this_scn] = (float) l_data[j];
			}
		  	break;
		case 'f':
		  	f_data = (float *)msg_p;
		  	for ( j = 0; j < nsamp; j++, nsamp_this_scn++ ) {
				mydata[nsamp_this_scn] = f_data[j];
			}
		  	break;
		}
		msg_p += eltSize * nsamp;
		AH_nsamp += nsamp;
	  
		/* End-check based on length of snippet buffer */
		if ((size_t) msg_p >= ((size_t) tr->actLen + (size_t) tr->pBuf))
		{
		  	/*if (Debug == 1)
				logit ("", "Setting done for <%s.%s.%s.%s>\n", wf->sta, wf->chan, 
				   wf->net, wf->loc); */
		  	break; /* Break out of the while(1) loop 'cuz we're done */
		}
	  
		/* msg_p has been advanced to the next TRACE_BUF; localize bytes *
		 * and check for gaps.                                           */
		wf = (TRACE2_HEADER *) msg_p;
		if (WaveMsg2MakeLocal(wf) < 0)
		{
		  	logit("e", "convertTraceBufReq2WEW: unknown trace data type: %s\n",
				wf->datatype);
			free( mydata );
		  	return( NULL );
		}
		nsamp = wf->nsamp;
		starttime = wf->starttime; 
		/* starttime is set for new packet; endtime is still set for old packet */
		gapSize = (starttime - (endtime + TimeOfOneSample));
		if ( gapSize > GapThresh ) 
		{
		  	/* there's a gap, so fill it */
		  	gapTime += gapSize;
		  	logit("e", "gap in %s.%s.%s.%s: last packet ended @ %lf, this one starts @ %lf (threshold = %lf) gap = %lf\n", 
				wf->sta, wf->chan, wf->net, wf->loc, endtime, starttime, GapThresh, gapSize);
		  	nfill = (int32_t) (samprate * (gapSize) - 1);
		  	/*
		  	if ( (nsamp_this_scn + nfill) * (long)sizeof(float) > OutBufferLen ) 
		  	{
				logit("e", "bogus gap (%d); skipping\n", nfill);
				return(EW_FAILURE);
		  	}
		  	*/
		  	/* do the filling */
	  		for ( j = 0; j < nfill; j++, nsamp_this_scn ++ ) 
				mydata[nsamp_this_scn] = fill;
		  	/* keep track of how many gaps and the largest one */
		  	gap_count++;
		  	if (nfill_max < nfill) 
				nfill_max = nfill;
		}
		/* Advance endtime to the new packet;        *
		 * process this packet in the next iteration */
		endtime = wf->endtime;
  }
  
  my_ewt = calloc( 1, sizeof( *my_ewt ) );
  my_ewt->dataType = EWTS_TYPE_SIMPLE;
  my_ewt->data = mydata;
  my_ewt->gaps = gap_count;
  my_ewt->gapTime = gapTime;
  my_ewt->gapValue = fill;
  my_ewt->trh2x = *((TRACE2X_HEADER *)( tr->pBuf ));
  my_ewt->trh2x.starttime = AH_starttime;
  my_ewt->trh2x.endtime = endtime;
  my_ewt->trh2x.nsamp = AH_nsamp;
  my_ewt->dataCount = dataLen;
  return my_ewt;
}


void ws2ts_purge( WShandle *wsh, TRACE_REQ *trh, EW_TIME_SERIES *ewtsh ) 
{
	if ( wsh != NULL ) 
		free( wsh );
	if ( trh != NULL ) {
		if ( trh->pBuf != NULL )
			free( trh->pBuf );
		free( trh );
	}
	if ( ewtsh != NULL ) {
		if ( ewtsh->data != NULL )
			free( ewtsh->data );
		free( ewtsh );
	}
}

int unary_calc_EWTS( EW_TIME_SERIES arg, int mode,
		     void(*op)(const double, const int, void* ), void *oparg )
{
	int i;
	if ( arg.dataType == EWTS_TYPE_SIMPLE )
		for ( i=0; i<arg.dataCount; i++ )
			op(arg.data[i], 0, oparg);
	else
		for ( i=0; i<arg.dataCount; i++ ) {
			if ( mode & 1 )
				op(arg.data[i*2], 0, oparg);
			if ( mode & 2 )
				op(arg.data[i*2+1], 1, oparg);
		}
	return 0;
}

int unary_modify_EWTS( EW_TIME_SERIES arg, int mode,
		       double(*op)(const double, const int, const double), 
		       const double oparg )
{
	int i;
	if ( arg.dataType == EWTS_TYPE_SIMPLE )
		for ( i=0; i<arg.dataCount; i++ )
			arg.data[i] = op(arg.data[i], 0, oparg);
	else
		for ( i=0; i<arg.dataCount; i++ ) {
			if ( mode & 1 )
				arg.data[i*2] = op(arg.data[i*2], 0, oparg);
			if ( mode & 2 )
				arg.data[i*2+1] = op(arg.data[i*2+1], 1, oparg);
		}
	return 0;
}

int unary_fill_EWTS( EW_TIME_SERIES input, EW_TIME_SERIES output, int mode,
					double(*op)(const double, const int, const double), 
					const double oparg ) 
{
	int i;
	if ( input.trh2x.nsamp != output.trh2x.nsamp ) {
		logit("e", "input & output timeseries' lengths differ\n");
		return 1;
	}
	if ( input.dataType != output.dataType ) {
		logit("e", "input & output timeseries' dataTypes differ\n");
		return 1;
	}
	if ( input.dataType == EWTS_TYPE_SIMPLE )
		for ( i=0; i<input.dataCount; i++ )
			output.data[i] = op(input.data[i], 0, oparg);
	else
		for ( i=0; i<input.dataCount; i++ ) {
			if ( mode & 1 ) 
				output.data[i*2] = op(input.data[i*2], 0, oparg);
			if ( mode & 2 ) 
				output.data[i*2+1] = op(input.data[i*2+1], 1, oparg);
		}
	return 0;
}

int binary_modify_EWTS( EW_TIME_SERIES arg1, EW_TIME_SERIES arg2, int mode,
					double(*op)(const double, const int, const double) ) 
{
	int i;
	if ( arg1.trh2x.nsamp != arg2.trh2x.nsamp )
		return 1;
	if ( arg1.trh2x.nsamp != arg2.trh2x.nsamp ) {
		logit("e", "input timeseries' lengths differ\n");
		return 1;
	}
	if ( arg1.dataType != arg2.dataType ) {
		logit("e", "input timeseries' dataTypes differ\n");
		return 1;
	}
	if ( arg1.dataType == EWTS_TYPE_SIMPLE )
		for ( i=0; i<arg1.dataCount; i++ )
			arg1.data[i] = op(arg1.data[i], i%2, arg2.data[i]);
	else
		for ( i=0; i<arg1.dataCount; i++ ) {
			if ( mode & 1 )
				arg1.data[i*2] = op(arg1.data[i*2], 0, arg2.data[i*2]);
			if ( mode & 2 )
				arg1.data[i*2+1] = op(arg1.data[i*2+1], 1, arg2.data[i*2+1]);
		}
	return 0;
}

int binary_fill_EWTS( EW_TIME_SERIES arg1, EW_TIME_SERIES arg2, EW_TIME_SERIES output, 
					int mode, 
					double(*op)(const double, const int, const double) ) 
{
	int i;
	if ( arg1.dataCount != arg2.dataCount ) {
		logit("e", "input timeseries' lengths differ\n");
		return 1;
	}
	if ( arg1.dataCount != output.dataCount ) {
		logit("e", "input & output timeseries' lengths differ\n");
		return 1;
	}
	if ( arg1.dataType != arg2.dataType ) {
		logit("e", "input timeseries' dataTypes differ\n");
		return 1;
	}
	if ( arg1.dataType != output.dataType ) {
		logit("e", "input & output timeseries' dataTypes differ\n");
		return 1;
	}
	if ( arg1.dataType == EWTS_TYPE_SIMPLE )
		for ( i=0; i<arg1.dataCount; i++ )
			output.data[i] = op(arg1.data[i], 0, arg2.data[i]);
	else 
		for ( i=0; i<arg1.dataCount; i++ ) {
			if ( mode & 1 )
				output.data[i*2] = op(arg1.data[i*2], 0, arg2.data[i*2]);
			if ( mode & 2 )
				output.data[i*2+1] = op(arg1.data[i*2+1], 1, arg2.data[i*2+1]);
		}
	return 0;
}

void taper_EWTS( EW_TIME_SERIES ewts, int taper_type, double fraction, int mode ) 
{
	/* taper length in samples */
	int nsamp = ewts.dataCount;
	double ffl= ((double) ewts.dataCount)*fraction;
	double f, x;
	double *fdata = ewts.data;
#ifdef M_PI
	double PI = M_PI;
#else
	double PI = 3.1415;
#endif
	int i;

	switch (taper_type) {
	case BARTLETT:
		if ( ewts.dataType == EWTS_TYPE_SIMPLE ) 
			for (i= 0; i < (int) ffl; i++) {
				f= 1.0*((float) i)/ffl;
				fdata[i]*= f;
				fdata[nsamp-1-i]*= f;
			}
		else
			for (i= 0; i < (int) ffl; i++) {
				f= 1.0*((float) i)/ffl;
				if ( mode & 1 ) {				
					fdata[i*2]*= f;
					fdata[(nsamp-1-i)*2]*= f;
				}
				if ( mode & 2 ) {
					fdata[i*2+1]*= f;
					fdata[(nsamp-1-i)*2+1]*= f;
				}
			}			
		break;
	case PARZAN:
		if ( ewts.dataType == EWTS_TYPE_SIMPLE ) 
			for (i= 0; i < (int) ffl; i++) {
				x= 1.0*((float) i)/(ffl+1.);
				if (x-0.5 <= 0)
					f= 6.0*x*x-6.0*x*x*x;
				else
					f= 1.0-2.0*pow((double) (1.0-x), (double) 3.0);
				fdata[i]*= f;
				fdata[nsamp-1-i]*= f;
			}
		else 
			for (i= 0; i < (int) ffl; i++) {
				x= 1.0*((float) i)/(ffl+1.);
				if (x-0.5 <= 0)
					f= 6.0*x*x-6.0*x*x*x;
				else
					f= 1.0-2.0*pow((double) (1.0-x), (double) 3.0);
				if ( mode & 1 ) {				
					fdata[i*2]*= f;
					fdata[(nsamp-1-i)*2]*= f;
				}
				if ( mode & 2 ) {
					fdata[i*2+1]*= f;
					fdata[(nsamp-1-i)*2+1]*= f;
				}
			}
		break;
	case HANNING:
		if ( ewts.dataType == EWTS_TYPE_SIMPLE ) 
			for (i= 0; i < (int) ffl; i++) {
				f= 0.5-0.5*cos(PI*((float) i)/(ffl+1.));
				fdata[i]*= f;
				fdata[nsamp-1-i]*= f;
			}
		else
			for (i= 0; i < (int) ffl; i++) {
				f= 0.5-0.5*cos(PI*((float) i)/(ffl+1.));
				if ( mode & 1 ) {				
					fdata[i*2]*= f;
					fdata[(nsamp-1-i)*2]*= f;
				}
				if ( mode & 2 ) {
					fdata[i*2+1]*= f;
					fdata[(nsamp-1-i)*2+1]*= f;
				}
			}
		break;
	case BMHARRIS:
		if ( ewts.dataType == EWTS_TYPE_SIMPLE ) 
			for (i= 0; i < (int) ffl; i++) {
				f= 0.35825-
				0.48829*cos(PI*((float) i)/(ffl+1.))+
				0.14128*cos(2.0*PI*((float) i)/(ffl+1.))-
				0.01168*cos(3.0*PI*((float) i)/(ffl+1.));
				if ( mode & 1 ) {				
					fdata[i*2]*= f;
					fdata[(nsamp-1-i)*2]*= f;
				}
				if ( mode & 2 ) {
					fdata[i*2+1]*= f;
					fdata[(nsamp-1-i)*2+1]*= f;
				}
			}
		else
			for (i= 0; i < (int) ffl; i++) {
				f= 0.35825-
				0.48829*cos(PI*((float) i)/(ffl+1.))+
				0.14128*cos(2.0*PI*((float) i)/(ffl+1.))-
				0.01168*cos(3.0*PI*((float) i)/(ffl+1.));
				if ( mode & 1 ) {				
					fdata[i*2]*= f;
					fdata[(nsamp-1-i)*2]*= f;
				}
				if ( mode & 2 ) {
					fdata[i*2+1]*= f;
					fdata[(nsamp-1-i)*2+1]*= f;
				}
			}
		break;
	}
}

double diff( const double a, int odd, const double b )
{
	return a - b;
}

int subtract_from_EWTS( EW_TIME_SERIES arg1, EW_TIME_SERIES arg2, int mode ) 
{
	return binary_modify_EWTS( arg1, arg2, mode, diff );
}

void addto( const double a, int odd , void * b )
{
	double * bptr = (double*)b;
	bptr[odd] += a;
}

int demean_EWTS( EW_TIME_SERIES arg ) 
{
	double sum[2] = {0,0};
	unary_calc_EWTS( arg, EWTS_MODE_BOTH, addto, sum );
	unary_modify_EWTS( arg, EWTS_MODE_FIRST, diff, sum[0]/arg.dataCount );
	if ( arg.dataType != EWTS_TYPE_SIMPLE )
		unary_modify_EWTS( arg, EWTS_MODE_SECOND, diff, sum[1]/arg.dataCount );
	return 0;
}

static double identity( int i, double t, void *arg ) 
{
	return t;
}

void print_EWTS(EW_TIME_SERIES *ts, char *col1header, 
	double(*op)(int i, double t, void *arg),
	void *entryMapArg,
	FILE *fp) 
{
  	int i;
  	double step = 1.0/ts->trh2x.samprate;
  	double t = ts->trh2x.starttime;
  	if ( col1header == NULL )
  		col1header = "Time";
  	if ( op == NULL )
  		op = identity;
  	if ( fp == NULL )
  		fp = stdout;
	if ( ts->dataType == EWTS_TYPE_SIMPLE ) {
		fprintf( fp, "%15s\t%15s\n", col1header, "Value" );
	    for ( i=0; i<ts->dataCount; i++, t+=step )
    		fprintf( fp, "%15.3lf\t%15.3lf\n", op(i,t, entryMapArg), ts->data[i] );
    } else {
    	if ( ts->dataType == EWTS_TYPE_COMPLEX )
			fprintf( fp, "%15s\t%15s\t%15s\n", col1header, "Real", "Imaginary" );
    	else
			fprintf( fp, "%15s\t%15s\t%15s\n", col1header, "Amplitude", "Phase" );
	    for ( i=0; i<ts->dataCount; i++, t+=step )
	    	fprintf( fp, "%15.3lf\t%15.3lf\t%15.3lf\n", op(i,t, entryMapArg), ts->data[i*2], ts->data[i*2+1] );
	}
}
