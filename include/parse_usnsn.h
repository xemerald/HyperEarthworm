
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.9  2002/11/03 00:14:26  lombard
 *     Protected from multiple includes.
 *
 *     Revision 1.8  2002/03/22 19:53:47  lucky
 *     *** empty log message ***
 *
 *     Revision 1.7  2001/07/01 22:16:07  davidk
 *     Added include of earthworm_defs.h.
 *     Oops, I guess that include of earthworm.h was neccessary.
 *
 *     Revision 1.6  2001/07/01 22:08:14  davidk
 *     Removed unneccessary include of earthworm.h and added prototype for
 *     ParseNSNMsg().
 *
 *     Revision 1.5  2000/10/02 21:28:23  lucky
 *     set lengths of sta and phase in PhaseStruct to #defines STA_LEN and PHA_LEN
 *
 *     Revision 1.4  2000/09/12 18:14:15  lucky
 *     Explicitly set size of phases (max number of phases in the DB)
 *
 *     Revision 1.2  2000/06/26 20:03:41  lucky
 *     Added RETURN_DELETE flag so that parse_usnsn can signal back to the
 *     calling routine that it encountered a delete message.
 *
 *     Revision 1.1  2000/02/14 20:05:54  lucky
 *     Initial revision
 *
 *
 */

#ifndef PARSE_USNSN_H
#define PARSE_USNSN_H

#include <earthworm_defs.h>


#define		DATE_BASE		    5
#define		DEPTH_BASE		    25
#define		EVENT_KEY_BASE		52
#define		PRELIM_BASE	   	    61
#define		ELLIPSE_BASE		22
#define		ELLIPSE_LENGTH		53
#define     EARTH_CIRCUM        40000.0

/* magnitudes */
#define     MAG_LEN     5
#define     DATUM_LEN   3
#define     MB_BASE     5
#define     MB_DATUM    11
#define     ML_BASE     21
#define     ML_DATUM    27
#define     MBLG_BASE   39
#define     MBLG_DATUM  45
#define     MD_BASE     55
#define     MD_DATUM    61
#define     MS_BASE     71
#define     MS_DATUM    77

/* phases */
#define		STA_BASE	1
#define		STA_LEN		5
#define		PHA_BASE	6
#define		PHA_LEN		8
#define		OT_BASE		14
#define		OT_LEN		11
#define		RES_BASE	25
#define		RES_LEN		6
#define		RES_USED	31
#define		DIST_BASE	32
#define		DIST_LEN    6
#define		AZM_BASE	39
#define		AZM_LEN		3
#define		MAG1_BASE	43
#define		MAG1_LEN	15
#define		MAG1_USED	58
#define		MAG2_BASE	60
#define		MAG2_LEN	15
#define		MAG2_USED	75


#define RETURN_DELETE -100

# define RETURN_BAD_SCN -3

#ifndef RETURN_SUCCESS
# define RETURN_SUCCESS 0
# define RETURN_FAILURE -1
#endif /* !DEF RETURN_SUCCESS */

#define	MAX_MAGS	5

/* Structures */
typedef struct _PhaseMag
{

	int	  	MagType;		/* see rw_mag.h */
	char	magLabel;		/* b, S, d, L, g */
	double	value;
	double	period;
	double	mag;
	int		used;

} PhaseMag;


typedef struct _PhaseStruct
{
	char		sta[STA_LEN+1];
	char		phase[PHA_LEN+1];
	int			automatic;
	char		onset;
	char		motion;
	double		ot;
	double		res;
	int			res_used;		/* is this phase used? */
	double		dist;
	int			azm;
	int			num_mags;
	PhaseMag	mag[5];			/* magnitudes */

} Phase;

typedef struct _OriginMag
{

	int		MagType;		/* see rw_mag.h */
	char	magLabel;		/* b, S, d, L, g */
	double	magAvg;
	int		numStas;
	int		isPref;			/* Is this the preferred mag */

} OriginMag;

typedef struct _ErrorElipse
{

	double		maj_s;		/* Semi-major axis - strike */
	double		maj_d;		/* Semi-major axis - dip    */
	double		maj_l;		/* Semi-major axis - length */

	double		min_s;		/* Semi-minor axis - strike */
	double		min_d;		/* Semi-minor axis - dip    */
	double		min_l;		/* Semi-minor axis - length */

	double		int_s;		/* Intermediate axis - strike */
	double		int_d;		/* Intermediate axis - dip    */
	double		int_l;		/* Intermediate axis - length */

} Err_Elipse;


typedef struct _NSNMsgStruct
{

	char		EventKey[6];
	char		EventDate[10];
	int			automatic;		/* 1-yes, 0-no */
	double		ot;				/* origin time, seconds since 1970 */
	double		ot_err;			/* time error in seconds */
	float		lat;            /* origin lattitude, in degrees */
	float		lat_err;        /* origin lattitude error, in km */
	float		lon;            /* origin longitude, in degrees */
	float		lon_err;        /* origin longitude error, in km */
	float		depth;          /* depth, in km */
	float		depth_err;      /* depth error, in km */
	int			depth_fixed;    /* 1-yes, 0-no */
	int			nph_used;       /* num phases used */
	int			nph_assoc;      /* num phases associated */
	int			nph_actual;     /* acutal phases parsed and stored */
	double		Dmin;      		/* distance from epicenter to nearest station */
	double		std_error;      /* standard error - rms of residuals,  */
                	            /* in seconds                          */
	Err_Elipse  error;			/* Error elipse - 9 values */
	int			numMags;		/* Number of magnitudes for the event */
	OriginMag   O_mag[MAX_MAGS];/* Magnitudes for the event */
	Phase   	phases[DB_MAX_PHS_PER_EQ];		/* Phases for the event */

} NSNStruct;


int ParseNSNMsg(char *NsnMsg, int msgLen, NSNStruct *msgStruct, 
                int debug, char *debug_dir);



#endif
