/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: rw_strongmotion.c 7113 2018-02-14 21:59:53Z baker $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.3  2005/03/17 17:27:07  davidk
 *     Non-functional change to remove annoying compiler warnings.
 *
 *     Revision 1.2  2000/02/25 18:07:56  dietz
 *     kludged to ensure that vendor an serialnumber are never null strings
 *
 *     Revision 1.1  2000/02/14 18:51:48  lucky
 *     Initial revision
 *
 *
 */

/* rw_strongmotion.c
 *
 * Contains functions in that convert from a
 * TYPE_STRONGMOTION message to a structure and visa versa.
 *
 * written by Lynn Dietz   October, 1999
 */

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "chron3.h"
#include "time_ew.h"
#include "rw_strongmotion.h"
#include "earthworm_simple_funcs.h"

static int strappend( char *s1, int s1max, char *s2 );
static int tokenlength( char *begtok, char c );

static char *sNullDate = "0000/00/00 00:00:00.00";

/********************************************************************
 * rd_strongmotion()                                                *
 * Reads an ascii TYPE_STRONGMOTION message and fills in a SM_DATA  *
 * structure.                                                       *
 * Returns 0 on success, -1 on failure                              *
 ********************************************************************/
int rd_strongmotion( char *msg, int msglen, SM_DATA *sm )
{
   char      line[256];
   char     *nextline;  /* working pointer into msg */
   char     *begtok;    /* points to the beginning of this token */
   char     *endline;   /* points at end of line */
   int       len;       /* length (bytes) of the next token */
   int       nline;     /* number of lines read from msg so far */
   int       nfreq;
   int       i;
   struct tm stm;       /* time structure for timestamp */
   time_t    tsec;      /* timestamp in whole sec since 1970 */    
   int       hsec;      /* hundredths of seconds field of timestamp */


   memset( sm, 0, sizeof(SM_DATA) );     /* zero output structure */
   nextline = msg;
   nline = 0;
   nfreq = 0;


/* Read next line from the message into working buffer */
   while( nextline < (msg+msglen-1) )
   {
     len = tokenlength( nextline, '\n' ); 
     if( len >= 256 ) return( -1 );
     strncpy( line, nextline, len ); 
     line[len] = 0; /* null terminate the line */
     endline = line + strlen(line);
     nline++;
     nextline += len + 1;

  /* First line = Vendor */ 
     if( nline == 1 )
     {
       if( strncmp(line,"Vendor:", 7) != 0 ) return( -1 );
       begtok = line + 7;
       while( begtok[0] == ' ' ) begtok++;    /* go to 1st non-blank char */
       len = strlen(begtok); 
       if( len > SM_VENDOR_LEN ) return( -1 );
       strncpy( sm->vendor, begtok, len );
     }

  /* Second line = Serial number */ 
     else if( nline == 2 )
     {
       if( strncmp(line,"SerialNo:", 9) != 0 ) return( -1 );
       begtok = line + 9;
       while( begtok[0] == ' ' ) begtok++;    /* go to 1st non-blank char */
       len = strlen(begtok); 
       if( len > SM_VENDOR_LEN ) return( -1 );
       strncpy( sm->sn, begtok, len );
     }

  /* Third line = number of channels */ 
     else if( nline == 3 )
     {
       if( strncmp(line,"nchan:", 5) != 0 ) return( -1 );
       if( sscanf( line, "nchan: %d", &(sm->nch) ) != 1 ) return( -1 );
     }

  /* Fourth line = timestamp from field unit */
     else if( nline == 4 )
     {
       if( strncmp(line,"Field Time:", 11) != 0 ) return( -1 );
       memset( &stm, 0, sizeof(struct tm) ); /* zero the time structure */
       if( sscanf( line, "Field Time: %d/%d/%d %d:%d:%d.%d", 
                  &stm.tm_year, &stm.tm_mon, &stm.tm_mday, &stm.tm_hour,
                  &stm.tm_min, &stm.tm_sec, &hsec ) != 7 ) return( -1 );
       stm.tm_year -= 1900;  /* to convert to definition of struct tm */
       stm.tm_mon  -= 1;     /* to convert to definition of struct tm */
       tsec  = timegm( &stm );
       sm->tfield = (double)tsec + 0.01*(double)hsec;
     }

  /* Fifth line = alternate timestamp and its code */
     else if( nline == 5 )
     {
       if( strncmp(line,"Alternate Time:", 15) != 0 ) return( -1 );
       memset( &stm, 0, sizeof(struct tm) ); /* zero the time structure */
       if( sscanf( line, "Alternate Time: %d/%d/%d %d:%d:%d.%d code: %d", 
                  &stm.tm_year, &stm.tm_mon, &stm.tm_mday, &stm.tm_hour,
                  &stm.tm_min, &stm.tm_sec, &hsec, &(sm->altcode) ) != 8 ) 
                  return( -1 );
       if( stm.tm_year == 0 ) {
         sm->talt = 0.0;
       } else {
         stm.tm_year -= 1900;  /* to convert to definition of struct tm */
         stm.tm_mon  -= 1;     /* to convert to definition of struct tm */
         tsec  = timegm( &stm );
         sm->talt = (double)tsec + 0.01*(double)hsec;
       }
     }

   /* Sixth line = timestamp from DBMS (or other system) */
     else if( nline == 6 )
     {
       if( strncmp(line,"DBMS Load Time:", 15) != 0 ) return( -1 );
       memset( &stm, 0, sizeof(struct tm) ); /* zero the time structure */
       if( sscanf( line, "DBMS Load Time: %d/%d/%d %d:%d:%d.%d", 
                  &stm.tm_year, &stm.tm_mon, &stm.tm_mday, &stm.tm_hour,
                  &stm.tm_min, &stm.tm_sec, &hsec ) != 7 ) return( -1 );
       if( stm.tm_year == 0 ) {
         sm->tload = 0.0;
       } else {
         stm.tm_year -= 1900;  /* to convert to definition of struct tm */
         stm.tm_mon  -= 1;     /* to convert to definition of struct tm */
         tsec  = timegm( &stm );
         sm->tload = (double)tsec + 0.01*(double)hsec;
       }
     }

 /* Seventh line = SCNL names */
     else if( nline == 7 )
     {
       if( strncmp(line,"SCNL:", 5) != 0 ) return( -1 );
       begtok = line + 5;
       for( i=0; i<sm->nch; i++ )
       {
         while( begtok[0] == ' ' ) begtok++;    /* go to 1st non-blank char */
         if( begtok >= endline ) return( -1 );
         len = tokenlength( begtok, '.' );
         if( len > SM_STA_LEN ) return( -1 );
         strncpy( sm->ch[i].sta, begtok, len ); 
         begtok += len + 1;

         if( begtok >= endline ) return( -1 );
         len = tokenlength( begtok, '.' );
         if( len > SM_COMP_LEN ) return( -1 );
         strncpy( sm->ch[i].comp, begtok, len ); 
         begtok += len + 1;

         if( begtok >= endline ) return( -1 );
         len = tokenlength( begtok, '.' );
         if( len > SM_NET_LEN ) return( -1 );
         strncpy( sm->ch[i].net, begtok, len ); 
         begtok += len + 1;

         if( begtok > endline ) return( -1 );
         len = tokenlength( begtok, ' ' );
         if( len > SM_LOC_LEN ) return( -1 );
         strncpy( sm->ch[i].loc, begtok, len ); 
         begtok += len + 1;
       }
     }

  /* Eighth line = peak acceleration values */
     else if( nline == 8 )
     {
       if( strncmp(line,"Acc:", 4) != 0 ) return( -1 );
       begtok = line + 4;
       for( i=0; i<sm->nch; i++ )
       {
         while( begtok[0] == ' ' ) begtok++;  /* go to 1st non-blank char */
         if( begtok >= endline ) return( -1 );
         if( sscanf( begtok, "%lf", &(sm->ch[i].acc) ) != 1 ) return( -1 );
         begtok += tokenlength( begtok, ' ' ) + 1;
       } 
     }

  /* Ninth line = peak velocity values */
     else if( nline == 9 )
     {
       if( strncmp(line,"Vel:", 4) != 0 ) return( -1 );
       begtok = line + 4;
       for( i=0; i<sm->nch; i++ )
       {
         while( begtok[0] == ' ' ) begtok++;  /* go to 1st non-blank char */
         if( begtok >= endline ) return( -1 );
         if( sscanf( begtok, "%lf", &(sm->ch[i].vel) ) != 1 ) return( -1 );
         begtok += tokenlength( begtok, ' ' ) + 1;
       } 
     }

  /* Tenth line = peak displacement values */
     else if( nline == 10 )
     {
       if( strncmp(line,"Disp:", 5) != 0 ) return( -1 );
       begtok = line + 5;
       for( i=0; i<sm->nch; i++ )
       {
         while( begtok[0] == ' ' ) begtok++;  /* go to 1st non-blank char */
         if( begtok >= endline ) return( -1 );
         if( sscanf( begtok, "%lf", &(sm->ch[i].disp) ) != 1 ) return( -1 );
         begtok += tokenlength( begtok, ' ' ) + 1;
       } 
     }

  /* Eleventh line = number of points in the response spectrum */
     else if( nline == 11 )
     {
       if( strncmp(line,"nRSA:", 5) != 0 ) return( -1 );
       begtok = line + 5;
       for( i=0; i<sm->nch; i++ )
       {
         while( begtok[0] == ' ' ) begtok++;  /* go to 1st non-blank char */
         if( begtok >= endline ) return( -1 );
         if( sscanf( begtok, "%d", &(sm->ch[i].nRSA) ) != 1 ) return( -1 );
         begtok += tokenlength( begtok, ' ' ) + 1;
         if( sm->ch[i].nRSA > SM_MAX_RSA ) return( -1 );
       } 
     }

  /* Remaining lines = response spectrum */
     else if( nline > 11 )
     {
       double tmp1, tmp2;
       if( strncmp(line,"frq/RSA:", 8) != 0 ) return( -1 );
       begtok = line + 8;
       for( i=0; i<sm->nch; i++ )
       {
         while( begtok[0] == ' ' ) begtok++;  /* go to 1st non-blank char */
         if( begtok >= endline ) return( -1 );
         if( sscanf( begtok, "%lf/%lf", &tmp1, &tmp2 ) != 2 ) return( -1 );
         if( nfreq < sm->ch[i].nRSA )
         {
           sm->ch[i].freq[nfreq] = tmp1;
           sm->ch[i].RSA[nfreq]  = tmp2;
         } 
         begtok += tokenlength( begtok, '/' ) + 1; /* skip over freq field */
         while( begtok[0] == ' ' ) begtok++;       /* go to 1st non-blank char */
         begtok += tokenlength( begtok, ' ' ) + 1; /* skip over RSA field */
       } 
       nfreq++;
     }

   } /*end while*/


   return( 0 );
}

