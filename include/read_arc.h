
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: read_arc.h 6383 2015-06-18 16:12:51Z saurel $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.10  2004/05/28 22:22:42  dietz
 *     added loc field to struct Hpck
 *
 *     Revision 1.9  2002/12/06 22:31:26  dietz
 *      Added 3 fields (nphS,nphtot,nPfm) to struct Hsum
 *
 *     Revision 1.8  2002/11/03 00:15:28  lombard
 *     Protected from multiple includes.
 *
 *     Revision 1.7  2002/10/29 18:47:13  lucky
 *     Added origin version number
 *
 *     Revision 1.6  2000/12/06 17:47:07  lucky
 *     Removed Pph and Sph strings from Hpck, and added Plabel, Slabel, Ponset,
 *     and Sonset. We need these to properly track the onset.
 *
 *     Revision 1.5  2000/09/12 19:07:02  lucky
 *     Defined separate FM fields for S and P picks
 *
 *     Revision 1.4  2000/08/21 19:48:36  lucky
 *     Modified the Hpck structure to be able to handle both P and S
 *     picks at the same time, by specifying time, residual, quality, and phase
 *     remarks separately for P and S picks.
 *
 *     Revision 1.3  2000/06/12 21:34:00  lucky
 *     Changed NO_FLOAT_VAL, NO_INT_VAL, and NO_LONG_VAL to 0.
 *
 *     Revision 1.2  2000/03/30 15:35:30  davidk
 *     Added fields to Hsum and Hpck that contained information neccessary for
 *     orareport, so that the read_arc routines could be used by orareport.
 *
 *     Revision 1.1  2000/02/14 20:05:54  lucky
 *     Initial revision
 *
 *
 */

#ifndef READ_ARC_H
#define READ_ARC_H

#include <limits.h>

/* Some special unlikely values */
#define NO_FLOAT_VAL  0.0
#define NO_INT_VAL    0
#define NO_LONG_VAL   0

/* Structure of info read from TYPE_HYP2000ARC message
 **************************************************/
struct Hsum {
        long    qid;       /* event id from binder */
        double  ot;        /* origin time as sec since 1600   */
        float   lat;       /* latitude (North=positive)       */
        float   lon;       /* longitude(East=positive)        */
        float   z;         /* depth (down=positive)           */
        int     nph;       /* # phases (P&S) w/ weight >0.1   */
        int     nphS;      /* # S phases w/ weight >0.1       */
        int     nphtot;    /* # phases (P&S) w/ weight >0.0   */
        int     nPfm;      /* # P first motions               */
        int     gap;       /* maximum azimuthal gap           */
        int     dmin;      /* distance (km) to nearest station*/
        float   rms;       /* RMS travel time residual        */
        int     e0az;      /* azimuth of largest principal error */
        int     e0dp;      /* dip of largest principal error  */
        float   e0;        /* magnitude (km) of largest principal error */
        int     e1az;      /* azimuth of intermediate principal error */
        int     e1dp;      /* dip of intermediate principal error  */
        float   e1;        /* magnitude (km) of intermed principal error */
        float   e2;        /* magnitude (km) of smallest principal error */
        float   erh;       /* horizontal error (km) */
        float   erz;       /* vertical error (km) */
        float   Md;        /* duration magnitude */
        char    reg[4];    /* location region */
        char    cdate[20]; /* date character buffer */
        char    labelpref; /* character describing preferred magnitude */
        float   Mpref;     /* preferred magnitude */
        float   wtpref;    /* weight (~ # readings) of preferred Mag */
        char    mdtype;    /* Coda duration magnitude type code */
        float   mdmad;     /* Median-absolute-difference of duration mags */
        float   mdwt;      /* weight (~ # readings) of Md */
        long    version;   /* version number of the origin */
};

/* Structure to hold raw pick info from a phase line & its shadow
 ****************************************************************/
struct Hpck {
        char    site[6];   /* site code, null terminated */
        char    net[3];    /* seismic network code, null terminated */
        char    comp[4];   /* component code, null terminated */
        char    loc[3];    /* location code, null terminated */
        char    Plabel;    /* P phase label */
        char    Slabel;    /* S phase label */
        char    Ponset;    /* P phase onset */
        char    Sonset;    /* S phase onset */
        double  Pat;       /* P-arrival-time as sec since 1600 */
        double  Sat;       /* S-arrival-time as sec since 1600 */
        float   Pres;      /* P travel time residual */  
        float   Sres;      /* S travel time residual */  
        int     Pqual;     /* Assigned P weight code */
        int     Squal;     /* Assigned S weight code */
        int     codalen;   /* Coda duration time */
        int     codawt;    /* Coda weight */
        char    Pfm;       /* P first motion */
        char    Sfm;       /* S first motion */
        char    cdate[18]; /* date character buffer */
        char    datasrc;   /* Data source code. */
        float   Md;        /* Station duration magnitude */
        int     azm;       /* azimuth */
        int     takeoff;   /* emergence angle at source */
        float   dist;      /* epicentral distance (km) */
        float   Pwt;       /* P weight actually used. */
        float   Swt;       /* S weight actually used. */
        int     pamp;      /* peak P-wave half amplitude */
        int     codalenObs;/* Coda duration time (Measured) */
        int     ccntr[6];  /* Window center from P time */
        int     caav[6];   /* Average Amplitude for ccntr[x] */
};

#define MAX_PHASES 5000
typedef struct {
	struct Hsum sum;
	int num_phases;
	struct Hpck *phases[MAX_PHASES];
} HypoArc;
	
int parse_arc(char *msg, HypoArc *arc);
int parse_arc_no_shdw(char *msg, HypoArc *arc);
int free_phases(HypoArc *arc);
int write_arc(char *msg, HypoArc *arc);

/* Function Prototypes
 *********************/
int read_hyp( char *sumline, char *shdw, struct Hsum *sumP );
int read_phs( char *phsline, char *shdw, struct Hpck *pckP );
int read_phs_no_shdw( char *phsline, struct Hpck *pckP );

void write_hyp( char *sumcard, struct Hsum sumP );
void write_phs( char *phscard, struct Hpck pckP );
void write_term( char *termcard, long HypId );

char ComputeAverageQuality(float rms, float erh, float erz, float depth, float dmin, int no, int gap);
char *parse_arc_next_shadow(char *src);

#endif
