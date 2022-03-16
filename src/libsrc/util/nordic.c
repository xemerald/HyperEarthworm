
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <earthworm.h>
#include <read_arc.h>
#include <rw_mag.h>
#include <chron3.h>
#include "nordic.h"


static int Debug = 0;


/* Functions in this file */
NordicPhase* InitializePhase( void );
void freeNordicLocation( NordicLoc* location );
void freeNordicPhase( NordicPhase* phase );
int fwriteNorLoc( FILE* str, NordicLoc* location );
double readdouble( char* source, int len );
void fixChannelIds( Nordic* event );
int parseCommentCommand( char* command, char* value, char* source );
int addNordicComment( Nordic* event, char* cmt );
int addNordicPhase( Nordic* event, struct Hpck *arcphase );
int addNordicAmpPhase( Nordic* event, MAG_CHAN_INFO *magphase );
int findNumericalCommand( long* target, char* command, Nordic* event );
Nordic* InitializeEvent( void );
NordicLoc* InitializeLocation( void );
NordicPhase* InitializePhase( void );

/* paulf discovered that the amps passed to seisan were off by 100.0, not sure quite
	why that is just yet, but lets fix this in seisan_report regardless
*/
#define LOCALMAG_AMP_MULTIPLIER 100.0
/******************************************************************************
 * Print a seisan event to a stream                                           *
 ******************************************************************************/
int fwriteNor( FILE* str, Nordic* event )
{
	unsigned int i;
	int ccount = 0, caux;
	time_t timeNow;
	struct tm utc_time;
	char datestr[18];
	struct Greg ot, at;
	char tempstr[60];

	/* Create line type 1 for the primary location */
	caux = fwriteNorLoc( str, event->primloc );
	if( caux < 0 ) return -1;
	ccount += caux;

	/* Create line type E, optional */
	if( event->hypErr != NULL )
	{
		caux = fprintf( str, " GAP=%-3d    %6.2f    %6.1f  "
				"%6.1f%5.1f%12.4e%12.4e%14.4eE\n",
				event->hypErr->gap, event->hypErr->ot_err, event->hypErr->lat_err,
				event->hypErr->lon_err, event->hypErr->z_err,
				event->hypErr->cov_xy, event->hypErr->cov_xz,
				event->hypErr->cov_yz );
		if( caux < 0 ) return -1;
		ccount += caux;
	}

	/* Create line type 1 for other locations */
	for( i = 0; i < event->nlocations; i++ )
	{
		if( event->primloc == event->locations[i] ) continue;
		caux = fwriteNorLoc( str, event->locations[i] );
		if( caux < 0 ) return -1;
		ccount += caux;
	}

	/* Create line type I */

	time( &timeNow );

	utc_time = *gmtime( &timeNow );
	date17( event->primloc->ot, datestr );
	caux = fprintf( str, " ACTION:NEW %02d-%02d-%02d %02d:%02d OP:EW   "
			"STATUS:               ID:%.4s%.2s%.2s%.2s%.2s%.2s     I\n",
			utc_time.tm_year % 100, utc_time.tm_mon + 1, utc_time.tm_mday,
			utc_time.tm_hour, utc_time.tm_min,
			datestr, datestr + 4, datestr + 6,
			datestr + 8, datestr + 10, datestr + 12 );
	if( caux < 0 ) return -1;
	ccount += caux;

	/* Waveform references */
	for( i = 0; i < event->nwaveref; i++ )
	{
		caux = fprintf( str, " %-78.78s6\n", event->waverefs[i] );
		if( caux < 0 ) return -1;
		ccount += caux;
	}

	/* Create Comment lines - type 3 */
	for( i = 0; i < event->ncomments; i++ )
	{
		caux = fprintf( str, " %-78.78s3\n", event->comments[i] );
		if( caux < 0 ) return -1;
		ccount += caux;
	}


	/* Add phases header line */
	caux = fprintf( str, " STAT SP IPHASW D HRMM SECON CODA AMPLIT PERI AZIMU "
			"VELO AIN AR TRES W  DIS CAZ7\n" );
	if( caux < 0 ) return -1;
	ccount += caux;

	/* Add phases */
	ot = *datime( event->primloc->ot, &ot );
	for( i = 0; i < event->nphases; i++ )
	{

		/* Tempstr will replace weigth */
		if( event->phases[i]->weight == 0 )
			strcpy( tempstr, " " );
		else
			sprintf( tempstr, "%d", event->phases[i]->weight );
		caux = fprintf( str, " %-5.5s%c%c %c%-4.4s%s%c%c ",
				event->phases[i]->station, event->phases[i]->channel[0],
				event->phases[i]->channel[2], event->phases[i]->qualityIndicator,
				event->phases[i]->phaseIndicator, tempstr,
				//( event->phases[i]->autoIndicator == ' ' ) ? ( ( *options.automaticIndicator == 1 ) ? 'A' : ' ' ) : ( event->phases[i]->autoIndicator ),
				event->phases[i]->autoIndicator,
				event->phases[i]->firstMotion );
		if( caux < 0 ) return -1;
		ccount += caux;

		/* Arrival time */
		if( event->phases[i]->at != 0 )
		{
			at = *datime( event->phases[i]->at, &at );
			if( ot.day != at.day )
			{
				if( ot.day == at.day - 1 )
					at.hour += 24;
			}
			caux = fprintf( str, "%2d%2d %5.2f ",
					at.hour, at.minute, at.second );
			if( caux < 0 ) return -1;
			ccount += caux;
		}
		else
		{
			/* Invalid time, leave blank */
			caux = fprintf( str, "           " );
			if( caux < 0 ) return -1;
			ccount += caux;
		}

		/* Coda duration */
		if( event->phases[i]->duration == 0 )
		{
			caux = fprintf( str, "    " );
		}
		else
		{
			caux = fprintf( str, "%4d", event->phases[i]->duration );
		}
		if( caux < 0 ) return -1;
		ccount += caux;

		/* Amplitude */
		if( event->phases[i]->amplitude == 0 )
		{
			caux = fprintf( str, "        " );
		}
		else
		{
			caux = fprintf( str, "%7.1f ", event->phases[i]->amplitude * LOCALMAG_AMP_MULTIPLIER );
		}
		if( caux < 0 ) return -1;
		ccount += caux;

		/* Period */
		if( event->phases[i]->period == 0 )
		{
			caux = fprintf( str, "     " );
		}
		else
		{
			caux = fprintf( str, "%4.1f ", event->phases[i]->period );
		}
		if( caux < 0 ) return -1;
		ccount += caux;

		/* Things that are not inserted */
		caux = fprintf( str, "                " );
		if( caux < 0 ) return -1;
		ccount += caux;

		/* Travel time residual */
		if( event->phases[i]->atRMS == 0 )
		{
			caux = fprintf( str, "        " );
		}
		else
		{
			caux = fprintf( str, "%5.1f   ", event->phases[i]->atRMS );
		}
		if( caux < 0 ) return -1;
		ccount += caux;

		/* Epicentral distance */
		if( event->phases[i]->distance == 0 )
		{
			caux = fprintf( str, "      " );
		}
		else
		{
			caux = fprintf( str, "%5.0f ", event->phases[i]->distance );
		}
		if( caux < 0 ) return -1;
		ccount += caux;

		/* Azimuth */
		if( event->phases[i]->azimuth == 0 )
		{
			caux = fprintf( str, "    \n" );
		}
		else
		{
			caux = fprintf( str, "%3d \n", event->phases[i]->azimuth );
		}
		if( caux < 0 ) return -1;
		ccount += caux;
	}
	/* Empty line to terminate event */
	caux = fprintf( str, "\n" );
	if( caux < 0 ) return -1;
	ccount += caux;
	return ccount;
}

