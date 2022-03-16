
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: xfrm.h,v 1.0 2009/12/21 12:00 Exp $
 *
 *    Revision history:
 *     $Log: xfrm.h,v $
 *
 */

/*
 * transform.h: Definitions for the Decimate Earthworm Module.
 */

/*******                                                        *********/
/*      Redefinition Exclusion                                          */
/*******                                                        *********/
#ifndef __XFRM_H__
#define __XFRM_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <transport.h>  
#include <trace_buf.h>
#include <mem_circ_queue.h>

/*******                                                        *********/
/*      Constant Definitions                                            */
/*******                                                        *********/


/*    StatusReport Error Codes - Must coincide with .desc       */
#define  ERR_MISSMSG       0   /* message missed in transport ring      */
#define  ERR_TOOBIG        1   /* retreived msg too large for buffer    */
#define  ERR_NOTRACK       2   /* msg retreived; tracking limit exceeded*/
#define  ERR_XFRM          3   /* call to the transform routine failed  */
#define  ERR_QUEUE         4   /* trouble with the MsgQueue operation   */

/*    Buffer Lengths                                                    */
#define MAXFILENAMELEN  80      /* Maximum length of a file name.       */
#define MAXLINELEN      240     /* Maximum length of a line read from   */
                                /*   or written to a file.              */
#define MAXMESSAGELEN   1024     /* Maximum length of a status or error  */
                                /*   message.                           */
#define MAX_LOGO        10      /* Maximum number of logos to listen to */
#define BUFFSIZE MAX_TRACEBUF_SIZ * 2 /* Size of a channel trace buffer */
#define MAXSTAGE        10      /* Maximum number of decimation stages  */
#define	QUEUE_SIZE	    300	    /* How many msgs can we queue           */
#define THREAD_STACK    81920    /* How big is our thread stack          */
#define NUM_BASE_COMMANDS 11    /* Number of standard commands          */
#define TP_PREFIX       int     /* Type of prefix added to TraceBufs    */

/* the offset of wave bytes in the message buffer */
#define WAVE_BYTES_OFFSET (sizeof(TRACE2_HEADER))
/* the max number of wave byte values */
#define MAX_WAVE_BYTES_SIZ (MAX_TRACEBUF_SIZ-WAVE_BYTES_OFFSET)
/* the max number of wave short values */
#define MAX_WAVE_SHORT_SIZ (MAX_WAVE_BYTES_SIZ/sizeof(short))
/* the max number of wave long values */
#define MAX_WAVE_LONG_SIZ (MAX_WAVE_BYTES_SIZ/sizeof(long))

#define VERSION_MAGIC	0x0666feed	/* Magic number signalling version defined */

/*******                                                        *********/
/*      Structure Definitions                                           */
/*******                                                        *********/

  /** SCNL Structure */
typedef struct _SCNL
{
	char *sta;
	char *chan;
	char *net;
	char *loc;
} SCNL;

  /*    Xfrm Parameter Structure (read from *.d files)             */
typedef struct _XFRMPARAM
{
#include "xfrm_param.h"
} XFRMPARAM;

  /*    Information Retrieved from Earthworm*.h                         */
typedef struct _XFRMEWH
{
#include "xfrm_ewh.h"
} XFRMEWH;

  /*    Information about an individual SCNL:                           */
typedef struct _XFRMSCNL
{
#include "xfrm_scnl.h"
} XFRMSCNL;

  /*    Xfrm World structure                                            */
typedef struct _XFRMWORLD
{
  XFRMEWH       *xfrmEWH;       /* Structure for Earthworm parameters.  */
  XFRMPARAM     *xfrmParam;     /* Network parameters.                  */
#include "xfrm_world.h"
} XFRMWORLD;


/* Xfrm function prototypes                                         */

/* Functions that MUST be defined for a specific transformer             */

