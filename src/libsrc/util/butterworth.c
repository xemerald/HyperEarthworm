/*----------------------------------------------------------------**
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: butterworth.c 2067 2006-01-30 19:35:36Z friberg $
 *
 * butterworth_filter()
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2006/01/30 19:35:35  friberg
 *     added in some Hydra libs needed by raypicker under NT
 *
 *     Revision 1.1.1.1  2005/06/22 19:30:24  michelle
 *     new directory tree built from files in HYDRA_NEWDIR_2005-06-20 tagged hydra and earthworm projects
 *
 *     Revision 1.2  2005/05/04 20:51:23  cjbryan
 *     changed return codes to EW_SUCCESS, EW_WARNING, and EW_FAILURE
 *
 *     Revision 1.1  2004/04/21 20:03:34  cjbryan
 *     *** empty log message ***
 *
 * 
 *  Generates the poles for a n-th order Butterworth (low pass)
 *  filter with a cut off frequency of wc radians/second.
 *  Array p[n] will contain the complex pole positions,
 *        a0 will be the real normalizing constant.
 *  The transfer function of the filter is evaluated:
 * 
 *      B[s] = a0 / (s+p[0]) * (s+p[1]) * ... * (s+p[n-1])
 * 
 *  where:
 * 
 *      s = cmplx(0.,w) and w is angular frequency.
 * 
 * returns:
 *      EW_SUCCESS = successful
 *      EW_FAILURE = bad call
 *      EW_WARNING = math error
 *  
 *  Original FORTRAN [mkbw()] from 23 August 1979 by R. Buland
 **-------------------------------------------------------------*/
#include <stdio.h>
#include <math.h>
#include <complex_math.h>
#include <math_constants.h>  /* HALF_PI */
#include <earthworm.h>


int make_butterworth_filter(const unsigned int n, Complex *poles,
                            double *a0, const double wc)
{
    int     half_order;
    int     i;                      /* loop counter */
    int	    k = 0;                  /* index into poles */
    double  an;
    Complex c;			

    /* bad call */
    if (poles == NULL || a0 == NULL || n == 0)
        return EW_FAILURE;
    else 
    {  
        /* The poles are spaced 2*an radians apart around a
         * circle of radius wc (cufoff frequency).*/
    
        an = HALF_PI/(double)n;
        half_order = n/2;
   
        /* If the order is odd, then set the first pole explicitly */
        if ((n % 2) != 0) 
        {
            poles[k].real = wc;
            poles[k].imag = 0.0;
            k++;
        }

        /* Generate the other poles */ 
        if (n > 1)
        { 			
            for (i = 0; i < half_order; i++, k +=2)
            {
                c.real = 0.0;
                c.imag = (k + 1) * an;
                poles[k] = rcmult(wc, Cexp(c));
                poles[k+1] = Conj(poles[k]);
            }
        }
      
        /*  Normalize the filter to one in the pass band. */
        if ((*a0 = pow(wc, (double)n)) == HUGE_VAL)
            return EW_WARNING;
    }   
    return EW_SUCCESS;
}
