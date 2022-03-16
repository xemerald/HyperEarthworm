/***************************************************************************
 * seiutils.c - routines for writing seisan waveform files
 *
 * NB - the general parts of this file are also used by the GSE
 *      put away routines
 *
 * Seisan file format synopsis:
 *
 * Record | Size | Description
 * -------+------+--------------------------------------------------
 * 1      | 80   | Network name, number of stations, event time
 * 2      | 80   | Not used
 * 3      | 80   | 1st, 2nd, 3rd channel details
 * 4-N    | 80   | Other channel details, 3 per record - there is
 *        |      | a minimum of 12 channel headers, so N has
 *        |      | a minimum value of 6 (but no maximum value)
 * N+1    | 1040 | Channel 1 header - time and response data
 * N+2    | X    | Channel 1 data - entire data is written
 *        |      | as a single record
 * ...    |      | Channel 2,3,... header/data follow
 *
 * Sizes are in bytes. Each record is written with its length as
 * a 4 byte integer immediately before and after the data:
 *   <length (4 bytes)> <record (x bytes)> <length (4 bytes)>
 *
 * To work correctly, you must call the routines in this file in the
 * following order:
 *
 *  open_seisan_file()
 *  foreach channel
 *      add_seisan_channel()
 *  foreach channel
 *      start_seisan_channel()
 *      add_seisan_channel_data()
 *      end_seisan_channel()
 *  close_seisan_file()
 *
 * S. Flower, Oct 2001
 *
 ***************************************************************************/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <float.h>

/* operating system specific includes */
#if defined (_WINNT)
#include <direct.h>
#include <io.h>
#elif defined  (_SOLARIS) || defined(_LINUX) || defined(_MACOSX)
#include <unistd.h>
#else
#error "_WINNT or _SOLARIS or _LINUX must be set before compiling"
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
static char output_dir [MAX_DIR_LEN];   /* directory to use for output file */
static char tmp_dirname [MAX_DIR_LEN];  /* name of the temporary directory for this request */
static char network_name [50];      /* name of the network for this request */
static double req_start_time;   /* start time of requested data */
static double duration;         /* duration of requested data */
static int n_channels;          /* number of channels in this request */
static struct Sei_channel_details *channels;        /* channel details for this request */
static struct Sei_channel_details *current_channel; /* channel currently being processed */
static FILE *channel_fp;        /* currently open data file */
static int output_native_numbers;   /* if TRUE write numbers without byte swapping */
static int tmp_dir_count = 0;       /* count of temporary directories */

/* private forward declarations */
static void write_seisan (FILE *fp, int32_t length, void *data);


/***************************************************************************
 * open_seisan_file
 *
 * Description: open a Seisan file and write the start of the header
 *
 * Input parameters: od - the directory for seisan files
 *                   nn - the network name
 *                   st - the start time for the data
 *                   dur - length of data in seconds
 *                   onn - if TRUE write numbers without byte swapping
 * Output parameters: channel details
 * Returns: TRUE on success, FALSE on error
 *
 * Comments:
 *
 ***************************************************************************/
int open_seisan_file (char *od, char *nn, double st, double dur, int onn)

{

  /* create a temporary directory for the parts of the seisan file */
  sprintf (tmp_dirname, "%s%c%d_%d_sei.tmp",
           od, DIR_DELIM, getpid (), tmp_dir_count ++);
  if (CreateDir (tmp_dirname) != EW_SUCCESS) return FALSE;

  /* reset global variables */
  strcpy (output_dir, od);
  strcpy (network_name, nn);
  req_start_time = st;
  duration = dur;
  n_channels = 0;
  channels = current_channel = 0;
  output_native_numbers = onn;

  return TRUE;

}

/***************************************************************************
 * add_seisan_channel
 *
 * Description: add a channel to the current seisan file
 *
 * Input parameters: chan_name, type, network, location - the channel name and type, network and location
 *                                     (e.g. EKR1, SZ, NN, LL)
 * Output parameters: none
 * Returns: TRUE if channel added OK, FALSE otherwise
 *
 * Comments: all channels must be added using this routine before any
 * channel data is written - channels that are not added will be
 * ignored
 *
 ***************************************************************************/
