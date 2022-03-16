/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: sacputaway.c 7121 2018-02-15 02:49:51Z baker $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.27  2005/06/13 15:00:46  patton
 *     Checked in changes to make error returns from WaveMsg2MakeLocal more clear,
 *     converted SACPABase_write_trace from TRACE_HEADER to TRACE2_HEADER, changed
 *     logic so that if a packet with a bad end time is a fatal error only if it's
 *     the first packet in a TRACE_REQ, otherwise skip packet as bad and fill it as
 *     a gap.  These changes were needed in Golden for the Centroid Moment Tensor
 *     Effort.  JMP
 *
 *     Revision 1.26  2004/07/16 20:44:21  lombard
 *     Modified to provide minimal support for SEED location codes.
 *
 *     Revision 1.25  2002/06/28 21:06:22  lucky
 *     Lucky's pre-departure checkin. Most changes probably had to do with bug fixes
 *     in connection with the GIOC scaffold.
 *
 *     Revision 1.24  2002/03/22 18:38:13  lucky
 *     Fixed SACPABase_init prototype
 *
 *     Revision 1.23  2002/03/22 18:25:24  lucky
 *     Changed SACPABase_init to only create a new directory if told to do so. This way, files
 *     can be added to a directory.
 *
 *     Revision 1.22  2001/08/02 22:48:50  davidk
 *     Added explicit float casts to 4 statements to get rid of
 *     compiler warnings on NT.
 *
 *     Revision 1.21  2001/08/02 22:42:01  lucky
 *     *** empty log message ***
 *
 *     Revision 1.20  2001/06/06 20:56:43  lucky
 *     Fixing support for many mag types and amplitude picks
 *
 *     Revision 1.19  2001/05/31 17:08:54  lucky
 *     Added evdp for event depth - used to only write to evel.
 *
 *     Revision 1.18  2001/04/12 03:47:36  lombard
 *     Major reorganization of SACPABase_write_trace to improve efficiency
 *     Reformatted code; cleaned up lots of comments and logit calls
 *     Removed mapping of NC component codes (only needed at UW)
 *
 *     Revision 1.17  2000/12/06 17:50:07  lucky
 *     Added cOnset -- we keep track of the pick's onset in the fourth
 *     character of the phase label (ka, or kt0)
 *
 *     Revision 1.16  2000/11/15 17:03:27  lucky
 *     Added SACPABase_next_ev_review which sets SacDir variable. (needed this
 *     to write PZ files correctly). As a result, we no longer need end_ev_review,
 *     so I removed it.
 *
 *     Revision 1.15  2000/09/07 21:17:45  lucky
 *     Final version after the Review pages have been demonstrated.
 *
 *     Revision 1.14  2000/08/25 18:20:41  lucky
 *     Fixed to work with multiple arrival types (S and P)
 *
 *     Revision 1.13  2000/08/17 18:36:51  lucky
 *     Fixed major potential bugs: Increased size of sacfile to 2*MAXTXT;
 *     also, we now make sure that the target string is big enough for th
 *     source and also the termination character.
 *
 *     Revision 1.12  2000/07/27 15:55:59  lucky
 *     Changed MAX_PHS_PER_EQ to DB_MAX_PHS_PER_EQ
 *
 *     Revision 1.11  2000/07/03 18:26:24  lucky
 *     Added SACPABase_end_scn_review for sac writing from inside the
 *     review pages. This is necessary for the review applet to run.
 *
 *     Revision 1.10  2000/05/24 15:58:31  lucky
 *     Changed calls to creat/write to fopen/fwrite. The old way would not write
 *     correct SAC files under NT.
 *
 *     Revision 1.9  2000/04/13 17:12:58  lucky
 *     Fixed several logit lines to only log to file.
 *
 *     Revision 1.8  2000/04/13 17:06:01  lucky
 *     In CreateSACSupportFiles: took out the chdir call to SacDir. Instead, the support
 *     files are opened from the current directory by appending SacDir to the name of the
 *     file. This was necessary in order to allow for multiple events to be processed by
 *     the archiver. Before, we would never chdir back to the top level directory, causing
 *     all subsequent calls to the Sac putaway routines to bomb.
 *
 *     Also, fixed zCurEventIDACPABase_next_ev so that if dEventStartTime is 0 for some reason,
 *     the directory created will have the current time. Before it would always create
 *     a directory with 1970 date (i.e. time_t of 0).
 *
 *     Revision 1.7  2000/03/31 18:26:47  davidk
 *     *** empty log message ***
 *
 *     Revision 1.6  2000/03/24 07:36:40  davidk
 *     added code to pad the pick label field with blanks, instead of the default (-12345)
 *
 *     Revision 1.5  2000/03/14 18:34:41  lucky
 *     Fixed a bug with creation of SAC directories: they were being created
 *     as yyyymmmm, instead of yyyymm.
 *
 *     Revision 1.4  2000/03/13 21:36:05  lucky
 *     Fixed error in SACPA_next_ev where pOrigin was used when it was
 *     still set to NULL.
 *
 *     Revision 1.3  2000/03/10 23:27:33  davidk
 *     Redesigned the sacputaway routines so that they could also be
 *     used to write parametric event and station data in addition to
 *     being used for trace disposal.  Moved most of the old functionality
 *     into a SACPABase_XXX layer.  Now the SACPA_XXX routines serve as
 *     wrappers for the SACPABase_XXX routines, which do the actual SAC
 *     header filling and file writing.  Added code adapted from ora2sac
 *     to write Origin and arrival data.  Added code to write Poles and Zeroes
 *     station signal response data.  Changed the SACPA_XXX routines to
 *     matche the new putaway interface in putaway.c
 *
 *     Revision 1.1  2000/02/14 18:51:48  lucky
 *     Initial revision
 *
 *
 */

/* sacputaway.c 

  Wed Feb 23 XX:XX:XX PST 2000 David Kragness
    Modified the sacputaway routines to use an additional layer.
        It was desired that the same core code be used to do all 
        data disposal into SAC, and some programs needed/wanted to
        write parametric data as well as trace data.  In the putaway
        interface, there is only room for trace data.  So the SACPA_XXX
        routines have been modified so that the guts of the original 
        routines have gone into a layer of functions called 
        SACPABase_XXX, and the original SACPA_XXX routines have
        become wrappers for those routines using the XXX_tracesave 
        "putaway" interface.  Additional functionality has been added
        to the SACPABase_XXX routines for writing parametric data to
        SAC in addition to the waveform data that was written by the
        original SACPA_XXX routines.


  Thu Dec 30 00:01:12 GMT 1999 Lucky Vidmar
    Made the fixes necessary to produce SAC files on NT which 
    are readable on Solaris. For this, we needed to do some magic
    with the SAC header, so that it can be byte-swapped correctly.
    Namely, we now have two SAC headers floating around: one is 
    in the local machine format, and its values are copied to the 
    temporary header so that it can be swapped (the "problem" being
    that byte-swapping is done in-place). Then, once we get back
    a byte-swapped header, we memcpy it back to the memory that is
    written out along with the previously byte-swapped data. 

  Thu Jul 22 14:56:37 MDT 1999 Lucky Vidmar
    Major structural changes to accomodate making all putaway routines
    into library calls so that they can be shared between trig2disk 
    and wave2disk. Eliminated global variables set and allocated outside
    of this file. We should only have static globals from now on, with 
    everything else being passed in via the putaway.c calls.


  Tue Apr 13 13:01:23 MDT 1999 Lucky Vidmar
    Changed target directory names to reflect ora2sac. Also, we now
    create a bunch of support files (sac macros) in the target directory,
    much like ora2sac. As a matter of fact, most of the code which 
    creates sac macros was modelled after ora2sac code.


 These are the routines which plug into routines in putaway.c, and cause 
 the trace data snippets to be put away into a SAC format trace files.  

   Written by: Pete Lombard, March 1998

 Modified by withers to work with trig2disk Sep, 1998
 Ported to NT withers Oct 1998
   
   */

