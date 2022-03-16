/*  k2hder2ew.c
 *  Read a K2-format .evt data file, crack the header, and fill
 *  the return structure to be returned to the calling program.  
 *   This code was cloned from nsmp2ew; Credit Jim Luetgert. 
 */

/*  k2evt2ew() and extract() modified by DK 20030108 in order to:
 *  1) Prevent data from being overwritten(corrupted) when the 
 *     length of the EVT file exceeds the length of the K2InfoStruct.
 *  2) Return a warning to the k2evt2ew() caller when the data output
 *     is trunctated because the length of the EVT file exceeds the 
 *     length of the K2InfoStruct.
 */

#include <inttypes.h>		/* PRIu32 */
#include <math.h>
#include <stdint.h>		/* int32_t, uint32_t */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#include "earthworm.h"
#include "chron3.h"
#include "kom.h"
#include "swap.h"
#include "k2evt2ew.h"

#define PACKET_MAX_SIZE  3003      /* 2992 + 8 + 3, same as QT */
#define GRAVITY          981.0     /* Gravity in cm/sec/sec */
#define SWAP(a,b) temp=(a);(a)=(b);(b)=temp
#define DECADE   315532800  /* Number of seconds betweeen 1/1/1970 and 1/1/1980 */

static		int		K2EW_Debug;

/* Functions in this source file
 *******************************/
static	int read_tag (FILE *fp, KFF_TAG *);
static	int read_head (FILE *fp, MW_HEADER *, int tagLength);
static	int read_frame (FILE *fp, FRAME_HEADER *, uint32_t *channels );

int extract(int pktdatalen, const unsigned char *pktbuff,
            float *Data, int32_t *Counts, float scale, int nchan, int jchan,
            int *ind, int array_size);
int peak_ground(float *Data, int npts, int itype, float dt, SM_INFO *sm);
void demean(float *A, int N);
void locut(float *s, int nd, float fcut, float delt, int nroll, int icaus);
void rdrvaa(float *acc, int na, float omega, float damp, float dt,
            float *rd, float *rv, float *aa, int *maxtime);
void amaxper(int npts, float dt, float *fc, float *amaxmm, 
             float *aminmm, float *pmax, int *imin, int *imax);

/******************************************************************************
 * k2evt2ew()   Read and crack one .evt file.
 *
 *  fp:   		File pointer to the .evt file to crack; file must
 *              previuosly be opened for binary reads.
 *  fname:      name of the file pointed to by fp (debug purposes)
 *  pk2info:    pointer to a pre-allocated structure where info from
 *              the file will be stored.
 *  pChanName:  pointer to the CHANNELNAME structure containing mappings
 *              from SM Box and channel number to SCN. This is optional;
 *              if it is NULL, then default SCN names will be assigned as
 *              follows: S=box, C=channel number, N=NetCode
 *  numChans:   Number of channels defined in pChanName
 *  NetCode:    Network Code to use if pChanName is null.
 *  Debug:      Integer Debug level.
 *
 *  Return Values: EW_SUCCESS and EW_FAILURE, with pk2info filled with
 *   other useful info.  
 *                 EW_WARNING  when data output is truncated for
 *   one or more channels.
 *
 ******************************************************************************/

