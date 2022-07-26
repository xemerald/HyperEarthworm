/******************************************************************
 * gseputaway.c - routines to enable GSE data to be stored from
 *                Earthworm
 *
 * S. Flower, Feb 2001
 *
 * Modified 11 Oct 2001 - instead of writing direct to the GSE
 * file we write to a temporary file and rename it when it is
 * complete - this ensures that the file can be read OK as soon
 * as it is available.
 *
 * Modified 11 Oct 2001 - removed the check that all requested
 * channels had been written - GSE can cope if there are missing
 * channels and the check was deleting files if all channels were
 * not present.
 *
 * Modified 26 Oct 2001 - fixed bug that puts gaps at wrong end
 * of output traces
 *
 * Modified 31st Jan 2002 - new routines to make dealing with
 * earthworm data structures easier to follow (pa_find_data())
 ******************************************************************/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#if defined (_WINNT)
#include <process.h>
#include <io.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include "earthworm.h"
#include "earthworm_defs.h"
#include "trace_buf.h"
#include "swap.h"
#include "ws_clientII.h"
#include "site.h"
#include "time_ew.h"

#include "gsehead.h"
#include "seihead.h"


/* the value used to represent missing data in GSE */
#define GSE_MISSING_DATA_FLAG		0

/* private global variables */
static char *line_terminator;
static char *file_open_mode;
static char gse_filename [MAX_DIR_LEN];
static char tmp_filename [MAX_DIR_LEN];
static FILE *gse_fp;
static int new_data_channel;
static int32_t channel_checksum = 0l;
static int data_line_length;
static int n_channels, channels_written;

/* private forward declarations */
static int open_gse_file (char *filename, char *msg_id, char *sta_code);
static int write_gse_channel_header (int start_year, int start_month, int start_day,
                                     int start_hour, int start_min, double start_sec,
                                     char *chan_name, char *chan_type, char *aux_id,
                                     int32_t n_samps,
                                     double frequency, double calib_value,
                                     double calib_period, char *instrum_name,
                                     double horiz_angle, double vert_angle);
static int write_gse_channel_data (int n_samps, int32_t *data);
static int write_gse_channel_trailer (void);
static int close_gse_file (void);



/**********************************************************************
 * GSEPA_init
 *
 * Description: initialise GSE put away routines
 *
 * Input parameters: output_dir - the directory to be used for GSE
 *                                output files
 *                   output_format - "intel" or "sparc" for line termination
 *                   debug - flag for debugging output
 * Output parameters: none
 * Returns: EW_SUCCESS or EW_FAILURE
 *
 * Comments:
 *
 ***********************************************************************/
int GSEPA_init (char *output_dir, char *output_format, int debug)

{
    
  if (debug)
  {
    logit ("e", "Entering GSEPA_init, output dir: %s\n", output_dir);
  }

  /* make sure that the output directory exists */
  if (CreateDir (output_dir) != EW_SUCCESS)
  {
    logit ("e", "GSEPA_init: Call to CreateDir failed\n");
    return EW_FAILURE;
  }

#if defined (_INTEL)
  if (! strcmp (output_format, "intel"))
  {
    line_terminator = "\n";
    file_open_mode = "w";
  }
  else if (! strcmp (output_format, "sparc"))
  {
    line_terminator = "\n";
    file_open_mode = "wb";
  }
  else
  {
    logit ("e", "GSEPA_init: can't recognise OutputFormat (%s)\n", output_format);
    return EW_FAILURE;
  }
#elif defined (_SPARC)
  if (! strcmp (output_format, "sparc"))
  {
    line_terminator = "\n";
    file_open_mode = "w";
  }
  else if (! strcmp (output_format, "intel"))
  {
    line_terminator = "\r\n";
    file_open_mode = "wb";
  }
  else
  {
    logit ("e", "SEIPA_init: can't recognise OutputFormat (%s)\n", output_format);
    return EW_FAILURE;
  }
#else
#error "_INTEL or _SPARC must be set before compiling"
#endif

  return EW_SUCCESS;

}