/********************************************************************
 * wr_strongmotion()                                                *
 * Reads a SM_DATA structure and writes an ascii TYPE_STRONGMOTION  *
 * message (null terminated)                                        *
 * Returns 0 on success, -1 on failure (buffer overflow)            *
 ********************************************************************/
int wr_strongmotion( SM_DATA *sm, char *buf, int buflen )
{
   char     tmp[256]; /* working buffer */
   int      nchan;
   int      nRSA;
   int      i,j;

   memset( buf, 0, (size_t)buflen );    /* zero output buffer */
   nchan = ( sm->nch > SM_MAX_CHAN ) ? SM_MAX_CHAN : sm->nch ;

/* Print 3 header lines: vendor, serial number, nchan */
   if( strlen(sm->vendor)==0 ) strcpy( sm->vendor, "unknown" ); /*KLUDGE*/
   if( strlen(sm->sn)==0     ) strcpy( sm->sn,     "unknown" ); /*KLUDGE*/
    sprintf( tmp, "Vendor: %s\nSerialNo: %s\nnchan: %d\nField Time:     ", 
            sm->vendor, sm->sn, nchan );
   if( strappend( buf, buflen, tmp ) ) return( -1 );

/* field time */
   datestr23( sm->tfield, tmp, 256 );  
   if( strappend( buf, buflen, tmp ) ) return( -1 );

/* alternate time & its code */
   if( strappend( buf, buflen, "\nAlternate Time: " ) ) return ( -1 );
   if( sm->talt == 0.0 )
   {
     if( strappend( buf, buflen, sNullDate ) ) return( -1 );
   } else {
     datestr23( sm->talt, tmp, 256 );  
     if( strappend( buf, buflen, tmp ) ) return( -1 );
   }
   sprintf( tmp, " code: %d", sm->altcode );
   if( strappend( buf, buflen, tmp ) ) return( -1 );

/* load time */
   if( strappend( buf, buflen, "\nDBMS Load Time: " ) ) return ( -1 );
   if( sm->tload == 0.0 )
   {
     if( strappend( buf, buflen, sNullDate ) ) return( -1 );
   } else {
     datestr23( sm->tload, tmp, 256 );  
     if( strappend( buf, buflen, tmp ) ) return( -1 );
   }

/* Print the SCNL names */
   if( strappend( buf, buflen, "\nSCNL:    " ) ) return( -1 );
   for( i=0; i<nchan; i++ )
   {
      sprintf( tmp, "   %s.%s.%s.%s", 
               sm->ch[i].sta, sm->ch[i].comp, sm->ch[i].net, sm->ch[i].loc );
      if( strappend( buf, buflen, tmp ) ) return( -1 );
   }
   
/* Print peak acceleration values */
   if( strappend( buf, buflen, "\nAcc:     " ) ) return( -1 );
   for( i=0; i<nchan; i++ )
   {
      sprintf( tmp, " %15.6lf", sm->ch[i].acc );
      if( strappend( buf, buflen, tmp ) ) return( -1 );
   }
      
/* Print peak velocity values */
   if( strappend( buf, buflen, "\nVel:     " ) ) return( -1 );
   for( i=0; i<nchan; i++ )
   {
      sprintf( tmp, " %15.6lf", sm->ch[i].vel );
      if( strappend( buf, buflen, tmp ) ) return( -1 );
   }   
      
/* Print peak displacement values */
   if( strappend( buf, buflen, "\nDisp:    " ) ) return( -1 );
   for( i=0; i<nchan; i++ )
   {
      sprintf( tmp, " %15.6lf", sm->ch[i].disp );
      if( strappend( buf, buflen, tmp ) ) return( -1 );
   }   

/* Print number of points in the response spectrum */
   nRSA = 0;
   if( strappend( buf, buflen, "\nnRSA:    " ) ) return( -1 );
   for( i=0; i<nchan; i++ )
   {
      if( sm->ch[i].nRSA < SM_MAX_RSA ) sprintf( tmp, " %15d", sm->ch[i].nRSA );
      else                              sprintf( tmp, " %15d", SM_MAX_RSA );
      if( strappend( buf, buflen, tmp ) ) return( -1 );
      if( nRSA < sm->ch[i].nRSA ) nRSA = sm->ch[i].nRSA;
   }   

/* Print the response spectrum (frequency/acceleration pairs) */
   if( nRSA > SM_MAX_RSA ) nRSA = SM_MAX_RSA;
   for( j=0; j<nRSA; j++ )
   {
      if( strappend( buf, buflen, "\nfrq/RSA: " ) ) return( -1 );
      for( i=0; i<nchan; i++ )
      {
        if( j < sm->ch[i].nRSA )  /* print real values for this channel */
        {
          sprintf( tmp, " %6.2lf/%.6lf", sm->ch[i].freq[j], sm->ch[i].RSA[j] );
        }
        else                      /* print null values for this channel */
        {
          sprintf( tmp, " %6.2lf/%.6lf", SM_NULL, SM_NULL );
        } 
        if( strappend( buf, buflen, tmp ) ) return( -1 );
      }
   }
   if( strappend( buf, buflen, "\n" ) ) return( -1 );

   return( 0 );
}