int k2evt2ew (FILE *fp, char *fname, K2InfoStruct *pk2info, 
		CHANNELNAME *pChanName, int numChans, char *NetCode, int Debug)
{
	unsigned char g_k2mi_buff[PACKET_MAX_SIZE-9]; /* received data buffer */
    char     serial[10];
    int      i, j, ci, chanind, nscans, ret, rc=EW_SUCCESS;
    int      flen,  stat, nchans, itype[18], azim[18];
    uint32_t  decade, channels;
    float   dt, scale[18];

	if ((fp == NULL) || (fname == NULL) || (pk2info == NULL))
	{
		logit ("e", "k2evt2ew: Invalid arguments passed in.\n");
		return EW_FAILURE;
	}

    if ((strstr (fname, ".EVT") == 0) && (strstr (fname, ".evt") == 0))
	{
		logit ("e", "k2evt2ew: invalid file name <%s>\n", fname);
		return EW_FAILURE;
	}

	K2EW_Debug = Debug;
/*	sec1970 = (float) 11676096000.00; */ /* # seconds between Carl Johnson's */
                                      /* time 0 and 1970-01-01 00:00:00.0 GMT */

/* Initialize variables
 **********************/
    memset ( &pk2info->frame,  0, sizeof(FRAME_HEADER) );
    memset ( &pk2info->head,  0, sizeof(K2_HEADER) );
    memset ( &pk2info->tag,  0, sizeof(KFF_TAG) );
    memset ( &pk2info->sm,  0, MAX_SM_PER_BOX*sizeof(SM_INFO) );


	/*  Kinemetric time starts 10 years after EW time  */
    decade = (365*10+2)*24*60*60;     

/* First, just read the header & first frame to get some basic info.
 ******************************************************************/
    rewind(fp);

    ret = read_tag(fp, &pk2info->tag);
	if ( ret != EW_SUCCESS ) return EW_FAILURE;
    stat = read_head(fp, &pk2info->head, pk2info->tag.length);
    if ( stat != EW_SUCCESS )
        return stat;

    ret = read_tag(fp, &pk2info->tag);
	if ( ret != EW_SUCCESS ) return EW_FAILURE;
    flen = pk2info->tag.dataLength;
    read_frame(fp, &pk2info->frame, &channels);

    nscans = pk2info->head.roParms.stream.duration;
    nchans = pk2info->head.rwParms.misc.nchannels;
    dt  = (float)(1.0/(10.0*(flen/(3.0*nchans))));

    if(pk2info->head.roParms.stream.flags != 0) 
	{
        logit("e", "%s: not data. flags = %d\n",
                    fname, pk2info->head.roParms.stream.flags);
        return EW_FAILURE;
    }

	 sprintf(serial, "%d", pk2info->head.rwParms.misc.serialNumber);

	
    for (i=0; i<nchans; i++) 
	{
         pk2info->sm[i].t = pk2info->head.roParms.stream.triggerTime +  
                     (float)pk2info->head.roParms.stream.triggerTimeMsec/1000.0;
         /* JHL 20040223 Set the sm time to the stream start time so that peak times
                         can be calculated.  It will be reset to trigger time before
                         the sm message is delivered. */
         pk2info->sm[i].t = pk2info->head.roParms.stream.startTime   + 
                     (float)pk2info->head.roParms.stream.startTimeMsec/1000.0;
         pk2info->sm[i].t += decade;
         pk2info->sm[i].talt    = (float) time(NULL);
         /* JHL 20080223 Make sure time is not in the future   */
         if(pk2info->sm[i].t > pk2info->sm[i].talt) pk2info->sm[i].t = pk2info->sm[i].talt;
         pk2info->sm[i].altcode = SM_ALTCODE_RECEIVING_MODULE;
         itype[i] = 0;
         if( pk2info->head.rwParms.channel[i].sensorType==SENSOR_VELOCITY ||
	                pk2info->head.rwParms.channel[i].sensorType==SENSOR_MARKL22 ||
	                pk2info->head.rwParms.channel[i].sensorType==SENSOR_MARKL4C) 
				itype[i] = 2;
         else
         if((pk2info->head.rwParms.channel[i].sensorType>=SENSOR_FBA11 &&
               		pk2info->head.rwParms.channel[i].sensorType<=SENSOR_FBA23) ||
                	pk2info->head.rwParms.channel[i].sensorType==SENSOR_EPI ||
                	pk2info->head.rwParms.channel[i].sensorType==SENSOR_ACCELERATION) 
				itype[i] = 3;

         scale[i] = (float)((2.0*pk2info->head.rwParms.channel[i].fullscale/(1<<24))
				/ pk2info->head.rwParms.channel[i].sensitivity);

         if (itype[i]==3) 
				scale[i] = scale[i] * (float) GRAVITY;

         if (itype[i]==0) 
		 {
                logit("e", "Can't decode sensor type! = %d\n",
                           pk2info->head.rwParms.channel[i].sensorType);
         }
         azim[i] = pk2info->head.rwParms.channel[i].azimuth;

		 /* Find this channel in the list */
		chanind = -1;
		for (ci = 0; ci < numChans; ci++) 
		{
			if ((strcmp(serial, pChanName[ci].box) == 0) &&
			    (pChanName[ci].chan == i) &&
                	    (strcmp(pk2info->head.rwParms.misc.stnID, pChanName[ci].sta) == 0)  )
			{
				chanind = ci;
				break;
			}
		}

		
	   	if (chanind < 0) 	
		{
			/* build default name */
			strcpy (pk2info->sm[i].sta,  pk2info->head.rwParms.misc.stnID);
			sprintf (pk2info->sm[i].comp, "%d", i);
			strcpy (pk2info->sm[i].net, NetCode);
			strcpy (pk2info->sm[i].loc,  "");
		}
		else
		{
			strcpy (pk2info->sm[i].sta,  pChanName[chanind].sta);
			strcpy (pk2info->sm[i].comp, pChanName[chanind].comp);
			strcpy (pk2info->sm[i].net,  pChanName[chanind].net);
			strcpy (pk2info->sm[i].loc,  pChanName[chanind].loc);
		}
    }

    if(K2EW_Debug)
        logit("e", "\n|%s| |%d| |%f| |%s| |%s|\n",
            pk2info->head.rwParms.misc.stnID, 
			pk2info->head.rwParms.misc.serialNumber, dt,
            fname, pk2info->head.rwParms.misc.comment);

/* Read and process each channel individually.
 *********************************************/
    for(i=0;i<nchans;i++) 
	{
           rewind(fp);
           ret = read_tag(fp, &pk2info->tag);
	if ( ret != EW_SUCCESS ) return EW_FAILURE;
           stat = read_head(fp, &pk2info->head, pk2info->tag.length);
           if ( stat != EW_SUCCESS )
              return stat;

		/* initialize trace buffer */
		for (j=0; j<MAXTRACELTH; j++) 
		{
			pk2info->Databuf[i][j] = 0.0;
			pk2info->Counts[i][j] = 0;
		}
		pk2info->numDataPoints[i] = 0;


		for(j=0;j<nscans;j++) 
		{
               ret = read_tag(fp, &pk2info->tag);
				if ( ret != EW_SUCCESS ) return EW_FAILURE;

               if(pk2info->numDataPoints[i] > MAXTRACELTH)
               {
                 /* DK 20030108 BEGIN */
                 /* this isn't good, this means that we've written
                    passed the end of the data allocation area in
                    the struct, thus corrupting the next channel or
                    the number of datapoints per channel.
                  ************************************************/
                 rc = EW_FAILURE;
                 logit("","k2evt2ew(): exceeded MAXTRACELTH (%d <= %d)\n",
                          MAXTRACELTH, pk2info->numDataPoints[i]); 
                 /* DK 20030108 END */
                 break;
               }
               read_frame(fp, &pk2info->frame, &channels);
               flen = pk2info->tag.dataLength;
               if ( fread(g_k2mi_buff, 1, flen, fp) != 1 )
               {
                 logit ("e", "k2evt2ew: read of file failed.\n");
                 return EW_FAILURE;
               }

               ret = extract(flen, g_k2mi_buff, pk2info->Databuf[i], pk2info->Counts[i], 
			scale[i], nchans, i, &pk2info->numDataPoints[i], MAXTRACELTH);
               if(ret)
               {
                 if(ret == 1)
                 {
                   rc = EW_WARNING;
                   logit("t","k2evt2ew(): extract(): returned warning while processing "
                             " channel (%s,%s,%s).  Data was truncated after %d datapoints.\n",
                         pk2info->sm[i].sta, pk2info->sm[i].comp, pk2info->sm[i].net, 
                         pk2info->numDataPoints[i]);
                 }
                 else 
                 {
                   rc = EW_FAILURE;
                   logit("t","k2evt2ew(): extract(): returned ERROR while processing "
                             " channel (%s,%s,%s).\n",
                         pk2info->sm[i].sta, pk2info->sm[i].comp, pk2info->sm[i].net); 
                 }
                 
                 break;  /* this moves us to the next channel */
               }
               /* DK 20030108  Added MAXTRACELTH param to extract() call */
		}

            if(K2EW_Debug)
                logit("e", "%2d |%s| |%s| |%s| %3d\n",
                           i, pk2info->sm[i].sta, 
                           pk2info->sm[i].comp, pk2info->sm[i].net, azim[i]);

		dt  = (float)(1.0/(10.0*(flen/(3.0*nchans))));

		peak_ground(pk2info->Databuf[i], pk2info->numDataPoints[i], 
				itype[i], dt, &pk2info->sm[i]);

         /* JHL 20040223 Set the sm time to the trigger time */
         pk2info->sm[i].t = pk2info->head.roParms.stream.triggerTime +  
                     (float)pk2info->head.roParms.stream.triggerTimeMsec/1000.0;
         pk2info->sm[i].t += decade;
         /* JHL 20101022 Set the length of time scanned. */
         pk2info->sm[i].length = pk2info->numDataPoints[i]*dt;
    } /* for loop over channels */
    return rc;
}

