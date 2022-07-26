/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: complex_math.c 7332 2018-04-17 18:49:32Z paulf $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2006/01/30 19:35:35  friberg
 *     added in some Hydra libs needed by raypicker under NT
 *
 *     Revision 1.2  2005/08/30 16:06:49  cjbryan
 *     added Cmod function
 *
 *     Revision 1.1.1.1  2005/06/22 19:30:24  michelle
 *     new directory tree built from files in HYDRA_NEWDIR_2005-06-20 tagged hydra and earthworm projects
 *
 *     Revision 1.3  2004/04/14 21:58:04  davidk
 *     Changed code in Cexp(), so that complex numbers that are initialized
 *     from variables, are now initialized in longhand, instead of in shorthand
 *     as a single structure, due to complaints by Sun C compiler.
 *
 *     Revision 1.2  2004/04/09 23:22:55  davidk
 *     Added Csub() subtraction function.
 *
 *     Revision 1.1  2004/04/09 23:17:10  davidk
 *     Moved complex_math.c to libsrc from raypicker.
 *
 *     Revision 1.1.1.1  2004/03/31 18:43:27  michelle
 *     New Hydra Import
 *
 *
 *
 */

#include <math.h>
#include <complex_math.h>

/*****************************************************
 *                COMPLEX                            *
 *                                                   *
 * Accept two real numbers; Returns a complex number *
 *****************************************************/
Complex COMPLEX(const double r, const double i)
{
    Complex z;

    z.real = r;
    z.imag = i;
    return z;
}

/*****************************************************
 *                  Real                             *
 *                                                   *
 * Returns the real part of a complex number         *
 *****************************************************/
double Real(const Complex z)
{
    return z.real;
}

/*****************************************************
 *                  Imag                             *
 *                                                   *
 * Returns the imaginary part of a complex number    *
 *****************************************************/
double Imag(const Complex z)
{
    return z.imag;
}

/*****************************************************
 *                  Cadd                             *
 *                                                   *
 * Adds two complex numbers, returning a complex #   *
 *****************************************************/
Complex Cadd(const Complex z1, const Complex z2)
{
    Complex z3;

    z3.real = z1.real + z2.real;
    z3.imag = z1.imag + z2.imag;
    return z3;
}

/*****************************************************
 *                  Csub                             *
 *                                                   *
 * Adds two complex numbers, returning a complex #   *
 *****************************************************/
Complex Csub(const Complex z1, const Complex z2)
{
    Complex z3;

    z3.real = z1.real - z2.real;
    z3.imag = z1.imag - z2.imag;
    return z3;
}

/*****************************************************
 *                 Cmult                             *
 *                                                   *
 * Multiplies two complex numbers; returns a complex *
 *****************************************************/
Complex Cmult(const Complex z1, const Complex z2)
{
    Complex z3;

    z3.real = z1.real * z2.real - z1.imag * z2.imag;
    z3.imag = z1.imag * z2.real + z1.real * z2.imag;
    return z3;
}

/*****************************************************
 *                  Cdiv                             *
 *                                                   *
 * Divides two complex numbers, returning a complex  *
 *****************************************************/
Complex Cdiv(const Complex z1, const Complex z2)
{
    Complex z3;
    double r, denom;

    if (fabs(z2.real) >= fabs(z2.imag))
    {
      r = z2.imag/z2.real;
      denom = z2.real + r * z2.imag;
      z3.real = (z1.real + r * z1.imag)/denom;
      z3.imag = (z1.imag - r * z1.real)/denom;
    }
    else 
    {
      r = z2.real/z2.imag;
      denom = z2.imag + r * z2.real;
      z3.real = (z1.real * r + z1.imag)/denom;
      z3.imag = (z1.imag * r - z1.real)/denom;
    }
    return z3;
}

/*****************************************************
 *                  Cexp                             *
 *                                                   *
 * Returns the exponential e to the z,               *
 * where z = x + iy is a complex number              *
 * exp(z) = exp(x){cos(y) + isin(y)}                 *
 *****************************************************/
