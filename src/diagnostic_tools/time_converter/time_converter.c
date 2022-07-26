
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <transport.h>
#include <swap.h>
#include <time_ew.h>
#include <chron3.h>

/* 
 * This is just a super simple command-line program to go from epoch time 
 * to human time, extracted from other Earthworm code
 * I find it useful when I'm reading log files where human time wasn't used
 *
 * - Stefan 20110711
 *
 * Example of a compile on OSX: 
 * source ew_macosx_intel.sh
 * cc $CFLAGS -L$EW_HOME/$EW_VERSION/lib -lchron3.o -ltime_ew.o -o \
 *    $EW_HOME/$EW_VERSION/bin/tc time_converter.c 
 *
 */

/**************************************************************************
 *  Define the structure for time records.                                *
 **************************************************************************/

typedef struct TStrct {
    double  Time1600; /* Time (Sec since 1600/01/01 00:00:00.00)          */
    double  Time;     /* Time (Sec since 1970/01/01 00:00:00.00)          */
    int     Year;     /* Year                                             */
    int     Month;    /* Month                                            */
    int     Day;      /* Day                                              */
    int     Hour;     /* Hour                                             */
    int     Min;      /* Minute                                           */
    double  Sec;      /* Second                                           */
} TStrct;

double    sec1970 = 11676096000.00;  /* # seconds between Carl Johnson's        */

void Decode_Time( double secs, TStrct *Time);
void Encode_Time( double *secs, TStrct *Time);

int main( int argc, char **argv ) {
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
/**********************************************************************
 * Decode_Time : Decode time from seconds since 1970                  *
 *                                                                    *
 **********************************************************************/
void Decode_Time( double secs, TStrct *Time)
{
  struct Greg    g;
  long    minute;
  double    sex;

  Time->Time = secs;
  secs += sec1970;
  Time->Time1600 = secs;
  minute = (long) (secs / 60.0);
  sex = secs - 60.0 * minute;
  grg(minute, &g);
  Time->Year  = g.year;
  Time->Month = g.month;
  Time->Day   = g.day;
  Time->Hour  = g.hour;
  Time->Min   = g.minute;
  Time->Sec   = sex;
}


/**********************************************************************
 * Encode_Time : Encode time to seconds since 1970                    *
 *                                                                    *
 **********************************************************************/
void Encode_Time( double *secs, TStrct *Time)
{
  struct Greg    g;

  g.year   = Time->Year;
  g.month  = Time->Month;
  g.day    = Time->Day;
  g.hour   = Time->Hour;
  g.minute = Time->Min;
  *secs    = 60.0 * (double) julmin(&g) + Time->Sec - sec1970;
}