/******************************************************************************
 * read_tag(fp)  Read the 16 byte tag, swap bytes if necessary, and print.    *
 ******************************************************************************/

int read_tag (FILE *fp, KFF_TAG *tag)
{
	if ((fp == NULL) || (tag == NULL))
	{
		logit ("e", "read_tag: Invalid arguments passed in\n");
		return EW_FAILURE;
	}

    if (fread(tag, 1, 16, fp) != 16)
	{
		logit ("e", "read_tag: read of file failed.\n");
		return EW_FAILURE;
	}

#ifdef _INTEL
    SwapUint32 (&tag->type);
    SwapShort ((short *)&tag->length);
    SwapShort ((short *)&tag->dataLength);
    SwapShort ((short *)&tag->id);
    SwapShort ((short *)&tag->checksum);
#endif /* _INTEL */

    if (K2EW_Debug > 0) 
	{
        logit("e", "TAG: %c %d %d %d %d %d %d %d %d\n",
                tag->sync,
                (int)tag->byteOrder,
                (int)tag->version,
                (int)tag->instrumentType,
                tag->type, tag->length, tag->dataLength,
                tag->id, tag->checksum);
    }

	/* look ahead, and check on the upcoming record for sanity
	**********************************************************/
	{
		long fpos;
		char checkBuffer[MAX_REC];
		unsigned short checksum=0;
		int bytesToCheck;
		int i;

		fpos = ftell (fp); /* remember where we were */
		bytesToCheck = tag->length + tag->dataLength;
		if (bytesToCheck > MAX_REC) 
		{
			logit ("e", "read_tag: record too long.\n");
			logit ("e", "record + data length > MAX_REC.\n");
			return EW_FAILURE;
		}
		if (fread(checkBuffer, 1, bytesToCheck, fp) != bytesToCheck)
		{
			logit ("e", "read_tag: read of file failed.\n");
			return EW_FAILURE;
		}
		/* look at the synch character */
		if( tag->sync != 'K')
		{
			logit ("e", "read_tag: bad synch character.\n");
			return EW_FAILURE;
		}
		for ( i=0; i<bytesToCheck; i++)
			checksum = checksum + (unsigned char) checkBuffer[i];
		if (checksum != tag->checksum)
		{
			logit("","read_tag: checksum error\n");
			return EW_FAILURE;
		}

		/* now put things back the way they were */
		fseek(fp, fpos, SEEK_SET );
	}


    return EW_SUCCESS;
}

/******************************************************************************
 * read_head(fp)  Read the file header, swap bytes if necessary, and print.   *
 ******************************************************************************/

