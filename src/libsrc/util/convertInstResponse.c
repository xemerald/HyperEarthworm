/*******************************************************************
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE        *
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.                    *
 *                                                                 *
 *    $Id: convertInstResponse.c 7406 2018-05-03 14:59:48Z alexander $                                                       *
 *                                                                 *
 *                                                                 *
 *                                                                 *
 *                                                                 *
 *    Revision history:                                            *
 *     $Log$
 *     Revision 1.1  2006/01/30 19:35:35  friberg
 *     added in some Hydra libs needed by raypicker under NT
 *
 *     Revision 1.1.1.1  2005/06/22 19:30:24  michelle
 *     new directory tree built from files in HYDRA_NEWDIR_2005-06-20 tagged hydra and earthworm projects
 *
 *     Revision 1.11  2004/09/22 15:56:37  labcvs
 *     Reverted to v1.9 to remove reportErrors.   JMP
 *
 *     Revision 1.9  2004/09/08 17:48:54  labcvs
 *     corrected error in trace amp calculation
 *
 *     Revision 1.8  2004/08/25 19:29:47  cjbryan
 *     cleanup
 *
 *     Revision 1.7  2004/08/25 19:27:05  cjbryan
 *     added more error checking to trapFilterZeroes
 *                             *
 *     Revision 1.5  2004/07/27 21:44:55  labcvs                   *
 *     added more error checking to evaluateTransferFn             *
 *                                                                 *
 *     Revision 1.4  2004/06/23 18:05:04  cjbryan                  *
 *                                                                 *
 *     fixed bug in constructTransferFnPZ                          *
 *                                                                 *
 *     Revision 1.2  2004/05/26 19:14:13  cjbryan                  *
 *     changed ResponseStruct to ResponseFnStruct                  *
 *                                                                 *
 *     Revision 1.1  2004/04/21 20:04:17  cjbryan                  *
 *     *** empty log message ***                                   *
 *                                                                 *
 *******************************************************************/

/* system includes */
#include <stdio.h>
#include <math.h>

/* earthworm and hydra includes */
#include <earthworm.h>
#include <complex_math.h>
#include <convertInstResponse.h>
#include <macros.h>
#include <math_constants.h>  /* PI */
#include <residue.h>


 /*******************************************************************
  * createAtoDFilter()                                              *
  *                                                                 *
  * Constructs a digital IIR filter to convert the response given   *
  * by the channel's poles and zeroes to the desired instrument     *
  * type response.                                                  *
  *                                                                 *
  * traceAmplitude = TRUE ==> calculate true amplitudes             *
  *                = FALSE ==> ground motion                        *
  * returns:                                                        *
  *      EW_SUCCESS = successful                                    *
  *      EW_FAILURE = something wrong                               *
  *                                                                 *
  *******************************************************************/
int createAtoDFilter(ResponseFnStruct *fromInstResp, FILTER  *pFilter, 
                     ResponseFnStruct *toInstResp, ResponseFnStruct *convertedResp,
                     int traceAmplitude)
{
    /* construct analog IIR filter */
    if (convertInstResponse(fromInstResp, pFilter, toInstResp, convertedResp, TRUE) 
         != EW_SUCCESS)
    {
        logit("", "Call to convertInstResponse failed. \n");
        return EW_FAILURE;
    }

    /* ensure that the IIR filter is stable; if input sensor is not sufficiently 
     * broadband or is a short period, the resulting IIR filter will be unstable. 
     * Even if the following calculation could be done, the result would be 
     * rubbish. */

    if (trapFilterZeroes(convertedResp, pFilter) != EW_SUCCESS)
    {
        logit("", "Call to trapFilterZeroes failed. \n");
        return EW_FAILURE;
    }

    /* construct digital filter */
    if (digitalFilterCoeffs(convertedResp->transferFn, pFilter) != EW_SUCCESS)
    {
        logit("", "Call to digitalFilterCoeffs failed. \n");
        return EW_FAILURE;
    }

    return EW_SUCCESS;
}

 /*******************************************************************
  * convertInstResponse()                                           *
  *                                                                 *
  * Constructs an analog IIR filter to convert the response given   *
  * by the channel's poles and zeroes to the desired instrument     *
  * type response.                                                  *
  *                                                                 *
  * assumes that the incoming response is given in displacement     *
  *                                                                 *
  * traceAmplitude = TRUE ==> calculate true amplitudes             *
  *                = FALSE ==> ground motion                        *
  *                                                                 *
  * returns:                                                        *
  *      EW_SUCCESS = successful                                    *
  *      EW_FAILURE = something wrong                               *
  *******************************************************************/
