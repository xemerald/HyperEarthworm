/* PSN4PutAway.c - Save data to PSN Type 4 Event Files
   Created Dec 14 2005 by Larry Cochrane, Redwood City, PSN
   Updated July 8 2006 - Replaced TRACE_HEADER with TRACE2_HEADER
   Updated Jan 26 2012 - Fixed problem with saving more then one file 
 */

/************************************/
/** #includes                      **/
/************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <earthworm.h>
#include <time.h>
#include <trace_buf.h>
#include <swap.h>
#include <pa_subs.h>
#include <time_ew.h>
#include <math.h>

#ifndef WIN32
#define WORD unsigned short
#define BYTE unsigned char
#define UINT unsigned int			// needs to be 4 bytes
#endif

#include <psn4putaway.h>

static char *DataloggerString = "Earthworm 7.x";

static PSNType4 PsnHeader;		/* header that get saves to disk */
static int32_t *BufferInt32 = 0;	/* buffer for storing output before writing */
static short *BufferShort = 0;  /* write out data as short integers */
static float *BufferFloat = 0;  /* write out data as float */
static char FileNameRoot[256];		/* holds the file name we will be using */
static char SaveDir[256];		/* holds the directory we will be using */
static int GoodData = 0;		/* used to check for deleting empty file */
static char StaLocation[256];	/* station location string */
static char SensorInfo[256];	/* sensor info string */
static ChannelInfo ChanInfo;	/* holds additional info about the channel */

static void MakeHdrTime( DateTime *, double );
static void ReadStationFile( char *scnl, PSNType4 *hdr, int debug  );
static void ParseLine( char *, char *, char * );
static char *SkipSpace( char *string );
static int Strib( char *string );
static void SwapPsnHeader( PSNType4 * );
static void SwapPsnVarHeader( VarHeader * );
static void SwapChanInfo( ChannelInfo *);
static WORD CalcFileCRC( FILE *fp, int length );
static void CalcCRC16(WORD *crc, BYTE *cp, int cnt);

/* Initialization function,
*       This is the Put Away startup intializer. This is called when    *
*       the system first comes up. Here is a chance to look around      *
*       and see if it's possible to do business, and to complain        *
*       if not ,BEFORE an event has to be processed.                    *
*/
int PSN4PA_init( long OutBufferLen, char *OutDir, char *OutputFormat, int debug )
{
	int len;

	if( ( BufferInt32 = (int32_t *)malloc( OutBufferLen * sizeof (char) ) ) == NULL )  {
		logit ("et", "PSN4PA_init: couldn't malloc BufferInt32\n");
		return EW_FAILURE;
	}
	if( ( BufferFloat = (float *)malloc( OutBufferLen * sizeof (char) ) ) == NULL )  {
		logit ("et", "PSN4PA_init: couldn't malloc BufferFloat\n");
		return EW_FAILURE;
	}
	if( ( BufferShort = (short *)malloc( OutBufferLen * sizeof (char) ) ) == NULL )  {
		logit ("et", "PSN4PA_init: couldn't malloc BufferShort\n");
		return EW_FAILURE;
	}

	strcpy( SaveDir, OutDir );

	/* remove trailing '\' or '/' char in the directory string */
	len = (int)strlen( SaveDir );
	if( len > 2 && ( SaveDir[len-1] == '\\' ||  SaveDir[len-1] == '/' ) )
		SaveDir[len-1] = 0;

	/* Make sure that the top level output directory exists */
	CreateDir( SaveDir );

	return EW_SUCCESS;
}

/****************************************************************************
* This is the Put Away event initializer. It's called when a snippet	    *
* has been received, and is about to be processed.							*
* It gets to see the pointer to the TraceRequest array,						*
* and the number of loaded trace structures.								*
*****************************************************************************/
int PSN4PA_next_ev( TRACE_REQ *ptrReq, char *EventDate, char *EventTime, int debug )
{
	char toDir[ 256 ];
	
#ifdef WIN32
	sprintf( toDir, "%s\\%-4.4s", SaveDir, &EventDate[2] );
	sprintf( FileNameRoot, "%s\\%-6.6s.%-6.6s", toDir, &EventDate[2], EventTime );
#else
	sprintf( toDir, "%s/%-4.4s", SaveDir, &EventDate[2] );
	sprintf( FileNameRoot, "%s/%-6.6s.%-6.6s", toDir, &EventDate[2], EventTime );
#endif	
	CreateDir( toDir );

	if( debug )
		logit ("t", "PSN 4 Root File Name %s\n", FileNameRoot );

	GoodData = 0;
	return(EW_SUCCESS);
}