int read_head (FILE *fp, MW_HEADER *head, int tagLength)
{
   int        i, maxchans, siz;

	if ((fp == NULL) || (head == NULL))
	{
		logit ("e", "read_head: Invalid arguments passed in\n");
		return EW_FAILURE;
	}

/* Read in the file header.
   If a K2, there will be 2040 bytes,
   otherwise assume a Mt Whitney.
 ************************************/
    maxchans = tagLength==2040? MAX_K2_CHANNELS:MAX_MW_CHANNELS;

    if (fread(head, 1, 8, fp) != 8)
	{
		logit ("e", "read_head: read of file failed.\n");
		return EW_FAILURE;
	}

	siz = sizeof(struct MISC_RO_PARMS) + sizeof(struct TIMING_RO_PARMS);
    if (fread(&head->roParms.misc, 1, siz, fp) != (unsigned int) siz)
	{
		logit ("e", "read_head: read of file failed.\n");
		return EW_FAILURE;
	}

	siz = sizeof(struct CHANNEL_RO_PARMS)*maxchans;
    if (fread(&head->roParms.channel[0], 1, siz, fp) != (unsigned int) siz)
	{
		logit ("e", "read_head: read of file failed.\n");
		return EW_FAILURE;
	}

	siz = sizeof(struct STREAM_RO_PARMS);
    if (fread(&head->roParms.stream, 1, siz, fp) != (unsigned int) siz)
	{
		logit ("e", "read_head: read of file failed.\n");
		return EW_FAILURE;
	}

	siz = sizeof(struct MISC_RW_PARMS)+sizeof(struct TIMING_RW_PARMS);
    if (fread(&head->rwParms.misc, 1, siz, fp) != (unsigned int) siz)
	{
		logit ("e", "read_head: read of file failed.\n");
		return EW_FAILURE;
	}

    siz = sizeof(struct CHANNEL_RW_PARMS)*maxchans;
    if (fread(&head->rwParms.channel[0], 1, siz, fp) != (unsigned int) siz)
	{
		logit ("e", "read_head: read of file failed.\n");
		return EW_FAILURE;
	}

    if(tagLength==2040) {
        siz = sizeof(struct STREAM_K2_RW_PARMS);
        if (fread(&head->rwParms.stream, 1, siz, fp) != (unsigned int) siz)
		{
			logit ("e", "read_head: read of file failed.\n");
			return EW_FAILURE;
		}
    } else {
        siz = sizeof(struct STREAM_MW_RW_PARMS);
        if (fread(&head->rwParms.stream, 1, siz, fp) != (unsigned int) siz)
		{
			logit ("e", "read_head: read of file failed.\n");
			return EW_FAILURE;
		}
    }
    siz = sizeof(struct MODEM_RW_PARMS);
    if (fread(&head->rwParms.modem, 1, siz, fp) != (unsigned int) siz)
	{
		logit ("e", "read_head: read of file failed.\n");
		return EW_FAILURE;
	}

#ifdef _INTEL
	/* Byte-swap values, if necessary */

	/* roParms */
    SwapShort ((short *)&head->roParms.headerVersion);
    SwapShort ((short *)&head->roParms.headerBytes);

    SwapShort ((short *)&head->roParms.misc.installedChan);
    SwapShort ((short *)&head->roParms.misc.maxChannels);
    SwapShort ((short *)&head->roParms.misc.sysBlkVersion);
    SwapShort ((short *)&head->roParms.misc.bootBlkVersion);
    SwapShort ((short *)&head->roParms.misc.appBlkVersion);
    SwapShort ((short *)&head->roParms.misc.dspBlkVersion);
    SwapShort (&head->roParms.misc.batteryVoltage);
    SwapShort ((short *)&head->roParms.misc.crc);
    SwapShort ((short *)&head->roParms.misc.flags);
    SwapShort (&head->roParms.misc.temperature);

    SwapShort ((short *)&head->roParms.timing.gpsLockFailCount);
    SwapShort ((short *)&head->roParms.timing.gpsUpdateRTCCount);
    SwapShort (&head->roParms.timing.acqDelay);
    SwapShort (&head->roParms.timing.gpsLatitude);
    SwapShort (&head->roParms.timing.gpsLongitude);
    SwapShort (&head->roParms.timing.gpsAltitude);
    SwapShort ((short *)&head->roParms.timing.dacCount);
    SwapShort (&head->roParms.timing.gpsLastDrift[0]);
    SwapShort (&head->roParms.timing.gpsLastDrift[1]);
    SwapUint32 (&head->roParms.timing.gpsLastTurnOnTime[0]);
    SwapUint32 (&head->roParms.timing.gpsLastTurnOnTime[1]);
    SwapUint32 (&head->roParms.timing.gpsLastUpdateTime[0]);
    SwapUint32 (&head->roParms.timing.gpsLastUpdateTime[1]);
    SwapUint32 (&head->roParms.timing.gpsLastLockTime[0]);
    SwapUint32 (&head->roParms.timing.gpsLastLockTime[1]);

    SwapUint32 (&head->roParms.stream.startTime);
    SwapUint32 (&head->roParms.stream.triggerTime);
    SwapUint32 (&head->roParms.stream.duration);
    SwapShort ((short *)&head->roParms.stream.errors);
    SwapShort ((short *)&head->roParms.stream.flags);
    SwapShort ((short *)&head->roParms.stream.startTimeMsec);
    SwapShort ((short *)&head->roParms.stream.triggerTimeMsec);
    SwapUint32 (&head->roParms.stream.nscans);


    for(i=0;i<head->roParms.misc.maxChannels;i++) 
	{
        SwapInt32 (&head->roParms.channel[i].maxPeak);
        SwapUint32 (&head->roParms.channel[i].maxPeakOffset);
        SwapInt32 (&head->roParms.channel[i].minPeak);
        SwapUint32 (&head->roParms.channel[i].minPeakOffset);
        SwapInt32 (&head->roParms.channel[i].mean);
	}


	/* rwParams */
    SwapShort ((short *)&head->rwParms.misc.serialNumber);
    SwapShort ((short *)&head->rwParms.misc.nchannels);
    SwapShort (&head->rwParms.misc.elevation);
    SwapFloat (&head->rwParms.misc.latitude);
    SwapFloat (&head->rwParms.misc.longitude);
    SwapShort (&head->rwParms.misc.userCodes[0]);
    SwapShort (&head->rwParms.misc.userCodes[1]);
    SwapShort (&head->rwParms.misc.userCodes[2]);
    SwapShort (&head->rwParms.misc.userCodes[3]);
    SwapUint32 (&head->rwParms.misc.cutler_bitmap);
    SwapUint32 (&head->rwParms.misc.channel_bitmap);

    SwapShort (&head->rwParms.timing.localOffset);

    for(i=0;i<head->rwParms.misc.nchannels;i++) 
	{
        SwapShort ((short *)&head->rwParms.channel[i].sensorSerialNumberExt);
        SwapShort (&head->rwParms.channel[i].north);
        SwapShort (&head->rwParms.channel[i].east);
        SwapShort (&head->rwParms.channel[i].up);
        SwapShort (&head->rwParms.channel[i].altitude);
        SwapShort (&head->rwParms.channel[i].azimuth);
        SwapShort ((short *)&head->rwParms.channel[i].sensorType);
        SwapShort ((short *)&head->rwParms.channel[i].sensorSerialNumber);
        SwapShort ((short *)&head->rwParms.channel[i].gain);
        SwapShort ((short *)&head->rwParms.channel[i].StaLtaRatio);
        SwapFloat (&head->rwParms.channel[i].fullscale);
        SwapFloat (&head->rwParms.channel[i].sensitivity);
        SwapFloat (&head->rwParms.channel[i].damping);
        SwapFloat (&head->rwParms.channel[i].naturalFrequency);
        SwapFloat (&head->rwParms.channel[i].triggerThreshold);
        SwapFloat (&head->rwParms.channel[i].detriggerThreshold);
        SwapFloat (&head->rwParms.channel[i].alarmTriggerThreshold);
    }

    SwapShort ((short *)&head->rwParms.stream.eventNumber);
    SwapShort ((short *)&head->rwParms.stream.sps);
    SwapShort ((short *)&head->rwParms.stream.apw);
    SwapShort ((short *)&head->rwParms.stream.preEvent);
    SwapShort ((short *)&head->rwParms.stream.postEvent);
    SwapShort ((short *)&head->rwParms.stream.minRunTime);
    SwapShort (&head->rwParms.stream.VotesToTrigger);
    SwapShort (&head->rwParms.stream.VotesToDetrigger);
    SwapShort (&head->rwParms.stream.Timeout);
    SwapShort ((short *)&head->rwParms.stream.TxBlkSize);
    SwapShort ((short *)&head->rwParms.stream.BufferSize);
    SwapShort ((short *)&head->rwParms.stream.SampleRate);
    SwapUint32 (&head->rwParms.stream.TxChanMap);
#endif /* _INTEL */

    if(K2EW_Debug > 0)
	{
	    logit("e", "HEADER: %c%c%c %d %hu %hu\n",
   	         head->roParms.id[0], head->roParms.id[1], head->roParms.id[2],
   	         (int)head->roParms.instrumentCode,
             head->roParms.headerVersion,
             head->roParms.headerBytes);

	    logit("e", "MISC_RO_PARMS: %d %d %d %hu %hu %hu %hu %hu %hu %d %hu %hu %d\n",
            (int)head->roParms.misc.a2dBits,
            (int)head->roParms.misc.sampleBytes,
            (int)head->roParms.misc.restartSource,
            head->roParms.misc.installedChan,
            head->roParms.misc.maxChannels,
            head->roParms.misc.sysBlkVersion,
            head->roParms.misc.bootBlkVersion,
            head->roParms.misc.appBlkVersion,
            head->roParms.misc.dspBlkVersion,
            head->roParms.misc.batteryVoltage,
            head->roParms.misc.crc,
            head->roParms.misc.flags,
            head->roParms.misc.temperature );


	    logit("e", "TIMING_RO_PARMS: %d %d %d %hu %hu %d %d %d %d %hu %d %d"
	               " %" PRIu32 " %" PRIu32 " %" PRIu32 " %" PRIu32
	               " %" PRIu32 " %" PRIu32 "\n",
            (int)head->roParms.timing.clockSource,
            (int)head->roParms.timing.gpsStatus,
            (int)head->roParms.timing.gpsSOH,
            head->roParms.timing.gpsLockFailCount,
            head->roParms.timing.gpsUpdateRTCCount,
            head->roParms.timing.acqDelay,
            head->roParms.timing.gpsLatitude,
            head->roParms.timing.gpsLongitude,
            head->roParms.timing.gpsAltitude,
            head->roParms.timing.dacCount,
            head->roParms.timing.gpsLastDrift[0],
            head->roParms.timing.gpsLastDrift[1],
            head->roParms.timing.gpsLastTurnOnTime[0],
            head->roParms.timing.gpsLastTurnOnTime[1],
            head->roParms.timing.gpsLastUpdateTime[0],
            head->roParms.timing.gpsLastUpdateTime[1],
            head->roParms.timing.gpsLastLockTime[0],
            head->roParms.timing.gpsLastLockTime[1] );


	    for(i=0;i<head->roParms.misc.maxChannels;i++) 
		{
        	logit("e", "CHANNEL_RO_PARMS: %d %" PRIu32 " %d %" PRIu32 " %d %d\n",
	        head->roParms.channel[i].maxPeak,
                head->roParms.channel[i].maxPeakOffset,
   	        head->roParms.channel[i].minPeak,
                head->roParms.channel[i].minPeakOffset,
                head->roParms.channel[i].mean,
                head->roParms.channel[i].aqOffset );
    	}

    	logit("e", "STREAM_RO_PARMS: %d %d %d %d %d %d %d %d\n",
            head->roParms.stream.startTime,
            head->roParms.stream.triggerTime,
            head->roParms.stream.duration,
            head->roParms.stream.errors,
            head->roParms.stream.flags,
            head->roParms.stream.startTimeMsec,
            head->roParms.stream.triggerTimeMsec,
            head->roParms.stream.nscans  );

	    logit("e", "MISC_RW_PARMS: %hu %hu %.5s %.33s %d %f %f %d %d %d %d"
	               " %d %" PRIo32 " %d %.17s %d %d\n",
            head->rwParms.misc.serialNumber,
            head->rwParms.misc.nchannels,
            head->rwParms.misc.stnID,
            head->rwParms.misc.comment,
            head->rwParms.misc.elevation,
            head->rwParms.misc.latitude,
            head->rwParms.misc.longitude,
           (int)head->rwParms.misc.cutlerCode,
           (int)head->rwParms.misc.minBatteryVoltage,
           (int)head->rwParms.misc.cutler_decimation,
           (int)head->rwParms.misc.cutler_irig_type,
            head->rwParms.misc.cutler_bitmap,
            head->rwParms.misc.channel_bitmap,
           (int)head->rwParms.misc.cutler_protocol,
            head->rwParms.misc.siteID,
           (int)head->rwParms.misc.externalTrigger,
           (int)head->rwParms.misc.networkFlag );

	    logit("e", "TIMING_RW_PARMS: %d %d %hu\n",
           (int)head->rwParms.timing.gpsTurnOnInterval,
           (int)head->rwParms.timing.gpsMaxTurnOnTime,
               head->rwParms.timing.localOffset  );

	    for(i=0;i<head->rwParms.misc.nchannels;i++) 
		{
        	logit("e", "CHANNEL_RW_PARMS: %s %d %d %d %d %d %d %d %d %d %d"
        	           " %d %d %d %d %d %f %f %f %f %f %f %f\n",
                   head->rwParms.channel[i].id,
                   head->rwParms.channel[i].sensorSerialNumberExt,
                   head->rwParms.channel[i].north,
                   head->rwParms.channel[i].east,
                   head->rwParms.channel[i].up,
                   head->rwParms.channel[i].altitude,
                   head->rwParms.channel[i].azimuth,
                   head->rwParms.channel[i].sensorType,
                   head->rwParms.channel[i].sensorSerialNumber,
                   head->rwParms.channel[i].gain,
               (int)head->rwParms.channel[i].triggerType,
               (int)head->rwParms.channel[i].iirTriggerFilter,
               (int)head->rwParms.channel[i].StaSeconds,
               (int)head->rwParms.channel[i].LtaSeconds,
                   head->rwParms.channel[i].StaLtaRatio,
               (int)head->rwParms.channel[i].StaLtaPercent,
                   head->rwParms.channel[i].fullscale,
                   head->rwParms.channel[i].sensitivity,
                   head->rwParms.channel[i].damping,
                   head->rwParms.channel[i].naturalFrequency,
                   head->rwParms.channel[i].triggerThreshold,
                   head->rwParms.channel[i].detriggerThreshold,
                   head->rwParms.channel[i].alarmTriggerThreshold);
		}

	    logit("e", "STREAM_K2_RW_PARMS: |%d| |%d| |%d| %d %d %d %d %d %d %d"
	               " %d %d %d %d %d %d %d %" PRIu32 "\n",
            (int)head->rwParms.stream.filterFlag,
            (int)head->rwParms.stream.primaryStorage,
            (int)head->rwParms.stream.secondaryStorage,
                head->rwParms.stream.eventNumber,
                head->rwParms.stream.sps,
                head->rwParms.stream.apw,
                head->rwParms.stream.preEvent,
                head->rwParms.stream.postEvent,
                head->rwParms.stream.minRunTime,
                head->rwParms.stream.VotesToTrigger,
                head->rwParms.stream.VotesToDetrigger,
            (int)head->rwParms.stream.FilterType,
            (int)head->rwParms.stream.DataFmt,
                head->rwParms.stream.Timeout,
                head->rwParms.stream.TxBlkSize,
                head->rwParms.stream.BufferSize,
                head->rwParms.stream.SampleRate,
                head->rwParms.stream.TxChanMap);

	} /* If debug */

    return EW_SUCCESS;

} /* read_head */

