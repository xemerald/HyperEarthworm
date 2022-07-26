
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: rw_glevt.h 2050 2006-01-17 17:27:29Z friberg $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2006/01/17 17:27:29  friberg
 *     added from MWithers work on global routines
 *
 *     Revision 1.2  2002/11/03 00:18:24  lombard
 *     Protected from multiple includes.
 *
 *     Revision 1.1  2002/09/10 17:20:03  dhanych
 *     Initial revision
 *
 *
 */

#ifndef RW_GLEVT_H
#define RW_GLEVT_H

#include <limits.h>
#include <ewdb_ora_api.h>
#include <ew_event_info.h>

#define PHASE_NAME_LEN  10
#define AUTHOR_LEN		12


/* Structure of info read from global archive message
 **************************************************/
typedef struct Glarc_sum_struct {
        long    qid;       /* event id from binder */
        double  ot;        /* origin time as sec since 1600   */
        float   lat;       /* latitude (North=positive)       */
        float   lon;       /* longitude(East=positive)        */
        float   z;         /* depth (down=positive)           */
        int     nph;       /* number of phases w/ weight >0.1 */
        int     gap;       /* maximum azimuthal gap           */
        int     dmin;      /* distance (km) to nearest station*/
        float   rms;       /* RMS travel time residual        */
} GlarcSum;

/* Structure to hold raw pick info from a phase line & its shadow
 ****************************************************************/
typedef struct Glarc_pck_struct {
	char    sta[6];  				 	/* site code */
	char    net[3];    					/* seismic network code */
	char    comp[4];   					/* station component code */
	char    phase[PHASE_NAME_LEN];   	/* phase label */
	char    pickAuthor[AUTHOR_LEN];     /* who created the pick */
	char    ExtPickID[AUTHOR_LEN];      /* who created the pick */
	double  at;       					/* arrival-time as sec since 1600 */
	char    fm;       					/* first motion */
	char    wt;       					/* phase weights */
	double	ampTime;					/* Time of Post-P amplitude */
	long	pAmp;
	double	period;
	int     caav[6];   					/* Average Amplitude for ccntr[x] */
	int     codalen;   					/* Coda duration time */
	int     codawt;    					/* Coda weight */
	char    onset;    					/* Phase onset */
	long	pamp[3];					/* amplitudes */
} GlarcPhase;

/* Function Prototypes
 *********************/
int     write_glevt_sumcard (GlarcSum *pSum, char *sumcard);
int     write_glevt_phasecard (GlarcPhase *pPhs, char *phscard);

int     read_glevt_sumcard (GlarcSum *pSum, char *sumcard);
int     read_glevt_phasecard (GlarcPhase *pPhs, char *phasecard);

int	GlEvt2EWEvent (EWEventInfoStruct *pEWEvent, char *pGlEvt, int GlEvtLen);


#endif
