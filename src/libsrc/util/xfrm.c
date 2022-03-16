/*
 * xfrm.c:  
 *
 * 
 *  Initial version:
 *      
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#ifdef _WINNT
#include <windows.h>
#define mutex_t HANDLE
#else
#ifdef _SOLARIS
#ifndef _LINUX
#include <synch.h>      /* mutex's                                      */
#endif
#endif
#endif

#include "earthworm.h"
#include "kom.h"
#include "swap.h"
#include "trheadconv.h" /* trheadconv()                                 */
#include "xfrm.h"

#define SCNL_INC     100      /* how many more are allocated each     */
                                /*   time we run out                    */
#define NUM_BASE_CMDS 12
#define CMD_ID_GETSCNL       8
#define CMD_ID_GETWAVESFROM 11

static char cnull = '\0';
static int  maxSta = 0;     /* number of scnls allocated   */
static char **BaseCmd=NULL; /* array of command definitions */
static char **ModCmds;      /* array of module-specific command defs */
static int  CmdCount;       /* number of command definitions */
static void **ParamTarget=NULL; /* array of addrs for value of matching commands */
static void **ModParamTargets; /* ParamTargets for ModCmds */
/* length of message buffer */
static long MsgBufLen = (MAX_TRACEBUF_SIZ + sizeof (TP_PREFIX));


static void InitializeParameters( XFRMWORLD* pXfrm )
{
  int i;

  /*    Initialize members of the XFRMWORLD structure                    */
  pXfrm->completionFcn          = NULL;

  /*    Initialize members of the xfrmEWH structure                      */
  pXfrm->xfrmEWH->myInstId = 0;
  pXfrm->xfrmEWH->myModId  = 0;
  pXfrm->xfrmEWH->typeError     = 0;
  pXfrm->xfrmEWH->typeHeartbeat = 0;
  pXfrm->xfrmEWH->ringInKey     = 0l;
  pXfrm->xfrmEWH->ringOutKey    = 0l;
  pXfrm->xfrmEWH->typeTrace     = 0;
  pXfrm->xfrmEWH->typeTrace2    = 0;

  /*    Initialize members of the xfrmParam structure                    */
  for( i=0; i<MAX_LOGO; i++ ) {
     pXfrm->xfrmParam->readInstName[i][0] = '\0';
     pXfrm->xfrmParam->readModName[i][0] = '\0';
     pXfrm->xfrmParam->readTypeName[i][0] = '\0';
  }
  pXfrm->xfrmParam->ringIn[0]      = '\0';
  pXfrm->xfrmParam->ringOut[0]     = '\0';
  pXfrm->xfrmParam->nlogo          = 0;
  pXfrm->xfrmParam->heartbeatInt   = 15;
  pXfrm->xfrmParam->debug          = 0;
  pXfrm->xfrmParam->logSwitch      = 1;
  pXfrm->xfrmParam->cleanStart     = 1;
  pXfrm->xfrmParam->testMode       = 0;
  pXfrm->xfrmParam->maxGap         = 1.5;
  pXfrm->xfrmParam->getAllSCNLs    = 0;

  InitializeXfrmParameters();
  
  return;
}


static int IsWild( char *str )
{
   if( strcmp( str, "*" ) == 0 ) return 1;
   return 0;
}

/*
 * Determines if the station name is valid.
 * In:  pxfrm pointer to the Xfrm World structure
 *      outputFlag true if output, false if input
 *      str the station name
 * Returns non-zero if valid, 0 if invalid
 */
static int IsValidSta(XFRMWORLD* pXfrm, int outputFlag, char *str) {
    if( !str  ||  strlen(str) >= TRACE2_STA_LEN  ||  IsWild(str) ) {
        if( !str ) str = &cnull;
        logit ("e", "%s: Invalid %s station code: %s "
                    "in GetSCNL cmd; exiting!\n",
                    (outputFlag ? "output" : "input"),
                    pXfrm->mod_name, str);
        return 0;
    }
    return 1;
}

/*
 * Determines if the channel name is valid.
 * In:  pxfrm pointer to the Xfrm World structure
 *      outputFlag true if output, false if input
 *      str the channel name
 * Returns non-zero if valid, 0 if invalid
 */
static int IsValidChan(XFRMWORLD* pXfrm, int outputFlag, char *str) {
    if( !str  ||  strlen(str) >= TRACE2_CHAN_LEN  ||  IsWild(str)) {
        if( !str ) str = &cnull;
        logit ("e", "%s: Invalid %s channel code: %s "
                    "in GetSCNL cmd; exiting!\n",
        			(outputFlag ? "output" : "input"),
        			pXfrm->mod_name, str);
        return 0;
    }
    return 1;
}

/*
 * Determines if the network name is valid.
 * In:  pxfrm pointer to the Xfrm World structure
 *      outputFlag true if output, false if input
 *      str the network name
 * Returns non-zero if valid, 0 if invalid
 */
static int IsValidNet(XFRMWORLD* pXfrm, int outputFlag, char *str) {
    if( !str  ||  strlen(str) >= TRACE2_NET_LEN  ||  IsWild(str)) {
        if( !str ) str = &cnull;
        logit ("e", "%s: Invalid %s network code: %s "
                    "in GetSCNL cmd; exiting!\n",
                    (outputFlag ? "output" : "input"),
                    pXfrm->mod_name, str);
        return 0;
    }
    return 1;
}

/*
 * Determines if the location name is valid.
 * In:  pxfrm pointer to the Xfrm World structure
 *      outputFlag true if output, false if input
 *      str the location name
 * Returns non-zero if valid, 0 if invalid
 */
static int IsValidLoc(XFRMWORLD* pXfrm, int outputFlag, char *str) {
    if( !str  ||  strlen(str) >= TRACE2_LOC_LEN  ||  IsWild(str)) {
        if( !str ) str = &cnull;
        logit ("e", "%s: Invalid %s location code: %s "
                    "in GetSCNL cmd; exiting!\n",
                    (outputFlag ? "output" : "input"),
                    pXfrm->mod_name, str);
        return 0;
    }
    return 1;
}

/*
 * Set the output SCNL
 * In:  newSCNL the new SCNL
 *      outSCNL the output SCNL or NULL if none
 */
static void SetOutSCNL(XFRMSCNL *newSCNL, SCNL *outSCNL) {

  if ( outSCNL == NULL )
    return;

  strncpy( newSCNL->outSta,  outSCNL->sta,  TRACE2_STA_LEN );
  newSCNL->outSta[TRACE2_STA_LEN - 1]   = '\0';
  strncpy( newSCNL->outChan, outSCNL->chan, TRACE2_CHAN_LEN );
  newSCNL->outChan[TRACE2_CHAN_LEN - 1] = '\0';
  strncpy( newSCNL->outNet,  outSCNL->net,  TRACE2_NET_LEN );
  newSCNL->outNet[TRACE2_NET_LEN - 1]   = '\0';
  strncpy( newSCNL->outLoc,  outSCNL->loc,  TRACE2_LOC_LEN );
  newSCNL->outLoc[TRACE2_LOC_LEN - 1]   = '\0';

}

/*
 * Allocate the message buffer.
 * Returns the message buffer
 */
static char* AllocateMsgBuf() {
	return (char *) malloc ((size_t) MsgBufLen);
}

/*
 * Get the XFRMSCNL for the station
 * In:  pxfrm pointer to the Xfrm World structure
 *      jSta the station index
 * Returns the XFRMSCNL
 */
static XFRMSCNL* GetXFRMSCNLForSta(XFRMWORLD* pXfrm, int jSta) {
	return (XFRMSCNL *)(pXfrm->scnls + jSta*(pXfrm->scnlRecSize));
}

/*
 * Get the input message buffer for the station
 * In:  pxfrm pointer to the Xfrm World structure
 *      jSta the station index
 * Returns the input message buffer
 */
