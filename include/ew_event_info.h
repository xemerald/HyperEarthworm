/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: ew_event_info.h 7113 2018-02-14 21:59:53Z baker $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.2  2008/12/23 04:10:24  stefan
 *     improved permissions?
 *
 *     Revision 1.1  2008/12/02 21:37:42  paulf
 *     oracle surgery additions
 *
 *     Revision 1.11  2004/08/12 22:48:19  davidk
 *     Added prototype for CleanupEWEvent(), which deallocates memory from 
 *     and resets pointers in *     an EWEventInfoStruct.
 *
 *     Revision 1.10  2004/06/10 18:51:11  davidk
 *     There were two version of this file, the other was in ew/include.  I am 
 *     updating this
 *     file with lucky's changes from 10/2003, and I deleted the ew/include file.
 *
 *     Revision 1.2  2003/10/01 20:09:40  lucky
 *     Increased MAX_ORIGINS_PER_EVENT to 50, and MAX_MAGS_PER_ORIGIN to 15
 *
 *     Revision 1.9  2002/06/28 21:06:22  lucky
 *     Lucky's pre-departure checkin. Most changes probably had to do with bug fixes
 *     in connection with the GIOC scaffold.
 *
 *     Revision 1.8  2002/05/28 17:24:05  lucky
 *     *** empty log message ***
 *
 *     Revision 1.7  2002/03/22 18:23:18  lucky
 *     Added retrieval of unpicked snippets on demand to speed things up
 *
 *     Revision 1.5  2001/09/26 21:43:40  lucky
 *     Added support for multi-screen event displays.
 *
 *     Revision 1.4  2001/09/10 20:47:16  lucky
 *     Added station grouping of channels.
 *
 *     Revision 1.3  2001/07/01 21:55:16  davidk
 *     Cleanup of the Earthworm Database API and the applications that utilize it.
 *     The "ewdb_api" was cleanup in preparation for Earthworm v6.0, which is
 *     supposed to contain an API that will be backwards compatible in the
 *     future.  Functions were removed, parameters were changed, syntax was
 *     rewritten, and other stuff was done to try to get the code to follow a
 *     general format, so that it would be easier to read.
 *
 *     Applications were modified to handle changed API calls and structures.
 *     They were also modified to be compiler warning free on WinNT.
 *
 *     Revision 1.2  2001/06/26 17:35:34  lucky
 *     State of the code after the Utah specs have been met
 *
 *     Revision 1.1  2001/05/21 22:31:49  davidk
 *     Initial revision
 *
 *     Revision 1.10  2001/04/17 16:34:07  davidk
 *     Added prototypes for InitEWEvent(), InitEWChan(), and EWEvent2ArcMsg(),
 *     in order to get rid of compiler warnings on NT.
 *
 *     Revision 1.9  2001/03/19 17:20:48  lucky
 *     Added RCS headers
 *
 * 
 */

#ifndef EW_EVENT_INFO_H
# define EW_EVENT_INFO_H


/********************************************************************
 *                          !!!WARNING!!!  
 * This file includes the main header file from the EWDB source tree
 ********************************************************************/
#include "ewdb_ora_api.h"
#include "rw_mag.h"
#include "earthworm_simple_funcs.h" /* logit() */

/* MAX_DATA_PER_CHANNEL_PER_EVENT (MDPCPE) */
#define MDPCPE 6

/* Constant used for allocating Channel structs */
/* How many Chan structures do we allocate initially? */
#define     INIT_NUM_CHANS      150
/* number of chans allocated subsequently */
#define     CHAN_ALLOC_INCR     50


/* GetEWEventInfo_Super() event information type constants */
/* bitwise OR these constants together and pass them as 
   the "Flags" param to GetEWEventInfo_Super() in order
   to control what types of information is retrieved for
   an event.
************************************************************/
#define GETEWEVENTINFO_SUMMARYINFO    0x01
#define GETEWEVENTINFO_PICKS          0x02
#define GETEWEVENTINFO_STAMAGS        0x04
#define GETEWEVENTINFO_COMPINFO       0x08
#define GETEWEVENTINFO_COOKEDTF       0x10
#define GETEWEVENTINFO_STRONGMOTION   0x20
#define GETEWEVENTINFO_WAVEFORM_DESCS 0x40
#define GETEWEVENTINFO_WAVEFORMS      0x80