int convertInstResponse(ResponseFnStruct *fromInstResp, FILTER *rFilter, 
                  ResponseFnStruct *toInstResp, ResponseFnStruct *convertedResp,
                  int traceAmplitude)
{
    Complex transFn;            /* evaluated transfer function  */

    /* First we construct the transfer function for the desired 
     * response */
    if (constructTransferFunctionPZ(&fromInstResp->transferFn, rFilter, 
                                  &toInstResp->transferFn, 
                                  &convertedResp->transferFn) != EW_SUCCESS)
    {
        logit("", "constructTransferFunctionPZ fell on its sword \n");
        return EW_FAILURE;
    }

	/* load frequency of converted response */
	convertedResp->transferFn.tfFreq = toInstResp->transferFn.tfFreq;

	/* initialize normalization constant for converted response */
	convertedResp->transferFn.normConstant = 1.0;

    /* Evaluate this transfer function */
    if (evaluateTransferFn(&convertedResp->transferFn, &transFn) != EW_SUCCESS)
    {
        logit("", "evaluateTransferFn unsuccessful \n");
        return EW_FAILURE;
    }
    
    /* Compute the normalization constant a0 for the converted transfer function,
     * i.e., find a0 such that a0 = 1.0/|Hp(zs)| 
     * NB: the normalization constant does not include 
     * any information pertaining to instrument gain/sensitivity */
    convertedResp->transferFn.normConstant = 1.0/Cabs(transFn);

    /* trace amplitude or ground motion ? */
    if (traceAmplitude == TRUE)
        convertedResp->transferFn.normConstant *= toInstResp->gain[0] 
                                / (fromInstResp->gain[0] / fromInstResp->transferFn.normConstant);

    return EW_SUCCESS;

}


 /*******************************************************************
  * constructTransferFunctionPZ()                                   *
  *                                                                 *
  * Determines the poles and zeroes of a transfer function for      *
  * deconvolution of the given instrument reponse and convolution   *
  * of the desired instrument response                              *
  *                                                                 *
  * Only poles and zeroes below the Nyquist frequency implied       *
  * by sample interval filter.sampleRate are used. Discarding out   * 
  * band poles provides a computational savings. Also, if the       *
  * out-of-band poles were not discarded, the broadband             *
  * anti-aliasing poles would cause the new low-pass filter         *
  * to become unstable for very high frequencies (e.g., 50 Hz for   *
  * WWSSN). In practical terms, this can result in an IIR filter    *
  * that has a Z-transform that is significantly different than     *
  * the desired Laplace transform.                                  *
  *                                                                 *
  * Note that the sign on the poles and zeroes must be changed      *
  * because the USNSN standard is that the poles and zeroes for     *
  * stable filters are all in the left half of the complex plane    *
  * (i.e., the real parts are negative). The standard for the IIR   *
  * filter routines implemented here is the opposite. This merely   *
  * implies different choices for the FT sign convention. However,  *
  * it is important to get it right or the result is an unstable    *
  * filter that blows up when applied to a time series. Note that   *
  * it is safe to change sign on the whole pole or zero rather      *
  * than just the real part because the poles and zeroes are        *
  * either purely real or come in complex conjugate pairs.          *
  *                                                                 *
  *                                                                 *
  * returns:                                                        *
  *      EW_SUCCESS = successful                                    *
  *      EW_FAILURE = something wrong                               *
  *                                                                 *
  * Original FORTRAN [cntoww()] from 14 February 1979 by R. Buland  *
  *******************************************************************/
