/******************************************************************************
 *
 *	File:			ew_spectra_io.h
 *
 *	Function:		Definitions for writing spectra data to a file or a ring
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

#include <stdio.h>
#include <earthworm_defs.h>
#include <ew_timeseries.h>
#include <transport.h>
#include <ew_spectra.h>

/***********************************************************************
 * EWSConvertTime () - given pStart return a double representing          *
 *     number of seconds since 1970                                    *
 ***********************************************************************/
int EWSConvertTime (char *pStart, double *start);

/***********************************************************************
 * EWSUnConvertTime () - given double representing number of seconds      *
 *     since 1970, set pStart to string rep of same                    *
 ***********************************************************************/
int EWSUnConvertTime (char *pStart, double start);

/***********************************************************************
 * write_spectra_to_ring ()                                            *
 ***********************************************************************/
int write_spectra_to_ring( EW_SPECTRA_HEADER *ewsh, double *data,
		char* Argv0, SHM_INFO *region, MSG_LOGO *logo );

/***********************************************************************
 * write_EWTS_as_spectra_to_ring ()                                       *
 ***********************************************************************/
int write_EWTS_as_spectra_to_ring( EW_TIME_SERIES *ewspec, 
	char* Argv0, SHM_INFO *region, MSG_LOGO *logo );
 

/***********************************************************************
 * write_spectra_to_file ()                                            *
 ***********************************************************************/
int write_spectra_to_file( EW_SPECTRA_HEADER *ewsh, double *data,
	char* Argv0, FILE *fp );

/***********************************************************************
 * write_EWTS_as_spectra_to_file ()                                    *
 ***********************************************************************/
int write_EWTS_as_spectra_to_file( EW_TIME_SERIES *ewspec, 
	char* Argv0, FILE *fp );
 
