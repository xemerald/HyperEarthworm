/******************************************************************************
 *
 *	File:			ew_spectra.h
 *
 *	Function:		Header for ewspectra module
 *
 *	Author(s):		Scott Hunter, ISTI
 *
 *	Source:			Started anew.
 *
 *	Notes:			
 *
 *	Change History:
 *			4/26/11	Started source
 *	
 *****************************************************************************/

#ifndef SPECTRA_MSG_H
#define SPECTRA_MSG_H

#include <trace_buf.h>
//#define	TRACE2_STA_LEN	  7    /* SEED: 5 chars plus terminating NULL */
//#define	TRACE2_NET_LEN	  9    /* SEED: 2 chars plus terminating NULL */
//#define	TRACE2_CHAN_LEN   4    /* SEED: 3 chars plus terminating NULL */
//#define	TRACE2_LOC_LEN	  3    /* SEED: 2 chars plus terminating NULL */
//#define LOC_NULL_STRING  "--"  /* NULL string for location code field */

typedef struct {
        char    sta[TRACE2_STA_LEN];   /* Site name (NULL-terminated) */
        char    net[TRACE2_NET_LEN];   /* Network name (NULL-terminated) */
        char    chan[TRACE2_CHAN_LEN]; /* Component/channel code (NULL-terminated)*/
        char    loc[TRACE2_LOC_LEN];   /* Location code (NULL-terminated) */
        char    units[2];              /* a for amplitude/phase, c for complex values, p for peaks 
					  lower case is Intel byte order, upper case SPARC*/

        int     nsamp;                 /* Number of frequency  samples in packet */
        double  starttime;             /* time of first sample in epoch seconds
                                          (seconds since midnight 1/1/1970) */
        double  delta_frequency;       /* delta frequency of spectra (Hz) */
        int     numPeaks;              /* number of peaks (valid only if units is p or P */
		char    pad[12];	/* unused, but takes header to multiple of 8 bytes */
} EW_SPECTRA_HEADER;
/* In ring message, data follows header as 2 doubles per sample 
	(amp phase or real imaginary or freq amp) */

#endif