int constructTransferFunctionPZ(TransferFnStruct *fromInstResp, 
                                FILTER *pFilter, TransferFnStruct *toInstResp, 
                                TransferFnStruct *convertedResp)
{

    double  wmax;                          /* maximum frequency to allow              */
    double  eps = 0.00001 * TWO_PI;        /* tolerance for identical poles           */
    int     totalPoles = 0;                /* number of poles in converted response   */
    int     totalZeroes = 0;               /* number of zeroes in converted response  */
    Complex  poles[2 * MAX_PZ];            /* temporary array for poles of converted
                                            * response                                */
    Complex  zeroes[2 * MAX_PZ];           /* temporary array for zeroes of converted
                                            * response                                */
    int     usePole[2 * MAX_PZ];           /* use this pole ??                        */
    int     useZero[2 * MAX_PZ];           /* use this zero ??                        */
    int     np = 0;                        /* number of poles in combined response
                                            * (before cancellations)                  */
    int     nz = 0;                        /* number of zeroes in combined response  
                                            * (before cancellations)                  */
    int     i, j;                          /* loop counters                           */


    if (fromInstResp == NULL || toInstResp == NULL || convertedResp == NULL)
    {
        logit ("", "Invalid argument(s) passed in.\n");
        return EW_FAILURE;
    }

    
    /* max frequency of interest in radians - Nyquist frequency or some % thereof.
     * For typical broadband systems, the max desired frequency is usually
     * about 75% of the Nyquist frequency as this is about the location
     * of the brick wall FIR filter corner frequency */
    wmax =  pFilter->passband.freq_max * TWO_PI;

    /* check to make sure that wmax is reasonble, ie. wmax > 0 and wmax <= Nyquist frequency;
     * if not, set to the Nyquist frequency */
    if ((wmax <= eps) || (wmax > TWO_PI *  (pFilter->sampleRate / 2.)))
        wmax = TWO_PI *  (pFilter->sampleRate / 2.);

    /* poles of the converted response  = product of the zeroes of fromInstResp
     * and the poles of toInstResp */

    /* first the poles in the converted response due to the incoming inst resp 
     * recall that we discard out-of-band poles and zeroes */
    for (i = 0; i < fromInstResp->numZeroes; i++)
    {
        if (Cabs(fromInstResp->Zeroes[i]) < wmax)
        {
            poles[np].real = -1.0 * fromInstResp->Zeroes[i].real;
            poles[np].imag = -1.0 * fromInstResp->Zeroes[i].imag;
            usePole[np] = TRUE;
            np++;
        }
    }

    /* the poles in the converted response due to the inst type to which 
     * the response is to be converted */
    for (i = 0; i < toInstResp->numPoles; i++)
    {
        if (Cabs(toInstResp->Poles[i]) < wmax)
        {
            poles[np].real = -1.0 * toInstResp->Poles[i].real;
            poles[np].imag = -1.0 * toInstResp->Poles[i].imag;
            usePole[np]= TRUE;
            np++;
        }
    }

    /* zeroes of the converted response = product of the poles of fromInstResp
     * and the zeroes of toInstResp */

    /* first the zeroes in the converted response due to the incoming inst resp */
    for (i = 0; i < fromInstResp->numPoles; i++)
    {
        if (Cabs(fromInstResp->Poles[i]) < wmax)
        {
            zeroes[nz].real = -1.0 * fromInstResp->Poles[i].real;
            zeroes[nz].imag = -1.0 * fromInstResp->Poles[i].imag;
            useZero[nz] = TRUE;
            nz++;
        }
    }

    /* zeroes of converted response due to outgoing instrument response */
    for (i = 0; i < toInstResp->numZeroes; i++)
    {
        if (Cabs(toInstResp->Zeroes[i]) < wmax)
        {
            zeroes[nz].real = -1.0 * toInstResp->Zeroes[i].real;
            zeroes[nz].imag = -1.0 * toInstResp->Zeroes[i].imag;
            useZero[nz] = TRUE;
            nz++;
        }
    }

    totalPoles = np;
    totalZeroes = nz;

    /* Cancel out matching poles and zeroes */
    for (i = 0; i < np; i++)
    {
        if (usePole[i] == TRUE)
        {
            for (j = 0; j < nz; j++)
            {
                if (useZero[j] == TRUE)
                {
                    if ((poles[i].real == zeroes[j].real) &&
                        (poles[i].imag == zeroes[j].imag))
                    {
                        usePole[i] = FALSE;
                        totalPoles--;
                        useZero[j] = FALSE;
                        totalZeroes--;
                        break;
                    }
                }
            }
        }
    }


    /* check to make sure the number of poles and zeroes in the combined response
     * don't exceed the max allowed */
    if (totalPoles >= MAX_PZ)
    {
        logit("", "Number of poles in converted response (%d) exceeds allowed number %d\n",
                    totalPoles, MAX_PZ);
        return EW_FAILURE;
    }

    if (totalZeroes >= MAX_PZ)
    {
        logit("", "Number of zeroes in converted response (%d) exceeds allowed number %d\n",
                    totalZeroes, MAX_PZ);
        return EW_FAILURE;
    }

    /* move poles and zeroes to convertedResp structure */
    convertedResp->numPoles = 0;
    for (i = 0; i < np; i++)
    { 
        if (usePole[i] == TRUE)
        {
            convertedResp->Poles[convertedResp->numPoles].real = poles[i].real;
            convertedResp->Poles[convertedResp->numPoles].imag = poles[i].imag;
            convertedResp->numPoles++;
        }
    }

    convertedResp->numZeroes = 0;
    for (i = 0; i < nz; i++)
    { 
        if (useZero[i] == TRUE)
        {
            convertedResp->Zeroes[convertedResp->numZeroes].real = zeroes[i].real;
            convertedResp->Zeroes[convertedResp->numZeroes].imag = zeroes[i].imag;
            convertedResp->numZeroes++;
        }
    }

    /* split apart the duplicate poles */
    for (i = 0; i < totalPoles - 1; i++)
    {
        /* first, move poles at zero frequency to 10,000 s to avoid a divide by zero */
        if (Cabs(convertedResp->Poles[i]) <= eps)
                  rcadd(eps, convertedResp->Poles[i]);
        for (j = i + 1; j < totalPoles; j++)
        {
            if (Cabs(Csub(convertedResp->Poles[i], convertedResp->Poles[j])) <=
                eps * Cabs(Cadd(convertedResp->Poles[i], convertedResp->Poles[j])))
            {
                convertedResp->Poles[j] = Cadd(convertedResp->Poles[i], convertedResp->Poles[j]);
                rcmult(0.5, convertedResp->Poles[j]);
                convertedResp->Poles[i] = rcmult((1.0 + eps), convertedResp->Poles[j]);
                convertedResp->Poles[j] = rcmult((1.0 - eps), convertedResp->Poles[j]);
            }
        }
    }
    if (Cabs(convertedResp->Poles[totalPoles]) <= eps)
             rcadd(eps, convertedResp->Poles[totalPoles]);


    return EW_SUCCESS;
}

 /*******************************************************************
  * evaluateTransferFn()                                            *
  *                                                                 *
  * Evaluates the complex transfer function defined by real         *
  * normalization constant transferFn.normConst, the                *
  * transferFn->numPoles complex poles (transferFn->Poles[]), and   *
  * the transferFn->numZeroes complex zeroes (transferFn->Zeroes[], *
  * at period 1/transferFn.tfFreq.                                  *
  *                                                                 *
  * The units of the original transfer function are preserved.      *
  *                                                                 *
  * returns:                                                        *
  *      EW_SUCCESS = successful                                    *
  *      EW_FAILURE = meaningless result                            *
  *                                                                 *
  * Original FORTRAN [ewznp()] from R. Buland - 18 September 1991   *
  *******************************************************************/
