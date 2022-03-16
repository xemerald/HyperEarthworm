
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: stopmodule.c 6803 2016-09-09 06:06:39Z et $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.6  2010/03/09 20:22:44  stefan
 *      comments
 *
 *     Revision 1.5  2010/03/09 19:07:05  stefan
 *      comments
 *
 *     Revision 1.4  2007/03/28 18:22:46  paulf
 *     added _MACOSX flag
 *
 *     Revision 1.3  2007/02/20 22:36:31  stefan
 *     added feedback on successfully sent message
 *
 *     Revision 1.2  2007/02/20 22:15:56  stefan
 *     name issues
 *
 *     Revision 1.1  2007/02/20 22:01:45  stefan
 *     stopmodule first revision
 *
 *
 *
 */


/*****************************************************************************
 *                       stopmodule
 *    Program to stop an Earthworm module while startstop is running
 *
 *    stopmodule:
 *    sends a TYPE_STOP message to the first RING listed in startstop_*.d with
 *    a message payload of the process id of the module to stop
 *
 *
 *    startstop:
 *    sees TYPE_STOP and runs StopChild. This should kill the child
 *    if it isn't already marked internally with  a "Stopped" status
 *    If successful it sets the status for the module as "Stopped"
 *
 *    statmgr:
 *    sees a TYPE_STOP messge and sets a module's restart status to STOPPED.
 *    Until statmgr sees a TYPE_RESTART message for a stopped module, it
 *    will not request a restart of the module.
 *
 *****************************************************************************/

/* 5/21/98: changed to allow config file to be specified on the command line.
 * If none is specified, then a default file is used depending on the OS.
 * PNL, UW Geophysics. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <kom.h>
#include <transport.h>
#include <earthworm.h>

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

void SendstopRequest( char* );
void ReadRingName( char*, char* );

SHM_INFO   region;                /* The shared memory region   */
static unsigned char InstId;      /* local installation id      */
static unsigned char MyModId;
static unsigned char TypeStop;
static int ringKey;

int main( int argc, char *argv[] )
{
  char *pid;
  char *runPath;
  char  ringname[MAX_RING_STR];
  char  configFile[FILENAME_MAX] = DEF_CONFIG;

/* Check the number of arguments.
   ******************************/
  switch( argc )
  {
  case 3:
    strcpy ( configFile, argv[1] );
    pid = argv[2];
    break;
  case 2:
    pid = argv[1];
    break;
  default:
    fprintf( stderr, "Usage: stopmodule [config_file] <pid>\n" );
    return -1;
  }


/* Change working directory to environment variable EW_PARAMS value
   ****************************************************************/
  runPath = getenv( "EW_PARAMS" );

  if ( runPath == NULL )
  {
      fprintf(stderr, "stopmodule: Environment variable EW_PARAMS not defined." );
      fprintf(stderr, " Exiting.\n" );
      return -1;
  }

  if ( *runPath == '\0' )
  {
      fprintf(stderr, "stopmodule: Environment variable EW_PARAMS " );
      fprintf(stderr, "defined, but has no value. Exiting.\n" );
      return -1;
  }

  if ( chdir_ew( runPath ) == -1 )
  {
      fprintf(stderr, "stopmodule: Params directory not found: %s\n", runPath );
      fprintf(stderr, "stopmodule: Reset environment variable EW_PARAMS." );
      fprintf(stderr, " Exiting.\n" );
      return -1;
  }

/* Look up ids in earthworm.d tables
 ***********************************/
   if ( GetLocalInst( &InstId ) != 0 )
   {
      fprintf( stderr,
               "stop: error getting local installation id; exiting!\n" );
      return -1;
   }

   if ( GetType( "TYPE_STOP", &TypeStop ) != 0 )
   {
      fprintf( stderr,
               "stopmodule: Invalid message type <TYPE_STOP> exiting!\n" );
      return -1;
   }

/* Read startstop's config file to find a valid transport ring name
 ******************************************************************/
   ReadRingName( ringname, configFile );

   if( (ringKey = GetKey(ringname)) == -1 )
   {
       printf( "stopmodule: Invalid ring name <%s>. Exiting.\n",
                ringname );
       return -1;
   }

/* Attach to shared memory ring
   *****************************/
   tport_attach( &region, ringKey );

   SendstopRequest( pid );
   printf( "stopmodule: sent a request to startstop to stop the pid %s\n",pid);
   printf( "            the module may take a little while to die\n");
   printf( "            check startstop's status window for the next\n");
   printf( "            thirty seconds or so to confirm success.\n");
   tport_detach( &region );
   return 0;
}

  /************************************************************
   *                   SendstopRequest                     *
   *  To send a message requesting the stoping of a module *
   ************************************************************/
void SendstopRequest( char*  modPid ) /* pid of module as ascii string*/
{
   MSG_LOGO logo;
   char message[512];

/* Build message
   *************/
   strcpy (message,modPid);
   strcat( message, "\n\0" );

/* Set logo values of pager message
   ********************************/
   logo.type   = TypeStop;
   logo.mod    = MyModId;
   logo.instid = InstId;

/* Send stop message to transport ring
   **************************************/
   if ( tport_putmsg( &region, &logo, (long)strlen(message), message ) != PUT_OK )
      fprintf(stderr, "stop: Error sending message to transport region.\n" );

   return;

}

  /*************************************************************
   *                      ReadRingName                         *
   * Read startstop's configfile as far as the first ring name *
   *************************************************************/
void ReadRingName( char *ringname, char *configFile )
{
   int        nfiles;
   char      *com, *str;

   nfiles = k_open( configFile );
   if ( nfiles == 0 )
   {
        printf( "stopmodule: Error opening file <%s>. Exiting.\n",
                 configFile );
        return;
   }

/* Process Ring commands from startstop's command file
   ***************************************************/
   while(nfiles > 0)          /* While there are command files open */
   {
        while(k_rd())         /* Read next line from active file  */
        {
            com = k_str();    /* Get the first token from line */

        /* Ignore blank lines & comments
           *****************************/
            if( !com )           continue;
            if( com[0] == '#' )  continue;

        /* Process only 1st Ring command
           *****************************/
           if( k_its( "Ring" ) )
           {
                str = k_str();
                strcpy( ringname, str );
                break;
           }

        /* See if there were any errors processing this command
           ****************************************************/
           if( k_err() )
           {
               printf( "stopmodule: Bad <%s> command in <%s>. Exiting.\n",
                        com, configFile );
               return;
           }
        }
        nfiles = k_close();
   }

   return;
}
