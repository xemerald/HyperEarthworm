/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: putpick.c 7248 2018-04-06 20:52:10Z baker $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.2  2004/06/24 20:34:03  dietz
 *     changed comments
 *
 *     Revision 1.1  2004/06/24 18:58:14  dietz
 *     New tool for writing pick/coda msgs to ring
 *
 */

  /*****************************************************************
   *                           putpick.c                           *
   *                                                               *
   * Morphed from putter.c                                         *
   *                                                               *
   * Program to read a file containing a series of pick/coda msgs  *
   * and write them to shared memory.  The file can contain a mix  *
   * of TYPE_PICK2K, TYPE_PICK_SCNL, TYPE_CODA2K, TYPE_CODA_SCNL,  *
   * and also comments.  Only valid TYPE_PICK* and TYPE_CODA* msgs *
   * will be written to shared memory; all other lines will be     *
   * ignored.                                                      *
   *                                                               *
   * Putpick is useful for replaying picks into binder - binder's  *
   * logfile (or a portion of it) can be used as the input file    *
   * to putpick, or the picks can be placed in a seperate file.    *
   *                                                               *
   * If pause = y, the relative timing of writing msgs to          *
   *    ring is based on delta-t in the timestamps in the picks.   *
   * If pause = n, the msgs are written to the ring as fast        *
   *    as they are read from the file.                            *
   *                                                               *
   * Usage: putpick <pickfile> <RING_NAME> <pause: y/n>            *
   *                                                               *
   *****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "earthworm.h"
#include "transport.h"
#include "rdpickcoda.h"

#define  MAX_CHAR 256

int main( int argc, char *argv[] )
{
   FILE *     fp;
   char       ringname[50];
   char       line[MAX_CHAR];
   SHM_INFO   region;
   MSG_LOGO   logo;
   EWPICK     pick;
   EWCODA     coda;
   long       RingKey;         /* Key to the transport ring to write to */
   int        pause;
   double     tlast, dt;
   int        firstpick = 1;
   int        pauseMsecs = 0;

/* Check command line arguments
   ****************************/
   if ( argc < 4 )
   {
      printf( "Usage: putpick <pickfile> <RING_NAME> <pause: y/n or microsecs delay>\n" );
      exit( -1 );
   }

/* Initialize logging 
   ******************/
   logit_init( argv[0], 0, 1024, 0 );

/* Open pick file
   **************/
   fp = fopen( argv[1], "r" );
   if ( fp == NULL )
   {
      logit("e", "putpick: Cannot open pick file: %s; exiting!\n", argv[1] );
      exit( -2 );
   }

/* Set the pause flag
   ******************/
   pauseMsecs = atoi(argv[3]);
   if      ( strcmp(argv[3],"y") == 0 ) pause = 1;
   else if ( strcmp(argv[3],"Y") == 0 ) pause = 1;
   else if ( strcmp(argv[3],"n") == 0 ) pause = 0;
   else if ( strcmp(argv[3],"N") == 0 ) pause = 0;
   else if ( pauseMsecs > 0) pause = 2;
   else
   {
      logit("e", "putpick: Must specify pause: y or n or value of milliseconds to pause\n");
      exit( -3 );
   }

/* Attach to transport ring
   ************************/
   strcpy( ringname, argv[2] );

   if ( ( RingKey = GetKey(ringname) ) == -1 )
   {
      logit("e", "putpick: Invalid RingName <%s>; exiting!\n", ringname );
      exit( -4 );
   }
   tport_attach( &region, RingKey );
   logit("o", "putpick: Writing to <%s> ", ringname );
   if( pause == 1) logit( "o", "with relative timing based on pick times.\n" );
   else if( pause == 2) logit( "o", "with relative timing of %d milliseconds between pick releases.\n", pauseMsecs );
   else        logit( "o", "as fast as I can read the file!\n" );

/*----------------------main loop----------------------*/

/* Read from pick file, put to transport ring
   ******************************************/
   while ( fgets( line, MAX_CHAR, fp ) != NULL )
   {
      if ( tport_getflag( &region ) == TERMINATE )  break;

   /* Try to decode the line as a pick
    **********************************/
      if( rd_pick2k( line, strlen(line), &pick )    == EW_SUCCESS  ||
          rd_pick_scnl( line, strlen(line), &pick ) == EW_SUCCESS     )
      {
      /* check times only if we want relative timing */
         if( pause ) 
         {         
            if( firstpick )
            {
               tlast = pick.tpick;
               firstpick = 0;
            }
            dt = pick.tpick - tlast;
         /* Pause only if this pick is "younger" than newest pick sent */
            if (pause == 2) {
               sleep_ew( (int)(pauseMsecs) );
            } else if( dt > 0.0 ) {
               logit("o","putpick: going to sleep for %.2lf seconds\n", dt );
               sleep_ew( (int)(dt*1000) );
               tlast = pick.tpick;
            }
         }
         logo.type   = pick.msgtype;
         logo.mod    = pick.modid;
         logo.instid = pick.instid;         
      }

   /* Try to decode the line as a coda
    **********************************/
      else if( rd_coda2k( line, strlen(line), &coda )    == EW_SUCCESS  ||
               rd_coda_scnl( line, strlen(line), &coda ) == EW_SUCCESS     )
      {
         logo.type   = coda.msgtype;
         logo.mod    = coda.modid;
         logo.instid = coda.instid;  
      }

   /* Couldn't read line as a valid message, skip it
    ************************************************/
      else
      {
         continue;
      }

      logit("o", "%s", line );

      if ( tport_putmsg( &region, &logo, strlen(line), line ) != PUT_OK )
      {
         logit("e", "putpick: Error putting message in region %ld\n",
               region.key );
      }
   }

/*----------------------end of main loop----------------------*/

   tport_detach( &region );
   fclose( fp );
   logit("o", "putpick: Reached end of pick file: %s; exiting.\n", argv[1] );
   return 0;
}
