/* THIS FILE IS UNDER CVS - DO NOT MODIFY UNLESS YOU CHECKED IT OUT!
 *
 *  $Id: rw_coda_aav.c 3596 2009-05-19 22:24:48Z dietz $
 * 
 *  Revision history:
 *   $Log$
 *   Revision 1.1  2009/05/19 22:24:48  dietz
 *   Added rw_coda_aav.c for processing TYPE_CODA_AAV messages
 *
 */

/* rw_coda_aav.c
 *
 * Contains functions in that convert from a
 * TYPE_CODA_AAV message to a structure and visa versa.
 *
 * written by Lynn Dietz   April, 2009
 */

#include <stdio.h>
#include <string.h>
#include <chron3.h>
#include <earthworm.h>
#include "rw_coda_aav.h"

/********************************************************************
 * wr_coda_aav()                                                    *
 * Builds ascii TYPE_CODA_AAV message from a SCNL_CAAV struct       *
 * Returns: 1 on success                                            *
 *          0 on failure                                            *
 ********************************************************************/
int wr_coda_aav( char *msg, int maxlen, SCNL_CAAV *ch )
{
   int rc;

   memset( msg, 0, (size_t)maxlen );

   rc = snprintf( msg, (size_t)maxlen, 
                 "%s.%s.%s.%s %.3lf %.3lf %d %.2f\n",
                  ch->site, ch->comp, ch->net, ch->loc,
                  ch->caav.tstart, ch->caav.tend,
                  ch->caav.amp, ch->caav.completeness );
   if( rc >= maxlen )
   {
      logit( "","wr_coda_aav: ERROR msg buffer too short, must be %d char\n",
             rc+1 );
      memset( msg, 0, (size_t)maxlen );
      return 0;
   }
   else if( rc < 0 )
   {
      logit( "","wr_coda_aav: ERROR in snprintf while writing msg\n" );
      memset( msg, 0, (size_t)maxlen );
      return 0;
   }
   return 1;
}


/********************************************************************
 * rd_coda_aav()                                                    *
 * Reads ascii TYPE_CODA_AAV message and fills in SCNL_CAAV struct  *
 * Returns: 1 on success                                            *
 *          0 on failure                                            *
 ********************************************************************/
int rd_coda_aav( char *msg, SCNL_CAAV *ch )
{
   int rc;

   memset( ch, 0, sizeof(SCNL_CAAV) );

   rc = sscanf( msg, "%[^.].%[^.].%[^.].%s %lf %lf %d %f",
                ch->site, ch->comp, ch->net, ch->loc,
                &ch->caav.tstart, &ch->caav.tend,
                &ch->caav.amp, &ch->caav.completeness );
   if( rc != 8 ) return 0;
   else          return 1;
}

/********************************************************************
 * log_coda_aav()  writes SCNL_CAAV struct to earthworm log file    *
 ********************************************************************/
void log_coda_aav( SCNL_CAAV *ch )
{
   char ststart[20];
   char stend[20];

   date18( GSEC1970+ch->caav.tstart, ststart );
   date18( GSEC1970+ch->caav.tend,   stend   );

   logit( "", "%s.%s.%s.%s %s %s %d %.2f\n",
          ch->site, ch->comp, ch->net, ch->loc,
          ststart, stend,
          ch->caav.amp, ch->caav.completeness );
   return;
}

