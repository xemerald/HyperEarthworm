/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: fft_prep.h 1108 2002-11-03 00:06:30Z lombard $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.3  2002/11/03 00:06:30  lombard
 *     Fixed RCS keywords
 *
 *
 *
 *
 */

/* Header for fft_prep.c, a collection of routines to prepare for
 * the use of Temperton FFT99 routines.
 */

#ifndef FFT_PREP_H
#define FFT_PREP_H

#include <fft99.h>

#define N_FAC 30
#define N_RADIX 3

/* Element of linked list of FFT factors and their trig structure */
typedef struct _FACT
{
  long nfft;
  long fact_power[N_RADIX];
  double *trigs;
  long *ifax;
  struct _FACT *next;
} FACT;


/* Public function prototypes */
long buildFacList(long n);
void trimFacList(long n);
long prepFFT( long n, FACT **pf );
void printFacList( );
void fftPrepDebug( int );

#endif