/* Isolated function to describe a location */
int fwriteNorLoc( FILE* str, NordicLoc* location )
{
	int j;
	int ccount = 0, caux;
	char datestr[18];

	/* Origin time */
	date17( location->ot, datestr );
	caux = fprintf( str, " %.4s %.2s%.2s %.2s%.2s %.4s ",
			datestr, datestr + 4, datestr + 6,
			datestr + 8, datestr + 10, datestr + 12 );
	if( caux < 0 ) return -1;
	ccount += caux;

	/* Some direct stuff */
	caux = fprintf( str, "%c%c",
			location->distIndicator, location->eventType );
	if( caux < 0 ) return -1;
	ccount += caux;

	/* Location */
	if( location->isValid == 1 )
	{
		caux = fprintf( str, "%7.3f%8.3f%5.1f  %.3s%3d%4.1f",
				location->lat, location->lon,
				location->z, location->agency,
				location->nsta, location->rms );
		if( caux < 0 ) return -1;
		ccount += caux;
	}
	else
	{
		caux = fprintf( str, "                                " );
		if( caux < 0 ) return -1;
		ccount += caux;
	}

	/* Magnitudes */
	for( j = 0; j < 3; j++ )
	{
		if( location->magnitudes[j].isValid != 1 )
		{
			/* Not a valid magnitude */
			caux = fprintf( str, "        " );
			if( caux < 0 ) return -1;
			ccount += caux;
		}
		else
		{
			caux = fprintf( str, "%4.1f%c%.3s",
					location->magnitudes[j].mag,
					location->magnitudes[j].magtype,
					location->magnitudes[j].agency );
			if( caux < 0 ) return -1;
			ccount += caux;
		}
	}

	/* End of type 1 line */
	caux = fprintf( str, "1\n" );
	if( caux < 0 ) return -1;
	ccount += caux;

	return ccount;
}

/******************************************************************************
 * Parse a seisan message                                                     *
 ******************************************************************************/
