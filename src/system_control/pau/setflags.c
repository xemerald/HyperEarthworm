/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: setflags.c 644 2001-05-11 20:46:13Z dietz $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2001/05/11 20:45:42  dietz
 *     Initial revision
 *
 *
 *
 */

/*  setflags.c   does most of the work for pau & paupid.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <earthworm.h>
#include <kom.h>
#include <transport.h>
#include <earthworm.h>

int setflags( char *execname, char *cfgfile, int flagvalue )
{
   SHM_INFO   region;
   long       key;
   char      *com;
   char      *ringname;
   char      *runPath;
   int        nfiles;

/* Change working directory to environment variable EW_PARAMS value
   ***************************************************************/
   runPath = getenv( "EW_PARAMS" );

   if ( runPath == (char *)NULL )
   {
      printf( "%s: Environment variable EW_PARAMS not defined;"  
              " exiting!\n", execname );
      return -1;
   }

   if ( *runPath == '\0' )
   {
      printf( "%s: Environment variable EW_PARAMS "
              "defined, but has no value; exiting!\n", execname );
      return -1;
   }

   if ( chdir_ew( runPath ) == -1 )
   {
      printf( "%s: Params directory not found: %s\n", execname, runPath );
      printf( "%s: Reset environment variable EW_PARAMS;"
              " exiting!\n", execname );
      return -1;
   }

   nfiles = k_open( cfgfile );
   if ( nfiles == 0 )
   {
      printf( "%s: Error opening file <%s>; exiting!\n",
               execname, cfgfile );
      return -1;
   }

/* Process Ring commands from startstop's command file
   ***************************************************/
   while(nfiles > 0)              /* While there are command files open */
   {
        while(k_rd())             /* Read next line from active file  */
        {
            com = k_str();        /* Get the first token from line */

        /* Ignore blank lines & comments
           *****************************/
            if( !com )           continue;
            if( com[0] == '#' )  continue;

        /* Process only Ring commands
           **************************/
           if( k_its( "Ring" ) )
           {
                ringname = k_str();
                if( !ringname )  break;
                if( (key = GetKey(ringname)) == -1 )
                {
                   printf( "%s: Invalid ring name <%s>; exiting!\n",
                            execname, ringname );
                   exit( -1 );
                }
                tport_attach ( &region, key );
                tport_putflag( &region, flagvalue );
                printf( "%s:  Set termination flag in %s, key:%ld\n",
	                 execname,  ringname, key );
                tport_detach ( &region );
           }

        /* See if there were any errors processing this command
           ****************************************************/
            if( k_err() )
            {
               printf( "%s: Bad <%s> command in <%s>; exiting!\n",
                       execname, com, cfgfile );
               return -1;
            }
        }
        nfiles = k_close();
   }
   return 0;
}
 
