/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: make_triglist.c 7113 2018-02-14 21:59:53Z baker $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.5  2007/03/28 20:54:26  paulf
 *     added MACOSX flag for using / instead of \
 *
 *     Revision 1.4  2006/03/10 13:49:50  paulf
 *     minor linux related fixes to removing _SOLARIS from the include line
 *
 *     Revision 1.3  2001/07/01 22:11:36  davidk
 *     Added a comment about another comment.
 *
 *     Revision 1.2  2000/05/02 19:45:32  lucky
 *     Cosmetic fixes (define extern fns) to make NT compile without warnings.
 *
 *     Revision 1.1  2000/02/14 18:51:48  lucky
 *     Initial revision
 *
 *
 */

  /************************************************************
   *                          writetrig.c                     *
   *                                                          *
   *         Functions for maintaining trigger files.         *
   *                  (based on logit.c)                      *
   *                                                          *
   *     First, call writetrig_Init.  Then, call writetrig.   *
   *     Call writetrig_close before exitting!                *
   *                                                          *
   *       These functions are NOT MT-Safe, since they store  *
   *     data in static buffers: PNL, 12/8/98                 *
   ************************************************************/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "time_ew.h"
#include "earthworm.h"
#include "transport.h"
#include "earthworm_simple_funcs.h"


static FILE   *fp;
static char   date[9];
static char   date_prev[9];
static time_t now;
static char   trigName[100];
static char   trigfilepath[70];
static char   template[25];
static struct tm res;
static int    Init  = 0;        /* 1 if writetrig_Init has been called */
static int    Disk  = 1;        /* 1 if output goes to Disk file       */

extern	void date17 (double, char *);

/* Functions in make_triglist.c */
int writetrig_init( char*, char* );
int writetrig( char*, char*, char* );
void writetrig_close(void);
void bldtrig_head( char*, double , char* , char* );
void bldtrig_phs( char *trigmsg, char* sta, char* comp, char* net, 
		  char ph, double pickTime, double saveStart, double durSave);
char  *make_datestr( double, char * );


/*************************************************************************
 *                             writetrig_Init                            *
 *                                                                       *
 *      Call this function once before the other writetrig routines.     *
 *                                                                       *
 *************************************************************************/
int writetrig_init( char* trigFileBase, char* outputDir )
{
   char *str;
   char baseName[50];
   int  lastchar;

/* Set time zone using the TZ environmental variable.
   This is not required under Solaris.
   In OS2 v2 or v3.0, use _tzset().
   In OS2 v3.0, use tzset().
   *************************************************/
#if defined(_OS2) || defined(_WINNT)
   if ( getenv( "TZ" ) != NULL ) _tzset();
#endif

/* Make sure we should write a trigger file
   ****************************************/
   if     ( strncmp(trigFileBase, "none",4)==0 )  Disk=0;
   else if( strncmp(trigFileBase, "NONE",4)==0 )  Disk=0;
   else if( strncmp(trigFileBase, "None",4)==0 )  Disk=0;
   else if( strncmp(outputDir,    "none",4)==0 )  Disk=0;
   else if( strncmp(outputDir,    "NONE",4)==0 )  Disk=0;
   else if( strncmp(outputDir,    "None",4)==0 )  Disk=0;
   if( Disk==0 ) return( 0 );

/* Truncate everything beyond and
   including "." in the base file name
   ***********************************/
   strcpy( baseName, trigFileBase );
   str = strchr( baseName, '.' );
   if ( str != NULL ) *str = '\0';

/* Check Init flag
   ***************/
   if( Init ) return( 0 );
   Init = 1;

/* Get path & base file name from config-file parameters
   *****************************************************/
   strcpy ( trigfilepath, outputDir );
   lastchar = strlen(outputDir)-1;

#if defined(_OS2) || defined(_WINNT)
   if( outputDir[lastchar] != '\\' &&  outputDir[lastchar] != '/' )
      strcat( trigfilepath, "\\" );
#endif
#if defined(_SOLARIS) || defined(_LINUX) || defined(_MACOSX)
   if( outputDir[lastchar] != '/' ) strcat( trigfilepath, "/" );
#endif

   sprintf( template, "%s.trg_", baseName );

/* Build trigger file name by appending time
   *****************************************/
   time( &now );
   gmtime_ew( &now, &res );
   sprintf( date, "%04d%02d%02d", (res.tm_year+1900), (res.tm_mon+1),
            res.tm_mday );

   strcpy( trigName,  trigfilepath );
   strcat( trigName,  template );
   strcat( trigName,  date );
   strcpy( date_prev, date );

/* Open trigger list file
   **********************/
   fp = fopen( trigName, "a" );
   if ( fp == NULL )
   {
      logit("e",
            "make_triglist: Error opening triglist file <%s>\n",
             trigName );
      return( -1 );
   }

/* Print startup message to trigger file
   *************************************/
   fprintf( fp, "\n-------------------------------------------------\n" );
   fprintf( fp, "make_triglist: startup at UTC_%s_%02d:%02d:%02d",
                 date, res.tm_hour, res.tm_min, res.tm_sec );
   fprintf( fp, "\n-------------------------------------------------\n" );
   fflush ( fp );

/* Log a warning message
   *********************/
#if defined(_OS2) || defined(_WINNT)
   if ( getenv( "TZ" ) == NULL )
   {
      writetrig("WARNING: The TZ environmental variable is not set.\n", trigFileBase, outputDir );
      writetrig("         Roll-over dates of trigger files may be bogus.\n", trigFileBase, trigFileBase );
   }
#endif

   return( 0 );
}


