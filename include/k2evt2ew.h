/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: k2evt2ew.h 7206 2018-03-21 21:19:16Z baker $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.4  2003/01/08 18:08:33  davidk
 *     Increased the MAXTRACELTH constant to 800k from 20k, in order to support
 *     EVT files collected by urban hazards program @ Golden.
 *
 *     Revision 1.3  2002/11/03 00:11:02  lombard
 *     Protected from multiple includes.
 *
 *     Revision 1.2  2002/08/16 20:49:29  alex
 *     fix for checksums and sanity
 *     Alex
 *
 *     Revision 1.1  2002/03/22 19:53:47  lucky
 *     Initial revision
 *
 *
 */

#ifndef K2EVT2EW_H
#define K2EVT2EW_H

#include "kwhdrs_ew.h"		/* Kinemetrics header definitions */
#include "rw_strongmotionII.h"

#define	MAX_CHANS_PER_BOX	24
#define	MAX_SM_PER_BOX		20
/* DK 2003/01/08  MAXTRACELTH changed to 800000 in order to
   support 4000 second (200 s/s) evt file for Urban Hazards
   programs at Golden.  Note that this change increases the
   VM footprint size of the K2InfoStruct to approx 150MB!!!!
 ***********************************************************/
#define MAXTRACELTH 	800000
#define	MAX_REC		4096	/* largest "length" + "dataLength" in any tag */


#define SM_BOX_LEN       25	/* maximum length of a box name               */
#define SM_MAX_CHAN      48	/* max number chans on one strongmotion box   */

/* Structure Definitions
 ***********************/
typedef struct _k2info
{
	KFF_TAG		tag;
	MW_HEADER	head;
	FRAME_HEADER	frame;
	SM_INFO		sm[MAX_SM_PER_BOX];
	float		Databuf[MAX_CHANS_PER_BOX][MAXTRACELTH];
	int32_t		Counts[MAX_CHANS_PER_BOX][MAXTRACELTH];
	int		numDataPoints[MAX_CHANS_PER_BOX];
} K2InfoStruct;

typedef struct _CHANNELNAME_ {
   char box[SM_BOX_LEN];      /* Installation-assigned box name (or serial#) */
   int  chan;             /* Channel number on this box                  */
   char sta[TRACE_STA_LEN];   /* NTS: Site code as per IRIS SEED */
   char comp[TRACE_CHAN_LEN]; /* NTS: Channel/Component code as per IRIS SEED*/
   char net[TRACE_NET_LEN];   /* NTS: Network code as per IRIS SEED */
   char loc[TRACE_LOC_LEN];   /* NTS: Location code as per IRIS SEED */
} CHANNELNAME;

int k2evt2ew (FILE *fp, char *fname, K2InfoStruct *pk2info, CHANNELNAME *pChanName,
                            int numChans, char *NetCode, int Debug);

#endif
