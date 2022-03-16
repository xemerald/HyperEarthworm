/*
*
*    Based on SUDS putaway and Geomag WDC putaway.
*    For Erol Kalcan
*
*/

/* cosmos0putaway.c

Routines for writing trace data in COSMOS V0 format.

Volume 0 files have raw data values in digital counts, obtained directly from the native binary files of
the recording instrument. (Records obtained by digitizing analog film generally do not have a V0
file.) V0 files may have almost no quality control or checking, and so they may be treated as internal
files by networks. V0 files have adequate information in the headers to be converted to files with
physical values. There is one component per file, so one record obtained by a triaxial recorder will
result in three files.

Header information is populated by a database of files. We don't need to know
about it after we use the SCNL to find the right file and prepend it.

Count data is extracted from a wave server:

Data Section:
• The first line of the data section includes the type of data (accel., velocity, displ., etc.) and its
units, the number of data points, and the format to be used in reading the data. The format
will type be for integer or real values, as appropriate; the format for real values can be
floating point (e.g., 8f10.5) or exponential (e.g., 5e16.7). Although not required, 80-character
line lengths are most convenient for many engineering strong motion data users.
• The first line is followed by as many lines of data as are needed to accommodate the number
of points, given the format in which the data values are written.

In our case we'll alway use units=counts, and interger Format=(10I8):
First line example followed by a data line.
17629    raw accel.   pts, approx  176 secs, units=counts (50),Format=(10I8)
-28596  -28611  -28630  -28617  -28609  -28550  -28543  -28654  -28698  -28661

*/

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "earthworm.h"
#include "trace_buf.h"
#include "swap.h"
#include "ws_clientII.h"
#include "cosmos0head.h"
#include "cosmos0putaway.h"
#include "pa_subs.h"
#include "earthworm_simple_funcs.h"
#include "chron3.h"
 



// just testing
// sprintf(global_outputdir, "e:\earthworm\memphis\data\database_v0");



static  long   *COSMOS0Buffer;           /* write out COSMOS0 data as long integers */
static  char    COSMOS0OutputFormat[MAXTXT];
static  long    LineMean;            /* average of the 60 samples in the line */



/* Internal Function Prototypes */
static int StructMakeLocal(void *, int, char, int);
/************************************************************************
* Initialization function,                                              *
*       This is the Put Away startup intializer. This is called when    *
*       the system first comes up. Here is a chance to look around      *
*       and see if it's possible to do business, and to complain        *
*       if not ,BEFORE an event has to be processed.                    *
*                                                                       *
*       For COSMOS0, all we want to do is to make sure that the       *
*       directory where files should be written exists.                 *
*************************************************************************/
int COSMOS0PA_init(int OutBufferLen, char *OutDir, char *OutputFormat,
	int debug)
{
	/** Allocate COSMOS0Buffer and COSMOS0BufferShort
	We waste RAM by allocating both the long and short buffers here
	at the beginning of the code because some fluke (feature?) of NT
	which I don't understand becomes unhappy if we do the allocation
	later. Win2000, of course, behaves differently, and is quite happy
	with buffer allocation after we have determined the format of the
	incoming data */
	if ((COSMOS0Buffer = (long *)malloc(OutBufferLen * sizeof(char))) == NULL)
	{
		logit("et", "COSMOS0PA_init: couldn't malloc COSMOS0Buffer\n");
		return EW_FAILURE;
	}

	/* Allocate the meta structures */
	if ((cosmos_info = (COSMOS *)calloc(1, sizeof(COSMOS)))
		== (COSMOS *)NULL) {
		logit("e",
			"Out of memory for COSMOS structures.\n");
		return EW_FAILURE;
	}
	sprintf(cosmos_info->OutDir, "%s", OutDir);
	/* Make sure that the top level output directory exists */
	if (RecursiveCreateDir(cosmos_info->OutDir) != EW_SUCCESS)
	{
		logit("e", "COSMOS0PA_init: Call to RecursiveCreateDir failed\n");
		return EW_FAILURE;
	}

	if (strlen(OutputFormat) >= sizeof(COSMOS0OutputFormat))
	{
		logit("", "COSMOS0PA_init: Error: OutputFormat(%s) is too long! Quitting!\n",
			OutputFormat);
		return(EW_FAILURE);
	}
	else
	{
		strcpy(COSMOS0OutputFormat, OutputFormat);
	}


	return EW_SUCCESS;
}

