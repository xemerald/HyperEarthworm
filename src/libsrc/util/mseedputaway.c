/******************************************************************
 * mseedputaway.c - Additional putaway functions to store miniSEED data from Earthworm
 *                  Uses Chad Trabant's libmseed functions downloaded from IRIS
 *                  All of the libmseed library has been added to libsrc/util
 *                  Richard Luckett BGS Sep 2006
 *
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

#include "earthworm.h"
#include "earthworm_defs.h"
#include "trace_buf.h"
#include "swap.h"
#include "ws_clientII.h"
#include "time_ew.h"

#include "libmseed/libmseed.h"

/* Private global variables */
static char mseed_filename [MAX_DIR_LEN];
static FILE *fp;
static int byte_order_flag;

/* Private forward declaration of tiny function to write a single record to file */
/* The address of this function is passed as an argument to mst_pack */
static void MSEEDPA_write_record (char *record, int reclen, void *ptr);


/**********************************************************************
 * MSEEDPA_init
 *
 * Description: Check that output directory given exists OK
 *              Set byte order flag depeding on architecture
 *
 * Input parameters: output_dir - the directory to be used for output files
 *                   debug      - flag for debugging output
 *
 * Output parameters: none
 * Returns: EW_SUCCESS or EW_FAILURE
 *
 ***********************************************************************/
int MSEEDPA_init (char *output_dir, char *OutputFormat, int debug)
{
    
  if (debug)
    logit ("e", "Entering MSEEDPA_init, output dir: %s\n", output_dir);

  /* make sure that the output directory exists can permissions are OK */
  if (CreateDir (output_dir) != EW_SUCCESS)
  {
    logit ("e", "MSEEDPA_init: Call to CreateDir failed\n");
    return EW_FAILURE;
  }

  /* byte_order_flag is input as an argument to mst_pack */
  /* 1 is big-endian, 0 little-endian */
  if (strcmp(OutputFormat, "intel")==0)
    byte_order_flag = 0;
  else if (strcmp(OutputFormat, "sparc")==0)
    byte_order_flag = 1;
  else
  {
    logit ("e", "MSEEDPA_init: can't recognise OutputFormat (%s)\n", OutputFormat);
    return EW_FAILURE;
  }
  return EW_SUCCESS;

}

/****************************************************************************
 * MSEEDPA_next_ev
 *
 * Description: Called when a snippet has been received, before it is processed
 *              Works out filename and opens file. 
 *
 * Input parameters: trace_req  - array of trace reqeust structures
 *                   n_reqs     - size of array
 *                   output_dir - output directory
 *                   e_date     - event date in the form 'ccyymmdd'
 *                   e_time     - event time in the form 'hhmmss.ff'
 *                   debug      - flag for debugging output
 *
 * Output parameters: none
 * Returns: EW_SUCCESS or EW_FAILURE
 *
 *
 ***********************************************************************/
int MSEEDPA_next_ev (char *output_dir,char *e_date, char *e_time, int debug)
{

  int event_year, event_month, event_day, event_hour, event_min;
  double event_sec;
  char string [100];

  if (debug)
    logit ("e", "Entering MSEEDPA_next_ev, date/time: %s %s\n", e_date, e_time);

  /* Parse and store the date/time */
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
  
  /* Filename created from date and time */
  sprintf (mseed_filename, "%s/%04d_%02d_%02d_%02d%02d_%02.0f.msd",
           output_dir, event_year, event_month, event_day,
           event_hour, event_min, event_sec);

  /* Open the file */
  if ( (fp = fopen(mseed_filename,"w+b")) == NULL)
  {
    logit ("e", "MSEEDPA_next_ev: unable to open output file %s\n",mseed_filename);
    return EW_FAILURE;
  }

  return EW_SUCCESS;
}

/*****************************************************************************
 * MSEEDPA_next
 *
 * Description: Called once for each channel requested.
 *              Selects the required samples from the given traces and calls mst_pack to pack them as miniSEED records.
 *              In turn mst_pack calls MSEEDPA_write_record to write the records to the file opened in MSEEDPA_next_ev.
 *
 * Input parameters: trace_req  - trace request structure returned from waveserverV including traces containing the data requested
 *                   gap_thresh - maximum size of gap in seconds before warning given
 *                   debug      - flag for debugging output
 *
 * Output parameters: none
 * Returns: EW_SUCCESS or EW_FAILURE
 *
 *****************************************************************************/
