/******************************************************************
 * seiputaway.c - routines to enable Seisan data to be stored from
 *                Earthworm
 *
 * S. Flower, Feb 2001
 *
 * Modified 11 Oct 2001 - instead of writing direct to the output
 * file we write to a temporary file and rename it when it is
 * complete - this ensures that the file can be read OK as soon
 * as it is available
 *
 * Modified 26 Oct 2001 - fixed bug that puts gaps at wrong end
 * of output traces
 *
 * Modified 26 Oct 2001 - seisan file writing routines moved to a
 * sparate file because of complexity of doing seisan writes
 * with the Earthworm putaway structure
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
#include <float.h>
#if defined (_WINNT)
#include <io.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include "earthworm.h"
#include "trace_buf.h"
#include "swap.h"
#include "ws_clientII.h"
#include "site.h"
#include "time_ew.h"

#include "seihead.h"


/* private global variables */
static int output_native_numbers;


/**********************************************************************
 * SEIPA_init
 *
 * Description: initialise SEIsan put away routines
 *
 * Input parameters: output_dir - the directory to be used for Seisan
 *                                output files
 *                   output_format - "intel" or "sparc" for byte ordering
 *                   debug - flag for debugging output
 * Output parameters: none
 * Returns: EW_SUCCESS or EW_FAILURE
 *
 * Comments:
 *
 ***********************************************************************/
int SEIPA_init (char *output_dir, char *output_format, int debug)