/************************************************************************
*   This is the Put Away event initializer. It's called when a snippet  *
*   has been received, and is about to be processed.                    *
*   It gets to see the pointer to the TraceRequest array,               *
*   and the number of loaded trace structures.                          *
*                                                                       *
*   We need to make sure that the target directory                      *
*   exists, create it if it does not, then open the COSMOS0 file        *
*   for writing.                                                        *
*                                                                       *
*   This also opens the arc hypo file, and writes its struct to our     *
*   struct                                                              *
*************************************************************************/
int COSMOS0PA_next_ev(TRACE_REQ *ptrReq,
	char *OutDir, char *LibDir, char *EventDate, char *EventTime,
	int cadence, int debug, double Multiplier)

{
	char    COSMOS0LibFile[4 * MAXTXT];
/*	char    c; */
	char    year[5];
	char    yr[3];
	char    mo[3];
	char    dy[3];
	char    hr[3];
	char    mn[3];
	char    sec[7];
	char	cos_date[35]; /* Start date of data requested */
	char    record_id[35];
	char    tempstr[COSMOSLINELEN];
	int     LineNumber = 0;
	char		time_type[30] = { 0 };					/* Time type UTC or local */
	size_t	nfbuffer;		/* Read bytes */
	char	*fbuffer;		/* File read buffer */
	static long	 MaxMessageSize = 100000;  /* size (bytes) of largest msg */


	/* Grab the date-related substrings that we need for filenames. */
	strncpy(year, EventDate, 4);
	year[4] = '\0';
	strncpy(yr, EventDate + 2, 2);
	yr[2] = '\0';
	strncpy(mo, EventDate + 4, 2);
	mo[2] = '\0';
	strncpy(dy, EventDate + 6, 2);
	dy[2] = '\0';

	strncpy(hr, EventTime, 2);
	hr[2] = '\0';
	strncpy(mn, EventTime + 2, 2);
	mn[2] = '\0';
	strncpy(sec, EventTime + 4, 5);
	sec[5] = '\0';

	cos_date[34] = '\0';
	record_id[34] = '\0';	
	tempstr[0]='\0';


	sprintf(cosmos_info->LibDir, "%s", LibDir);
	sprintf(cosmos_info->EventTime, "%s", EventTime);
	sprintf(cosmos_info->EventDate, "%s", EventDate);

	/* I think we already created this, but, it is OK to try again. */
	RecursiveCreateDir(OutDir);
	sprintf(COSMOS0LibFile, "%s/%s_%s_%s_%s.dlv0", 
		LibDir, ptrReq->net, ptrReq->sta, ptrReq->loc, ptrReq->chan);

	/* If there's enough time to implement, this filename should be able to be specified in the dot D file. */
	sprintf(cosmos_info->EventArcFile, "%s/event.arc", cosmos_info->LibDir);
	if (debug == 1)
		logit("t", "Attempting to open event file which always must be named event.arc and located in the LibraryDirectory. \n LibraryDirectory is currently configured as: %s\n",
			cosmos_info->LibDir);

	/* open Event file just for reading */
	if ((EVENTARCfp = fopen(cosmos_info->EventArcFile, "r")) == NULL)
	{
		logit("e", "COSMOS0PA_next_ev: unable to open file %s: %s\nAn .arc file is required to set event information in the COSMOS file format.",
			cosmos_info->EventArcFile, strerror(errno));
		return EW_FAILURE;
	}

	/* Read file to buffer
	*********************/
	fbuffer = (char*)malloc(sizeof(char) * MaxMessageSize); /*check logic*/
	if (fbuffer == NULL)
	{
		logit("et", "Unable to allocate memory for filter buffer\n");
		return -1;
	}

	nfbuffer = fread(fbuffer, sizeof(char),
		(size_t)MaxMessageSize, EVENTARCfp);
	fclose(EVENTARCfp); // Done reading
	if (nfbuffer == 0)
	{
		logit("eto", "COSMOS0PA_next_ev: No data read from %s\n\nAn .arc file is required to set event information in the COSMOS file format.",
			cosmos_info->EventArcFile);
		return EW_FAILURE;
	}
	/* We should be able to do something like 		origintime = arcmsg.sum.ot - GSEC1970; after parse_arc */
	if (debug == 1) logit("et", "COSMOS0PA_next_ev: Debug: Arc parsing %s\n", cosmos_info->EventArcFile);
	//if (parse_arc(fbuffer, &cosmos_info->arcmsg) != 0)
	if (parse_arc_no_shdw(fbuffer, &cosmos_info->arcmsg) != 0)
	{
		logit("et", "COSMOS0PA_next_ev: Error parsing %s\n",
			cosmos_info->EventArcFile);
	}



	return (EW_SUCCESS);
}


/************************************************************************
*   This we open the COSMOS0 file                                       *
*   for writing.                                                        *
*                                                                       *
*   This also opens the library file, and writes its contents to our    *
*   output file                                                         *
*************************************************************************/
int COSMOS0PA_header(TRACE_REQ *ptrReq,
	char *OutDir, char *LibDir, char *EventDate, char *EventTime,
	int cadence, int debug, double Multiplier)

