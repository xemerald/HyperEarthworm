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

/* transferFunction.c: Routines for dealing with instrument transfer functions */
#include <stdio.h>
#include <stdlib.h>
#include <transferFunction.h>


/*****************************************
 * cleanRF                               *
 *                                       *
 * zero a response structure             *
 *****************************************/
void cleanRF(ResponseFnStruct *rf)
{
    if (rf->gain != (double *)NULL)
    {
        free(rf->gain);
        rf->gain = (double *)NULL;
    }
    rf->numChanGains = 0;
    cleanTF(&rf->transferFn);
}

/*****************************************
 * cleanRTF                              *
 *                                       *
 * zero a transfer function structure    *
 *****************************************/
void cleanTF(TransferFnStruct *tf)
{
    if (tf->Zeroes != (Complex *)NULL)
    {
        free(tf->Zeroes);
        tf->Zeroes = (Complex *)NULL;
    }

    if (tf->Poles != (Complex *)NULL)
    {
        free(tf->Poles);
        tf->Poles = (Complex *)NULL;
    }

    tf->numPoles = 0;
    tf->numZeroes = 0;
    tf->normConstant = 0;
    tf->tfFreq = 0;

    return;
}
