
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: chron3.c 7748 2019-01-31 02:02:20Z stefan $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.9  2009/06/05 14:52:23  ilya
 *     Fixed month in tm_to_gregorian
 *
 *     Revision 1.8  2009/06/05 13:53:06  tim
 *     Add support for converting tm struct to gregorian struct, needed to convert epoch to julian
 *
 *     Revision 1.7  2007/03/28 13:10:03  paulf
 *     MAC OS X changes
 *
 *     Revision 1.6  2007/02/26 20:47:30  paulf
 *     yet another time_t warning (minor)
 *
 *     Revision 1.5  2004/10/19 23:03:03  dietz
 *     Fixed bug in date18 in which the padding 0 was being omitted
 *     for when the second field was less than 10.000.
 *
 *     Revision 1.4  2004/08/06 14:55:06  davidk
 *     Fixed bug in ew_t_to_Date17() where the hundredths of a second field was not
 *     being padded with zeroes and was getting tokenly separated from the
 *     main date value.
 *     Added #includes to get rid of "unknown function" warnings.
 *
 *     Revision 1.3  2004/08/05 22:52:50  davidk
 *     Added ew_t_to_Date17() and ew_Date17_to_t() functions to convert back and
 *     forth between a time_t-like value and a 17 char ascii string.
 *
 *     Revision 1.2  2004/04/29 21:35:55  dietz
 *     added date18, julsec18, epochsec18 functions
 *
 *     Revision 1.1  2000/02/14 18:51:48  lucky
 *     Initial revision
 *
 *
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <chron3.h>
#include <time_ew.h>
#include <math.h>

/*
   chron3.c : Time/date conversion routines.

   91May07 CEJ Version 1.0 - chron.c, chron.h

   Modified by W. Kohler, March 8, 1993.
   Source file name changed from chron.c to chron2.c.
   Include file name changed from chron.h to chron2.h.
   Routine datime added to calculate gregorian date and time from
   julian seconds.  Seconds added to structure Greg in file chron2.h.

   Modified by L. Dietz,  March 30, 1995.
   Source file name changed from chron2.c to chron3.c.
   Include file name changed from chron2.h to chron3.h.
   Routines date15 and julsec15 added to convert between time in
   julian seconds and character strings.
   Added a define statement to set the century (#define CENTURY 1900)

   Modified by L. Dietz, January 30, 1995.
   Added routine epochsec15 to convert from time given in a
   15-character string and seconds since 1970-01-01 00:00:00.0 GMT.

   Modified by L. Dietz, October, 1998.
   Changed make the library Y2K-compliant. 
   + Removed the CENTURY definition from chron3.h! 
   + Removed all functions that dealt with 2-digit-year date strings 
     and replaced them with 4-digit-year counterparts.
        date15     -> date17
        date18     -> date20
        julsec15   -> julsec17
        epochsec15 -> epochsec17
        timegm     -> timegm (fixed to call epochsec17)

   Modified by L. Dietz, November, 1998.
   Changed to make function MT-safe.  
   Eliminated the file-global "struct Greg G" workspace and had each function 
   declare its own "struct Greg" variable, if needed.  Changed function
   arguments for grg(), gregor(), and datime() to include a pointer to 
   struct Greg for returning information to calling function.

*/

/*********************C O P Y R I G H T   N O T I C E ***********************/
/* Copyright 1991 by Carl Johnson.  All rights are reserved. Permission     */
/* is hereby granted for the use of this product for nonprofit, commercial, */
/* or noncommercial publications that contain appropriate acknowledgement   */
/* of the author. Modification of this code is permitted as long as this    */
/* notice is included in each resulting source module.                      */
/****************************************************************************/

int mo[] = {   0,  31,  59,  90, 120, 151, 181, 212, 243, 273, 304, 334,
               0,  31,  60,  91, 121, 152, 182, 213, 244, 274, 305, 335};