/******************************************************************************
 * read_frame(fp)  Read the frame header, swap bytes if necessary.            *
 ******************************************************************************/

int read_frame (FILE *fp, FRAME_HEADER *frame, uint32_t *channels )
{
    unsigned short   frameStatus, frameStatus2, samprate, streamnumber;
    unsigned char    BitMap[4];
    uint32_t    bmap;

	if ((fp == NULL) || (frame == NULL) || (channels == NULL))
	{
		logit ("e", "read_frame: invalid arguments passed in.\n");
		return EW_FAILURE;
	}

    if (fread(frame, 32, 1, fp) != 1)
	{
		logit ("e", "read_frame: read of file failed.\n");
		return EW_FAILURE;
	}

#ifdef _INTEL
    SwapShort ((short *)&frame->recorderID);
    SwapShort ((short *)&frame->frameSize);
    SwapShort ((short *)&frame->blockTime1);
    SwapShort ((short *)&frame->blockTime2);
    SwapShort ((short *)&frame->channelBitMap);
    SwapShort ((short *)&frame->streamPar);
    SwapShort ((short *)&frame->msec);
#endif /* _INTEL */

    BitMap[0] = frame->channelBitMap & 255;
    BitMap[1] = frame->channelBitMap >> 8;
    BitMap[2] = frame->channelBitMap1;
    BitMap[3] = 0;

    bmap = *((uint32_t *)(BitMap));
    frameStatus      = frame->frameStatus;
    frameStatus2     = frame->frameStatus2;
    samprate         = frame->streamPar & 4095;
    streamnumber     = frame->streamPar >> 12;
    
    if(K2EW_Debug > 0)
   	    logit("e", "FRAME: %d %d %d %d   %" PRIu32 " X%0hX   "
   	               "%hu X%0hX %hu %hu     X%0hX X%0hX %hu X%0hX\n",
            (int)frame->frameType,
            (int)frame->instrumentCode,
            frame->recorderID,
            frame->frameSize,
            *(uint32_t *)&frame->blockTime1,
            frame->channelBitMap,
            frame->streamPar,
            streamnumber, samprate, (unsigned short) ( samprate >> 8 ),
            frameStatus,
            frameStatus2,
            frame->msec,
            (unsigned short)frame->channelBitMap1);

    *channels = bmap;

    return EW_SUCCESS;
}

