/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: fft99.h 517 2001-04-12 03:11:07Z lombard $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2001/04/12 03:11:07  lombard
 *     Initial revision
 *
 *
 *
 */

/* Header file for fft99.c, Temperton's FFT99 package */

#ifndef FFT99_H
#define FFT99_H

#define PI 3.14159265358979323846
/* Number of extra words needed for fft data storage */
#define FFT_EXTRA 2

/* Function prototypes */
void cfft99(double *a, double *work, double *trigs, long *ifax, long inc, 
            long jump, long n, long lot, long isign);
void cftfax(long n, long *ifax, double *trigs);
void fft99(double *a, double *work, double *trigs, long *ifax, long inc, 
           long jump, long n, long lot, long isign);
void fft991(double *a, double *work, double *trigs, long *ifax, long inc, 
            long jump, long n, long lot, long isign);
void fftfax(long n, long *ifax, double *trigs);



#endif
