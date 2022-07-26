
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: ahhead.h 6803 2016-09-09 06:06:39Z et $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.3  2001/04/12 03:03:08  lombard
 *     reorgainized to put the format-specific header items in *head.h
 *     and the putaway-specific items in *putaway.h, of which there
 *     currently is only sacputaway.h
 *     Function protoypes used by putaway.c moved to pa_subs.h
 *
 *     Revision 1.2  2000/03/09 21:54:36  davidk
 *     Added function prototypes for the ah putaway routines, so that
 *     users of the ahputaway routines would not get compile warnings.
 *
 *     Revision 1.1  2000/02/14 20:05:54  lucky
 *     Initial revision
 *
 *
 */

#ifndef AHHEAD_H
#define AHHEAD_H

#define AHUNDEF        -12345  /* undefined value -- withers 8/98*/


/*      structure for data file header  --      witte, 11 June 85       */

#define AHHEADSIZE 1024
#define CODESIZE 6
#define CHANSIZE 6
#define STYPESIZE 8
#define COMSIZE 80
#define TYPEMIN 1
#define TYPEMAX 6
#define LOGSIZE 202
#define LOGENT 10
#define NEXTRAS 21
#define NOCALPTS 30

typedef struct {
  float x;
  float y;
} vector;

typedef struct {
  float r;
  float i;
} complex;

typedef struct {
  double r;
  double i;
} d_complex;

typedef struct {
  float xx;
  float yy;
  float xy;
} tensor;

struct ah_time {
  short         yr;     /* year         */
  short         mo;     /* month        */
  short         day;    /* day          */
  short         hr;     /* hour         */
  short         mn;     /* minute       */
  float         sec;    /* second       */
};

struct calib {
  complex               pole;   /* pole         */
  complex               zero;   /* zero         */
};

struct station_info {
  char          code[CODESIZE]; /* station code         */
  char          chan[CHANSIZE]; /* lpz,spn, etc.        */
  char          stype[STYPESIZE];/* wwssn,hglp,etc.     */
  float         slat;           /* station latitude     */
  float         slon;           /*    "    longitude    */
  float         elev;           /*    "    elevation    */
  float         DS;     /* gain */
  float         A0;     /* normalization */
  struct        calib   cal[NOCALPTS];  /* calibration info     */
};

struct event_info {
  float         lat;            /* event latitude       */
  float         lon;            /*   "   longitude      */
  float         dep;            /*   "   depth          */
  struct        ah_time ot;             /*   "   origin time    */
  char          ecomment[COMSIZE];      /*      comment line    */
};

struct record_info {
  short         type;   /* data type (int,float,...)    */
  int32_t       ndata;  /* number of samples            */
  float         delta;  /* sampling interval            */
  float         maxamp; /* maximum amplitude of record  */
  struct        ah_time abstime;/* start time of record section */
  float         rmin;   /* minimum value of abscissa    */
  char          rcomment[COMSIZE];      /* comment line         */
  char          log[LOGSIZE]; /* log of data manipulations */
};

typedef struct {
  struct        station_info    station;        /* station info */
  struct        event_info      event;          /* event info   */
  struct        record_info     record;         /* record info  */
  float         extra[NEXTRAS]; /* freebies */
} ahhed;


#define FLOAT   1
#define COMPLEX 2
#define VECTOR  3
#define TENSOR  4
#define DOUBLE  6


#endif
