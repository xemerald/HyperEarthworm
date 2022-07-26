/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: ioc_filter.c 3507 2008-12-22 19:48:04Z paulf $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.2  2008/12/22 19:48:04  paulf
 *     removed malloc.h include for Mac OS X compilation
 *
 *     Revision 1.1  2006/01/30 19:35:35  friberg
 *     added in some Hydra libs needed by raypicker under NT
 *
 *     Revision 1.6  2005/06/21 22:12:59  cjbryan
 *     added missing #include <math.h>
 *
 *     Revision 1.5  2005/06/20 21:24:53  cjbryan
 *     added preFilterSamplePoint and filterTimeSeriesSample
 *
 *     Revision 1.3  2005/05/04 21:01:53  cjbryan
 *     added more shared functions; cleaned up error reporting
 *
 *     Revision 1.2  2004/07/13 19:00:50  cjbryan
 *     revised filter routines to use RECURSIVE_FILTER struct
 *
 *     Revision 1.1  2004/04/21 20:04:41  cjbryan
 *     *** empty log message ***
 *
 *
 */

/* system includes */
#include <stdio.h>
#include <math.h>
#include <string.h>           /* for memcpy */


/* hydra includes */
#include <earthworm.h>
#include <butterworth.h>
#include <complex_math.h>
#include <convertInstResponse.h>
#include <ioc_filter.h>
#include <math_constants.h>
#include <transferFunction.h>
#include <watchdog_client.h>

static FILTER *Filters;
static TransferFnStruct filterTF;

/* prototypes for functions seen only by this file */
static void initFilter(FILTER *filter);
static int  setupBroadbandFilter(const double sampleInterval, FILTER *filter);
static int  setupShortPeriodFilter(const double sampleInterval, FILTER *filter);

/***********************************************************
 *              initTransferFn()                           *
 *                                                         *
 * Initializes transfer function structure for filter      *
 ***********************************************************/
int initTransferFn(void)
{
    if ((filterTF.Poles = (Complex *)malloc(sizeof(Complex) * MAX_PZ)) == NULL)
    {
        reportError(WD_FATAL_ERROR, MEMALLOC, "initTransferFn(): Could not allocate filterTF.Poles \n");
        return EW_FAILURE;
    }

    if ((filterTF.Zeroes = (Complex *)malloc(sizeof(Complex) * MAX_PZ)) == NULL)
    {
        reportError(WD_FATAL_ERROR, MEMALLOC, "initTransferFn(): Could not allocate filterTF.Zeroes \n");
        return EW_FAILURE;
    }
    
    resetTransferFn();

    return EW_SUCCESS;
}

/***********************************************************
 *              resetTransferFn()                          *
 *                                                         *
 * Initialize transfer function poles and zeroes           *
 ***********************************************************/
void resetTransferFn(void)
{
    filterTF.numPoles = 0;
    filterTF.numZeroes = 0;
    filterTF.tfFreq = 0.0;
    filterTF.normConstant = 1.0;
}

/***********************************************************
 *              freeTransferFn()                           *
 *                                                         *
 * Frees transfer function poles and zeroes                *
 ***********************************************************/
void freeTransferFn(void)
{
    cleanTF(&filterTF);
}

/***********************************************************
 *              initAllFilters()                           *
 *                                                         *
 * Initializes all shared filter arrays                    *
 * Call once at the start of processing                    *
 ***********************************************************/
int initAllFilters(int maxFilters)
{
    int f;

    if ((Filters = (FILTER *)calloc(maxFilters, sizeof(FILTER))) ==  NULL)
    {
        reportError(WD_FATAL_ERROR, MEMALLOC, "initAllFilters cannot allocate Filters; exiting.\n");
        return EW_FAILURE;
    }

    for (f = 0; f < maxFilters; f++)
        initFilter(&Filters[f]);

    return EW_SUCCESS;
}

/***********************************************************
 *               initFilter()                              *
 *                                                         *
 * Initializes a single shared filter array                *
 ***********************************************************/
static void initFilter(FILTER *filter)
{
    filter->sampleRate  = 0.0;
    filter->isBroadband = TRUE;
    filter->len         = 0;
    filter->passband.freq_min = 0;
    filter->passband.freq_max = 0;
}

/***********************************************************
 *                  setupBroadbandFilter()                 *
 *                                                         *
 * Sets up a broadband filter                              *
 ***********************************************************/
