/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: config.c 7387 2018-05-02 01:28:01Z baker $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.12  2005/04/11 17:56:32  dietz
 *       Added new optional command: GetLogo <instid> <modid> <msgtype>
 *     If no GetLogo cmds are given, gaplist will process all TYPE_TRACEBUF
 *     and TYPE_TRACEBUF2 messages (previous behavior).
 *       Modified to allow processing of TYPE_TRACE_COMP_UA and TYPE_TRACE2_COMP_UA
 *     messages (headers are NOT compressed!).
 *
 *     Revision 1.11  2004/05/14 22:44:54  kohler
 *     Yet more defensive programming. WMK
 *
 *     Revision 1.10  2004/05/14 22:08:48  kohler
 *     More defensive programming.  WMK
 *
 *     Revision 1.9  2004/05/14 20:25:02  kohler
 *     Defensive programming to prevent buffer overflow.  WMK
 *
 *     Revision 1.8  2004/05/11 17:04:23  kohler
 *     Now, the scnl array is allocated using realloc, so any number of Scnl and
 *     Label lines are permitted in the config file.
 *
 *     Fixed two bugs:
 *     Label strings longer than 31 characters weren't null-terminated in the
 *     program.  Now, they are, so long labels are correctly truncated.
 *
 *     The label array, defined in gaplist.h, had a length of 31 characters,
 *     which caused some values in the scnl array to get clobbered.  This resulted
 *     in continuous dead/alive messages being sent to statmgr.
 *
 *     Revision 1.7  2004/04/21 19:00:56  kohler
 *     gaplist now looks for gaps in TYPE_TRACEBUF2 message, ie trace messages
 *     that contain location codes.  The program ignores TYPE_TRACEBUF messages.
 *     WMK 4/21/04
 *
 *     Revision 1.6  2002/05/15 16:45:30  patton
 *     Made Logit changes.
 *
 *     Revision 1.5  2001/12/07 22:46:23  dietz
 *     added Label command (optional)
 *
 *     Revision 1.4  2001/04/19 19:14:26  kohler
 *     Config file now read twice.  First time to get number of Scn lines.
 *
 *     Revision 1.3  2000/11/20 19:20:41  kohler
 *     Added new parameter: ReportDeadChan
 *
 *     Revision 1.2  2000/04/18 23:27:52  dietz
 *     reads new optional config command, MyModuleId
 *
 *     Revision 1.1  2000/02/14 17:39:05  lucky
 *     Initial revision
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "earthworm.h"
#include "kom.h"
#include "transport.h"
#include "trace_buf.h"
#include "gaplist.h"

#define ncommand 4           /* Number of commands in the config file */

int  nScnl = 0;              /* Number of scnl's to monitor, including label lines */
SCNL *scnl = NULL;           /* Array of scnl structures */


   /*****************************************************************
    *                          GetConfig()                          *
    *         Processes command file using kom.c functions.         *
    *****************************************************************/