int evaluateTransferFn(TransferFnStruct *transferFn, 
                       Complex *transFnValue)
{
    Complex *poles;
    Complex *zeroes;
    double  tol = 1e-30;        /* tolerance                    */
    double  cn;                 /* exponent over/underflow      */
    Complex s;                  /* Laplace transform variable   */
    Complex resp;               /* response function            */
    int     min_pz;             /* min of n_poles & n_zeroes    */
    int     i;                  /* loop counter                 */


    /* Bail out if the period or normalization constant are not
     * meaningful */
    if ((1.0/transferFn->tfFreq) <= tol || transferFn->tfFreq <= tol || transferFn->normConstant <= 0.0)
    {
        logit("", "evaluateTransferFn: Meaningless arguments passed in. tfFreq: %lf tol: %lf normConstant %lf\n",
			        transferFn->tfFreq, tol, transferFn->normConstant);
        return EW_FAILURE;
    }

    /* Compute Laplace transform variable */
    s = COMPLEX(0.0, (TWO_PI * transferFn->tfFreq));

    /* now for some intitialization */
    resp = COMPLEX(1.0, 0.0);
    cn = 1.0;
    poles = transferFn->Poles;
    zeroes = transferFn->Zeroes;

    /* Comnpute the transfer function. To avoid exponent overflow, 
     * do the poles and zeroes in pairs as much as is possible */
    min_pz = MIN(transferFn->numPoles, transferFn->numZeroes);
    if (min_pz > 0)
    {
        /* Loop over poles and zeroes accumulating their contributions 
         * to the response function */
        for (i = 0; i < min_pz;  i++)
        {
            resp = Cmult(resp, Cdiv(Csub(zeroes[i], s), Csub(poles[i], s)));
            if (pz_overflow(&resp, &cn) != EW_SUCCESS)
            {
                logit("", "Error in routine pz_overflow.\n");
                return EW_FAILURE;
            }
        }
    }

    /* Take care of any remaining poles and zeroes */

    /* More poles than zeroes ==> loop over poles accumulating
     * their contributions to the reponse function */
    if (transferFn->numPoles > min_pz)
    {
        for (i = min_pz; i < transferFn->numPoles; i++)
        {
            resp = Cdiv(resp, Csub(poles[i], s));
            if (pz_overflow(&resp, &cn) != EW_SUCCESS)
            {
                logit("", "Error in routine pz_overflow.\n");
                return EW_FAILURE;
            }
        }
    }
    /* more zeroes than poles ==> loop over zeroes accumulating 
     * their contributions to the response function */
    else if (transferFn->numZeroes > min_pz)
    {
        for (i = min_pz; i < transferFn->numZeroes; i++)
        {
            resp = Cmult(resp, Csub(zeroes[i], s));
            if (pz_overflow(&resp, &cn) != EW_SUCCESS)
            {
                logit("", "Error in routine pz_overflow.\n");
                return EW_FAILURE;
            }
        }
    }
    /* transferFn->numPoles = transferFn->numZeroes ==> 
     * nothing more to accumulate */
    else {
        ;
    }

    /* Factor the constants back in */
    *transFnValue = rcmult((transferFn->normConstant * cn), resp);

    return EW_SUCCESS;
}


 /*******************************************************************
  * pz_overflow()                                                   *
  *                                                                 *
  * Traps intermediate product over and underflow in the absolute   *
  * value of the complex variable response. If adjustment of the    *
  * exponent is necessary, the compensating value is stored in      *
  * compValue.                                                      *
  *                                                                 *
  * returns:                                                        *
  *      EW_SUCCESS = successful                                    *
  *                                                                 *
  * Original FORTRAN [zpovfl()] from R. Buland - 18 September 1991  *
  *******************************************************************/
