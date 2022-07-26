/******************************************************************
 * wfdiscputaway.c - routines to enable wfdisc data to be stored from
 *                Earthworm
 *
 ******************************************************************/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <float.h>
#ifndef _WINNT
#include <unistd.h>
#endif
#include <fcntl.h>
#include <time.h>			/* time_t */

#include "earthworm.h"
#include "trace_buf.h"
#include "swap.h"
#include "ws_clientII.h"
#include "site.h"
#include "kom.h"
#include "time_ew.h"			/* gmtime_ew() */
#include "chron3.h"

#include "wfdischead.h"
#include "seihead.h"

/* private global variables */
int global_output_format;
char storage_dir[65];
//char w_dir[65]; //In case we want the w files to be in different locations from the wfdisc files
char eventDate[8];
char eventTime[6];
char wfdisc_name[256]; //Arbitrary, since 33 characters was too small.  I should define both 33 and 256 for use in snprintf the filenames
//wfdisc_params params_data[200]; //Size is arbitrary, what is max # of channels?  set as 200 here
int num_chan_data;
FP_PAIR fp_array[256];
int fp_array_size;
char dirpath[22]; //The path the wfdisc will be placed in
//
//Config struct
wfdisc_config config;

int ymd2doy(char* yyyymmdd);
FILE* open_wfdisc_file(char* station, char *channel, char* evtDate);
int initialize(char *config_filename, wfdisc_config *config_local); //Returns fail or success, if fail calling program should return fail as well

/**********************************************************************
 * WFDISCPA_init
 *
 * Description: initialise wfdisc put away routines
 *
 * Input parameters: output_dir - the directory to be used for wfdisc
 *                                output files
 *                   output_format - "intel" or "sparc" for byte ordering
 *                   debug - flag for debugging output
 * Output parameters: none
 * Returns: EW_SUCCESS or EW_FAILURE
 *
 * Comments:
 *
 ***********************************************************************/
int WFDISCPA_init (char *output_dir, char *output_format, int debug)

{
  
//  FILE* wfdisc_param;
//  FILE* wfdisc_loc_param;
//  int w_dir_len;
  char * EW_PARAMS;
  char wfdisc_params_full_path[256];

  if (debug)
  {
    logit ("e", "Entering WFDISCPA_init\n");
  }
  
  /* make sure that the output directory exists */
  if (CreateDir (output_dir) != EW_SUCCESS)
  {
    logit ("e", "WFDISCPA_init: Call to CreateDir failed\n");
    return EW_FAILURE;
  }

  //Make output dir the CWD
  if ( -1 == chdir_ew(output_dir) ) {
	logit("e", "WFDISCPA_init: Call to chdir to:%s failed\n", output_dir); 
	return EW_FAILURE;
  }
  
  //Copy dir name to global variable
  strncpy(storage_dir, output_dir, 65);

  //Call Initialize for file wfdisc_waveman.d

  //Read in wfdisc_w_loc.d file
//  if (debug) logit ("e", "Reading wfdisc_w_loc.d file for .w save location\n");
  EW_PARAMS = getenv("EW_PARAMS");
  snprintf(wfdisc_params_full_path, 256, "%s/%s", EW_PARAMS, "wfdisc_waveman.d");
//  wfdisc_loc_param = fopen(wfdisc_params_full_path, "r");
  if (initialize(wfdisc_params_full_path,&config) == EW_FAILURE) return EW_FAILURE ;
//  if (wfdisc_loc_param == NULL) {
//	logit ("e", "WFDISCPA_init: unable to open wfdisc_w_loc.d, assuming .w files stored in same loc as .wfdisc\n");
//	strncpy(w_dir, storage_dir, 65);
//  } else if ((w_dir_len = fread(w_dir, sizeof(char), 65, wfdisc_loc_param)) > 0) {
//	//Good, we have copied the data in
//	w_dir[w_dir_len-1] = '\0';
//	if (debug) logit ("e", "WFDISCPA_init: .w files will be saved to %s, filename length = %d\n", w_dir, w_dir_len);
//  } else {
//	snprintf(w_dir, 3, "./");
//  }
//  fclose(wfdisc_loc_param);

  //Get .w dir if it exists
  //If exists
  //Copy into w_dir
  //else
  //copy ./ into w_dir
//  if (CreateDir (w_dir) != EW_SUCCESS)
//  {
//    logit ("e", "WFDISCPA_init: Call to CreateDir failed\n");
//    return EW_FAILURE;
//  }

  //This can be relative since we are already in output dir
  //Make output dir the CWD
//  if ( -1 == chdir_ew(w_dir) ) {
//	logit("e", "WFDISCPA_init: Call to chdir to:%s failed\n", w_dir); 
//	return EW_FAILURE;
//  }
  //Make output dir the CWD
  if ( -1 == chdir_ew(output_dir) ) {
	logit("e", "WFDISCPA_init: Call to chdir to:%s failed\n", output_dir); 
	return EW_FAILURE;
  }

  if (! strcmp (output_format, "intel"))
    global_output_format = INTEL_OUTPUT;
  else if (! strcmp (output_format, "sparc"))
    global_output_format = SPARC_OUTPUT;
  else
  {
    logit ("e", "WFDISCPA_init: can't recognise OutputFormat (%s)\n", output_format);
    return EW_FAILURE;
  }

  //Read in wfdisc_params.d file
//  EW_PARAMS = getenv("EW_PARAMS");
//  snprintf(wfdisc_params_full_path, 256, "%s/%s", EW_PARAMS, "wfdisc_params.d");
//  wfdisc_param = fopen(wfdisc_params_full_path, "r");
//  num_chan_data = 0;
//  if (!wfdisc_param) {
    //Error, didn't open the params file
//	logit("e", "Error opening params file: %s\n", wfdisc_params_full_path);
//  } else {
//    while (!feof(wfdisc_param)) {
//      if(num_chan_data >= 200) {
//	    logit("e", "Too much channel data(%d lines) in %s\n", 200, wfdisc_params_full_path);
//	    break;
//	  }
//	  comment_char = fgetc(wfdisc_param);
//	  if(comment_char < '0' || (comment_char > '9' && comment_char < 'A') || (comment_char > 'Z' && comment_char < 'a') || comment_char > 'z') {
//	  	fgets(comment_line, sizeof comment_line, wfdisc_param);
//	  } else {
//	  	fseek( wfdisc_param, -1, SEEK_CUR);
//	  }
//      fscanf(wfdisc_param, "%7s%9s%d %f %f %7s%2s\n", params_data[num_chan_data].sta, params_data[num_chan_data].chan, 
//	                                                  &params_data[num_chan_data].chanid, &params_data[num_chan_data].calib, 
//		    										  &params_data[num_chan_data].calper, params_data[num_chan_data].instype, 
//		  											  params_data[num_chan_data].segtype);
	  //Create new wfdisc_param entry 
//	  num_chan_data++;
//    }
//  }

  //Set fp_array_size to 0 to start
  fp_array_size=0;

  return EW_SUCCESS;

}