#define		MAX_ORIGINS_PER_EVENT	50
#define		MAX_MAGS_PER_ORIGIN	  15	
#define		MAX_CHANS_PER_STATION	24
#define 	MAX_NUM_STATIONS		10000


typedef struct _EWChannelDataStruct
{
  EWDBid                      idChan;
  EWDB_StationStruct          Station;

  EWDB_WaveformStruct         Waveforms[MDPCPE];
  int                         iNumWaveforms;

  EWDB_ArrivalStruct          Arrivals[MDPCPE];
  int                         iNumArrivals;

  EWDB_StationMagStruct       Stamags[MDPCPE];
  int                         iNumStaMags;

  EWDB_TriggerStruct          Triggers[MDPCPE];
  int                         iNumTriggers;

  EWDB_ChanTCTFStruct         ResponseInfo;
  int                         bResponseIsValid;
} EWChannelDataStruct;


typedef struct _EWStationDataStruct
{
	char	Sta[10];
	char	Net[10];
	int		iNumChans;
	float	Lat;
	float	Lon;
	float	Elev;
	double	Dist;
	double	Azm;
	EWChannelDataStruct	*Chans[MAX_CHANS_PER_STATION];
	
} EWStationDataStruct;


typedef struct _EWEventInfoStruct
{
  EWDB_EventStruct      Event;
  EWDB_CoincEventStruct	CoincEvt;
  EWDB_OriginStruct     PrefOrigin;
  EWDB_MagStruct        Mags[MAX_MAGS_PER_ORIGIN];
  int         			iPrefMag;  /* Which of the magnitudes is preferred */
  int                   iNumMags;
  int         			iML;       /* Which of the magnitudes is ML */
  int         			iMd;       /* Which of the magnitudes is Md */
  EWStationDataStruct   *pStaInfo;
  int                   iNumStas;

/* This many EWChannelDataStructs have been allocated */
/* at pChanInfo -- NOTE: this value is set in GetEWEventInfo */
/* and should be reset if any memory is freed later */ 
  EWChannelDataStruct   *pChanInfo;
  int                   iNumChans;
  int                   iNumAllocChans; 

  /* Set in GetDBInfo -- tells us what we have retrieved */
  int                   GotLocation; 
  int                   GotTrigger; 
} EWEventInfoStruct;



/* Coincidence Events */

typedef struct _ChanTriggerStruct
{
	EWDB_StationStruct 			Station;
	EWDB_TriggerStruct			Trigger;
	double 						tSnippetStart;
	int							iSnippetDuration;
} EWChanTriggerStruct;

typedef struct _CoincTriggerStruct
{
	EWDB_EventStruct		Event;
	EWDB_CoincEventStruct	CoincEvt;
	char					sAuthor[256];
	char					sEventID[256];
	int						iNumTrigs;
	int						iNumAlloc;
	EWChanTriggerStruct		*pChanTrigs;
} EWCoincEvtStruct;


/* Review Flags */

#define     ACT_GETFROMDB       100
#define     ACT_GETUNPICKED     150
#define     ACT_RELOCATE        200
#define     ACT_ALARMS_YES      300
#define     ACT_ALARMS_NO       400
#define     ACT_NULL            500
#define     ACT_OVERRIDE        600
#define     ACT_REFRESH         700
#define     ACT_INSERT          800
#define     ACT_DELETE          900


int read_triglist (char *msg, long msgSize, EWCoincEvtStruct *pCoinc);
int t_atodbl(char* YYYYMMDD, char* HHMMSS, double* starttime);


/* Function prototypes
 *********************/

/* functions in init_ewevent.c */
int   InitEWEvent (EWEventInfoStruct *pEventInfo);
int   InitEWChan (EWChannelDataStruct *pChan);
int   CleanupEWEvent(EWEventInfoStruct *pEventInfo);

/* functions in arc_2_ewevent.c */
int   EWEvent2ArcMsg (EWEventInfoStruct *, char *, int);
int   ArcMsg2EWEvent (EWEventInfoStruct *pEWEvent, char *pArc, int ArcLen);

#endif /* EW_EVENT_INFO_H */
