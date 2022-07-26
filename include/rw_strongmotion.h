
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: rw_strongmotion.h 5678 2013-07-27 12:56:17Z paulf $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.3  2001/02/06 18:31:40  dietz
 *     added definition of GRAVITY_CGS
 *
 *     Revision 1.2  2000/11/03 18:16:01  dietz
 *     Changed SM_MAX_CHAN from 6 to 18 (to accommodate K2 data)
 *
 *     Revision 1.1  2000/02/14 20:05:54  lucky
 *     Initial revision
 *
 *
 */

/* rw_strongmotion.h
 *
 * Header file for the functions in rw_strongmotion.c that
 * convert from a TYPE_STRONGMOTION message to a structure 
 * and visa versa.
 * 
 * Each TYPE_STRONGMOTION message is intended to contain information
 * from all the channels of one physical field unit.  
 * Timestamps are in seconds since 1970/01/01 00:00:00.00 UTC.
 * Data are in units of cgs.
 *
 * written by Lynn Dietz   October, 1999
 */

#ifndef RW_STRONGMOTION_H
#define RW_STRONGMOTION_H

/* Usefult constants
 *******************/
#define GRAVITY_CGS 978.03    /* Gravity in cm/sec/sec */

/* Define maximum lengths for strings/arrays 
   in and Earthworm TYPE_STRONGMOTION message
 ********************************************/
#define SM_VENDOR_LEN 49
#define SM_STA_LEN     6
#define SM_COMP_LEN    8
#define SM_NET_LEN     8
#define SM_LOC_LEN     5
#define SM_MAX_CHAN   36         /* max # channels in a TYPE_SM message       */
#define SM_MAX_RSA    25         /* max # spectral values for a given channel */
#define SM_NULL       -1.0         /* null value for freq & RSA                 */

/* Sources for the alternate time in the SM_DATA structure
 *********************************************************/
#define SM_ALTCODE_NONE                0
#define SM_ALTCODE_RECEIVING_MODULE    1
#define SM_ALTCODE_DATABASE            2
#define SM_ALTCODE_AUTOMATED_REVIEWER  3
#define SM_ALTCODE_HUMAN_REVIEWER      4

/* Structure to contain strongmotion data from one channel
   In the comments below, NTS = Null Terminated String
 *********************************************************/
typedef struct _SM_CHAN {
   char    sta[SM_STA_LEN+1];    /* NTS: Site code as per IRIS SEED                */  
   char    comp[SM_COMP_LEN+1];  /* NTS: Component code as per IRIS SEED           */  
   char    net[SM_NET_LEN+1];    /* NTS: Network code as per IRIS SEED             */
   char    loc[SM_LOC_LEN+1];    /* NTS: Location code as per IRIS SEED            */
   double  acc;                  /* peak acceleration (cm/sec/sec)                 */
   double  vel;                  /* peak velocity (cm/sec)                         */
   double  disp;                 /* peak displacement (cm)                         */
   int     nRSA;                 /* # pts describing response spectrum accel (RSA) */
   double  freq[SM_MAX_RSA];     /* frequencies at which RSA values are given      */
   double  RSA[SM_MAX_RSA];      /* RSA value for this channel at given frequency  */ 
} SM_CHAN;

/* Structure to contain all strongmotion data from one instrument
   In the comments below, NTS = Null Terminated String,
   OPTIONAL marks a field that need not be entered for data to
   be considered valid (these fields may be filled in later in DBMS).
 ********************************************************************/
typedef struct _SM_DATA {
   char    vendor[SM_VENDOR_LEN+1]; /* NTS: Name of company producing this device */
   char    sn[SM_VENDOR_LEN+1];     /* NTS: identifying (serial?) number of box   */
   double  tfield;                  /* field time, reported by SM box (seconds    */
                                    /*   since 1970/01/01 00:00:00.00)            */
   double  talt;                    /* OPTIONAL: alternate time, reported by      */
                                    /*   nobody, acq. software, analyst, etc.     */
   int     altcode;                 /* OPTIONAL: code specifying the source of    */
                                    /*   the alternate time field                 */
   double  tload;                   /* OPTIONAL: time data was loaded into DBMS   */
   int     nch;                     /* # channels contained in this structure     */
   SM_CHAN ch[SM_MAX_CHAN];         /* actual data for each channel in this box   */
} SM_DATA;

/* Function Prototypes
 *********************/
int  rd_strongmotion( char *msg, int msglen, SM_DATA *sm );
int  wr_strongmotion( SM_DATA *sm, char *buf, int buflen );
void log_strongmotion( SM_DATA *sm );

#endif