/****************************************************************************
 * WFDISCPA_next_ev
 *
 * Description: called when a snippet has been received,
 *              before it is processed
 *
 * Input parameters: trace_req - array of trace request structures
 *                   n_reqs - size of array
 *                   output_dir - output directory
 *                   e_date, e_time - event time in the form
 *                                    'ccyymmdd' and 'hhmmss.ff'
 *                   debug - flag for debugging output
 * Output parameters: none
 * Returns: EW_SUCCESS or EW_FAILURE
 *
 * Comments:
 *
 ***********************************************************************/
int WFDISCPA_next_ev (TRACE_REQ *trace_req, int n_reqs, char *output_dir,
                   char *e_date, char *e_time, char *subnet, int debug)
{

  if (debug)
  {
    logit ("e", "Entering WFDISCPA_next_ev, date/time: %s %s\n", e_date, e_time);
  }

  strncpy(eventDate, e_date, 8);
  strncpy(eventTime, e_time, 6);

  //Create wfdisc file name, shall use subnet.date.etime.wfdisc
  //Requested by ISLA to change to yyyyddd_hh.wfdisc
  // IGD 02/10/2017 Requested by ISLA now to change to yyyymmdd_hh.wfdisc
  snprintf(wfdisc_name, 256, "%c%c%c%c%c%c%c%c_%c%c.wfdisc",  eventDate[0], eventDate[1], eventDate[2], eventDate[3], 
	                                                      eventDate[4], eventDate[5], eventDate[6], eventDate[7], 
	                                                      eventTime[0], eventTime[1]);
  if (debug)
  {
     logit ("e", "WFDISCPA_next_ev: wfdisc filename %s\n", wfdisc_name);
  }


  if (debug)
  {
    logit ("e", "WFDISCPA_next_ev data: n_reqs: %d, output_dir: %s, e_date: %s, e_time: %s, subnet: %s\n",n_reqs, output_dir,  e_date, e_time, subnet);
  }

  return EW_SUCCESS;

}

/*****************************************************************************
 * WFDISCPA_next
 *
 * Description: called once for each trace to be put away
 *
 * Input parameters: trace_req - the data to be stored
 *                   gap_thresh - size of any gap in seconds
 *                   debug - flag for debugging output
 * Output parameters: none
 * Returns: EW_SUCCESS or EW_FAILURE
 *
 * Comments: see pa_find_data()
 *
 *****************************************************************************/
int WFDISCPA_next (TRACE_REQ *trace_req, double gap_thresh, int debug)