/****************************************************************************
 * GSEPA_next_ev
 *
 * Description: called when a snippet has been received,
 *              before it is processed
 *
 * Input parameters: trace_req - array of trace reqeust structures
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
int GSEPA_next_ev (TRACE_REQ *trace_req, int n_reqs, char *output_dir,
                   char *e_date, char *e_time, int debug)
{

  int event_year, event_month, event_day, event_hour, event_min;
  double event_sec;
  char string [100];

  static int tmp_file_count = 0;


  if (debug)
  {
    logit ("e", "Entering GSEPA_next_ev, date/time: %s %s\n", e_date, e_time);
  }

  /* parse and store the date/time - we assume that it doesn't need to
   * be checked for validity - I don't know if that is correct */
  strcpy (string, e_date);
  event_day = atoi (&string[6]);
  string[6] = '\0';
  event_month = atoi (&string[4]);
  string[4] = '\0';
  event_year = atoi (string);
  strcpy (string, e_time);
  event_sec = atof (&string[4]);
  string[4] = '\0';
  event_min = atoi (&string[2]);
  string[2] = '\0';
  event_hour = atoi (string);
  
  /* use the temporary file as the output file - we will rename it to the proper GSE file name
   * when it is complete and closed */
  sprintf (tmp_filename, "%s%c%d_%d_gse.tmp",
           output_dir, DIR_DELIM, getpid (), tmp_file_count ++);
  sprintf (gse_filename, "%s%c%04d_%02d_%02d_%02d_%02d_%02.0f.gse",
           output_dir, DIR_DELIM, event_year, event_month, event_day,
           event_hour, event_min, event_sec);

  /* open the GSE file */
  if (! open_gse_file (tmp_filename, "EW", trace_req->net))
  {
    logit ("e", "GSEPA_init: unable to open GSE output file\n");
    return EW_FAILURE;
  }

  /* store tracking information */
  n_channels = n_reqs;
  channels_written = 0;

  return EW_SUCCESS;

}

/*****************************************************************************
 * GSEPA_next
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
int GSEPA_next (TRACE_REQ *trace_req, double gap_thresh, int debug)

{

  int status, loop_count;
  short short_val;
  int32_t value, count;
  float float_val;
  double sample_time, double_val, data_end_time;
  char *data_ptr;
  struct Found_data data;
  time_t ltime;
  struct tm unix_time;


  if (debug) logit ("e", "Entering GSEPA_next\n");

  /* loop over the requested times for the data - the test for n_samples==0
   * is an emergency condition that could happen with very bad floating point
   * rounding errors (very unlikely though) */
  data.n_samples = 1;
  for (sample_time = trace_req->reqStarttime, loop_count = 0;
       (sample_time < trace_req->reqEndtime) && (data.n_samples > 0);
	   sample_time += ((double) data.n_samples / data.sample_rate), loop_count ++)
  {
    /* get the data for this time */
    status = pa_find_data (trace_req, sample_time, &data);
    switch (status)
	{
    case FD_FOUND_REQUESTED:
    case FD_FOUND_GAP:
	  break;
    case FD_NO_MORE_DATA:
      /* if no data has been found on previous calls, then exit
	   * so that no data will be recorded for this channel */
	  if (sample_time == trace_req->reqStarttime) return EW_SUCCESS;

	  /* otherwise calculate the size of the gap to the end of requested data */
	  data.n_samples = (int32_t) (data.sample_rate * (trace_req->reqEndtime - sample_time));
	  break;

    case FD_BAD_DATATYPE:
      logit("e", "GSEPA_next: unrecognised data type code, skipping this scn: %s.%s.%s.%s\n",
            trace_req->sta, trace_req->chan, trace_req->net, trace_req->loc);
      return EW_FAILURE;

    case FD_CHANGED_SRATE:
      logit("e", "GSEPA_next: bad sample rate, skipping this scn: %s.%s.%s.%s\n",
            trace_req->sta, trace_req->chan, trace_req->net, trace_req->loc);
      return EW_FAILURE;
	}

    /* adjust the number of samples to ensure it doesn't go beyond the end time */
	data_end_time = sample_time + ((double) data.n_samples / data.sample_rate);
	if (data_end_time > trace_req->reqEndtime)
	  data.n_samples = (int32_t) (data.sample_rate * (trace_req->reqEndtime - sample_time));
    if (debug) logit ("e", "Loop %d: %s %ld @ %.1lfHz, time %.3lf\n",
                      loop_count, status == FD_FOUND_REQUESTED ? "samples" : "gap",
				(long)data.n_samples, data.sample_rate, sample_time);

    /* on the first time write the gse channel header */
    if (sample_time == trace_req->reqStarttime)
	{
      ltime = (time_t) sample_time;
      gmtime_ew (&ltime, &unix_time);
      if (! write_gse_channel_header (unix_time.tm_year + 1900, unix_time.tm_mon +1,
                                      unix_time.tm_mday, unix_time.tm_hour, unix_time.tm_min,
                                      ((double) unix_time.tm_sec) + (sample_time - (double) ltime),
                                      trace_req->sta, trace_req->chan, "-",
                                      (int32_t) (data.sample_rate * (trace_req->reqEndtime - trace_req->reqStarttime)),
								      data.sample_rate, -1.0, -1.0, "-", -1.0, -1.0))
	  {
        logit ("e", "GSEPA_next: error starting new GSE channel\n");
        return EW_FAILURE;
	  }
	}

	/* write the data */
	value = GSE_MISSING_DATA_FLAG;
	data_ptr = data.data;
	for (count=0; count<data.n_samples; count++)
	{
	  if (status == FD_FOUND_REQUESTED)
	  {
		switch (data.data_type_code)
		{
		case FD_SHORT_INT:
  		  short_val = *((short *) data_ptr);
          value = (int32_t) short_val;
  		  data_ptr += sizeof (short);
		  break;
		case FD_LONG_INT:
  		  value = *((int32_t *) data_ptr);
  		  data_ptr += sizeof (int32_t);
		  break;
		case FD_FLOAT:
  		  float_val = *((float *) data_ptr);
          value = (int32_t) float_val;
  		  data_ptr += sizeof (float);
		  break;
		case FD_DOUBLE:
  		  double_val = *((double *) data_ptr);
          value = (int32_t) double_val;
  		  data_ptr += sizeof (double);
		  break;
		}
	  }
      write_gse_channel_data (1, &value);
	}
  }

  /* stop writing on this channel and fill in any gap at the end of the data */
  write_gse_channel_trailer ();

  if (debug)
  {
    logit ("e", "Successful completion of GSEPA_next\n");
  }
  return EW_SUCCESS;

}