/************************************/
/** #includes                      **/
/************************************/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "earthworm.h"
#include "trace_buf.h"
#include "swap.h"
#include "ws_clientII.h"
#include "chron3.h"
#include "sachead.h"
#include "sacputaway.h"
#include "pa_subs.h"
#include "time_ew.h"


/*********************************************/
/** GLOBAL VARIABLES  (global to this file )**/
/*********************************************/
static SACFileListStruct   FileList[DB_MAX_PHS_PER_EQ];
static int                 FileIndex;
static char                szCurEventID[EVENTID_SIZE+1];
static int                 SACPAB_Debug=0;
static char                SAC_szOutDir[MAXTXT];
static char                SAC_szOutputFormat[20];
static double              SAC_starttime;
static SAC_OriginStruct    SAC_Origin;


/* use statically allocated SAC headers.  It's safer and faster
   than dynamically mallocing them and it's not 
   that much space (6XX bytes each)
****************************************************************/
static char default_sachead[SACHEADERSIZE];
static char sachead[SACHEADERSIZE];

/* sacheadp is where the "real" representation of the header is, i.e
   the one that is valid on the architecture that executes the code
****************************************************************/
static struct   SAChead *sacheadp=(struct SAChead *) sachead;  

/* default_sacheadp is the default sac header image.  This way we don't
   have to go through painful initialization each time, we just slam
   the default into our target header.
****************************************************************/
static struct   SAChead *default_sacheadp=(struct SAChead *) default_sachead;

static char  SacDir[2*MAXTXT + 4];
static char *SacBuffer;   /* buffer for storing output before writing */
static char  SAC_szSta[10], SAC_szChan[10], SAC_szNet[10], SAC_szLoc[10];
static int   BufferLen;
static int   bSACPABase_init=FALSE;
static int   bSACPABase_scnl_open=FALSE;


/************************************/
/** FUNCTION PROTOTYPES            **/
/************************************/

/* supporting routines 
*************************************/
void sacinit (struct SAChead *);
void swapsac (struct SAChead *);
int SAC_Compare (const void *, const void *);
int CreateSACSupportFiles (char *, SACFileListStruct *, int);
int SACmac_init (void);                          
int SACmac_quicklook (SACFileListStruct *, int);
int SACmac_repick (SACFileListStruct *, int);
int SAC_filelist (SACFileListStruct *, int, char *);
int SAC_SwapDouble(double * pValue, char cDataType);
int SAC_SwapInt32(int32_t * pValue, char cDataType);
int SAC_SwapShort(short * pValue, char cDataType);


/********************************************************************
*********************************************************************
###################### SAC PUTAWAY ROUTINES #########################
*********************************************************************
********************************************************************/

/* Initialization function, 
*       This is the Put Away startup intializer. This is called when    *
*       the system first comes up. Here is a chance to look around      *
*       and see if it's possible to do business, and to complain        *
*       if not ,BEFORE an event has to be processed.                    *
*/
int SACPA_init(long OutBufferLen, char *OutDir, char * OutputFormat, int debug)
{
  int rc;

  rc=SACPABase_Debug(debug);
  if(rc!=EW_SUCCESS)
  {
    logit("","%s(): %s() failed returning %d.\n",
          "SACPA_init","SACPABase_Debug",rc);
    return(EW_FAILURE);
  }

  rc=SACPABase_init(OutBufferLen,OutDir,TRUE,OutputFormat);
  if(rc!=EW_SUCCESS)
  {
    logit("","%s(): %s() failed returning %d.\n",
          "SACPA_init","SACPABase_init",rc);
    return(EW_FAILURE);
  }
  return(EW_SUCCESS);
}

/****************************************************************************
*       This is the Put Away event initializer. It's called when a snippet  *
*       has been received, and is about to be processed.                    *
*       It gets to see the pointer to the TraceRequest array,               *
*       and the number of loaded trace structures.                          *
*****************************************************************************/
int SACPA_next_ev(char *EventID, TRACE_REQ *ptrReq, int nReq, 
                  char *OutDir, char *EventDate, char *EventTime, int debug)
{

  int rc;
  char szTime[40];
  double dTime;

  rc=SACPABase_Debug(debug);
  if(rc!=EW_SUCCESS)
  {
    logit("","%s(): %s() failed returning %d.\n",
          "SACPA_next_ev","SACPABase_Debug",rc);
    return(EW_FAILURE);
  }

  sprintf(szTime,"%s%s",EventDate,EventTime);
  if(epochsec17(&dTime,szTime))
  {
    logit("","SACPA_next_ev(): Error in epochsec17() parsing time:(%s)\n",
          szTime);
    return(EW_FAILURE);
  }

  rc=SACPABase_next_ev(EventID,dTime,NULL);
  if(rc!=EW_SUCCESS)
  {
    logit("","%s(): %s() failed returning %d.\n",
          "SACPA_next_ev","SACPABase_next_ev",rc);
    return(EW_FAILURE);
  }

  return(EW_SUCCESS);
}


/*****************************************************************************
 *   This is the working entry point into the disposal system. This routine  *
 *   gets called for each trace snippet which has been recovered. It gets    *
 *   to see the corresponding SNIPPET structure, and the event id            *
 *****************************************************************************/
/* Process one channel of data */
int SACPA_next(TRACE_REQ *getThis, double GapThresh, 
               long OutBufferLen, int debug)
     /*
 * input:  getThis   pointer to buffer for the trace data request
 *         eventId   not used here 
 */

{
  int rc;

  rc=SACPABase_Debug(debug);
  if(rc!=EW_SUCCESS)
  {
    logit("","%s(): %s() failed returning %d.\n",
          "SACPA_next","SACPABase_Debug",rc);
    return(EW_FAILURE);
  }

  rc=SACPABase_next_scnl(getThis->sta,getThis->chan,getThis->net,getThis->loc);
  if(rc!=EW_SUCCESS)
  {
    logit("","%s(): %s() failed returning %d.\n",
          "SACPA_next","SACPABase_next_scnl",rc);
    /*Patch, in trig2disk we would continue the event, skipping a failed trace*/
    bSACPABase_scnl_open=FALSE;
    return(EW_FAILURE);
  }

  rc=SACPABase_write_trace(getThis,GapThresh);
  if(rc!=EW_SUCCESS)
  {
    logit("","%s(): %s() failed returning %d.\n",
          "SACPA_next","SACPABase_write_trace",rc);
    return(EW_FAILURE);
  }

  rc=SACPABase_end_scnl();
  if(rc!=EW_SUCCESS)
  {
    logit("","%s(): %s() failed returning %d.\n",
          "SACPA_next","SACPABase_end_scnl()",rc);
    return(EW_FAILURE);
  }

  return(EW_SUCCESS);
}



/************************************************************************
*       This is the Put Away end event routine. It's called after we've     *
*       finished processing one event.                                  *
*************************************************************************/
int SACPA_end_ev (int debug)
{
  int rc;

  rc=SACPABase_Debug(debug);
  if(rc!=EW_SUCCESS)
  {
    logit("","%s(): %s() failed returning %d.\n",
          "SACPA_end_ev","SACPABase_Debug",rc);
    return(EW_FAILURE);
  }

  rc=SACPABase_end_ev();
  if(rc!=EW_SUCCESS)
  {
    logit("","%s(): %s() failed returning %d.\n",
          "SACPA_end_ev","SACPABase_end_ev",rc);
    return(EW_FAILURE);
  }

  return(EW_SUCCESS);
}


