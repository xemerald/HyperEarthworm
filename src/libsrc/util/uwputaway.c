/* uwputaway.c

	Routines for writing trace data in UW-2 format

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include "earthworm.h"
#include "trace_buf.h"
#include "swap.h"
#include "ws_clientII.h"
#include "site.h"
#include "uwdfif.h"

extern int	CreateDir (char *);

/********  UW Putaway stuff from here on down ****************/

static char    UWFile[256];
static void	*traceBuf;
enum ByteOrder {ByteOrderHost, ByteOrderIEEE, ByteOrderIntel};
static int OutputByteOrder = ByteOrderHost;

/************************************************************************
* Initialization function,                                              *
*       This is the Put Away startup intializer. This is called when    *
*       the system first comes up. Here is a chance to look around      *
*       and see if it's possible to do business, and to complain        *
*       if not ,BEFORE an event has to be processed.                    *
*                                                                       *
*************************************************************************/
int UWPA_init (int traceBufLen, char *OutDir,
		char *OutputFormat, int debug)
{

	/* set the output byte order */
	if ( OutputFormat == NULL )
		OutputByteOrder = ByteOrderHost;
	else if ( strncasecmp(OutputFormat,"s",1) == 0 )
		OutputByteOrder = ByteOrderIEEE;
	else if ( strncasecmp(OutputFormat,"i",1) == 0 )
		OutputByteOrder = ByteOrderIntel;
	else
		OutputByteOrder = ByteOrderHost;

	/** Allocate traceBuf */
	if ( (traceBuf = malloc(traceBufLen)) == NULL ) {
		logit ("et", "UWPA_next_ev: can't malloc traceBuf\n\t%s\n",strerror(errno));
		return EW_FAILURE;
	}

	/* Make sure that the top level output directory exists */
	if ( OutDir != NULL ) {
		if (CreateDir (OutDir) != EW_SUCCESS) {
			logit ("e", "UWPA_init: Call to CreateDir failed\n");
			return EW_FAILURE;
		}
	}

	return EW_SUCCESS;
}

