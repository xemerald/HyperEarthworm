
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.5  2010/03/10 22:26:58  scott
 *     Made startstop's stoping of modules more robust; touched up release_notes & debias's nt makefile
 *
 *     Revision 1.4  2007/02/27 05:01:40  stefan
 *     sends stop and restart messages to accommodate new statmgr
 *
 *     Revision 1.3  2007/02/22 23:20:19  stefan
 *     lock changes
 *
 *     Revision 1.2  2007/02/20 22:51:05  stefan
 *     stop, reconfigure fixes, lock start
 *
 *     Revision 1.1  2006/04/04 18:00:14  stefan
 *     startstop with reconfigure and libraries 20060404 s.lisowski
 *
 *     Revision 1.0  2006/02/23 10:44:30  lisowski
 *     Initial revision. Removed common code from
 *     system_control\startstop_nt\startstop.c and
 *     system_control\startstop_service\startstop_service.c
 *     and put it here.
 *
 #    return ERROR_FILE_NOT_FOUND; <- these are Microsoft #defined integers
 *
 */

/********************************************************************
 *                 startstop_lib.c for Windows 32, Linux, Solaris   *
 ********************************************************************/

#include <startstop_lib.h> /* Header for this .c file */


/******************************************************************
 *                         Heartbeat()                            *
 *          Send a heartbeat to a transport ring buffer           *
 ******************************************************************/
void Heartbeat( METARING *metaring )
{
   char           msg[20];
   static time_t  tlastbeat=0;
   time_t         now;
   unsigned short length;
   MSG_LOGO       logo;

   logo.instid = metaring->InstId;
   logo.mod    = metaring->MyModId;
   logo.type   = metaring->TypeHeartBeat;

   time( &now );
   if( (now - tlastbeat) < metaring->HeartbeatInt ) return;

   sprintf( msg, "%d\n%c", (int)now, 0 );
   length = strlen( msg );

   if ( tport_putmsg( &(metaring->Region[0]), &logo, length, msg ) != PUT_OK )
      logit("t", "startstop: Error sending heartbeat to transport region: %s\n",
            metaring->ringName[0] );

   tlastbeat = now;
   return;
}


  /************************************************************
   *                   SendstopRequest                     *
   *  To send a message requesting the stoping of a module *
   ************************************************************/
void SendStopReq( METARING *metaring, char*  modPid ) /* pid of module as ascii string*/
{
   MSG_LOGO logo;
   char message[512];

 /* Set logo values of pager message
   ********************************/
   logo.instid = metaring->InstId;
   logo.mod    = metaring->MyModId;
   logo.type   = metaring->TypeStop;

/* Build message
   *************/
   strcpy (message,modPid);
   strcat( message, "\n\0" );

/* Send stop message to transport ring
   **************************************/
   if ( tport_putmsg( &(metaring->Region[0]), &logo, strlen(message), message ) != PUT_OK )
      logit("t", "startstop: Error sending stop message (pid=%s) to transport region: %s\n",
            modPid, metaring->ringName[0] );      

   return;

}

  /************************************************************
   *                   SendRestartRequest                     *
   *  To send a message requesting the restarting of a module *
   ************************************************************/
void SendRestartReq( METARING *metaring, char*  modPid ) /* pid of module as ascii string*/
{
   MSG_LOGO logo;
   char message[512];

 /* Set logo values of pager message
   ********************************/
   logo.instid = metaring->InstId;
   logo.mod    = metaring->MyModId;
   logo.type   = metaring->TypeRestart;

/* Build message
   *************/
   strcpy (message,modPid);
   strcat( message, "\n\0" );

/* Send restart message to transport ring
   **************************************/
   if ( tport_putmsg( &(metaring->Region[0]), &logo, strlen(message), message ) != PUT_OK )
      logit("t", "startstop: Error sending restart message (pid=%s) to transport region: %s\n",
            modPid, metaring->ringName[0] );      

   return;

}


  /******************************************************************
   *                          GetCurrentUTC                         *
   *                                                                *
   *     Get the current UTC time as a 26 character string.         *
   ******************************************************************/

void GetCurrentUTC( char *timenow )
{
   time_t     now;
   struct tm  tstruct;

   time( &now );
   gmtime_ew( &now, &tstruct );
   asctime_ew( &tstruct, timenow, 26 );
   return;
}

/******************************************************************
 *                            ReportError()                       *
 *                                                                *
 *           Send error message to a transport ring buffer        *
 ******************************************************************/

void ReportError(   int errNum,
                    char *errStr,
                    METARING *metaring)
{
   MSG_LOGO logo;
   unsigned short length;
   time_t tstamp;
   #ifdef _WINNT
       char errMsg[128];
   #else
       char errMsg[MAXLINE*2];
   #endif

   time( &tstamp );
   sprintf( errMsg, "%d %d %s\n%c", (int)tstamp, errNum, errStr, 0 );

   logit("et","startstop: %s\n", errStr );

   logo.instid = metaring->InstId;
   logo.mod    = metaring->MyModId;
   logo.type   = metaring->TypeError;
   length      = strlen( errMsg );

   if ( tport_putmsg( &(metaring->Region[0]), &logo, length, errMsg ) != PUT_OK )
      logit("t", "startstop: Error sending error msg to transport region: %s\n",
               metaring->ringName[0] );
} /* end ReportError */


  /******************************************************************
   *                         StartError()                           *
   *                                                                *
   *               Report an error in starting a child              *
   ******************************************************************/