/************************************************************************
*       This is the Put Away close routine. It's called after when      *
*       we're being shut down.                                          *
*************************************************************************/
int SACPA_close (int debug)
{
  int rc;

  rc=SACPABase_Debug(debug);
  if(rc!=EW_SUCCESS)
  {
    logit("","%s(): %s() failed returning %d.\n",
          "SACPA_close","SACPABase_Debug",rc);
    return(EW_FAILURE);
  }

  rc=SACPABase_close();
  if(rc!=EW_SUCCESS)
  {
    logit("","%s(): %s() failed returning %d.\n",
          "SACPA_close","SACPABase_close",rc);
    return(EW_FAILURE);
  }

  return(EW_SUCCESS);
}



/********************************************************************
*********************************************************************
#################### SAC PUTAWAY BASE ROUTINES ######################
*********************************************************************
********************************************************************/




/********************************************************************
*********************************************************************
    Function:      SACPABase_Debug

    Description:   Turns debugging on/off for SACPABase 
                   functions
  
    Return Value:  success: EW_SUCCESS 
                   failure: EW_FAILURE
                   others:  Undefined

    Comments:
*********************************************************************
********************************************************************/
int SACPABase_Debug(int bDebug)
{
  SACPAB_Debug=bDebug;
  return(EW_SUCCESS);
}

/********************************************************************
*********************************************************************
    Function:      SACPABase_SetOutputFormat

    Description:   Sets the output format for the SACPABase functions.
                    ("intel" or "sparc")
  
    Return Value:  success: EW_SUCCESS 
                   failure: EW_FAILURE
                            SACPAB_OUTPUT_FORMAT_TOO_LONG
                   others:  Undefined

    Comments:
*********************************************************************
********************************************************************/
int SACPABase_SetOutputFormat(char * szOutputFormat)
{
  if(strlen(szOutputFormat) > sizeof(SAC_szOutputFormat))
  {
    logit("","SACPABase_SetOutputFormat():Error: %s output format is too long!\n",
          szOutputFormat);
    return(SACPAB_OUTPUT_FORMAT_TOO_LONG);
  }
  strcpy(SAC_szOutputFormat,szOutputFormat);
  return(EW_SUCCESS);
}

/********************************************************************
*********************************************************************
    Function:      SACPABase_init

    Description:   SAC Putaway Base initializer.
  
    Return Value:  success: EW_SUCCESS 
                   failure: EW_FAILURE (unknown error)
                            SACPAB_ALREADY_INIT (SACPABase is already 
                             initialized.  Please call SACPABase_close()
                             before calling SACPABase_init() again.

                   others:  Undefined

    Comments:      This should be called when the system first comes 
                   up.  It gives SACPABase  a chance to look around 
                   and see if it's possible to do business, and to  
                   complain if not ,BEFORE an event has to be 
                   processed. 
*********************************************************************
********************************************************************/
int SACPABase_init(int OutBufferLen, char *szOutDir, int CreateNewDir, char * szOutputFormat)
{
  if (bSACPABase_init)
  {
    logit("","SACPABase_init() called multiple times.\n");
    return(SACPAB_ALREADY_INIT);
  }

  if(!(szOutDir && szOutputFormat))
  {
    logit("","SACPABase_init():  Error: Null pointers passed.\n");
    return(SACPAB_NULL_POINTERS);
  }

  if(OutBufferLen > MIN_OUTBUFFER_LEN)
    BufferLen=OutBufferLen;
  else 
    BufferLen=MIN_OUTBUFFER_LEN;

  /* Allocate SacBuffer */
  if ((SacBuffer = (char *) malloc (BufferLen * sizeof (char))) == NULL)
  {
    logit ("e", "SACPABase_init: couldn't malloc SacBuffer(%d)\n", BufferLen);
    return EW_FAILURE;
  }

  /* set init flag here to minimize memory leaks. */
  bSACPABase_init=TRUE;

  if(strlen(szOutDir) > (sizeof(SAC_szOutDir) + 1))
  {
    logit("e","SACPABase_init():Error: %s directory name is too long!\n",
          szOutDir);
    return(SACPAB_DIRECTORY_TOO_LONG);
  }
  strcpy(SAC_szOutDir,szOutDir);

  /* Make sure that the top level output directory exists */

  if (CreateNewDir == TRUE)
  {
    if (CreateDir (SAC_szOutDir) != EW_SUCCESS)
    {
      logit ("e", "SACPABase_init: Call to CreateDir(%s) failed\n",SAC_szOutDir);
      return EW_FAILURE;
    }
  }

  if(strlen(szOutputFormat) > sizeof(SAC_szOutputFormat))
  {
    logit("e","SACPABase_init():Error: %s output format is too long!\n",
          szOutputFormat);
    return(SACPAB_OUTPUT_FORMAT_TOO_LONG);
  }
  strcpy(SAC_szOutputFormat,szOutputFormat);

  /* Initialize all header values to SAC defaults */
  sacinit(default_sacheadp);

  /* the following sac header vars must be set, but not neccessarily 
     here:  npts,b,e,iftype,leven,delta
  ******************************************************************/
  /* Set some header values that will never change */
  default_sacheadp->idep  = SAC_IUNKN;      /* unknown independent data type */
  default_sacheadp->iztype = SAC_IBEGINTIME; /* Reference time is Begin time */
  default_sacheadp->iftype = SAC_ITIME;    /* File type is time series */
  default_sacheadp->leven  = 1;            /* evenly spaced data */
  default_sacheadp->b      = 0; /* beginning time relative to reference time */
  strncpy(default_sacheadp->ko, "origin ", K_LEN);

  /* Initialize CurEventID */
  strcpy(szCurEventID,"-1");
  
  return(EW_SUCCESS); 
}


/****************************************************************************
*       This is the Put Away event initializer. It's called when a snippet  *
*       has been received, and is about to be processed.                    *
*       It gets to see the pointer to the TraceRequest array,               *
*       and the number of loaded trace structures.                          *
*****************************************************************************/
int SACPABase_next_ev(char * szEventID, double dEventStartTime, 
                      SAC_OriginStruct * pOrigin)
{

  char     tmpname[256];
  char     tmp1[256];
  time_t tTemp=(int)dEventStartTime;
  struct tm tmTemp;


  if (dEventStartTime == 0.0)
    tTemp = time (NULL);

  gmtime_ew(&tTemp,&tmTemp);

  strncpy(szCurEventID,szEventID,EVENTID_SIZE);
  szCurEventID[EVENTID_SIZE]=0;

  /* Sac files will be written into the directory structure
   * borrowed from ora2sac:
   *   SacDir = OutDir/yyyymm/yyyymmdd_hhmmss-iiii/
   *
   *  If SacDir does not exist, it will be created
   */

  /* tmpname = OutDir/yyyymm */
  sprintf (tmpname, "%s/%04d%02d", SAC_szOutDir, 
           tmTemp.tm_year+1900, tmTemp.tm_mon+1);

  if (CreateDir (tmpname) != EW_SUCCESS)
  {
    logit ("e", "SACPA_init: Call to CreateDir failed\n");
    return EW_FAILURE;
  }

  /* build the event directory name */
  sprintf (tmp1, "%04d%02d%02d_%02d%02d%02d_%s", 
           tmTemp.tm_year+1900, tmTemp.tm_mon+1, tmTemp.tm_mday,
           tmTemp.tm_hour, tmTemp.tm_min, tmTemp.tm_sec, 
           szCurEventID
           );

  sprintf (SacDir, "%s/%s", tmpname, tmp1); 

  if (CreateDir (SacDir) != EW_SUCCESS)
  {
    logit ("e", "SACPABase_next_ev: Call to CreateDir(%s)failed\n",
           SacDir);
    return EW_FAILURE;
  }

  /* reset file index to 0 (for support files) */
  FileIndex = 0;
        
  if(pOrigin)
  {
    memcpy(&SAC_Origin,pOrigin,sizeof(SAC_OriginStruct));
    default_sacheadp->evla=(float)(pOrigin->dLat);
    default_sacheadp->evlo=(float)(pOrigin->dLon);
    default_sacheadp->evel=(float)(pOrigin->dElev);
    default_sacheadp->evdp=(float)(pOrigin->dElev);
  }
  else
  {
    SAC_Origin.tOrigin=dEventStartTime;
  }

  return(EW_SUCCESS);
}

