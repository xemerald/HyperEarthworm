
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: nano_buf.h 15 2000-02-14 20:06:34Z lucky $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2000/02/14 20:05:54  lucky
 *     Initial revision
 *
 *
 */

/* November 1 1994
 * The Nanometrics trace data header format
 * NOTE: 3 bytes per data sample
 * 
 * NOTE: Header must be an even multiple of bytes long, because the receiver
 *       moves everything in int's for speed reasons.  The Nanometrics 
 *       acquisition system runs on an Intel computer. 
 */

#ifndef NANO_BUF_H
#define NANO_BUF_H

typedef struct {
   unsigned long  tssec;      /* Time stamp - seconds                           */
   unsigned long  tsmic;      /* Time stamp - microseconds past the last second */
   unsigned long  first_scan; /* Number of first scan in buffer                 */
   unsigned short series;     /* Incremented each time a data source is started */
   unsigned short sample_dt;  /* Length of standard interval in seconds         */
   unsigned short nsample;    /* Number of samples per this interval            */
   unsigned char  mod_id;     /* Data source module id                          */
   unsigned char  padding;    /* will always be 0                               */
   unsigned short errword;    /* Error word (will always be 0?)                 */
   unsigned short nchan;      /* Number of channels                             */
   unsigned short nscan;      /* Number of scans per buffer                     */
   unsigned short netid;      /* Network identifier                             */
   unsigned char  dig_num;    /* digitzier number, as set in CED for the        */
                              /* fep/port (1-8)                                 */
   unsigned char  chan_num;   /* channel number for given digitizer (1-6)       */
} NANO_HEADER;

#define NBYTES_NANO_HEADER  30   /* define the number of bytes in a NANO_HEADER */

#endif

