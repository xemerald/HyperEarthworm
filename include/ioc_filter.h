/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: ioc_filter.h 2068 2006-01-30 19:41:11Z friberg $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2006/01/30 19:41:11  friberg
 *     added in hydra includes for raypicker module
 *
 *     Revision 1.1.1.1  2005/06/22 19:30:36  michelle
 *     new directory tree built from files in HYDRA_NEWDIR_2005-06-20 tagged hydra and earthworm projects
 *
 *     Revision 1.7  2005/06/20 21:25:12  cjbryan
 *     added preFilterSamplePoint and filterTimeSeriesSample
 *
 *     Revision 1.6  2005/05/04 21:03:42  cjbryan
 *     added shared functions
 *
 *     Revision 1.5  2004/08/25 16:46:58  cjbryan
 *     added PASSBAND struct; modified FILTER struct
 *
 *     Revision 1.4  2004/07/16 22:32:59  michelle
 *     made .h function declarations match the implementation in the .c file
 *
 *     Revision 1.3  2004/07/13 18:59:24  cjbryan
 *     added RECURSIVE_FILTER struct
 *
 *     Revision 1.2  2004/04/23 16:06:21  cjbryan
 *      changed bool to int
 *
 *     Revision 1.1  2004/04/21 20:00:56  cjbryan
 *     *** empty log message ***
 *
 *
 *
 */
 

#ifndef IOC_FILTER_H
#define IOC_FILTER_H

#include <complex_math.h>  /* for Complex */

/* recursive filter maximum lengt */
#define RECURSIVE_FILTER_MAX_LEN 30

typedef struct _PASSBAND
{ 
    double  freq_min;                       /* min frequency (Hz)        */
    double  freq_max;                       /* max frequency (Hz)
                                             * (should NEVER exceed the 
                                             * Nyquist frequency)        */   
} PASSBAND;

typedef struct _FILTER
{ 
    short   len;                            /* filter length             */
    double  sampleRate;                     /* sampling rate             */
    int     isBroadband;                    /* broadband or short-period */

    PASSBAND passband;                       /* passband for this filter  */
   
    double  a[RECURSIVE_FILTER_MAX_LEN][2]; /* quadratic filter section  */
    double  g[RECURSIVE_FILTER_MAX_LEN][2]; /* quadratic filter section  */

} FILTER;

typedef struct _RECURSIVE_FILTER
{ 
    FILTER  *filter;                         /* filter design info       */
    double   q[RECURSIVE_FILTER_MAX_LEN][2]; /* recursive filter section */
    double   mean;
} RECURSIVE_FILTER;


/* function prototypes */
int     initTransferFn(void);
void    resetTransferFn(void);
void    freeTransferFn(void);
int     initAllFilters(int maxFilters);
int     initChannelFilter(const double sampleRate, const double mean, 
                          const int isBroadband, RECURSIVE_FILTER *rfilter,
                          int maxFilters);
double  wtmags(double period, double per0);
int     initRecursiveFilter(RECURSIVE_FILTER *rfilter);
int     preFilterSamplePoint(const double  sampleValue, RECURSIVE_FILTER *prefilter, 
                             double *filteredValue);
double  filterTimeSeriesSample(double xn, RECURSIVE_FILTER *recursive_filter);
Complex evalFilter(double w, FILTER *filter);

#endif /* IOC_FILTER_H */
