
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: watchdog_client.c 7102 2018-02-13 00:22:53Z baker $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2006/01/17 21:38:06  friberg
 *     added 2 new .c files for global processing
 *
 *     Revision 1.4  2003/06/11 17:32:52  michelle
 *     added guts to reportError to write tp logit
 *
 *     Revision 1.3  2003/05/27 20:38:22  lucky
 *     Added cast to (char *) in logit_init call to comply with prototype
 *
 *     Revision 1.2  2003/05/22 23:33:46  michelle
 *     added call in reportErrorInit to logit_init
 *
 *     Revision 1.1  2003/05/22 21:47:31  michelle
 *     Initial revision
 *
 *
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <earthworm.h>
#include <watchdog_client.h>

static int iReportErrorBufferSize;

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
int   reportErrorInit (int  iBufferSize, int bLogToLocalFileFlag,
                      const char *callingProgramName)
{
   iReportErrorBufferSize = iBufferSize;

   /* what should the mid value be??? */
   logit_init ((char *) callingProgramName, 1, iBufferSize, bLogToLocalFileFlag);

   return EW_SUCCESS;
}


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
int   registerMe(int bRegisterFlag, long pid,
                  			long heartBeatInterval)
{
   int r_status = 0;
 
 
   return r_status;
}


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
int   reportError(int severityLevel, int shortDesc,
                 const char *messageFormat, ... )
{
   va_list ap;
   int r_status = 0;
   /* make this be the buffersize passed in on reportErrorInit */
#define MSGSTR_LEN 1024
   char  msgStr[MSGSTR_LEN];

   /* put the variable arg list into a formated string for passing onto logit */ 
   va_start(ap, messageFormat);
   vsnprintf( msgStr, MSGSTR_LEN, messageFormat, ap );
   va_end(ap);

   logit( "t", "%s", msgStr ); 
   return r_status;
}