/*****************************************************************************
 * This is the working entry point into the disposal system. This routine	 *
 * gets called for each trace snippet which has been recovered. It gets		 *
 * to see the corresponding SNIPPET structure, and the event id				 *
 *****************************************************************************/
/* Process one channel of data */
int PSN4PA_next( TRACE_REQ *getThis, double GapThresh, long OutBufferLen, int debug)
{
	TRACE2_HEADER *wf;
	PSNType4 *hdr = &PsnHeader;
	short *s_data, s;
	int32_t *l_data, l;
	float *f_data, f;
	char *msg_p;				/* pointer into tracebuf data */
	char datatype;
	int qual = 0, j, data_size;
	int gap_count = 0;
	long nsamp, nfill;
	long nfill_max = 0l;
	long nsamp_this_scnl = 0l;
	long this_size;
	double begintime, starttime, endtime;
	double samprate;
	int32_t fill = 0l;
	char scnl[80], fname[ 256 ];
	double min, max, mean;
	int len, varLen, crcLen;
	VarHeader varHdr;
	int swap = FALSE;
	WORD crc;
	FILE *psnFile = 0;		/* file pointer used to write out event file */
	int haveVarHdr = 0;
	
#if defined (_SPARC)
	swap = TRUE;
#endif

	/* Check arguments */
	if( getThis == NULL )  {
		logit ("et", "PSN4PA_next: invalid argument passed in.\n");
		return EW_FAILURE;
	}

	memset( hdr, 0, sizeof( PSNType4 ) );

	if( (msg_p = getThis->pBuf) == NULL )  {	 /* pointer to first message */
		logit ("et", "PSN4PA_next: Message buffer is NULL.\n");
		return EW_FAILURE;
	}
	wf = (TRACE2_HEADER *)msg_p;

	if( !strncmp( wf->loc, "--", 2 ) || !strncmp( wf->loc, "NC", 2 ) )
		sprintf( fname, "%s.%s.%s.%s.psn", FileNameRoot, wf->sta, wf->chan, wf->net );
	else
		sprintf( fname, "%s.%s.%s.%s.%s.psn", FileNameRoot, wf->sta, wf->chan, wf->net, wf->loc );
	_strlwr( fname );
	
	if( debug == 1 )
		logit ("t", "Opening PSN Type 4 file %s\n", fname );

	if( ( psnFile = fopen( fname, "w+b" ) ) == NULL )  {
		logit ("et", "PSN4PA_init: couldn't open output file %s\n", fname );
		return EW_FAILURE;
	}
	
	/* Look at the first TRACE2_HEADER and get set up of action */
	if (WaveMsg2MakeLocal(wf) < 0)  {
		logit("et", "PSN4PA_next: unknown trace data type: %s\n", wf->datatype);
		fclose( psnFile );
		return( EW_FAILURE );
	}
	nsamp = wf->nsamp;
	starttime = wf->starttime;
	endtime = wf->endtime;
	samprate = wf->samprate;
	if (samprate < 0.01 ) {
		logit("et", "unreasonable sample rate (%f) for <%s.%s.%s>\n", samprate,
			wf->sta, wf->chan, wf->net);
		fclose( psnFile );
		return( EW_FAILURE );
	}
	begintime = starttime;
	datatype = 'n';
	if( wf->datatype[0] == 's' || wf->datatype[0] == 'i' )  {
		if(wf->datatype[1] == '2')
			datatype = 's';
		else if(wf->datatype[1] == '4')
			datatype = 'l';
	}
	else if( wf->datatype[0] == 't' || wf->datatype[0] == 'f' && wf->datatype[1] == '4' )
		datatype = 'f';

	if (datatype == 'n')  {
		logit("et", "PSN4PA_next: unsupported datatype: %s\n", datatype);
		fclose( psnFile );
		return( EW_FAILURE );
	}

	if (debug == 1)
		logit("et", "PSN4PA_next: working on <%s/%s/%s> datatype: %c\n",
			wf->sta, wf->chan, wf->net, datatype );

	min = 999999999.0;
	max = -999999999.0;
	mean = 0.0;

	/* loop through all the messages for this s-c-n */
	while (1) {
		/* advance message pointer to the data */
		msg_p += sizeof(TRACE2_HEADER);

		/* check for sufficient memory in output buffer */
		this_size = (nsamp_this_scnl + nsamp ) * sizeof(int32_t);
		if ( OutBufferLen < this_size )  {
			logit( "et", "out of space for <%s.%s.%s>; saving long trace.\n",
				 wf->sta, wf->chan, wf->net);
			break;
		}

		switch( datatype )
		{
		case 's':
			s_data = (short *)msg_p;
			for ( j = 0; j < nsamp ; j++, nsamp_this_scnl++ )  {
				s = s_data[j];
				if( s > max )
					max = s;
				if( s < min )
					min = s;
				mean += s;
				BufferInt32[nsamp_this_scnl] = (int32_t)s;
			}
			msg_p += sizeof(short) * nsamp;
			break;
		case 'l':
			l_data = (int32_t *)msg_p;
			for ( j = 0; j < nsamp; j++, nsamp_this_scnl++ )  {
				l = l_data[j];
				if( l > max )
					max = l;
				if( l < min )
					min = l;
				BufferInt32[nsamp_this_scnl] = l;
			}
			msg_p += sizeof(int32_t) * nsamp;
			break;
		case 'f':
			f_data = (float *)msg_p;
			for ( j = 0; j < nsamp; j++, nsamp_this_scnl++ )  {
				f = f_data[j];
				if( f > max )
					max = f;
				if( f < min )
					min = f;
				BufferFloat[nsamp_this_scnl] = f;
			}
			msg_p += sizeof(float) * nsamp;
			break;
		}

		/* End-check based on length of snippet buffer */
		if ((size_t) msg_p >= ((size_t) getThis->actLen + (size_t) getThis->pBuf) )  {
			if (debug == 1)
				logit ("", "Setting done for <%s.%s.%s>\n", wf->sta, wf->chan, wf->net);
			break; /* Break out of the while(1) loop 'cuz we're done */
		}

		/* msg_p has been advanced to the next TRACE_BUF; localize bytes *
		 * and check for gaps.  */
		wf = (TRACE2_HEADER *) msg_p;
		if (WaveMsg2MakeLocal(wf) < 0)  {
			logit("et", "PSN4PA_next: unknown trace data type: %s\n",
						wf->datatype);
			fclose( psnFile );
			return( EW_FAILURE );
		}
		qual = wf->quality[0];		// Get time reference quality
		nsamp = wf->nsamp;
		starttime = wf->starttime;
		/* starttime is set for new packet; endtime is still set for old packet */
		if ( endtime + ( 1.0/samprate ) * GapThresh < starttime ) {
			/* there's a gap, so fill it */
			logit("et", "gap in %s.%s.%s: %lf: %lf\n", wf->sta, wf->chan, wf->net,
						endtime, starttime - endtime);
			nfill = (long) (samprate * (starttime - endtime) - 1);
			if ( (nsamp_this_scnl + nfill) * (long)sizeof(int32_t) > OutBufferLen ) {
				logit("et", "bogus gap (%d); skipping\n", nfill);
				fclose( psnFile );
				return(EW_FAILURE);
			}
			/* do the filling */
			for ( j = 0; j < nfill; j++, nsamp_this_scnl ++ )
				BufferInt32[nsamp_this_scnl] = fill;
			/* keep track of how many gaps and the largest one */
			gap_count++;
			if (nfill_max < nfill)
				nfill_max = nfill;
		}
		/* Advance endtime to the new packet;				*
		 * process this packet in the next iteration */
		endtime = wf->endtime;
	} /* while(1) */

	if ( datatype == 's' )  {
		for (j = 0; j < nsamp_this_scnl; j++)
			BufferShort[j] = (short)BufferInt32[j];
	}

	/* now make the PSN header */
	memcpy(hdr->headerID, "PSNTYPE4", 8);

	/* Make the SCN test string and then try and find it in the station.lst file */
	sprintf(scnl, "%s.%s.%s.%s", wf->sta, wf->chan, wf->net, wf->loc );
	ReadStationFile( scnl, hdr, debug );

	/* set the time of the first sample of data */
	MakeHdrTime( &hdr->startTime, begintime );

	/* calculate the size of the variable header section of the event file */
	varLen = 0;
	
	if( ( len = (int)strlen( StaLocation) ) != 0 )  {
		varLen += ( len + 1 + sizeof( VarHeader) );		// + 1 for null at the end of the string
		haveVarHdr = 1;
	}
	if( ( len = (int)strlen( SensorInfo) ) != 0 )  {
		varLen += ( len + 1 + sizeof( VarHeader) );
		haveVarHdr = 1;
	}
	if( ChanInfo.sensorOutVolt != 0.0 || ChanInfo.ampGain != 0.0 ||
			ChanInfo.adcInputVolt != 0.0 )  {
		varLen += ( sizeof( ChannelInfo ) + sizeof( VarHeader ) );
		haveVarHdr = 1;
	}
	if( ( len = (int)strlen( DataloggerString ) ) != 0 )  {
		varLen += ( len + 1 + sizeof( VarHeader) );
		haveVarHdr = 1;
	}
	if( haveVarHdr )
		hdr->variableHdrLength = ( varLen + sizeof( VarHeader ) );		// add one more var hdr for end mark
	hdr->sampleCount = nsamp_this_scnl;
	hdr->spsRate = wf->samprate;
	hdr->maximum = max;
	hdr->minimum = min;
	hdr->mean = mean / (double)nsamp_this_scnl;
	if( (qual & TIME_TAG_QUESTIONABLE) == 0 )
		hdr->timeRefStatus = 'L';
	if( datatype == 's' )
		hdr->sampleType = 0;
	else if( datatype == 'l' )
		hdr->sampleType = 1;
	else
		hdr->sampleType = 2;

	if( strlen( wf->sta ) >= 6)
		strncpy(hdr->name, wf->sta, 6 );
	else
		strcpy(hdr->name, wf->sta );

	if( strlen( wf->chan ) >= 3 )
		strncpy(hdr->compName, wf->chan, 3 );
	else
		strcpy(hdr->compName, wf->chan );

	if( strlen( wf->net ) >= 6 )
		strncpy(hdr->network, wf->net, 6);
	else
		strcpy( hdr->network, wf->net );

	if( datatype == 's' )
		data_size = nsamp_this_scnl * sizeof(short);
	else if( datatype == 'l')
		data_size = nsamp_this_scnl * sizeof(int32_t);
	else
		data_size = nsamp_this_scnl * sizeof(float);

	crcLen = sizeof( PSNType4 ) + hdr->variableHdrLength + data_size;

	if( swap )
		SwapPsnHeader( hdr );

	/* now write the main header */
	if( fwrite( (void *)hdr, 1, sizeof(PSNType4), psnFile ) != sizeof(PSNType4) )  {
		logit ("et", "PSN4PA_next: error writing PSN4 Header. \n");
		fclose( psnFile );
		return EW_FAILURE;
	}

	if( haveVarHdr )  {
		/* now save any variable header blocks */
		if( ( len = (int)strlen( StaLocation ) ) != 0 )  {
			++len;
			varHdr.checkNumber = 0x55;
			varHdr.id = 1;
			varHdr.length = len;
			if( swap )
				SwapPsnVarHeader( &varHdr );
			if(fwrite(&varHdr, 1, sizeof(VarHeader), psnFile ) != (UINT)sizeof(VarHeader) )  {
				logit ("et", "PSN4PA_next: error writing PSN4 Header. \n");
				fclose( psnFile );
				return EW_FAILURE;
			}
			if(fwrite( StaLocation, 1, len, psnFile ) != (UINT)len )  {
				logit ("et", "PSN4PA_next: error writing PSN4 Header. \n");
				fclose( psnFile );
				return EW_FAILURE;
			}
		}

		if( ( len = (int)strlen( SensorInfo ) ) != 0 )  {
			++len;
			varHdr.checkNumber = 0x55;
			varHdr.id = 2;
			varHdr.length = len;
			if( swap )
				SwapPsnVarHeader( &varHdr );
			if(fwrite(&varHdr, 1, sizeof(VarHeader), psnFile ) != (UINT)sizeof(VarHeader) )  {
				logit ("et", "PSN4PA_next: error writing PSN4 Header. \n");
				fclose( psnFile );
				return EW_FAILURE;
			}
			if( fwrite( SensorInfo, 1, len, psnFile ) != (UINT)len )  {
				logit ("et", "PSN4PA_next: error writing PSN4 Header. \n");
				fclose( psnFile );
				return EW_FAILURE;
			}
		}
	
		if( ( len = (int)strlen( DataloggerString ) ) != 0 )  {
			++len;
			varHdr.checkNumber = 0x55;
			varHdr.id = 7;
			varHdr.length = len;
			if( swap )
				SwapPsnVarHeader( &varHdr );
			if(fwrite(&varHdr, 1, sizeof(VarHeader), psnFile ) != (UINT)sizeof(VarHeader) )  {
				logit ("et", "PSN4PA_next: error writing PSN4 Header. \n");
				fclose( psnFile );
				return EW_FAILURE;
			}
			if( fwrite( DataloggerString, 1, len, psnFile ) != (UINT)len )  {
				logit ("et", "PSN4PA_next: error writing PSN4 Header. \n");
				fclose( psnFile );
				return EW_FAILURE;
			}
		}

		if( ChanInfo.sensorOutVolt != 0.0 || ChanInfo.ampGain != 0.0 ||
					ChanInfo.adcInputVolt != 0.0 )  {
			varHdr.checkNumber = 0x55;
			varHdr.id = 11;
			varHdr.length = sizeof( ChannelInfo );
			if( swap )
				SwapPsnVarHeader( &varHdr );
			if(fwrite(&varHdr, 1, sizeof(VarHeader), psnFile ) != sizeof( VarHeader ) )  {
				logit ("et", "PSN4PA_next: error writing PSN4 Header. \n");
				fclose( psnFile );
				return EW_FAILURE;
			}
			if( swap )
				SwapChanInfo( &ChanInfo );

			if( fwrite( &ChanInfo, 1, sizeof( ChannelInfo ), psnFile ) != sizeof( ChannelInfo ) )  {
				logit ("et", "PSN4PA_next: error writing PSN4 Header. \n");
				fclose( psnFile );
				return EW_FAILURE;
			}
		}

		// end of var header marker
		varHdr.checkNumber = 0x55;
		varHdr.id = 0;
		varHdr.length = 0;
		if( swap )
			SwapPsnVarHeader( &varHdr );
		if(fwrite(&varHdr, 1, sizeof(VarHeader), psnFile ) != sizeof( VarHeader ) )  {
			logit ("et", "PSN4PA_next: error writing PSN4 Header End Mark. \n");
			fclose( psnFile );
			return EW_FAILURE;
		}
	}

	/* Convert to the appropriate output format */
	if( swap )  {
		/* we are on sparc, data will be read on intel */
		for (j = 0; j < nsamp_this_scnl; j++)  {
			if(datatype == 's' )
				SwapShort( &BufferShort[j] );
			else if( datatype == 'l' )
				SwapInt32( &BufferInt32[j] );
			else if( datatype == 'f' )
				SwapFloat( &BufferFloat[j] );
		}
	}

	if (debug == 1 )
	    logit ("", "Writing %d bytes of trace data\n", data_size );

	if( datatype == 's' )  {
		if ((long)fwrite ((void *) BufferShort, sizeof (char), data_size, psnFile ) !=
				data_size )  {
			logit ("et", "PSN4PA_next: error writing short TRACE data. \n");
			fclose( psnFile );
			return EW_FAILURE;
		}
	}
	else if( datatype == 'l' ) {
		if( (long)fwrite ((void *) BufferInt32, sizeof (char), data_size, psnFile ) !=
				data_size )  {
			logit ("et", "PSN4PA_next: error writing long TRACE data. \n");
			fclose( psnFile );
			return EW_FAILURE;
		}
  	}
	else  {		// write out float data
		if( (long)fwrite ((void *) BufferFloat, sizeof (char), data_size, psnFile ) !=
				data_size )  {
			logit ("et", "PSN4PA_next: error writing long TRACE data. \n");
			fclose( psnFile );
			return EW_FAILURE;
		}
  	}

	crc = CalcFileCRC( psnFile, crcLen );
	if( swap )
		SwapShort( (short *)&crc );
	if( fwrite(&crc, 1, sizeof(short), psnFile ) != sizeof(short))
		logit ("et", "PSN4PA_next: error writing crc. \n");
	GoodData = TRUE;
	fclose( psnFile );
  	return EW_SUCCESS;
}