/******************************************************************************
 * extract: Processes k2 file buffers to expand data from 24 bits to          *
 *         36 bits, sort channels and scale to cgs units.                     *
 *         pktdatalen - length of data in buffer                              *
 *         pktbuff    - address of buffer containing the data bytes           *
 *         Data       - the destination array.                                *
 *         scale      - the scaling factor (units/count).                     *
 *         nchan      - the total number of channels in the buffer.           *
 *         jchan      - the channel to be extracted.                          *
 *         ind        - the current index in the destination array.           *
 *         array_size - the size of the destination array.                    *
 *                                                                            *
 *         return values:                                                     *
 *                       0    Success                                         *
 *                       1    Extraction was truncated because packet         * 
 *                             would have cause overrun in destination array. *
 ******************************************************************************/

int extract(int pktdatalen, const unsigned char *pktbuff, float *Data,
            int32_t *Counts, float scale, int nchan, int jchan, int *ind,
            int array_size)
{
    int     k, pktpos, ichan;
    unsigned char btarr[4];
    int     rc  = 0;  /* DK 20030108  Added so that we could set return code in loop */
                          

  /* process and store serial data stream values */
    k = *ind;
    pktpos = ichan = 0;
    while (pktpos < pktdatalen) {
        /* loop through each serial data stream sample data value */
        if(ichan == jchan) {
            btarr[1] = pktbuff[pktpos++];    /* copy MSByte of 24-bit value */
            btarr[2] = pktbuff[pktpos++];    /* copy middle-byte of 24-bit value */
            btarr[3] = pktbuff[pktpos++];    /* copy LSByte of 24-bit value */
            /* sign extend to 32-bits */

            btarr[0] = ((btarr[1] & (unsigned char)0x80) ==
                      (unsigned char)0) ? (unsigned char)0 : (unsigned char)0xFF;

            /* byte-swap and enter value */
#ifdef _INTEL
			SwapInt32 ((int32_t *) btarr);
#endif
            /* DK 20030108  Make sure we don't overrun the destination arrays */
            if(k >= array_size)
            {
              /* Uh Oh!  We are about to write past the end of the array. 
                 Truncate operations here.
               ************************************************************/
               rc = 1;
               break;
            } 
          
            Counts[k] = (*((int32_t *)(btarr)));
            Data[k] = Counts[k]*scale;
			k = k + 1;
        } else {
            pktpos += 3;
        }
        ichan += 1;
        if(ichan >= nchan) ichan = 0;
    }

    *ind = k;
    return rc;  /* DK return rc which defaults to 0, but may be 1 */
}

/******************************************************************************
 *   subroutine for estimation of ground motion                               *
 *                                                                            *
 *   input:                                                                   *
 *           Data   - data array                                              *
 *           npts   - number of points in timeseries                          *
 *           itype  - units for timeseries. 1=disp 2=vel 3=acc                *
 *           dt     - sample spacing in sec                                   *
 *   return:                                                                  *
 *           0      - All OK                                                  *
 *           1      - error                                                   *
 *                                                                            *
 ******************************************************************************/