static char* GetInMsgBufForSta(XFRMWORLD* pXfrm, int jSta) {
	return GetXFRMSCNLForSta(pXfrm, jSta)->inMsgBuf;
}

/*
 * Get the input buffer for the message buffer.
 * In:  msgBuf the message buffer
 * Returns the input buffer
 */
TracePacket* GetInBufForMsgBuf(char* msgBuf) {
	return (TracePacket *)(msgBuf + sizeof(TP_PREFIX));
}

/*
 * Get the input buffer for the station.
 * In:  pxfrm pointer to the Xfrm World structure
 *      jSta the station index
 * Returns the input buffer
 */
TracePacket* GetInBufForSta(XFRMWORLD* pXfrm, int jSta) {
	char* inMsgBuf = GetInMsgBufForSta(pXfrm, jSta);
	return (TracePacket *)(inMsgBuf + sizeof(TP_PREFIX));
}

/*
 * Get the wave bytes for the buffer.
 * In:  buf the buffer
 * Returns the wave bytes.
 */
char* GetWave(TracePacket* buf) {
	return buf->msg + WAVE_BYTES_OFFSET;
}

/*
 * Get the data size.
 * In:  buf the buffer
 * Return the data size.
 */
int GetDataSize(TracePacket* buf) {
	return buf->trh2.datatype[1]-'0';
}

/*
 * Compare the SCNLs.
 * In:  scnl1 the first scnl
 *      scnl2 the second scnl
 * Return 0 if the SCNLs match, non-zero otherwise.
 */
int BaseCompareSCNL( XFRMSCNL *scnl1, XFRMSCNL *scnl2 ) {

  TracePacket *buf1, *buf2;

  /* Both SCNLs must exist */
  if ( ( scnl1 == NULL ) || ( scnl2 ==NULL ) )
    return 1;

  /* Both must contain valid tracebufs; see BaseFilterXfrm() */
  /*   status = -1, no data yet                              */
  /*   status =  1, data is bad                              */
  /*   status =  0, data is good                             */
  if ( ( scnl1->status != 0 ) || ( scnl2->status != 0 ) )
    return 2;

  /* Both must have the same end time */
  /* inEndtime = 0 is used in many places to mean no data */
  if ( ( scnl1->inEndtime == 0.0 ) || ( scnl2->inEndtime == 0.0 ) ||
       ( scnl1->inEndtime != scnl2->inEndtime ) )
    return 3;

  /* Both must have the same sample rate */
  if ( scnl1->samprate != scnl2->samprate )
    return 4;

  /* Both must have the same data size (they have been converted to native type) */
  if ( scnl1->datatype[1] != scnl2->datatype[1] )
    return 5;

  /* Both must have the same number of samples */
  buf1 = GetInBufForMsgBuf( scnl1->inMsgBuf );
  buf2 = GetInBufForMsgBuf( scnl2->inMsgBuf );
  if ( buf1->trh2.nsamp != buf2->trh2.nsamp )
    return 6;

  return 0;

}

/*
 * Compare the SCNL to the SCNL in the XFRMSCNL.
 * In:  scnl the SCNL
 *      xfrmSCNL the XFRMSCNL
 * Returns 0 if the SCNL matches, non-zero otherwise
 */
static int CmpSCNL(SCNL *scnl, XFRMSCNL* xfrmSCNL) {
	if (strcmp(xfrmSCNL->inSta, scnl->sta) != 0) {
		return 1;
	}
	if (strcmp(xfrmSCNL->inChan, scnl->chan) != 0) {
		return 2;
	}
	if (strcmp(xfrmSCNL->inNet, scnl->net) != 0) {
		return 3;
	}
	if (strcmp(xfrmSCNL->inLoc, scnl->loc) != 0) {
		return 4;
	}
	return 0;
}

/*
 * Get the XFRMSCNL for the specified SCNL
 * In:  pxfrm pointer to the Xfrm World structure
 *      scnl the SCNL
 * Out: jSta the station index
 * Returns the XFRMSCNL or NULL if none
 */
static XFRMSCNL* GetXFRMSCNL(XFRMWORLD* pXfrm, SCNL *scnl, int *jSta) {
	XFRMSCNL* xfrmSCNL = NULL;

	for (*jSta = 0; *jSta < pXfrm->nSCNL; (*jSta)++) {
		xfrmSCNL = GetXFRMSCNLForSta(pXfrm, *jSta);
		if (CmpSCNL(scnl, xfrmSCNL) == 0) {
			return xfrmSCNL;
		}
	}
	return NULL;
}

/*
 * Add the input and output SCNL
 * In:  pxfrm pointer to the Xfrm World structure
 *      inSCNL the input SCNL
 *      outSCNL the output SCNL or NULL if none
 * Out: jSta the station index
 * Returns 0 if add was successful, non-zero if error
 */
int AddSCNL( XFRMWORLD* pXfrm, SCNL *inSCNL, SCNL *outSCNL, int *jSta ) {

  char *scnls;
  int nSCNL;
  XFRMSCNL *newSCNL;

  /* Determine if NSCL already exists */
  newSCNL = GetXFRMSCNL( pXfrm, inSCNL, jSta );
  if ( newSCNL != NULL ) {
    SetOutSCNL( newSCNL, outSCNL );
    if ( pXfrm->xfrmParam->debug )
      logit( "", "%s: sta[%d] outSCNL=<%s.%s.%s.%s>\n",
                 pXfrm->mod_name, *jSta,
                 newSCNL->outSta, newSCNL->outChan, newSCNL->outNet, newSCNL->outLoc );
    return 0;
  }

  if ( pXfrm->nSCNL >= maxSta ) {
    /* Need to allocate more */
    maxSta += SCNL_INC;
    if ( pXfrm->scnlRecSize < sizeof( XFRMSCNL ) ) {
      logit( "e", "%s: invalid size of SCNL info record (%d); exiting!\n",
                  pXfrm->mod_name, pXfrm->scnlRecSize );
      return 1;
    }
    pXfrm->scnls = realloc( pXfrm->scnls, maxSta * pXfrm->scnlRecSize );
    if ( pXfrm->scnls == NULL ) {
      logit( "e", "%s: realloc for SCNL list failed; exiting!\n", pXfrm->mod_name );
      return 2;
    }
  }
  scnls = pXfrm->scnls;
  nSCNL = pXfrm->nSCNL;
  newSCNL = memset( (XFRMSCNL *) ( scnls + nSCNL * pXfrm->scnlRecSize ),
                    0, pXfrm->scnlRecSize );
  *jSta = nSCNL;

  strncpy( newSCNL->inSta,  inSCNL->sta,  TRACE2_STA_LEN );
  newSCNL->inSta[TRACE2_STA_LEN - 1]   = '\0';
  strncpy( newSCNL->inChan, inSCNL->chan, TRACE2_CHAN_LEN );
  newSCNL->inChan[TRACE2_CHAN_LEN - 1] = '\0';
  strncpy( newSCNL->inNet,  inSCNL->net,  TRACE2_NET_LEN );
  newSCNL->inNet[TRACE2_NET_LEN - 1]   = '\0';
  strncpy( newSCNL->inLoc,  inSCNL->loc,  TRACE2_LOC_LEN );
  newSCNL->inLoc[TRACE2_LOC_LEN - 1]   = '\0';
  if ( pXfrm->xfrmParam->debug )
    logit( "", "%s: sta[%d] SCNL=<%s.%s.%s.%s> (uninitialized)\n",
               pXfrm->mod_name, *jSta,
               newSCNL->inSta, newSCNL->inChan, newSCNL->inNet, newSCNL->inLoc );

  SetOutSCNL( newSCNL, outSCNL );
  if ( pXfrm->xfrmParam->debug )
    if ( outSCNL != NULL )
      logit( "", "%s: sta[%d] outSCNL=<%s.%s.%s.%s>\n",
                 pXfrm->mod_name, *jSta,
                 newSCNL->outSta, newSCNL->outChan, newSCNL->outNet, newSCNL->outLoc );

  newSCNL->status = -1; /* uninitialized */
  newSCNL->prevSta = -1;
  newSCNL->nextSta = -1;
  newSCNL->conversionFactor = TRACE2_NO_CONVERSION_FACTOR;
  pXfrm->nSCNL++;
  return 0;

}