/************************************************************************
* This is the Put Away end event routine. It's called after we've		*
* finished processing one event.										*
*************************************************************************/
int PSN4PA_end_ev(int debug)
{
	return(EW_SUCCESS);
}


/************************************************************************
* This is the Put Away close routine. It's called after when	        *
* we're being shut down.									            *
*************************************************************************/
int PSN4PA_close(int debug)
{
	if( BufferInt32 )
		free( (char *)BufferInt32 );
	if( BufferShort )
		free( (char *)BufferShort );
	if( BufferFloat )
		free( (char *)BufferFloat );

	return(EW_SUCCESS);
}

/* Make a PSN Type 4 header time based on the start time  */
static void MakeHdrTime( DateTime *ti, double startTime )
{
	struct tm *tme;
	time_t ltime = (time_t)startTime;
	double frac, secs;

	/* gmt time makes months from 0 - 11 */
	tme = gmtime( &ltime );
  	ti->year = (WORD)tme->tm_year + 1900;
	ti->month = (char)(tme->tm_mon + 1);
	ti->day = (char)tme->tm_mday;
	ti->hour = (char)tme->tm_hour;
	ti->minute = (char)tme->tm_min;
	ti->second = (char)tme->tm_sec;
	frac = modf( startTime, &secs );
	ti->nanoseconds = (int)( frac * 1000000000 );
}

