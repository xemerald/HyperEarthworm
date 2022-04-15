/*
 * SWAP.C
 *
 *  Byte swapping functions
 */

/* Make sure one of the platforms is defined properly... */
#ifndef _INTEL
	#ifndef _SPARC
		#error _INTEL and _SPARC are both undefined
	#endif
#endif
/* */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
/* */
#include <swap.h>

/* */
static int WaveMsgVersionMakeLocal( TRACE2X_HEADER *, char );
/* */
static int32_t testInt32Var = 0;

/* SwapUint16: Byte swap 2-byte unsigned integer */
void SwapUint16( void *data )
{
	uint8_t temp;
	union {
		uint8_t c[2];
	} dat;

	memcpy(&dat, data, 2);
	temp     = dat.c[0];
	dat.c[0] = dat.c[1];
	dat.c[1] = temp;
	memcpy(data, &dat, 2);

	return;
}

/* SwapUint32: Byte swap 4-byte unsigned integer */
void SwapUint32( void *data )
{
	uint8_t temp;
	union {
		uint8_t c[4];
	} dat;

	memcpy(&dat, data, 4);
	temp     = dat.c[0];
	dat.c[0] = dat.c[3];
	dat.c[3] = temp;
	temp     = dat.c[1];
	dat.c[1] = dat.c[2];
	dat.c[2] = temp;
	memcpy(data, &dat, 4);

	return;
}

/* SwapUint64: Byte swap 8-byte unsigned integer */
void SwapUint64( EW_UNALIGNED void *data )
{
	uint8_t temp;
	union {
		uint8_t c[8];
	} dat;

	memcpy(&dat, data, 8);
	temp     = dat.c[0];
	dat.c[0] = dat.c[7];
	dat.c[7] = temp;
	temp     = dat.c[1];
	dat.c[1] = dat.c[6];
	dat.c[6] = temp;
	temp     = dat.c[2];
	dat.c[2] = dat.c[5];
	dat.c[5] = temp;
	temp     = dat.c[3];
	dat.c[3] = dat.c[4];
	dat.c[4] = temp;
	memcpy(data, &dat, 8);

	return;
}

/************************ WaveMsgMakeLocal **************************
*       Byte-swap a universal TYPE_TRACEBUF message in place.       *
*       Changes the 'datatype' field in the message header          *
*       Returns -1 if unknown data type.                            *
*       Returns -1 if _SPARC or _INTEL not defined.                 *
*       Returns -2 if checksumish calculation of header fails.      *
*       Elsewise (SUCCESS) returns 0.                               *
*********************************************************************/
int WaveMsgMakeLocal( TRACE_HEADER *wvmsg )
{
	return WaveMsgVersionMakeLocal( (TRACE2X_HEADER *)wvmsg, '1' );
}

/************************ WaveMsg2MakeLocal *************************
*       Byte-swap a universal TYPE_TRACEBUF2 message in place.      *
*       Changes the 'datatype' field in the message header          *
*       Returns -1 if unknown data type.                            *
*       Returns -1 if _SPARC or _INTEL not defined.                 *
*       Returns -1 if more than max number of samples for tbuf      *
*	 size allowed  2000 or 1000 depending on data type          *
*       Returns -2 if checksumish calculation of header fails.      *
*       Elsewise (SUCCESS) returns 0.                               *
*********************************************************************/
int WaveMsg2MakeLocal( TRACE2_HEADER *wvmsg )
{
	return WaveMsgVersionMakeLocal( (TRACE2X_HEADER *)wvmsg, wvmsg->version[0] );
}

/************************ WaveMsg2XMakeLocal *************************
*       Byte-swap a universal TYPE_TRACEBUF2X message in place.      *
*       Changes the 'datatype' field in the message header          *
*       Returns -1 if unknown data type.                            *
*       Returns -1 if _SPARC or _INTEL not defined.                 *
*       Returns -1 if more than max number of samples for tbuf      *
*	 size allowed  2000 or 1000 depending on data type          *
*       Returns -2 if checksumish calculation of header fails.      *
*       Elsewise (SUCCESS) returns 0.                               *
*********************************************************************/
int WaveMsg2XMakeLocal( TRACE2X_HEADER *wvmsg )
{
	return WaveMsgVersionMakeLocal( wvmsg, wvmsg->version[0] );
}

/*
 *
 */
