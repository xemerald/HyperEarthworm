
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: ew2rsam.h 1107 2002-11-03 00:03:24Z lombard $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.2  2002/11/03 00:03:24  lombard
 *     Protected from multiple includes
 *
 *     Revision 1.1  2000/02/14 20:05:54  lucky
 *     Initial revision
 *
 *
 */

/*
 * ew2rsam.h
 *
 *   Header file for the ew2rsam module
 *   Defines local data structure used for book-keeping
 *
 *   Lucky Vidmar Thu May  6 10:07:21 MDT 1999
 *
 */

#ifndef EW2RSAM_H
#define EW2RSAM_H

#define	DC_ARRAY_ENTRIES	24
#define	DC_TIME_DIFF		2.56
#define	DC_STARTUP_ENTRIES	5
#define	DC_OFFSET_INVALID	-1.00
#define	TIME_INVALID        -1.00
#define	MAX_TIME_PERIODS	5

typedef struct time_period_struct
{
	double	time_period; 	/* time period in seconds */
	double	rsam_value;		/* currently kept rsam total */
	int		rsam_nsamp;		/* samples in the current total */
	double	rsam_starttime;	/* when did we start counting ? */

} Tstruct;

/* there will be one for each requested channel */
/* with the maximum read from the configuration file */

typedef struct rsam_values_struct
{

	/* START DC offset section */

	double	DC_offset;			/* DC offset value */

	double 	DC_cur_val;			/* Current DC total */
	int		DC_cur_nsamp;		/* Number of samples in current total */
	double	DC_starttime;		/* When did we start counting ? */

	double	DC_array[DC_ARRAY_ENTRIES];	/* Averages for previous time slices */
	int		DC_cur_pos;			/* index of the current position in the array */
	int		DC_start_pos;		/* index of the starting position in the array */

	int		DC_startup;			/* TRUE if we are still starting out */
	/* END DC offset section */

	/* START time period section */
	Tstruct		TP[MAX_TIME_PERIODS];

} RSAM_val;


typedef struct rsam_struct
{

	char	sta[7];
	char	chan[9];
	char	net[9];
	RSAM_val *values;

} RSAM;

#endif
