/* THIS FILE IS UNDER CVS - DO NOT MODIFY UNLESS YOU CHECKED IT OUT!
 *
 *  $Id: rw_coda_aav.h 3595 2009-05-19 22:21:12Z dietz $
 * 
 *  Revision history:
 *   $Log$
 *   Revision 1.1  2009/05/19 22:21:12  dietz
 *   Header file for reading/writing coda avg absolute amplitude messages.
 *
 */

#ifndef _RW_CODA_AAV_H
#define _RW_CODA_AAV_H

#include <trace_buf.h>

#define MAX_CODA_AAV_LEN  72    /* max length of ascii coda_aav msg */   
/* s.c.n.l:23+time:18+time:18+amp:7+complete:4+spaces:4+cr:1+null:1 */
/* Example:                                                         */
/* MCB.HHZ.NC.-- 20090415170304.000 20090415170305.990 287 1.00     */

/* Coda AAV structure
 ********************/
typedef struct _CAAV {
   double tstart;               /* start time of the AAV window      */
   double tend;                 /* end time of the AAV window        */
   int    amp;                  /* average absolute value of rdat    */
   float  completeness;         /* fraction of expected data points  */ 
                                /* used in computing this aav value  */
} CAAV;

/* SCNL Coda AAV structure: everything needed for TYPE_CODA_AAV msg
   In the comments below, NTS = Null Terminated String 
 ******************************************************************/
typedef struct _SCNL_CAAV {
   char  site[TRACE2_STA_LEN];  /* NTS: Site code as per IRIS SEED      */
   char  net[TRACE2_NET_LEN];   /* NTS: Network code as per IRIS SEED   */
   char  comp[TRACE2_CHAN_LEN]; /* NTS: Component code as per IRIS SEED */
   char  loc[TRACE2_LOC_LEN];   /* NTS: location code as per IRIS SEED  */
   CAAV  caav;                  /* one coda avg absolute value          */
} SCNL_CAAV;            

/* Function Prototypes
 *********************/
int  rd_coda_aav( char *msg, SCNL_CAAV *ch_aav );
int  wr_coda_aav( char *msg, int maxlen, SCNL_CAAV *ch_aav );
void log_coda_aav( SCNL_CAAV *ch_aav );

#endif