int add_seisan_channel (char *chan_name, char *chan_type, char *network, char *location)

{

  struct Sei_channel_details *channel_ptr;


  /* allocate memory for this channel */
  channel_ptr = realloc (channels, sizeof (struct Sei_channel_details) * (n_channels +1));
  if (! channel_ptr) return FALSE;
  channels = channel_ptr;
  n_channels ++;

  /* fill out the new channel details - fill these in so that if a channel is
   * completely missing it will appear as a 1hz trace of the same duration as
   * the complete request */
  channel_ptr = channels + n_channels -1;
  strcpy (channel_ptr->chan_name, chan_name);
  strcpy (channel_ptr->chan_type, chan_type);
  strcpy (channel_ptr->network, network);
  strcpy (channel_ptr->location, location);
  if (strlen(channel_ptr->network) == 1) {
     /* pad the network code with a space */
     channel_ptr->network[1] = ' ';
     channel_ptr->network[2] = 0;
  }
  channel_ptr->start_time = req_start_time;
  channel_ptr->n_samples = (int32_t) duration;
  channel_ptr->n_written = 0;
  channel_ptr->sample_rate = 1.0;
  channel_ptr->channel_count = n_channels -1;
  strcpy (channel_ptr->filename, "");

  return TRUE;

}

/***************************************************************************
 * start_seisan_channel
 *
 * Description: start writing data for the next channel
 *
 * Input parameters: chan_name, type - the channel name and type
 *                                     (e.g. EKR1, SZ)
 *                   start_time - start time of data for this channel
 *                   sample_rate - the sample rate for the data
 *                   n_samples - the number of samples
 * Output parameters: none
 * Returns: TRUE if write succeeded, FALSE otherwise
 *
 * Comments: call this before add_seisan_channel_data() to start writing
 *           the next channel
 *
 ***************************************************************************/
int start_seisan_channel (char *chan_name, char *chan_type,
                          double start_time, double sample_rate,
                          int32_t n_samples)

{

  int count;
  int32_t value;
  struct Sei_channel_details *channel_ptr;


  /* find the correpsonding channel details - if they don't exist
   * then return without error - this will cause the channel to
   * be ignored */
  current_channel = 0;
  for (count=0, channel_ptr=channels; count<n_channels; count++, channel_ptr++)
  {
    if ((! strcmp (channel_ptr->chan_name, chan_name)) &&
        (! strcmp (channel_ptr->chan_type, chan_type)))
    {
      current_channel = channel_ptr;
      break;
    }
  }
  if (! current_channel) return TRUE;

  /* fill in the channel details */
  current_channel->start_time = start_time;
  current_channel->sample_rate = sample_rate;
  current_channel->n_samples = n_samples;
  sprintf (current_channel->filename, "%s%c%d.tmp", tmp_dirname, DIR_DELIM, current_channel->channel_count);

  /* open a file for this channel */
  channel_fp = fopen (current_channel->filename, "wb");
  if (! channel_fp)
  {
    strcpy (channel_ptr->filename, "");
    return FALSE;
  }

  /* write the length of the data at the start of the block */
  value = (int32_t)(current_channel->n_samples * sizeof (int32_t));
  if (! output_native_numbers) SwapInt32 (&value);
  fwrite (&value, sizeof (int32_t), 1, channel_fp);

  return TRUE;

}

/***************************************************************************
 * add_seisan_channel_data
 *
 * Input parameters: data_len - the number of samples to add
 *                   data - the data as an array
 * Output parameters: none
 * Returns: none
 *
 * Comments:
 *
 ***************************************************************************/
void add_seisan_channel_data (int32_t data_len, int32_t *data)