int parseNordic( Nordic** event, char* buf, int nbuf )
{
	char line[81];
	int ccount = 0;
	int lpos = 0;
	char linetype;
	NordicLoc* curLoc = NULL;
	NordicPhase* curPhase = NULL;
	int i;
	char* magpos;
	char tstr[20];
	double otp;
	int hour;
	int min;
	double sec;
	char comment[81];
	char* cpos;
	//char command[80];
	//char value[80];


	/* Initialize new event */
	*event = InitializeEvent( );
	if( *event == NULL ) return FALSE;
	//if( Debug ) printf( "Allocated memory for new event\n" );

	/* Parse copy of buffer  line by line */
	while( ccount < nbuf )
	{
		/* Copy buffer to temporary line */
		if( buf[ccount] != 10 && buf[ccount] != 13 && lpos < 80 )
		{
			line[lpos++] = buf[ccount++];
			continue;
		}
		line[lpos] = '\0';
		lpos = 0;
		ccount++;
		if( buf[ccount] == 10 || buf[ccount] == 13 ) ccount++; // For windows

		//printf("LINE: <%s>\n", line );

		/* Determine type of line */
		if( strlen( line ) >= 80 )
		{
			linetype = line[79];
		}
		else
		{
			if( Debug )
				fprintf( stderr, "Line error: Less than 80 characters\n%s\n", line );
			continue;
		}

		/* Parse line */
		switch( linetype )
		{
			case '1':
				/********************************
				 * Location and magnitudes line *
				 ********************************/
				/* Initialize a new location */
				( *event )->locations[( *event )->nlocations] = InitializeLocation( );
				curLoc = ( *event )->locations[( *event )->nlocations];
				if( curLoc == NULL )
				{
					freeNordic( *event );
					return -1;
				}
				( *event )->nlocations++;


				/* Scan contents of line */


				/* Origin time */
				strncpy( tstr + 0, line + 1, 4 ); // Year
				strncpy( tstr + 4, line + 6, 2 ); // Month
				strncpy( tstr + 6, line + 8, 2 ); // Day
				strncpy( tstr + 8, line + 11, 2 ); // Hour
				strncpy( tstr + 10, line + 13, 2 ); // Minute
				strncpy( tstr + 12, line + 16, 4 ); // Second.Tenths of second
				strcpy( tstr + 16, "00" );
				for( i = 0; i < 17; i++ ) if( tstr[i] == ' ' ) tstr[i] = '0';
				curLoc->ot = julsec17( tstr );
				if( curLoc->ot == 0.0 )
				{
					if( Debug )
						fprintf( stderr, "Error parsing origin time in line:\n%s\n", line );
					freeNordic( *event );
					return -1;
				}

				/* Set primary location */
				if( ( *event )->primloc == NULL )
				{
					/* Set this location as primary */
					( *event )->primloc = curLoc;

					/* Prepare ot for phase arrival time calculation */
					date17( ( *event )->primloc->ot, tstr );
					for( i = 8; i < 14; i++ ) tstr[i] = '0';
					for( i = 15; i < 17; i++ ) tstr[i] = '0';
					otp = julsec17( tstr );
					//printf( "Primary: %s (%f)\n", tstr, otp );
				}


				/* Location model */
				curLoc->locModel = line[20];

				/* Distance indicator */
				curLoc->distIndicator = line[21];

				/* Event type */
				curLoc->eventType = line[22];

				/* Origin Location */
				strncpy( tstr, line + 23, 7 );
				tstr[7] = '\0';
				if( strcmp( tstr, "       " ) == 0 )
				{
					/* Location is invalid*/
					curLoc->isValid = 0;
					curLoc->hasZ = 0;
				}
				else
				{
					curLoc->isValid = 1;
					curLoc->lat = ( float ) atof( tstr );

					/* Longitude */
					strncpy( tstr, line + 30, 8 );
					tstr[8] = '\0';
					curLoc->lon = ( float ) atof( tstr );

					/* Depth */
					strncpy( tstr, line + 38, 5 );
					tstr[5] = '\0';
					if( strcmp( tstr, "     " ) == 0 )
					{
						/* Does not have depth */
						curLoc->hasZ = 0;
					}
					else
					{
						curLoc->z = ( float ) atof( tstr );
						curLoc->hasZ = 1;
						/* Detph indicator */
						curLoc->zIndicator = line[43];
					}

					/* Location indicator */
					curLoc->lIndicator = line[44];

					/* Agency */
					strncpy( curLoc->agency, line + 45, 3 );
					curLoc->agency[3] = '\0';

					/* Number of stations */
					strncpy( tstr, line + 48, 3 );
					tstr[3] = '\0';
					curLoc->nsta = atoi( tstr );

					/* RMS */
					strncpy( tstr, line + 51, 4 );
					tstr[4] = '\0';
					curLoc->rms = ( float ) atof( tstr );
				}

				/* Magnitudes */
				magpos = line + 55;

				for( magpos = line + 55, i = 0;
						i < 3;
						magpos += ( 4 + 1 + 3 ), i++ )
				{

					/* Check magnitude value */
					strncpy( tstr, magpos, 4 );
					tstr[4] = '\0';
					if( strcmp( tstr, "    " ) == 0 )
					{
						/* No magnitude present */
						curLoc->magnitudes[i].isValid = 0;
						continue;
					}

					/* Initialize this magnitude */
					curLoc->magnitudes[i].mag = ( float ) atof( tstr );
					curLoc->magnitudes[i].isValid = 1;

					/* Magnitude type */
					curLoc->magnitudes[i].magtype = magpos[4];

					/* Agency */
					strncpy( curLoc->magnitudes[i].agency, magpos + 5, 3 );
					curLoc->magnitudes[i].agency[3] = '\0';
				}
				break;

			case '4':
			case ' ':
				/********************************
				 * Phase line                   *
				 ********************************/
				/* Make sure this is not the end of an event */
				if( line[1] == ' ' )
				{
					/* End of event */
					break;
				}

				/* Allocate memory for new phase */
				curPhase = InitializePhase( );
				if( curPhase == NULL )
				{
					freeNordic( *event );
					return -1;
				}
				( *event )->phases[( *event )->nphases] = curPhase;
				( *event )->nphases++;

				/* Station */
				strncpy( curPhase->station, line + 1, 5 );
				for( i = 0; i < 5; i++ )
					if( curPhase->station[i] == ' ' )
					{
						curPhase->station[i] = '\0';
						break;
					}

				/* Instrument and Component */
				curPhase->channel[0] = *( line + 6 );
				curPhase->channel[1] = ' ';
				curPhase->channel[2] = *( line + 7 );

				/* Quality indicator */
				curPhase->qualityIndicator = *( line + 9 );

				/* Phase ID */
				strncpy( curPhase->phaseIndicator, line + 10, 4 );
				for( i = 0; i < 5; i++ )
					if( curPhase->phaseIndicator[i] == ' ' )
					{
						curPhase->phaseIndicator[i] = '\0';
						break;
					}

				/* Weight */
				curPhase->weight = ( line[14] == ' ' ) ? 0 : ( line[14] - '0' );

				/* Auto pick */
				curPhase->autoIndicator = line[15];

				/* First motion */
				curPhase->firstMotion = line[16];

				/* Arrival time */
				strncpy( tstr, line + 18, 2 );
				tstr[2] = '\0';
				hour = atoi( tstr );
				strncpy( tstr, line + 20, 2 );
				tstr[2] = '\0';
				min = atoi( tstr );
				strncpy( tstr, line + 23, 5 );
				tstr[5] = '\0';
				sec = atof( tstr );
				/* Compute final arrival time */
				curPhase->at = otp + ( double ) hour * 3600 +
						( double ) min * 60 + sec;

				/* Duration */
				curPhase->duration = ( int ) readdouble( line + 29, 4 );

				/* Amplitude */
				curPhase->amplitude = readdouble( line + 33, 7 );

				/* Period */
				curPhase->period = readdouble( line + 41, 4 );

				/* Direction of approach */
				curPhase->dirApproach = readdouble( line + 46, 5 );

				/* Phase velocity */
				curPhase->velocity = readdouble( line + 52, 4 );

				/* Travel time residual */
				curPhase->atRMS = readdouble( line + 63, 5 );

				/* Epicentral distance */
				curPhase->distance = readdouble( line + 70, 5 );

				/* Azimuth */
				curPhase->azimuth = ( int ) readdouble( line + 76, 3 );
				break;

			case '3':
				/********************************
				 * Comment line                 *
				 ********************************/
				for( i = 78; i >= 0 && line[i] == ' '; i-- );
				strncpy( comment, line + 1, i );
				comment[i] = '\0';
				if( addNordicComment( *event, comment ) == -1 )
				{
					freeNordic( *event );
					return -1;
				}
				break;

			case 'E':
				/********************************
				 * Error line                   *
				 ********************************/
				/* Initialize Hypocenter error line */
				( *event )->hypErr = ( NordicHypError* ) calloc( 1, sizeof ( NordicHypError ) );
				if( ( *event )->hypErr == NULL )
				{
					freeNordic( *event );
					return -1;
				}
				/* Azimuthal gap */
				( *event )->hypErr->gap = ( int ) readdouble( line + 5, 3 );
				break;
			case '6':
				/********************************
				 * Waveform reference line      *
				 ********************************/
				/* Isolate reference name */
				for( cpos = line + 78; cpos > line && *cpos == ' '; cpos-- );
				if( cpos == line ) break; /* empty reference */
				strncpy( comment, line + 1, cpos - line );
				comment[ cpos - line ] = '\0';
				if( NorAddWaveRef( *event, comment ) != 1 )
				{
					freeNordic( *event );
					return -1;
				}
		}
	};

	/* Process comments, if required */
	fixChannelIds( *event );
	return 1;
}

