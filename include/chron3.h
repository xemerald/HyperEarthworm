
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: chron3.h 7654 2019-01-08 03:42:02Z stefan $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.4  2009/06/05 13:52:02  tim
 *     Add support for tm_to_gregorian function, which converts from tm struct to gregorian struct, needed to convert to julian date
 *
 *     Revision 1.3  2004/08/05 22:45:53  davidk
 *     Added ew_t_to_Date17() and ew_Date17_to_t() functions to convert back and
 *     forth between a time_t-like value and a 17 char ascii string.
 *
 *     Revision 1.2  2004/04/29 21:39:15  dietz
 *     added date18, julsec18, epochsec18
 *
 *     Revision 1.1  2000/02/14 20:05:54  lucky
 *     Initial revision
 *
 *
 */

/*
 *  chron3.h   include file for time-conversion routines
 */

#ifndef CHRON3_H
#define CHRON3_H

#include <time.h>

#define GSEC1970 11676096000.00 /* Gregorian seconds equivalent for 19700101 */
                                /* # seconds between Carl Johnson's chron3   */
                                /* time 0 and 1970-01-01 00:00:00.0 GMT      */

struct Greg {
        int year;
        int month;
        int day;
        int hour;
        int minute;
        float second;
};

/*  Function prototypes  */

void date20( double, char * );
void date17( double, char * );
void date18( double, char * );

struct Greg *datime( double, struct Greg * );  /*changed to be MT-safe */
struct Greg *gregor( long, struct Greg * );    /*changed to be MT-safe */
struct Greg *grg( long, struct Greg * );       /*changed to be MT-safe */

long julian( struct Greg * );
long julmin( struct Greg * );

double julsec17( char * );
double julsec18( char * );

int    epochsec17( double *, char * );
int    epochsec18( double *, char * );
time_t timegm( struct tm * );

double tnow( void );

/* ew_t_to_Date17() and ew_Date17_to_t() convert back
   and forth between epoch seconds (time_t as a double)
   and a 17 char date string  CCYYMMDDHHMMSS.SS
   The function utilize CRT and time_ew.c to implement
   the functionality.   DK 072804
   ew_Date17_to_t() is equivalent to epochsec().
   ew_t_to_Date17() is equivalent to Date17() except
   that it uses epoch seconds as input instead of Julian.
   dk 072804
********************************************************/
char * ew_t_to_Date17(double dTime, char *szBuffer);
double ew_Date17_to_t(char *szDate17);

// tm_to_gregorian converts from the tm struct to gregorian struct
struct Greg *tm_to_gregorian(struct tm *tm_time, struct Greg *pg);

/**************************************************************************
*  Define the structure for time records for Decode_Time and Encode_Time *
**************************************************************************/

typedef struct TStruct {
	double  Time1600; /* Time (Sec since 1600/01/01 00:00:00.00)          */
	double  Time;     /* Time (Sec since 1970/01/01 00:00:00.00)          */
	int     Year;     /* Year                                             */
	int     Month;    /* Month                                            */
	int     Day;      /* Day                                              */
	int     Hour;     /* Hour                                             */
	int     Min;      /* Minute                                           */
	double  Sec;      /* Second                                           */
} TStruct;

/* Jim or Pete's functions */
void cDecode_Time(double secs, TStruct *Time);
void cEncode_Time(double *secs, TStruct *Time);

/* Duff's function */
int JulianDay(int StaYear, int StaMonth, int StaDay);

/* CodeCogs */
int dayOfYear(int);

#endif