int StartError( int iChild, char *commandLine, METARING *metaring, int *nChild )
{
    char ErrText[255];
    char environmentScript[36];
#ifndef _WINNT /* this is probably Solaris or Linux  */
    strcpy( environmentScript, "ew_linux.bash or ew_sol_sparc.cmd");
#else /* This is startstop NT or startstop service */
    strcpy( environmentScript, "ew_nt.cmd");
#endif

    logit("e","\nstartstop: **** CHILD FATAL ERROR ****\n" );
    sprintf( ErrText,"error starting <%s>", commandLine );
    ReportError( ERR_STARTCHILD, ErrText, metaring );
    logit("e","\nTroubleshooting Tips:" );
    logit("e","\n------------------------------------------" );
    logit("e","\n\nEarthworm Program Errors:" );
    logit("e","\n1. Is the executable command %s spelled correctly?", commandLine );
    logit("e","\n\tTo correct this error, fix the command name in your %s file.\n", metaring->ConfigFile );
    logit("e","\n2. Are you running the Earthworm version you expected?" );
    logit("e","\n\tThese are your current Earthworm Environment Settings:" );
    logit("e","\n\t\tEW_HOME=%s", getenv("EW_HOME") );
    logit("e","\n\t\tEW_VERSION=%s", getenv("EW_VERSION") );
    logit("e","\n\t\tEW executable directory: %s/%s/bin",
          getenv("EW_HOME"), getenv("EW_VERSION") );
    logit("e","\n\tTo Correct this error, Modify the above values in your %s file, or similar local file, to the ", environmentScript );
    logit("e","correct EW_HOME, EW_VERSION, or EW executable directory values.\n" );
    logit("e","\n3. Is the executable command %s in the Earthworm executable directory: %s\\%s\\bin? ",
          commandLine, getenv("EW_HOME"), getenv("EW_VERSION") );
    logit("e","\n\tTo Correct this error, copy the above executable command to the Earthworm executable directory (listed above).\n" );
    logit("e","\n4. Is the Earthworm executable directory: %s/%s/bin in the PATH environment variable below?",
          getenv("EW_HOME"), getenv("EW_VERSION") );
    logit("e","\n\tThese are your current paths:\n\t\tPATH=%s", getenv("PATH") );
    logit("e","\n\tTo Correct this error, Modify the set path line in your %s, or similar local file, to include ", environmentScript );
    logit("e","the Earthworm executable directory.\n" );
    logit("e","\n\nNon-Earthworm Program Errors:" );
    logit("e","\n1. Does the non-Earthworm executable exist in any directory listed in PATH (listed above)?" );
    logit("e","\n\tTo Correct this error, Modify the set path line in your %s file, or similar local file, to include ", environmentScript );
    logit("e","the path to the non-Earthworm executable.\n\n" );
    logit("e","\n------------------------------------------\n" );
    /* logit("e","\nstartstop: Exiting due to error starting <%s>\n", commandLine );*/
    /* *nChild = iChild; */ /*reset nChild to the number of live children*/

#ifdef _WINNT
    return -1;
#else /* this is probably Solaris or Linux  */
    /* Send the kill flag to start Earthworm shutdown (Parent and other children) */
/* for ( i = 0; i < metaring->nRing; i++ )
        tport_putflag( &(metaring->Region[i]), TERMINATE ); stefan test*/

    /* Have this child exit */
    exit ( 1 );
#endif


}

  /******************************************************************
   *                         LockStartstop()                        *
   *                                                                *
   *  Create the lockfile which won't allow a second startstop      *
   *  to run if it uses a config files of the same name,            *
   *  ie: startstop_nt.d. After this routine is run, the lock is    *
   *  in place.                                                     *
   *                                                                *
   ******************************************************************/

void LockStartstop(char  *ConfigFile, char *ProgramName) {
   lockfile = ew_lockfile_path(ConfigFile); 
   if ( (lockfile_fd = ew_lockfile(lockfile) ) == -1) {
        fprintf(stderr, "Error creating lockfile. \nEither %s is already running, and only\n", ProgramName);
        fprintf(stderr, "one instance can run at the same time with the same config file, or \n");
        fprintf(stderr, "the EW_LOG directory %s\n", getenv("EW_LOG") ); 
        fprintf(stderr, "is not writable or does not exist.\n");
        exit(-1);
   }
}
  /******************************************************************
   *                         UnlockStartstop()                      *
   *                                                                *
   *  Unlock the lockfile which won't allow a second startstop      *
   *  to run if it uses a config files of the same name,            *
   *  ie: startstop_nt.d. After this routine is run, all locks are  *
   *  cleared out and startstop and be started up once again.       *
   *                                                                *
   ******************************************************************/

void UnlockStartstop() {
   ew_unlockfile(lockfile_fd);
   ew_unlink_lockfile(lockfile);
}