{

  int status, loop_count, param_index, change_bit_order = (int)FALSE;
  int last_status = 0;
  int32_t prev_samples;
  double sample_time;
  char *data_ptr;
  struct Found_data data;
  char datatype[3];
  char lddate[19]; //Needs space for the null character from strftime, so makeing 19 bytes
  char dfile_name[34]; // 33 taken from WFDISC30 struct in gbase libraries (added one more for null char)
  FILE * w_fp;
  long foff = 0; //byte offset for .w file
  //Used for jdate //
  struct tm start_tm;
  struct Greg gregorian;
  time_t start_time;
  long jdate = -1;

  //Used for current date in lddate in wfdisc record
  struct tm *t;
  // int chars_copied;
  time_t current_time;

  //Used for getting param info
  int i;


  FILE* wfdisc_fp;

  if (debug) logit ("e", "Entering WFDISCPA_next\n");
  if (debug) logit ("e", "WFDISCPA_next data; gap_thresh: %lf\n", gap_thresh);
  if (debug) logit ("e", "WFDISCPA_next trace_req data; sta:%s, chan:%s, net:%s, loc:%s, pinno:%d, reqStarttime:%lf, reqEndtime:%lf, partial:%d, pBuf:0x%08lx, bufLen:%ld, timeout:%ld, fill:%ld, retFlag:%d, waitSec:%lf, actStarttime:%lf, actEndtime:%lf, actLen:%ld, samprate:%lf\n",
                                       trace_req->sta, trace_req->chan, trace_req->net, 
									   trace_req->loc, trace_req->pinno, 
									   trace_req->reqStarttime, trace_req->reqEndtime, 
									   trace_req->partial, (unsigned long)trace_req->pBuf, trace_req->bufLen,
									   trace_req->timeout, trace_req->fill, (int)trace_req->retFlag,
									   trace_req->waitSec, trace_req->actStarttime, 
									   trace_req->actEndtime, trace_req->actLen, trace_req->samprate);
  if (debug) logit ("e", "WFDISCPA_next data:0x%x \n", trace_req->pBuf[0]);

  prev_samples = 0;

  //Get current date
  ////Include <ctime.h>
  time(&current_time);
  t = localtime(&current_time);
  // chars_copied = (int)
     strftime(lddate, 19, " %Y%m%d %H:%M:%S", t);
  //printf ("Chars copied to string array: %d, array:%s\n", chars_copied, lddate);

  //See if there was param info
  param_index = -1; //If this is not changed, there was no param data for this station/channel
  for (i=0;i<config.n_calib_params; i++) {
    //Compare sta and chan
	if (!strcmp(config.calib_params[i].sta, trace_req->sta) && !strcmp(config.calib_params[i].chan, trace_req->chan) ) {
	  //Success
	  param_index = i;
      if (debug) logit ("e", "Found wfdisc param data for sta: %s chan:%s\n", trace_req->sta, trace_req->chan);
	}
  }

  data.n_samples = 1;

  /* IGD 02/10/2017: I am changing logic below at request of Milton G. so we do not have main subdir anymore
   * and filename is now yyyymmdd_hh.wfdisc, not yyyydoy_hh.w as before. notes below are updated
   */
  //Create all the dirs neccessary and logic for the content of the wfdisc and w files.
	//If station is listed as special: Use file specified
	//If all stations get the same file: Use file specified
	//Else if all stations get their own file: create yyyy/mm/station/yyyymmdd_hh.wfdisc
	//build dir path yyyy/mm/station and call RecursiveCreateDir()
  wfdisc_fp = open_wfdisc_file(trace_req->sta, trace_req->chan, eventDate);
//  snprintf(dirpath, 22, "%c%c%c%c/%c%c/%s", eventDate[0], eventDate[1], eventDate[2],
//                                                  eventDate[3], eventDate[4], eventDate[5], 
//												  trace_req->sta);
//	
//  if (RecursiveCreateDir (dirpath) != EW_SUCCESS)
//  {
//    logit ("e", "WFDISCPA_next: Call to CreateDir failed\n");
//    return EW_FAILURE;
//  }
//  //Change dir to dirpath and open wfdisc for writing
//  if ( -1 == chdir_ew(dirpath) ) {
//	logit("e", "WFDISCPA_next: Call to chdir to:%s failed\n", dirpath); 
//	return EW_FAILURE;
//  }
//  if (CreateDir (w_dir) != EW_SUCCESS)
//  {
//    logit ("e", "WFDISCPA_next: Call to CreateDir failed\n");
//    return EW_FAILURE;
//  }
//
//  wfdisc_fp = fopen(wfdisc_name, "a+");
//  if (!wfdisc_fp) { 
//    logit("e", "WFDISCPA_next: error opening file %s for writing\n", wfdisc_fp );
//	return EW_FAILURE;
//  }

	//and yyyy/mm/station/(.w configged file location)/yyyyddd_hh.w
	//For any of these, make container directories recursively
	
	
  //Create .w file
  snprintf(dfile_name, 33, "%s%s%s%s.%c%c%c%c%c%c%c%c_%c%c.w", trace_req->sta, 
                                       trace_req->chan, trace_req->net, trace_req->loc, 
									   eventDate[0], eventDate[1], eventDate[2],  eventDate[3],
			  						   eventDate[4], eventDate[5], eventDate[6],  eventDate[7], 
									   eventTime[0], eventTime[1]);

  //Chdir to the storage_dir before and after to make sure I can use relative dirs
  if ( -1 == chdir_ew(storage_dir) ) {
  }
  if ( -1 == chdir_ew(dirpath) ) {
	logit("e", "WFDISCPA_next: Call to chdir to:%s failed\n", dirpath); 
	return EW_FAILURE;
  }
  if ( -1 == chdir_ew(config.w_dir) ) {
	logit("e", "WFDISCPA_next: Call to chdir to:%s failed\n", config.w_dir); 
	return EW_FAILURE;
  }
  w_fp = fopen(dfile_name, "w+");
  if (!w_fp) {logit("e", "WFDISCPA_next: error opening file %s for writing\n", dfile_name) ; }
  if ( -1 == chdir_ew(storage_dir) ) {
	logit("e", "WFDISCPA_next: Call to chdir to:%s failed\n", storage_dir); 
	return EW_FAILURE;
  }
  
  sample_time = trace_req->reqStarttime;
  
  //////////////////////////
  //For loop which goes till all data for this channel is taken care of
  //////////////////////////
  
  for (sample_time = trace_req->reqStarttime, loop_count = 0;
    (sample_time < trace_req->reqEndtime) && (data.n_samples > 0);
    sample_time += ((double) data.n_samples / data.sample_rate), loop_count ++)
  {

    status = pa_find_data (trace_req, sample_time, &data);
    switch (status)
    {
      case FD_FOUND_REQUESTED:
	    break;
      case FD_FOUND_GAP:
		last_status = status;
		prev_samples = 0;
        break;
      case FD_NO_MORE_DATA:
	    last_status = status;
		prev_samples = 0;
       /* if no data has been found on previous calls, then exit
        *        * so that no data will be recorded for this channel */
        if (sample_time == trace_req->reqStarttime) return EW_SUCCESS;
        /* otherwise calculate the size of the gap to the end of requested data */
        data.n_samples = (int32_t) (data.sample_rate * (trace_req->reqEndtime - sample_time));
        break;
      case FD_BAD_DATATYPE:
        logit("e", "WFDISCPA_next: unrecognised data type code, skipping this scn: %s.%s.%s.%s\n",
             trace_req->sta, trace_req->chan, trace_req->net, trace_req->loc);
        return EW_FAILURE;
      case FD_CHANGED_SRATE:
        logit("e", "WFDISCPA_next: bad sample rate, skipping this scn: %s.%s.%s.%s\n",
             trace_req->sta, trace_req->chan, trace_req->net, trace_req->loc);
        return EW_FAILURE;
    }

    if (debug) logit ("e", "Loop %d: %s %ld @ %.1lfHz, time %.3lf\n",
           loop_count, status == FD_FOUND_REQUESTED ? "samples" : "gap",
           (long)data.n_samples, data.sample_rate, sample_time);
    if (debug && status == FD_FOUND_REQUESTED) logit ("e", "num_samples %ld @ %.1lfHz, start_time %.3lf end_time %.3lf\n",
           (long)data.n_samples, data.sample_rate, sample_time, sample_time + ((double) data.n_samples / data.sample_rate));

    data_ptr = data.data;
    if (status == FD_FOUND_REQUESTED) {
	  switch(data.trace_hdr->datatype [0])
	  {
	  	case 's':
		  if (debug) logit("e", "incoming byte order is sparc\n");
		  if (global_output_format == SPARC_OUTPUT) {
		  	change_bit_order = FALSE;
		  } else if (global_output_format == INTEL_OUTPUT) {
		  	change_bit_order = TRUE;
		  }
		  break;
		case 'i':
		  if (debug) logit("e", "incoming byte order is intel\n");
		  if (global_output_format == SPARC_OUTPUT) {
		    change_bit_order = TRUE;
		  } else if (global_output_format == INTEL_OUTPUT) {
		    change_bit_order = FALSE;
		  }
		  break;
		default:
		  logit ("e", "WFDISCPA_next: can't handle that datatype byte order: %d\n", (int)data.trace_hdr->datatype [0]);
		  return EW_FAILURE;
	  }
      switch(data.data_type_code)
      {
        case FD_SHORT_INT:
		  if (global_output_format == SPARC_OUTPUT) {
		    snprintf(datatype, 3, "s2");
			if (debug) logit("e", "outgoing datatype is short int sparc\n");
		  } else if (global_output_format == INTEL_OUTPUT) {
	        snprintf(datatype, 3, "i2");
		    if (debug) logit("e", "outgoing datatype is short int intel\n");
		  } else {
		    logit ("e", "WFDISCPA_next: can't handle that datatype code: %d-%d\n", global_output_format, data.data_type_code);
		    return EW_FAILURE;
		  }
          break;
	    case FD_LONG_INT:
		  if (global_output_format == SPARC_OUTPUT) {
		    snprintf(datatype, 3, "s4");
			if (debug) logit("e", "outgoing datatype is long int sparc\n");
	      } else if (global_output_format == INTEL_OUTPUT) {
		    snprintf(datatype, 3, "i4");
		    if (debug) logit("e", "outgoing datatype is long int intel\n");
		  } else {
		    logit ("e", "WFDISCPA_next: can't handle that datatype code: %d-%d\n", global_output_format, data.data_type_code);
			return EW_FAILURE;
		  }
		  break;
	    default:
	      //Error, do I handle that datatype?
	      logit ("e", "WFDISCPA_next: can't handle that datatype code: %d\n", data.data_type_code);
		  return EW_FAILURE;
      }

          //This was needed to keep it writing, bugfix. 
          wfdisc_fp = open_wfdisc_file(trace_req->sta, trace_req->chan, eventDate);
	  if(last_status == FD_FOUND_REQUESTED) {
            if (debug) logit("e", "WFDISC_next: Update existing wfdisc record\n");
	  	//Need to update the last record
		//Seek back to the endtime entry and write new endtime and numsamples
		fseek(wfdisc_fp, -223, SEEK_CUR);
		fprintf(wfdisc_fp, "%16.6lf%9ld", sample_time + ((double) (data.n_samples - 1) / data.sample_rate), (long)(data.n_samples + prev_samples));
		//Seek to end of file
		fseek(wfdisc_fp, 0, SEEK_END);
		prev_samples += data.n_samples;
	  } else {
	    //This is the start of a new wfdisc record
            if (debug) logit("e", "WFDISC_next: Start of a new wfdisc record\n");
		//
		//Convert sample_time to julian date
		//First convert epoch to tm struct
                start_time = (time_t) sample_time;
		gmtime_ew(&start_time, &start_tm); 
		//Then convert tm struct to gregorian date
                tm_to_gregorian(&start_tm, &gregorian);
		//Then convert gregorian to julian
		jdate = julian(&gregorian);
		//
		//If no param data found, use defaults
		if (param_index == -1) {
          fprintf(wfdisc_fp, "%-7s%-9s%16.6lf%9d%9d%9ld %16.6lf%9ld%12.6f %16.10f %16.6f %-7s%-2s%-3s%-2s%-65s%-33s %9ld%9d%-18s\n",
	             trace_req->sta, trace_req->chan, sample_time, /* wfid */ -1, /* chanid */ -1,
		         /* jdate */ jdate, /* endtime */ sample_time + ((double) (data.n_samples - 1) / data.sample_rate),
		  	     /* nsamp */ (long)data.n_samples, /* samprate */ data.sample_rate, /* calib */ (double)0,
			     /* calper */ (double)-1, /* instype */ "-", /* segtype */ "-", /* datatype */ datatype, /* clip */ "-",
			     /* dir */ config.w_dir, /* dfile */ dfile_name, /* foff */ foff, /* commid */ -1, /* lddate */ lddate);
	    } else {
          fprintf(wfdisc_fp, "%-7s%-9s%16.6lf%9d%9ld%9ld %16.6lf%9ld%12.6f %16.10f %16.6f %-7s%-2s%-3s%-2s%-65s%-33s %9ld%9d%-18s\n",
	             trace_req->sta, trace_req->chan, sample_time, /* wfid */ -1, /* chanid */ (long)config.calib_params[param_index].chanid,
		         /* jdate */ jdate, /* endtime */ sample_time + ((double) (data.n_samples - 1) / data.sample_rate),
		  	     /* nsamp */ (long)data.n_samples, /* samprate */ data.sample_rate, /* calib */ config.calib_params[param_index].calib,
			     /* calper */ config.calib_params[param_index].calper, /* instype */ config.calib_params[param_index].instype, 
				 /* segtype */ config.calib_params[param_index].segtype, /* datatype */ datatype, /* clip */ "-",
			     /* dir */ config.w_dir, /* dfile */ dfile_name, /* foff */ foff, /* commid */ -1, /* lddate */ lddate);
		}
	    prev_samples += data.n_samples;
	  }
      //If FD_FOUND_REQUESTED Open new .w file

      // logit("e", "sample_time += %lf\n", ((double) data.n_samples / data.sample_rate));
      //logit("e", "num samples: %d\n", data.n_samples);
      switch (data.data_type_code)
      {
        case FD_SHORT_INT:
		  if (change_bit_order == FALSE) {
		    if (debug) logit("e", "Writing data out direct from buffer, no byte order change\n");
            foff += (long)(sizeof (short) * fwrite(data_ptr, (sizeof (short)), data.n_samples, w_fp));
		  } else if (change_bit_order == TRUE) {
		    if (debug) logit("e", "Changing byte order of data for output\n");
		    //REORDER shorts
			for (i=0; i<data.n_samples; i++) {
			  foff += (long)(sizeof (char) * fwrite(&data_ptr[i*2+1], (sizeof (char)), 1, w_fp));
			  foff += (long)(sizeof (char) * fwrite(&data_ptr[i*2+0], (sizeof (char)), 1, w_fp));
			}
		  }
          break;
	    case FD_LONG_INT:
		  if (change_bit_order == FALSE) {
		    if (debug) logit("e", "Writing data out direct from buffer, no byte order change\n");
		    foff += (long)(sizeof (int32_t) * fwrite(data_ptr, (sizeof (int32_t)), data.n_samples, w_fp));
		  } else if (change_bit_order == TRUE) {
		    if (debug) logit("e", "Changing byte order of data for output\n");
		    //REORDER longs
			for (i=0; i<data.n_samples; i++) {
			  foff += (long)(sizeof (char) * fwrite(&data_ptr[i*4+3], (sizeof (char)), 1, w_fp));
			  foff += (long)(sizeof (char) * fwrite(&data_ptr[i*4+2], (sizeof (char)), 1, w_fp));
			  foff += (long)(sizeof (char) * fwrite(&data_ptr[i*4+1], (sizeof (char)), 1, w_fp));
			  foff += (long)(sizeof (char) * fwrite(&data_ptr[i*4+0], (sizeof (char)), 1, w_fp));
			}
		  }
	      break;
      }
	  last_status = status;
    }
  }
  
  fclose(w_fp);
  
  if (debug)
  {
    logit ("e", "Successful completion of WFDISCPA_next\n");
  }
  return EW_SUCCESS;

}