/* Read the station.lst file. This file is currently located
   in the directory defined by the system variable "EW_PARAMS" */
static void ReadStationFile( char *scnl, PSNType4 *hdr, int debug  )
{
	char *dir = getenv( "EW_PARAMS" ), buff[256], fname[256];
	char scnlSearch[64], key[256], data[256], *ptr;
	int found = 0;
	FILE *fp;

	StaLocation[0] = SensorInfo[0] = 0;
	ChanInfo.sensorOutVolt = ChanInfo.ampGain = ChanInfo.adcInputVolt = 0;

	if( debug )
		logit("t", "Start ReadStationFile - Looking for %s\n", scnl );

	if( !dir )  {
		if( debug == 1 )
			logit("et", "Environment variable EW_PARAMS not found\n");
		return;
	}

	/* Make the file name and try and open the file */
#ifdef WIN32
	sprintf(fname, "%s\\station.lst", dir );
#else
	sprintf(fname, "%s/station.lst", dir );
#endif		
		
	if( (fp = fopen( fname, "r" ) ) == NULL )  {
		if( debug == 1 )
			logit("et", "Station information file %s not found\n", fname );
		return;
	}

	/* Make the search string and then look for the SCNL in the file */
	sprintf( scnlSearch, "[%s]", scnl );
	while( fgets( buff, 256, fp ) )   {
		if( buff[0] == '#' || buff[0] == ';' || buff[0] == '!' )
			continue;
		Strib( buff );
		ptr = SkipSpace( buff );
		if( strcmp( scnlSearch, ptr ) )
			continue;
		found = TRUE;
		break;
	}
	if( !found )  {
		fclose( fp );
		if( debug )
			logit("et", "Station information not found\n" );
		return;
	}

	if( debug )
		logit("et", "Station information found\n" );

	/* We found the SCNL so now get the key=data pairs from the file */
	while( fgets( buff, 256, fp ) )  {
		if( buff[0] == '#' || buff[0] == ';' || buff[0] == '!' )	// comment
			continue;
		Strib( buff );
		ptr = SkipSpace( buff );
		if( ptr[0] == '[' )				// if next SCNL we are done.
			break;

		ParseLine( key, data, ptr );	// get key and data

		if( !strcmp( key, "latitude" ) )  {
			hdr->latitude = atof( data );
			continue;
		}
		if( !strcmp( key, "longitude" ) )  {
			hdr->longitude = atof( data );
			continue;
		}
		if( !strcmp( key, "elevation" ) )  {
			hdr->elevation = atof( data );
			continue;
		}
		if( !strcmp( key, "orientation" ) )  {
			hdr->compOrientation = data[0];
			continue;
		}
		if( !strcmp( key, "incident" ) )  {
			hdr->compIncident = atof( data );
			continue;
		}
		if( !strcmp( key, "azimuth" ) )  {
			hdr->compAzimuth = atof( data );
			continue;
		}
		if( !strcmp( key, "sensortype" ) )  {
			if( data[0] == 'A' )
				hdr->sensorType = 1;
			else if( data[0] == 'V' )
				hdr->sensorType = 2;
			else if( data[0] == 'D' )
				hdr->sensorType = 3;
			continue;
		}
		if( !strcmp( key, "sensitivity" ) )  {
			hdr->sensitivity = atof( data );
			continue;
		}
		if( !strcmp( key, "referencetype" ) )  {
			hdr->timeRefType[0] = data[0];
			hdr->timeRefType[1] = data[1];
			hdr->timeRefType[2] = data[2];
			continue;
		}
		if( !strcmp( key, "adbits" ) )  {
			hdr->atodBits = (short)atoi( data );
			continue;
		}
		if( !strcmp( key, "magcorrection" ) )  {
			hdr->magCorr = atof( data );
			continue;
		}
		if( !strcmp( key, "stationlocation" ) )  {
			strcpy( StaLocation, data );
			continue;
		}
		if( !strcmp( key, "sensorinformation" ) )  {
			strcpy( SensorInfo, data );
			continue;
		}
		if( !strcmp( key, "sensoroutputvoltage" ) )  {
			ChanInfo.sensorOutVolt = atof( data );
			continue;
		}
		if( !strcmp( key, "amplifiergain" ) )  {
			ChanInfo.ampGain = atof( data );
			continue;
		}
		if( !strcmp( key, "adcinputvoltage" ) )  {
			ChanInfo.adcInputVolt = atof( data );
			continue;
		}
		if( debug )
			logit("", "Unknown Key in station.lst file = %s\n", key );
	}
	fclose( fp );
}

