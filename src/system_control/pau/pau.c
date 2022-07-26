/*  pau.c   simple program to attach to earthworm's
 *          shared memory region(s) and set the flag
 *          in the header to the terminate flag
 */

/* 5/21/98: changed to allow config file to be specified on the command line.
 * If none is specified, then a default file is used depending on the OS.
 * PNL, UW Geophysics. */

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
   printf( "pau: Requesting shutdown of entire Earthworm system!\n" );

/* If given, use config file name on command line
   **********************************************/
   if ( argc == 2 )
   {
      setflags( "pau", argv[1], TERMINATE );
   }

/* Otherwise, use default config file
   **********************************/
   else
   {
      printf( "pau: Using default startstop config file %s\n",
               DEF_CONFIG );
      setflags( "pau", DEF_CONFIG, TERMINATE );
   }

   return 0;
}