/************************************************************************
 * WFDISCPA_end_ev
 *
 * Description: called when an event has been completely processed
 *
 * Input parameters: debug - flag for debugging output
 * Output parameters: none
 * Returns: EW_SUCCESS or EW_FAILURE
 *
 * Comments:
 *
 *****************************************************************************/
int WFDISCPA_end_ev (int debug)

{
  int i=0;
//  int empty = 0;
  if (debug)
  {
    logit ("e", "Entering WFDISCPA_end_ev\n");
  }
  for (i=0; i<fp_array_size; i++)
  { 
    fclose(fp_array[i].wfdisc_fp);
  }

  return EW_SUCCESS;

}

/************************************************************************
 * WFDISCPA_close
 *
 * Description: called at program shutdown
 *
 * Input parameters: debug - flag for debugging output
 * Output parameters: none
 * Returns: EW_SUCCESS or EW_FAILURE
 *
 * Comments:
 *
 *****************************************************************************/
int WFDISCPA_close (int debug)
{

  if (debug)
  {
    logit ("e", "Entering WFDISCPA_close\n");
  }

  return EW_SUCCESS;

}


/************************
 * ymd2doy
 *
 * Description: called to convert the char string YYYYMMDD to the day of the year integer
 *
 * Input char* containing YYYYMMDD
 * Returns integer for the day of the year
 *
 * ****************************/