/****************************************************************************
*       This is the Put Away event initializer for the review system.       *
*       It doesn't do much except for setting the SacDir variable           *
*       which gets used a lot in the Sac putaway routines                   *
*****************************************************************************/
int SACPABase_next_ev_review (char *EventDir, int EventID, SAC_OriginStruct *pSacOrigin)
{

	char	evtid[256];

	if ((EventDir == NULL) || (pSacOrigin == NULL))
	{
		logit ("", "Invalid arguments passed in.\n");
		return EW_FAILURE;
	}

	/* Set SacDir */
	strcpy (SacDir, EventDir);

	/* Set szCurEventID */
	sprintf (evtid, "%d", EventID);
	strncpy (szCurEventID, evtid, EVENTID_SIZE);
	szCurEventID[EVENTID_SIZE] = 0;


	/* Set event parameters */
    memcpy (&SAC_Origin, pSacOrigin, sizeof(SAC_OriginStruct));
    default_sacheadp->evla = (float)(pSacOrigin->dLat);
    default_sacheadp->evlo = (float)(pSacOrigin->dLon);
    default_sacheadp->evel = (float)(pSacOrigin->dElev);
    default_sacheadp->evdp = (float)(pSacOrigin->dElev);
  
	return EW_SUCCESS;

}


/*****************************************************************************
 *   This is the working entry point into the disposal system. This routine  *
 *   gets called for each trace snippet which has been recovered. It gets    *
 *   to see the corresponding SNIPPET structure, and the event id            *
 *****************************************************************************/
/* Process one channel of data */
int SACPABase_next_scnl(char *szSta, char * szChan, char * szNet, char * szLoc)
{
  size_t i;

  if(bSACPABase_scnl_open)
  {
    logit("","SACPABase_next_scnl(): ERROR! SCNL record is already open,\n"
          "please call SACPABase_scnl_end() to close the existing record,\n"
          "before calling SACPABase_next_scnl() again.\n");
    return(SACPAB_SCNL_ALREADY_OPEN);
  }
    
  /* slam the default header into our working one. */
  memcpy(sacheadp,default_sacheadp,SACHEADERSIZE);

  /* Save the SCNL for use by other SAC_PA routines */
  strncpy(SAC_szSta,szSta,sizeof(SAC_szSta));
  SAC_szSta[sizeof(SAC_szSta)-1]=0;
  strncpy(SAC_szChan,szChan,sizeof(SAC_szChan));
  SAC_szChan[sizeof(SAC_szChan)-1]=0;
  strncpy(SAC_szNet,szNet,sizeof(SAC_szSta));
  SAC_szNet[sizeof(SAC_szNet)-1]=0;
  strncpy(SAC_szLoc,szLoc,sizeof(SAC_szSta));
  SAC_szLoc[sizeof(SAC_szLoc)-1]=0;
  /* Make sure spaces don't mess up file names */
  for (i = 0; i < strlen(SAC_szLoc); ++i)
      if (SAC_szLoc[i] == ' ') SAC_szLoc[i] = '-';

  /* Copy the SCNL into the header and blank bad the trailing chars */
  strcpy(sacheadp->kstnm, SAC_szSta);   /* station name */
  for (i = strlen(SAC_szSta); i < K_LEN; ++i)
    sacheadp->kstnm[i] = ' ';

  strcpy(sacheadp->kcmpnm, SAC_szChan);
  for (i = strlen(SAC_szChan); i < K_LEN; ++i)
    sacheadp->kcmpnm[i] = ' ';

  strcpy(sacheadp->knetwk, SAC_szNet);
  for (i = strlen(SAC_szNet); i < K_LEN; ++i)
    sacheadp->knetwk[i] = ' ';
  
  strcpy(sacheadp->khole, SAC_szLoc);
  for (i = strlen(SAC_szLoc); i < K_LEN; ++i)
    sacheadp->khole[i] = ' ';
  
  /* orientation of seismometer -
     determine the orientation based on the third character
     of the component name */
  switch ((int) SAC_szChan[2]) 
  {
    /* vertical component */
  case 'Z' :
  case 'z' :
    sacheadp->cmpaz = 0;
    sacheadp->cmpinc = 0;
    break;
    /* north-south component */
  case 'N' :
  case 'n' :
    sacheadp->cmpaz = 0;
    sacheadp->cmpinc = 90;
    break;
    /* east-west component */
  case 'E' :
  case 'e' :
    sacheadp->cmpaz = 90;
    sacheadp->cmpinc = 90;
    break;
    /* anything else */
  default :
    sacheadp->cmpaz = SACUNDEF;
    sacheadp->cmpinc = SACUNDEF;
    break;
  } /* switch */

  /* so far we have the default header, plus SCNL information */
  /* no time or trace info */
  bSACPABase_scnl_open=TRUE;
  
  return(EW_SUCCESS);
}


/*
 * SACPABase_write_trace: transfer trace data from TRACE_REQ buffer into
 *          SAC data section; fill in some header info.
 *          The actual writing of the file happens in SACPABase_end_scnl.
 */