/*
 * Reads the SCNL
 * In:  pxfrm pointer to the Xfrm World structure
 *      outputFlag true if output, false if input
 *      locFlag non-zero if location is provided, 0 otherwise
 * Out: scnl the SCNL
 * Returns 0 if read was successful, non-zero if error
 */
int ReadSCNL(XFRMWORLD* pXfrm, int outputFlag, int locFlag, SCNL *scnl) {
	scnl->sta = k_str(); /* read station code */
	if (!IsValidSta(pXfrm, outputFlag, scnl->sta)) {
		return 1;
	}
	scnl->chan = k_str(); /* read channel code */
	if (!IsValidChan(pXfrm, outputFlag, scnl->chan)) {
		return 2;
	}
	scnl->net = k_str(); /* read network code */
	if (!IsValidNet(pXfrm, outputFlag, scnl->net)) {
		return 3;
	}

    if (locFlag) {  /* read input location code */
    	scnl->loc = k_str(); /* read location code */
    	if (!IsValidLoc(pXfrm, outputFlag, scnl->loc))
    	{
    		return 4;
    	}
    }
    else {      /* use default blank location code */
    	scnl->loc = LOC_NULL_STRING;
    }
    return 0;
}

static int BaseProcessXfrmCommand( XFRMWORLD* pXfrm, int cmd_id, char *com ) {
  switch ( cmd_id ) {
  case CMD_ID_GETSCNL: /* GetSCNL or GetSCN */
  {
	SCNL inSCNL1, inSCNL2;
	SCNL outSCNL;
	XFRMSCNL *xfrmSCNL;
	double conversionFactor;
	int jSta1, jSta2;
	int locFlag = strcmp(com,"GetSCN");
	int outputFlag = 0;  /* input SCNL */
	/* exit if error reading the input SCNL */
	if (ReadSCNL(pXfrm, outputFlag, locFlag, &inSCNL1) != 0) {
		return -2;
	}
	if (pXfrm->nInSCNL > 1) {
		if (ReadSCNL(pXfrm, outputFlag, locFlag, &inSCNL2) != 0) {
			return -2;
		}
	}
	outputFlag = 1;  /* output SCNL */
	/* exit if error reading the output SCNL */
	if (ReadSCNL(pXfrm, outputFlag, locFlag, &outSCNL) != 0) {
		return -2;
	}
	/* exit if adding the input and output SCNL */
    if (AddSCNL(pXfrm, &inSCNL1, &outSCNL, &jSta1) != 0) {
    	return -2;
    }

	/* read conversion factor if present */
	conversionFactor = k_val();
	if (!k_err()) { /* if conversion factor present */
		xfrmSCNL = GetXFRMSCNLForSta(pXfrm, jSta1);
		xfrmSCNL->conversionFactor = conversionFactor;
	}

    if (pXfrm->nInSCNL > 1) {
    	if (AddSCNL(pXfrm, &inSCNL2, NULL, &jSta2) != 0) {
    		return -2;
    	}
	xfrmSCNL = GetXFRMSCNLForSta(pXfrm, jSta1);
	if (xfrmSCNL == NULL) {
		return -1;
	}
	xfrmSCNL->nextSta = jSta2;
	if ( pXfrm->xfrmParam->debug )
	    logit( "", "%s: sta[%d] nextSta=%d\n", pXfrm->mod_name, jSta1, jSta2 );
	xfrmSCNL = GetXFRMSCNLForSta(pXfrm, jSta2);
	if (xfrmSCNL == NULL) {
		return -1;
	}
	xfrmSCNL->prevSta = jSta1;
	if ( pXfrm->xfrmParam->debug )
	    logit( "", "%s: sta[%d] prevSta=%d\n", pXfrm->mod_name, jSta2, jSta1 );
    }
    break;
  }
  case CMD_ID_GETWAVESFROM: /* GetWavesFrom */
  {
	char *str;
	int ilogo = pXfrm->xfrmParam->nlogo;
	if(ilogo+1 >= MAX_LOGO) 
    {
       logit( "e","%s: Too many GetWavesFrom commands; "
              "max=%d; exiting!\n", pXfrm->mod_name, MAX_LOGO );
       return -2;
    }
    if((str = k_str()) != NULL) 
      strcpy(pXfrm->xfrmParam->readInstName[ilogo], str);

    if((str = k_str()) != NULL) 
      strcpy(pXfrm->xfrmParam->readModName[ilogo], str);

    if((str = k_str()) != NULL)
    {
      if( strcmp(str,"TYPE_TRACEBUF" ) != 0 &&
          strcmp(str,"TYPE_TRACEBUF2") != 0    ) 
      {
         logit( "e","%s: GetWavesFrom: invalid msg type: %s "
                    "(must be TYPE_TRACEBUF or TYPE_TRACEBUF2); "
                    "exiting!\n", pXfrm->mod_name, str );
         return -2;
      }
      strcpy(pXfrm->xfrmParam->readTypeName[ilogo], str);
    }
    pXfrm->xfrmParam->nlogo++;
    break;
  }
  default:
    return ProcessXfrmCommand( cmd_id, com );
  }
  return cmd_id;
}

