
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.2  2006/06/06 21:12:32  stefan
 *     hydra console incorporation
 *
 *     Revision 1.1  20060210 20:05:54  lisowski
 *     Initial revision
 *
 *	   This is for use by both starstop_nt and startstop_service
 *
 */

/*
 * startstop_win.h: startstop parameter definitions for WindowsNT.
 *
 */

#ifndef startstop_winlib_H
#define startstop_winlib_H

  /******************************************************************
   *                         Includes                               *
   *                                                                *
   ******************************************************************/

#include <startstop_lib.h>
#include <service_ew.h>		/* the non-service needs to know about this too */
#include <wtypes.h>		/* defines boolean */

  /******************************************************************
   *                         #defines                               *
   *                                                                *
   ******************************************************************/

/* The default startstop config file (in EW_PARAMS directory)
 * This is used by pau, status and restart in addition to startstop
 ******************************************************************/

#define DEF_CONFIG "startstop_nt.d"
/* MAX_RING_STR defined in earthworm_defs.h and at this time happens to be set to 32*/
#define DEBUG

  /******************************************************************
   *                         Prototypes                             *
   *                                                                *
   ******************************************************************/

/* GetConfig reads configuration file        */
int GetConfig( METARING *, CHILD [MAX_CHILD], int * );

/* SpawnChildren starts child processes besides statmanager      */
void SpawnChildren( METARING *, CHILD [MAX_CHILD], int *, boolean, volatile int * );

/* Encode status message           */
void EncodeStatus( char [MAX_STATUS_LEN], METARING *, CHILD [MAX_CHILD], int * );

/* Send a status msg via tranpsort */
void SendStatus( int, METARING *, CHILD [MAX_CHILD], int * );

/* Turn on one child process       */
int  StartChild( int, METARING *, CHILD [MAX_CHILD] );

/* Terminate specific child process*/
int  TerminateChild( int, METARING *, CHILD [MAX_CHILD] );

/* Terminate/restart child process */
void RestartChild( char *, METARING *, CHILD [MAX_CHILD], int * );

int	StartstopSetup ( METARING *, volatile int *, boolean, CHILD [MAX_CHILD], int * );

int FinalLoop ( METARING *, volatile int *, char [MAX_STATUS_LEN], volatile int *, boolean, CHILD [MAX_CHILD], int * );

/* Launch a command prompt console that can talk to startstop_service */
void LaunchNewConsole();

/* Launch a command prompt console with I/O redirected to pipe */
void LaunchPipeConsole();

/* Terminates pipe-console. */
void DoTerminatePipeConsole();


#endif