int SACPABase_write_trace(TRACE_REQ * pTrace, double dGapThresh)
     /*
 * input:  pTrace   pointer to buffer for the trace data request
 *         eventId   not used here 
 */
{
  TRACE2_HEADER *wf;
  char        *msg_p;        /* pointer into tracebuf data */
  short       *s_data;
  int32_t     *l_data;
  float       *f_data;
  float       *sac_p;      /* the data part of the SAC buffer */  
  int          j;
  long         nsamp, nfill;
  long         nfill_max = 0l;
  long         nsamp_this_scnl = 0l;
  long         this_size;
  double       starttime, endtime; /* times for current scnl         */
  double       samprate;
  float        fill = (float)SACUNDEF;
  struct tm    *time;
  time_t       ltime;
  char         datatype;  /* 's' for short, 'l' for long */
  int          rc;
  char	       stime[256];
  char	       etime[256];

  /* Check validity of the arguments */
  if (pTrace == NULL)
  {
    logit ("e", "SACPABase_write_trace: Invalid arguments passed in\n");
    return (EW_FAILURE);
  }

  if ((msg_p = pTrace->pBuf) == NULL)
  {
    logit ("e", "SACPABase_write_trace: message buffer is NULL\n");
    return EW_FAILURE;
  }

  GapCount = 0;

  sac_p = (float *)(SacBuffer + SACHEADERSIZE);

  wf = (TRACE2_HEADER *) msg_p;
  rc = WaveMsg2MakeLocal(wf);
  if (rc == -1)
  {
    logit("e", "SACPABase_write_trace: unknown trace data type for first packet: %s\n",
          wf->datatype);
    logit("e", "Exiting\n");
    return( EW_FAILURE );
  }
  if (rc == -2)
  {
    logit("e", "SACPABase_write_trace: WaveMsgMakeLocal for first packet failed with error %d\n",
          rc);

    datestr23 (wf->starttime, stime, 256);
    datestr23 (wf->endtime,   etime, 256);
    logit("e", "Bad Header:\n");

    logit("e", "%d %d %s (%.4f) %s (%.4f) %.1f %s.%s.%s.%s %s %s %s %s\n",
          wf->pinno, wf->nsamp, stime, wf->starttime, etime, 
          wf->endtime, wf->samprate, wf->sta, wf->chan, wf->net, 
          wf->loc, wf->version, wf->datatype, GET_TRACE2_QUALITY(wf), GET_TRACE2_PAD(wf));

    logit("e", "Exiting\n");
    return( EW_FAILURE );
  }
    
  nsamp = wf->nsamp;
  starttime = wf->starttime;
  endtime = wf->endtime;
  samprate = wf->samprate;
  if (samprate < 0.01)
  {
    logit("et", "SACPABase_write_trace: unreasonable samplerate (%f) for <%s.%s.%s.%s>\n",
          samprate, SAC_szSta, SAC_szChan, SAC_szNet, SAC_szLoc);
    return( EW_FAILURE );
  }
  
  SAC_starttime = starttime;
  datatype = 'n';
  if (wf->datatype[0] == 's' || wf->datatype[0] == 'i')
  {
    if (wf->datatype[1] == '2') datatype = 's';
    else if (wf->datatype[1] == '4') datatype = 'l';
  }
  else if (wf->datatype[0] == 't' || wf->datatype[0] == 'f')
  {
    if (wf->datatype[1] == '4') datatype = 'f';
  }
  if (datatype == 'n')
  {
    logit("et", "SACPABase_write_trace: ffunsupported datatype: %c\n", datatype);
    return( EW_FAILURE );
  }
  
  /* loop through all the messages for this s-c-n */
  while (1) 
  {
    /* advance message pointer to the data */
    msg_p += sizeof(TRACE2_HEADER);
        
    /* check for sufficient memory in output buffer */
    this_size = (nsamp_this_scnl + nsamp ) * sizeof(float);

    if ( BufferLen < (this_size + SACHEADERSIZE) ) 
    {
      logit( "", "out of space for <%s.%s.%s.%s>; saving short trace.\n",
             SAC_szSta, SAC_szChan, SAC_szNet, SAC_szLoc);
      break;
    }

    switch( datatype )
    {
    case 's':
      s_data = (short *)msg_p;
      for ( j = 0; j < nsamp ; j++, nsamp_this_scnl++ )
        sac_p[nsamp_this_scnl] = (float) s_data[j];
      msg_p += sizeof(short) * nsamp;
      break;
    case 'l':
      l_data = (int32_t *)msg_p;
      for ( j = 0; j < nsamp; j++, nsamp_this_scnl++ )
        sac_p[nsamp_this_scnl] = (float) l_data[j];
      msg_p += sizeof(int32_t) * nsamp;
      break;
    case 'f':
      f_data = (float *)msg_p;
      for ( j = 0; j < nsamp; j++, nsamp_this_scnl++ )
        sac_p[nsamp_this_scnl] = f_data[j];
      msg_p += sizeof(float) * nsamp;
      break;
    }
    
    /* End-check based on length of snippet buffer */
    if ((size_t) msg_p >= ((size_t) pTrace->actLen + (size_t) pTrace->pBuf))
    {
      if (SACPAB_Debug == 1)
        logit ("", "Setting done for <%s.%s.%s.%s>\n", SAC_szSta, SAC_szChan, 
               SAC_szNet, SAC_szLoc);
      break;
    }

    /* msg_p has been advanced to the next TRACE_BUF; localize bytes *
     * and check for gaps.                                           */
    wf = (TRACE2_HEADER *) msg_p;

    rc = WaveMsg2MakeLocal(wf);
	if (rc == -1)
	{
	  logit("e", "SACPABase_write_trace: unknown trace data type: %s\n",
            wf->datatype);
      logit("e", "Exiting\n");
	  return( EW_FAILURE );
	}
    if (rc == -2)
	{
       logit("e", "SACPABase_write_trace: WaveMsgMakeLocal failed with error %d\n",
	         rc);
       datestr23 (wf->starttime, stime, 256);
       datestr23 (wf->endtime,   etime, 256);
       logit("e", "Bad Header:\n");

       logit("e", "%d %d %s (%.4f) %s (%.4f) %.1f %s.%s.%s.%s %s %s %s %s\n",
             wf->pinno, wf->nsamp, stime, wf->starttime, etime, 
             wf->endtime, wf->samprate, wf->sta, wf->chan, wf->net, 
             wf->loc, wf->version, wf->datatype, GET_TRACE2_QUALITY(wf), GET_TRACE2_PAD(wf));

       logit("e", "Skipping packet and continuing\n");
	   continue;
	}

    nsamp = wf->nsamp;
    starttime = wf->starttime; 
    /* starttime is set for new packet; endtime is still set for old packet */
    if ( endtime + ( 1.0/samprate ) * dGapThresh < starttime ) 
    {
      /* there's a gap, so fill it */
      nfill = (long) ((float)samprate * 
						(float) (starttime - endtime) -(float) 1.0);

      if (SACPAB_Debug == 1)
      	logit("e", "gap in %s.%s.%s.%s: %lf: %lf, fill with %ld samples\n", 
	      SAC_szSta, SAC_szChan, SAC_szNet, SAC_szLoc,
	      endtime, starttime - endtime, nfill);

      if ( (nsamp_this_scnl + nfill) * (long)sizeof(float) > BufferLen ) 
      {
        logit("e", "bogus gap (%ld); skipping\n", nfill);
        return(EW_FAILURE);
      }
      /* do the filling */
      for ( j = 0; j < nfill; j++, nsamp_this_scnl ++ ) 
        sac_p[nsamp_this_scnl] = fill;
      /* keep track of how many gaps and the largest one */
      GapCount++;
      if (nfill_max < nfill) 
        nfill_max = nfill;
    }
    /* Advance endtime to the new packet;        *
     * process this packet in the next iteration */
    endtime = wf->endtime;

  } /* while (1) */

  /*  All trace data fed into SAC data section.  Now fill in header */
  sacheadp->npts = (int32_t) nsamp_this_scnl;  /* samples in trace */
  sacheadp->delta = (float) (1.0/samprate);   /* sample period */
  sacheadp->e = (float) nsamp_this_scnl * sacheadp->delta;  /* end time */
  ltime = (time_t)SAC_starttime;
  /* gmttime makes juldays starting with 0 */
  time = gmtime( &ltime );
  sacheadp->nzyear = (int32_t)time->tm_year + (int32_t)1900; /* calendar year of reference time */
  sacheadp->nzjday = (int32_t)time->tm_yday + (int32_t)1;  /* julian day, 0 - 365 */
  sacheadp->nzhour = (int32_t)time->tm_hour;
  sacheadp->nzmin  = (int32_t)time->tm_min;
  sacheadp->nzsec  = (int32_t)time->tm_sec;
  sacheadp->nzmsec = (int32_t)((SAC_starttime - (int32_t)SAC_starttime) * 1000.0);

  /* set the origin time */
  sacheadp->o      = (float)(SAC_Origin.tOrigin - SAC_starttime);
  
  if (GapCount) 
    logit("e", "%d gaps; largest %ld for <%s.%s.%s.%s>\n",
          GapCount, nfill_max, SAC_szSta, SAC_szChan, SAC_szNet, SAC_szLoc);

  return(EW_SUCCESS);
}