double readdouble( char* source, int len )
{
	char temp[81];
	char zeros[] = "                                        "
			"                                         ";
	strncpy( temp, source, len );
	temp[len] = '\0';
	zeros[len] = '\0';
	if( strcmp( temp, zeros ) == 0 )
	{
		return 0.0;
	}
	return atof( temp );
}

int parseCommentCommand( char* command, char* value, char* source )
{
	int i, pos;

	/* Find ':' character */
	for( pos = 0, i = strlen( source ); pos < i && source[pos] != ':'; pos++ );
	if( pos == i ) return -1;

	/* Isolate command */
	strncpy( command, source, pos );
	command[ pos ] = '\0';

	/* Find value limits */
	for( i--, pos++; i > pos && source[i] == ' '; i-- );
	strncpy( value, source + pos + 1, i - pos );
	value[ i - pos ] = '\0';
	return 1;
}

void fixChannelIds( Nordic* event )
{
	unsigned int i, j;
	char command[80];
	char value[80];
	//int score = 0;
	//char curId[50];
	char sta[6], cha[4], net[3], loc[3];
	char* ptrs;
	char* ptrd;

	for( i = 0; i < event->ncomments; i++ )
	{
		if( parseCommentCommand( command, value, event->comments[i] ) == -1 )
			continue;
		if( strcmp( command, CHANNEL_ID_COMMAND ) == 0 )
		{
			/* Found a channel ID */
			loc[0] = '\0';
			//printf( "channelID -> %s\n", event->comments[i] );
			for( ptrs = value, ptrd = sta; *ptrs != '.' && ptrd < ( sta + 5 ); ptrs++, ptrd++ )
				*ptrd = *ptrs;
			*ptrd = '\0';
			for( ptrs++, ptrd = cha; *ptrs != '.' && ptrd < ( cha + 3 ); ptrs++, ptrd++ )
				*ptrd = *ptrs;
			*ptrd = '\0';
			for( ptrs++, ptrd = net; *ptrs != '.' && ptrd < ( net + 2 ); ptrs++, ptrd++ )
				*ptrd = *ptrs;
			*ptrd = '\0';
			for( ptrs++, ptrd = loc; ptrd < ( loc + 2 ); ptrs++, ptrd++ )
				*ptrd = *ptrs;
			*ptrd = '\0';
			//printf( "STA: %s.%s.%s.%s\n", sta, cha, net, loc );


			// Find the most similar phases
			for( j = 0; j < event->nphases; j++ )
			{
				//printf( "%s\n", event->phases[j]->station );
				if( strcmp( event->phases[j]->station, sta ) == 0 )
				{
					/* Found a matching station - Check channel characters */
					if( event->phases[j]->channel[0] == cha[0] &&
							event->phases[j]->channel[2] == cha[2] )
					{
						/* This is a good candidate, with maximum match */
						if( strlen( event->phases[j]->network ) == 0 )
						{
							strcpy( event->phases[j]->channel, cha );
							strcpy( event->phases[j]->network, net );
							if( loc[0] != '\0' )
								strcpy( event->phases[j]->location, loc );
						}
					}
				}
			}
		}
	}
}