int peak_ground(float *Data, int npts, int itype, float dt, SM_INFO *sm)
{
    int    imax_acc, imax_vel, imax_dsp, imax_raw;
    int    imin_acc, imin_vel, imin_dsp, imin_raw;
    int    kk, icaus, maxtime;
    float  totint, tpi, omega, damp, rd, rv, aa;
    float  amax_acc, amin_acc, pmax_acc;
    float  amax_vel, amin_vel, pmax_vel;
    float  amax_dsp, amin_dsp, pmax_dsp;
    float  amax_raw, amin_raw, pmax_raw;

    /* Made these float arrays static because Solaris was Segfaulting on an
     * allocation this big from the stack.  These currently are 3.2 MB each 
     * DK 20030108
     ************************************************************************/
	  static float    d1[MAXTRACELTH];
	  static float    d2[MAXTRACELTH];
	  static float    d3[MAXTRACELTH];

    icaus = 1;

    tpi  = (float)(8.0*atan(1.0));

/* Find the raw maximum and its period
 *************************************/

    demean(Data, npts);
    amaxper(npts, dt, Data, &amin_raw, &amax_raw, &pmax_raw, &imin_raw, &imax_raw);

    if(itype == 1) {  /* input data is displacement  */
        for(kk=0;kk<npts;kk++) d1[kk] = Data[kk];
        locut(d1, npts, 0.17f, dt, 2, icaus);
        for(kk=1;kk<npts;kk++) {
            d2[kk] = (d1[kk] - d1[kk-1])/dt;
        }
        d2[0] = d2[1];
        demean(d2, npts);
        for(kk=1;kk<npts;kk++) {
            d3[kk] = (d2[kk] - d2[kk-1])/dt;
        }
        d3[0] = d3[1];
        demean(d3, npts);
    } else
    if(itype == 2) {  /* input data is velocity      */
        for(kk=0;kk<npts;kk++) d2[kk] = Data[kk];
        locut(d2, npts, 0.17f, dt, 2, icaus);
        for(kk=1;kk<npts;kk++) {
            d3[kk] = (d2[kk] - d2[kk-1])/dt;
        }
        d3[0] = d3[1];
        demean(d3, npts);

        totint = 0.0;
        for(kk=0;kk<npts-1;kk++) {
            totint = totint + (d2[kk] + d2[kk+1])*0.5f*dt;
            d1[kk] = totint;
        }
        d1[npts-1] = d1[npts-2];
        demean(d1, npts);

    } else
    if(itype == 3) {  /* input data is acceleration  */
        for(kk=0;kk<npts;kk++) d3[kk] = Data[kk];
        locut(d3, npts, 0.17f, dt, 2, icaus);

        totint = 0.0;
        for(kk=0;kk<npts-1;kk++) {
            totint = totint + (d3[kk] + d3[kk+1])*0.5f*dt;
            d2[kk] = totint;
        }
        d2[npts-1] = d2[npts-2];
        demean(d2, npts);

        totint = 0.0;
        for(kk=0;kk<npts-1;kk++) {
            totint = totint + (d2[kk] + d2[kk+1])*0.5f*dt;
            d1[kk] = totint;
        }
        d1[npts-1] = d1[npts-2];
        demean(d1, npts);
    } else {
        return 1;
    }

/* Find the displacement(cm), velocity(cm/s), & acceleration(cm/s/s) maxima  and their periods
 *********************************************************************************************/

    amaxper(npts, dt, &d1[0], &amin_dsp, &amax_dsp, &pmax_dsp, &imin_dsp, &imax_dsp);
    amaxper(npts, dt, &d2[0], &amin_vel, &amax_vel, &pmax_vel, &imin_vel, &imax_vel);
    amaxper(npts, dt, &d3[0], &amin_acc, &amax_acc, &pmax_acc, &imin_acc, &imax_acc);

/* Find the spectral response
 ****************************/

    damp = 0.05f;
    kk = 0;
    sm->pdrsa[kk] = 0.3;
    omega = (float)(tpi/sm->pdrsa[kk]);
    rdrvaa(&d3[0], npts-1, omega, damp, dt, &rd, &rv, &aa, &maxtime);
    sm->rsa[kk] = aa;
    sm->trsa[kk] = sm->t + dt*maxtime;
    kk += 1;

    sm->pdrsa[kk] = 1.0;
    omega = (float)(tpi/sm->pdrsa[kk]);
    rdrvaa(&d3[0], npts-1, omega, damp, dt, &rd, &rv, &aa, &maxtime);
    sm->rsa[kk] = aa;
    sm->trsa[kk] = sm->t + dt*maxtime;
    kk += 1;

    sm->pdrsa[kk] = 3.0;
    omega = (float)(tpi/sm->pdrsa[kk]);
    rdrvaa(&d3[0], npts-1, omega, damp, dt, &rd, &rv, &aa, &maxtime);
    sm->rsa[kk] = aa;
    sm->trsa[kk] = sm->t + dt*maxtime;
    kk += 1;

    sm->nrsa = kk;

    sm->pgd = fabs(amin_dsp)>fabs(amax_dsp)? fabs(amin_dsp):fabs(amax_dsp);
    sm->pgv = fabs(amin_vel)>fabs(amax_vel)? fabs(amin_vel):fabs(amax_vel);
    sm->pga = fabs(amin_acc)>fabs(amax_acc)? fabs(amin_acc):fabs(amax_acc);

    sm->tpgd = fabs(amin_dsp)>fabs(amax_dsp)? sm->t + dt*imin_dsp:sm->t + dt*imax_dsp;
    sm->tpgv = fabs(amin_vel)>fabs(amax_vel)? sm->t + dt*imin_vel:sm->t + dt*imax_vel;
    sm->tpga = fabs(amin_acc)>fabs(amax_acc)? sm->t + dt*imin_acc:sm->t + dt*imax_acc;

    return 0;
}

/******************************************************************************
 *  demean removes the mean from the n point series stored in array A.        *
 *                                                                            *
 ******************************************************************************/

void demean(float *A, int n)
{
    int       i;
    float    xm;

    xm = 0.0;
    for(i=0;i<n;i++) xm = xm + A[i];
    xm = xm/n;
    for(i=0;i<n;i++) A[i] = A[i] - xm;
}

/******************************************************************************
 *  Butterworth locut filter order 2*nroll (nroll<=8)                         *
 *   (see Kanasewich, Time Sequence Analysis in Geophysics,                   *
 *   Third Edition, University of Alberta Press, 1981)                        *
 *  written by W. B. Joyner 01/07/97                                          *
 *                                                                            *
 *  s[j] input = the time series to be filtered                               *
 *      output = the filtered series                                          *
 *      dimension of s[j] must be at least as large as                        *
 *        nd+3.0*float(nroll)/(fcut*delt)                                     *
 *  nd    = the number of points in the time series                           *
 *  fcut  = the cutoff frequency                                              *
 *  delt  = the timestep                                                      *
 *  nroll = filter order                                                      *
 *  causal if icaus.eq.1 - zero phase shift otherwise                         *
 *                                                                            *
 * The response is given by eq. 15.8-6 in Kanasewich:                         *
 *  Y = sqrt((f/fcut)**(2*n)/(1+(f/fcut)**(2*n))),                            *
 *                 where n = 2*nroll                                          *
 *                                                                            *
 * Dates: 01/07/97 - Written by Bill Joyner                                   *
 *        12/17/99 - D. Boore added check for fcut = 0.0, in which case       *
 *                   no filter is applied.  He also cleaned up the            *
 *                   appearance of the code (indented statements in           *
 *                   loops, etc.)                                             *
 *        02/04/00 - Changed "n" to "nroll" to eliminate confusion with       *
 *                   Kanesewich, who uses "n" as the order (=2*nroll)         *
 *        03/01/00 - Ported to C by Jim Luetgert                              *
 *                                                                            *
 ******************************************************************************/

void locut(float *s, int nd, float fcut, float delt, int nroll, int icaus)
{
    float    fact[8], b1[8], b2[8];
    float    pi, w0, w1, w2, w3, w4, w5, xp, yp, x1, x2, y1, y2;
    int       j, k, np2, npad;

    if (fcut == 0.0) return;       /* Added by DMB  */

    pi = (float)(4.0*atan(1.0));
    w0 = (float)(2.0*pi*fcut);
    w1 = (float)(2.0*tan(w0*delt/2.0));
    w2 = (float)((w1/2.0)*(w1/2.0));
    w3 = (float)((w1*w1)/2.0 - 2.0);
    w4 = (float)(0.25*pi/nroll);

    for(k=0;k<nroll;k++) {
        w5 = (float)(w4*(2.0*k + 1.0));
        fact[k] = (float)(1.0/(1.0+sin(w5)*w1 + w2));
        b1[k] = w3*fact[k];
        b2[k] = (float)((1.0-sin(w5)*w1 + w2)*fact[k]);
    }

    np2 = nd;

    if(icaus != 1) {
        npad = (int)(3.0*nroll/(fcut*delt));
        np2 = nd+npad;
        for(j=nd;j<np2;j++) s[j] = 0.0;
    }

    for(k=0;k<nroll;k++) {
        x1 = x2 = y1 = y2 = 0.0;
        for(j=0;j<np2;j++) {
            xp = s[j];
            yp = (float)(fact[k]*(xp-2.0*x1+x2) - b1[k]*y1 - b2[k]*y2);
            s[j] = yp;
            y2 = y1;
            y1 = yp;
            x2 = x1;
            x1 = xp;
        }
    }

    if(icaus != 1) {
        for(k=0;k<nroll;k++) {
            x1 = x2 = y1 = y2 = 0.0;
            for(j=0;j<np2;j++) {
                xp = s[np2-j-1];
                yp = (float)(fact[k]*(xp-2.0*x1+x2) - b1[k]*y1 - b2[k]*y2);
                s[np2-j-1] = yp;
                y2 = y1;
                y1 = yp;
                x2 = x1;
                x1 = xp;
            }
        }
    }

    return;
}

