/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: butterworth.h 2093 2006-03-09 21:24:06Z luetgert $
 *
 *	  butterworth_filter.h
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.2  2006/03/09 21:24:06  luetgert
 *     .
 *
 *     Revision 1.1  2006/01/30 19:41:10  friberg
 *     added in hydra includes for raypicker module
 *
 *     Revision 1.1.1.1  2005/06/22 19:30:36  michelle
 *     new directory tree built from files in HYDRA_NEWDIR_2005-06-20 tagged hydra and earthworm projects
 *
 *     Revision 1.2  2004/07/09 17:32:56  michelle
 *     added include for complex_math, since that is where the Complex data type
 *     is defined.  this is needed for builds in c++ to work
 *
 *     Revision 1.1  2004/04/21 19:59:10  cjbryan
 *
 *     CVSicreate butterworth filter: ----------------------------------------------------------------------
 *
 *
 */

#ifndef BUTTERWORTH_H
#define BUTTERWORTH_H

#include "complex_math.h"

int make_butterworth_filter(const unsigned int n, Complex *poles,
                            double *a0, const double wc);

#endif /*  BUTTERWORTH_H  */