static int WaveMsgVersionMakeLocal( TRACE2X_HEADER *wvmsg, char version )
{
	static const int tracedata_max_size = MAX_TRACEBUF_SIZ - sizeof(TRACE2X_HEADER);
/* */
	int      data_size  = 0;   /* flag telling us how many bytes in the data */
	char     byte_order = ' ';
#if defined( _SPARC )
	char     loc_ibyte_order = 's';
	char     loc_fbyte_order = 't';
	char     ops_ibyte_order = 'i';
	char     ops_fbyte_order = 'f';
#elif defined( _INTEL )
	char     loc_ibyte_order = 'i';
	char     loc_fbyte_order = 'f';
	char     ops_ibyte_order = 's';
	char     ops_fbyte_order = 't';
#else
	char     loc_ibyte_order = ' ';
	char     loc_fbyte_order = ' ';
	char     ops_ibyte_order = ' ';
	char     ops_fbyte_order = ' ';
	printf("WaveMsg2MakeLocal warning: _INTEL and _SPARC are both undefined.");
	return -1;
#endif
/* */
	int32_t *int_ptr;
	int16_t *short_ptr;
	float   *float_ptr;
	double  *double_ptr;

	int    i;
	int    nsamp;
	double samprate;
	double starttime;
	double endtime;

	double _endtime;
	double fudge;

/* See what sort of data it carries */
	if ( wvmsg->datatype[0] == 's' && (wvmsg->datatype[1] == '2' || wvmsg->datatype[1] == '4') )
		byte_order = 's';
	else if ( wvmsg->datatype[0] == 'i' && (wvmsg->datatype[1] == '2' || wvmsg->datatype[1] == '4') )
		byte_order = 'i';
	else if ( wvmsg->datatype[0] == 't' && (wvmsg->datatype[1] == '4' || wvmsg->datatype[1] == '8') )
		byte_order = 't';
	else if ( wvmsg->datatype[0] == 'f' && (wvmsg->datatype[1] == '4' || wvmsg->datatype[1] == '8') )
		byte_order = 'f';
	else
	/* We don't know this message type*/
		return -1;
/* */
	data_size  = wvmsg->datatype[1] - '0';

/* SWAP the header (if neccessary) */
	if ( byte_order != loc_ibyte_order && byte_order != loc_fbyte_order ) {
	/* swap the header */
		SwapInt( &(wvmsg->pinno) );
		SwapInt( &(wvmsg->nsamp) );
		SwapDouble( &(wvmsg->starttime) );
		SwapDouble( &(wvmsg->endtime)   );
		SwapDouble( &(wvmsg->samprate)  );
		if ( version == TRACE2_VERSION0 ) {
			switch ( wvmsg->version[1] ) {
			case TRACE2_VERSION11:
				SwapFloat( &(wvmsg->x.v21.conversion_factor) );
				break;
			}
		}
	}
/* */
	if ( wvmsg->nsamp > tracedata_max_size / data_size ) {
		logit(
			"et","WaveMsg2MakeLocal: packet from %s.%s.%s.%s has bad number of samples=%d datatype=%s\n",
			wvmsg->sta, wvmsg->chan, wvmsg->net, wvmsg->loc, wvmsg->nsamp, wvmsg->datatype
		);
		return -1;
	}

/*
 * Perform a CheckSumish kind of calculation on the header
 * ensure that the tracebuf ends within 5 samples of the given endtime.
 * DK 2002/03/18
 */

/* moved nsamp memcpy to here to avoid byte-alignment with next statement */
	memcpy(&nsamp,     &wvmsg->nsamp,     sizeof(int)   );
	memcpy(&samprate,  &wvmsg->samprate,  sizeof(double));
	memcpy(&starttime, &wvmsg->starttime, sizeof(double));
	memcpy(&endtime,   &wvmsg->endtime,   sizeof(double));

	_endtime = starttime + ((nsamp - 1) / samprate);
	fudge    = 5.0 / samprate;

/*
 * This is supposed to be a simple sanity check to ensure that the
 * endtime is within 5 samples of where it should be.  We're not
 * trying to be judgemental here, we're just trying to ensure that
 * we protect ourselves from complete garbage, so that we don't segfault
 * when allocating samples based on a bad nsamp
 */
	if ( endtime < (_endtime - fudge) || endtime > (_endtime + fudge) ) {
		logit(
			"e","WaveMsg2MakeLocal: packet from %s.%s.%s.%s has inconsistent header values!\n",
			wvmsg->sta, wvmsg->chan, wvmsg->net, wvmsg->loc
		);
		logit("e","WaveMsg2MakeLocal: header.starttime  : %.4lf\n", starttime);
		logit("e","WaveMsg2MakeLocal: header.samplerate : %.1lf\n", samprate );
		logit("e","WaveMsg2MakeLocal: header.nsample    : %d\n",    nsamp    );
		logit("e","WaveMsg2MakeLocal: header.endtime    : %.4lf\n", endtime  );
		logit("e","WaveMsg2MakeLocal: computed.endtime  : %.4lf\n", _endtime );
		logit("e","WaveMsg2MakeLocal: header.endtime is not within 5 sample intervals of computed.endtime!\n");
		return -2;
	}

/* SWAP the data (if neccessary) */
	if ( byte_order == ops_ibyte_order ) {
	/* Swap the data.  */
		int_ptr   = (int32_t *)(wvmsg + 1);
		short_ptr = (int16_t *)(wvmsg + 1);
		for ( i = 0; i < nsamp; i++, int_ptr++, short_ptr++ ) {
			if ( data_size == 2 )
				SwapUint16( short_ptr );
			else if ( data_size == 4 )
				SwapUint32( int_ptr );
		}
	/* Re-write the data type field in the message */
		wvmsg->datatype[0] = loc_ibyte_order;
		if ( data_size == 2 )
			wvmsg->datatype[1] = '2';
		else if ( data_size == 4 )
			wvmsg->datatype[1] = '4';
	}
	else if ( byte_order == ops_fbyte_order ) {
	/* Swap the data.  */
		float_ptr  = (float *)(wvmsg + 1);
		double_ptr = (double *)(wvmsg + 1);
		for ( i = 0; i < nsamp; i++, float_ptr++, double_ptr++ ) {
			if ( data_size == 4 )
				SwapFloat( float_ptr );
			else if ( data_size == 8 )
				SwapDouble( double_ptr );
		}
	/* Re-write the data type field in the message */
		wvmsg->datatype[0] = loc_fbyte_order;
		if ( data_size == 4 )
			wvmsg->datatype[1] = '4';
		else if ( data_size == 8 )
			wvmsg->datatype[1] = '8';
	}

	return 0;
}