/********************************************************************
 * log_strongmotion()                                               *
 * Writes the contents of a SM_DATA structure to an Earthworm       *
 * log file                                                         *
 ********************************************************************/
void log_strongmotion( SM_DATA *sm )
{
   char date[30];
   int nRSA, i, j;

   logit("", "Vendor:%s sn:%s nch:%d\n", sm->vendor, sm->sn, sm->nch );
   
/* Log time values */
   datestr23( sm->tfield, date, 30 );
   logit("", "Field Time:     %s  (%.2lf)\n", date, sm->tfield );
   if( sm->talt != 0.0 ) {
     datestr23( sm->talt, date, 30 );
     logit("", "Alternate Time: %s  (%.2lf)  code: %d\n", 
           date, sm->talt, sm->altcode );
   } else {
     logit("", "Alternate Time: %s  (%.2lf)  code: %d\n", 
           sNullDate, sm->talt, sm->altcode );
   }

   if( sm->talt != 0.0 ) {
     datestr23( sm->tload, date, 30 );
     logit("", "DBMS Load Time: %s  (%.2lf)\nSCNL:", date, sm->tload );
   } else {
     logit("", "DBMS Load Time: %s  (%.2lf)\nSCNL:", sNullDate, sm->tload );
   }

/* Log station codes */
   for( i=0; i<sm->nch; i++ )
   {
     logit("", "   %s.%s.%s.%s", 
             sm->ch[i].sta, sm->ch[i].comp, sm->ch[i].net, sm->ch[i].loc );
   }

/* Log peak acceleration values */
   logit("", "\nAcc:     ");
   for( i=0; i<sm->nch; i++ )
   {
      logit("", " %15.6lf", sm->ch[i].acc );
   }
      
/* Log peak velocity values */
   logit("", "\nVel:     ");
   for( i=0; i<sm->nch; i++ )
   {
      logit("", " %15.6lf", sm->ch[i].vel );
   }   
      
/* Log peak displacement values */
   logit("","\nDisp:    ");
   for( i=0; i<sm->nch; i++ )
   {
      logit("", " %15.6lf", sm->ch[i].disp );
   }   

/* Log number of points in the response spectrum */
   nRSA = 0;
   logit("","\nnRSA:    ");
   for( i=0; i<sm->nch; i++ )
   {
      logit("", " %15d", sm->ch[i].nRSA );
      if( nRSA < sm->ch[i].nRSA ) nRSA = sm->ch[i].nRSA;
   }   

/* Log the response spectrum (frequency/acceleration pairs) */
   for( j=0; j<nRSA; j++ )
   {
      logit("", "\nfrq/RSA: " );
      for( i=0; i<sm->nch; i++ )
      {
        if( j < sm->ch[i].nRSA )  /* print real values for this channel */
        {
          logit("", " %6.2lf/%8.6lf", sm->ch[i].freq[j], sm->ch[i].RSA[j] );
        }
        else                      /* print null values for this channel */
        {
          logit("", " %6.2lf/%8.6lf", SM_NULL, SM_NULL );
        } 
      }
   }
   logit("","\n");

   return;
}

/********************************************************************
 * strappend() append second null-terminated character string to    *
 * the first as long as there's enough room in the target buffer    * 
 * for both strings an the null-byte                                *
 ********************************************************************/
int strappend( char *s1, int s1max, char *s2 )
{
   if( (int)strlen(s1)+(int)strlen(s2)+1 > s1max ) return( -1 );
   strcat( s1, s2 );
   return( 0 );
}

/********************************************************************
 * tokenlength() given a null-terminated character string and a     *
 * character that delimits the end of a token, tokenlength returns  * 
 * the length (in bytes) of the next token. If the character wasn't * 
 * found, tokenlength returns the length of the string.             *
 ********************************************************************/
int tokenlength( char *begtok, char c )
{
   char    *endtok;   /* points to the end of this token */

   endtok = strchr( begtok, c );
   if( endtok == NULL ) return( (int)strlen(begtok) );
   return( (int)(endtok-begtok) );
}