{

  int count;
  int32_t value;


  /* output the data */
  for (count=0; count<data_len; count++)
  {
    if (current_channel->n_written >= current_channel->n_samples) break;

    value = *(data + count);
    if (! output_native_numbers) SwapInt32 (&value);
    fwrite (&value, 4, 1, channel_fp);

    current_channel->n_written ++;
  }

}

/***************************************************************************
 * end_seisan_channel
 *
 * Description: call this to complete writing data to a channel
 *
 * Input parameters: none
 * Output parameters: none
 * Returns: TRUE if channel completed OK, FALSE otherwise
 *
 * Comments:
 *
 ***************************************************************************/
int end_seisan_channel (void)

{

  int ret_val;
  int32_t value;


  /* add any gap data to the end of the file */
  value = SEISAN_MISSING_DATA_FLAG;
  if (! output_native_numbers) SwapInt32 (&value);
  while (current_channel->n_written < current_channel->n_samples)
  {
    fwrite (&value, 4, 1, channel_fp);
    current_channel->n_written ++;
  }

  /* write the length of the data at the end of the block */
  value = (int32_t)(current_channel->n_samples * sizeof (int32_t));
  if (! output_native_numbers) SwapInt32 (&value);
  fwrite (&value, sizeof (int32_t), 1, channel_fp);

  /* check the file, close it and return */
  if (ferror (channel_fp)) ret_val = FALSE;
  else ret_val = TRUE;
  fclose (channel_fp);
  return ret_val;

}

/***************************************************************************
 * close_seisan_file
 *
 * Description: check and close a Seisan event file
 *
 * Input parameters: none
 * Output parameters: none
 * Returns: TRUE/FALSE
 *
 * Comments:
 *
 ***************************************************************************/
int close_seisan_file (void)

