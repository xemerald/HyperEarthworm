/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: rw_mag.h 6353 2015-05-13 23:21:42Z et $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.17  2006/06/06 20:21:45  paulf
 *     upgraded for location codes
 *
 *     Revision 1.16  2002/09/10 17:20:03  dhanych
 *     Stable scaffold
 *
 *     Revision 1.15  2002/06/10 16:17:58  lucky
 *     Added mblg and changed mb label to Mb (per Harley)
 *
 *     Revision 1.14  2001/10/02 17:29:34  lucky
 *     fixed doubles to floats of amplitudes and periods.
 *
 *     Revision 1.13  2001/08/07 16:49:27  lucky
 *     Pre v6.0 checkin
 *
 *     Revision 1.12  2001/05/30 15:52:22  lucky
 *     Added a comment/warning about the order of magnitude types array
 *     and the associated string->number mappings.
 *
 *     Revision 1.11  2001/05/29 21:24:09  lucky
 *     Changed order of MagNames to comply with what is in the database.
 *     Note: Mag types ***MUST*** remain in the same order and with the same
 *     numbers, otherwise the database thigies break.
 *
 *     Revision 1.10  2001/05/26 21:07:42  lombard
 *     Changed ML_INFO struct to MAG_CHAN_INFO struct to make it more generic.
 *     This structure should now work with most mag types.
 *     Added Mwp, removed M0 from mag type table.
 *     Changed some MAG_*_LEN macros to TRACE_*_LEN for consistency.
 *     Added period (double) element to MAG_CHAN_INFO struct for future
 *     use with body and surface wave magnitudes.
 *
 *     Revision 1.9  2001/05/22 18:45:33  lucky
 *     Fixed the MagNames initialization to properly include Md. Also, had to increase
 *     N_MAG_NAMES to 7 -- used to be 6, but there were seven entries??
 *
 *     Revision 1.8  2001/05/21 22:30:11  davidk
 *     Added MAGTYPE constants and a duration magtype to the magtype table.
 *
 *     Revision 1.5  2001/05/01 22:39:31  davidk
 *     moved the MagNames table out of the .h file and into rw_mag.c
 *
 *     Revision 1.4  2001/05/01 20:27:13  davidk
 *     fixed syntax errors in an array declaration and in the rd_mag_sta() prototype.
 *
 *     Revision 1.3  2001/04/29 00:10:53  alex
 *     alex: tweak for table of mag type strings and numbers
 *
 *     Revision 1.2  2001/04/06 18:04:26  lombard
 *     added <earthworm.h> since it is needed for some macros.
 *
 *     Revision 1.1  2001/04/05 15:40:09  lombard
 *     Initial revision
 *
 *
 *
 */

/* rw_mag.h
 *
 * Header file for the functions in rw_mag.c that
 * convert from a TYPE_MAGNITUDE message to a structure 
 * and visa versa.
 * 
 * Each TYPE_MAGNITUDE message is intended to contain information
 * for one event.
 *
 * Timestamps are in seconds since 1970/01/01 00:00:00.00 UTC.
 *
 * written by Pete Lombard  February 2001
 */

#ifndef RW_MAG_H
#define RW_MAG_H

#include <trace_buf.h>
#include <earthworm_defs.h> /* for mag types */

/* Define maximum lengths for strings/arrays 
   in and Earthworm TYPE_MAGNITUDE message
 *********************************************/
#define MAG_ALG_LEN     8
#define MAG_NAME_LEN    6

/* Message-data strings used by 'rd_nomag_msg()' and  'wr_nomag_msg()' */
#define NO_MAGAVAIL_MSTR "No local magnitude available"
#define EVENT_ID_MSTR "eventId"
#define MAG_TYPE_MSTR "magType"
#define MAG_TIDX_MSTR "magTypeIdx"

/* Structure to contain magnitude summary information for one event
   In the comments below, NTS = Null Terminated String
 *********************************************************/
