/******************************************************************************
 *
 *	File:			ew_spectra_io.c
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

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time_ew.h>
#include "ew_spectra_io.h"
#include <ctype.h>
#include <earthworm_simple_funcs.h>

/***********************************************************************
 * EWSConvertTime ()                                                   *
 ***********************************************************************/
int EWSConvertTime (char *pStart, double *start)
{
    char		strDate[30];
    struct tm 	tmDate;
    
    if (pStart == NULL) {
	logit ("e", "Invalid parameters passed to EWSConvertTime.\n");
	return EW_FAILURE;
    }
    
    strncpy (strDate, pStart, (size_t) 14);
    strDate[14] = '\0';
	tmDate.tm_sec 	= atoi( strDate+12 );
	strDate[12] = '\0';
	tmDate.tm_min 	= atoi( strDate+10 );
	strDate[10] = '\0';
	tmDate.tm_hour 	= atoi( strDate+8 );
	strDate[ 8] = '\0';
	tmDate.tm_mday 	= atoi( strDate+6 );
	strDate[ 6] = '\0';
	tmDate.tm_mon 	= atoi( strDate+4 )-1;
	strDate[ 4] = '\0';
	tmDate.tm_year 	= atoi( strDate ) - 1900;
    tmDate.tm_wday = tmDate.tm_yday = tmDate.tm_isdst = 0;
#if defined (_SOLARIS) || defined(_WINNT)
#else
	tmDate.tm_gmtoff = 0;
    tmDate.tm_zone = "GMT";
#endif
    *start = (double)timegm_ew( &tmDate );
    
    return EW_SUCCESS;
}

/***********************************************************************
 * EWSUnConvertTime ()                                                 *
 ***********************************************************************/
int EWSUnConvertTime (char *pStart, double start)
{
    struct tm 	tmDate;
    time_t		start_time;
    
    if (pStart == NULL) {
	logit ("e", "Invalid parameters passed to EWSUnConvertTime.\n");
	return EW_FAILURE;
    }
    
    start_time = (int)start;
    gmtime_ew( &start_time, &tmDate );
    sprintf( pStart, "%04d%02d%02d%02d%02d%02d", 
    	tmDate.tm_year+1900, tmDate.tm_mon+1, tmDate.tm_mday,
    	tmDate.tm_hour, tmDate.tm_min, tmDate.tm_sec );
    pStart[14] = 0;
    
    return EW_SUCCESS;
}


/***********************************************************************
 * write_spectra_to_ring ()                                            *
 ***********************************************************************/
int write_spectra_to_ring( EW_SPECTRA_HEADER *ewsh, double *data,
		char* Argv0, SHM_INFO *region, MSG_LOGO *logo )
 {
	int ndata = ewsh->nsamp;
	size_t bufsize = sizeof(EW_SPECTRA_HEADER) + sizeof(double) * (ndata*2);
	char *buffer = malloc( bufsize );
	if ( buffer == NULL ) {
		logit("e", "%s: Failed to allocate memory for writing spectra to ring\n", Argv0 );
		return 1;
	}
	
	memcpy( buffer, ewsh, (size_t) sizeof(EW_SPECTRA_HEADER) );
	memcpy( buffer+sizeof(EW_SPECTRA_HEADER), data, (size_t) bufsize-sizeof(EW_SPECTRA_HEADER) );
	GetType( "TYPE_SPECTRA_DATA", &(logo->type) );
	if( tport_putmsg( region, logo, (long) bufsize, buffer ) != PUT_OK ) {
	   	logit("et", "%s:  Error writing spectra to ring.\n",
			  Argv0 );
		return 1;
	}
	free( buffer );
	return 0;
 }
 
static void fill_EWSH_from_EWTS( EW_TIME_SERIES *ewspec, EW_SPECTRA_HEADER *ewsh )
{
	memcpy( ewsh->sta,  ewspec->trh2x.sta,  (size_t) TRACE2_STA_LEN );
	memcpy( ewsh->net,  ewspec->trh2x.net,  (size_t) TRACE2_NET_LEN );
	memcpy( ewsh->chan, ewspec->trh2x.chan, (size_t) TRACE2_CHAN_LEN );
	memcpy( ewsh->loc,  ewspec->trh2x.loc,  (size_t) TRACE2_LOC_LEN );
	ewsh->delta_frequency = 1.0/ewspec->trh2x.samprate;
	ewsh->nsamp = ewspec->dataCount;
	ewsh->starttime = ewspec->trh2x.starttime;
#if defined (_SPARC)
	strcpy( ewsh->units, ewspec->dataType == EWTS_TYPE_COMPLEX ? "C" : "A" );
#elif defined (_INTEL)
	strcpy( ewsh->units, ewspec->dataType == EWTS_TYPE_COMPLEX ? "c" : "a" );
#else
#error "_INTEL or _SPARC must be set before compiling"
#endif
}


 /***********************************************************************
 * write_EWTS_as_spectra_to_ring ()                                     *
 ************************************************************************/
int write_EWTS_as_spectra_to_ring( EW_TIME_SERIES *ewspec, char* Argv0, SHM_INFO *region, MSG_LOGO *logo )
{
	EW_SPECTRA_HEADER ewsh;
	fill_EWSH_from_EWTS( ewspec, &ewsh );
	return write_spectra_to_ring( &ewsh, ewspec->data, Argv0, region, logo );
}
 
/***********************************************************************
 * strcat_header ()                                                    *
 ***********************************************************************/
static char *strcat_header( EW_SPECTRA_HEADER *ewsh, char *data ) 
{	
	char date[100];
	EWSUnConvertTime( date, ewsh->starttime );	
	sprintf( data, "nsamp=%15d\tdelta_freq=%15.3lf starttime=%s\n", 
		ewsh->nsamp, ewsh->delta_frequency, date );
	data += strlen(data);
	return data;
}

/***********************************************************************
 * write_spectra_to_file ()                                            *
 ***********************************************************************/
int write_spectra_to_file( EW_SPECTRA_HEADER *ewsh, double *data,
	char* Argv0, FILE *fp )
{
	double delta = ewsh->delta_frequency;
	double nyquist= 1.0/(2.0*delta);
	int ndata = ewsh->nsamp;
	double scale = nyquist/ndata;
	char buffer[200], *bufPos;
	int i;
	
	bufPos = strcat_header( ewsh, buffer );
	*bufPos = 0;
	if ( tolower(ewsh->units[0])=='c' )
		fprintf( fp, "%s%15s\t%15s\t%15s\n", buffer, "Frequency", "Real", "Imaginary" );
	else if ( tolower(ewsh->units[0])=='a' )
		fprintf( fp, "%s%15s\t%15s\t%15s\n", buffer, "Frequency", "Amplitude", "Phase" );
	else {
		logit( "e", "%s: invalid type or spectra for writing ('%s')\n", Argv0,ewsh->units );
		return 1;
	}
	for ( i=0; i<=ndata*2; i+=2 )
	  fprintf( fp, "%15.3lf\t%15.3lf\t%15.3lf\n", i*scale/2, data[i], data[i+1] );
	fflush( fp );
	return 0;
 }
 
/***********************************************************************
 * write_EWTS_as_spectra_to_file ()                                    *
 ***********************************************************************/
int write_EWTS_as_spectra_to_file( EW_TIME_SERIES *ewspec, char* Argv0, FILE *fp )
{
	EW_SPECTRA_HEADER ewsh;
	fill_EWSH_from_EWTS( ewspec, &ewsh );
	return write_spectra_to_file( &ewsh, ewspec->data, Argv0, fp );
}
	