{

  int channel_count, ret_val, length, end, channel_pos, century_code, count;
  int32_t value;
  char padded_chan_name [50], padded_network_name [50], sei_filename [MAX_DIR_LEN];
  struct Sei_channel_details *channel_ptr;
  FILE *sei_fp, *tmp_fp;
  time_t ltime;
  struct tm conv_time;

  static char string [1500];


  ret_val = TRUE;

  /* work out the seisan style network name */
  length = (int)strlen (network_name);
  for (count=0; count<5; count++)
  {
    padded_network_name [count] = '_';
    if (count < length)
    {
      if (isprint (*(network_name + count)))
        padded_network_name [count] = *(network_name + count);
    }
  }
  padded_network_name [5] = '\0';

  /* open the main seisan file */
  ltime = (time_t) req_start_time;
  gmtime_ew (&ltime, &conv_time);
  sprintf (sei_filename, "%s%c%04d-%02d-%02d-%02d%02d-%02dS.%s_%03d",
           output_dir, DIR_DELIM, conv_time.tm_year + 1900, conv_time.tm_mon +1,
           conv_time.tm_mday, conv_time.tm_hour, conv_time.tm_min,
           conv_time.tm_sec, padded_network_name, n_channels);
  sei_fp = fopen (sei_filename, "wb");
  if (! sei_fp) return FALSE;

  /* write the first two lines to the event file */
  ltime = (time_t) req_start_time;
  gmtime_ew (&ltime, &conv_time);
  sprintf (string, " %-29.29s%3d%03d %03d %02d %02d %02d %02d %6.3lf %9.3f           ",
           network_name, n_channels,
           conv_time.tm_year, conv_time.tm_yday +1, conv_time.tm_mon +1,
           conv_time.tm_mday, conv_time.tm_hour, conv_time.tm_min,
           (double) conv_time.tm_sec + (double) ltime - req_start_time, duration);
  write_seisan (sei_fp, 80, string);
  memset (string, ' ', 80);
  write_seisan (sei_fp, 80, string);

  /* write the station details, minimum 12 records, 3 records per line */
  if (n_channels > 30) end = n_channels;
  else end = 30;
  if (end % 3) end += 3 - (end % 3);
  for (channel_count=0, channel_ptr = channels; channel_count<end;
       channel_count++, channel_ptr ++)
  {
    channel_pos = channel_count %3;
    if (channel_pos == 0) memset (string, ' ', 80);

    if (channel_count < n_channels)
    {
      strcpy (padded_chan_name, channel_ptr->chan_name);
      while (strlen (padded_chan_name) < 5) strcat (padded_chan_name, " ");
      sprintf (&string[channel_pos * 26], " %-4.4s%-2.2s%2.2s%c%7.2f %8.2f",
               padded_chan_name, channel_ptr->chan_type, channel_ptr->chan_type+2,
               *(padded_chan_name + 4),
               channel_ptr->start_time - req_start_time, channel_ptr->n_samples / channel_ptr->sample_rate);
      string [(channel_pos +1) * 26] = ' ';
    }

    if (channel_pos == 2) write_seisan (sei_fp, 80, string);
  }

  /* for each channel ... */
  for (channel_count=0, channel_ptr = channels; channel_count<n_channels;
       channel_count++, channel_ptr ++)
  {
    /* write the channel header information */
    ltime = (time_t) channel_ptr->start_time;
    gmtime_ew (&ltime, &conv_time);
    century_code = conv_time.tm_year / 100;
    sprintf (string, "%-5.5s%-2.2s%1.1s%c%d%02d%c%03d%1.1s%02d%c%02d %02d %02d %6.3lf %7.2lf %6ld                          4",
             channel_ptr->chan_name, channel_ptr->chan_type, channel_ptr->location, *(channel_ptr->chan_type+2),
             century_code, conv_time.tm_year % 100, *(channel_ptr->location+1),
             conv_time.tm_yday +1, channel_ptr->network, conv_time.tm_mon +1, *(channel_ptr->network+1),
             conv_time.tm_mday, conv_time.tm_hour, conv_time.tm_min,
             ((double) conv_time.tm_sec) + (channel_ptr->start_time - (double) ltime),
             channel_ptr->sample_rate, (long)channel_ptr->n_samples);
    length = (int)strlen (string);
    memset (&string[length], ' ', 1040 - length);
    write_seisan (sei_fp, 1040, string);

    /* was the channel written OK by an earlier call to
     * add_seisan_channel_data() ?? */
    if (channel_ptr->n_written == channel_ptr->n_samples)
    {
      /* yes - copy the data from the temporary file */
      tmp_fp = fopen (channel_ptr->filename, "rb");
      if (! tmp_fp) ret_val = FALSE;
      else
      {
        while ((length = (int)fread (string, 1, sizeof (string), tmp_fp)) > 0)
          fwrite (string, 1, length, sei_fp);
        if (ferror (tmp_fp)) ret_val = FALSE;
        fclose (tmp_fp);
      }
    }
    else
    {
      /* no - invent missing data for the channel - we create a 1Hz data set
       * containing missing data, starting at the event start time */
      value = (int32_t)(channel_ptr->n_samples * sizeof (int32_t));
      if (! output_native_numbers) SwapInt32 (&value);
      fwrite (&value, 4, 1, sei_fp);
      value = SEISAN_MISSING_DATA_FLAG;
      if (! output_native_numbers) SwapInt32 (&value);
      for (count=0; count<channel_ptr->n_samples; count++)
        fwrite (&value, 4, 1, sei_fp);
      value = (int32_t)(channel_ptr->n_samples * sizeof (int32_t));
      if (! output_native_numbers) SwapInt32 (&value);
      fwrite (&value, 4, 1, sei_fp);
    }

    /* delete the temporary file for this channel */
    if (channel_ptr->filename [0]) remove (channel_ptr->filename);
  }

  /* check and close the seisan file */
  if (ferror (sei_fp)) ret_val = FALSE;
  fclose (sei_fp);

  /* free the channel details */
  free (channels);
  channels = 0;
  n_channels = 0;

  /* remove the temporary directory */
  rmdir (tmp_dirname);

  /* if there was an error remove the seisan output file */
  if (! ret_val) remove (sei_filename);

  return ret_val;

}

