/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: Sac2DBEvent.h 515 2001-04-12 03:08:49Z lombard $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.6  2001/04/12 03:08:49  lombard
 *     added sacputaway.h header
 *
 *     Revision 1.5  2001/03/19 17:20:48  lucky
 *     Added RCS headers
 *
 * 
 */

#ifndef SAC2DBEVENT_H
#define SAC2DBEVENT_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <trace_buf.h>
#include <ewdb_ora_api.h>
#include <sacputaway.h>
#include <db_event_info.h>

#define 	MAX_SNIPPET_LEN 	100000
#define     MAX_SAC_DATA        5000000     /* SAC data cannot be bigger than this */



/*  Top level functions
***************************************************/
int 	WriteSAC_init (char * szOutDir,char * szOutputFormat, int bDebug);
int 	WriteSAC_shutdown (void);
int 	WriteSAC_Event_BAD (DBEventInfoStruct * pEvent, int);
int		SacHeaderInit (struct SAChead *);
int		Sac2DBEvent (DBEventInfoStruct *, struct SAChead *, char *, int);



/* For more advanced use, use the following functions possibly
    in conjunction with the SACPA and/or SACPABase routines in
    sacputaway.c
***************************************************/
int WriteSAC_StartEvent(DBEventInfoStruct * pEvent);
int WriteSAC_EndEvent(void);
int WriteSAC_NextStationForEvent(DBChannelDataStruct * pChannel);
int ProduceSAC_NextStationForEvent(DBChannelDataStruct * pChannel);


/* Utility Routine for setting waveform Gap Threshhold 
***************************************************/

int WriteSAC_Set_dGapThresh(float IN_dGapThresh);


#endif