/*************************************************************************
    SetupXfrm creates/initializes the world, param and EWH structures, 
    as well as the information about commands to be read from the config 
    file(s)
**************************************************************************/
void      SetupXfrm( 
	XFRMWORLD **pXfrm,        /* World structure                         */
	char      **Cmds[], 	  /* Command definitions to ADD to xfrm ones */
	int       *CmdCount,      /* Length of Cmds                          */
	void      **ParamTarget[] ); /* Address of each added command target */

/*************************************************************************
    ConfigureXfrm gets called after the config file(s) and earthworm.h 
    table info have been read in, so any module-specific processing of 
    that data can be done.  If it returns EW_FAILURE, module will abort.
**************************************************************************/
int       ConfigureXfrm();

/*************************************************************************
    InitializeXfrmParameters gets called after standard parameters have 
    been initialized, so your module-specific parameters can be 
    initialized
**************************************************************************/
void      InitializeXfrmParameters();

/*************************************************************************
    SpecifyXfrmLogos gets called after standard incomming and outgoing 
    logos have been specified, so any specific to this module can be added
**************************************************************************/
void      SpecifyXfrmLogos();

/*************************************************************************
    ReadXfrmEWH gets called after standard earthworm.h information has been
    read in, so any earthworm.h information specific to this module can be
    handled.  If it returns EW_FAILURE, module will abort.
**************************************************************************/
int       ReadXfrmEWH();

/*************************************************************************
    FreeXfrmWorld gets called just before the module exits, so that any 
    module-specific cleanup can be done
**************************************************************************/
void      FreeXfrmWorld();

/*************************************************************************
    ProcessXfrmCommand gets called when either the type of a known command is
    unknown (cmd_id will be its index) or when the command itself is unknown
    (cmd_id will be -1).  This allows any module-specific command processing
    that isn't a simple parse (string, int, double or flag) to be handled.
    Return the id of the command, -1 if something goes wrong.
**************************************************************************/
int       ProcessXfrmCommand( 
	int  cmd_id,                      /* index of command; -1 if unknown */
	char *com );                      /* command text                    */

/*************************************************************************
    XfrmThread is the function run in its own thread to process individual
    packets
**************************************************************************/
thr_ret   XfrmThread(
	void *);         /* a pointer to the World structure for this module */

/*************************************************************************
    ReadXfrmConfig after the config file is processed, so any module-specific
    processing of the config file(s) can be handled
**************************************************************************/
int       ReadXfrmConfig( 
	char *init );    /* init[i] = 'command w/ index i has been accepted' */

/*************************************************************************
    PreprocessXfrmBuffer gets called before the packet is sent off to the 
    worker thread.  If you want the (possibly modified) packet to be passed
    to the thread, return 0; if you don't want it passed on, return -1;
    any other negative value will signify an error and terminate the module.
**************************************************************************/
int       PreprocessXfrmBuffer( 
	TracePacket *TracePkt, /* packet about to be processed */
	MSG_LOGO logoMsg, /* logo of packet */
	char *inBuf );

/*************************************************************************
    ProcessXfrmRejected gets called for each packet not matched to a GetSCNL 
    command.  If it returns EW_FAILURE, module will abort.
**************************************************************************/
int       ProcessXfrmRejected( TracePacket *TracePkt, MSG_LOGO logoMsg, char *inBuf );

/*************************************************************************
    FilterXfrm gets called for each packet matched to a GetSCNL 
    command by BaseXfrmThread.  This needs to be defined even if you are 
    neither calling it from your code nor using BaseXfrmThread. 
**************************************************************************/
int       FilterXfrm (
    XFRMWORLD     *pXfrm,       /* world structure                       */
    TracePacket   *inBuf,       /* packet read from input ring           */
    int           jSta,         /* index of SCNL                         */
    unsigned char msgtype,      /* type of packet                        */
    TracePacket   *outBuf);     /* space for corresponding output packet */


/*************************************************************************
    XfrmResetSCNL gets called at the start of each span of packets for a
    the specified SCNL.  If you use BaseFilterXfrm, then the datatype, 
    endTime and samprate fields of xSCNL will be set before this is called.
**************************************************************************/
int XfrmResetSCNL( 
	XFRMSCNL *xSCNL, 
	TracePacket *inBuf, 
	XFRMWORLD* xWorld );        /* world structure                       */

