/*
**
*/
#include <global_msg.h>
#include <time_ew.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>   /* floor() */

/* ---------------------------------------------------------------------- */

double DTStringToTime( const char * p_datestring )
{
    struct tm _timestruct;
    double tConvertedTime;

    time_t _epochtime; /* secs since 1/1/1970 */

    char _wrk[5];

    strncpy( _wrk, p_datestring, 4 );
    _wrk[4] = '\0';
    _timestruct.tm_year = atoi(_wrk) - 1900;

    strncpy( _wrk, p_datestring + 4, 2 );
    _wrk[2] = '\0';
    _timestruct.tm_mon  = atoi(_wrk) - 1;

    strncpy( _wrk, p_datestring + 6, 2 );
    _wrk[2] = '\0';
    _timestruct.tm_mday = atoi(_wrk);

    strncpy( _wrk, p_datestring + 8, 2 );
    _wrk[2] = '\0';
    _timestruct.tm_hour = atoi(_wrk);

    strncpy( _wrk, p_datestring + 10, 2 );
    _wrk[2] = '\0';
    _timestruct.tm_min  = atoi(_wrk);

    strncpy( _wrk, p_datestring + 12, 2 );
    _wrk[2] = '\0';
    _timestruct.tm_sec  = atoi(_wrk);

    _timestruct.tm_isdst = -1;

    _epochtime = timegm_ew(&_timestruct);

    strncpy( _wrk, p_datestring + 14, 4 );
    _wrk[4] = '\0';

    tConvertedTime = (double)_epochtime + atof(_wrk);

    return(tConvertedTime);
}

/* ---------------------------------------------------------------------- */
int TimeToDTString( const double p_time, char * p_buffer )
{
   time_t _ltime = (int)p_time;

   struct tm * _tmptr;

   if ( p_buffer == NULL )
   {
      return GLOBAL_MSG_NULL;
   }

   _tmptr = gmtime( &_ltime );

    sprintf( p_buffer
          , "%04d%02d%02d%02d%02d%02d.%03d"
          , _tmptr->tm_year + 1900
          , _tmptr->tm_mon  + 1
          , _tmptr->tm_mday
          , _tmptr->tm_hour
          , _tmptr->tm_min
          , _tmptr->tm_sec
          , (int)((p_time - floor(p_time)) * 1000.0)
          );

   return GLOBAL_MSG_SUCCESS;
}


/* ---------------------------------------------------------------------- */

GLOBAL_MSG_STATUS EncodeAuthor( MSG_LOGO   p_logo
                              , char     * r_buffer
                              )
{
   if ( r_buffer == NULL )
   {
      return GLOBAL_MSG_NULL;
   }
   sprintf( r_buffer
          , "%03u%03u%03u"
          , p_logo.type
          , p_logo.mod
          , p_logo.instid
          );

   return GLOBAL_MSG_SUCCESS;
}

/* ------------------------------------------------------------------------- */

GLOBAL_MSG_STATUS DecodeAuthor( MSG_LOGO * p_logo
                              , char     * r_buffer
                              )
{
   char _wrk[4];
   short _value;

   if ( r_buffer == NULL )
   {
      return GLOBAL_MSG_NULL;
   }

   if ( strlen( r_buffer ) != 9 )
   {
      return GLOBAL_MSG_BADPARAM;
   }

   strncpy( _wrk, r_buffer, 3 );
   _wrk[3] = '\0';

   if ( (_value = atoi(_wrk)) == 0 )
   {
      return GLOBAL_MSG_FORMATERROR;
   }

   p_logo->type = (unsigned char)_value;


   strncpy( _wrk, r_buffer + 3, 3 );
   _wrk[3] = '\0';

   if ( (_value = atoi(_wrk)) == 0 )
   {
      return GLOBAL_MSG_FORMATERROR;
   }

   p_logo->mod = (unsigned char)_value;


   strncpy( _wrk, r_buffer + 6, 3 );
   _wrk[3] = '\0';

   if ( (_value = atoi(_wrk)) == 0 )
   {
      return GLOBAL_MSG_FORMATERROR;
   }

   p_logo->instid = (unsigned char)_value;

   return GLOBAL_MSG_SUCCESS;
}
/* ------------------------------------------------------------------------- */

const char * GetGlobalAmpTypeName( MAGNITUDE_TYPE p_type )
{
   if ( p_type < 0 || MAGTYPE_COUNT <= p_type )
   {
      return MagNames[MAGTYPE_UNDEFINED];
   }                       
   return MagNames[p_type];
}

/* ---------------------------------------------------------------------- */