int SACPABase_write_parametric(SAC_ArrivalStruct * pArrival, int WaveType)
{
  int i;

  if (pArrival == NULL)
  {
    logit ("", "Invalid parameters passed in\n");
    return EW_FAILURE;
  }

  if (WaveType == PWAVE)
  {
    for (i = 0; i < K_LEN; ++i)
      sacheadp->ka[i] = ' ';

    sacheadp->a    = (float)(pArrival->tPhase - SAC_starttime);
    sacheadp->ka[0]= 'P';
    sacheadp->ka[1]= pArrival->cFMotion;
    sacheadp->ka[2]= (char)(pArrival->iPhaseWt + '0');
    sacheadp->ka[3]= pArrival->cOnset;
    sacheadp->f    = (float)(pArrival->dCodaLen + sacheadp->a);
    sacheadp->dist = (float)(pArrival->dDist);
    sacheadp->az   = (float)(pArrival->dAzm);
  }
  else if (WaveType == SWAVE)
  {

    for (i = 0; i < K_LEN; ++i)
      sacheadp->kt0[i] = ' ';

    sacheadp->t0    = (float)(pArrival->tPhase - SAC_starttime);
    sacheadp->kt0[0]= 'S';
    sacheadp->kt0[1]= pArrival->cFMotion;
    sacheadp->kt0[2]= (char)(pArrival->iPhaseWt + '0');
    sacheadp->kt0[3]= pArrival->cOnset;
    sacheadp->f    = (float)(pArrival->dCodaLen + sacheadp->a);
    sacheadp->dist = (float)(pArrival->dDist);
    sacheadp->az   = (float)(pArrival->dAzm);
  }
  else
  {
    logit ("", "Invalid WaveType %d.\n", WaveType);
    return EW_FAILURE;
  }

  return(EW_SUCCESS);
}

int SACPABase_write_amppicks(SAC_AmpPickStruct * pAmpPick)
{

	double	tmp1, tmp2;

	if (pAmpPick == NULL)
	{
		logit ("", "Invalid arguments passed in.\n");
		return EW_FAILURE;
	}

	if (pAmpPick->PickType == 0)
	{
		/* zero-to-peak case */
		sacheadp->t0 = (float ) (pAmpPick->ZP_time - SAC_starttime);
		sacheadp->user0 = (float) (pAmpPick->ZP_amp);

		sprintf (sacheadp->kt0, "0-P_max");
		sprintf (sacheadp->kuser0, "0-P_amp");
	}
	else if (pAmpPick->PickType == 1)
	{
		/* peak-to-peak case */

		tmp1 = (pAmpPick->PPmin_time - SAC_starttime);
		tmp2 = (pAmpPick->PPmax_time - SAC_starttime);
		sacheadp->t1 = (float) tmp1;
		sacheadp->t2 = (float) tmp2;
		sacheadp->user1 = (float) (pAmpPick->PP_amp);

		sprintf (sacheadp->kt1, "P-P_min");
		sprintf (sacheadp->kt2, "P-P_max");
		sprintf (sacheadp->kuser1, "P-P_amp");
	}
	else
	{
		logit ("", "Invalid PickType value: %d\n", pAmpPick->PickType);
		return EW_FAILURE;
	}

	sprintf (sacheadp->kinst, "W_A_(mm)");

	return EW_SUCCESS;

}


int SACPABase_write_stainfo(SAC_StationStruct * pStation)
{
  char szPZFilename[256];
  FILE * fpPZ;
  int i;
  double dist, azm;
  /*
    SAMPLE POLES AND ZEROES FILE
    ============================

    ZEROS 4
    -0.125  0.0
    -50.0  0.0
    POLES 4
    -0.13 0.0
    -6.02 0.0
    -8.66 0.0
    -35.2 0.0
    CONSTANT -394.0     
  */

  sacheadp->stla=pStation->dLat;
  sacheadp->stlo=pStation->dLon;
  sacheadp->stel=pStation->dElev;


  /* 
   * This is a good time to make sure that distance and azm get set. 
   * Story: dist and azm are properties of Arrivals in the DB schema. 
   * This means that they don't get set for SAC traces without picks.
   * This is not good. So, here we cloodge it. LV 6/2001.
   */

  if ((sacheadp->evla != (float) SACUNDEF)  && (sacheadp->evlo != (float) SACUNDEF)  &&
      (sacheadp->stla != (float) SACUNDEF)  && (sacheadp->stlo != (float) SACUNDEF))
  {
        if (geo_to_km (sacheadp->evla, sacheadp->evlo,
                        sacheadp->stla, sacheadp->stlo, &dist, &azm) != 1)
        {
            logit ("", "Call to geo_to_km failed - ignoring.\n");
        }
        else 
        {
          if (sacheadp->dist == (float) SACUNDEF)
          {
             sacheadp->dist = (float) dist;
          }
          if (sacheadp->az == (float) SACUNDEF)
          {
             sacheadp->az = (float) azm;
          }
        }
  }

  if ((pStation->bResponseIsValid) && 
		((pStation->pResponse->iNumPoles > 0) || 
			(pStation->pResponse->iNumZeroes > 0)))
  {
    /* create the full SAC file name for this SCNL PZ file */
    sprintf(szPZFilename,"%s/%s.%s.%s.%s.pz",SacDir, SAC_szSta, SAC_szChan,
            SAC_szNet, SAC_szLoc);
    fpPZ=fopen(szPZFilename,"w");
    if(!fpPZ)
    {
      logit("e","SACPABase_write_stainfo():  Failed to open file: %s! Errno=%d\n",
            szPZFilename,errno);
      return(SACPAB_FOPEN_FAILED);
    }
          
    fprintf(fpPZ,"POLES %d\n",pStation->pResponse->iNumPoles);
    for(i=0;i<pStation->pResponse->iNumPoles;i++)
    {
      fprintf(fpPZ,"%f %f\n",pStation->pResponse->Poles[i].dReal,
              pStation->pResponse->Poles[i].dImag);
    }

    fprintf(fpPZ,"ZEROS %d\n",pStation->pResponse->iNumZeroes);
    for(i=0;i<pStation->pResponse->iNumZeroes;i++)
    {
      fprintf(fpPZ,"%f %f\n",pStation->pResponse->Zeroes[i].dReal,
              pStation->pResponse->Zeroes[i].dImag);
    }

    fprintf(fpPZ,"CONSTANT %e\n",pStation->pResponse->dGain);

    fclose(fpPZ);
  }  /* end if(bResponseIsValid) */
          
  return(EW_SUCCESS);
}




int SACPABase_end_scnl (void)
{
  size_t towrite;           /* bytes to write to SAC file */
  char   sacfile[2*MAXTXT]; /* the name of the sac file */
  FILE  *fp;                /* sacfile file pointer */
  int32_t   i, npts;        /* number of SAC data points */
  float *sac_p;

  if(!bSACPABase_scnl_open)
  {
    logit("e","SACPABase_end_scnl(): ERROR! There is no open SCNL record.\n");
    return(SACPAB_SCNL_NOT_OPEN);
  }

  npts = sacheadp->npts;
  sac_p = (float *)(SacBuffer + SACHEADERSIZE);
  if (strcmp (SAC_szOutputFormat, SAC_DIFFERENT_PLATFORM) == 0)
  {
    swapsac(sacheadp);
    for (i = 0; i < npts; i++)
      SwapFloat(&sac_p[i]);
  }

  /* copy the (potentially byte-swapped) SAC header
     into the SAC buffer and write it out.
  ******************************************************/
  memcpy ((void *) SacBuffer, (void *) sacheadp, SACHEADERSIZE);

  /* Update SAC file list with file name and arrival time */
  sprintf (FileList[FileIndex].filename, "%s.%s.%s.%s",  SAC_szSta, SAC_szChan, 
           SAC_szNet, SAC_szLoc);
  FileList[FileIndex].sort_param = SAC_starttime;

  if ((FileIndex = FileIndex + 1) > DB_MAX_PHS_PER_EQ)
  {
    logit ("e", "SACPABase_end_scnl: WARNING: Maximum number of phases per "
           "event (%d) exceeded.\n", DB_MAX_PHS_PER_EQ);

    FileIndex = FileIndex - 1;
  } 

  /* create the full SAC file name for this SCNL */
  sprintf(sacfile, "%s/%s", SacDir, FileList[FileIndex-1].filename);


  /* Open the sacfile */
  if ((fp = fopen (sacfile, "wb")) == NULL) 
  {
    logit("e", "SACPABase_next_scnl: cannot open %s\n", sacfile);
    return(EW_FAILURE);
  }

  /* Write the buffer to the sacfile */
  towrite = SACHEADERSIZE + sizeof(float) * npts;
  if (fwrite ((const void *) SacBuffer, towrite, 1, fp) != 1)
  {
    logit("e", "SACPABase_end_scnl: cannot write %s\n", sacfile);
    (void) unlink(sacfile);
    return EW_FAILURE;
  }
  fclose (fp);

  /* mark the SCNL closed */
  bSACPABase_scnl_open=FALSE;
  return(EW_SUCCESS);
}