int pz_overflow(Complex *response, double *compValue)
{
    double aresp;

    aresp = Cabs(*response);

    /* Guard against overflows */
    if (aresp >= 1e30)
    {
        *response = rcmult(1e-10, *response);
        *compValue = 1e10 * (*compValue);
    }
    else
    {
        /* Guard against underflows */
        if (aresp <= 1e-30)
        {
            *response = rcmult(1e10, *response);
            *compValue = 1e-10 * (*compValue);
        }
    }

    return EW_SUCCESS;
}
    

/*********************************************************************************************
 * digitalFilterCoeffs                                                                       * 
 *                                                                                           * 
 *  digitalFilterCoeffs converts a causal analog filter into a discrete time filter          *
 *  whose impulse response is the same as that of the analog filter                          *
 *  at sample points spaced filter.sample_interval seconds apart.                            *
 *  The analog filter is of the form:                                                        *
 *                                                                                           * 
 *     T(s) = tf.normConst *                                                                 *
 *             (s+tf.Zeroes[0]) * (s+tf.Zeroes[1]) * ... * (s+tf.Zeroes[tf.numZeroes - 1])   *
 *          /(s+tf.Poles[0]) * (s+tf.Poles[1]) * ... * (s+tf.Poles[tf.numPoles -1])          *
 *                                                                                           *
 *     where tf.normConst is the normalizing constant, tf.numZeroes is the number of zeroes, *
 *     tf.Zeroes is the array of zeroes, tf.numPoles is the number of poles,                 *
 *     tf.Poles is the array of poles, and s is the Legendre transform variable.             *
 *                                                                                           * 
 *  Note that parameters must be:                                                            *
 *                                                                                           * 
 *    0 <  tf.numPoles <= MAX_PZ                                                             *
 *    0 <= tf.numZeroes <= tf.numPoles                                                       *
 *                                                                                           * 
 *  quadratic filter sections filter->a and filter->g must be the same length,               *
 *  and at least 1/2 the length of tf.numPoles (+1 if tf.numPoles is odd).                   *
 *  filter->len is the used length of filter->a, filter->g                                   *
 *                                                                                           * 
 * @return  EW_SUCCESS = successful                                                          *
 *          EW_FAILURE = bad call                                                            *
 *                                                                                           * 
 * original FORTRAN [nprfrc] circa 1979 by Ray Buland                                        *
 *********************************************************************************************/
