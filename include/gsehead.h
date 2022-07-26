/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: gsehead.h 1109 2002-11-03 00:10:13Z lombard $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.2  2002/11/03 00:10:13  lombard
 *     Added earthworm RCS header.
 *     Protected from multiple includes.
 *
 *
 *
 */
#ifndef GSEHEAD_H
#define GSEHEAD_H

/* gsehead.h - include file for GSE putaway routines */

int GSEPA_init (char *output_dir, char *output_format, int debug);
int GSEPA_next_ev (TRACE_REQ *trace_req, int n_reqs, char *output_dir,
                   char *e_date, char *e_time, int debug);
int GSEPA_next (TRACE_REQ *trace_req, double gap_thresh, int debug);
int GSEPA_end_ev (int debug);
int GSEPA_close (int debug);

#endif