/* Parse the line from the ReadStationFile above. Returns the key and data
   parts of the string */
void ParseLine( char *key, char *data, char *string )
{
	char *ptr;

	*key = 0;
	*data = 0;
	ptr = strchr( string, '=' );	// find the = between key and data
	if( ptr == NULL )
		return;
	*ptr = 0;
	strcpy( key, string );			// get the key
	Strib( key );
	_strlwr( key );
	++ptr;
	ptr = SkipSpace( ptr );
	strcpy( data, ptr );			// get the data
}

/* Skip any white spaces in a string */
static char *SkipSpace( char *string )
{
	while( *string )  {
		if( *string != ' ' && *string != '\t' )
			return string;
		++string;
	}
	return string;
}

/* Strip trailing blanks and newlines from string. */
static int Strib( char *string )
{
	int i, length;

	length = (int)strlen( string );
	if ( length )  {
    for ( i = length-1; i >= 0; i-- )  {
		if ( string[i] == ' ' || string[i] == '\n' || string[i] == '\r' )
			string[i] = '\0';
      	else
			return ( i+1 );
    	}
  	}
  	else
    	return length;
	return i+1;
}

/* Swap the main PSN Type 4 header information structure */
static void SwapPsnHeader( PSNType4 *hdr )
{
	SwapInt( (int *)(&hdr->variableHdrLength) );
	SwapShort( (short *)(&hdr->startTime.year) );
	SwapInt( (int *)(&hdr->startTime.nanoseconds) );
	SwapDouble( &hdr->startTimeOffset );
	SwapDouble( &hdr->spsRate );
	SwapInt( (int *)(&hdr->sampleCount) );
	SwapInt( (int *)(&hdr->flags) );
	SwapDouble( &hdr->compIncident );
	SwapDouble( &hdr->compAzimuth );
	SwapDouble( &hdr->latitude );
	SwapDouble( &hdr->longitude );
	SwapDouble( &hdr->elevation );
	SwapDouble( &hdr->sensitivity );
	SwapDouble( &hdr->magCorr );
	SwapShort( &hdr->atodBits );
	SwapDouble( &hdr->minimum );
	SwapDouble( &hdr->maximum );
	SwapDouble( &hdr->mean );
}