int MSEEDPA_next (TRACE_REQ *trace_req, double gap_thresh, int debug)
{

  MSTrace       *mst;                          /* structure from libmseed.h */
  TRACE2_HEADER *trace_head;                   /* structure from tracebuf.h */

  char           *msg_p;                       /* pointer into tracebuf data */
  
  float          *f_data;                      /* temporary pointer for casting data */
  int32_t        *l_data;                      /* temporary pointer for casting data */

  double         starttime, endtime;           /* times for current trace */
/*  double         temp_starttime, temp_endtime; */ /* in case first or final samp fall out of range  */

  int            sample_size;                  /* number of bytes per sample */
  int            first_samp, final_samp;       /* wanted sample positions within trace */
  int            first_trace, final_trace;     /* flags for special case traces */
  int64_t        psamples;                     /* output from pack routine - number of samples packed */
  int            precords;                     /* return from pack routine - number of records packed */
  int            i,j;

  if (debug)
    logit ("e","Entering MSEEDPA_next\n");

  /* pointer to first message */
  if ( (msg_p = trace_req->pBuf) == NULL)   
  {
    logit("et","MSEEDPA_next: message buffer is NULL\n");
    return EW_FAILURE;
  }
  
  /* Read header of first trace */
  trace_head = (TRACE2_HEADER *) msg_p;

  /* Swap byte order in header if need be */
  if (WaveMsg2MakeLocal(trace_head) < 0)
  {
    logit("et","MSEEDPA_next: unknown trace data type: %s for <%s.%s.%s.%s>\n",
                 trace_head->datatype, trace_head->sta, trace_head->chan, 
	         trace_head->net, trace_head->loc);
    return EW_FAILURE;
  }
  
  /* Make some checks */
  if (trace_head->samprate < 0.01)
  {
    logit("et","MSEEDPA_next: unreasonable samplerate (%f) for <%s.%s.%s.%s>\n",
                 trace_head->samprate, trace_head->sta, trace_head->chan, 
	         trace_head->net, trace_head->loc);
    return EW_FAILURE;
  }
  if (trace_head->nsamp < 1)
  {
    logit("et","MSEEDPA_next: no samples for <%s.%s.%s.%s>\n",
                 trace_head->sta, trace_head->chan, 
	         trace_head->net, trace_head->loc);
    return EW_FAILURE;
  }
  
  if (debug)
    logit("e","MSEEDPA_next: total trace length = %ld\n",trace_req->actLen);
    
  /* Initialise miniSEED trace structure */
  mst = mst_init (NULL);

  /* Populate MSTrace */
  strcpy (mst->network, trace_head->net);
  strcpy (mst->station, trace_head->sta);
  strcpy (mst->channel, trace_head->chan);
  strcpy (mst->location,trace_head->loc);
  mst->samprate = trace_head->samprate;
  
  
  /* Find type of words data stored as */
  if (strcmp(trace_head->datatype,"i4")==0 || strcmp(trace_head->datatype,"s4")==0){
    mst->sampletype = 'i';
    sample_size = sizeof(int32_t);
  }
  else if (strcmp(trace_head->datatype,"f4")==0 || strcmp(trace_head->datatype,"t4")==0){
    mst->sampletype = 'f';  
    sample_size = sizeof(float);
  }
  else
  {
    logit("et","MSEEDPA_next: unsupported datatype: %s\n", trace_head->datatype);
    return( EW_FAILURE );
  }
  
  /* Loop through all the traces in request for this chan */
  first_trace = 1;
  final_trace = 0;
  while (1) 
  { 
 
    /* Advance message pointer to the data */
    msg_p += sizeof(TRACE2_HEADER);

    if (trace_head->samprate != mst->samprate)
    {
      logit("et","MSEEDPA_next: changing samplerate (%lf) for <%s.%s.%s.%s>\n",
                 trace_head->samprate, trace_head->sta, trace_head->chan, 
	         trace_head->net, trace_head->loc);
      return( EW_FAILURE );
    }
  
    /* Probably only want part of first trace */
    if (first_trace)
    {
      if (trace_req->reqStarttime<trace_head->starttime)
      {
        logit("et","MSEEDPA_next: requested start time was earlier than trace start time. Using trace start time\n");
        first_samp = 0;
        mst->numsamples += trace_head->nsamp;
        mst->starttime = (hptime_t)(MS_EPOCH2HPTIME(trace_head->starttime));
      } else {
        first_samp = (int)(trace_head->samprate * (trace_req->reqStarttime - trace_head->starttime));
        mst->numsamples += trace_head->nsamp - first_samp;
        mst->starttime = (hptime_t)(MS_EPOCH2HPTIME(trace_req->reqStarttime));
      }
      first_trace = 0;
    }
    else
    {
      first_samp = 0;
      mst->numsamples += trace_head->nsamp;
      mst->starttime = (hptime_t)(MS_EPOCH2HPTIME(trace_head->starttime));
    }

    /* Probably only want part of final trace too - end check based on length of request buffer */
    if ((size_t)(msg_p + sample_size * trace_head->nsamp) >= ((size_t)trace_req->actLen + (size_t)trace_req->pBuf))
    {
      final_samp = (int) (trace_head->samprate * (trace_req->reqEndtime - trace_head->starttime) + 1);
      if (final_samp<0)
      {
        final_samp = (int) (trace_head->samprate * (trace_head->endtime - trace_head->starttime) + 1);
      } else if (final_samp>trace_head->nsamp) 
      {
        final_samp=trace_head->nsamp;
      }
      mst->numsamples -= (trace_head->nsamp - final_samp);
      final_trace = 1;
    }
    else
    {
      final_samp = trace_head->nsamp;
    }
 
    /* Not sure why need to set this but function from libmseed complains otherwise */
    mst->samplecnt = mst->numsamples;

    /* Allocate memory for data array in mst structure.  This is freed inside mst_pack */
    if ((mst->datasamples = malloc((final_samp-first_samp)*sample_size)) == NULL)
    {
      logit("et","MSEEDPA_next: trouble allocating memory for <%s.%s.%s.%s> wants %d\n",
                   trace_head->sta, trace_head->chan, 
	           trace_head->net, trace_head->loc,(final_samp-first_samp)*sample_size);
      return EW_FAILURE;
    }

    /* Fill data array for mst structure - need to switch on word type */
    switch( mst->sampletype )
    {
    case 'i':
      l_data = (int32_t *)msg_p;
      for (i=first_samp, j=0; i<final_samp; i++, j++)
        ((int32_t*)mst->datasamples)[j] = l_data[i];
      break;

    case 'f':
      f_data = (float *)msg_p;
      for (i=first_samp, j=0; i<final_samp; i++, j++)
        ((float*)mst->datasamples)[j] = f_data[i];
      break;
    }

    /* Pack 4096 byte records using Steim-2 compression */
    precords = mst_pack (mst, &MSEEDPA_write_record, NULL, 4096, DE_STEIM2, (flag)byte_order_flag, &psamples, 1, (flag)debug, NULL);
    if (precords == 0)
    {
      logit("et","MSEEDPA_next: mst_pack problem for <%s.%s.%s.%s>\n",
                   trace_head->sta, trace_head->chan, 
	           trace_head->net, trace_head->loc);
      return EW_FAILURE;
    }    
    if (debug)
      logit("e","MSEEDPA_next: Packed %ld samples into %d records\n", (long)psamples, precords);

    /* If this was the final trace then finish */
    if (final_trace)
      break;

    /* Remember end of this trace for gap check */
    endtime = trace_head->endtime;

    /* Advace msg_p to next TRACE_BUF and read in next header */
    msg_p += sample_size * trace_head->nsamp;
    trace_head = (TRACE2_HEADER *) msg_p;
    
    /* Swap byte order in header if need be */
    if (WaveMsg2MakeLocal(trace_head) < 0)
    {
      logit("et","MSEEDPA_next: unknown trace data type: %s for <%s.%s.%s.%s>\n",
                 trace_head->datatype, trace_head->sta, trace_head->chan, 
	         trace_head->net, trace_head->loc);
      return EW_FAILURE;
    }
  
    /* Check for gaps. */
    starttime = trace_head->starttime; 
    /* starttime is set for new packet; endtime is still set for old packet */
    if ( endtime + ( 1.0/trace_head->samprate ) * gap_thresh < starttime ) 
    {
      /* Warn that there's a gap */
      if (debug)
        logit("e","MSEEDPA_next: %s.%s.%s.%s: gap of %lf secs at %lf\n",
                 trace_head->sta, trace_head->chan, trace_head->net,
		 trace_head->loc, starttime - endtime, endtime);
    }
  } /* while(1) */

  mst_free (&mst);

  if (debug)
    logit ("e","MSEEDPA_next: Successful completion\n");

  return EW_SUCCESS;

}