/******************************************************************************
 * rdrvaa                                                                     *
 *                                                                            *
 * This is a modified version of "Quake.For", originally                      *
 * written by J.M. Roesset in 1971 and modified by                            *
 * Stavros A. Anagnostopoulos, Oct. 1986.  The formulation is                 *
 * that of Nigam and Jennings (BSSA, v. 59, 909-922, 1969).                   *
 * Dates: 02/11/00 - Modified by David M. Boore, based on RD_CALC             *
 *        03/01/00 - Ported to C by Jim Luetgert                              *
 *                                                                            *
 *     acc = acceleration time series                                         *
 *      na = length of time series                                            *
 *   omega = 2*pi/per                                                         *
 *    damp = fractional damping (e.g., 0.05)                                  *
 *      dt = time spacing of input                                            *
 *      rd = relative displacement of oscillator                              *
 *      rv = relative velocity of oscillator                                  *
 *      aa = absolute acceleration of oscillator                              *
 * maxtime = time of max aa                                                   *
 ******************************************************************************/

void rdrvaa(float *acc, int na, float omega, float damp, float dt,
            float *rd, float *rv, float *aa, int *maxtime)
{
    float    omt, d2, bom, d3, omd, om2, omdt, c1, c2, c3, c4, cc, ee;
    float    s1, s2, s3, s4, s5, a11, a12, a21, a22, b11, b12, b21, b22;
    float    y, ydot, y1, z, z1, z2, ra;
    int       i;

    omt  = omega*dt;
    d2   = (float)(sqrt(1.0-damp*damp));
    bom  = damp*omega;
    d3   = (float)(2.0*bom);
    omd  = omega*d2;
    om2  = omega*omega;
    omdt = omd*dt;
    c1 = (float)(1.0/om2);
    c2 = (float)(2.0*damp/(om2*omt));
    c3 = c1+c2;
    c4 = (float)(1.0/(omega*omt));
    ee = (float)(exp(-damp*omt));
    cc = (float)(cos(omdt)*ee);
    s1 = (float)(sin(omdt)*ee/omd);
    s2 = s1*bom;
    s3 = s2 + cc;
    s4 = (float)(c4*(1.0-s3));
    s5 = s1*c4 + c2;

    a11 =  s3;          a12 = s1;
    a21 = -om2*s1;      a22 = cc - s2;

    b11 =  s3*c3 - s5;  b12 = -c2*s3 + s5 - c1;
    b21 = -s1+s4;       b22 = -s4;

    y = ydot = *rd = *rv = *aa = 0.0;
    *maxtime = -1;
    for(i=0;i<na-1;i++) {
        y1   = a11*y + a12*ydot + b11*acc[i] + b12*acc[i+1];
        ydot = a21*y + a22*ydot + b21*acc[i] + b22*acc[i+1];
        y = y1;/* y is the oscillator output at time corresponding to index i */
        z = (float)fabs(y);
        if (z > *rd) *rd = z;
        z1 = (float)fabs(ydot);
        if (z1 > *rv) *rv = z1;
        ra = -d3*ydot -om2*y1;
        z2 = (float)fabs(ra);
        if (z2 > *aa) {
        	*aa = z2;
        	*maxtime = i;
        }
    }
}

/******************************************************************************
 *   compute maximum amplitude and its associated period                      *
 *                                                                            *
 *   input:                                                                   *
 *           npts   - number of points in timeseries                          *
 *           dt     - sample spacing in sec                                   *
 *           fc     - input timeseries                                        *
 *   output:                                                                  *
 *           amaxmm - raw maximum                                             *
 *           pmax   - period of maximum                                       *
 *           imax   - index of maxmimum point                                 *
 *                                                                            *
 ******************************************************************************/

void amaxper(int npts, float dt, float *fc, float *aminmm, float *amaxmm,
                       float *pmax, int *imin, int *imax)
{
    float    amin, amax, pp, pm, mean, frac;
    int       i, j, jmin, jmax;

    *imax = jmax = *imin = jmin = 0;
    amax = amin = *amaxmm = *aminmm = fc[0];
    *aminmm = *pmax = mean = 0.0;
    for(i=0;i<npts;i++) {
        mean = mean + fc[i]/npts;
        if (fc[i] > amax) { jmax = i; amax = fc[i]; }
        if (fc[i] < amin) { jmin = i; amin = fc[i]; }
    }

/*     compute period of maximum    */

    pp = pm = 0.0;
    if (fc[jmax] > mean) {
        j = jmax+1;
        while(fc[j] > mean && j < npts) {
            pp += dt;
            j  += 1;
        }
        frac = dt*(mean-fc[j-1])/(fc[j]-fc[j-1]);
        frac = 0.0;
        pp = pp + frac;
        j = jmax-1;
        while(fc[j] > mean && j >= 0) {
            pm += dt;
            j  -= 1;
        }
        frac = dt*(mean-fc[j+1])/(fc[j]-fc[j+1]);
        frac = 0.0;
        pm = pm + frac;
    } else {
        j = jmax+1;
        if(fc[j] < mean && j < npts) {
            pp += dt;
            j  += 1;
        }
        frac = dt*(mean-fc[j-1])/(fc[j]-fc[j-1]);
        frac = 0.0;
        pp = pp + frac;
        j = jmax-1;
        if(fc[j] < mean && j >= 0) {
            pm += dt;
            j  -= 1;
        }
        frac = dt*(mean-fc[j+1])/(fc[j]-fc[j+1]);
        frac = 0.0;
        pm = pm + frac;
    }

    *imin = jmin;
    *imax = jmax;
    *pmax = (float)(2.0*(pm+pp));
    *aminmm = amin;
    *amaxmm = amax;

    return;
}
