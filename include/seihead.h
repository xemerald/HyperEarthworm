/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: seihead.h 6803 2016-09-09 06:06:39Z et $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.6  2007/03/28 13:54:45  paulf
 *     changed _UNIX to _MACOSX to be more specific
 *
 *     Revision 1.5  2007/03/28 13:47:53  paulf
 *     added UNIX check into slash orientation #ifdef
 *
 *     Revision 1.4  2006/03/10 13:50:56  paulf
 *     minor linux related fixes to removing _SOLARIS from the include line
 *
 *     Revision 1.3  2002/11/03 00:19:40  lombard
 *     Added earthworm RCS header
 *
 *
 *
 */

/* seihead.h - include file for SEIsan putaway routines */

#ifndef SEIHEAD_H
#define SEIHEAD_H


#include "earthworm_defs.h"
#include "trace_buf.h"

/* standard TRUE/FALSE definitions */
#ifndef TRUE
#define TRUE 		1
#endif
#ifndef FALSE
#define FALSE		0
#endif

/* define the operating system specific directory delimiter */
#if defined (_WINNT)
#define DIR_DELIM	'\\'
#elif defined  (_SOLARIS) || defined(_LINUX) || defined(_MACOSX)
#define DIR_DELIM	'/'
#else
#error "_WINNT or _SOLARIS must be set before compiling"
#endif

/* the value used to represent missing data in Seisan */
#define SEISAN_MISSING_DATA_FLAG		0

/* a structure used to define a seisan channel */
struct Sei_channel_details
{
  char chan_name [50];		/* channel name */
  char chan_type [50];		/* channel type code */
  char location [3];            /* 2 char location code */
  char network [3];             /* 2 char network code */
  double start_time;		/* start time of the data on this channel */
  int32_t n_samples;		/* number of data samples */
  int32_t n_written;		/* number of samples written to the file */
  double sample_rate;		/* the sample rate in Hz */

  int channel_count;		/* unique index for this channel */
  char filename [MAX_DIR_LEN];	/* name of the file holding data for this channel */
};

/* return codes for what was found by pa_find_data() */
#define FD_FOUND_REQUESTED	1		/* the requested data sample was found */
#define FD_FOUND_GAP		2		/* the sample was not found */
#define FD_NO_MORE_DATA		3		/* there is no more data in the request structure */
#define FD_BAD_DATATYPE		4		/* found an unreconisable data type code */
#define FD_CHANGED_SRATE	5		/* sample rate changes between snippets */

/* codes for data types in pa_find_data() */
#define FD_SHORT_INT		1
#define FD_LONG_INT			2
#define FD_FLOAT			3
#define FD_DOUBLE			4

/* this structure holds information and pointers to data found by find_data() */
struct Found_data
{
  int32_t n_samples;			/* number of samples found OR size of gap */
  double sample_rate;		/* sample rate in Hz */
  void *data;				/* pointer to the data */
  int data_type_code;		/* one of the codes above */
  TRACE_HEADER *trace_hdr;	/* pointer to the trace header */
};


/* forward declarations for seiutils.c: */
int open_seisan_file (char *output_dir, char *network_name, double st,
                      double dur, int onn);
int add_seisan_channel (char *chan_name, char *chan_type, char *network, char *location);
int start_seisan_channel (char *chan_name, char *chan_type,
                          double start_time, double sample_rate,
			  int32_t n_samples);
void add_seisan_channel_data (int32_t data_len, int32_t *data);
int end_seisan_channel (void);
int close_seisan_file (void);

int pa_find_data (TRACE_REQ *trace_req, double sample_time,
				  struct Found_data *data);

/* forward declarations for seiputaway.c: */
int SEIPA_init (char *output_dir, char *output_format, int debug);
int SEIPA_next_ev (TRACE_REQ *trace_req, int n_reqs, char *output_dir,
                   char *e_date, char *e_time, char *subnetName, int debug);
int SEIPA_next (TRACE_REQ *trace_req, double gap_thresh, int debug);
int SEIPA_end_ev (int debug);
int SEIPA_close (int debug);

#endif 	/* SEIHEAD_H */