/*************************************************************************
    XfrmInitSCNL gets called once the first packet for a SCNL is received, 
    so that pSCNL can be initialized.
**************************************************************************/
int XfrmInitSCNL( 
	XFRMWORLD* pDb, 
	XFRMSCNL* pSCNL );  


/* Functions defined the xfrm.c that may be used elsewhere */

/*************************************************************************
    BaseXfrmThread is an implementation of XfrmThread which calls XfrmFilter 
    for each packet that it gets from the queue
**************************************************************************/
thr_ret   BaseXfrmThread(
	void *); /* a pointer to the World structure for this module */

/*************************************************************************
    BaseFilterXfrm does the common processing FilterXfrm would do:
    - Initialize SCNL structure if first packet for that SCNL
    - Convert packet to TRACEBUF2 if needed
    - Validate datatype
    - Check for gaps and overlaps
    - Verify datatype nor samplerate changes between packets in a span
    Your FilterXfrm can call this before doing its module-specific processing
**************************************************************************/
int       BaseFilterXfrm (
    XFRMWORLD     *pXfrm,       /* world structure                       */
    TracePacket   *inBuf,       /* packet read from input ring           */
    int           jSta,         /* index of SCNL                         */
    unsigned char msgtype,      /* type of packet                        */
    TracePacket   *outBuf);     /* space for corresponding output packet */

/*************************************************************************
    BaseXfrmResetSCNL handles the assignments that preceed calls to 
    XfrmResetSCNL and then calls XfrmResetSCNL
**************************************************************************/
int BaseXfrmResetSCNL( XFRMSCNL *xSCNL, TracePacket *inBuf, XFRMWORLD* xWorld );

/*************************************************************************
    XfrmInitSCNL gets called once the first packet for a SCNL is received, 
    so that pSCNL can be initialized.
**************************************************************************/
int XfrmWritePacket( XFRMWORLD* pDb, XFRMSCNL* pSCNL, unsigned char msgtype, 
	TracePacket* outbuf, int outBufLen );

/*
 * Add the input and output SCNL
 * In:  pxfrm pointer to the Xfrm World structure
 *      inSCNL the input SCNL
 *      outSCNL the output SCNL or NULL if none
 * Out: jSta the station index
 * Returns 0 if add was successful, non-zero if error
 */
int AddSCNL(XFRMWORLD* pXfrm, SCNL *inSCNL, SCNL *outSCNL, int *jSta);

/*
 * Get the input buffer for the message buffer.
 * In:  msgBuf the message buffer
 * Returns the input buffer
 */
TracePacket* GetInBufForMsgBuf(char* msgBuf);

/*
 * Get the input buffer for the station.
 * In:  pxfrm pointer to the Xfrm World structure
 *      jSta the station index
 * Returns the input buffer
 */
TracePacket* GetInBufForSta(XFRMWORLD* pXfrm, int jSta);

/*
 * Get the wave bytes for the buffer.
 * In:  buf the buffer
 * Returns the wave bytes.
 */
char* GetWave(TracePacket* buf);

/*
 * Get the data size.
 * In:  buf the buffer
 * Return the data size.
 */
int GetDataSize(TracePacket* buf);

/*
 * Compare the SCNLs.
 * In:  scnl1 the first scnl
 *      scnl the second scnl
 * Return 0 if the SCNLs match, non-zero otherwise.
 */
int BaseCompareSCNL(XFRMSCNL* scnl1, XFRMSCNL* scnl2);

/*
 * Reads the SCNL
 * In:  pxfrm pointer to the Xfrm World structure
 *      outputFlag true if output, false if input
 *      locFlag non-zero if location is provided, 0 otherwise
 * Out: scnl the SCNL
 * Returns 0 if read was successful, non-zero if error
 */
int ReadSCNL(XFRMWORLD* pXfrm, int outputFlag, int locFlag, SCNL *scnl);


#endif  /*  __XFRM_H__                                              */
