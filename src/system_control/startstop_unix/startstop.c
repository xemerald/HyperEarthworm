       /***********************************************************
        *                    Program startstop                    *
        *                                                         *
        *     Program to start and stop the Earthworm system      *
        ***********************************************************/

/* Changed 1/13/98 by LDD: increased MAXLINE, changed parm field of the
 * CHILD struct to "char parm[MAXLINE]" to allow longer process command
 * strings in the startstop config file.
 *
 * Changes:
 * Lombard: 11/19/98: V4.0 changes:
 *   0) no Y2K dates
 *   1) changed argument of logit_init to the config file name.
 *   2) process ID in heartbeat message: not applicable
 *   3) flush input transport ring: not applicable
 *   4) add `restartMe' to .desc file: not applicable
 *   5) multi-threaded logit: not applicable
 *
 * Changed 6/11/98 by PNL: added call to setpgid so that startstop would
 * the process group leader. This will let StopEarthworm() kill its
 * children but not other processes. Requested by Kent Lindquist, UAF.
 *
 * Changed 5/4/98 by KGL: migrated addition of Agent field to parameter
 * file into this version of startstop, so all the modules don't have
 * to run as root. Also added paragraph to allow specification of
 * configuration file name on command line. Added signal handler to
 * exit cleanly on SIGTERM.
 * Added nice() call to restore default nice value.
 *
 * Changed 4/7/98 by PNL: fixed problem when running startstop from a script.
 * fgets() returns ESPIPE instead of EIO.
 *
 * Changed 3/17/98 by LDD: changed Earthworm status request/response to
 * use the transport ring instead of Solaris msgsnd() & msgrcv().
 * Also moved Heartbeat() function from a separate thread back into the
 * main thread.
 *
 * Changed 12/2/97 by PNL: status message changed to include the arguments
 * given to each command from the startstop_unix.d file, up to 80 characters
 * so the message will still fit on an 80-column screen.
 *
 * Changed 11/1/97 by PNL: now can be run in background as well as in
 * foreground. This is done by setting SIGTTIN to IGNORE.
 */


#include <startstop_unix_generic.h>


int main( int argc, char *argv[] )
{

   int returnval;
   returnval = RunEarthworm( argc, argv ); /* found in startstop_unix_generic.c */
   UnlockStartstop();
   return returnval;
}