static int setupBroadbandFilter(const double sampleInterval, FILTER *filter)
{
    static const unsigned int nhp1 = 2;                        /* number of poles for high pass filter 1  */    
    static const unsigned int nhp2 = 2;                        /* number of poles for high pass filter 2  */ 
    static const unsigned int nlp1 = 2;                        /* number of poles for low pass filter 1   */
    static const unsigned int nlp2 = 2;                        /* number of poles for low pass filter 2   */ 
    static const double       hp1 = 0.5;                       /* cutoff freq for high pass filter 1      */
    static const double       hp2 = 1.05;                      /* cutoff freq for high pass filter 2      */
    static const double       lp1 = 2.65;                      /* cutoff freq for low pass filter 1       */
    static const double       lp2 = 6.5;                       /* cutoff freq for low pass filter 2       */
                        
                 double       hp1pi2 = hp1 * TWO_PI;           /* cutoff freq (radians) for hp1 */
                 double       hp2pi2 = hp2 * TWO_PI;           /* cutoff freq (radians) for hp2 */
                 double       lp1pi2 = lp1 * TWO_PI;           /* cutoff freq (radians) for lp1 */
                 double       lp2pi2 = lp2 * TWO_PI;           /* cutoff freq (radians) for lp2 */
                 int          np;                              /* number of poles               */
                 int          nz;                              /* number of zeroes              */

                 Complex      p[8];                            /* array of poles                */
                 Complex      z[4] = {{0.0, 0.0}, {0.0, 0.0}, 
                                      {0.0, 0.0}, {0.0, 0.0}}; /* array of zeroes               */
	 
                 double       a0, a1, a2;                      /* normalization constants       */
                 int          rc;                              /* return code                   */


    if (filter == NULL)
	{
		reportError(WD_FATAL_ERROR, GENFATERR, "setupBroadbandFilter: filter passed in NULL \n");
		return EW_FAILURE;
	}
			
	/* Make up a recursive filter for broadband data. */
    nz = nhp1 + nhp2;
    np = nz + nlp1 + nlp2;

    if (np > MAX_PZ || nz > MAX_PZ)
    {
        reportError(WD_FATAL_ERROR, GENFATERR, "Programming error: Too many poles (%d > %d allowed) or zeroes (%d . %d allowed) \n",
                     np, MAX_PZ, nz, MAX_PZ);
        return EW_FAILURE;
    }

    /* make first high pass filter */   
    if ((rc = make_butterworth_filter(nhp1, &p[0], &a2, hp1pi2)) != EW_SUCCESS)
    {
        reportError(WD_WARNING_ERROR, EWERR, "Call to make_butterworth_filter failed for first HP filter. \n");
		return rc;
    }
	
    /* make second high pass filter */   
    if ((rc = make_butterworth_filter(nhp2, &p[nhp1], &a2, hp2pi2)) != EW_SUCCESS)
    {
        reportError(WD_WARNING_ERROR, EWERR, "Call to make_butterworth_filter failed for second HP filter. \n");
		return rc;
    }
      
    /* make first low pass filter */
    if ((rc = make_butterworth_filter(nlp1, &p[nz], &a1, lp1pi2)) != EW_SUCCESS)
    {
        reportError(WD_WARNING_ERROR, EWERR, "Call to make_butterworth_filter failed for first LP filter. \n");
		return rc;
    }
      
    /* make second low pass filter */
    if ((rc = make_butterworth_filter(nlp2, &p[nz+nlp1], &a0, lp2pi2)) != EW_SUCCESS)
    {
        reportError(WD_WARNING_ERROR, EWERR, "Call to make_butterworth_filter failed for second LP filter. \n");
		return rc;
    }

    /* normalization constant depends only on low-pass filter constants */
    a1 = a1 * a0;

   /* fill in the transfer function structure */
    filterTF.numPoles = np;
    memcpy(filterTF.Poles, p, sizeof(Complex) * filterTF.numPoles);

    filterTF.numZeroes = nz;
    memcpy(filterTF.Zeroes, z, sizeof(Complex) * filterTF.numZeroes);

    filterTF.tfFreq = 1.0 / sampleInterval;
    filterTF.normConstant = a1;
   
    /* construct digital filter */
    if ((rc = digitalFilterCoeffs(filterTF, filter)) != EW_SUCCESS)
		return rc;
   
    return EW_SUCCESS;
}

/***********************************************************
 *              setupShortPeriodFilter()                   *
 *                                                         *
 * Sets up a short-period filter                           *
 ***********************************************************/
