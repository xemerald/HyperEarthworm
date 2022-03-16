/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: complex_math.h 7345 2018-04-18 03:55:20Z baker $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2006/01/30 19:41:10  friberg
 *     added in hydra includes for raypicker module
 *
 *     Revision 1.2  2005/08/30 16:06:16  cjbryan
 *     added Cmod prototype
 *
 *     Revision 1.1.1.1  2005/06/22 19:30:36  michelle
 *     new directory tree built from files in HYDRA_NEWDIR_2005-06-20 tagged hydra and earthworm projects
 *
 *     Revision 1.3  2004/04/14 21:52:46  davidk
 *     Added a newline to the end of the file to get rid of a warning msg.
 *
 *     Revision 1.2  2004/04/09 23:22:03  davidk
 *     Added Csub() subtraction function.
 *
 *     Revision 1.1  2004/04/09 23:04:56  davidk
 *     Moved complex_math.h to include from raypicker.
 *
 *     Revision 1.1.1.1  2004/03/31 18:43:27  michelle
 *     New Hydra Import
 *
 *
 *   Functions for performing complex arithmetic
 */

#ifndef COMPLEX_MATH_H
#define COMPLEX_MATH_H

typedef struct {
		double real;
		double imag;
} Complex; 

Complex COMPLEX(const double r, const double i);
double Real(const Complex z);
double Imag(const Complex z);
Complex Cadd(const Complex z1, const Complex z2);
Complex Csub(const Complex z1, const Complex z2);
Complex Cmult(const Complex z1, const Complex z2);
Complex Cdiv(const Complex z1, const Complex z2);
Complex Cexp(const Complex z);
Complex Conj(const Complex z);
double Cabs(const Complex z);
double Cmod(const Complex z);
Complex Csqrt( Complex z );
Complex rcadd(const double r, const Complex z);
Complex rcsub(const double r, const Complex z);
Complex rcmult(const double r, const Complex z);
Complex rcdiv(const double r, const Complex z);

#endif  /*  COMPLEX_MATH_H */