/*      Function: BaseReadXfrmConfig                                            */
static int BaseReadXfrmConfig (char *configfile, XFRMWORLD* pXfrm )
{
  char     		*init=NULL;
  /* init flags, one byte for each required command */
  int      		nmiss;
  /* number of required commands that were missed   */
  char    		*com;
  char    		*str;
  int      		nfiles;
  int      		success;
  int      		i;
  /* Assume failure (for goto CONFIG_FAILURE) */
  int      		ret=EW_FAILURE;

  BaseCmd = calloc( sizeof(char*), CmdCount );
  ParamTarget = calloc( sizeof(void*), CmdCount );
  init = malloc( CmdCount );
  if ( BaseCmd==NULL || ParamTarget==NULL || init==NULL) {
    logit ("e",
             "%s: Allocation failure reading config file %s; exiting!\n",
             pXfrm->mod_name, configfile);
    goto CONFIG_FAILURE;
  }
  BaseCmd[0] = "RSMyModId";
  ParamTarget[0] = &(pXfrm->xfrmParam->myModName);
  BaseCmd[1] = "RSInRing";
  ParamTarget[1] = &(pXfrm->xfrmParam->ringIn);
  BaseCmd[2] = "RSOutRing";
  ParamTarget[2] = &(pXfrm->xfrmParam->ringOut);
  BaseCmd[3] = "RIHeartBeatInterval";
  ParamTarget[3] = &(pXfrm->xfrmParam->heartbeatInt);
  BaseCmd[4] = "RSLogFile";
  ParamTarget[4] = &(pXfrm->xfrmParam->logSwitch);
  BaseCmd[5] = "RVMaxGap";
  ParamTarget[5] = &(pXfrm->xfrmParam->maxGap);
  BaseCmd[6] = "OFTestMode";
  ParamTarget[6] = &(pXfrm->xfrmParam->testMode);
  BaseCmd[7] = "OFDebug";
  ParamTarget[7] = &(pXfrm->xfrmParam->debug);
  BaseCmd[CMD_ID_GETSCNL] = "RXGetSCNL";
  BaseCmd[9] = "OFProcessRejected";
  ParamTarget[9] = &(pXfrm->xfrmParam->processRejected);
  BaseCmd[10] = "OFGetAllSCNLs";
  ParamTarget[10] = &(pXfrm->xfrmParam->getAllSCNLs);
  BaseCmd[CMD_ID_GETWAVESFROM] = "RXGetWavesFrom";

  for ( i=NUM_BASE_CMDS; i<CmdCount; i++ ) {
    BaseCmd[i] = ModCmds[i-NUM_BASE_CMDS];
    ParamTarget[i] = ModParamTargets[i-NUM_BASE_CMDS];
  }

  /* Set to zero one init flag for each required command */
  for (i = 0; i < CmdCount; i++)
    init[i] = 0;

  /* Open the main configuration file 
   **********************************/
  nfiles = k_open (configfile); 
  if (nfiles == 0) {
    logit ("e",
             "%s: Error opening command file <%s>; exiting!\n",
             pXfrm->mod_name, configfile);
    goto CONFIG_FAILURE;
  }

  /* Process all command files
   ***************************/
  while (nfiles > 0)   /* While there are command files open */
  {
    while (k_rd ())        /* Read next line from active file  */
    {  
      com = k_str ();         /* Get the first token from line */

    /* Ignore blank lines & comments
     *******************************/
      if (!com) continue;
      if (com[0] == '#') continue;

      /* Open a nested configuration file */
      if (com[0] == '@') 
      {
        success = nfiles + 1;
        nfiles  = k_open (&com[1]);
        if (nfiles != success) {
          logit ("e",
                   "%s: Error opening command file <%s>; exiting!\n",
                   pXfrm->mod_name, &com[1]);
          goto CONFIG_FAILURE;
        }
        continue;
      }

      if ( strcmp( com, "GetSCN" ) == 0 ) /* Set com to text of GetSCNL */
        com = BaseCmd[CMD_ID_GETSCNL]+2;
      
      ret = -1;
      for ( i=0; i<CmdCount; i++ )
      {
        if ( strcmp( com, BaseCmd[i]+2 ) == 0 ) /*(k_its (BaseCmd[i]+2)) */
        {
          char type = BaseCmd[i][1];
          /* char kind = BaseCmd[i][0]; */
          switch ( type ) {
          case 'S': /* String */
            if ((str = k_str ()) != NULL )
            {
              strcpy( ParamTarget[i], str );
/*          } else {    */
/*            kind = 0; */
            }
            break;
          case 'I': /* Integer */
            *((int*)(ParamTarget[i])) = k_long();
            break;
          case 'V': /* Value ? */
            *((double*)(ParamTarget[i])) = k_val();
            break;
          case 'F': /* Flag */
            *((int*)(ParamTarget[i])) = 1;
          default: /* Unknown; user-defined? */
            ret = BaseProcessXfrmCommand( pXfrm, i, com );
            if ( ret >= 0 )
              i = ret;
            break;
          }
          break;
        }
      }
      if ( i >= CmdCount )
      {
        ret = BaseProcessXfrmCommand( pXfrm, -1, com );
      } else {
        ret = i;
      }
      if ( ret >= 0 )
      { 
        if ( pXfrm->xfrmParam->debug )
          logit("","%s: command '%s' accepted\n",pXfrm->mod_name, com);
        init[ret] = 1;
      } else if ( ret == -1 ) {
        logit ("e", "%s: <%s> Unknown command in <%s>.\n", 
               pXfrm->mod_name, com, configfile);
        continue;
      } else if ( ret < -1 ) {
          logit ("e",
                   "%s: Bad <%s> command in <%s>; exiting!\n",
                   pXfrm->mod_name, com, configfile);
          goto CONFIG_FAILURE;
      }

   /* See if there were any errors processing the command */
      if ( k_err() ) {
        logit ("e",
                 "%s: Bad <%s> command in <%s>; exiting!\n",
                 pXfrm->mod_name, com, configfile);
        goto CONFIG_FAILURE;
      }

    } /** end while k_rd() **/

    nfiles = k_close ();

  } /** end while nfiles **/

/* After all files are closed, check init flags for missed commands */
  nmiss = 0;
  for ( i = 0; i < CmdCount; i++ )
    if ( BaseCmd[i][0]=='R' && !init[i] )
      nmiss++;

  if ( nmiss > 0 ) {
    logit ("e", "%s: ERROR, no ", pXfrm->mod_name);
    for(i = 0; i < CmdCount; i++) if (!init[i]) logit("e", "<%s> ", BaseCmd[i]+2 );

    logit ("e", "command%s in <%s>; exiting!\n", (nmiss==1 ? "" : "s"), configfile);
    goto CONFIG_FAILURE;
  }

  ret = ReadXfrmConfig( init );

/* ret is initialized to EW_FAILURE */
CONFIG_FAILURE:
  free( BaseCmd );
  free( ParamTarget );
  free( init );
  return ret;
}

static int BaseReadXfrmEWH( XFRMWORLD *pXfrm )
{
  int i;

  if ( GetLocalInst( &(pXfrm->xfrmEWH->myInstId)) != 0 )
  {
    logit("e", "%s: Error getting myInstId for %d.\n", 
          pXfrm->mod_name, (int) pXfrm->xfrmEWH->myInstId);
    return EW_FAILURE;
  }
  
  if ( GetModId( pXfrm->xfrmParam->myModName, &(pXfrm->xfrmEWH->myModId)) != 0 )
  {
    logit("e", "%s: Error getting myModId for %s.\n",
           pXfrm->mod_name, pXfrm->xfrmParam->myModName );
    return EW_FAILURE;
  }

  for (i=0; i<pXfrm->xfrmParam->nlogo; i++ ) {
    if ( GetInst( pXfrm->xfrmParam->readInstName[i], &(pXfrm->xfrmEWH->readInstId[i])) != 0)
    {
      logit("e", "%s: Error getting readInstId for %s.\n",
             pXfrm->mod_name, pXfrm->xfrmParam->readInstName[i] );
      return EW_FAILURE;
    }
  
    if ( GetModId( pXfrm->xfrmParam->readModName[i], &(pXfrm->xfrmEWH->readModId[i])) != 0 )
    {
      logit("e", "%s: Error getting readModName for %s.\n",
             pXfrm->mod_name, pXfrm->xfrmParam->readModName[i] );
      return EW_FAILURE;
    }

    if ( GetType( pXfrm->xfrmParam->readTypeName[i], &(pXfrm->xfrmEWH->readMsgType[i])) != 0 )
    {
      logit("e", "%s: Error getting readMsgType for %s.\n",
             pXfrm->mod_name, pXfrm->xfrmParam->readTypeName[i] );
      return EW_FAILURE;
    }
  }

  /* Look up keys to shared memory regions */
  if ((pXfrm->xfrmEWH->ringInKey = GetKey (pXfrm->xfrmParam->ringIn)) == -1) 
  {
    logit("e", "%s:  Invalid input ring name <%s>; exiting!\n", 
           pXfrm->mod_name, pXfrm->xfrmParam->ringIn);
    return EW_FAILURE;
  }

  if ((pXfrm->xfrmEWH->ringOutKey = GetKey (pXfrm->xfrmParam->ringOut) ) == -1) 
  {
    logit("e", "%s:  Invalid output ring name <%s>; exiting!\n", 
          pXfrm->mod_name, pXfrm->xfrmParam->ringOut);
    return EW_FAILURE;
  }

  /* Look up message types of interest */
  if (GetType ("TYPE_HEARTBEAT", &(pXfrm->xfrmEWH->typeHeartbeat)) != 0) 
  {
    logit("e", "%s: Invalid message type <TYPE_HEARTBEAT>; exiting!\n", 
            pXfrm->mod_name);
    return EW_FAILURE;
  }

  if (GetType ("TYPE_ERROR", &(pXfrm->xfrmEWH->typeError)) != 0) 
  {
    logit("e", "%s: Invalid message type <TYPE_ERROR>; exiting!\n", 
            pXfrm->mod_name);
    return EW_FAILURE;
  } 

  if (GetType ("TYPE_TRACEBUF", &(pXfrm->xfrmEWH->typeTrace)) != 0) 
  {
    logit("e", "decimate: Invalid message type <TYPE_TRACEBUF>; exiting!\n");
    return EW_FAILURE;
  }

  if (GetType ("TYPE_TRACEBUF2", &(pXfrm->xfrmEWH->typeTrace2)) != 0) 
  {
    logit("e", "decimate: Invalid message type <TYPE_TRACEBUF2>; exiting!\n");
    return EW_FAILURE;
  }

  return ReadXfrmEWH();

} 

