
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: getavail.c 1878 2005-07-15 18:20:22Z friberg $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2005/07/15 18:20:19  friberg
 *     Unix version of libsrc for POSIX systems
 *
 *     Revision 1.1  2000/02/14 18:46:17  lucky
 *     Initial revision
 *
 *
 */


       /****************************************************
        *                   GetDiskAvail                   *
        *                                                  *
        *  DiskAvail = Available disk space in kilobytes.  *
        *  This function returns -1 if error;              *
        *                         0 if no error.           *
        *  SOLARIS 2.4 version.                            *
        ****************************************************/

#include <sys/types.h>
#include <sys/statvfs.h>

int GetDiskAvail( unsigned *DiskAvail )
{
   struct statvfs buf;

   if ( statvfs( ".", &buf ) == -1 )
      return( -1 );

/* printf( "f_bsize:  %u\n", buf.f_bsize );       Block size
   printf( "f_blocks: %u\n", buf.f_blocks );      Total blocks
   printf( "f_bfree:  %u\n", buf.f_bfree );       Available to super user
   printf( "f_bavail: %u\n", buf.f_bavail );      Available to others */

   *DiskAvail = buf.f_bavail;
   return( 0 );
}