void GetConfig( char *config_file, GPARM *Gparm )
{
   char init[ncommand];     /* Flags, one for each command */
   int  nmiss;              /* Number of commands that were missed */
   int  nfiles;
   int  i;
   int  scnlSize = 0;       /* Num bytes in scnl array */

/* Set to zero one init flag for each required command
   ***************************************************/
   for ( i = 0; i < ncommand; i++ ) init[i] = 0;

/* Initialize Configuration parameters
   ***********************************/
   Gparm->MyModName[0]   = '\0';
   Gparm->ReportDeadChan = 10.0;      /* Minutes */
   Gparm->nGetLogo       = 0;
   Gparm->GetLogo        = NULL;

/* Open the main configuration file
   ********************************/
   nfiles = k_open( config_file );
   if ( nfiles == 0 )
   {
      logit( "e", "gaplist: Error opening configuration file <%s> Exiting.\n",
              config_file );
      exit( -1 );
   }

/* Process all nested configuration files
   **************************************/
   while ( nfiles > 0 )          /* While there are config files open */
   {
      while ( k_rd() )           /* Read next line from active file  */
      {
         int  success;
         char *com;
         char *str;

         com = k_str();          /* Get the first token from line */

         if ( com == NULL )   continue;    /* Ignore blank lines */
         if ( com[0] == '#' ) continue;    /* Ignore comments */

/* Open another configuration file
   *******************************/
         if ( com[0] == '@' )
         {
            success = nfiles + 1;
            nfiles  = k_open( &com[1] );
            if ( nfiles != success )
            {
               logit( "e", "gaplist: Error opening command file <%s>. Exiting.\n",
                       &com[1] );
               exit( -1 );
            }
            continue;
         }

/* Read configuration parameters
   *****************************/
         else if ( k_its( "InRing" ) )
         {
            str = k_str();
            if ( str != NULL )
            {
               strncpy( Gparm->InRing, str, MAX_RING_STR );
               Gparm->InRing[MAX_RING_STR-1] = '\0';

               if( (Gparm->InKey = GetKey(str)) == -1 )
               {
                  logit( "e", "gaplist: Invalid InRing name <%s>. Exiting.\n", str );
                  exit( -1 );
               }
            }
            init[0] = 1;
         }

         else if ( k_its( "HeartbeatInt" ) )
         {
            Gparm->HeartbeatInt = k_int();
            init[1] = 1;
         }

         else if ( k_its( "MinGapToLog" ) )
         {
            Gparm->MinGapToLog = k_val();
            init[2] = 1;
         }

         else if ( k_its( "TablePrintInterval" ) )
         {
            Gparm->TablePrintInterval = k_int();
            init[3] = 1;
         }

         else if ( k_its( "ReportDeadChan" ) )
            Gparm->ReportDeadChan = k_val();

         else if ( k_its( "MyModuleId" ) )
         {
            str = k_str();
            if ( str != NULL )
            {
               strncpy( Gparm->MyModName, str, MAX_MOD_STR );
               Gparm->MyModName[MAX_MOD_STR-1] = '\0';
            }
         }

 /*opt*/ else if ( k_its( "GetLogo" ) )
         {
            MSG_LOGO *tlogo = NULL;
            int       nlogo = Gparm->nGetLogo;
            tlogo = (MSG_LOGO *)realloc( Gparm->GetLogo, 
                                         (nlogo+1)*sizeof(MSG_LOGO) );
            if( tlogo == NULL )
            {
               logit( "e", "gaplist: GetLogo: error reallocing %zu bytes;"
                      " exiting!\n", (nlogo+1)*sizeof(MSG_LOGO) );
               exit( -1 );
            }
            Gparm->GetLogo = tlogo;

            str = k_str(); /* read instid */
            if ( str != NULL )
            {
               if( GetInst( str, &(Gparm->GetLogo[nlogo].instid) ) != 0 )
               {
                  logit( "e", "gaplist: Invalid installation name <%s>"
                         " in <GetLogo> cmd; exiting!\n", str );
                  exit( -1 );
               }
               str = k_str(); /* read module id */
               if ( str != NULL )
               {
                  if( GetModId( str, &(Gparm->GetLogo[nlogo].mod) ) != 0 )
                  {
                     logit( "e", "gaplist: Invalid module name <%s>"
                            " in <GetLogo> cmd; exiting!\n", str );
                     exit( -1 );

                  }
                  str = k_str(); /* read message type */
                  if ( str != NULL )
                  {
                     if( strcmp(str,"TYPE_TRACEBUF")      !=0 &&
                         strcmp(str,"TYPE_TRACEBUF2")     !=0 && 
                         strcmp(str,"TYPE_TRACE_COMP_UA") !=0 &&
                         strcmp(str,"TYPE_TRACE2_COMP_UA")!=0    )
                     {
                        logit( "e","gaplist: Invalid message type <%s> in <GetLogo>"
                               " cmd; must be TYPE_TRACEBUF, TYPE_TRACEBUF2,"
                               " TYPE_TRACE_COMP_UA, or TYPE_TRACE2_COMP_UA;"
                               " exiting!\n", str );
                        exit( -1 );
                     }
                     if( GetType( str, &(Gparm->GetLogo[nlogo].type) ) != 0 ) {
                        logit( "e", "gaplist: Invalid message type <%s>"
                               " in <GetLogo> cmd; exiting!\n", str );
                        exit( -1 );
                     }
                     Gparm->nGetLogo++;
                  } /* end if msgtype */
               } /* end if modid */
            } /* end if instid */
         } /* end GetLogo cmd */


         else if ( k_its( "Scnl" ) )
         {
            int i;
            SCNL *scnlPtr;
            scnlSize += sizeof( SCNL );
            scnlPtr = (SCNL *)realloc( scnl, scnlSize );
            if ( scnlPtr ==  NULL )
            {
               printf( "gaplist: realloc() error in getconfig(). Exiting.\n" );
               exit( -1 );
            }
            scnl = scnlPtr;
            scnl[nScnl].isLabel = 0;              /* Not a label */
            strncpy( scnl[nScnl].sta,  k_str(), TRACE2_STA_LEN );
            strncpy( scnl[nScnl].chan, k_str(), TRACE2_CHAN_LEN );
            strncpy( scnl[nScnl].net,  k_str(), TRACE2_NET_LEN );
            strncpy( scnl[nScnl].loc,  k_str(), TRACE2_LOC_LEN );
            scnl[nScnl].sta[TRACE2_STA_LEN-1]   = '\0';
            scnl[nScnl].chan[TRACE2_CHAN_LEN-1] = '\0';
            scnl[nScnl].net[TRACE2_NET_LEN-1]   = '\0';
            scnl[nScnl].loc[TRACE2_LOC_LEN-1]   = '\0';
            scnl[nScnl].alive       = 0;          /* 0 or 1 */
            scnl[nScnl].status      = 1;          /* 0, 1, or -1 */
            scnl[nScnl].EndTimePrev = -1.0;
            scnl[nScnl].totalGap    = 0.0;
            scnl[nScnl].timeOfDeath = 0;
            for ( i = 0; i < NWINDOW; i++ )
               scnl[nScnl].count[i] = 0;
            nScnl++;
         }

         else if ( k_its( "Label" ) )
         {
            SCNL *scnlPtr;
            scnlSize += sizeof( SCNL );
            scnlPtr = (SCNL *)realloc( scnl, scnlSize );
            if ( scnlPtr ==  NULL )
            {
               printf( "gaplist: realloc() error in getconfig(). Exiting.\n" );
               exit( -1 );
            }
            scnl = scnlPtr;
            scnl[nScnl].isLabel = 1;
            strncpy( scnl[nScnl].label, k_str(), LABEL_LEN );
            scnl[nScnl].label[LABEL_LEN-1] = '\0';    /* Null-terminate */
            nScnl++;
         }

/* An unknown parameter was encountered
   ************************************/
         else
         {
            logit( "e", "gaplist: <%s> unknown parameter in <%s>\n",
                    com, config_file );
            continue;
         }

/* See if there were any errors processing the command
   ***************************************************/
         if ( k_err() )
         {
            logit( "e", "gaplist: Bad <%s> command in <%s>. Exiting.\n", com,
                    config_file );
            exit( -1 );
         }
      }
      nfiles = k_close();
   }

/* After all files are closed, check flags for missed commands
   ***********************************************************/
   nmiss = 0;
   for ( i = 0; i < ncommand; i++ )
      if ( !init[i] )
         nmiss++;

   if ( nmiss > 0 )
   {
      logit( "e", "gaplist: ERROR, no " );
      if ( !init[0] ) logit( "e", "<InRing> " );
      if ( !init[1] ) logit( "e", "<HeartbeatInt> " );
      if ( !init[2] ) logit( "e", "<MinGapToLog> " );
      if ( !init[3] ) logit( "e", "<TablePrintInterval> " );
      logit( "e", "command(s) in <%s>. Exiting.\n", config_file );
      exit( -1 );
   }
   return;
}


   /**************************************************************
    *                         LogConfig()                        *
    *              Log the configuration parameters              *
    **************************************************************/