static int BaseConfigureXfrm(XFRMWORLD *pXfrm, char **argv)
{
  /* Set initial values of WORLD structure */
  InitializeParameters( pXfrm );

  /* Initialize name of log-file & open it  
   ***************************************/
  logit_init (argv[1], 0, MAXMESSAGELEN, 1);

  /* Read config file and configure the decimator */
  if (BaseReadXfrmConfig(argv[1], pXfrm) != EW_SUCCESS)
  {
    logit("e", "%s: failed reading config file <%s>\n", pXfrm->mod_name, argv[1]);
    return EW_FAILURE;
  }
  logit ("" , "Read command file <%s>\n", argv[1]);

  /* Look up important info from earthworm.h tables
   ************************************************/
  if (BaseReadXfrmEWH(pXfrm) != EW_SUCCESS)
  {
    logit("e", "%s: Call to ReadEWH failed \n", pXfrm->mod_name );
    return EW_FAILURE;
  }

  /* Reinitialize logit to the desired logging level 
   ***********************************************/
  logit_init (argv[1], 0, MAXMESSAGELEN, pXfrm->xfrmParam->logSwitch);
  
  /* Get our process ID
   **********************/
  if ((pXfrm->MyPid = getpid ()) == -1)
  {
    logit ("e", "%s: Call to getpid failed. Exiting.\n", pXfrm->mod_name);
    return (EW_FAILURE);
  }

  return ConfigureXfrm();
}

int matchXfrmSCNL( TracePacket* pPkt, unsigned char msgtype, XFRMWORLD* pXfrm )
{
  int   i;

/* Look for match in TYPE_TRACEBUF2 packet
 *****************************************/
  if( msgtype == pXfrm->xfrmEWH->typeTrace2 )
  {
     /* This should NEVER happen;
        scblRecSize had to have been set to allocate SCNL records */
     if ( pXfrm->xfrmParam->getAllSCNLs == 0 && pXfrm->scnlRecSize < sizeof(XFRMSCNL) )
     {
       logit ("e", "%s: invalid size of SCNL info record (%d); exiting!\n", pXfrm->mod_name, pXfrm->scnlRecSize );
       return -3;
     }

     for(i = 0; i < pXfrm->nSCNL; i++ )
     {
       XFRMSCNL *testSCNL = (XFRMSCNL *)(pXfrm->scnls + i*(pXfrm->scnlRecSize));
       /* try to match explicitly */
       if ((strcmp(pPkt->trh2.sta,  testSCNL->inSta)  == 0) &&
           (strcmp(pPkt->trh2.chan, testSCNL->inChan) == 0) &&
           (strcmp(pPkt->trh2.net,  testSCNL->inNet)  == 0) &&
           (strcmp(pPkt->trh2.loc,  testSCNL->inLoc)  == 0)    )
         return( i );
     }
     
     if ( pXfrm->xfrmParam->getAllSCNLs ) {
     	/* Try to add new SCNL, then "find" it if successful */
     	SCNL newSCNL;
     	memset( &newSCNL, 0, sizeof(newSCNL) );

	strncpy( newSCNL.sta,  pPkt->trh2.sta,  TRACE2_STA_LEN );
	newSCNL.sta[TRACE2_STA_LEN - 1]   = '\0';
	strncpy( newSCNL.chan, pPkt->trh2.chan, TRACE2_CHAN_LEN );
	newSCNL.chan[TRACE2_CHAN_LEN - 1] = '\0';
	strncpy( newSCNL.net,  pPkt->trh2.net,  TRACE2_NET_LEN );
	newSCNL.net[TRACE2_NET_LEN - 1]   = '\0';
	strncpy( newSCNL.loc,  pPkt->trh2.loc,  TRACE2_LOC_LEN );
	newSCNL.loc[TRACE2_LOC_LEN - 1]   = '\0';
	if ( AddSCNL( pXfrm, &newSCNL, &newSCNL, &i) == 0 )
		return matchXfrmSCNL( pPkt, msgtype, pXfrm );
     }
     return -1;  /* no match in SCNL for TYPE_TRACEBUF2 */
  }

/* Look for match in TYPE_TRACEBUF packet
 ****************************************/
  else if( msgtype == pXfrm->xfrmEWH->typeTrace )
  {
     /* This should NEVER happen;
        scblRecSize had to have been set to allocate SCNL records */
     if ( pXfrm->xfrmParam->getAllSCNLs == 0 && pXfrm->scnlRecSize < sizeof(XFRMSCNL) )
     {
       logit ("e", "%s: invalid size of SCNL info record (%d); exiting!\n", pXfrm->mod_name, pXfrm->scnlRecSize );
       return -3;
     }

     for(i = 0; i < pXfrm->nSCNL; i++ )
     {
       XFRMSCNL *testSCNL = (XFRMSCNL *)(pXfrm->scnls + i*(pXfrm->scnlRecSize));
       /* try to match explicitly */
       if ((strcmp(pPkt->trh.sta,   testSCNL->inSta)  == 0) &&
           (strcmp(pPkt->trh.chan,  testSCNL->inChan) == 0) &&
           (strcmp(pPkt->trh.net,   testSCNL->inNet)  == 0) &&
           (strcmp(LOC_NULL_STRING, testSCNL->inLoc)  == 0)    )
         return( i );
     }
     if ( pXfrm->xfrmParam->getAllSCNLs ) {
     	/* Try to add new SCNL, then "find" it if successful */
     	SCNL newSCNL;
     	memset( &newSCNL, 0, sizeof(newSCNL) );
     	
	strncpy( newSCNL.sta,  pPkt->trh2.sta,  TRACE2_STA_LEN );
	newSCNL.sta[TRACE2_STA_LEN - 1]   = '\0';
	strncpy( newSCNL.chan, pPkt->trh2.chan, TRACE2_CHAN_LEN );
	newSCNL.chan[TRACE2_CHAN_LEN - 1] = '\0';
	strncpy( newSCNL.net,  pPkt->trh2.net,  TRACE2_NET_LEN );
	newSCNL.net[TRACE2_NET_LEN - 1]   = '\0';
	strncpy( newSCNL.loc,  LOC_NULL_STRING, TRACE2_LOC_LEN );
	newSCNL.loc[TRACE2_LOC_LEN - 1]   = '\0';
	if ( AddSCNL( pXfrm, &newSCNL, &newSCNL, &i) == 0 )
		return matchXfrmSCNL( pPkt, msgtype, pXfrm );
     }
     return( -1 );  /* no match in SCN for TYPE_TRACEBUF */
  }

/* Unknown Message Type
 **********************/
  if ( pXfrm->xfrmParam->debug ) 
    logit("", "%s: Unknown message type %d\n", pXfrm->mod_name, msgtype);
  
  return( -2 );
}