int ymd2doy(char* yyyymmdd) 
{
  int leapyear = 0;
  int year = 0;
  int month = 0;
  int day = 0;
  int doy = 0;
  char ascii_year[5] = "";
  char ascii_month[3] = "";
  char ascii_day[3] = "";
  strncpy(ascii_year, yyyymmdd, 4);
  strncpy(ascii_month, &yyyymmdd[4], 2);
  strncpy(ascii_day, &yyyymmdd[6], 2);

  year = atoi((char*)ascii_year);
  if (((year % 4) == 0 && (year % 100) != 0) || (year % 400) == 0) { leapyear = 1; }
  month = atoi((char*)ascii_month);
  day = atoi((char*)ascii_day);

  switch ( month )
  { 
    case 1:
	  doy = 0 + day;
      break;
	case 2:
	  doy = 31 + day;
	  break;
	case 3:
	  doy = 59 + leapyear + day;
	  break;
	case 4:
	  doy = 90 + leapyear + day;
	  break;
	case 5:
	  doy = 120 + leapyear + day;
	  break;
	case 6:
	  doy = 151 + leapyear + day;
	  break;
	case 7:
	  doy = 181 + leapyear + day;
	  break;
	case 8:
	  doy = 212 + leapyear + day;
	  break;
	case 9:
	  doy = 243 + leapyear + day;
	  break;
	case 10:
	  doy = 273 + leapyear + day;
	  break;
	case 11:
	  doy = 304 + leapyear + day;
	  break;
	case 12:
	  doy = 334 + leapyear + day;
	  break;
  }
  return doy;
}
	

