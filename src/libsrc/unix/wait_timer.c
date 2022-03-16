
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: wait_timer.c 1878 2005-07-15 18:20:22Z friberg $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2005/07/15 18:20:22  friberg
 *     Unix version of libsrc for POSIX systems
 *
 *     Revision 1.1  2000/02/14 18:46:17  lucky
 *     Initial revision
 *
 *
 */

     /********************************************************************
      *                wait_timer.c   for   Solaris                      *
      *                                                                  *
      *  These are dummy functions in Solaris.                           *
      ********************************************************************/

#include <earthworm.h>


         /**********************************************************
          *                    init_wait_timer()                   *
          *                Create a new timer object               *
          **********************************************************/

int init_wait_timer( timer_t *timerHandle, DWORD *errorCode )
{
   return 0;
}


          /***********************************************************
           *                    start_wait_timer()                   *
           *  Start the timer.                                       *
           *  lPeriod is the repeat interval in milliseconds.        *
           ***********************************************************/

int start_wait_timer( timer_t timerHandle, LONG lPeriod, DWORD *errorCode )
{
   return 0;
}


         /**********************************************************
          *                      wait_timer()                      *
          *             Wait for the timer to complete             *
          **********************************************************/

int wait_timer( timer_t timerHandle, DWORD *errorCode )
{
   return 0;
}
