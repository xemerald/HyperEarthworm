
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: time_ew.h 6858 2016-10-28 17:47:35Z kevin $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.4  2009/06/15 18:40:07  paulf
 *     added in windows version of gettimeofday
 *
 *     Revision 1.3  2004/07/13 01:12:00  davidk
 *     Added comment for timegm_ew().
 *     Added function utc_ctime_ew() w/comment.
 *
 *     Revision 1.2  2001/07/01 22:09:26  davidk
 *     Added prototype for datestr23_local().
 *
 *     Revision 1.1  2000/02/14 20:05:54  lucky
 *     Initial revision
 *
 *
 */


     /********************************************************
      *                      time_ew.h                       *
      *                                                      *
      *  Include file for the earthworm multi-thread safe    *
      *  versions of time routines.                          *
      *  Note: #include <time.h>    must be placed before    *
      *        #include <time_ew.h> in each source file      *
      *  using the earthworm time functions.                 *
      ********************************************************/

#ifndef TIME_EW_H
#define TIME_EW_H

#include <time.h>
#ifdef _WINNT                /* if Windows then */
#include <winsock2.h>        /* include for definition of 'timeval' */
#endif

/* Function prototypes
 *********************/
time_t     timegm_ew   ( struct tm * );
struct tm *localtime_ew( const time_t *, struct tm * );
char      *ctime_ew    ( const time_t *,    char *, int );
char      *asctime_ew  ( const struct tm *, char *, int );
double     hrtime_ew   ( double * );
char      *datestr23   ( double, char *, int );
char *datestr23_local  ( double t, char *pbuf, int len );

/* gmtime_ew() converts time_t to struct tm using UTC */
struct tm *gmtime_ew   ( const time_t *, struct tm * ); 

/* utc_ctime_ew() prints time_t UTC */
char      *utc_ctime_ew(time_t * pTime);


#define DATESTR23  23   /* length of string required by datestr23() */

#ifdef _WINNT
/* added in a gettimeofday() function for Windows - used by new sniffwave */
/*
 * http://social.msdn.microsoft.com/forums/en-US/vcgeneral/thread/430449b3-f6dd-4e18-84de-eebd26a8d668
 */

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

struct timezone
{
      int  tz_minuteswest; /* minutes W of Greenwich */
        int  tz_dsttime;     /* type of dst correction */
};

int gettimeofday(struct timeval *tv, struct timezone *tz);
#endif


#endif