int digitalFilterCoeffs(TransferFnStruct tf, FILTER *filter)
{
    static const char POLE_USED   = 1;
    static const char POLE_UNUSED = 0;
    char poleused[MAX_PZ];

    int polesinpair;       /* counter for obtaining real pole pairs */
    double at[2];          /* pole position for real poles          */
    double pt[2];          /* residue for real poles                */
    Complex s;             /* conjugate of complex pole             */
    Complex ak;            /* pole position for complex pole        */
    Complex zk;            /* residue for complex pole              */
    int i;                 /* loop counter                          */
    int j;                 /* index of matched conjugate pole       */
    double sampleInterval; /* sampling interval for filter          */


    if (tf.Poles == NULL || tf.Zeroes == NULL || filter == NULL)
	{
        logit("", "digitalFilterCoeffs: NULL pointer passed in. \n");
        return EW_FAILURE;
	} 
   
    /* Check for valid number of poles and zeroes */
    if (tf.numZeroes < 0 || tf.numPoles < 1 || tf.numPoles < tf.numZeroes 
        || MAX_PZ < tf.numPoles) 
	{
        logit("", "digitalFilterCoeffs: invalid number of poles %d or zeroes %d \n",
			tf.numPoles, tf.numZeroes);
        return EW_FAILURE;
	}

    /* set sampling interval to save typing later */
    sampleInterval = 1.0 / filter->sampleRate;
       
    /* Initialize filter length */
    filter->len = 0;
   
    /* Search for pairs of real poles */
    polesinpair = 0;
   
    for (i = 0; i < tf.numPoles; i++)
    {
        /* Flag pole not used */
        poleused[i] = POLE_UNUSED;

        if (Imag(tf.Poles[i]) == 0.0)
        {
            /* Flag the pole as used */
             poleused[i] = POLE_USED;
            
            /*  Compute the pole position and z plane residue */
            pt[polesinpair] = exp(-(Real(tf.Poles[i])) * sampleInterval);
         
            at[polesinpair] = sampleInterval
              * Real(residue(tf, rcmult(-1.0, tf.Poles[i]), i));                            
  
            if ((++polesinpair) == 2)
            {
                /* A pair of real poles have been found,
                 * combine them into a quadratic filter section */
                filter->g[filter->len][0] =  at[0] + at[1];
                filter->g[filter->len][1] = -1.0 *(at[0] * pt[1] + at[1] * pt[0]);
                filter->a[filter->len][0] =  pt[0] + pt[1];
                filter->a[filter->len][1] = -1.0 * pt[0] * pt[1];
                filter->len++;
               
                /* reinitialize counter for next real pole pair */
                polesinpair = 0;            
            }
        }
    }
      
    if (polesinpair == 1)
    {
        /* There are an odd number of real poles,
         * make a degenerate section for the last one */
        filter->g[filter->len][0] = at[0];
        filter->g[filter->len][1] = 0.0;
        filter->a[filter->len][0] = pt[0];
        filter->a[filter->len][1] = 0.0;
        filter->len++;
	}
      
      
    /* Search for complex conjugate pairs among the unused poles */
    for (i = 0; i < tf.numPoles; i++)
    { 
        if (poleused[i] == POLE_UNUSED)
        {
            s = Conj(tf.Poles[i]);
      
            /* Look for the complex conjugate of tf.Poles[i]
             * among other unused poles */
            
            for (j = i + 1; j < tf.numPoles; j++)
            {
                if (tf.Poles[j].real == s.real && tf.Poles[j].imag == s.imag)
                {
                    /* found the conjugate, exit the loop */
                    break;
                }
            }
            
            /* j now points to the conjugate or past the end of the array (tf.numPoles);
             * No conjugate was found (i.e. the filter is non-causal); report an error */
            if (j == tf.numPoles)
			{ 
                logit("", "digitalFilterCoeffs: conjugate pole not found for %lf %lf \n",
                          tf.Poles[i].real, tf.Poles[i].imag);
                return EW_FAILURE;
			}
            
            /* Flag pole used to prevent reuse in next i loop pass */
            poleused[j] = POLE_USED;
            
            /* Compute the pole position and z plane residues */
            zk = Cexp(rcmult(sampleInterval, rcmult(-1.0, s))); 
            ak = rcmult(sampleInterval, 
                    residue(tf, rcmult(-1.0, s), j));
             
            /* Combine the pair into a quadratic section */
            filter->g[filter->len][0] =  2.0 * Real(ak);
            filter->g[filter->len][1] = -2.0 * Real(Cmult(ak, Conj(zk))) ;
            filter->a[filter->len][0] =  2.0 * Real(zk);
            filter->a[filter->len][1] = -(Real(Cmult(zk, Conj(zk))));                        
            filter->len++;
        }
    }  
    return EW_SUCCESS;
}
 /*******************************************************************
  * trapFilterZeroes()                                              *
  *                                                                 *
  * Ensure that the zeroes of a filter (the poles of the input      *
  * instrument) are not in the passband of the desired response     *
  *                                                                 *
  * User is responsible for setting the passband frequencies        *
  *       fmax should NEVER exceed the Nyquist frequency            *
  *       fmin should be greater than zero                          *
  *                                                                 *
  * returns:                                                        *
  *      EW_FAILURE if the modulus of any zero falls between the    *
  *                  passband min and max frequencies               *
  *      EW_SUCCESS = if tall modulai OK                            *
  *                                                                 *
  * Original FORTRAN [bbtrap()] from R. Buland                      *
  *******************************************************************/
