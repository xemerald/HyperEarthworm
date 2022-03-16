/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: pidpau.c 6803 2016-09-09 06:06:39Z et $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.3  2007/03/28 18:20:22  paulf
 *     added _MACOSX flag
 *
 *     Revision 1.2  2006/04/04 19:04:56  stefan
 *     removing deendency on old .h file
 *
 *     Revision 1.1  2001/05/11 20:46:13  dietz
 *     Initial revision
 *
 *
 *
 */

/*  pidpau.c   simple program to attach to earthworm's
 *             shared memory region(s) and set the flag
 *             in the header to the given value (pid).
 *             This is intended to signal that processid
 *             to terminate gracefully.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <transport.h>

/* include the appropriate definition of DEF_CONFIG for the OS */
#if defined(_SOLARIS) || defined(_LINUX) || defined(_MACOSX)
#include <startstop_unix_generic.h>
#endif

#ifdef _WINNT
#include <startstop_winlib.h>

#endif
#ifdef _OS2
#include <startstop_os2.h>
#endif

int setflags( char *execname, char *cfgfile, int flagvalue );

int main( int argc, char **argv )
{
   int pid;

/* Check for minimum # arguments
   *****************************/
   if( argc < 2 )
   {
      printf( "Usage: pidpau <process_id> <optional:startstop config file>\n" );
      return -1;
   }

/* Get processid
   *************/
   pid = (int) strtol( argv[1], (char **)NULL, 10 );
   if( pid == 0 )
   {
      printf( "pidpau: Trouble converting %s to a process_id.\n", argv[1] );
      return -1;
   }

/* If given, use config file name on command line
   **********************************************/
   printf( "pidpau: Requesting shutdown of Earthworm process_id %d.\n",
           pid );

   if( argc == 3 )
   {
      setflags( "pidpau", argv[2], pid );
   }

/* Otherwise, use default config file
   **********************************/
   else
   {
      printf( "pidpau: Using default startstop config file %s\n",
               DEF_CONFIG );
      setflags( "pidpau", DEF_CONFIG, pid );
   }

   return 0;
}