/************************************************************************
*   This is the Put Away event initializer. It's called when a snippet  *
*   has been received, and is about to be processed.                    *
*   It gets to see the pointer to the TraceRequest array,               *
*   and the number of loaded trace structures.                          *
*                                                                       *
*   For UW-2, we need to make sure that any specified target directory  *
*   exists, and we can write there, then initialize a new UW file for   *
*   for writing.  The file name is contructed from the data and time.   *
*   Checking is done to prevent buffer overruns of the file name.       *
*************************************************************************/
int UWPA_next_ev (char *EventID, TRACE_REQ *ptrReq, int nReq,
		char *OutDir, char *EventDate, char *EventTime,
		char *EventInst, char *EventMod, int debug)
{
	char	delim = PATH_SEP, cmt[80], *pos;
	struct tm *reqTm;
	time_t epoch;
	struct stime UWTm;
	int16_t flags[10] = {0,0,0,0,0,0,0,0,0,0};

	if ( ptrReq == NULL ) {
		logit ("e", "UWPA_next: invalid argument passed in.\n");
		return EW_FAILURE;
	}

	/* Put in path, if any */
	*UWFile = '\0';
	if ( OutDir != NULL )
		if ( strlen(OutDir) > 0 ) {
			if ( strlen(OutDir) + sizeof(delim) >= sizeof(UWFile) ) {
				logit ( "et", "UWPA_next_ev: OutDir name too long\n");
				return EW_FAILURE;
			}
			sprintf (UWFile, "%s%c", OutDir, delim);
		}
	/* Now append file name created from the date and time string */
	if ( EventDate == NULL || EventTime == NULL ) {
		logit("et", "UWPA_next_ev: no date or time!\n");
		return EW_FAILURE;
	}
	if ( (pos = strchr(EventTime,'.')) != NULL )
		*pos = '\0';	/* drop fractional part of second, if any */
	if ( strlen(UWFile) + strlen(EventDate) + strlen(EventTime) >= sizeof(UWFile) ) {
		logit ( "et", "UWPA_next_ev: filename too long\n");
		return EW_FAILURE;
	}
	sprintf (UWFile + strlen(UWFile), "%s%sW", EventDate, EventTime);

	if (debug == 1)
		logit ("et", "Opening UW-2 file %s\n", UWFile);

	/* initialize new UW file for writing */
	if ( UWDFinit_for_new_write(UWFile) == FALSE ) {
		logit ("et", "UWPA_next_ev: unable to open file %s\n",UWFile);
		return EW_FAILURE;
	}

	/* Put event info into the UW header comment.  Ignore missing info,
	   and silently truncate info if it would cause buffer overruns. */
	*cmt = '\0';
	strncpy(cmt,"Earthworm", sizeof(cmt) - strlen(cmt) - 1);
	if ( EventMod != NULL )
		if ( strlen(EventMod) ) {
			strncat(cmt, " modID ", sizeof(cmt) - strlen(cmt) - 1);
			strncat(cmt, EventMod, sizeof(cmt) - strlen(cmt) - 1);
		}
	if ( EventInst != NULL )
		if ( strlen(EventInst) ) {
			strncat(cmt, ", instID ", sizeof(cmt) - strlen(cmt) - 1);
			strncat(cmt, EventInst, sizeof(cmt) - strlen(cmt) - 1);
		}
	if ( EventID != NULL )
		if ( strlen(EventID) ) {
			strncat(cmt, ", evtID ", sizeof(cmt) - strlen(cmt) - 1);
			strncat(cmt, EventID, sizeof(cmt) - strlen(cmt) - 1);
	}
	cmt[sizeof(cmt)-1] = '\0';
	if ( UWDFset_dhcomment(cmt) == FALSE ) {
		logit ("et", "UWPA_next_ev: unable to add comment to UW header\n");
		return EW_FAILURE;
	}

	/* Set event time based on requested start time */
	/* Get time_t (usually int) part of reqStarttime (which is a double) */
	epoch = ptrReq->reqStarttime;
	reqTm = gmtime( &epoch );
	UWTm.yr = 1900 + reqTm->tm_year;
	UWTm.mon = reqTm->tm_mon + 1;
	UWTm.day = reqTm->tm_mday;
	UWTm.hr = reqTm->tm_hour;
	UWTm.min = reqTm->tm_min;
	UWTm.sec = reqTm->tm_sec + (ptrReq->reqStarttime - (int)epoch);
	if ( UWDFset_dhref_stime(UWTm) == FALSE ) {
		logit ("et", "UWPA_next_ev: unable to set UW header time\n");
		return EW_FAILURE;
	}

	/* set other things in the event header */
	if ( EventMod != NULL )
		UWDFset_dhtapeno(atoi(EventMod));
	if ( EventID != NULL )
		UWDFset_dhevno(atoi(EventID));

	/* Claim that the sample rate and time are correct, which may not be
	   true for everyone */
	flags[0] = 2;
	UWDFset_dhflgs(flags);

	/* now write master UW header */
	if ( UWDFwrite_new_head() == FALSE ) {
		logit ("et", "UWPA_next_ev: unable to write new UW header\n");
		return EW_FAILURE;
	}

	/* Overwrite EventID with the output file name for possible use
	   by the post-processing command - this is a major kludge */
	sprintf(EventID,"%s%sW", EventDate, EventTime);

	return (EW_SUCCESS);

}