static int setupShortPeriodFilter(const double sampleInterval, FILTER *filter)
{
    Complex           p[4];                            /* array of poles              */
    Complex           z[2] = {{0.0, 0.0}, {0.0, 0.0}}; /* array of zeroes             */
    double            a1, a2;                          /* normalization constants     */
    int               rc;                              /* return code                 */


    /* set up high pass filter */
    if ((rc = make_butterworth_filter(2, &p[0], &a2, M_PI)) != EW_SUCCESS)
    {
        reportError(WD_WARNING_ERROR, EWERR, "Call to make_butterworth_filter failed for SP HP filter. \n");
		return rc;
    }

    /* set up low pass filter */
    if ((rc = make_butterworth_filter(2, &p[2], &a1, EIGHT_PI)) != EW_SUCCESS)
    {
        reportError(WD_WARNING_ERROR, EWERR, "Call to make_butterworth_filter failed for SP LP filter. \n");
		return rc;
    }

    /* fill in the transfer function structure */
    filterTF.numPoles = 4;
    memcpy(filterTF.Poles, p, sizeof(Complex) * filterTF.numPoles);

    filterTF.numZeroes = 2;
    memcpy(filterTF.Zeroes, z, sizeof(Complex) * filterTF.numZeroes);

    filterTF.tfFreq = 1.0 / sampleInterval;
    filterTF.normConstant = a1;
 
    /* construct digital filter */
    if ((rc = digitalFilterCoeffs(filterTF, filter)) != EW_SUCCESS)
		return rc;
   
    return EW_SUCCESS;
}

/***********************************************************
 *                  initChannelFilter()                    *
 *                                                         *
 * Initializes the filter for a given sampling rate.       *
 *                                                         *
 * Call once, when the channel is first encountered.       *
 *                                                         *
 * @ return  EW_SUCCESS = okay                             *
 *           EW_FAILURE = no matching common pre-filter or *
 *               failed initializing common pre-filter     *
 ***********************************************************/
int initChannelFilter(const double sampleRate, const double mean, 
                      const int isBroadband, RECURSIVE_FILTER *rfilter,
                      int maxFilters)
{
    double check_value = 0.05 / sampleRate;    /* tolerance for sampleRate match */      
    int    isNew    = TRUE;                    /* flag for new filter            */
    int    filterIndex;
    int    f;                                  /* index into filter array        */
    double sample_interval;                    /* sampling interval              */
    int    rc;                                 /* return code                    */

    /* set up */
    filterIndex = maxFilters;

    /* See if we have a filter already set up for this rate. */
    for (f = 0; f < maxFilters; f++)
    {
        /* at any time filter matches, exit loop to use it */		      
        if (fabs((1.0 / sampleRate) - (1.0 / Filters[f].sampleRate)) <= check_value
           && Filters[f].isBroadband == isBroadband)    
        {
            filterIndex = f;
            isNew = FALSE;
            break;
        }
    } /* checked all existing pre-filters for matching sampling rate */

    /* did not find a matching pre-filter, so check for an empty slot - 
     * sample interval 0.0 means this pre-filter slot not yet used. */
	if (filterIndex == maxFilters)
    {
        for (f = 0; f < maxFilters; f++)
        {
            if (Filters[f].sampleRate == 0.0)
            {
                filterIndex = f;
			    break;
            }
        }
    } /* checked for an empty slot */
   
    /* Unable to find a matching filter or an unused filter. Exit with an error condition.*/
    if (filterIndex == maxFilters)
    {
        reportError(WD_WARNING_ERROR, EWERR, "initChannelFilter(): Insufficient space for additional pre-filter\n");
        return EW_WARNING;
    }
   
    /* No matching pre-filter found, Set up the new coefficients.*/
    if (isNew)
    { 
        Filters[f].sampleRate = sampleRate;
	    sample_interval = 1.0 / sampleRate;

        /* set high end of passband to Nyquist frequency to prevent aliasing */
        Filters[f].passband.freq_max = 0.5 * Filters[f].sampleRate;
      
        if (isBroadband)
        {
            if ((rc = setupBroadbandFilter(sample_interval, &Filters[filterIndex])) != EW_SUCCESS)
            {	
                /* failed setup; this filter no longer any good */
                initFilter(&Filters[f]);
                return rc;
            }
        }
        else
        {
            if ((rc = setupShortPeriodFilter(sample_interval, &Filters[filterIndex])) != EW_SUCCESS)
            {
                /* failed setup; his filter no longer any good */
                initFilter(&Filters[filterIndex]);
                return rc;
            }
        }
    }
   
    /* initialize this channel's filter data (for the start of a new time series) */
    rfilter->filter              = &Filters[filterIndex];
    rfilter->filter->isBroadband =  isBroadband;
    rfilter->mean                =  mean;
    
    /* initialize the filter coefficients */
    if (isBroadband)
        initRecursiveFilter(rfilter);

    return EW_SUCCESS;
}