Complex Cexp(const Complex z)
{
    double ex = exp(z.real);
    double cosy = cos(z.imag);
    double siny = sin(z.imag);
    Complex a;
    Complex b;
    Complex c;

    a.real = ex; a.imag = 0.0;
    b.real = cosy; b.imag = siny;

    /* c.real = ex * cosy;
    c.imag = ex * siny;*/

    c = Cmult(a, b); 

    return c;
}

/*****************************************************
 *                  Conj                             *
 *                                                   *
 * Returns the conjugate of a complex number         *
 *****************************************************/
Complex Conj(const Complex z)
{
    Complex c;
   
    c.real = z.real;
    c.imag = -z.imag;
    return c;
}


/*****************************************************
 *                  Cabs                             *
 *                                                   *
 * Returns the absolute value of a complex number    *
 *****************************************************/
double Cabs(const Complex z)
{
    double x = fabs(z.real);
   
    if (x == 0.0)
    {
      return fabs(z.imag);
    }
    else
    {
      double y = fabs(z.imag);
      if (y == 0.0)
      {
          return x;
      }
      else if (y < x)
      {
          double tmp = y/x;
          return (x * sqrt(1.0 + tmp * tmp));
      }
      else
      {
          double tmp = x/y;
          return (y * sqrt(1.0 + tmp * tmp));
      }
    }
}

/*****************************************************
 *                  Cmod                             *
 *                                                   *
 * Returns the modulus of a complex number           *
 *****************************************************/
double Cmod(const Complex z)			
{
    double mod;

	mod = (z.real * z.real) + (z.imag * z.imag);
	mod = sqrt((double)mod);
	return(mod);

}

/*****************************************************
 *                 rcadd                             *
 *                                                   *
 * Adds a real and a complex number;                 *
 * returns a complex number                          *
 *****************************************************/
Complex rcadd(const double r, const Complex z)
{
    Complex c;

    c.real = r + z.real;
    c.imag = z.imag;
    return c;
}

/*****************************************************
 *                 rcsub                             *
 *                                                   *
 * Subtracts a complex number from a real number;    *
 * returns a complex number                          *
 *****************************************************/
Complex rcsub(const double r, const Complex z)
{
    Complex c;

    c.real = r - z.real;
    c.imag = -z.imag;
    return c;
}

/*****************************************************
 *                 rcmult                            *
 *                                                   *
 * Multiplies a real and a complex number;           *
 * returns a complex number                          *
 *****************************************************/
Complex rcmult(const double r, const Complex z)
{
    Complex c;

    c.real = r * z.real;
    c.imag = r * z.imag;
    return c;
}

/*****************************************************
 *                 rcdiv                             *
 *                                                   *
 * Divides a real by a complex number;               *
 * returns a complex number                          *
 *****************************************************/
Complex rcdiv(const double r, const Complex z)
{
    Complex c;
    double ratio, denom;

    if (fabs(z.real) >= fabs(z.imag))
    {
      ratio = z.imag/z.real;
      denom = z.real + ratio * z.imag;
      c.real = r/denom;
      c.imag = (-ratio * r)/denom;
    }
    else
    {
      ratio = z.real/z.imag;
      denom = z.imag + ratio * z.real;
      c.real = r * ratio/denom;
      c.imag = -r/denom;
    }
    return c;
}

Complex Csqrt( Complex z )
{
   Complex c;
   double    x, y, w, r;

   if ( (z.real == 0.0) && (z.imag == 0.0) ) 
   {
      c.real = 0.0;
      c.imag = 0.0;
      return c;
   } 
   else 
   {
      x = fabs (z.real);
      y = fabs (z.imag);
      if ( x >= y ) 
      {
         r = y / x;
         w = (sqrt (x) * sqrt (0.5 * (1.0 + sqrt (1.0 + r*r))));
      } 
      else 
      {
         r = x / y;
         w = (sqrt (y) * sqrt (0.5 * (r + sqrt (1.0 + r*r))));
      }
      if ( z.real >= 0.0 ) 
      {
         c.real = w;
         c.imag = (z.imag / (2.0*w));
      } 
      else 
      {
         c.imag = (z.imag >= 0) ? w : -w;
         c.real = (z.imag / (2.0*c.imag));
      }
      return c;
   }
}
