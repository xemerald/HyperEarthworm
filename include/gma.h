/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: gma.h 879 2002-02-28 17:03:21Z lucky $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2002/02/28 17:03:21  lucky
 *     Initial revision
 *
 *
 */
/* gma.h: header for gma.c */

#ifndef GMA_H
#define GMA_H

#include <transfer.h>

/* Function prototypes: */
int gma(double *, long, double, ResponseStruct *, double *, double,
        long *, long *, double *, int, double *, int, double *, long,
        double *, double *);
void gmaDebug( int );


#endif




