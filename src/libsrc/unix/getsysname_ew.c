
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: getsysname_ew.c 1878 2005-07-15 18:20:22Z friberg $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2005/07/15 18:20:20  friberg
 *     Unix version of libsrc for POSIX systems
 *
 *     Revision 1.1  2000/02/14 18:46:17  lucky
 *     Initial revision
 *
 *
 */

/*
 *  getsysname_ew.c  - Solaris version
 *
 *  Earthworm utility for getting the system name from the system
 * 
 */

#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>

int getsysname_ew( char *sysname, int length )
{
   struct utsname name;

   if( uname( &name ) == -1 ) 
   {
      fprintf( stderr,
              "getsysname_ew: error on uname() call.\n" ); 
      return( -1 );
   }

   if( strlen( name.nodename ) >= (size_t) length ) 
   {
      fprintf( stderr,
              "getsysname_ew: system name too long for target address.\n");
      return( -2 );
   }

   strcpy( sysname, name.nodename );
   return( 0 );
}