/*      Function: SubtusReport                                          */
void StatusReport( XFRMWORLD *pXfrm, unsigned char type, short code, 
                   char* message )
{
  char          outMsg[MAXMESSAGELEN];  /* The outgoing message.        */
  time_t        msgTime;        /* Time of the message.                 */

  /*  Get the time of the message                                     */
  time( &msgTime );

  /*  Build & process the message based on the type                   */
    if ( pXfrm->xfrmEWH->typeHeartbeat == type )
    {
      sprintf( outMsg, "%ld %ld\n", (long) msgTime, (long) pXfrm->MyPid );
      
      /*Write the message to the output region                          */
      if ( tport_putmsg( &(pXfrm->regionOut), &(pXfrm->hrtLogo), 
                         (long) strlen( outMsg ), outMsg ) != PUT_OK )
      {
        /*     Log an error message                                    */
        logit( "t", "%s: Failed to send a heartbeat message (%d).\n",
               pXfrm->mod_name, code );
      }
    }
    else
    {
      if ( message ) {
        sprintf( outMsg, "%ld %hd %s\n", (long) msgTime, code, message );
        logit("t","Error:%d (%s)\n", code, message );
      }
      else {
        sprintf( outMsg, "%ld %hd\n", (long) msgTime, code );
        logit("t","Error:%d (No description)\n", code );
      }

      /*Write the message to the output region                         */
      if ( tport_putmsg( &(pXfrm->regionOut), &(pXfrm->errLogo), 
                         (long) strlen( outMsg ), outMsg ) != PUT_OK )
      {
        /*     Log an error message                                    */
        logit( "t", "%s: Failed to send an error message (%d).\n",
               pXfrm->mod_name, code );
      }
      
    }
}

/*      Function: BaseXfrmThread                                            */
thr_ret BaseXfrmThread (void* args) {
  XFRMWORLD     *World = ( XFRMWORLD *) args;
  char          *inMsgBuf;
  int            jSta;
  XFRMSCNL      *pSCNL;
  DATA           peekData;
  int            ret;
  MSG_LOGO       reclogo;       /* logo of retrieved message     */
  TracePacket   *inBuf;       /* pointer to raw trace message  */
  TracePacket   *outBuf;        /* pointer to debiased trace message  */

  if (!World->useInBufPerSCNL) {
	  inMsgBuf = AllocateMsgBuf();
  } else {
	  inMsgBuf = NULL;
  }

  /* set up input buffer for each SCNL */
  for(jSta = 0; jSta < World->nSCNL; jSta++ )
  {
	  pSCNL = GetXFRMSCNLForSta(World, jSta);
	  if (inMsgBuf != NULL) {
		  pSCNL->inMsgBuf = inMsgBuf;
	  } else {
		  pSCNL->inMsgBuf = AllocateMsgBuf();
		  if (pSCNL->inMsgBuf == NULL)
		  {
		    logit ("e", "%s: Cannot allocate input buffer\n", World->mod_name);
		    World->XfrmStatus = -1;
		    KillSelfThread();
		  }
	  }
  }

  outBuf    = (TracePacket *) AllocateMsgBuf();
  if (outBuf == NULL)
  {
    logit ("e", "%s: Cannot allocate output buffer\n", World->mod_name);
    World->XfrmStatus = -1;
    KillSelfThread();
  }

  /* Tell the main thread we're feeling ok */
  World->XfrmStatus = 0;

  while (1)
  {
    /* Get top message from the MsgQueue */
    RequestMutex ();
    ret = -1;
    peekData = peekNextElement(&(World->MsgQueue));
    if (peekData != NULL)
    {
        /* Extract the SCNL number; recall, it was pasted as an int on the front
         * of the message by the main thread */
    	jSta = *((int*) peekData);
    	inMsgBuf = GetInMsgBufForSta(World, jSta);
    	inBuf  = GetInBufForMsgBuf(inMsgBuf);

    	ret = dequeue (&(World->MsgQueue), inMsgBuf, &MsgBufLen, &reclogo);
    }
    ReleaseMutex_ew ();
    
    if (ret < 0)
    {                                 /* empty queue */
      sleep_ew (500);
      continue;
    }

    if ( World->completionFcn != NULL && MsgBufLen==0 )
    {
      logit("","%s: thread knows to terminate\n", World->mod_name);
      World->completionFcn( 1 );
      return THR_NULL_RET;
    }
    
    if (FilterXfrm( World, inBuf, jSta, reclogo.type, outBuf ) ==
        EW_FAILURE)
    {
      logit("t", "%s: error from FilterXfrm; exiting\n", World->mod_name);
      World->XfrmStatus = -1;
      KillSelfThread();
      return THR_NULL_RET; /* should never reach here */
    }

  } /* while (1) - message dequeuing process */
  return THR_NULL_RET; /* should never reach here unless there is a break in the continuous loop */

}

int BaseXfrmResetSCNL( XFRMSCNL *pSCNL, TracePacket *inBuf, XFRMWORLD* pWorld ) {

  pSCNL->inEndtime = 0.0;    /* used to identify gaps */
  
  /* Save params that mustn't change over span of packets */
  strcpy( pSCNL->datatype, inBuf->trh2.datatype );
  pSCNL->samprate = inBuf->trh2.samprate;
  
  return XfrmResetSCNL( pSCNL, inBuf, pWorld );

}

int BaseFilterXfrm (XFRMWORLD* pDb, TracePacket *inBuf, int jSta, 
                    unsigned char msgtype, TracePacket *outBuf)
{
  XFRMSCNL *pSCNL;
  int ret;
  
  pSCNL = (XFRMSCNL*)(pDb->scnls + (jSta * pDb->scnlRecSize));
  
  /* Create SCNL info field if need be */
  if ( pSCNL->status < 0 ) 
  {
    ret = XfrmInitSCNL( pDb, pSCNL );  
    if ( ret != EW_SUCCESS )
    {
      return ret;
    }
    pSCNL->status = 0;
    if ( pDb->xfrmParam->debug )
      logit( "", "%s: sta[%d] status=0 (initialized)\n", pDb->mod_name, jSta );
  }
  else if ( pSCNL->status )
  {
    return EW_WARNING;
  }

  /* If it's tracebuf, make it look like a tracebuf2 message */
  if( msgtype == pDb->xfrmEWH->typeTrace ) TrHeadConv( &(inBuf->trh) );
  
  if (pDb->xfrmParam->debug)
    logit("t", "%s: enter filter with <%s.%s.%s.%s> start: %lf\n",
    	  pDb->mod_name,
          inBuf->trh2.sta, inBuf->trh2.chan, inBuf->trh2.net, inBuf->trh2.loc,
          inBuf->trh2.starttime );
  
  /* Check for useable data types: we only handle shorts and longs for now */
  if ( (inBuf->trh2.datatype[0] != 's' && inBuf->trh2.datatype[0] != 'i') ||
       (inBuf->trh2.datatype[1] != '2' && inBuf->trh2.datatype[1] != '4') )
  {
    logit("t","%s: unusable datatype <%s> from <%s.%s.%s.%s>; skipping\n",
          pDb->mod_name,
          inBuf->trh2.datatype,
          inBuf->trh2.sta, inBuf->trh2.chan, inBuf->trh2.net, inBuf->trh2.loc);
    return EW_WARNING;
  }

  /* If we have previous data, check for data gap */
  if ( pSCNL->inEndtime != 0.0 )
  {
    if ( (inBuf->trh2.starttime - pSCNL->inEndtime) * inBuf->trh2.samprate > 
         pDb->xfrmParam->maxGap )
    {
      logit("t","%s: gap in data for <%s.%s.%s.%s>:\n\tlast end: %lf this start: %lf; resetting\n",
            pDb->mod_name,
            pSCNL->inSta, pSCNL->inChan, pSCNL->inNet, pSCNL->inLoc, 
            pSCNL->inEndtime, inBuf->trh2.starttime);
      if ( BaseXfrmResetSCNL( (XFRMSCNL*)pSCNL, inBuf, (XFRMWORLD*)pDb ) != EW_SUCCESS )
      {
        pSCNL->status = 1;
        return EW_FAILURE;
      }
    }
    else if (inBuf->trh2.starttime < pSCNL->inEndtime)
    {
      logit("t","%s: overlapping times for <%s.%s.%s.%s>:\n"
            "\tlast end: %lf this start: %lf; resetting\n",
            pDb->mod_name,
            pSCNL->inSta, pSCNL->inChan, pSCNL->inNet, pSCNL->inLoc, 
            pSCNL->inEndtime, inBuf->trh2.starttime);
      if ( BaseXfrmResetSCNL( (XFRMSCNL*)pSCNL, inBuf, (XFRMWORLD*)pDb ) != EW_SUCCESS )
      {
        pSCNL->status = 1;
        return EW_FAILURE;
      }
    }
    else if ( inBuf->trh2.samprate != pSCNL->samprate ) 
    {
      logit("et","%s: sample rate changed in <%s.%s.%s.%s> at %lf from %lf to %lf; exiting.\n",
            pDb->mod_name,
            pSCNL->inSta, pSCNL->inChan, pSCNL->inNet, pSCNL->inLoc, inBuf->trh2.starttime,
            pSCNL->samprate, inBuf->trh2.samprate);
      pSCNL->status = 1;
      return EW_FAILURE;
    }
    else if ( strcmp( inBuf->trh2.datatype, pSCNL->datatype ) != 0 ) 
    {
      logit("et","%s: data type changed in <%s.%s.%s.%s> at %lf from %s to %s; exiting.\n",
            pDb->mod_name,
            pSCNL->inSta, pSCNL->inChan, pSCNL->inNet, pSCNL->inLoc, inBuf->trh2.starttime,
            pSCNL->datatype, inBuf->trh2.datatype);
      pSCNL->status = 1;
      return EW_FAILURE;
    }

  }
  else
  {
    BaseXfrmResetSCNL( (XFRMSCNL*)pSCNL, inBuf, (XFRMWORLD*)pDb );
  }
  
  pSCNL->inEndtime = inBuf->trh2.endtime;
    
  return EW_SUCCESS;
}