/******************************************************************************
 * Add a waveform reference to an event                                       *
 ******************************************************************************/
int NorAddWaveRef( Nordic* event, char* waveref )
{
	/* Reserve memory for waveform reference */
	event->waverefs[event->nwaveref] = ( char* ) calloc( strlen( waveref ) + 1,
			sizeof ( char ) );
	if( event->waverefs[event->nwaveref] == NULL )
		return -1;
	strcpy( event->waverefs[event->nwaveref], waveref );
	event->nwaveref++;

	return 1;
}








/******************************************************************************
 * Convert an arc message to seisan format                                    *
 ******************************************************************************/

/* Input parameters:
 * FILE dest - Stream to write data
 * char* msgbuf - Null terminated nordic message*/
int arc2nor( Nordic **event, HypoArc *arc, MAG_INFO *mag, char* agency )
{
	NordicMag* curmag; /* Temporary magnitude pointer */
	unsigned int nmag = 1;
	char IdStr[80];
	unsigned int i, j;
	MAG_CHAN_INFO* staMag;

	/* Check number of magnitudes to use */
	if( mag != NULL )
		nmag = 2; /* Md + Ml */

	/* Allocate memory for new seisan event */
	//*event = ( Nordic* ) malloc( sizeof( Nordic ) );
	//if( *event == NULL ) return FALSE;
	//if( Debug ) printf( "Allocated memoty for event.\n" );
	*event = InitializeEvent( );
	if( *event == NULL ) return FALSE;




	/**********************
	 * Origin Information *
	 **********************/

	/* Allocate memory for new location */
	( *event )->locations[0] = InitializeLocation( ); //( NordicLoc* ) malloc( sizeof( NordicLoc ) );
	if( ( *event )->locations[0] == NULL )
	{
		free( *event );
		return FALSE;
	}
	( *event )->nlocations = 1;
	if( Debug ) printf( "Allocated memory for location.\n" );

	/* Populate new location */
	( *event )->locations[0]->ot = arc->sum.ot;
	( *event )->locations[0]->locModel = ' '; /* Use default location model */
	( *event )->locations[0]->distIndicator = 'L'; /* For now assume all locations local */
	( *event )->locations[0]->eventType = 'Q'; /* For now assume all events as quakes */
	( *event )->locations[0]->lat = arc->sum.lat;
	( *event )->locations[0]->lon = arc->sum.lon;
	( *event )->locations[0]->z = arc->sum.z;
	( *event )->locations[0]->hasZ = 1;
	strncpy( ( *event )->locations[0]->agency, agency, 3 );
	( *event )->locations[0]->nsta = 0; /* TODO: THIS MUST BE UPDATED LATER */
	( *event )->locations[0]->rms = arc->sum.rms;
	( *event )->primloc = ( *event )->locations[0];
	( *event )->primloc->isValid = 1;

	/* Introduce duration magnitude */
	curmag = ( *event )->locations[0]->magnitudes;

	/* First ML */
	if( nmag == 2 )
	{
		curmag->mag = ( float ) mag->mag;
		curmag->magtype = 'L';
		strncpy( curmag->agency, agency, 3 );
		//( *event )->locations[0]->nmagnitudes++;
		curmag->isValid = 1;
		/* Set curmag to handle Md*/
		curmag++;
	}
	/* Second, Md */
	curmag->mag = arc->sum.Md;
	curmag->magtype = 'C'; /* Code for duration magnitudes */
	strncpy( curmag->agency, agency, 3 );
	//( *event )->locations[0]->nmagnitudes++;
	curmag->isValid = 1;


	/**********************
	 * Error Information  *
	 **********************/
	( *event )->hypErr = ( NordicHypError* ) malloc( sizeof ( NordicHypError ) );
	if( ( *event )->hypErr == NULL )
	{
		freeNordic( *event );
		return -1;
	}
	( *event )->hypErr->gap = arc->sum.gap;
	( *event )->hypErr->ot_err = 0; /* Could not find an equivalent in arc */
	/* The following is totally my invention. Probably VERY wrong */
	( *event )->hypErr->lat_err = 0;//arc->sum.erh * cos( arc->sum.e0az );
	( *event )->hypErr->lon_err = 0;//arc->sum.erh * sin( arc->sum.e0az );
	( *event )->hypErr->z_err = 0;//arc->sum.erz;
	( *event )->hypErr->cov_xy = 0;
	( *event )->hypErr->cov_xz = 0;
	( *event )->hypErr->cov_yz = 0;

	/**********************
	 * Event ID           *
	 **********************/
	snprintf( IdStr, 80, "%s: %ld", BINDER_ID_COMMAND, arc->sum.qid );
	if( addNordicComment( ( *event ), IdStr ) == -1 )
	{
		freeNordic( *event );
		return -1;
	}

	/**********************
	 * Version ID         *
	 **********************/
	snprintf( IdStr, 80, "%s: %ld", VERSION_ID_COMMAND, arc->sum.version );
	if( addNordicComment( ( *event ), IdStr ) == -1 )
	{
		freeNordic( *event );
		return -1;
	}

	/**********************
	 * Phases             *
	 **********************/
	( *event )->nphases = 0;
	/* Arrival phases */
	for( i = 0; i < ( unsigned int ) arc->num_phases; i++ )
	{
		if( addNordicPhase( *event, arc->phases[i] ) == -1 )
		{
			freeNordic( *event );
			return -1;
		}
	}

	/* Number of stations */
	for( i = 0, ( *event )->primloc->nsta = 0; i < ( *event )->nphases; i++ )
	{
		/* Compare this station with the previous ones */
		for( j = 0; j <= i; j++ )
		{
			if( strcmp( ( *event )->phases[j]->station,
					( *event )->phases[i]->station ) == 0 )
			{
				/* Found an identical station */
				if( i == j )
					( *event )->primloc->nsta++;
				else
					break;
			}
		}
	}

	/* Magnitude phases */
	if( mag != NULL )
	{
		for( i = 0; i < ( unsigned int ) mag->nchannels; i++ )
		{
			staMag = ( MAG_CHAN_INFO* ) ( mag->pMagAux + i * sizeof ( MAG_CHAN_INFO ) );
			if( addNordicAmpPhase( *event, staMag ) == -1 )
			{
				freeNordic( *event );
				return -1;
			}
		}
	}
	//if( Debug ) printf( "Added a total of %d phases\n", ( int ) ( *event )->nphases );

	return TRUE;
}