/************************************************************************
 * MSEEDPA_end_ev
 *
 * Description: Called when an event has been completely processed
 *              Closes file opened in MSEEDPA_next_ev
 *
 * Input parameters: debug - flag for debugging output
 * Output parameters: none
 * Returns: EW_SUCCESS or EW_FAILURE
 *
 *
 *****************************************************************************/
int MSEEDPA_end_ev (int debug)

{
  /* check and close the file */
  if (fclose(fp))
  {
    logit ("e", "MSEEDPA_end_ev: error closing file\n");
    return EW_FAILURE;
  }
  if (debug)
    logit ("e", "MSEEDPA_end_ev: closed %s\n",mseed_filename);

  return EW_SUCCESS;
}

/************************************************************************
 * MSEEDPA_close
 *
 * Description: Called at program shutdown - does nothing at all
 *
 * Input parameters: debug - flag for debugging output
 * Output parameters: none
 * Returns: EW_SUCCESS or EW_FAILURE
 *
 *
 *****************************************************************************/
int MSEEDPA_close (int debug)
{

  if (debug)
    logit ("e", "Entering MSEEDPA_close: \n");

  return EW_SUCCESS;

}

#pragma warning(disable : 4100)   /* suppress unreferenced formal parameter warning */

/************************************************************************
 * MSEEDPA_write_record
 *
 * Description: Function called from mst_pack to write packed record to file.
 *              Address of this function is passed to mst-pack as an argument.
 *
 * Input parameters: record - array with packed record
 *                   reclen - length of record array
 *                   void pointer to handlerdata object (not used here)
 *
 * Output parameters: none
 *
 *****************************************************************************/
static void MSEEDPA_write_record (char *record, int reclen, void *hdptr)
{
  if ( fwrite(record, reclen, 1, fp) != 1 )
      logit ("e", "MSEEDPA_write_record: Error writing to %s\n",mseed_filename);
}