int SACPABase_end_scnl_gm (void)
{
  size_t towrite;           /* bytes to write to SAC file */
  char   sacfile[2*MAXTXT]; /* the name of the sac file */
  FILE  *fp;                /* sacfile file pointer */
  int32_t   i, npts;        /* number of SAC data points */
  float *sac_p;

  if(!bSACPABase_scnl_open)
  {
    logit("e","SACPABase_end_scnl(): ERROR! There is no open SCNL record.\n");
    return(SACPAB_SCNL_NOT_OPEN);
  }

  npts = sacheadp->npts;
  sac_p = (float *)(SacBuffer + SACHEADERSIZE);
  if (strcmp (SAC_szOutputFormat, SAC_DIFFERENT_PLATFORM) == 0)
  {
    swapsac(sacheadp);
    for (i = 0; i < npts; i++)
      SwapFloat(&sac_p[i]);
  }

  /* copy the (potentially byte-swapped) SAC header
     into the SAC buffer and write it out.
  ******************************************************/
  memcpy ((void *) SacBuffer, (void *) sacheadp, SACHEADERSIZE);


  /* create the full SAC file name for this SCNL */
  sprintf(sacfile, "%s/%s.gm", SacDir, FileList[FileIndex-1].filename);


  /* Open the sacfile */
  if ((fp = fopen (sacfile, "wb")) == NULL) 
  {
    logit("e", "SACPABase_next_scnl: cannot open %s\n", sacfile);
    return(EW_FAILURE);
  }

  /* Write the buffer to the sacfile */
  towrite = SACHEADERSIZE + sizeof(float) * npts;
  if (fwrite ((const void *) SacBuffer, towrite, 1, fp) != 1)
  {
    logit("e", "SACPABase_end_scnl: cannot write %s\n", sacfile);
    (void) unlink(sacfile);
    return EW_FAILURE;
  }
  fclose (fp);

  /* mark the SCNL closed */
  bSACPABase_scnl_open=FALSE;
  return(EW_SUCCESS);
}


/************************************************************************
*       This is the Put Away end event routine. It's called after we've     *
*       finished processing one event.                                  *
*************************************************************************/
int SACPABase_end_ev (void)
{

  if (SACPAB_Debug == 1)
    logit ("", "In SACPA_end_ev\n");

  qsort ((void *) FileList, (size_t) FileIndex, 
         sizeof (SACFileListStruct), SAC_Compare);

  /* We haven't processed an event - something's fishy! */
  if (!strcmp(szCurEventID,"-1"))
  {
    logit ("e", "SACPA_end_ev: CurEventId is invalid - don't know how to proceed\n");

    return EW_FAILURE;
  }


  if (CreateSACSupportFiles (szCurEventID, FileList, FileIndex) != EW_SUCCESS)
  {
    logit ("e", "SACPABase_write_trace: Call to CreateSACSupportFiles failed\n");
    return EW_FAILURE;
  }

  return(EW_SUCCESS);
}


/************************************************************************
*       This is the Put Away close routine. It's called after when      *
*       we're being shut down.                                          *
*************************************************************************/
int SACPABase_close (void)
{

  free ((char *) SacBuffer);
  if (!bSACPABase_init)
  {
    logit("","SACPABase_close():  Error, SACPABase is not initialized!\n");
    return(SACPAB_NOT_INIT);
  }
  bSACPABase_init = FALSE;
  return EW_SUCCESS;
}


void sacinit( struct SAChead *head)
{
  int i;
/******************************************************************************
 *                                                                            *
 * SAChead2 uses arrays to overlay the fields in SAChead to make it easier to *
 * initialize them.  This presumption is specifically not provided for in the *
 * C  standard,  and  is therefore non-conforming, i.e., neither portable nor *
 * reliable.  See include/sachead.h.                                          *
 *                                                                            *
 ******************************************************************************/
  struct SAChead2 *head2;   /* use a simple structure here - we don't care what
                             * the variables are - set them to 'undefined' */

  /* change to a simpler format */
  head2 = (struct SAChead2 *) head;

  /*    set all of the floats to 'undefined'    */
  for (i = 0; i < NUM_FLOAT; ++i) head2->SACfloat[i] = SACUNDEF;
  /*    set all of the ints to 'undefined'      */
  for (i = 0; i < MAXINT-5; ++i) head2->SACint[i] = SACUNDEF;
  /*    except for the logical integers - set them to 1 */
  for ( ; i < MAXINT; ++i) head2->SACint[i] = 1;
  /*    set all of the strings to 'undefined'   */
  for (i = 0; i < MAXSTRING; ++i) (void) strncpy(head2->SACstring[i],
                                                 SACSTRUNDEF,K_LEN);
  /* Special case for kevnm[KEVNMLEN]: kevnm is 16 bytes long, not 8        */
  /* The code above writes "-12345  -12345  " instead of "-12345          " */
  strncpy( head->kevnm, SACSTRUNDEF_2, KEVNMLEN );

  /*    SAC I.D. number */
  head2->SACfloat[9] = SAC_I_D;
  /*    header version number */
  head2->SACint[6] = SACVERSION;

  return;       /* done */
}

/*
 * swapsac: swap all the numeric fields in the SAC header.
 */
void swapsac(struct SAChead *head)
{
  int i;
  struct SAChead2 *head2; /* use a simple structure here - we don't care what
                           * the variables are - just swap their bytes! */

  /* change to a simpler format */
  head2 = (struct SAChead2 *) head;

  /*    swap all of the floats  */
  for (i = 0; i < NUM_FLOAT; ++i) 
    SwapFloat(&head2->SACfloat[i]);
  
  /*    swap all of the ints   */
  for ( i = 0; i < MAXINT; ++i) 
    SwapInt32( &(head2->SACint[i]));

  return;
}

/**************************************************
 *  SAC_Compare()  compares 2 values              *
 *  This function is passed to qsort()            *
 **************************************************/
int SAC_Compare (const void *p1, const void *p2)
{
  SACFileListStruct *srt1 = (SACFileListStruct *) p1;
  SACFileListStruct *srt2 = (SACFileListStruct *) p2;

  if (srt1->sort_param < srt2->sort_param)   
    return (-1);

  if (srt1->sort_param > srt2->sort_param)
    return (1);

  return (0);
}




/* SACSupportFiles 
 *
 * Write simple SAC macros to disk files in current working directory, 
 * given a list of filenames sorted in some order.
 *
 * Also write other files necessary for relocating the event
 * with hypoinverse and for updating database.
 * July 1998, LDD
 */

/* how many traces do we want in the quicklook macro */
#define         NQUICKLOOK      10

