/******************************************************************************
 *
 *	File:			ew_timeseries.h
 *
 *	Function:		Representation of & a library of functions that can handle 
 *                  a time-series for processing in Earthworm modules
 *
 *	Author(s):		Paul Friberg, ISTI
 *                  Scott Hunter, ISTI
 *
 *	Source:			Started anew.
 *
 *	Notes:			
 *
 *	Change History:
 *			4/26/11	Started source
 *	
 *****************************************************************************/

#ifndef EW_TIME_SERIES_H
#define EW_TIME_SERIES_H

#include "trace_buf.h"
#include <ws_clientII.h>

typedef struct  {
	TRACE2X_HEADER trh2x;
	int gaps;		  /* # of gaps in the time-series */
	double gapTime;	  /* amount of time in gaps */
	double gapValue;  /* values used to fill gaps with */
	long   dataCount; /* # of entries in data */
	int    dataType;  /* type of data in series */
	double *data;	  /* a pointer to the data as an array of doubles, of trh2x.nsamp number of samples */
}
EW_TIME_SERIES ;

#define EWTS_TYPE_SIMPLE (0) 	/* each entry is one double */
#define EWTS_TYPE_COMPLEX (1)	/* each entry is a complex value:
									one double for the real part,
									the next for the imaginary part */
#define EWTS_TYPE_AMPPHS (2)	/* each entry is a pair of values:
									one double for the amplitude,
									the next for the phase */

#define EWTS_MODE_FIRST (1)		/* process only 1st component of each entry */
#define EWTS_MODE_SECOND (2)	/* process only 2nd component of each entry */
#define EWTS_MODE_BOTH (3)		/* process both components of each entry */

/*****************************************************************************
	unary_calc_EWTS( arg, op, oparg ):  
		Call op(d,oparg) for each d in arg
		Return 0 on success
*****************************************************************************/
int unary_calc_EWTS( 
	EW_TIME_SERIES arg,					/* the EWTS to calculate from */
	int mode,							/* which component(s) to process */
	void(*op)(const double, const int, void* ),	/* the accumulator function */
	void *oparg );						/* the "accumulator" argument for op */

/*****************************************************************************
	unary_modify_EWTS( arg, op, oparg ): 
		Replace each d in arg with op(d,oparg)
		Return 0 on success
*****************************************************************************/
int unary_modify_EWTS( 
	EW_TIME_SERIES arg,						/* the EWTS to modify */
	int mode,								/* which component(s) to process */
	double(*op)(const double, const int, const double),/* the modify function */
	const double oparg );					/* second argument to all calls to op */

/*****************************************************************************
	unary_fill_EWTS( input, output, op, oparg ): 
		For each d in input, assign corresponding element of output with op(d,oparg)
		Return 0 on success
*****************************************************************************/
int unary_fill_EWTS( 
	EW_TIME_SERIES input, 					/* the EWTS to calculate from */
	EW_TIME_SERIES output, 					/* the EWTS to fill */
	int mode,								/* which component(s) to process */
	double(*op)(const double, const int, const double),/* function to compute fill values */
	const double oparg );					/* second arg to function */

/*****************************************************************************
	binary_modify_EWTS( arg1, arg2, op ): 
		Replace each d in arg1 with op(d,d2), where d2 is the 
		corresponding element of arg2
		Return 0 on success
*****************************************************************************/
int binary_modify_EWTS( 
	EW_TIME_SERIES arg1, 						/* the EWTS to modify */
	EW_TIME_SERIES arg2, 						/* EWTS with second arguments */
	int mode,									/* which component(s) to process */
	double(*op)(const double, const int, const double) );	/* the modify function */

/*****************************************************************************
	binary_fill_EWTS( arg1, arg2, output, op ): 
		For each d1 in arg1, assign corresponding element of output with 
		op(d1,d2), where d2 is the corresponding element of arg2
		Return 0 on success
*****************************************************************************/
int binary_fill_EWTS( 
	EW_TIME_SERIES arg1, 						/* the 1st EWTS to read */
	EW_TIME_SERIES arg2, 						/* the 2nd EWTS to read */ 
	EW_TIME_SERIES output,  					/* the EWTS to fill */
	int mode,									/* which component(s) to process */
	double(*op)(const double, const int, const double) );	/* function to compute fill values */

/* Taper types for taper_EWTS */
#define BARTLETT	(1)
#define HANNING		(2)
#define PARZAN		(3)
#define BMHARRIS	(4)

/*****************************************************************************
	taper_EWTS( ewts, taper_type, fraction ): 
		Taper the fraction of ewts using the specified taper type
*****************************************************************************/
void taper_EWTS( 
	EW_TIME_SERIES ewts, 	/* EWTS to taper */
	int taper_type, 		/* type of taper to employ */
	double fraction,		/* fraction of EWTS to taper at each end */
	int mode );				/* which component(s) to process */

/*****************************************************************************
	subtract_from_EWTS( ewts1, ewts2 ): 
		Subtract the elements of ewts2 from the corresponding elements of ewts1
		Return 0 on success
*****************************************************************************/
int subtract_from_EWTS( 
	EW_TIME_SERIES ewts1, 	/* EWTS to subtract from */
	EW_TIME_SERIES ewts2,	/* EWTS of values to subtract */
	int mode );				/* which component(s) to process */

/*****************************************************************************
	demean_EWTS( ewts ): 
		Subtract the mean of ewts from all of its elements
		Return 0 on success
*****************************************************************************/
int demean_EWTS( 
	EW_TIME_SERIES ewts );	/* EWTS to demean */

/*****************************************************************************
	print_EWTS( ewts, filename ): 
		Print the contents of ewts (a line for each value with with 
		tab-separated time & value) to the specified file
*****************************************************************************/
void print_EWTS(
	EW_TIME_SERIES *ewts, 		/* EWTS to print */
	char *col1header,			/* header for first column ("Time" if NULL) */
	/* map entry index/time to first column value time if NULL */
		double(*op)(int i, double t, void *arg),	
	void *entryMapArg,			/* what to pass as 3rd arg to entryMap */
	FILE *fp);					/* open file to print to; stdout if NULL */

#endif