{
    
  /* make sure that the output directory exists */
  if (CreateDir (output_dir) != EW_SUCCESS)
  {
    logit ("e", "SEIPA_init: Call to CreateDir failed\n");
    return EW_FAILURE;
  }

#if defined (_INTEL)
  if (! strcmp (output_format, "intel"))
    output_native_numbers = TRUE;
  else if (! strcmp (output_format, "sparc"))
    output_native_numbers = FALSE;
  else
  {
    logit ("e", "SEIPA_init: can't recognise OutputFormat (%s)\n", output_format);
    return EW_FAILURE;
  }
#elif defined (_SPARC)
  if (! strcmp (output_format, "sparc"))
    output_native_numbers = TRUE;
  else if (! strcmp (output_format, "intel"))
    output_native_numbers = FALSE;
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
 * SEIPA_next_ev
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
int SEIPA_next_ev (TRACE_REQ *trace_req, int n_reqs, char *output_dir,
                   char *e_date, char *e_time, char *subnet, int debug)
{

  int count;
  double min_start_time, max_end_time;
  TRACE_REQ *trace_req_ptr;


  if (debug)
  {
    logit ("e", "Entering SEIPA_next_ev, date/time: %s %s\n", e_date, e_time);
  }

  /* calculate overall duration of all traces */
  min_start_time = DBL_MAX;
  max_end_time = 0.0;
  if (n_reqs == 0) {
	  /* avoiding bad dereferencing in our future if we have no requests */
	  min_start_time = 0;
  }
  for (count=0; count<n_reqs; count++)
  {
    trace_req_ptr = trace_req + count;
    if (trace_req_ptr->reqStarttime < min_start_time)
      min_start_time = trace_req_ptr->reqStarttime;
    if (trace_req_ptr->reqEndtime > max_end_time)
      max_end_time = trace_req_ptr->reqEndtime;
  }
  if (debug)
  {
    logit ("e", "Request details: %lf-%lf %s\n", min_start_time, max_end_time, trace_req->net);
  }
  
  /* open the SEIsan file */
  /* changed by CJB 3/20/2002 to use subnet name if one exists */
  if (subnet[0] != '\0')
  {
	if (! open_seisan_file (output_dir, subnet, min_start_time,
		                      max_end_time - min_start_time, output_native_numbers))
	{
		logit ("e", "SEIPA_init: unable to open Seisan output file\n");
		return EW_FAILURE;
	}
  }
  else
  {
	if (! open_seisan_file (output_dir, trace_req->net, min_start_time,
		                      max_end_time - min_start_time, output_native_numbers))
	{
		logit ("e", "SEIPA_init: unable to open Seisan output file\n");
		return EW_FAILURE;
	}
  }

  /* write the seisan event file headers */
  for (count=0; count<n_reqs; count++)
  {
    trace_req_ptr = trace_req + count;
    if (! add_seisan_channel (trace_req_ptr->sta, trace_req_ptr->chan, trace_req_ptr->net, trace_req_ptr->loc))
    {
      logit ("e", "SEIPA_init: error writing to Seisan output file\n");
      return EW_FAILURE;
    }
    if (debug)
    {
      logit ("e", "Channel %d/%d, %s %s %lf %lf\n",
             count, n_reqs, trace_req->sta,
             trace_req_ptr->chan, min_start_time,
             trace_req_ptr->reqStarttime);
    }
  }

  return EW_SUCCESS;

}

/*****************************************************************************
 * SEIPA_next
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
int SEIPA_next (TRACE_REQ *trace_req, double gap_thresh, int debug)

{

  int status, loop_count;
  short short_val;
  int32_t value, count;
  float float_val;
  double sample_time, double_val, data_end_time;
  char *data_ptr;
  struct Found_data data;


  if (debug) logit ("e", "Entering SEIPA_next\n");

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
      logit("e", "SEIPA_next: unrecognised data type code, skipping this scnl: %s.%s.%s.%s\n",
            trace_req->sta, trace_req->chan, trace_req->net, trace_req->loc);
      return EW_FAILURE;

    case FD_CHANGED_SRATE:
      logit("e", "SEIPA_next: bad sample rate, skipping this scnl: %s.%s.%s.%s\n",
            trace_req->sta, trace_req->chan, trace_req->net, trace_req->loc);
      return EW_FAILURE;
	}

    /* adjust the number of samples to ensure it doesn't go beyond the end time */
	data_end_time = sample_time + ((double) data.n_samples / data.sample_rate);
	if (data_end_time > trace_req->reqEndtime)
	  data.n_samples = (int32_t) (data.sample_rate * (trace_req->reqEndtime - sample_time));
    if (debug) logit ("e", "Loop %d: %s %d @ %.1lfHz, time %.3lf\n",
                      loop_count, status == FD_FOUND_REQUESTED ? "samples" : "gap",
					  data.n_samples, data.sample_rate, sample_time);

    /* on the first time write the seisan channel header */
    if (sample_time == trace_req->reqStarttime)
	{
      if (! start_seisan_channel (trace_req->sta, trace_req->chan,
                                  sample_time, data.sample_rate,
                                  (int32_t) (data.sample_rate * (trace_req->reqEndtime - trace_req->reqStarttime))))
	  {
        logit ("e", "SEIPA_next: error starting new Seisan channel\n");
        return EW_FAILURE;
	  }
	}

	/* write the data */
	value = SEISAN_MISSING_DATA_FLAG;
	data_ptr = data.data;
	for (count=0; count<data.n_samples; count++)
	{
	  if (status == FD_FOUND_REQUESTED)
	  {
		switch (data.data_type_code)
		{
		/* write all values into int32s regardless of input type */
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
      add_seisan_channel_data (1, &value);
	}
  }

  /* stop writing on this channel and fill in any gap at the end of the data */
  end_seisan_channel ();

  if (debug)
  {
    logit ("e", "Successful completion of SEIPA_next\n");
  }
  return EW_SUCCESS;

}


/************************************************************************
 * SEIPA_end_ev
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
int SEIPA_end_ev (int debug)

{

  int ret_val;


  if (debug)
  {
    logit ("e", "Entering SEIPA_end_ev\n");
  }
  ret_val = EW_SUCCESS;

  /* check and close the Seisan file */
  if (! close_seisan_file ())
  {
    logit ("e", "SEIPA_end_ev: error writing to Seisan file\n");
    ret_val = EW_FAILURE;
  }

  return ret_val;

}

/************************************************************************
 * SEIPA_close
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
int SEIPA_close (int debug)
{

  if (debug)
  {
    logit ("e", "Entering SEIPA_close\n");
  }

  return EW_SUCCESS;

}