/* Support file names */
#define         INIT_FNAME      "init"
#define         QUICKLOOK_FNAME "quicklook"
#define         REPICK_FNAME    "repick"
#define         SACLIST_FNAME   "saclist"

/*******************************************************
 * CreateSACSupportFiles() writes all the desired SAC macros    
 *  and Hypoinv files to the current working directory 
 *******************************************************/
int CreateSACSupportFiles (char * szEventid, SACFileListStruct *pfl, int nfile)
{
  int nlook;

  if (SACmac_init() != EW_SUCCESS)
  {
    logit ("e", "Call to SACmac_init failed\n");
    return EW_FAILURE;
  }

  nlook = (nfile < NQUICKLOOK) ? nfile : NQUICKLOOK;
  if (SACmac_quicklook (pfl, nlook) != EW_SUCCESS)
  {
    logit ("e", "Call to SACmac_quicklook failed\n");
    return EW_FAILURE;
  }


  if (SACmac_repick (pfl, nfile) != EW_SUCCESS)
  {
    logit ("e", "Call to SACmac_repick failed\n");
    return EW_FAILURE;
  }

  if (SAC_filelist (pfl, nfile, szEventid) != EW_SUCCESS)
  {
    logit ("e", "Call to SAC_filelist failed\n");
    return EW_FAILURE;
  }

  return (EW_SUCCESS);

}

/**************************************************
 * SACmac_init() writes a simple initialization   *
 *  macro to the current working directory in a   *
 *  file named "init"                             *
 **************************************************/
int SACmac_init (void)
{
  FILE *fp;
  char filename[256];

  sprintf (filename, "%s/%s", SacDir, INIT_FNAME);

  if ((fp = fopen (filename, "w+")) == NULL)
  {
    logit ("e", "SACmac_init: cannot open macro file %s; %s\n",
           filename, strerror (errno));
    return (EW_FAILURE);
  }

  fprintf (fp, "qdp 1000\n"
           "window 1 x 0 1 y 0.35 0.97\n"
           "bd X\n"
           "bw 1\n" );
  fclose (fp);

  return (EW_SUCCESS);
}


/**************************************************
 * SACmac_quicklook() writes a macro to bring up  *
 *  the first nlook files in the list.  The list  *
 *  should be sorted before invoking this funtion *
 **************************************************/
int SACmac_quicklook (SACFileListStruct *pfl, int nlook)
{
  FILE *fp;
  int i;
  char filename[256];

  sprintf (filename, "%s/%s", SacDir, QUICKLOOK_FNAME);

  if ((fp = fopen (filename, "w+")) == NULL)
  {
    logit("e", "SACmac_quicklook: cannot open macro file %s; %s\n",
          filename, strerror (errno));
    return (EW_FAILURE);
  }

  fprintf (fp, "qdp 500\n"
           "fileid type list kevnm kstcmp\n"
           "r ");

  for (i = 0; i < nlook; i++ ) 
    fprintf (fp, "%s ", pfl[i].filename);

  fprintf (fp, "\n"
           "p1\n");

  fclose (fp);

  return (EW_SUCCESS);
}


/**************************************************
 * SACmac_repick() writes a macro to bring up     *
 *  each trace for this event to repick it        *
 **************************************************/
int SACmac_repick (SACFileListStruct *pfl, int nfile)
{
  FILE *fp;
  int i;
  char filename[256];

  sprintf (filename, "%s/%s", SacDir, REPICK_FNAME);

  if ((fp = fopen (filename, "w+")) == NULL)
  {
    logit ("e", "SACmac_repick: cannot open macro file %s; %s\n",
           filename, strerror (errno));
    return (EW_FAILURE);
  }

  fprintf (fp, "qdp 500\n"
           "fileid type list kevnm kstcmp\n");

  fprintf (fp, "r %s\n" 
           "ppk p off b off\n"  
           "wh\n",
           pfl[0].filename);  /* Turn off that darn bell! */
  
  for (i = 1; i < nfile; i++) 
  {  
    fprintf (fp, "r %s\n" 
             "ppk p off\n"
             "wh\n",
             pfl[i].filename);
  }
  fclose (fp);

  return (EW_SUCCESS);
}


/**************************************************
 * SAC_filelist() writes a file containing the    *
 *  names of all the SAC files in the directory   *
 **************************************************/
int SAC_filelist (SACFileListStruct *pfl, int nfile, char * szEventid)
{
  FILE *fp;
  int i;
  char filename[256];

  sprintf (filename, "%s/%s", SacDir, SACLIST_FNAME);

  if ((fp = fopen (filename, "w+")) == NULL)
  {
    logit ("e", "SACmac_filelist: cannot open file %s; %s\n",
           filename, strerror (errno));
    return (EW_FAILURE);
  }

  fprintf (fp, "EVENTID:%s\n", szEventid);

  for (i = 0; i < nfile; i++) 
  {  
    fprintf (fp, "%s\n", pfl[i].filename);
  }

  fclose (fp);

  return (EW_SUCCESS);
}

int SAC_SwapInt32(int32_t * pValue, char cDataType)
{
#if defined (_SPARC)
  if( cDataType == 'i' || cDataType == 'f' )
    SwapInt32(pValue);
  return(EW_SUCCESS);
#elif defined (_INTEL)
  if( cDataType == 's' || cDataType == 't' )
    SwapInt32(pValue);
  return(EW_SUCCESS);
#else
  logit("e","SAC_SwapInt32():Error! Unable to determine platform! "
        "Please compile w/ _INTEL or _SPARC!\n");
  return(SACPAB_UNKNOWN_PLATFORM);
#endif
}

int SAC_SwapShort(short * pValue, char cDataType)
{
#if defined (_SPARC)
  if( cDataType == 'i' || cDataType == 'f' )
    SwapShort(pValue);
  return(EW_SUCCESS);
#elif defined (_INTEL)
  if( cDataType == 's' || cDataType == 't' )
    SwapShort(pValue);
  return(EW_SUCCESS);
#else
  logit("e","SAC_SwapShort():Error! Unable to determine platform! "
        "Please compile w/ _INTEL or _SPARC!\n");
  return(SACPAB_UNKNOWN_PLATFORM);
#endif
}

int SAC_SwapDouble(double * pValue, char cDataType)
{
#if defined (_SPARC)
  if( cDataType == 'i' || cDataType == 'f' )
    SwapDouble(pValue);
  return(EW_SUCCESS);
#elif defined (_INTEL)
  if( cDataType == 's' || cDataType == 't' )
    SwapDouble(pValue);
  return(EW_SUCCESS);
#else
  logit("e","SAC_SwapDouble():Error! Unable to determine platform! "
        "Please compile w/ _INTEL or _SPARC!\n");
  return(SACPAB_UNKNOWN_PLATFORM);
#endif
}


/* Write urban hazards external info into the header */
int SACPABase_write_extinfo (SAC_ExtChanStruct *pExtChan)
{

	if (pExtChan == NULL)
	{
		logit ("", "SACPABase_write_extinfo: Invalid arguments passed in\n");
		return (EW_FAILURE);
	}


	sacheadp->user0 = (float) pExtChan->iGain;
	sacheadp->user1 = (float) pExtChan->dFullscale;
	sacheadp->user2 = (float) pExtChan->dSensitivity;
	sacheadp->user3 = (float) pExtChan->dDamping;
	sacheadp->user4 = (float) pExtChan->dNaturalFrequency;
	sacheadp->user5 = (float) pExtChan->iSensorType;

	sacheadp->cmpaz = (float) pExtChan->dAzm;
	sacheadp->cmpinc = (float) pExtChan->dDip;

	return(EW_SUCCESS);

}