char *cmo[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
               "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

/*
 * Round a double to the nearest with specified decimal values
 */
double round_decimal(double f, int decimal) {
        int i;
        long pow10 = 1;
        for(i=1; i <= decimal; i++) {
                pow10 *= 10;
        }
        return floor(f * (double) pow10 + 0.5) / (double) pow10;
}

/*
 * Calculate julian minute from gregorian date and time.
 */
long julmin( struct Greg *pg )
{
        return(1440L * (julian(pg) - 2305448L) + 60L * pg->hour + pg->minute);
}

/*
 * Calculate gregorian date and time from julian minutes.
 */
struct Greg *grg( long min, struct Greg *pg )
{
        long j;
        long m;

        j = min/1440L;
        m = min-1440L*j;
        j += 2305448L;
        gregor(j,pg);
        pg->hour = m/60;
        pg->minute = m - 60 * pg->hour;
        return(pg);
}

typedef struct
{
   char  MonName[10];
   short NumDaysNorm;
   short NumDaysLeap;
   short MonNorm;    // Julian date, first day of the month, normal year
   short MonLeap;    // Julian date  first day of the month, leap year
} MonthStruct;

int JulianDay (int, int, int);

/* Below are the values assigned to the "Months" structure that's used to get
 * the Julian date for JulianDay
 *---------------------------------------------------------------------------*/
MonthStruct Months[] =
{
    { "January"  , 31, 31,   1,   1 },
    { "February" , 28, 29,  32,  32 },
    { "March"    , 31, 31,  60,  61 },
    { "April"    , 30, 30,  91,  92 },
    { "May"      , 31, 31, 121, 122 },
    { "June"     , 30, 30, 152, 153 },
    { "July"     , 31, 31, 182, 183 },
    { "August"   , 31, 31, 213, 214 },
    { "September", 30, 30, 244, 245 },
    { "October"  , 31, 31, 274, 275 },
    { "November" , 30, 30, 305, 306 },
    { "December" , 31, 31, 335, 336 }
};



/*
 * julian : Calculate julian date from gregorian date.
 */
long julian( struct Greg *pg )
{
        long jul;
        int leap;
        int year;
        int n;

        jul = 0;
        year = pg->year;
        if(year < 1) goto x110;
        year--;
        jul = 365;

/* four hundred year rule */
        n = year / 400;
        jul += n * 146097L;
        year -= n * 400;

/* hundred year rule */
        n = year / 100;
        jul += n * 36524L;
        year -= n * 100;

/* four year rule */
        n = year / 4;
        jul += n * 1461L;
        year -= n * 4;

/* one year rule */
        jul += year * 365L;

/* Handle days in current year */
x110:
        leap = 0;
        if(pg->year % 4   == 0) leap = 12;
        if(pg->year % 100 == 0) leap = 0;
        if(pg->year % 400 == 0) leap = 12;
        jul += mo[pg->month + leap - 1] + pg->day + 1721060L;
        return(jul);
}

/*
 * gregor : Calculate gregorian date from julian date.
 */
struct Greg *gregor( long min, struct Greg *pg )
{
        long test;
        long check;
        int leap;
        int left;
        int imo;

        pg->year = (int)((double)(min - 1721061L) / 365L);
        pg->month = 1;
        pg->day = 1;
        test = julian(pg);
        if(test <= min) goto x110;

x20:
        pg->year--;
        test = julian(pg);
        if(test > min) goto x20;
        goto x210;

x105:
        pg->year++;
        test = julian(pg);

x110:
        check = test - min - 366L;
        if(check < 0) goto x210;
        if(check > 0) goto x105;

        if(pg->year % 400 == 0) goto x210;
        if(pg->year % 100 == 0) goto x105;
        if(pg->year %   4 == 0) goto x210;
        goto x105;

x210:
        left = min - test;
        leap = 0;
        if(pg->year %   4 == 0) leap = 12;
        if(pg->year % 100 == 0) leap = 0;
        if(pg->year % 400 == 0) leap = 12;
        for(imo=1; imo<12; imo++) {
                if(mo[imo+leap] <= left)
                        continue;
                pg->month = imo;
                pg->day = left - mo[imo+leap-1] + 1;
                return(pg);
        }
        pg->month = 12;
        pg->day = left - mo[11+leap] + 1;
        return(pg);
}

/*
 * date20 : Create 20 char date string in the form 1988Jan23 1234 12.21
 *          from the julian seconds.  Remember to leave space for the
 *          string termination (NULL).
 *          Replaces non-Y2K-compliant date18() function
 *          Added to chron3.c 10/28/98 by LDD.
 */
void date20( double secs, char *c20)
{
        struct Greg g;
        long minute;
        double sex;
        int hrmn;

        minute = (long) (secs / 60.0);
        sex = round_decimal(secs - 60.0 * (double) minute, 2);
        if(sex >= 60.0) {
                minute++;
                sex -= 60.0;
        }
        grg(minute, &g);
        hrmn = 100 * g.hour + g.minute;
        sprintf(c20, "%04d%3s%2d %4d%6.2f",
                g.year, cmo[g.month-1], g.day, hrmn, sex);
}

/*
 * tnow : Returns current system time for time stamping
 */
double tnow( void )
{
        struct Greg g;
/*      struct timeb q; */
        time_t tsecs;
        double secs;

        g.year   = 1970;
        g.month  = 1;
        g.day    = 1;
        g.hour   = 0;
        g.minute = 0;
/* original code by Carl; ftime() not supported on Sparc C compiler 3.0.1 */
/*      ftime(&q);                                              */
/*      secs = 60.0 * julmin(&g) + q.time +  0.001 * q.millitm; */
        time(&tsecs);                                   /*950501:ldd.*/
        secs = 60.0 * julmin(&g) + (double) tsecs;      /*950501:ldd.*/
        return secs;
}

/*
 * Calculate gregorian date and time from julian seconds.
 */
struct Greg *datime( double secs, struct Greg *pg )
{
        long j, m, min;

        min = (long) (secs / 60.0);
        j = min/1440L;
        m = min-1440L*j;
        j += 2305448L;
        gregor(j,pg);
        pg->hour = m/60;
        pg->minute = m - 60 * pg->hour;
        pg->second = (float) (secs - 60.0 * min);
        return(pg);
}

/*
 * date17 : Build a 17 char date string in the form 19880123123412.21
 *          from the julian seconds.  Remember to leave space for the
 *          string termination (NULL).
 *          Replaces the non-Y2K-compliant date15() function.
 *          Added to chron3.c on 10/28/98 by LDD
 */
void date17( double secs, char *c17 )
{
        struct Greg g;
        long minute;
        double sex;

        minute = (long) (secs / 60.0);
        sex = round_decimal(secs - 60.0 * (double) minute, 2);
        if(sex >= 60.0) {
                minute++;
                sex -= 60.0;
        }
        grg(minute,&g);
        sprintf(c17, "%04d%02d%02d%02d%02d%05.2f", 
                g.year, g.month, g.day, g.hour, g.minute, sex);
}


/*
 * date18 : Build a 18 char date string in the form 19880123123412.215
 *          from the julian seconds.  Remember to leave space for the
 *          string termination (NULL).
 *          Added to chron3.c on 4/29/2004 by LDD
 */
void date18( double secs, char *c18 )
{
        struct Greg g;
        long minute;
        double sex;

        minute = (long) (secs / 60.0);
        sex = round_decimal(secs - 60.0 * (double) minute, 3);
        if(sex >= 60.0) {
                minute++;
                sex -= 60.0;
        }
        grg(minute,&g);
        sprintf(c18, "%04d%02d%02d%02d%02d%06.3f",
                g.year, g.month, g.day, g.hour, g.minute, sex);
}


/*
 * julsec17 : Calculate time in julian seconds from a character string
 *            of the form 19880123123412.21
 *            Replaces the non-Y2K-compliant julsec15() function.
 *            Added to chron3.c on 10/28/98 by LDD
 */
double julsec17( char *c17 )
{
        struct Greg  g;
        double       jsecs;
        int          narg, i;
        int          isec, hsec;

/*** Make sure there are no blanks in the time part of the pick ***/
        for(i=0; i<17; i++)
        {
                if( c17[i] == ' ' )  c17[i] = '0';
        }

/***  Read character string  ***/
        narg = sscanf( c17, "%4d%2d%2d%2d%2d%2d.%2d",
                        &g.year, &g.month, &g.day,
                        &g.hour, &g.minute, &isec, &hsec);

        if ( narg < 7 )  return( 0.0 );


/***  Calculate julian seconds ***/
        jsecs   = 60.0 * (double) julmin(&g) +
                         (double) isec +
                         (double) hsec / 100.0;

        return( jsecs );
}


/*
 * julsec18 : Calculate time in julian seconds from a character string
 *            of the form 19880123123412.215
 *            Added to chron3.c on 4/29/2004 by LDD
 */
double julsec18( char *c18 )
{
        struct Greg  g;
        double       jsecs;
        int          narg, i;
        int          isec, thsec;

/*** Make sure there are no blanks in the time part of the pick ***/
        for(i=0; i<18; i++)
        {
                if( c18[i] == ' ' )  c18[i] = '0';
        }

/***  Read character string  ***/
        narg = sscanf( c18, "%4d%2d%2d%2d%2d%2d.%3d",
                        &g.year, &g.month, &g.day,
                        &g.hour, &g.minute, &isec, &thsec);

        if ( narg < 7 )  return( 0.0 );


/***  Calculate julian seconds ***/
        jsecs   = 60.0 * (double) julmin(&g) +
                         (double) isec +
                         (double) thsec / 1000.0;

        return( jsecs );
}


/*
 * epochsec17 :  Convert time in a character string form of
 *               ccyymmddhhmmss.ff (19880231010155.23) to
 *               seconds since 1970-01-01 00:00:00.0
 *               Replaces the non-Y2K-compliant epochsec15() function.
 *               Added to chron3.c on 10/28/98 by LDD
 */
int epochsec17( double *sec, char *tstr )
{
   double jsec;

   jsec = julsec17( tstr );
   if( jsec==0.0 )
   {
      *sec=0.0;
      return ( -1 );
   }

   *sec = jsec-GSEC1970;
   return ( 0 );
}


/*
 * epochsec18 :  Convert time in a character string form of
 *               ccyymmddhhmmss.fff (19880231010155.235) to
 *               seconds since 1970-01-01 00:00:00.0
 *               Added to chron3.c on 4/29/2004 by LDD
 */
int epochsec18( double *sec, char *tstr )
{
   double jsec;

   jsec = julsec18( tstr );
   if( jsec==0.0 )
   {
      *sec=0.0;
      return ( -1 );
   }

   *sec = jsec-GSEC1970;
   return ( 0 );
}


/*
 * timegm :  Convert time as a struct tm to seconds since 1970-01-01 00:00:00.0
 *           This function is equivalent to timegm() in SunOS 4.x.
 *           Added to chron3.c on 2/27/98 by WMK
 *           Modified to be Y2K compliant 10/28/98 by LDD
 */
#ifndef _MACOSX
time_t timegm( struct tm *tm )
{
   char   tstr[18];
   double dsec;

   sprintf( tstr, "%04d%02d%02d%02d%02d%02d.00",
            tm->tm_year + 1900,
            tm->tm_mon + 1,
            tm->tm_mday,
            tm->tm_hour,
            tm->tm_min,
            tm->tm_sec );

   epochsec17( &dsec, tstr );
   return( (time_t)dsec );
}
#endif /* _MACOSX - this func is stock with BSD unix! */



/*
 * ew_Date17_to_t : Calculate time in julian seconds from a character string
 *            of the form 19880123123412.21
 */
double ew_Date17_to_t(char *szDate17)
{
  struct tm stm;
  double dTimeOut, dSecs;


  memset(&stm, 0, sizeof(stm));

  if(!(szDate17 && strlen(szDate17) == 17))
    return(-1.0);  /* invalid input params */

  if(sscanf(szDate17, "%4d%2d%2d%2d%2d%5lf",
            &stm.tm_year, &stm.tm_mon, &stm.tm_mday, 
            &stm.tm_hour, &stm.tm_min, &dSecs)!=6)
    return(-1.0);

  stm.tm_year-=1900;
  stm.tm_mon-=1;

  dTimeOut = (double) timegm_ew(&stm);
  dTimeOut += dSecs;

  return(dTimeOut);
}  /* end ew_Date17_to_t() */

/*
 * ew_t_to_Date17 : Calculate Date17 string from julian seconds.
 */
char * ew_t_to_Date17(double dTime, char *szBuffer)
{
  struct tm stm;
  double dHSecs;
  time_t tTime;


  memset(&stm, 0, sizeof(stm));
  tTime = (time_t)dTime;
  dHSecs = dTime - tTime;
  gmtime_ew(&tTime, &stm);

  sprintf(szBuffer, "%04d%02d%02d%02d%02d%02d.%02d",
          stm.tm_year+1900, stm.tm_mon+1, stm.tm_mday, 
          stm.tm_hour, stm.tm_min, stm.tm_sec, (int)(100*dHSecs));

  return(szBuffer);
}  /* end ew_t_to_Date17() */

/*
 * tm_to gregorian : Convert tm struct to gregorian struct
 */
struct Greg *tm_to_gregorian(struct tm *tm_time, struct Greg *pg) {
  pg->year = tm_time->tm_year + 1900;
  pg->month = tm_time->tm_mon + 1;
  pg->day = tm_time->tm_mday;
  pg->hour = tm_time->tm_hour;
  pg->minute = tm_time->tm_min;
  return(pg);
}

/*
* Seems these time encoders and decoders written either by Jim Luetgert or
* Pete Lombard (see Heli_ewII.c) should be in some common place. I've
* grabbed them and dropped them here (using the GSEC1970 constant
* already defined here). - Stefan 20101102
*
*/

/**********************************************************************
* Decode_Time : Decode time from seconds since 1970                  *
*                                                                    *
**********************************************************************/
void cDecode_Time(double secs, TStruct *Time)
{
	struct Greg    g;
	long    minute;
	double    sex;

	Time->Time = secs;
	secs += GSEC1970;
	Time->Time1600 = secs;
	minute = (long)(secs / 60.0);
	sex = secs - 60.0 * minute;
	grg(minute, &g);
	Time->Year = g.year;
	Time->Month = g.month;
	Time->Day = g.day;
	Time->Hour = g.hour;
	Time->Min = g.minute;
	Time->Sec = sex;
}


/**********************************************************************
* Encode_Time : Encode time to seconds since 1970                    *
*                                                                    *
**********************************************************************/
void cEncode_Time(double *secs, TStruct *Time)
{
	struct Greg    g;

	g.year = Time->Year;
	g.month = Time->Month;
	g.day = Time->Day;
	g.hour = Time->Hour;
	g.minute = Time->Min;
	*secs = 60.0 * (double)julmin(&g) + Time->Sec - GSEC1970;
}

/* I used the above two functions in this tiny command line converter function all the time */
/*int main( int argc, char **argv ) {
double time;
TStrct timeout;
if ( argc != 2 && argc != 7 ) {
fprintf(stderr,"Usage:   %s <numeric time>\n", argv[0]);
fprintf(stderr,"Example: %s 1153971780.0\n", argv[0] );
fprintf(stderr,"Usage:   %s <year> <month> <day> <hour> <min> <sec>\n", argv[0]);
fprintf(stderr,"Example: %s  2006 07 27 03 43 00\n", argv[0] );
exit( 1 );
}
if (argc == 2) {
time  =  atof( argv[1] );
Decode_Time( time, &timeout );
} else {
timeout.Year = atoi ( argv[1] );
timeout.Month = atoi ( argv[2] );
timeout.Day = atoi ( argv[3] );
timeout.Hour = atoi ( argv[4] );
timeout.Min = atoi ( argv[5] );
timeout.Sec = atof ( argv[6] );
Encode_Time( &time, &timeout);

}
printf("%.4d %.2d %.2d %.2d:%.2d %.2f\n",
timeout.Year, timeout.Month, timeout.Day, timeout.Hour, timeout.Min, timeout.Sec);
printf("%f\n", time);
}
*/