typedef struct _MAG_INFO {

/* fields supplied in first (summary) line of TYPE_MAGNITUDE msg */
  double  mag;            /* REQUIRED: magnitude value */
  double  error;          /* OPTIONAL: Error estimate (std deviation for Ml/AVG) */
  double  quality;        /* OPTIONAL: [0.0 - 1.0], -1.0 for NULL                */
  double  mindist;        /* OPTIONAL: Minumun distance from location to station *
                           *   used in mag.[0.0 - d], -1.0 for NULL              */
  int     azimuth;        /* OPTIONAL: Maximum azimuthal gap for stations *
                           *   used in mag. [0 - 360], -1 for NULL        */
  int     nstations;      /* OPTIONAL: Number of stations used to compute magnitude. */
  int     nchannels;      /* OPTIONAL: Number of data channels used to compute magnitude. */
  char    qid[EVENTID_SIZE];  /* REQUIRED: NTS, eventid data associates with    */
  char    qauthor[AUTHOR_FIELD_SIZE]; /* REQUIRED: NTS, author of the eventid   *
                                         *   (required if qid is given)           */
  unsigned int origin_version;
  unsigned int qdds_version;
  int     imagtype;                /* REQUIRED: Magnitude type from MagNames table above */
  char    szmagtype[MAG_NAME_LEN];   /* Magnitude type string */

  char    algorithm[MAG_ALG_LEN];  /* OPTIONAL: NTS, AVG for average, *
                                    *   MED for median, `_' for null) */
/* fields supplied by DBMS: */
   double  tload;         /* time data was loaded into DBMS - this field is *
                           *   not part of ascii TYPE_MAG msg,              *
                           *   but may be filled when pulling data from DBMS*/
  char *pMagAux;          /* pointer to auxiliary structure(s) specific for *
                           *   the magnitude type                           */
  size_t  size_aux;       /* Size of memory used for MagAux structure       */
} MAG_INFO;

/* Structure to contain generic magnitude information for one channel *
 * This structure is expected to be used for most magnitude types,    *
 * it consists of two pick times, amplitudes, and periods.
 * Moment magnitude is an obvious exception to this.                  */
typedef struct _MAG_CHAN_INFO {
   char    sta[TRACE_STA_LEN];    /* REQUIRED: NTS, Site code as per IRIS SEED      */  
   char    comp[TRACE_CHAN_LEN];  /* REQUIRED: NTS, Component code as per IRIS SEED */  
   char    net[TRACE_NET_LEN];    /* REQUIRED: NTS, Network code as per IRIS SEED   */
   char    loc[TRACE_LOC_LEN];    /* REQUIRED: NTS, Location code as per IRIS SEED   */
  double   mag;           /* REQUIRED: local magnitude for this channel */
  double   dist;          /* REQUIRED: station-event distance used for local magnitude */
  double   corr;          /* REQUIRED: correction that was added to get this local mag */
  double   Time1;         /* time of the first pick */
  float    Amp1;          /* amplitude of the first pick */
  float    Period1;       /* period associated with the first pick */
  double   Time2;         /* time of the second pick (if used) */
  float    Amp2;    	  /* amplitude of the second pick (if used) */
  float    Period2;       /* period of the second pick (if used) */
} MAG_CHAN_INFO;


/* Function Prototypes
 *********************/
int rd_mag( char *msg, int msglen, MAG_INFO *pMag );
int rd_chan_mag( char *msg, int msglen, MAG_CHAN_INFO *pMci, int size_ml);
int wr_mag( MAG_INFO *pMag, char *buf, int buflen );
int wr_mag_scnl( MAG_INFO *pMag, char *buf, int buflen );
int rd_nomag_msg(const char *msg, char *eventIdStr, char *magTypeStr,
                                              int *pMagTypeIdx, int maxLen);
void wr_nomag_msg(char *buf, const char *eventIdStr, const char *magTypeStr,
                                                            int magTypeIdx);

/* nothing should really call this next one as it is an internal function */
int wr_chan_mag( MAG_CHAN_INFO *pMci, int nchannels, char *buf, int buflen , int scnl_flag); 

/* Alex's crude parsers
***********************/
/* reads the summary line of a mag message */
int rd_mag_sum(char* magMsg, int msgSize, MAG_INFO* magSum);
	
/* reads one mag message channel line */
int rd_mag_sta(char* magMsg, int msgSize, int nxtChar, MAG_CHAN_INFO* magSta);


#endif