int XfrmWritePacket( XFRMWORLD* pDb, XFRMSCNL* pSCNL, unsigned char msgtype, 
	TracePacket* outBuf, int outBufLen )
{
  pDb->trcLogo.type = msgtype;
  pDb->trcLogo.mod = pDb->xfrmEWH->myModId;
  pDb->trcLogo.instid = pDb->xfrmEWH->myInstId;
  outBuf->trh2.version[0] = TRACE2_VERSION0;
  outBuf->trh2.version[1] = TRACE2_VERSION1;
  strcpy( outBuf->trh2.sta, pSCNL->outSta );
  strcpy( outBuf->trh2.net, pSCNL->outNet );
  strcpy( outBuf->trh2.chan, pSCNL->outChan );
  strcpy( outBuf->trh2.loc, pSCNL->outLoc );
  if (pSCNL->conversionFactor != TRACE2_NO_CONVERSION_FACTOR) {
	  outBuf->trh2x.version[1] = TRACE2_VERSION11;
	  outBuf->trh2x.x.v21.conversion_factor = pSCNL->conversionFactor;
  }
  if (tport_putmsg (&(pDb->regionOut), &(pDb->trcLogo), outBufLen, 
                    outBuf->msg) != PUT_OK)
  {
    logit ("t","%s: Error sending type:%d message.\n", pDb->mod_name, msgtype );
    return EW_FAILURE;
  }
    
  return EW_SUCCESS;
}