FILE* open_wfdisc_file(char* station, char* channel, char* evtDate) 
{
  char *output_name;
  int i=0;
  if ( -1 == chdir_ew(storage_dir) ) {
   logit("e", "WFDISCPA_next: Call to chdir to:%s failed\n", storage_dir); 
   return EW_FAILURE;
  }
  output_name = station;
  //If station exists in config.spec_array then use config.spec_array[i]->outname instead of station
  for(i=0; i<config.n_spec_array; i++)
  {
    if (strncmp("*", config.spec_array[i].sta,7) == 0)
	{
	  output_name = config.spec_array[i].outname;
	  break;
	}
    if ((strncmp(station, config.spec_array[i].sta, 7) == 0) && 
	    ((strncmp(channel, config.spec_array[i].chan, 9) == 0) || 
		 (strncmp("*", config.spec_array[i].chan, 9) == 0 )))
	{
	  output_name = config.spec_array[i].outname;
	  break;
	}
  }
  //If station exists in wfdisc_fp_array then return it,
  //else open file and add to the array and return that fp
  //
  //If there are no entries in wfdisc_fp_array then open
  //new file add it and return it
  if (fp_array_size == 0) 
  {
    snprintf(dirpath, 22, "%c%c%c%c/%c%c/%s/", evtDate[0], evtDate[1], evtDate[2],
                                                    evtDate[3], evtDate[4], evtDate[5], 
                                                    output_name);
	
    if (RecursiveCreateDir (dirpath) != EW_SUCCESS)
    {
      logit ("e", "WFDISCPA_next: Call to CreateDir failed\n");
      return EW_FAILURE;
    }
    //Change dir to dirpath and open wfdisc for writing
    if ( -1 == chdir_ew(dirpath) ) {
	  logit("e", "WFDISCPA_next: Call to chdir to:%s failed\n", dirpath); 
	  return EW_FAILURE;
    }
    if (CreateDir (config.w_dir) != EW_SUCCESS)
    {
      logit ("e", "WFDISCPA_next: Call to CreateDir failed\n");
      return EW_FAILURE;
    }

    fp_array[0].wfdisc_fp = fopen(wfdisc_name, "w+");
    if (!fp_array[0].wfdisc_fp) { 
      logit("e", "WFDISCPA_next: error opening wfdisc file %s for writing\n", wfdisc_name );
	  return EW_FAILURE;
    }
	//Add station to array
	strncpy(fp_array[0].sta, output_name, 7);
	fp_array_size=1;
	return fp_array[0].wfdisc_fp;
    //Create dir structure and open file
	//add fp to array
	//return fp
  } else {
    for(i=0; i<fp_array_size; i++) 
    {
      //If station == fp_array[i]->station
      if (strncmp(output_name, fp_array[i].sta, 7) == 0) 
      {
        return fp_array[i].wfdisc_fp;
      }
      //return fp_array[i]->wfdisc_fp;
    }

    snprintf(dirpath, 22, "%c%c%c%c/%c%c/%s/", evtDate[0], evtDate[1], evtDate[2],
                                                    evtDate[3], evtDate[4], evtDate[5], 
                                                    output_name);

	
    if (RecursiveCreateDir (dirpath) != EW_SUCCESS)
    {
      logit ("e", "WFDISCPA_next: Call to CreateDir failed\n");
      return EW_FAILURE;
    }
    //Change dir to dirpath and open wfdisc for writing
    if ( -1 == chdir_ew(dirpath) ) {
	  logit("e", "WFDISCPA_next: Call to chdir to:%s failed\n", dirpath); 
	  return EW_FAILURE;
    }
    if (CreateDir (config.w_dir) != EW_SUCCESS)
    {
      logit ("e", "WFDISCPA_next: Call to CreateDir failed\n");
      return EW_FAILURE;
    }

    fp_array[fp_array_size].wfdisc_fp = fopen(wfdisc_name, "w+");
    if (!fp_array[fp_array_size].wfdisc_fp) { 
      logit("e", "WFDISCPA_next: error opening wfdisc file %s for writing\n", wfdisc_name);
	  return EW_FAILURE;
    }

	//Add station to array
    strncpy(fp_array[fp_array_size].sta, output_name, 7);
    //printf("fp_array_size: %d, sta: %s, \n", fp_array_size, fp_array[fp_array_size].sta);
    fp_array_size++;
    return fp_array[fp_array_size].wfdisc_fp;
  }
}