/************************************************************************
 * GSEPA_end_ev
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
int GSEPA_end_ev (int debug)

{

  int ret_val;


  ret_val = EW_SUCCESS;

  /* check and close the GSE file */
  if (! close_gse_file ())
  {
    logit ("e", "GSEPA_end_ev: error writing to GSE file\n");
    ret_val = EW_FAILURE;
  }

  /* rename the temporary file to the proper GSE file name - the return value from
   * rename_ew() doesn't appear reliable, so check for the existence of the renamed
   * file after the rename operation */
  if (ret_val == EW_SUCCESS)
  {
    rename_ew (tmp_filename, gse_filename);
    if (access (gse_filename, 0))
    {
      logit ("e", "GSEPA_end_ev: unable to rename file (%s) to (%s)\n", tmp_filename, gse_filename);
      ret_val = EW_FAILURE;
    }
  }

  /* if there was an error, delete the file */
  if (ret_val != EW_SUCCESS)
  {
    logit ("e", "GSEPA_end_ev: output file is being deleted (%s)\n", tmp_filename);
    remove (tmp_filename);
  }

  return ret_val;

}

/************************************************************************
 * GSEPA_close
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
int GSEPA_close (int debug)
{

  if (debug)
  {
    logit ("e", "Entering GSEPA_close: \n");
  }

  return EW_SUCCESS;

}


/************************************************************************
 ************************************************************************
 * All code below this point is private to this file
 ************************************************************************
 ************************************************************************/

/************************************************************************
 * open_gse_file
 *
 * Description: open a GSE file and write the main header for the file
 *
 * Input parameters: filename - the file to write
 *                   msg_id - the message id
 *                   sta_code - the station code
 * Output parameters: none
 * Returns: TRUE if file written OK, FALSE otherwise
 *
 * Comments:
 *
 ************************************************************************/
static int open_gse_file (char *filename, char *msg_id, char *sta_code)

{

  /* attempt to open the file */
  gse_fp = fopen (filename, file_open_mode);
  if (! gse_fp) return FALSE;

  /* write the GSE header */
  fprintf (gse_fp, "BEGIN GSE2.0%s", line_terminator);
  fprintf (gse_fp, "MSG_TYPE DATA%s", line_terminator);
  fprintf (gse_fp, "MSG_ID %s %s%s", msg_id, sta_code, line_terminator);
  fprintf (gse_fp, "DATA_TYPE WAVEFORM GSE2.0%s", line_terminator);

  /* check the file */
  if (ferror (gse_fp)) return FALSE;
  return TRUE;

}

/************************************************************************
 * write_gse_channel_header
 *
 * Description: write a GSE channel header
 *
 * Input parameters: start_year, month, day - the date of the data
 *                   start_hour, min, sec - the time of the data
 *                   chan_name, type - the channel name and type
 *                                     (e.g. EKR1, SZ)
 *                   aux_id - auxiliary identification code
 *                   n_samps - number of samples in the following data block
 *                   frequency - the recording frequency (hz)
 *                   calib_value, calib_period - calibration details
 *                   instrum_name - GSE code for the sensor type
 *                   horiz_angle, vert_angle - sensor orientation
 * Output parameters: none
 * Returns: TRUE if file written OK, FALSE otherwise
 *
 * Comments:
 *
 ************************************************************************/
