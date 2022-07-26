/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: watchdog_client.h 4452 2011-07-27 20:15:34Z stefan $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2006/01/17 17:27:29  friberg
 *     added from MWithers work on global routines
 *
 *     Revision 1.7  2003/06/24 20:30:23  dhanych
 *     added NORESULT
 *
 *     Revision 1.6  2003/06/24 19:30:52  dhanych
 *     added PPINVAL, changed GENFATERR code to clear the ceiling
 *
 *     Revision 1.5  2003/06/24 19:28:56  lucky
 *     *** empty log message ***
 *
 *     Revision 1.4  2003/05/22 17:16:46  lucky
 *     Changed prototypes to return int;  Added some more short strings
 *
 *     Revision 1.3  2003/05/22 16:28:06  dhanych
 *     Added Waveserver error descriptions
 *
 *     Revision 1.2  2003/05/22 15:47:09  michelle
 *     made messageFormat string a const in reportError signature
 *
 *     Revision 1.1  2003/05/21 15:00:33  michelle
 *     Initial revision
 *
 *     Revision 1.2  2003/05/20 22:53:41  michelle
 *     made short descs ints, for ease of comparison, may need to add corresponding set of strings for dumping out to log file and in theory to email sent as notifications to humans
 *
 *     Revision 1.1  2003/05/20 21:15:41  michelle
 *     Initial revision
 *
 *
 */

#ifndef WATCHDOG_CLIENT_H
#define WATCHDOG_CLIENT_H

/* Prototypes for functions in watchdog.c
 ***********************************/

/*****************************
 * reportErrorInit
 * initializes the error reporting feature,
 * which includes calling logit_Init with
 * appropriate parameters.
 * reportErrorInit also determines the local machine name
 * via local system commands/environment variables.
 * the machine name will be used along with the
 * callingProgramName to uniquely identify applications.
 *
 * @param iBufferSize - is the max size of an
 *        error description that will be supplied
 *        to reportError for logging and for
 *        UDP distribution
 * @param logToLocalFileFlag - indicates whether to
 *        log to a local file or not.
 * @param callingProgramName - is used in the
 *        error report and log to identify where
 *        the error was logged from
 *******************************************/
int   reportErrorInit( int  iBufferSize
                      , int bLogToLocalFileFlag
                      , const char *callingProgramName );



/*****************************
 * registerMe
 * Broadcasts a UDP message intended for the
 * Watch Dog(s) to pick up and thus add the module
 * to its list of monitored applications.
 * registerMe determines the local machine name
 * via local system commands/environment variables.
 * the machine name will be used along with the
 * callingProgramName and pid (process id) to uniquely
 * identify applications.
 * Note that the Watch Dog should log/indicate
 * which applications it is monitoring, thus
 * to confirm monitoring or lack there of is
 * occurring the person configurating the
 * system should manually check the watch dog
 * accordingly
 *
 * @param registerFlag - indicates to register
 *        or to unregister the calling program
 *        for monitoring
 * @param pid - process id to uniquely identify
 *        the calling program/application
 * @param heartBeatInterval - the interval in secs
 *        of how often the calling program or
 *        application's heartbeat should be
 *        distributed
 *******************************************/
int   registerMe(int bRegisterFlag
                  , long pid
                  , long heartBeatInterval);



/*****************************
 * reportError
 * Logs errors to a local log file
 * based on directives set by reportErrorInit,
 * and it broadcasts a UDP message of the
 * error. reportError will include (based on
 * initialization) the callingProgramName and
 * machine name in the distributed UDP error messages
 * in order to uniquely identify the source of the
 * reported error. each processing machine will spew
 * errors through its primary network connection
 * (i.e., the one used to get to the DB). Thus it
 * is expected that the watchdog(s) will listen on
 * all subnets.
 *
 * @param severityLevel - is WD_SEVERITY_LEVEL
 *        indicates debug, info, warning, fatal
 * @param shortDesc - is a defined int that
 *        indicates type of error and will be used
 *        by the error notification system to
 *        determine who should be notified
 *        note these are ints for ease of comaprison
 * @param messageFormat - identical to printf
 *        indicates format of message string
 * @param remainder of params map to messageFormat
 *        just as is done in printf
 *******************************************/
int   reportError(int severityLevel
                 , int shortDesc
                 , const char *messageFormat
                 , ... );




/*****************************
 * Severity Levels
 *******************************************/
enum WD_SEVERITY_LEVEL
{
    WD_DEBUG = 1            /* Plethora of debug information(think kitchen sink).
                               OFF by default,
                               Only logged if reportErrorSetDebug(1) is called.
                               Timestamped: NO
                               EXAMPLE:  "Top of for loop for each passport Entry" */

  , WD_INFO = 0             /* Lots of program information and processing milestones,
                               ON - logged to file by default.  Turn off with reportErrorSetLogInfo(0).
                               Timestamped: NO
                               EXAMPLE:   "Filter coefficients:   1 -6.39235757e-003"*/

  , WD_MAJOR_INFO = 2       /* Major milestones, (such as per channel results in mag calculators),
                               and expected minor errors (such as couldn't get data for a channel.
                               ON - logged to file
                               Timestamped: NO
                               EXAMPLE:   "<YKW3:BHZ:CN:--> not on the sanctioned stations list; skipping! "*/

  , WD_WARNING_ERROR = -1   /* Minor problems/errors.
                               ON - ERROR Wire, Screen, Logged to file.
                               Timestamped: YES
                               EXAMPLE:   "Could not connect to wave_server 192.168.0.1:16024"
                               EXAMPLE:   "Channel <YKW3:BHZ:CN:--> not found in DB." "*/

  , WD_FATAL_ERROR = -2     /* MAJOR PROBLEMS/ERRORS.  - Show stoppers!
                               ON - ERROR Wire, Screen, Logged to file.
                               Timestamped: YES
                               EXAMPLE:   "Could not connect to DB: hdb_main@eqs.red"
                               EXAMPLE:   "hdb_api_CreateMagnitude() failed for  Mwp 6.5, 20 channels" */

};

/*****************************
 * Error Short Descriptions
 *******************************************/

#define GENFATERR   1      /* Generic fatal error (ha!) */

#define NORESULT    90     /* Unable to obtain result */

/****** Station List Error Short Descriptions */
#define STAERR   100     /* Generic Station List Error */
#define STAERRS  "STAERR"     /* Generic Station List Error */
#define STANFND  101     /* Station Not Found */

/****** Database Error Short Descriptions */
#define DBERR    200     /* Generic Database Error */
#define DBCONN   201     /* Cannot Connect to DB */

/****** Passport Error Short Descriptions */
#define PPERR    300     /* passport read problem */
#define PPREAD   301     /* passport read problem */
#define PPUNKN   302     /* unknown passport line */
#define PPMISS   303     /* missing a required passport line */
#define PPINVAL  304     /* invalid passport */

/****** Command Line Args Error Short Descriptions */
#define CMDLNERR 400     /* command line parameter problem */
#define CFGERR   420     /* Configuration file error */

/****** System Error Short Descriptions */
#define SYSERR   500     /* memory allocation failure */
#define MEMALLOC 501     /* memory allocation failure */
#define CLASINST 502     /* failed to instantiate a working class */

#define WSERR    600       /* Waveserver error */
#define WSCONN   601       /* WS connection error */

/****** Earthworm Related Errors Short Descriptions */
#define EWERR    700       /* Generic earthworm error */

/****** OTHER NEEDED Short descriptions */



#endif /* WATCHDOG_CLIENT_H */