/********************************************************************
 * write_seisan
 *
 * Description: write a FORTRAN unformatted record (!)
 *
 * Input parameters: fp - file to write to
 *           length - the length of the record in bytes
 *                   data - the data to write
 * Output parameters: none
 * Returns: none
 *
 * Comments:
 *
 ********************************************************************/
static void write_seisan (FILE *fp, int32_t length, void *data)

{

  int32_t value;


  value = length;
  if (! output_native_numbers) SwapInt32 (&value);
  fwrite (&value, 4, 1, fp);
  fwrite (data, length, 1, fp);
  fwrite (&value, 4, 1, fp);

}


/* --------------------------------------------------------------------
 * ------ General routines used by non-seisan putaway routines --------
 * -------------------------------------------------------------------- */

/************************************************************************
 * pa_find_data
 *
 * Description: find data in the earthworm data message
 *
 * Input parameters: trace_req - the trace request structure
 *                   start_time - start time for the data
 * Output parameters: data - the data found
 * Returns: one of the following codes:
 * FD_FOUND_REQUESTED   data was found starting at the requested time
 * FD_FOUND_GAP         there is no data for the requested time, but
 *                      there is data for a later time
 * FD_NO_MORE_DATA      the start time is after all data in the
 *                      request structure
 * FD_BAD_DATATYPE      found an unreconisable data type code
 * FD_CHANGED_SRATE     sample rate changes between snippets
 *
 * Comments: This is how the data structures work (I think - if this comment
 * is wrong, then so is the code):
 *
 * The TRACE_REQ structure is in ws_clientii.h
 *   trace_req->pBuf points to the data buffer
 *   trace_req->bufLen is the length of the buffer
 *
 * The data buffer starts with a TRACE_HEADER structure (trace_buf.h)
 *   trace_header->starttime, endtime, samprate hold the obvious things
 *   trace_header->datatype[0] holds 's' or 'i' for integer data
 *                             or    'f' or 't' for real data
 *   trace_header->datatype[1] holds the number of bytes/sample (coded in ASCII)
 * Immediately after the TRACE_HEADER structure is the data as an array.
 * Another TRACE_HEADER and data block may then follow - trace_req->bufLen
 * can be used to determine how many of these header/data pairs there are.
 *
 ************************************************************************/
int pa_find_data (TRACE_REQ *trace_req, double sample_time,
                  struct Found_data *data)

{

  int bytes_per_sample, data_type_code;
  int32_t n_samples, offset;
  double start_time, end_time, sample_rate, first_sample_rate;
  double test_start_time;
  char *msg_ptr;
  TRACE_HEADER *trace_hdr;