void LogConfig( GPARM *Gparm )
{
   int i;

   logit( "", "\n" );
   logit( "", "InRing:               %s\n",   Gparm->InRing );
   if( strlen(Gparm->MyModName) )
      logit( "", "MyModName:            %s\n",   Gparm->MyModName );
   logit( "", "HeartbeatInt:        %6d\n",   Gparm->HeartbeatInt );
   logit( "", "TablePrintInterval:  %6d\n",   Gparm->TablePrintInterval );
   logit( "", "MinGapToLog:      %9.2lf\n",   Gparm->MinGapToLog );
   logit( "", "nGetLogo:            %6d\n",   Gparm->nGetLogo );
   for( i=0; i<Gparm->nGetLogo; i++ ) {
      logit( "", "GetLogo[%d]:   i%u m%u t%u\n", i,
            Gparm->GetLogo[i].instid, Gparm->GetLogo[i].mod,
            Gparm->GetLogo[i].type );
   }
   logit( "", "nScnl:               %6d\n",   nScnl );
   for ( i = 0; i < nScnl; i++ ) {
      if ( scnl[i].isLabel ) {
         logit( "", "Label    %s\n", scnl[i].label );
      } else {
         logit( "", "Scnl     %5s %3s %2s %2s\n",
               scnl[i].sta, scnl[i].chan, scnl[i].net, scnl[i].loc );
      }
   }
   logit( "", "\n" );
   return;
}