/* Function to add comments to a nordic event */
int addNordicComment( Nordic* event, char* cmt )
{
	char** comments;

	comments = ( char** ) realloc( event->comments, sizeof ( char* )
			* ( event->ncomments + 1 ) );
	if( comments == NULL ) return -1;
	//if( Debug ) printf( "Allocated memory for comment pointer.\n" );
	event->comments = comments;
	event->comments[event->ncomments] = ( char* ) malloc( sizeof ( char )
			* ( strlen( cmt ) + 1 ) );
	if( event->comments[event->ncomments] == NULL ) return -1;
	//if( Debug ) printf( "Allocated memory for comment.\n" );
	strcpy( event->comments[event->ncomments], cmt );
	event->ncomments++;
	return 1;
}

/* Function to add a phase to a nordic event given a arc phase*/
int addNordicPhase( Nordic* event, struct Hpck *arcphase )
{
	NordicPhase* phase;
	char channelIDcmd[81];

	/* Allocate memory for new phase */
	phase = InitializePhase( );
	if( phase == NULL ) return -1;
	event->phases[event->nphases] = phase;
	event->nphases++;
        if( event->nphases >= MAX_NOR_PHASES) return -1;

	/**********************
	 * Station            *
	 **********************/
	strcpy( phase->station, arcphase->site );
	strcpy( phase->channel, arcphase->comp );


	/* Although seisan stations do not have network and location, we add them
	 * here for backwards compatibility 
	 **********************************/
	strcpy( phase->network, arcphase->net );
	strcpy( phase->location, arcphase->loc );

	/* Add a comment with the full channel name */
	snprintf( channelIDcmd, 80, "%s: %s.%s.%s.%s", CHANNEL_ID_COMMAND,
			phase->station, phase->channel, phase->network, phase->location );
	if( addNordicComment( event, channelIDcmd ) == -1 )
	{
		return -1;
	}

	/* Quality indicator */
	phase->qualityIndicator = 'I';

	/* Automatic calculation indicator */
	phase->autoIndicator = 'A';

	/* Coda duration */
	phase->duration = arcphase->codalen;

	/* Azimuth */
	phase->azimuth = arcphase->azm;

	/* Distance */
	phase->distance = arcphase->dist;

	/* Phase type */
	if( arcphase->Plabel == 'P' || arcphase->Ponset == 'P' )
	{
		/* Arrival time */
		phase->at = arcphase->Pat;

		/* Weight */
		phase->weight = arcphase->Pqual;

		/* Phase indicator */
		phase->phaseIndicator[0] = 'P'; /* Only P or S phases */

		/* First Motion */
		if( arcphase->Pfm == 'U' )
			phase->firstMotion = 'D';
		else if( arcphase->Pfm == 'D' )
			phase->firstMotion = 'C';
		else
			phase->firstMotion = arcphase->Pfm;

		/* Residual */
		phase->atRMS = arcphase->Pres;
	}
	else // If not a P phase, assume its an S phase. Non P or S phases are assumed S
	{
		/* Arrival time */
		phase->at = arcphase->Sat;

		/* Weight */
		phase->weight = arcphase->Squal;

		/* Phase indicator */
		phase->phaseIndicator[0] = 'S'; /* Only P or S phases */

		/* First Motion */
		phase->firstMotion = arcphase->Sfm;

		/* Residual */
		phase->atRMS = arcphase->Sres;
	}


	return 1;

}
/* returns -1 if phases overflow static buffer, or 1 if OK */
int addNordicAmpPhase( Nordic* event, MAG_CHAN_INFO *magphase )
{
	NordicPhase* phase;


	/* check if we reached maximum number of static phases allowed */

	/* Allocate memory for new phase */
	phase = InitializePhase( );
	if( phase == NULL ) return -1;
	event->phases[event->nphases] = phase;
	event->nphases++;
	if( event->nphases >= MAX_NOR_PHASES) return -1;

	/**********************
	 * Station            *
	 **********************/
	strncpy( phase->station, magphase->sta, 5 );
	strncpy( phase->channel, magphase->comp, 3 );

	/* Although seisan stations do not have network and location, we add them
	 * here for backwards compatibility 
	 **********************************/
	strncpy( phase->network, magphase->net, 2 );
	strncpy( phase->location, magphase->loc, 2 );
	//printf( "Location: <%s>\n", magphase->loc );


	/* Automatic calculation indicator */
	phase->autoIndicator = 'A';

	/* Phase Indicator */
	strcpy( phase->phaseIndicator, "IAML" ); // For amplitude magnitudes

	/* Time */
	phase->at = magphase->Time1;

	/* Amplitude */
	/* localmag either gives z2p in amp1 or min and max of p2p in amp1 and amp2 */
	if( magphase->Amp2 == MAG_NULL ) /* MAG_NULL is -1 */
		phase->amplitude = magphase->Amp1 * 2;
	else
		phase->amplitude = magphase->Amp2 - magphase->Amp1;

	/* Period */
	//phase->period = magphase->Period1;

	return 1;
}

