/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id:
 *
 *    Revision history:
 *     $Log:
 *
 *
 *
 */

/* transferFunction.h: header for transferFunction.c */

#ifndef TRANSFERFUNCTION_H
#define TRANSFERFUNCTION_H

#include <complex_math.h>

#ifndef MAX_PZ
#define MAX_PZ 25 /* max number of poles and zeroes in transfer function */
#endif

/* The structures describing transfer functions in pole-zero-gain form. */

typedef struct _TransferFnStruct
{
    double      normConstant;    /* normalization constant 
                                  * for transfer fn         */
    double      tfFreq;          /* frequency for this 
                                  * transfer function       */
    int         numPoles;        /* number of poles         */
    int         numZeroes;       /* number of zeroes        */
    Complex    *Poles;           /* poles                   */
    Complex    *Zeroes;          /* zeroes                  */
} TransferFnStruct;

/* Instrument response includes channel gain in addition to 
 * transfer function */
typedef struct _ResponseFnStruct
{
    int                 numChanGains;   /* number of channels for this instrument */
    double             *gain;           /* channel gain  */
    TransferFnStruct    transferFn;     /* transfer function for this instrument */
} ResponseFnStruct;



/* Function prototypes: */
void cleanRF(ResponseFnStruct *rf);
void cleanTF(TransferFnStruct *tf);

#endif /* TRANSFERFUNCTION_H */