  /* walk the header/data array, looking for the given time */
  first_sample_rate = -1.0;
  msg_ptr = trace_req->pBuf;
  if (! msg_ptr) return FD_NO_MORE_DATA;
  while (msg_ptr < (trace_req->pBuf + trace_req->actLen))
  {
    /* extract details for this trace */
    trace_hdr = (TRACE_HEADER *) msg_ptr;

    /* swap bytes on incoming data if necessary (cjb 2/20/2002) */
    if (WaveMsgMakeLocal(trace_hdr) < 0)
    {
        logit("e", "pa_find_next: unknown trace data type: %s\n",
            trace_hdr->datatype);
        return( EW_FAILURE );
    }

    bytes_per_sample  = atoi (&trace_hdr->datatype[1]);
    n_samples = trace_hdr->nsamp;
    start_time = trace_hdr->starttime;
    end_time = trace_hdr->endtime;
    sample_rate = trace_hdr->samprate;

    if (fabs (sample_rate) < 0.000001) sample_rate = 100.0;

    /* check the sample rate */
    if (first_sample_rate < 0.0) first_sample_rate = sample_rate;
    else if (sample_rate != first_sample_rate) return FD_CHANGED_SRATE;

    /* check the data type codes */
    switch (trace_hdr->datatype [0])
    {
    case 's':
    case 'i':
      switch (bytes_per_sample)
      {
      case 2: data_type_code = FD_SHORT_INT; break;
      case 4: data_type_code = FD_LONG_INT; break;
      default: return FD_BAD_DATATYPE;
      }
      break;
    case 't':
    case 'f':
      switch (bytes_per_sample)
      {
      case 4: data_type_code = FD_FLOAT; break;
      case 8: data_type_code = FD_DOUBLE; break;
      default: return FD_BAD_DATATYPE;
      }
      break;
    default:
      return FD_BAD_DATATYPE;
    }


    /* This fixes the problem with all nulls in the data at the end of the Seisan file
    after a gap that wasn't really a gap. For example, we see the problem here:
        1,148,793,769.1900005 <- start time
        1,148,793,769.1900001 <- sample time
    where we actually can consider this sample equal to the start time, and not count as
    a gap.
        - stefan 20060605 */
    /* stefan 20070103: More versatile fix, Simon Flower's suggestion: */
    test_start_time = start_time - (0.49 * (1.0 / sample_rate));

    /* work out if this is the trace that we want */
    if ((sample_time >= test_start_time) && (sample_time <= end_time))
    {
      /* found the data we want - work out what to tell the caller */
      offset = (int32_t) (sample_rate * (sample_time - start_time));
      data->n_samples = n_samples - offset;
      data->data = msg_ptr + sizeof (TRACE_HEADER) + (offset * bytes_per_sample);
      data->data_type_code = data_type_code;
      data->sample_rate = sample_rate;
      data->trace_hdr = trace_hdr;
      return FD_FOUND_REQUESTED;
    }

    /* is there a gap between the given time and this trace */
    if (sample_time < start_time)
    {
      /* calculate the gap size */
      data->n_samples = (int32_t) (sample_rate * (start_time - sample_time));
      if ((data->n_samples == 0) && ((sample_rate * (start_time - sample_time)) > 0)) {
        /* Stefan 20070104
           a fractional value here means that we don't want to
           call it zero and stop asking for data, which a zero
           value will do. For example we have:
           sample_rate = 40.0
           start_time = 1161994799.0279999
           sample_time = 1161994799.0249999
           sample_rate * (start_time - sample_time) = 0.12000083923339844
           (long)(sample_rate * (start_time - sample_time)) = 0

           Simon Flower 20070108 - When I wrote it I wasn't aware of digitisers
           that would produce time stamps that were not intereger multiples of the
           sample rate, or digitisers where the sample rate drifted. The Seisan
           data format is not able to handle either of these situations. So if you
           look in seiputaway.c, you will find the place where the function you are
           working on is used. The function is called in a loop, until all data
           traces are exhausted. As part of the loop this happens:

               sample_time += ((double) data.n_samples / data.sample_rate

           Going back to seiutils.c and taking your example, where
                  start_time = 1161994799.0279999
                  sample_time = 1161994799.0249999

           data.n_samples should be set to a fraction (0.003 in your example) not
           1.0 - so although this works for the data you have, I think it possible
           that there will be situations where it does not work. However, I don't
           have an easy (or any other) answer, since, as I said, Seisan does not
           have a way to handle this situation.

           Stefan 20070127
           Various users report good results with the current kludge. It shouldn't
           affect any data that doesn't have this gap situation, and appears to fix
           most cases tested; so I'm releasing it as is to avoid more rewriting than
           I want to get involved with, and hopefully to improved the Seisan
           writing situation.

           */
        data->n_samples = 1;
      }
      data->sample_rate = sample_rate;
      return FD_FOUND_GAP;
    }

    /* set up msg_ptr for the next header/data array */
    msg_ptr += sizeof (TRACE_HEADER) + (n_samples * bytes_per_sample);
  }

  /* if you get here, then the data was not found */
  return FD_NO_MORE_DATA;

}
