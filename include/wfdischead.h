/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: wfdischead.h 6803 2016-09-09 06:06:39Z et $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.5  2010/01/08 19:19:55  tim
 *     Added structs for a more mature configuration
 *
 *     Revision 1.4  2010/01/07 20:08:42  tim
 *     add fp_array struct
 *
 *     Revision 1.3  2009/10/07 20:51:01  tim
 *     Add defines for changing byte order
 *
 *     Revision 1.2  2009/06/04 19:20:13  tim
 *     Adding support for wfdisc params struct
 *
 *     Revision 1.1  2009/05/18 20:55:01  tim
 *     Add support for wfdisc output
 *
 */

/* wfdischead.h - include file for wfdisc putaway routines */

#ifndef WFDISCHEAD_H
#define WFDISCHEAD_H


//#include "earthworm_defs.h"
#include "trace_buf.h"

/* standard TRUE/FALSE definitions */
#ifndef TRUE
#define TRUE 		1
#endif
#ifndef FALSE
#define FALSE		0
#endif

#ifndef INTEL_OUTPUT
#define INTEL_OUTPUT       0
#endif
#ifndef SPARC_OUTPUT
#define SPARC_OUTPUT       1
#endif


//wfdisc params struct
typedef struct
{
  char sta[7];
  char chan[9];
  int32_t chanid;
  float calib;
  float calper;
  char instype[7];
  char segtype[2];
} wfdisc_params;

//wfdisc filepointer pair struct
typedef struct
{
  char sta[7];
  FILE *wfdisc_fp;
} FP_PAIR;

//wfdisc out put spec struct
typedef struct
{
  char outname[20]; //Max does not need to be 20 chars, made it so to simplify coding
  char sta[7];
  char chan[9];
} wfdisc_spec;

//config struct
typedef struct
{
  //Number of calibration parameters in array
  int n_calib_params;
  wfdisc_params *calib_params;
  //data file location
  char w_dir[65]; //Set max size of 65 because that is all that can fit in the wfdisc struct
  //whether stations should all be in one wfdisc file or not
  int allinone;
  //How wfdisc files should be split up: 1=every hour, 2=every 2 hours, etc. 
  int file_split;
  //number of wfdisc specifiers in array
  int n_spec_array;
  wfdisc_spec *spec_array;
} wfdisc_config;


/* forward declarations for seiputaway.c: */
int WFDISCPA_init (char *output_dir, char *output_format, int debug);
int WFDISCPA_next_ev (TRACE_REQ *trace_req, int n_reqs, char *output_dir,
                   char *e_date, char *e_time, char *subnetName, int debug);
int WFDISCPA_next (TRACE_REQ *trace_req, double gap_thresh, int debug);
int WFDISCPA_end_ev (int debug);
int WFDISCPA_close (int debug);

extern wfdisc_config wf_config;
#endif 	/* WFDISCHEAD_H */