/*****************************************************************
 *                            writetrig                          *
 *                                                               *
 *          Function to log a message to a Disk file.            *
 *                                                               *
 *  flag: A string controlling where output is written:          *
 *        If any character is 'e', output is written to stderr.  *
 *        If any character is 'o', output is written to stdout.  *
 *        If any character is 't', output is time stamped.       *
 *                                                               *
 *  The rest of calling sequence is identical to printf.         *
 *****************************************************************/
/* The comment above doesn't seem to have any relevance to writetrig()
   DK 06/28/2001 */


int writetrig( char *note, char* filename, char* outDir )
{
   int rc;

/* Check Init flag
   ***************/
   if ( !Init )
   {
     rc = writetrig_init(filename, outDir);
     if( rc != 0 ) return( rc );
   }
   if ( !Disk ) return( 0 );

/* Get current system time
   ***********************/
   time( &now );
   gmtime_ew( &now, &res );

/* See if the date has changed.
   If so, create a new trigger file.
   *********************************/
   sprintf( date, "%04d%02d%02d", (res.tm_year+1900), (res.tm_mon+1),
            res.tm_mday );

   if ( strcmp( date, date_prev ) != 0 )
   {
      fprintf( fp,
              "UTC date changed; trigger output continues in file <%s%s>\n",
               template, date );
      fclose( fp );
      strcpy( trigName, trigfilepath );
      strcat( trigName, template );
      strcat( trigName, date );
      fp = fopen( trigName, "a" );
      if ( fp == NULL )
      {
         fprintf( stderr, "Error opening trigger file <%s%s>!\n",
                  template, date );
         return( -1 );
      }
      fprintf( fp,
              "UTC date changed; trigger output continues from file <%s%s>\n",
               template, date_prev );
      strcpy( date_prev, date );

/* Send a warning message to the new log file
   ******************************************/
#if defined(_OS2) || defined(_WINNT)
      if ( getenv( "TZ" ) == NULL )
      {
         fprintf( fp, "WARNING: The TZ environmental variable is not set.\n" );
         fprintf( fp, "         Roll-over dates of trigger files may be bogus.\n" );
      }
#endif
   }

/* write the message to the trigger file
 ***************************************/
   fprintf( fp, "%s", note );
   fflush( fp );

   return( 0 );
}

void writetrig_close()
{
   fclose( fp );
   return;
}
/***************************************************************************/

/* Routines for creating a trig_list message */

/**************************************************************
 * bldtrig_head() builds the EVENT line of a trigger message   *
 * Modified for author id by alex 7/10/98                     *
 **************************************************************/
/** length of string required by make_datestr  **/
#define	DATESTR_LEN		22	
#define	PHASE_STR		200	
void bldtrig_head( char* trigmsg, double otime, char* evId, char* author)
{
   char datestr[DATESTR_LEN];

/* Sample EVENT line for trigger message:
EVENT DETECTED     970729 03:01:13.22 UTC EVENT ID:123456 AUTHOR: asdf:asdf\n
0123456789 123456789 123456789 123456789 123456789 123456789
************************************************************/
   make_datestr( otime, datestr );
   sprintf( trigmsg, "EVENT DETECTED     %s UTC EVENT ID: %s AUTHOR: %s  \n\n", 
			datestr, evId, author);
   strcat ( trigmsg, "Sta/Cmp/Net   Date   Time                       start save       duration in sec.\n" );
   strcat ( trigmsg, "-----------   ------ ---------------    ------------------------------------------\n");

   return;
}

/****************************************************************
 * bldtrig_phs() builds the "phase" lines of a trigger message  *
 ****************************************************************/
void bldtrig_phs( char *trigmsg, char* sta, char* comp, char* net, char ph, double pickTime, 
		  double saveStart, double durSave)
{
   char str[PHASE_STR];
   char pckt_str[DATESTR_LEN];
   char savet_str[DATESTR_LEN];

/* Convert times in seconds since 1600 to character strings
 **********************************************************/
   make_datestr( pickTime, pckt_str );
   make_datestr( saveStart, savet_str );

/* Build the "phase" line!  Here's a sample:
 MCM VHZ NC N 19970729 03:01:13.34 UTC    save: yyyymmdd 03:00:12.34      120\n
0123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789
********************************************************************************/
    /*alex 11/1/97: changed format to be variable lenth <station> <comp> <net>
      separated by spaces: */
    sprintf( str, " %s %s %s %c %s UTC    save: %s %8ld\n",
            sta, comp, net, ph, pckt_str, savet_str, (long)durSave );

   strcat( trigmsg, str );

   return;
}

/*********************************************************************
 * make_datestr()  takes a time in seconds since 1600 and converts   *
 *                 it into a character string in the form of:        *
 *                   "19880123 12:34:12.21"                          *
 *                 It returns a pointer to the new character string  *
 *                                                                   *
 *    NOTE: this requires an output buffer >=21 characters long      *
 *                                                                   *
 *  Y2K compliance:                                                  *
 *     date format changed to YYYYMMDD                               *
 *     date15() changed to date17()                                  *
 *                                                                   *
 *********************************************************************/

char *make_datestr( double t, char *datestr )
{
    char str17[18];   /* temporary date string */

/* Convert time to a pick-format character string */
    date17( t, str17 );

/* Convert a date character string in the form of:
   "19880123123412.21"        to one in the form of:
   "19880123 12:34:12.21"
    0123456789 123456789
   Requires a character string at least 21 characters long
*/
    strncpy( datestr, str17,    8 );    /*yyyymmdd*/
    datestr[8] = '\0';
    strcat ( datestr, " " );
    strncat( datestr, str17+8,  2 );    /*hr*/
    strcat ( datestr, ":" );
    strncat( datestr, str17+10,  2 );    /*min*/
    strcat ( datestr, ":" );
    strncat( datestr, str17+12, 5 );    /*seconds*/


    return( datestr );
}
