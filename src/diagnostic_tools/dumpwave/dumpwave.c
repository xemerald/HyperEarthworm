
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: dumpwave.c 6803 2016-09-09 06:06:39Z et $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.2  2004/07/26 17:32:08  dietz
 *     Modified to read TYPE_TRACEBUF2 and TYPE_TRACEBUF files
 *
 *     Revision 1.1  2001/08/30 07:53:38  dietz
 *     Initial revision
 *
 *     Revision 1.2  2000/09/07 15:43:07  dietz
 *     added checks for gaps and overlaps.
 *
 *     Revision 1.1  2000/02/14 18:31:49  lucky
 *     Initial revision
 *
 *
 */

/*
   dumpwave.c

   Dump a TYPE_TRACEBUF2 or TYPE_TRACEBUF waveform data file. 

   Command line arguments:   
       argv1 = file to dump
       argv2 = pin# to dump
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <trace_buf.h>
#include <swap.h>
#include <chron3.h>

char *epochsectostr( double, char * );

double   Sec1970;
char    *Str1970 = "19700101000000.00";

int main(int argc, char **argv)
{
   char          tstr[25];
   char         *pdata;
   short        *sdata;
   int32_t      *ldata;
   FILE         *fp;
   TracePacket   trpkt;
   int           i, j, x;
   size_t        nchar;
   int32_t       nsamp;
   int           byte_per_sample;
   double        prevtime=0.0;
   int           isSCNL;

/* Check arguments
 *****************/
   /*printf("sizeof(TRACE2_HEADER): %d\n", sizeof(TRACE2_HEADER) );*/
   if( argc < 2 ) {
      printf("Usage: dumpwave <filename> <optional:pinno>\n");
      return( 0 );
   }
   Sec1970 = julsec17( Str1970 );

/* Open waveform file
   ******************/
   fp = fopen( argv[1], "rb" );
   if ( fp == NULL )
   {
      fprintf( stderr, "Cannot open file <%s>\n", argv[1] );
      return( 0 );
   }
   if( argc==3) x=atoi( argv[2] );
   else         x=0;

/* Set pointers
   ************/
   pdata = trpkt.msg + sizeof(TRACE2_HEADER);
   ldata = (int32_t *) pdata;
   sdata = (short *)pdata;

/* Read all buffers in file
   ************************/
   for ( i = 0; 1; i++ )
   {

/* Read a header
   *************/
      isSCNL = 1;
      memset( trpkt.msg, 0, MAX_TRACEBUF_SIZ );
      if ( fread( (void *)trpkt.msg, sizeof(TRACE2_HEADER), 1, fp ) < (size_t)1 )
         break;

      if( !TRACE2_HEADER_VERSION_IS_VALID(&trpkt.trh2) )
      {
         isSCNL = 0;
      }

      nsamp           = (int32_t)trpkt.trh2.nsamp;
      byte_per_sample = atoi(&trpkt.trh2.datatype[1]);

      #ifdef _SPARC
         if( trpkt.trh2.datatype[0]=='i' )
         {
            SwapInt32( &nsamp );
         }
      #endif
      #ifdef _INTEL
         if( trpkt.trh2.datatype[0]=='s' )
         {
            SwapInt32( &nsamp );
         }
      #endif
      
/* Read the data samples
   *********************/
      nchar = (size_t)nsamp * byte_per_sample;   /* samples */
      if( fread( pdata, sizeof(char), nchar, fp ) < nchar ) break;
     
      if( isSCNL ) WaveMsg2MakeLocal( &trpkt.trh2 );
      else         WaveMsgMakeLocal ( &trpkt.trh  );

/* Print out pinno x 
   ******************/
      if( trpkt.trh2.pinno!=x && x!=0 ) continue;

      if( prevtime != 0.0 )
      {
         double actdt = trpkt.trh2.starttime - prevtime;
         double expdt = 1.0/trpkt.trh2.samprate;
         if( actdt <= 0.0      ) printf("DATA OVERLAP  %.4lf sec\n\n", actdt );
         if( actdt > expdt*1.5 ) printf("DATA GAP  %.4lf sec\n\n", actdt );     
      }

      if( isSCNL ) {
        printf("  version: %c%c\n", trpkt.trh2.version[0], trpkt.trh2.version[1] );
        printf("  s.c.n.l: %s.%s.%s.%s\n", 
               trpkt.trh2.sta, trpkt.trh2.chan, trpkt.trh2.net, trpkt.trh2.loc );
      } else {
        printf("    s.c.n: %s.%s.%s\n", 
               trpkt.trh.sta, trpkt.trh.chan, trpkt.trh.net );
      }
      printf("    pinno: %5d\n",   trpkt.trh2.pinno );  
      printf("    nsamp: %5d\n",   trpkt.trh2.nsamp );
      printf(" samprate: %.4lf\n", trpkt.trh2.samprate );
      printf("starttime: %.4lf   %s\n", 
              trpkt.trh2.starttime, epochsectostr( trpkt.trh2.starttime, tstr ) );
      printf("  endtime: %.4lf   %s\n", 
              trpkt.trh2.endtime,   epochsectostr( trpkt.trh2.endtime, tstr ) );
      printf(" datatype: %s\n", trpkt.trh2.datatype );
      switch (trpkt.trh2.version[1]) {
      case TRACE2_VERSION1:
         printf(" quality0: %c   quality1: %c\n", 
              trpkt.trh2.quality[0], trpkt.trh2.quality[1] );
         printf("     pad0: %c       pad1: %c\n", 
              trpkt.trh2.pad[0], trpkt.trh2.pad[1] );
         break;
      case TRACE2_VERSION11:  /* version 21 */
         printf(" convfact: %.4lf\n", trpkt.trh2x.x.v21.conversion_factor );
         break;
      }

      for ( j = 0; j < nsamp; j++ )
      {
         if( byte_per_sample==4 ) printf( "%8ld", (long)(ldata[j]) );
         if( byte_per_sample==2 ) printf( "%8hd", sdata[j] );
         if( j%10  == 9 )         printf( "\n" );
      }
      printf( "\n" ); 

      prevtime = trpkt.trh2.endtime;
   }
   fclose( fp );
   return( 0 );
}


char *epochsectostr( double esec, char *str )
{
   char  tmpstr[18];

/* tmpstr: 19980103234516.54     
 * str:    1998/01/03:2345:16.54   
 *         0123456789 123456789 12  
 */

   date17( esec+Sec1970, tmpstr );
   
   strncpy( str, tmpstr, 4 );
   str[4] = '/';
   strncpy( str+5, tmpstr+4, 2 );
   str[7] = '/';
   strncpy( str+8, tmpstr+6, 2 );
   str[10] = ':';
   strncpy( str+11, tmpstr+8, 4 );
   str[15] = ':';
   strncpy( str+16, tmpstr+12, 5 );
   str[21] = '\0';

   return( str );
}