/******************************************************************************
 * Convert a seisan event into an arc structure                               *
 ******************************************************************************/
int nor2arc( HypoArc* arc, Nordic *event )
{
	unsigned int i;
	NordicPhase* sphase;
	struct Hpck* dphase;

	/* qid */
	if( findNumericalCommand( &( arc->sum.qid ), BINDER_ID_COMMAND, event ) == -1 )
		arc->sum.qid = 0;

	/* version */
	if( findNumericalCommand( &( arc->sum.version ), VERSION_ID_COMMAND, event ) == -1 )
		arc->sum.version = 0;

	/* ot */
	arc->sum.ot = event->primloc->ot;


	/* Location */
	if( event->primloc->isValid != 1 )
	{
		/* The primary location is invalid. Cannot be converted */
		return -1;
	}
	/* The primary location is valid... Everything Ok */
	arc->sum.lat = event->primloc->lat;
	arc->sum.lon = event->primloc->lon;
	arc->sum.z = event->primloc->z;

	/* Number of P and S phases and minimum distance*/
	/* Todo: FIX THE PHASE THRESHOLDS FOR THE COUNT */
	arc->sum.dmin = ( int ) event->phases[0]->distance;
	for( i = 0; i < event->nphases; i++ )
	{
		if( event->phases[i]->phaseIndicator[0] == 'P' )
		{
			arc->sum.nphtot++;
			arc->sum.nph++;
			arc->sum.nPfm++;
		}
		if( event->phases[i]->phaseIndicator[0] == 'S' )
		{
			arc->sum.nphtot++;
			arc->sum.nph++;
			arc->sum.nphS++;
		}
		if( event->phases[0]->distance < arc->sum.dmin )
			arc->sum.dmin = ( int ) event->phases[0]->distance;
	}

	/* Az. Gap */
	if( event->hypErr != NULL )
	{
		arc->sum.gap = event->hypErr->gap;
	}

	/* RMS */
	arc->sum.rms = event->primloc->rms;

	/* Magnitude */
	/* Find a primary duration magnitude, if available */
	arc->sum.mdtype = ' ';
	arc->sum.labelpref = ' ';
	for( i = 0; i < 3; i++ )
	{
		if( event->primloc->magnitudes[i].isValid != 1 )
			continue;
		if( event->primloc->magnitudes[i].magtype == 'C' )
		{
			arc->sum.Md = event->primloc->magnitudes[i].mag;
			arc->sum.mdtype = 'D';
			arc->sum.labelpref = 'D';
			arc->sum.Mpref = arc->sum.Md;
			break;
		}
	}

	/* Phases */
	for( i = 0; i < event->nphases && i < MAX_PHASES; i++ )
	{
		sphase = event->phases[i];
		dphase = arc->phases[i];

		/* Check if this is an arrival phase */
		if( sphase->phaseIndicator[0] != 'P' &&
				sphase->phaseIndicator[0] != 'S' )
			continue;

		/* Allocate memory for phase */
		dphase = ( struct Hpck* ) calloc( 1, sizeof ( struct Hpck ) );
		if( dphase == NULL )
		{
			free_phases( arc );
			return -1;
		}
		arc->phases[arc->num_phases] = dphase;

		/* Increment number of phases */
		arc->num_phases++;

		/* SCNL */
		strcpy( dphase->site, sphase->station );
		strcpy( dphase->comp, sphase->channel );
		strcpy( dphase->net, sphase->network );
		strcpy( dphase->loc, sphase->location );

		/* Switch depending on Phase label */
		if( sphase->phaseIndicator[0] == 'P' )
		{
			dphase->Ponset = 'P'; /* Label */
			dphase->Sonset = ' ';
			dphase->Pat = sphase->at; /* Arrival time */
			dphase->Pres = ( float ) sphase->atRMS; /* RMS */
			dphase->Pqual = sphase->weight; /* Quality */
			/* First motion */
			dphase->Sfm = ' ';
			if( sphase->firstMotion == 'C' )
				dphase->Pfm = 'D';
			else if( sphase->firstMotion == 'D' )
				dphase->Pfm = 'U';
			else
				dphase->Pfm = sphase->firstMotion;
		}
		else
		{
			dphase->Ponset = ' ';
			dphase->Sonset = 'S'; /* Label */
			dphase->Sat = sphase->at; /* Arrival time */
			dphase->Sres = ( float ) sphase->atRMS; /* RMS */
			dphase->Squal = sphase->weight; /* Quality */
			/* First motion */
			dphase->Pfm = ' ';
			if( sphase->firstMotion == 'C' )
				dphase->Sfm = 'D';
			else if( sphase->firstMotion == 'D' )
				dphase->Sfm = 'U';
			else
				dphase->Sfm = sphase->firstMotion;
		}

		dphase->codalen = sphase->duration; /* Coda */
		/* Todo: Coda weigth */
		dphase->azm = sphase->azimuth; /* Azimuth */

		dphase->datasrc = ' '; /* Data source */
	}


	return 1;
}