static int write_gse_channel_header (int start_year, int start_month, int start_day,
                                     int start_hour, int start_min, double start_sec,
                                     char *chan_name, char *chan_type, char *aux_id,
                                     int32_t n_samps,
                                     double frequency, double calib_value,
                                     double calib_period, char *instrum_name,
                                     double horiz_angle, double vert_angle)

{
    
  /* write the header for this channel */
  fprintf (gse_fp, "WID2 %04d/%02d/%02d %02d:%02d:%06.3lf %-5.5s %-3.3s "
    	       "%-4.4s INT %8ld %11.6lf %10.2le %7.3lf %-6.6s %5.1lf %4.1lf%s",
    	   start_year, start_month, start_day, start_hour, start_min, start_sec,
    	   chan_name, chan_type, aux_id, (long)n_samps, frequency, calib_value,
           calib_period, instrum_name, horiz_angle, vert_angle, line_terminator);
  fprintf (gse_fp, "DAT2%s", line_terminator);

  /* flag write_gse_channel_data to expect a new channel */
  new_data_channel = TRUE;
    
  /* check the file */
  if (ferror (gse_fp)) return FALSE;
  return TRUE;

}

/************************************************************************
 * write_gse_channel_data
 *
 * Description: write channel data in GSE format
 *
 * Input parameters: n_samps - the number of samples to write
 *                   data - the data to write
 * Output parameters: none
 * Returns: TRUE if file written OK, FALSE otherwise
 *
 * Comments: This routine may be called more than once if all the data
 *           is not available at one go - the total number of samples
 *           written must equal to number of samples declared in the header
 *
 ************************************************************************/
static int write_gse_channel_data (int n_samps, int32_t *data)

{

  int count, length;
  int32_t value;
  char string [100];

  static int32_t modulo = 100000000;


  /* if this is the start of a new channel, reset the sample
   * counter and the checksum */
  if (new_data_channel)
  {
    data_line_length = 0;
    new_data_channel = FALSE;
    channel_checksum = 0l;
  }

  /* for each data sample ... */
  for (count=0; count<n_samps; count++)
  {
    /* format the sample, checking for missing data */
    value = *(data + count);
    sprintf (string, "%ld", (long)value);

    /* write the sample using an 80 char. line length */
    length = (int)strlen (string);
    if ((length + data_line_length +1) > 80)
    {
      fprintf (gse_fp, "%s", line_terminator);
      data_line_length = 0;
    }
    if (data_line_length > 0)
    {
      fprintf (gse_fp, " ");
      data_line_length ++;
    }
    fprintf (gse_fp, "%s", string);
    data_line_length += length;
          
    /* accumulate the checksum */
    if (labs (value) >= modulo)
      value -= (value / modulo) * modulo;
    channel_checksum += value;
    if (labs (channel_checksum) >= modulo)
      channel_checksum -= (channel_checksum / modulo) * modulo;
  }

  /* check the file */
  if (ferror (gse_fp)) return FALSE;
  return TRUE;

}

/************************************************************************
 * write_gse_channel_trailer
 *
 * Description: write trailer info. for a channel
 *
 * Input parameters: none
 * Output parameters: none
 * Returns: TRUE if file written OK, FALSE otherwise
 *
 * Comments:
 *
 ************************************************************************/
static int write_gse_channel_trailer (void)

{

  /* write the checksum for this channel */
  if (data_line_length > 0)
  {
    fprintf (gse_fp, "%s", line_terminator);
    data_line_length = 0;
  }
  fprintf (gse_fp, "CHK2 %d%s", abs (channel_checksum), line_terminator);

  /* check the file */
  if (ferror (gse_fp)) return FALSE;
  return TRUE;

}

/************************************************************************
 * close_gse_file
 *
 * Description: write trailer info. for a file and close it
 *
 * Input parameters: none
 * Output parameters: none
 * Returns: TRUE if file written OK, FALSE otherwise
 *
 * Comments:
 *
 ************************************************************************/
static int close_gse_file (void)

{

  int ret_val;


  /* write the GSE trailer */
  fprintf (gse_fp, "STOP%s", line_terminator);
  
  /* check the file */
  if (ferror (gse_fp)) ret_val = FALSE;
  else ret_val = TRUE;

  /* close it and return */
  fclose (gse_fp);
  return ret_val;

}