/* Swap the variable header information structure */
static void SwapPsnVarHeader( VarHeader *var )
{
	SwapInt( (int *)(&var->length) );
}

/* Swap the channel information structure */
static void SwapChanInfo( ChannelInfo *chan )
{
	SwapDouble( &chan->sensorOutVolt );
	SwapDouble( &chan->ampGain );
	SwapDouble( &chan->adcInputVolt );
}

/* The following are used to calculate the CRC of the event file */

static WORD CalcFileCRC( FILE *fp, int length )
{
	BYTE buff[512];
	WORD crc = 0;
	int cnt, len;
	long pos = ftell(fp);

	fseek(fp, 0, SEEK_SET);
	while( length )  {
		if( length > 512 )
			len = 512;
		else
			len = length;
		cnt = (int)fread( buff, 1, len, fp );
		if( !cnt )
			logit("et", "Error calculating PSN file CRC. Read error\n");
		CalcCRC16( &crc, buff, cnt );
		length -= len;
	}
	fseek(fp, pos, SEEK_SET);
	return crc ;
}

#define	P	0x1021
#define	W 	16
#define	B 	8

static WORD crctab[1 << B] = {
    0x0000,  0x1021,  0x2042,  0x3063,  0x4084,  0x50a5,  0x60c6,  0x70e7,
    0x8108,  0x9129,  0xa14a,  0xb16b,  0xc18c,  0xd1ad,  0xe1ce,  0xf1ef,
    0x1231,  0x0210,  0x3273,  0x2252,  0x52b5,  0x4294,  0x72f7,  0x62d6,
    0x9339,  0x8318,  0xb37b,  0xa35a,  0xd3bd,  0xc39c,  0xf3ff,  0xe3de,
    0x2462,  0x3443,  0x0420,  0x1401,  0x64e6,  0x74c7,  0x44a4,  0x5485,
    0xa56a,  0xb54b,  0x8528,  0x9509,  0xe5ee,  0xf5cf,  0xc5ac,  0xd58d,
    0x3653,  0x2672,  0x1611,  0x0630,  0x76d7,  0x66f6,  0x5695,  0x46b4,
    0xb75b,  0xa77a,  0x9719,  0x8738,  0xf7df,  0xe7fe,  0xd79d,  0xc7bc,
    0x48c4,  0x58e5,  0x6886,  0x78a7,  0x0840,  0x1861,  0x2802,  0x3823,
    0xc9cc,  0xd9ed,  0xe98e,  0xf9af,  0x8948,  0x9969,  0xa90a,  0xb92b,
    0x5af5,  0x4ad4,  0x7ab7,  0x6a96,  0x1a71,  0x0a50,  0x3a33,  0x2a12,
    0xdbfd,  0xcbdc,  0xfbbf,  0xeb9e,  0x9b79,  0x8b58,  0xbb3b,  0xab1a,
    0x6ca6,  0x7c87,  0x4ce4,  0x5cc5,  0x2c22,  0x3c03,  0x0c60,  0x1c41,
    0xedae,  0xfd8f,  0xcdec,  0xddcd,  0xad2a,  0xbd0b,  0x8d68,  0x9d49,
    0x7e97,  0x6eb6,  0x5ed5,  0x4ef4,  0x3e13,  0x2e32,  0x1e51,  0x0e70,
    0xff9f,  0xefbe,  0xdfdd,  0xcffc,  0xbf1b,  0xaf3a,  0x9f59,  0x8f78,
    0x9188,  0x81a9,  0xb1ca,  0xa1eb,  0xd10c,  0xc12d,  0xf14e,  0xe16f,
    0x1080,  0x00a1,  0x30c2,  0x20e3,  0x5004,  0x4025,  0x7046,  0x6067,
    0x83b9,  0x9398,  0xa3fb,  0xb3da,  0xc33d,  0xd31c,  0xe37f,  0xf35e,
    0x02b1,  0x1290,  0x22f3,  0x32d2,  0x4235,  0x5214,  0x6277,  0x7256,
    0xb5ea,  0xa5cb,  0x95a8,  0x8589,  0xf56e,  0xe54f,  0xd52c,  0xc50d,
    0x34e2,  0x24c3,  0x14a0,  0x0481,  0x7466,  0x6447,  0x5424,  0x4405,
    0xa7db,  0xb7fa,  0x8799,  0x97b8,  0xe75f,  0xf77e,  0xc71d,  0xd73c,
    0x26d3,  0x36f2,  0x0691,  0x16b0,  0x6657,  0x7676,  0x4615,  0x5634,
    0xd94c,  0xc96d,  0xf90e,  0xe92f,  0x99c8,  0x89e9,  0xb98a,  0xa9ab,
    0x5844,  0x4865,  0x7806,  0x6827,  0x18c0,  0x08e1,  0x3882,  0x28a3,
    0xcb7d,  0xdb5c,  0xeb3f,  0xfb1e,  0x8bf9,  0x9bd8,  0xabbb,  0xbb9a,
    0x4a75,  0x5a54,  0x6a37,  0x7a16,  0x0af1,  0x1ad0,  0x2ab3,  0x3a92,
    0xfd2e,  0xed0f,  0xdd6c,  0xcd4d,  0xbdaa,  0xad8b,  0x9de8,  0x8dc9,
    0x7c26,  0x6c07,  0x5c64,  0x4c45,  0x3ca2,  0x2c83,  0x1ce0,  0x0cc1,
    0xef1f,  0xff3e,  0xcf5d,  0xdf7c,  0xaf9b,  0xbfba,  0x8fd9,  0x9ff8,
    0x6e17,  0x7e36,  0x4e55,  0x5e74,  0x2e93,  0x3eb2,  0x0ed1,  0x1ef0
};

static void CalcCRC16(WORD *crc, BYTE *cp, int cnt)
{
	while(cnt--)
		*crc = (*crc << B) ^ crctab[(*crc>>(W - B)) ^ *cp++];
}

