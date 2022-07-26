/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: residue.c 2073 2006-01-30 21:45:52Z friberg $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2006/01/30 21:45:52  friberg
 *     added from Hydra for raypicker
 *
 *     Revision 1.1.1.1  2005/06/22 19:30:24  michelle
 *     new directory tree built from files in HYDRA_NEWDIR_2005-06-20 tagged hydra and earthworm projects
 *
 *     Revision 1.2  2004/04/22 14:29:36  patton
 *     Update from Carol.
 *
 *
 * 
 * @author Ray Buland, original FORTRAN
 */

/* system includes */
#include <stdio.h>

/* hydra includes */
#include <residue.h>

Complex residue(TransferFnStruct transferFn, const Complex s, const int i)
{
    Complex	d;
    int		j;

   if (transferFn.Poles == NULL || transferFn.numPoles <= 0)
      return COMPLEX(0.0, 0.0);

   else 
   {
      d = COMPLEX(1.0, 0.0);
      
      /* Compute the contribution of all but the ith pole */
      for (j = 0; j < transferFn.numPoles ; j++)
      {
          if (j != i)
              d = Cmult(d, Cadd(s, transferFn.Poles[j]));
      }
  
      d = rcdiv(1.0, d);

      /* Compute the contribution of the zeroes if any */
      if (transferFn.Zeroes != NULL)
      {
          for (j = 0; j < transferFn.numZeroes; j++ )
              d = Cmult(d, Cadd(s, transferFn.Zeroes[j]));
      }
		
      return rcmult(transferFn.normConstant, d);
   }
}
                       
 