/************************************************************************
* This is the working entry point into the disposal system. This        *
* routine gets called for each trace snippet which has been recovered.  *
* It gets to see the corresponding SNIPPET structure, and the event id  *
*                                                                       *
*************************************************************************/
/* Process one channel of data */
int UWPA_next (TRACE_REQ *getThis, double GapThresh,
		int traceBufLen, int debug)
{
	TRACE2_HEADER *wf;
	char *msg_p, sta[6], comp[4], net[4], otype;
	double samprate, starttime, bias;
	int nsamp, byte_per_sample, tot_samp;
	int ii, tb_pos;
	int16_t *sd_in, *sd_out;
	int32_t *ld_in, *ld_out;
	float *fd_in, *fd_out;
	struct tm *chanTm;
	time_t epoch;
	struct stime UWTm;

	/* Check trace request and packet buffer */
	if ( getThis == NULL ) {
		logit ("e", "UWPA_next: invalid argument passed in.\n");
		return EW_FAILURE;
	}
	if ( getThis->pBuf == NULL ) {
		logit ("e", "UWPA_next: Message buffer is NULL.\n");
		return EW_FAILURE;
	}

        wf = (TRACE2_HEADER *) getThis->pBuf;
	/* get localized copy of first packet header and data */
	if ( WaveMsg2MakeLocal(wf) < 0 ) {
		logit("et","UWPA_next: bad packet.  failed WaveMsg2MakeLocal(), Skipping trace\n");
		return EW_FAILURE;
	}

	/* grab basic trace info from first packet */
	if ( (nsamp = wf->nsamp) <= 0 ) {
		logit ("e", "UWPA_next: Bad message data length.\n");
		return EW_FAILURE;
	}
	strncpy(sta, wf->sta, 4);
	strncpy(comp, wf->chan, 3);
	strncpy(net, wf->net, 3 );
	sta[4] = comp[3] = net[3] = '\0';
	starttime = wf->starttime;
	samprate = wf->samprate;
	byte_per_sample = atoi(wf->datatype + 1);

	/* Set trace start time to snippet start time */
	epoch = starttime;
	chanTm = gmtime( &epoch );
	UWTm.yr = 1900 + chanTm->tm_year;
	UWTm.mon = chanTm->tm_mon + 1;
	UWTm.day = chanTm->tm_mday;
	UWTm.hr = chanTm->tm_hour;
	UWTm.min = chanTm->tm_min;
	UWTm.sec = chanTm->tm_sec + (starttime - (int)epoch);

	UWDFset_chref_stime(UWTm);
	UWDFset_chname(sta);
	UWDFset_chcompflg(comp);
	UWDFset_chid(net);
	UWDFset_chsrate(samprate);
	UWDFset_chlta(0);
	UWDFset_chtrig(0);

	/* set the UW output byte order */
	if ( OutputByteOrder == ByteOrderIntel )
		UWDFwrite_intel_byte_order();
	else if ( OutputByteOrder == ByteOrderIEEE )
		UWDFwrite_ieee_byte_order();
	else
		UWDFwrite_host_byte_order();

	/* Set the UW output data type */
	if ( wf->datatype[0]=='i' || wf->datatype[0]=='s' ) {
		/* integer types */
		if ( byte_per_sample == 2 ) {
			otype = 'S';
		} else if ( byte_per_sample == 4 ) {
			otype = 'L';
		}
	} else if ( wf->datatype[0] == 'f' || wf->datatype[0] == 't' ) {
		/* float types */
		if ( byte_per_sample == 4 )
			otype = 'F';
	} else {
		logit ("e", "UWPA_next: Unknown data type\n");
		return EW_FAILURE;
	}
	/* For Reclamation, short-period data is 16-bit, and broad-band is 24-bit */
	if (  strcmp(net,"RE") == 0 ) {
		if ( comp[0] == 'E' )
			otype = 'S';
		else if ( comp[0] == 'H' || comp[0] == 'B' )
			otype = 'L';
	}

	/* loop through enough messages to get bias for this s-c-n */
	bias = 0.; tot_samp = 0;
	for ( tb_pos = 0, msg_p = getThis->pBuf ;
		tb_pos * (otype == 'S' ? 2 : 4) < traceBufLen &&
			msg_p < getThis->pBuf + getThis->actLen ;
		tb_pos += nsamp,
			msg_p += sizeof(TRACE_HEADER) + nsamp * byte_per_sample ) {

		/* don't massage first message again */
		if ( msg_p > getThis->pBuf ) {
			/* get localized copy of message header and data */
  			wf = (TRACE2_HEADER *) msg_p;
			if ( WaveMsg2MakeLocal(wf) < 0 ) {
				logit("et","UWPA_next: bad packet.  Ending trace\n");
				break;
			}

			/* check for bad message, then get actual start time,
			   etc. of this message */
			if ( (samprate = wf->samprate) <= 0. ) {
				logit("e", "UWPA_next: bad sample rate; Ending trace\n");
				break;
			}
			byte_per_sample = atoi(wf->datatype + 1);
			nsamp = wf->nsamp;
		}
		tot_samp += nsamp;

		switch ( byte_per_sample ) {
			case 2:
				sd_in = (int16_t *) (msg_p + sizeof(TRACE2_HEADER));
				for ( ii = 0 ; ii < nsamp ; ii++ )
					bias += sd_in[ii];
				break;
			case 4:
				switch ( wf->datatype[0] ) {
					case 'f':
					case 't':
						fd_in = (float *) (msg_p + sizeof(TRACE2_HEADER));
						for ( ii = 0 ; ii < nsamp ; ii++ )
							bias += fd_in[ii];
						break;
					case 'i':
					case 's':
						ld_in = (int32_t *) (msg_p + sizeof(TRACE2_HEADER));
						for ( ii = 0 ; ii < nsamp ; ii++ )
							bias += ld_in[ii];
						break;
				}
				break;
		}
	}
	bias /= tot_samp;
	UWDFset_chbias( bias );

	/* initialize the trace data buffer to all zeros */
	memset(traceBuf, 0, traceBufLen);

	/* loop through the messages for this s-c-n and put de-meaned, gap-filled
	   data into the output trace buffer */
	tot_samp = 0;
	for ( tb_pos = 0, msg_p = getThis->pBuf ;
		tb_pos * (otype == 'S' ? 2 : 4) < traceBufLen &&
			msg_p < getThis->pBuf + getThis->actLen ;
		tb_pos += nsamp,
			msg_p += sizeof(TRACE_HEADER) + nsamp * byte_per_sample ) {
		double exp_starttime;

		/* get localized copy of message header and data */
     		wf = (TRACE2_HEADER *) msg_p;
		if ( WaveMsg2MakeLocal(wf) < 0 ) {
			logit("et","UWPA_next: bad packet.  Ending trace\n");
			break;
		}

		/* after the first, predict the start time of the current message */
		if ( msg_p > getThis->pBuf )
			exp_starttime = starttime + (nsamp + 1) * samprate;

		starttime = wf->starttime;
		byte_per_sample = atoi(wf->datatype + 1);
		nsamp = wf->nsamp;

		/* fill gaps with bias level by just skipping over them (output
		   buf initialized to 0 = gap-filling value for de-meaned data).
		   Gaps less than GapThresh value assumed to not be gaps at all,
		   which will wreck time stamping if wrong. Uwdfif assumes the start
		   time of the channel is correct, and that subsequent sample
		   times = sample offset * sample_rate.  Conversion to UW drops
		   the (redundant, we hope) time-stamp info from later packets */
		if ( msg_p > getThis->pBuf ) {
			if ( starttime > exp_starttime + GapThresh / samprate ) {
				int gap = ROUND((starttime - exp_starttime) / samprate);
				tb_pos += gap;
				tot_samp += gap;
				logit("", "UWPA_next: Gap of %d samples in %s.%s.%s: %lf -> %lf\n",
					gap, sta, comp, net, exp_starttime, starttime);
			}
		}

		/* check that we have space for this data */
		if ( (tb_pos + nsamp) * (otype == 'S' ? 2 : 4) >= traceBufLen ) {
			logit ("et", "UWPA_next: Ran out of buffer for %s.%s.%s.  Truncating.\n",
					sta, comp, net);
			nsamp = traceBufLen / (otype == 'S' ? 2 : 4) - tb_pos - 1;
			/* nsamp can be <= 0 here, and loop will correctly terminate */
		}
		tot_samp += nsamp;

		/* remove bias, and add data from this message to the UW output
		   trace buf in the correct output format */
		switch ( byte_per_sample ) {
			case 2:
				sd_in = (int16_t *) (msg_p + sizeof(TRACE2_HEADER));
				switch ( otype ) {
					case 'S':
						sd_out = (int16_t *)traceBuf + tb_pos;
						for ( ii = 0 ; ii < nsamp ; ii++ )
							sd_out[ii] = sd_in[ii] - bias;
						break;
					case 'L':
						ld_out = (int32_t *)traceBuf + tb_pos;
						for ( ii = 0 ; ii < nsamp ; ii++ )
							ld_out[ii] = sd_in[ii] - bias;
						break;
					case 'F':
						fd_out = (float *)traceBuf + tb_pos;
						for ( ii = 0 ; ii < nsamp ; ii++ )
							fd_out[ii] = sd_in[ii] - bias;
						break;
				}
				break;
			case 4:
				switch ( wf->datatype[0] ) {
					case 'f':
					case 't':
						fd_in = (float *) (msg_p + sizeof(TRACE2_HEADER));
						switch ( otype ) {
							case 'S':
								sd_out = (int16_t *)traceBuf + tb_pos;
								for ( ii = 0 ; ii < nsamp ; ii++ )
									sd_out[ii] = fd_in[ii] - bias;
								break;
							case 'L':
								ld_out = (int32_t *)traceBuf + tb_pos;
								for ( ii = 0 ; ii < nsamp ; ii++ )
									ld_out[ii] = fd_in[ii] - bias;
								break;
							case 'F':
								fd_out = (float *)traceBuf + tb_pos;
								for ( ii = 0 ; ii < nsamp ; ii++ )
									fd_out[ii] = fd_in[ii] - bias;
								break;
						}
						break;
					case 'i':
					case 's':
						ld_in = (int32_t *) (msg_p + sizeof(TRACE2_HEADER));
						switch ( otype ) {
							case 'S':
								sd_out = (int16_t *)traceBuf + tb_pos;
								for ( ii = 0 ; ii < nsamp ; ii++ )
									sd_out[ii] = ld_in[ii] - bias;
								break;
							case 'L':
								ld_out = (int32_t *)traceBuf + tb_pos;
								for ( ii = 0 ; ii < nsamp ; ii++ )
									ld_out[ii] = ld_in[ii] - bias;
								break;
							case 'F':
								fd_out = (float *)traceBuf + tb_pos;
								for ( ii = 0 ; ii < nsamp ; ii++ )
									fd_out[ii] = ld_in[ii] - bias;
								break;
						}
						break;
				}
				break;
		}
	}

	if ( UWDFwrite_new_chan(otype, tot_samp, traceBuf, otype) == FALSE) {
		logit("e", "earth2uw: UWPA_event: error writing <%s><%s><%s> to file: %s\n",
					sta, comp, net, strerror(errno));
		return EW_FAILURE;
	}

	return EW_SUCCESS;
}


/************************************************************************
* This is the Put Away end event routine. It's called after we've       *
* finished processing one event                                         *
*                                                                       *
* For UW - close the UW file                                            *
*************************************************************************/
int UWPA_end_ev (int debug)
{

	if (debug == 1)
		logit ("t", "Debug: UWPA_end_ev Closing UW file %s\n",UWFile);

	if ( UWDFclose_new_file() == FALSE ) {
		logit ("et", " UWPA_end_ev: error closing %s.\n",UWFile);
		return EW_FAILURE;
	}

	return( EW_SUCCESS );
}


/************************************************************************
*       This is the Put Away close routine. It's called after we've     *
*       been told to shut down.                                         *
*************************************************************************/
int UWPA_close (int debug)
{
	return( EW_SUCCESS );
}