int findNumericalCommand( long* target, char* command, Nordic* event )
{
	unsigned int i;
	char value[80];
	char ocommand[80];
	for( i = 0; i < event->ncomments; i++ )
	{
		if( parseCommentCommand( ocommand, value, event->comments[i] ) == -1 )
			continue;
		if( strcmp( command, ocommand ) == 0 )
		{
			/* Found the right command */
			*target = ( long ) atoi( value );
			return 1;
		}
	}
	return -1;
}

/******************************************************************************
 * Initialization Functions                                                   *
 ******************************************************************************/
Nordic* InitializeEvent( void )
{
	Nordic* event;
	int i;

	/* Allocate memory for new seisan event */
	event = ( Nordic* ) calloc( 1, sizeof ( Nordic ) );
	if( event == NULL ) return NULL;

	event->primloc = NULL;
	event->nlocations = 0;
	event->hypErr = NULL;
	event->nphases = 0;
	event->comments = NULL;
	event->ncomments = 0;
	for( i = 0; i < MAX_NOR_LOCATIONS; i++ )
		event->locations[i] = NULL;
	for( i = 0; i < MAX_NOR_PHASES; i++ )
		event->phases[i] = NULL;

	/* Done! */
	return event;

}

NordicLoc* InitializeLocation( void )
{
	NordicLoc* location;

	/* Allocate memory for new location */
	location = ( NordicLoc* ) malloc( sizeof ( NordicLoc ) );
	if( location == NULL )
	{
		return NULL;
	}

	/* Defaults */
	location->ot = 0.0;
	location->locModel = ' ';
	location->distIndicator = ' ';
	location->eventType = ' ';
	location->lat = 0.0;
	location->lon = 0.0;
	location->z = 0;
	location->hasZ = 0;
	strcpy( location->agency, "   " );
	location->nsta = 0;
	location->rms = 0.0;
	//location->nmagnitudes = 0;

	return location;
}

NordicPhase* InitializePhase( void )
{
	NordicPhase* phase;
	char aux[9];
	int i;

	/* Allocate memory */
	phase = ( NordicPhase* ) malloc( sizeof ( NordicPhase ) );
	if( phase == NULL ) return NULL;

	/* Set default values */
	for( i = 0; i < 6; i++ ) aux[i] = '\0';
	strncpy( phase->station, aux, 5 );
	strncpy( phase->channel, aux, 3 );
	strncpy( phase->network, aux, 2 );
	strncpy( phase->location, aux, 2 );
	phase->weight = 0;
	phase->qualityIndicator = ' ';
	phase->autoIndicator = ' ';
	strncpy( phase->phaseIndicator, aux, 8 );
	phase->firstMotion = ' ';
	phase->duration = 0;
	phase->amplitude = 0.0;
	phase->period = 0.0;
	phase->azimuth = 0;
	phase->atRMS = 0;
	phase->distance = 0;

	return phase;
}

/******************************************************************************
 * Finalization Functions                                                     *
 ******************************************************************************/
void freeNordic( Nordic *event )
{

	unsigned int i;

	if( event == NULL ) return;
	/* Free all locations */
	for( i = 0; i < event->nlocations; i++ )
		freeNordicLocation( event->locations[i] );

	/* Free all phases */
	for( i = 0; i < event->nphases; i++ )
		freeNordicPhase( event->phases[i] );
	/* Free hypocenter error */
	if( event->hypErr != NULL ) free( event->hypErr );

	/* Free event */
	free( event );
}

void freeNordicLocation( NordicLoc* location )
{

	if( location == NULL ) return;
	free( location );
}

void freeNordicPhase( NordicPhase* phase )
{
	if( phase == NULL ) return;
	free( phase );
}