int trapFilterZeroes(ResponseFnStruct *convertedResp, FILTER *pFilter)
{
    double fmin;         /* min acceptable frequency (radians) */
    double fmax;         /* max acceptable frequency (radians) */
    double value;        /* magnitude of zero                  */
    double eps = 1e-9 * TWO_PI; 
    int    i;


    fmin = pFilter->passband.freq_min * TWO_PI;
    fmax = pFilter->passband.freq_max * TWO_PI;

    /* don't allow zeroes at zero frequency */
    if (fmin < eps)
        fmin = eps;

    /* check to make sure that fmax is reasonble, ie. fmax > 0 and fmax <= Nyquist frequency;
     * if not, set to the Nyquist frequency */
    if ((fmax <= eps) || (fmax > TWO_PI *  (pFilter->sampleRate / 2.)))
        fmax = TWO_PI *  (pFilter->sampleRate / 2.);

    /* now check for zeroes in the passband */
    for (i = 0; i < convertedResp->transferFn.numZeroes; i++)
    {
        value = Cabs(convertedResp->transferFn.Zeroes[i]);
        if (value >= fmin && value <= fmax)
        {
            logit("", "trapFilterZeroes: filter has a zero at %lf, i.e in passband (%lf - %lf) of filter \n",
                         value, fmin, fmax);
            return EW_FAILURE;
        }
    }

    return EW_SUCCESS;
}

