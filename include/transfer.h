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

/* transfer.h: header for transfer.c */

#ifndef TRANSFER_H
#define TRANSFER_H

/* 
 * The structures describing transfer functions in pole-zero-gain form.
 * These are nearly identical to the structures in sachead.h but with
 * different names. Also, here we use pointers to the Poles and Zeros
 * arrays which are not pre-allocated; in case we want more than sac.
 * Perhaps the sachead structures can be changed to use these names.
 */

typedef struct _PZNum
{
  double      dReal;
  double      dImag;
} PZNum;

typedef struct _ResponseStruct
{
  double    dGain;
  int         iNumPoles;
  int         iNumZeros;
  PZNum  *Poles;
  PZNum  *Zeros;
} ResponseStruct;

/* Debug levels */
/*                poles. zeros, and gain */
#define TR_DBG_PZG   1<<0
/*                Trial response function */
#define TR_DBG_TRS   1<<1
/*                Actual response function */
#define TR_DBG_ARS   1<<2


/* Function prototypes: */
void response(long, double, ResponseStruct *, double *, double *);
int readPZ( char *, ResponseStruct * );
double ftaper(double, double, double);
void taper(double *, long, long);
int convertWave(double *, long, double, ResponseStruct *, ResponseStruct *, 
                double *, int, long *, long *, double *, long, double *, 
                double *);
int respLen( ResponseStruct *, double, double *);
void deMean( double *, long, double *);
void cleanPZ( ResponseStruct *);
void pzCancel(ResponseStruct *, double);
void transferDebug( int );
void setResponseInMeters( int );
#endif