/*******************************************************************
 * wtmags()                                                        *
 *                                                                 *
 * Calculates a period dependent weight used for choosing the      *
 * amplitude and period. The weight is set such that the reading   *
 * will be at the largest amplitude near a period of per0.         *
 *                                                                 *
 * returns:                                                        *
 *      weighting factor                                           *
 *******************************************************************/
double wtmags(double period, double per0)
{
    if (period > per0)
        return (per0/period);
    else
        return (period/per0);
}
      
/*******************************************************************
 * initRecursiveFilter()                                           *
 *                                                                 *
 * Initializes the recursive filter.                               *
 *                                                                 *
 * returns:                                                        *
 *      EW_SUCCESS = successful                                    *
 *******************************************************************/
int initRecursiveFilter(RECURSIVE_FILTER *rfilter)
{
    int     i, j;


    for (i = 0; i < rfilter->filter->len; i++)
        for (j = 0; j < 2; j++)
            rfilter->q[i][j] = 0.0;

    return EW_SUCCESS;

}

/***********************************************************
 * preFilterSamplePoint()                                  *
 *                                                         *
 * Transforms one broadband or short-period sample value   *
 * into a pseudo short-period value suitable for similar   *
 * processing.                                             *
 *                                                         *
 * from Buland                                             *
 ***********************************************************/
int preFilterSamplePoint(const double  sampleValue, 
                          RECURSIVE_FILTER *prefilter, double *filteredValue)
{
    if (prefilter->filter->isBroadband)
    { 
        double xn = sampleValue - prefilter->mean;
       
        *filteredValue = filterTimeSeriesSample(xn, prefilter);
    }
    else
        *filteredValue = sampleValue - prefilter->mean;
   
    return EW_SUCCESS;
}

/*******************************************************************
 * filterTimeSeriesSample()                                        *
 *                                                                 *
 * Point-by-point application of the filter computed by            *
 * digitalFilterCoeffs to filter a time series with the sample     *
 * interval dt, The filter is applied recursively with the l       *
 * quadratic sections in parallel.                                 *
 *                                                                 *
 * xn = demeaned signal                                            *
 *                                                                 *
 * returns:                                                        *
 *      filtered value                                             *
 *                                                                 *
 * original FORTRAN [dfiln] 23 August 1970 by R. Buland            *
 *******************************************************************/
double filterTimeSeriesSample(double xn, RECURSIVE_FILTER *recursive_filter)
{

    double  qt;
    double  yn = 0.0;
    int     j;          /* loop counter */

    
    for (j = 0; j < recursive_filter->filter->len; j++)
    {
        qt = recursive_filter->filter->a[j][1] * recursive_filter->q[j][0] + 
             recursive_filter->filter->a[j][0] * recursive_filter->q[j][1] + xn;

        yn += recursive_filter->filter->g[j][0] * qt  +  
              recursive_filter->filter->g[j][1] * recursive_filter->q[j][1];

        recursive_filter->q[j][0] = recursive_filter->q[j][1];
        recursive_filter->q[j][1] = qt;
    }
       
    return yn;
}

/*******************************************************************
 * evalFilter()                                                    *
 *                                                                 *
 * Evaluates the z transform of the filter computed by             *
 * digitalFilterCoeffs at w radians/second                         *
 *                                                                 *
 * returns:                                                        *
 *      filtered value                                             *
 *                                                                 *
 * original FORTRAN [dfilz] 23 August 1970 by R. Buland            *
 *******************************************************************/
Complex evalFilter(double w, FILTER *filter)
{
    Complex zi;
    Complex hd          = COMPLEX(0.0, 0.0);
    Complex numerator   = COMPLEX(0.0, 0.0);
    Complex denominator = COMPLEX(0.0, 0.0);

    int     i;

    zi = Cexp(COMPLEX(0.0, -1.0 * w / filter->sampleRate));

    for (i = 0; i < filter->len; i++)
    {
        /* hd += (filter->g[i][0] + filter->g[i][1] * zi) / 
                    (1.0 - zi *(filter->a[i][0] + zi * filter->a[i][1])); */

        numerator   = rcadd(filter->g[i][0], rcmult(filter->g[i][1], zi));
        denominator = rcadd(filter->a[i][0], rcmult(filter->a[i][1], zi));
        denominator = rcsub(1.0, Cmult(zi, denominator));
        hd          = Cadd(hd, Cdiv(numerator, denominator));
    }

    return hd;
}