{
	char    COSMOS0File[4 * MAXTXT];
	char    COSMOS0LibFile[4 * MAXTXT];
	/*	char    c; */
	char    year[5];
	char    yr[3];
	char    mo[3];
	char    dy[3];
	char    hr[3];
	char    mn[3];
	char    sec[7];
	char	cos_date[35]; /* Start date of data requested */
	char    record_id[35];
	char    str[COSMOSLINELEN];
	char    tempstr[COSMOSLINELEN];
	char    tempstr2[COSMOSLINELEN];
	char    fifteen[16];
	int     LineNumber = 0;
	time_t  rawtime;
	struct tm * timeinfo;
	static long	 MaxMessageSize = 100000;  /* size (bytes) of largest msg */
	HypoArc			arcmsg;			/* ARC message */
	char		timestr[80];					/* Holds time messages */
	time_t 		ot;
	struct tm * (*timefunc)(const time_t *);
	char		time_type[30] = { 0 };					/* Time type UTC or local */


														/* Grab the date-related substrings that we need for filenames. */
	strncpy(year, EventDate, 4);
	year[4] = '\0';
	strncpy(yr, EventDate + 2, 2);
	yr[2] = '\0';
	strncpy(mo, EventDate + 4, 2);
	mo[2] = '\0';
	strncpy(dy, EventDate + 6, 2);
	dy[2] = '\0';

	strncpy(hr, EventTime, 2);
	hr[2] = '\0';
	strncpy(mn, EventTime + 2, 2);
	mn[2] = '\0';
	strncpy(sec, EventTime + 4, 5);
	sec[5] = '\0';

	cos_date[34] = '\0';
	record_id[34] = '\0';
	tempstr[0] = '\0';


	sprintf(COSMOS0File, "%s/%s_%s_%s_%s_%s%s%s_%s%s.v0", OutDir,
		ptrReq->net, ptrReq->sta, ptrReq->loc, ptrReq->chan, year, mo, dy, hr, mn);

	/* cos_date "06/17/2018, 18:34:38.004 GMT (Q=5) "<-35 chars; Q stands for Quality, we have no way to know that */
	sprintf(cos_date, "%s/%s/%s, %s:%s:%s  UTC", mo, dy, year, hr, mn, sec);

	RecursiveCreateDir(OutDir);
	sprintf(COSMOS0LibFile, "%s/%s_%s_%s_%s.dlv0",
		LibDir, ptrReq->net, ptrReq->sta, ptrReq->loc, ptrReq->chan);

	if (debug == 1)
		logit("t", "Opening COSMOS0 library file header %s\n", COSMOS0LibFile);

	/* open library file just for reading */
	if ((COSMOS0Libfp = fopen(COSMOS0LibFile, "r")) == NULL)
	{
		logit("e", "COSMOS0PA_next_ev: unable to open file %s: %s\n",
			COSMOS0LibFile, strerror(errno));
		return EW_FAILURE;
	}
	else {
		logit("e", "COSMOS0PA_next_ev: Opened file %s\n",
			COSMOS0LibFile);
	}

	if (debug == 1)
		logit("et", "Opening COSMOS0 file %s\n", COSMOS0File);

	/* open file */
	if ((COSMOS0fp = fopen(COSMOS0File, "wt")) == NULL)
	{
		logit("e", "COSMOS0PA_next_ev: unable to open file %s: %s\n",
			COSMOS0File, strerror(errno));
		return EW_FAILURE;
	}
	arcmsg = cosmos_info->arcmsg;
	timefunc = localtime;
	ot = (time_t)(arcmsg.sum.ot - GSEC1970);
	timeinfo = timefunc(&ot);
	/* Copy the library file to the putaway file*/
	/*	c = fgetc(COSMOS0Libfp);
	while (c != EOF)
	{
	fputc(c, COSMOS0fp);
	c = fgetc(COSMOS0Libfp);
	}
	*/
	while ((fgets(str, COSMOSLINELEN, COSMOS0Libfp) != NULL) && (LineNumber < 48)) {
		LineNumber++;
		sprintf(tempstr, "                                                                                ");/*clear it*/
		sprintf(fifteen, "               ");
		switch (LineNumber) {
		case 2: /*  2 1-40 Earthquake name (before a name has been assigned, may appear as “Record of”; test
				records may use “Test Record of”, etc.).
				41-80 Earthquake date and time (including time zone). */
			strftime(timestr, 80, "Record of                 Earthquake of %B %d, %Y, %H:%M:%S   UTC", timeinfo);
			fprintf(COSMOS0fp, "%s\n", timestr);
			break;
		case 3: /*3 12-46 Hypocenter: 12-20 Latitude (positive North); 22-31 Longitude (positive East); and
				35-37 Depth (km); 40-46 Source agency for hypocenter information.
				47-80 Magnitude(s), including source agency for values (may be entered manually, so
				spacing may be variable).**/
			sprintf(tempstr, "Hypocenter: %9.4f %9.4f H=%3.0fkm Md=%3.1f, M=%3.1f",
				arcmsg.sum.lat, arcmsg.sum.lon, arcmsg.sum.z, arcmsg.sum.Md, arcmsg.sum.Mpref); 
			fprintf(COSMOS0fp, "%s\n", tempstr);
			break;
		case 4: /*	4 9-42 Earthquake origin date and time, in UTC (i.e., GMT), with source agency; 43-80,
				For use by preparing agency; may include note, e.g., “Preliminary Processing”, etc.*/

				/* Prepare origin time. Second can have tenths place, so leaving two spaces here to indicate
				we're not going to that level of precision, the timestruct doesn't have tenths. Probably worth revisiting.*/
			strftime(timestr, 80, "%m/%d/%Y, %H:%M:%S  ", timeinfo);
			sprintf(tempstr, "Origin: %s UTC", timestr);
			fprintf(COSMOS0fp, "%s\n", tempstr);
			break;
		case 8: /*  Record information:
				8 17-50 Record start time (date and time of first sample; may be approximate - precise value
				should be obtained from real header) and time quality (0 through 5 correspond to
				lowest through highest quality).
				59-80 Record identification, assigned by preparing agency.
				"06/17/2018, 18:34:38.004 GMT (Q=5) "<- 35 chars   " (Q=5) " "38199368.SB.WLA.00.HN " <- 22char*/
				/* strncpy(record_id, str + 45, 34); no, we're not going to use any of the prev string */
				/* We pad with white space, because we're going to overwrite some of it, and we'll only copy
				the relevant characters to go up to 80, andything too long will get truncated. */
			sprintf(tempstr, "Rcrd start time:%s                             ", cos_date);
			/* sprintf(tempstr2, "RcrdId:%lu.%s.%s.%s.%s                  ",
				arcmsg.sum.qid, ptrReq->net, ptrReq->sta, ptrReq->loc, ptrReq->chan); <- prism doesn't like this*/
			/* sprintf(tempstr2,    "RcrdId:%s.%lu.%s.%s.%s.%s     ", 
				ptrReq->net, arcmsg.sum.qid, ptrReq->net, ptrReq->sta, ptrReq->chan, ptrReq->loc); <- prism likes this but it might go beyond legal 80 chars/line */
			sprintf(tempstr2, "RcrdId: (see comment)                   ");

			strncpy(tempstr + 51, tempstr2, 29);
			tempstr[80] = '\0'; /* Chop off any extra junk */
			fprintf(COSMOS0fp, "%s\n", tempstr);
			break;
		case 10: /* 10 20-27 Length of raw record, as recorded (seconds); 45-54 Maximum of raw (uncorrected)
				 record in g (or other units);* 60-67 Time max occurs in raw record (secs after start).
				 Example:
				 Raw record length = 175.340  sec, Uncor max = 100000000, at   40.220 sec. */
			sprintf(tempstr, "Raw record length = %f", (ptrReq->reqEndtime - ptrReq->reqStarttime));
			strncpy(tempstr + 28, "sec, Uncor max =                                    ", 53);
			fprintf(COSMOS0fp, "%s\n", tempstr);
			break;
		case 11: /*"Record Information; Line 11
				 11-40 Processing/release date, time and agency.
				 48-80 Maximum of data series in file, units, and time of maximum (seconds after start).*"
				 What should I put here? An example file looks like:
				 "Processed: 06/17/18 12:00 PDT UCB                                   "
				 */
			time(&rawtime);
			timeinfo = gmtime(&rawtime);
			sprintf(tempstr, "Processed: %d/%d/%d %d:%01d UTC                                       ",
				(timeinfo->tm_mon + 1),
				timeinfo->tm_mday,
				(timeinfo->tm_year + 1900),
				timeinfo->tm_hour,
				timeinfo->tm_min
			);
			fprintf(COSMOS0fp, "%s\n", tempstr);
			break;
		/* Real Headers include Earthquake information similar to comments above, comments for humans, below for PRISM*/
		case 27: /* Site geology stuff from the database are the first 4 params in this line, so read them in and leave
				    them be. But the 5th param we'll overwrite here with Earthquake latitude (decimal degrees, North positive)*/
			sprintf(fifteen, "%15.6f", arcmsg.sum.lat);
			strncpy(str + 60, fifteen, 15);
			fprintf(COSMOS0fp, "%s", str);
			break;
		case 28: /* Earthquake longitude (decimal degrees, East positive), Earthquake depth (km)., Moment magnitude, M., Surface-wave magnitude, MS., Local magnitude, ML.*/
			sprintf(tempstr, "%15.6f%15.6f%15.6f%15.6f%15.6f", arcmsg.sum.lon, arcmsg.sum.z, -999.0, -999.0, arcmsg.sum.Mpref);
			fprintf(COSMOS0fp, "%s\n", tempstr);
			break;
		case 29: /* Other magnitude; I guess I'll put Md here, Epicentral distance to station (km)., Epicenter-to-station azimuth (CW from north) <- we don't have those two, so -999 */
			sprintf(fifteen, "%15.6f", arcmsg.sum.Md);
			strncpy(str, fifteen, 15);			
			sprintf(fifteen, "%15.6f", -999.0);
			strncpy(str + 15, fifteen, 15);
			strncpy(str + 30, fifteen, 15);
			fprintf(COSMOS0fp, "%s", str);
			break;
		case 47: /*AQMS normally writes this comment, but we're not using AQMS*/
				 /* Example: | RcrdId: NC.72282711.NC.C031.HNE.01 */
			sprintf(tempstr, "| RcrdId: %s.%lu.%s.%s.%s.%s ", 
				ptrReq->net, arcmsg.sum.qid, ptrReq->net, ptrReq->sta, ptrReq->chan, ptrReq->loc);
			fprintf(COSMOS0fp, "%s\n", tempstr);
			break;
		case 48: /*AQMS normally writes this comment, but we're not using AQMS*/
				 /*Example:|<SCNL>C031.HNE.NC.01    <AUTH> 2015/03/01 16:46:25.000*/
			sprintf(tempstr, "|<SCNL> %s.%s.%s.%s    <AUTH> %d/%d/%d %d:%d:%d.000",
				ptrReq->sta, ptrReq->chan, ptrReq->net, ptrReq->loc,
				(timeinfo->tm_year + 1900), (timeinfo->tm_mon + 1), timeinfo->tm_mday,
				timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
			fprintf(COSMOS0fp, "%s\n", tempstr);
			break;
		default:
			fprintf(COSMOS0fp, "%s", str);
		}

	}
	fclose(COSMOS0Libfp);
	return (EW_SUCCESS);
}

/************************************************************************
* This is the working entry point into the disposal system. This        *
* routine gets called for each trace snippet which has been recovered.  *
* It gets to see the corresponding SNIPPET structure, and the event id  *
*                                                                       *
* For COSMOS0, this routine writes to the COSMOS0 file, pointed to by   *
* the COSMOS0fp pointer, all of the received trace data in COSMOS0      *
* format:                                                               *
*                                                                       *
*      1. COSMOS0 tag - indicating what follows                         *
*      2. COSMOS0_STATIONCOMP struct - describe the station             *
*      3. COSMOS0 tag - indicating what follows                         *
*      4. COSMOS0_DESCRIPTRACE struct - describe the trace data         *
*      5. trace data                                                    *
*                                                                       *
*  One bit of complexity is that we need to write the files in the      *
*  correct byte-order. Based on the OutputFormat parameter, determine   *
*  whether or not to swap bytes before writing the COSMOS0 file.        *
*                                                                       *
* WARNING: we clip trace data to -2147483648 - +2147483647 so it will   *
*  fit in a long int. Any incoming data that is longer than 32 bits     *
*  will be CLIPPED. cjb 5/18/2001                                       *
*************************************************************************/
/* Process one channel of data */
int COSMOS0PA_next(TRACE_REQ *getThis, double GapThresh,
	long OutBufferLen, int debug,
	int Cadence, double Multiplier)
{
	TRACE2_HEADER *wf;
	char    datatype;
	char    hour_line[81] = ";                                                                                "; 
	char    sample[12];
	char    eightdigits[9];
	char   *msg_p;        /* pointer into tracebuf data */
	double  begintime = 0, starttime = 0, endtime = 0, currenttime = 0;
	double  samprate;
	float  *f_data;
	int     gap_count = 0;
	int     i, j;
	int     seconds = 0;
	int     s_place = 0;
	int     total, raw_counts;
	long    nfill_max = 0l;
	long    nsamp, nfill;
	long    nsamp_this_scn = 0l;
	long    this_size;
	long   *l_data;
	short  *s_data;
	char    tempstr[COSMOSLINELEN];
	char    tempstr2[COSMOSLINELEN];

	/* Put this in the .d file once we know we want it. */
	/*  double multiplier = 0.001; */

	/* Check arguments */
	if (getThis == NULL)
	{
		logit("e", "COSMOS0PA_next: invalid argument passed in.\n");
		return EW_FAILURE;
	}


	COSMOS0PA_header(getThis, cosmos_info->OutDir, cosmos_info->LibDir, cosmos_info->EventDate, cosmos_info->EventTime, Cadence, debug, Multiplier);
	/* Used for computing trace statistics */
	total = 0;

	if ((msg_p = getThis->pBuf) == NULL)   /* pointer to first message */
	{
		logit("e", "COSMOS0PA_next: Message buffer is NULL.\n");
		return EW_FAILURE;
	}
	wf = (TRACE2_HEADER *)msg_p;

	/* Look at the first TRACE2_HEADER and get set up of action */
	if (WaveMsg2MakeLocal(wf) < 0)
	{
		logit("e", "COSMOS0PA_next: unknown trace data type: %s\n",
			wf->datatype);
		return(EW_FAILURE);
	}

	nsamp = wf->nsamp;
	starttime = wf->starttime;
	endtime = wf->endtime;
	samprate = wf->samprate;
	if (samprate < 0.0001)
	{
		logit("et", "unreasonable samplerate (%f) for <%s.%s.%s.%s>\n",
			samprate, wf->sta, wf->chan, wf->net, wf->loc);
		return(EW_FAILURE);
	}
	/* LAST header line now that we know sample rate*/
	/* WRITE Dynamic header; this probably can't be here though because we need to calculate these results */
	/* First we'll write a line that looks like this: */
	/* 17770    raw accel.pts, approx  178 secs, units = counts(50), Format = (7I11) */
	/*Line Cols
	1st 1 - 8 Number of data points following; 10 - 21 Physical parameter of the data.
	35 - 38 Approximate length of record(rounded to nearest sec; see Rhdr(66) for precise value).
	52 - 58 Units of the data values(e.g., cm / sec2); 60 - 61 Index code for units
	71-80 Format of the data values written on the following lines.
	*/
	/*sprintf(hour_line, "17770    raw accel.pts, approx  178 secs, units=counts (50),Format=(10I8)\r\n"); /* we want leading spaces */

/*	sprintf(tempstr, "17770    raw accel.pts,   approx  178  secs, units=counts (50),Format=(10I8)   \n");*/
	sprintf(tempstr, "         raw accel.pts,   approx           , units=counts (50),Format=(10I8)    \n");
	raw_counts = (getThis->reqEndtime - getThis->reqStarttime ) * samprate; 
	sprintf(tempstr2, "%d", raw_counts);
	strncpy(tempstr, tempstr2, strlen(tempstr2));
	seconds = (getThis->reqEndtime - getThis->reqStarttime);
	sprintf(tempstr2, "%d secs", seconds);
	strncpy(tempstr + 34, tempstr2, strlen(tempstr2));
	if (!COSMOS0fp) {
		logit("et", "Tried to write to a null file pointer, exiting. Does your library match your waveserver request?\n");
		exit(0);
	}
	if (fwrite(tempstr, 81, 1, COSMOS0fp) != 1)
	{
		logit("et", "COSMOS0PA_next: error writing COSMOS0 dynamic header line. \n");
		return EW_FAILURE;
	}
	begintime = starttime;
	/* datatype i4 = intel byte order 4 bytes, s2 = sparc byte order; 2 bytes */
	datatype = 'n';
	if (wf->datatype[0] == 's' || wf->datatype[0] == 'i')
	{
		if (wf->datatype[1] == '2') datatype = 's';
		else if (wf->datatype[1] == '4') datatype = 'l';
	}
	else if (wf->datatype[0] == 't' || wf->datatype[0] == 'f')
	{
		if (wf->datatype[1] == '4') datatype = 'f';
	}
	if (datatype == 'n')
	{
		logit( "et", "COSMOS0PA_next: unsupported datatype: %s\n", wf->datatype );
		return(EW_FAILURE);
	}

	if (debug == 1)
		logit("et", "COSMOS0PA_next: working on <%s/%s/%s/%s> datatype: %c \n",
			wf->sta, wf->chan, wf->net, wf->loc, datatype);

	/********************** loop through all the messages for this s-c-n **********************/
	while (1)
	{
		/* advance message pointer to the data */
		msg_p += sizeof(TRACE2_HEADER);

		/* check for sufficient memory in output buffer */
		this_size = (nsamp_this_scn + nsamp) * sizeof(long);
		if (OutBufferLen < this_size)
		{
			logit("e", "out of space for <%s.%s.%s.%s>; saving long trace.\n",
				wf->sta, wf->chan, wf->net, wf->loc);
			break;
		}

		/* if data are floats, clip to longs cjb 5/18/2001 */
		switch (datatype)
		{
		case 's':
			s_data = (short *)msg_p;
			for (j = 0; j < nsamp; j++, nsamp_this_scn++)
				COSMOS0Buffer[nsamp_this_scn] = (long)s_data[j];
			msg_p += sizeof(short) * nsamp;
			break;
		case 'l':
			l_data = (long *)msg_p;
			for (j = 0; j < nsamp; j++, nsamp_this_scn++)
				COSMOS0Buffer[nsamp_this_scn] = l_data[j];
			msg_p += sizeof(long) * nsamp;
			break;
		case 'f':
			f_data = (float *)msg_p;
			/* CLIP the data to long int */
			for (j = 0; j < nsamp; j++, nsamp_this_scn++)
			{
				if (l_data[j] < (float)LONG_MIN)
					COSMOS0Buffer[nsamp_this_scn] = LONG_MIN;
				else if (l_data[j] > (float)LONG_MAX)
					COSMOS0Buffer[nsamp_this_scn] = LONG_MAX;
				else
					COSMOS0Buffer[nsamp_this_scn] = (long)l_data[j];
			}
			msg_p += sizeof(float) * nsamp;
			break;
		}

		/* msg_p has been advanced to the next TRACE_BUF; localize bytes *
		* and check for gaps.                                            */
		wf = (TRACE2_HEADER *)msg_p;
		if (WaveMsg2MakeLocal(wf) < 0)
		{
			if (debug == 1)
				logit("e", "COSMOS0PA_next: unknown trace data type or unexpected end of data: %s\n",
					wf->datatype);
			else
				logit("e", "COSMOS0PA_next: unknown trace data type or unexpected end of data.\n");
			break;
			//return(EW_FAILURE);
		}
		nsamp = wf->nsamp;
		starttime = wf->starttime;
		/* starttime is set for new packet; endtime is still set for old packet */
		if (endtime + (1.0 / samprate) * GapThresh < starttime)
		{
			/* there's a gap, so fill it */
			if (debug == 1)
				logit("e", "gap in %s.%s.%s.%s: %lf: %lf\n", wf->sta, wf->chan, wf->net,
					wf->loc, endtime, starttime - endtime);
			nfill = (long)(samprate * (starttime - endtime) - 1);
			if ((nsamp_this_scn + nfill) * (long)sizeof(long) > OutBufferLen)
			{
				logit( "e", "bogus gap (%ld); skipping\n", nfill );
				return(EW_FAILURE);
			}
			/* do the filling */
			for (j = 0; j < nfill; j++, nsamp_this_scn++)
				COSMOS0Buffer[nsamp_this_scn] = getThis->fill; // changed from local variable fill swl
														   /* keep track of how many gaps and the largest one */
			gap_count++;
			if (nfill_max < nfill)
				nfill_max = nfill;
		}
		/* Advance endtime to the new packet;        *
		* process this packet in the next iteration */
		endtime = wf->endtime;
	} /******************** end while(1) ***************************************************/

	  /* If the sample rate is 1 sample per minute then we'll have a sample rate of .016 */
	  /* For minute data we want 24 rows of 60 samples each, 1440 samples in a day. */
	  /* A single file fills the month of October for SJG, and includes four trace types */
	  /* FYXZ, so there are a total of 2976 lines in this file. */

	  /* Match our metadata with our waveserver request, if possible.
	  * observatory's value is -1 if there isn't a match. */

	currenttime = begintime;
	j = 0;


	while ((j < nsamp_this_scn) && (currenttime < getThis->reqEndtime) && (j < raw_counts))
	{
		/* Only give them what they asked for, not each sample we got back.
		Tracebufs contain multiple samples, and we may need to request
		an earlier one to get the start sample we need, or a later one
		for the last sample*/
		while (currenttime < getThis->reqStarttime) {
			currenttime = currenttime + 1 / samprate;
			j++;
		}

		s_place = 0;

		/* 35-394 60I6   60 6-digit 1-minute values for the given element for that data hour.
		*               The values are in tenth-minutes for D and I, and in
		*               nanoTeslas for the intensity elements.
		*/
		total = 0;
		LineMean = 0;

		i = 0;

		/* WRITE DATA */
		/* Even if there are more samples in this scn, we shouldn't have more than the raw counts the user asked for*/
		while (i < 10 && j < nsamp_this_scn && j < raw_counts)
		{
			sprintf(eightdigits, "        "); /* we want leading spaces */
			if (COSMOS0Buffer[j] != getThis->fill) {
				if (((int)(COSMOS0Buffer[j] * Multiplier) > 99999999) || ((int)(COSMOS0Buffer[j] * Multiplier) < -9999999)) {
					sprintf(sample, GAP_FILL);
					/* prevent out of range string */
				}
				else {
					sprintf(sample, "%d", (int)(COSMOS0Buffer[j] * Multiplier));
				}
				strcpy(eightdigits + 8 - strlen(sample), sample);
				strcpy(hour_line + s_place, eightdigits);
			}
			else {
				/* We have a gap, this is where gap data is written */
				//strcpy(hour_line + s_place, "  9999");
				sprintf(sample, GAP_FILL);
				strcpy(eightdigits + 8 - strlen(sample), sample);
				strcpy(hour_line + s_place, eightdigits);				
			}
			s_place = s_place + 8;

			total += (int)(COSMOS0Buffer[j] * Multiplier);

			j++; i++;
		}


		/* 401-402       Record end marker.
		*               Two chars 'cr'= 13 and 'nl'= 10.
		*/
		/*hour_line[77] = ' '; /*Replace that null that sprintf got us*/
		hour_line[80] = '\n';

		/* Write out line */
		if (fwrite(&hour_line, sizeof(hour_line), 1, COSMOS0fp) != 1)
		{
			logit("et", "COSMOS0PA_next: error writing COSMOS0 line. \n");
			return EW_FAILURE;
		}
	}
	/* After we process all of the data, we have to write:
			End-of-Data Flag Line:
		N+1 1 -11 End of data flag string, “End-of-data for..”.
		21-36 Station channel number and physical parameter of data (a checksum may optionally
		be included on the remainder of this line).  */
	sprintf(tempstr, "                                                                                ");/*clear it*/
	sprintf(tempstr, "End-of-data for %s.%s.%s.%s acceleration", getThis->sta, getThis->chan, getThis->net, getThis->loc );

	if (fwrite(tempstr, 81, 1, COSMOS0fp) != 1)
	{
		logit("et", "COSMOS0PA_next: error writing COSMOS0 dynamic header line. \n");
		return EW_FAILURE;
	}
	return EW_SUCCESS;
}



/************************************************************************
* This is the Put Away end event routine. It's called after we've       *
* finished processing one event                                         *
*                                                                       *
* For PC-COSMOS0 - close the COSMOS0 file, pointed to by COSMOS0fp      *
*               free COSMOS0Buffer memory to be nice to everyone else   *
*************************************************************************/
int COSMOS0PA_end_ev(int debug)
{
	/* Actually we opened multiple files, one for each SCNL, and they should 
	  all be closed by the time we get here; we don't know who they all are to 
	  close them here. */
/*	fclose(COSMOS0fp);

	if (debug == 1)
		logit("t", "Closing COSMOS0 file \n"); */

	return(EW_SUCCESS);
}

/************************************************************************
*       This is the Put Away close routine. It's called after when      *
*       we're being shut down.                                          *
*************************************************************************/
int COSMOS0PA_close(int debug)
{

	free((char *)COSMOS0Buffer);
	return(EW_SUCCESS);
}

/*
*
*  Byte swapping functions
*/

/* swap bytes if necessary, depending on what machine */
/* we have been compiled, and what machine the data   */
/* is being read from/being written to                */

static int StructMakeLocal(void *ptr, int struct_type, char data_type,
	int debug)
{
	if (ptr == NULL)
	{
		logit("e", "StructMakeLocal: NULL pointer passed in\n");
		return EW_FAILURE;
	}

#if defined (_INTEL)
	if (data_type != '6')
	{
		if (debug == 1)
			logit("et", "Hoping we don't really have to do any swapping from Intel to Sun \n");

	}

#elif defined (_SPARC)
	if (data_type == '6')
	{
		if (debug == 1)
			logit("et", "Hoping we don't really have to do any swapping from Sun to Intel because we've deleted from COSMOS0 the SwapDo function that suds used. \n");
	}

#endif
	return EW_SUCCESS;
}

long Median(int number_of_array_elements, long *Array)
{
	long *LocalArray;

	LocalArray = (long*)malloc(number_of_array_elements * sizeof(long));

	qsort(&LocalArray[0], number_of_array_elements, sizeof(long), longCompareForQsort);
	/* Get Median */
	return(LocalArray[(long)(number_of_array_elements / 2)]);
}
/*****************************************************************/
/* Just a simple compare function so that Q sort does it's thing */
/*****************************************************************/
int longCompareForQsort(const void *x, const void *y)
{
	/* compare must have const void ptr params for qsort to work */
	const long   *ix = (const long *)x;
	const long   *iy = (const long *)y;
	/* returns 1, -1 or 0 */
	return (*ix > *iy) - (*ix < *iy);
}