int initialize(char *config_filename, wfdisc_config *config_local)
{
  int n_files, success, found;
  char *command, *ptr;
  wfdisc_params *calib_params_ptr;
  wfdisc_spec *wfdisc_spec_ptr;
  char *sta=NULL, *chan=NULL, *instype=NULL, *segtype=NULL, *outname=NULL,
       *calib_str=NULL, *calper_str=NULL;
  int32_t chanid = 0;
  float calib, calper;

  //Initialize my config
  strcpy (config_local->w_dir, "");
  config_local->n_calib_params = 0;
  config_local->calib_params = (wfdisc_params *) 0;
  config_local->n_spec_array = 0;
  config_local->spec_array = (wfdisc_spec *) 0;
  config_local->allinone = 0;

  n_files = k_open(config_filename);
  if(n_files == 0) 
  {
    logit("e", "WFDISCPA_init: Initialization failed to open config file %s\n", config_filename);
    return EW_SUCCESS;
  }
  /* while a configuration file is open ... */
  while (n_files > 0)
  {
    /* for each line in the file ... */
    while (k_rd ())
    {
      /* Get the first token from line, remove blank lines and comments */
      command = (char *)k_str();
      if (! command) continue;
      if (*command == '#') continue;

      /* process the command */
      if (*command == '@')
      {
        /* Open a nested configuration file */
        success = n_files+1;
        n_files = k_open (command +1);
        if (n_files != success)
        {
          logit("e", "WFDISCPA_init: Initialization Error opening command file <%s>\n",
                   command +1);
          return EW_FAILURE;
        }
      }
	  else if (k_its("w_location"))
	  {
	    ptr = (char *)k_str();
		if (ptr) strcpy (config_local->w_dir, ptr);
	  }
	  //else if (k_its("wfdisc_separate"))
	  //{
	  //  config_local->allinone = 1;
	  //}
	  else if (k_its("calib"))
	  {
	    //Import to the calib_params array
		//
		found = 1;
		if ((sta = (char *)k_str()) == NULL) found = 0;
		else if ((chan = (char *)k_str()) == NULL) found = 0;
		else if ((chanid = k_int()) == 0) found = 0;
		else if ((calib_str = (char *)k_str()) == NULL) found = 0;
		else if ((calper_str = (char *)k_str()) == NULL) found = 0;
		else if ((instype = (char *)k_str()) == NULL) found = 0;
		else if ((segtype = (char *)k_str()) == NULL) found = 0;
        calib = (float)atof(calib_str);
        calper = (float)atof(calper_str);
        if (!found) 
		{
		  logit("e", "WFDISCPA_init Reading calib parameters failed\n");
		  return EW_FAILURE;
		}
		//realloc memory
		//Need to define this ptr
		calib_params_ptr = realloc(config_local->calib_params, 
		                           sizeof(wfdisc_params) * (config_local->n_calib_params + 1));
		if (!calib_params_ptr) 
		{
		  logit("e", "WFDISCPA_init: Not enough memory available to store all calib parameters\n");
		  return EW_FAILURE;
		}
		config_local->calib_params = calib_params_ptr;
		calib_params_ptr += config_local->n_calib_params;
		config_local->n_calib_params++;

		strcpy(calib_params_ptr->sta, sta);
		strcpy(calib_params_ptr->chan, chan);
		calib_params_ptr->chanid = chanid;
		calib_params_ptr->calib = calib;
		calib_params_ptr->calper = calper;
		strcpy(calib_params_ptr->instype, instype);
		strcpy(calib_params_ptr->segtype, segtype);
	  }
	  else if (k_its("wfdisc_spec"))
	  {
	    //Import to the spec_array
		found = 1;
		if ((outname = (char *)k_str()) == NULL) found = 0;
		else if ((sta = (char *)k_str()) == NULL) found = 0;
		else if ((chan = (char *)k_str()) == NULL) found = 0;
		if (!found)
		{
		  logit("e", "WFDSICPA_init: Reading wfdisc_spec parameters failed\n");
		  return EW_FAILURE;
		}
		wfdisc_spec_ptr = realloc(config_local->spec_array, 
		                          sizeof(wfdisc_spec) * (config_local->n_spec_array + 1));
		if (!wfdisc_spec_ptr) 
		{
		  logit("e", "WFDISCPA_init: Not enough memory available to store all spec parameters\n");
		  return EW_FAILURE;
		}
		config_local->spec_array = wfdisc_spec_ptr;
		wfdisc_spec_ptr +=config_local->n_spec_array;
		config_local->n_spec_array ++;
        
		strncpy(wfdisc_spec_ptr->outname, outname, 20);
        strncpy(wfdisc_spec_ptr->sta, sta, 7);
		strncpy(wfdisc_spec_ptr->chan, chan, 9);
	  }
	}
    n_files=k_close();
  }
  return EW_SUCCESS;
}