int main (int argc, char **argv) 
{
  XFRMWORLD 	*World;                /* Our main data structure              */
  time_t    	timeNow;               /* current time                         */ 
  time_t    	timeLastBeat;          /* time last heartbeat was sent         */
  long      	sizeMsg;               /* size of retrieved message            */
  SHM_INFO  	regionIn;              /* Input shared memory region info.     */
  MSG_LOGO  	logoWave[MAX_LOGO];    /* Logo(s) of requested waveforms.      */
  int       	ilogo;                 /* working counter                      */
  MSG_LOGO  	logoMsg;               /* logo of retrieved message            */
  ew_thread_t  	tidXfrmor;             /* Transformer thread id                */
  char     		*inBuf;                 /* Pointer to the input message buffer. */
  int       	inBufLen;              /* Size of input message buffer         */
  TracePacket  	*TracePkt;
  int       	ret;
  char      	msgText[MAXMESSAGELEN];/* string for log/error messages        */

  /* Allocate a WORLD
   *****************************************************/
  CmdCount = NUM_BASE_CMDS;
  SetupXfrm( &World, &ModCmds, &CmdCount, &ModParamTargets );
  World->scnls = NULL;
  World->nSCNL = 0;

  /* Check command line arguments 
   ******************************/
  if (argc != 2)
  {
    fprintf (stderr, "Usage: %s <configfile>\n", World->mod_name);
    if ( World->version_magic == VERSION_MAGIC )
		fprintf( stderr, "Version %s\n", World->version );
    FreeXfrmWorld( World );
    exit (EW_FAILURE);
  }
  
  /* Read config file and configure the decimator */
  if (BaseConfigureXfrm(World, argv) != EW_SUCCESS)
  {
    logit("e", "%s: configure() failed \n", World->mod_name);
    FreeXfrmWorld( World );
    exit (EW_FAILURE);
  }

  if ( World->xfrmParam->testMode )
  {
    logit("e", "%s terminating normally for test mode\n", World->mod_name);
    FreeXfrmWorld( World );
    exit (EW_SUCCESS);
  }
  
  /* We will put the SCNL index in front of the trace message, so we  *
   * don't have to look up the SCNL again at the other end of the queue. */
  inBufLen = MAX_TRACEBUF_SIZ + sizeof( double );
  if ( ! ( inBuf = (char *) malloc( (size_t) inBufLen ) ) )
  {
    logit( "e", "%s: Memory allocation failed - initial message buffer!\n",
        argv[0] );
    FreeXfrmWorld( World );
    exit( EW_FAILURE );
  }
  TracePkt = (TracePacket *) (inBuf + sizeof(TP_PREFIX));
  
  /* Attach to Input shared memory ring 
   ************************************/

  tport_attach (&regionIn, World->xfrmEWH->ringInKey);
  if (World->xfrmParam->debug) {
    logit ("", "%s: Attached to public memory region %s: %ld\n", World->mod_name,
           World->xfrmParam->ringIn, World->xfrmEWH->ringInKey);
  }

  /* Attach to Output shared memory ring 
   *************************************/
  if (World->xfrmEWH->ringOutKey == World->xfrmEWH->ringInKey) {
    World->regionOut = regionIn;
  } else {
    tport_attach (&(World->regionOut), World->xfrmEWH->ringOutKey);
    if (World->xfrmParam->debug)
      logit ("", "%s: Attached to public memory region %s: %ld\n", World->mod_name,
             World->xfrmParam->ringOut, World->xfrmEWH->ringOutKey);
  }

 /* Specify logos of incoming waveforms 
  *************************************/
  for( ilogo=0; ilogo<World->xfrmParam->nlogo; ilogo++ ) {
    logoWave[ilogo].instid = World->xfrmEWH->readInstId[ilogo];
    logoWave[ilogo].mod    = World->xfrmEWH->readModId[ilogo];
    logoWave[ilogo].type   = World->xfrmEWH->readMsgType[ilogo];
    if ( World->xfrmParam->debug ) 
      logit("", "%s: Logo[%d] = %d,%d,%d\n", World->mod_name, ilogo,
          logoWave[ilogo].instid, logoWave[ilogo].mod, logoWave[ilogo].type); 
  }

  /* Specify logos of outgoing messages 
   ************************************/
  World->hrtLogo.instid = World->xfrmEWH->myInstId;
  World->hrtLogo.mod    = World->xfrmEWH->myModId;
  World->hrtLogo.type   = World->xfrmEWH->typeHeartbeat;
  if ( World->xfrmParam->debug ) 
    logit("", "%s: hrtLogo = %d,%d,%d\n", World->mod_name,
        World->hrtLogo.instid, World->hrtLogo.mod, World->hrtLogo.type); 

  World->errLogo.instid = World->xfrmEWH->myInstId;
  World->errLogo.mod    = World->xfrmEWH->myModId;
  World->errLogo.type   = World->xfrmEWH->typeError;
  if ( World->xfrmParam->debug ) 
    logit("", "%s: errLogo = %d,%d,%d\n", World->mod_name,
        World->errLogo.instid, World->errLogo.mod, World->errLogo.type); 
  
  SpecifyXfrmLogos();

  /* Force a heartbeat to be issued in first pass thru main loop  */
  timeLastBeat = time (&timeNow) - World->xfrmParam->heartbeatInt - 1;

  /* Flush the incoming transport ring */
  while (tport_getmsg (&regionIn, logoWave, (short)World->xfrmParam->nlogo, &logoMsg,
                       &sizeMsg, inBuf, inBufLen) != GET_NONE);

  /* Create MsgQueue mutex */
  CreateMutex_ew();

  /* Allocate the message Queue */
  initqueue (&(World->MsgQueue), QUEUE_SIZE, inBufLen);
  

  /* Start decimator thread which will read messages from   *
   * the Queue, decimate them and write them to the OutRing */
  if (StartThreadWithArg (XfrmThread, (void *) World, (unsigned) 
                          THREAD_STACK, &tidXfrmor) == -1)
  {
    logit( "e", 
           "%s: Error starting thread.  Exiting.\n", World->mod_name);
    tport_detach (&regionIn);
    
    if (World->xfrmEWH->ringOutKey != World->xfrmEWH->ringInKey) {
       tport_detach (&(World->regionOut)); 
    }
    free( inBuf );
    FreeXfrmWorld();
    exit( EW_FAILURE );
  }

  World->XfrmStatus = 0; /*assume the best*/

  if ( World->completionFcn != NULL )
  {
    ret = World->completionFcn( -1 );
    if ( ret != EW_SUCCESS )
    {
      logit("e", "%s: Problem with completionFcn(-1) ret=%d\n", World->mod_name, ret );
    }
  }

/*--------------------- setup done; start main loop -------------------------*/
  if ( World->xfrmParam->debug )
    logit("","%s: starting main loop\n", World->mod_name);
  while (tport_getflag (&regionIn) != TERMINATE  &&
         tport_getflag (&regionIn) != World->MyPid )
  {
    /* send module's heartbeat */
    if (time (&timeNow) - timeLastBeat >= World->xfrmParam->heartbeatInt) 
    {
      timeLastBeat = timeNow;
      StatusReport (World, World->xfrmEWH->typeHeartbeat, 0, ""); 
    }

    if (World->XfrmStatus < 0)
    {
      logit ("t", 
             "%s: thread died. Exiting\n", World->mod_name);
      exit (EW_FAILURE);
    }

    ret = tport_getmsg (&regionIn, logoWave, (short)World->xfrmParam->nlogo, 
                        &logoMsg, &sizeMsg, TracePkt->msg, MAX_TRACEBUF_SIZ);

    /* Check return code; report errors */
    if (ret != GET_OK)
    {
      if (ret == GET_TOOBIG)
      {
        sprintf (msgText, "msg[%ld] i%d m%d t%d too long for target",
                 sizeMsg, (int) logoMsg.instid,
                 (int) logoMsg.mod, (int)logoMsg.type);
        StatusReport (World, World->xfrmEWH->typeError, ERR_TOOBIG, msgText);
        continue;
      }
      else if (ret == GET_MISS)
      {
        sprintf (msgText, "missed msg(s) i%d m%d t%d in %s",
                 (int) logoMsg.instid, (int) logoMsg.mod, 
                 (int)logoMsg.type, World->xfrmParam->ringIn);
        StatusReport (World, World->xfrmEWH->typeError, ERR_MISSMSG, msgText);
      }
      else if (ret == GET_NOTRACK)
      {
        sprintf (msgText, "no tracking for logo i%d m%d t%d in %s",
                 (int) logoMsg.instid, (int) logoMsg.mod, 
                 (int)logoMsg.type, World->xfrmParam->ringIn);
        StatusReport (World, World->xfrmEWH->typeError, ERR_NOTRACK, msgText);
      }
      else if (ret == GET_NONE)
      {
        sleep_ew(500);
        continue;
      }
      else if (World->xfrmParam->debug)
        logit( "", "%s: Unexpected return from tport_getsg: %d\n", 
            World->mod_name, ret );
    }

    if ((ret = matchXfrmSCNL( TracePkt, logoMsg.type, World )) < -1 )
    {
      logit ("t", "%s: Call to matchSCNL failed with %d; exiting.\n", 
        World->mod_name, ret);
      FreeXfrmWorld();
      exit (EW_FAILURE);
    }
    else if ( ret == -1 )
    {
      /* Not an SCNL we want */
      if ( World->xfrmParam->processRejected ) 
      {
        if ( ProcessXfrmRejected( TracePkt, logoMsg, inBuf ) == EW_FAILURE )
        {
          logit ("t","%s: Error processing rejected packet; type:%d message.\n", 
          	World->mod_name, logoMsg.type );
          FreeXfrmWorld();
          exit (EW_FAILURE);
        }
      }
      continue;
    }
    
    /* stick the SCNL number as an int at the front of the message */
    *((int*)inBuf) = ret; 

    /* If necessary, swap bytes in the wave message */
    if( logoMsg.type == World->xfrmEWH->typeTrace2 )
        ret = WaveMsg2MakeLocal( &(TracePkt->trh2) );
    else if( logoMsg.type == World->xfrmEWH->typeTrace )  
        ret = WaveMsgMakeLocal( &(TracePkt->trh) );

    ret = PreprocessXfrmBuffer( TracePkt, logoMsg, inBuf );

    if ( ret < -1 )
    {
      FreeXfrmWorld();
      exit (EW_FAILURE);
    }
    if ( ret == -1 )
    {
      continue;
    }

    /* Queue retrieved msg */
    RequestMutex ();
    ret = enqueue (&(World->MsgQueue), inBuf, sizeMsg + sizeof(TP_PREFIX), logoMsg); 
    ReleaseMutex_ew ();

    if (ret != 0)
    {
      if (ret == -1)
      {
        sprintf (msgText, 
                 "Message too large for queue; Lost message.");
        StatusReport (World, World->xfrmEWH->typeError, ERR_QUEUE, msgText);
        continue;
      }
      if (ret == -3) 
      {
        sprintf (msgText, "Queue full. Old messages lost.");
        StatusReport (World, World->xfrmEWH->typeError, ERR_QUEUE, msgText);
        continue;
      }
      logit("", "%s: Problem w/ queue: %d\n", World->mod_name, ret );
    } /* problem from enqueue */

  } /* wait until TERMINATE is raised  */  

  if ( World->completionFcn != NULL ) /* send empty packet to signal termination */
  {
    RequestMutex (); 
    ret = enqueue (&(World->MsgQueue), inBuf, 0, logoMsg); 
    ReleaseMutex_ew ();
    ret = World->completionFcn( 0 );
    if ( ret != EW_SUCCESS )
    {
      logit("e", "%s: Problem with completionFcn(0) ret=%d\n", World->mod_name, ret );
    }
  }

  /* Termination has been requested */
  tport_detach (&regionIn);
  if (World->xfrmEWH->ringOutKey != World->xfrmEWH->ringInKey) {
    tport_detach (&(World->regionOut));
  }
  free( inBuf );
  FreeXfrmWorld();
  logit ("t", "Termination requested; exiting!\n" );
  exit (EW_SUCCESS);

}
