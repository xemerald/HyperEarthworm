//---------------------------------------------------------------------------
#ifndef TimeFuncsH
#define TimeFuncsH
//---------------------------------------------------------------------------

#include <time.h>
#include <worm_statuscode.h>

//---------------------------------------------------------------------------

enum WORM_TIME_LOCALE
{
    WORM_GMT_TIME
  , WORM_LOCAL_TIME
};

enum WORM_TIME_FORMAT
{
    WORM_TIMEFMT_8     = 0  // YYYYMMDD
  , WORM_TIMEFMT_14    = 1  // YYYYMMDDhhmmss
  , WORM_TIMEFMT_18    = 2  // YYYYMMDDhhmmss.sss
  , WORM_TIMEFMT_UTC21 = 3  // YYYYMMDD_UTC_hh:mm:ss
  , WORM_TIMEFMT_16    = 4  // YYYY/MM/DD hh:mm
  , WORM_TIMEFMT_19    = 5  // YYYY/MM/DD hh:mm:ss
  , WORM_TIMEFMT_23    = 6  // YYYY/MM/DD hh:mm:ss.sss
  , WORM_TIMEFMT_26    = 7  // DOW MON DD HH:MM:SS YYYY\n
};

#define WORM_TIMESTR_LENGTH 26 // max length (with terminator) of above formats

typedef char WORM_DATETIMESTR[WORM_TIMESTR_LENGTH];

typedef unsigned long ULONG_TIME;

//
//  sec.nano_secs
//
typedef double HIGHRES_TIME;



//---------------------------------------------------------------------------
class TTimeFuncs
{
protected:

   static const short MONTH_STARTS[2][12];

public:

   static const char DOW[7][4]; // = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

   static const char MOY[12][4]; // = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

   // seconds since Midnight, Jan 1, 1970 with milliseconds as fraction portion
   static HIGHRES_TIME GetHighResTime();

   /*  DateString()
   **
   **  r_buffer must point to a buffer of at least WORM_TIMESTR_LENGTH length
   **  Locale defaults to GMT
   **
   */
   static char * DateString( char * r_buffer
                           , WORM_TIME_FORMAT p_format
                           , WORM_TIME_LOCALE p_locale = WORM_GMT_TIME
                           , double p_time = -1.0 // -1.0 means 'now'
                           );

   /*  TimeToDouble -- converts a string in format YYYYMMDDHHMMSS.sss
   **                  into a double representing seconds since Jan 1, 1970
   **
   */
   static double TimeToDouble( const char * p_buffer );

   static void MSecSleep( unsigned int p_msec );

   static bool IsLeapyear( unsigned int p_year );

   static bool YearJulianToMonDay( short   p_year
                                 , short   p_julday
                                 , short * r_monthnum
                                 , short * r_monthday
                                 );

   static bool CrackDate( const char * p_buffer          // "YYYYMMDD...."
                        , short      * r_year            // 1900 - n
                        , short      * r_monthnum        // 1 - 12
                        , short      * r_monthday        // 1 - 31
                        , short      * r_year2           // 0 - 99
                        , short      * r_julday   = NULL // 1 - 366
                        , bool       * r_isleap   = NULL
                        );
   // NOTE: a compile error on the NULL can indicate that "#include <string.h>
   //       is needed in the referencing class.

   static bool CrackTime( const char * p_buffer  // "HHMMSS.sss" or "........HHMMSS.sss"
                        , short      * p_hour
                        , short      * p_min
                        , short      * p_sec    = NULL
                        , short      * p_msec   = NULL
                        , double     * p_fpsec  = NULL
                        );
};

#endif
