/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: findwave.c 7606 2018-11-26 20:47:34Z alexander $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.13  2005/05/23 21:13:56  davidk
 *     Fixed the USGE message that is issued when findwave is called without
 *     params, so that it lists the params in the proper order.
 *
 *     Revision 1.12  2005/03/22 17:00:03  davidk
 *     Fixed a bug, where findwave would set the record size for some tanks
 *     to values greater than MAX_TRACEBUF_SIZ, under some circumstances.
 *
 *     Revision 1.11  2005/03/17 20:04:53  davidk
 *     Merged NEIC changes into findwave:
 *       Option:  have tanks segmented by HD based on a HardDrive size
 *       Option:  have findwave truncate tanksizes to 1GB, to comply with
 *         wave_serverV maximum safe tank size.
 *       Option:  have findwave massage the record sizes to be a fixed size
 *         for all tanks.
 *       Option:  have findwave massage the record sizes for variable-record-length
 *         channels to be slightly larger than the largest record received for that channel.
 *     Merged code not run-tested.
 *
 *     Revision 1.10  2004/07/29 23:41:21  dietz
 *     increased TABLELEN from 1000 to 2000
 *
 *     Revision 1.9  2004/07/29 21:41:46  dietz
 *     added pinno to debug output
 *
 *     Revision 1.8  2004/07/29 20:58:23  dietz
 *     changed the sorting order: sort logo first, then SNLC
 *
 *     Revision 1.7  2004/07/29 18:59:04  dietz
 *     Added earthworm logo (both char string and numeric) to debug output.
 *     Changed waveserver output to list actual installation and moduleid
 *     (if installation is not the local installation, module is MOD_WILDCARD)
 *
 *     Revision 1.6  2004/07/28 23:38:00  dietz
 *     modified to work with both TYPE_TRACEBUF and TYPE_TRACEBUF2
 *
 */
 

  /*****************************************************************
   *                            FindWave.c                         *
   *                                                               *
   * Derived from sniffwave.c.                                     *
   * To listen to a ring, and write to a file the SCNL's of all    *
   * TRACE_BUF and TRACE_BUF2 messages found there.                *
   * Alex 2/22/2                                                   *
   *****************************************************************/

/* The file will be re-written periodically (every 5 sec)
   I'll run till killed 
   */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "earthworm.h"
#include "transport.h"
#include "swap.h"
#include "trace_buf.h"

#define TABLELEN 2000
typedef struct {
	TRACE2_HEADER TrHdr;
        MSG_LOGO      logo;
	int           maxMsg;
	int           minMsg;
	double        totalMsgTime;
	int           nMsg;
} TABLE ;

TABLE SCNLtable[TABLELEN];
int TableLast = 0;	/*Next available entry */

/* earthworm*d defined logo members */
unsigned char LocalInstId;
unsigned char TypeTracebuf;
unsigned char TypeTracebuf2;

/* strings for writing to output files */
char *Inst_Unlisted = "inst_unlisted";
char *Inst_Wild     = "INST_WILDCARD";
char *Mod_Unlisted  = "mod_unlisted";
char *Mod_NotLocal  = "mod_not_local";
char *Mod_Wild      = "MOD_WILDCARD";
char *Type_Unlisted = "type_unlisted";

/* Functions in this file: */
int  FindSCNL( TABLE* , TRACE2_HEADER*, MSG_LOGO* );
int  AddSCNL(TABLE* , TRACE2_HEADER*, int,  MSG_LOGO* );
void UpdateSCNL(TABLE*, TRACE2_HEADER* , int , int );
int  RewriteFile( FILE* , TABLE* );
int  compare( const void*, const void* );

/* Globals read from command line */
char     *index_size;
int      days_to_save;
char     output_type;
int      messageSize;
int      HDSizeMB;
char     bEnforceLimit;

int main( int argc, char *argv[] )
{
   SHM_INFO    region;
   long        RingKey;         /* Key to the transport ring to read from */
   MSG_LOGO    getlogo[2], logo;
   long        gotsize;
   TracePacket tpkt;
   char       *outFile, *inRing;
   short      *short_data;
   long       *long_data;
   int         rc;
   FILE       *fp;
   time_t      timeLastWrite;
   time_t      timeNow;
   long        writeInterval;
   int         indx;
   int         ret;

  /* Initialize things
  ********************/
  long_data  =  (long *)( tpkt.msg + sizeof(TRACE2_HEADER) ); 
  short_data = (short *)( tpkt.msg + sizeof(TRACE2_HEADER) );
  writeInterval = 5;	/* ******* NOTE *****/
  time(&timeLastWrite); /* set the re-write timer */

  /* Check command line argument 
  *****************************/
  if ( argc != 6 && argc != 9 )
  {
     fprintf(stderr,"Usage: %s <ring name> <index size> <days> <file name> <output type> [<msg size> <hd size> <enforce limit>]\n", 
             argv[0]);
     fprintf(stderr,"       output type = w for wave_server output; produces Tank commands\n" ); 
     fprintf(stderr,"                       to use in a wave_serverV configuration file.\n" );
     fprintf(stderr,"                       index size = waveserver index size,\n" );
     fprintf(stderr,"                       days = days to save per tank\n" );
     fprintf(stderr,"       output type = d for SCNL debug output. \n" );
     fprintf(stderr,"                       Must still provide index size and days.\n");
     fprintf(stderr,"         <msg size>      = the message size you wish to force all tanks to\n");
     fprintf(stderr,"          (-1 to not force message size)\n");
     fprintf(stderr,"          (-2 to produce \"padded\" message sizes)\n"); 
     fprintf(stderr,"          (\"padded\" message sizes = the max size seen + the difference\n");
     fprintf(stderr,"           between max message size and the min message size)\n");
     fprintf(stderr,"         <hd size>       = the size of the target harddrives in mb\n"); 
     fprintf(stderr,"          (-1 to not seperate out into harddrives),\n");
     fprintf(stderr,"         <enforce limit> = y to force tanks bigger than 1 gigabyte to\n");
     fprintf(stderr,"           1 gigabyte, n to not\n");
     exit( 1 );
  }
  inRing       = argv[1];
  outFile      = argv[4];
  index_size   = argv[2];
  days_to_save = atoi(argv[3]);
  output_type  = argv[5][0];

  if(argc == 9)
  {
    messageSize = atoi(argv[6]);
    if(messageSize > 0 && messageSize % 4)
    {
      fprintf(stderr,"<msg size> is %d.  It must be a multiple of 4.  Run findwave with no arguments for a USAGE description.\n",
              messageSize);
      exit(1);
    }

    HDSizeMB    = atoi(argv[7]);

    if(argv[8][0] == 'y' || argv[8][0] == 'Y')
      bEnforceLimit = TRUE;
    else
      bEnforceLimit = FALSE;
  }
  else
  {
    messageSize = -1;
    HDSizeMB    = -1;
    bEnforceLimit = FALSE;
  }
  
  fprintf(stdout, "Findwave %s, writing to %s\n", inRing, outFile);
  
  /* Open log file
  ****************/
  logit_init( "FindWave", 99, 256, 0 );

  /* Attach to ring
  *****************/
  if ((RingKey = GetKey( inRing )) == -1 )
  {
    logit( "e", "Invalid RingName; exiting!\n" );
    exit( -1 );
  }
  tport_attach( &region, RingKey );
  
  /* Create the output file
  *************************/
  fp = fopen( outFile, "w+");
  if (fp == NULL) 
  {
    logit("et","Cannot create file %s\n",outFile);
     exit(-1);
  }
   
  /* Look up local installation id
  *********************************/
  if ( GetLocalInst( &LocalInstId ) != 0 ) 
  {
     logit( "et", "Error getting local installation id\n" );
     exit( -1 );
  }

  /* Specify logos to get
  ***********************/
  if( GetType("TYPE_TRACEBUF2", &TypeTracebuf2) ) exit( -1 );
  if( GetType("TYPE_TRACEBUF",  &TypeTracebuf ) ) exit( -1 );

  if( GetInst("INST_WILDCARD",  &getlogo[0].instid) ) exit( -1 );
  if( GetModId("MOD_WILDCARD",  &getlogo[0].mod   ) ) exit( -1 );
  getlogo[0].type   = TypeTracebuf2;

  getlogo[1].instid = getlogo[0].instid;
  getlogo[1].mod    = getlogo[0].mod;
  getlogo[1].type   = TypeTracebuf;

  /* Initialize table
  *******************/
  memset( (void*)SCNLtable, 0, sizeof(TABLE) * TABLELEN );

  /* Flush the ring
  *****************/
  while ( tport_getmsg( &region, getlogo, (short)2, &logo, &gotsize,
			tpkt.msg, MAX_TRACEBUF_SIZ ) != GET_NONE );
  logit( "et", "FindWave: inRing flushed.\n" );
                          
  /* Listen to ring till told to quit
  ***********************************/
  while ( tport_getflag( &region ) != TERMINATE )
  {
    rc = tport_getmsg( &region, getlogo, (short)2,
		       &logo, &gotsize, tpkt.msg, MAX_TRACEBUF_SIZ );

    if ( rc == GET_NONE )
    {
      sleep_ew( 200 );
      continue;
    }

    if ( rc == GET_TOOBIG )
    {
      logit( "et", "FindWave: retrieved message too big (%ld) for msg\n",
	     gotsize );
      continue;
    }

    if ( rc == GET_NOTRACK )
      logit( "et", "FindWave: Tracking error.\n" );

    if ( rc == GET_MISS_LAPPED )
      logit( "et", "FindWave: Got lapped on the ring.\n" );

    if ( rc == GET_MISS_SEQGAP )
      logit( "et", "FindWave: Gap in sequence numbers\n" );

    if ( rc == GET_MISS )
      logit( "et", "FindWave: Missed messages\n" );

    WaveMsg2MakeLocal( &tpkt.trh2 );
	
   /* Have we seen this SCNL before?
    ********************************/
    indx = FindSCNL( SCNLtable, &tpkt.trh2, &logo );
    if (indx == -1 ) /* it's a new one */
    {
      ret = AddSCNL( SCNLtable, &tpkt.trh2, gotsize, &logo );
      if ( ret < 0 )
        logit( "et", "Table overflow. More than %d SCNLs found\n", TABLELEN );
    }
    else
    {
      UpdateSCNL(SCNLtable, &tpkt.trh2, indx, gotsize);
    }

   /* Re-write the file
    ********************/
    if  ( time(&timeNow) - timeLastWrite  >=  writeInterval ) 
    {
      timeLastWrite = timeNow;
      ret = RewriteFile( fp, SCNLtable ); 
      if (ret <0)
      {
        logit("et","Error re-writing file: %d\n",ret);
      }
    }
  }
  exit (0);
}
		
/******************************************************************************
 * FindSCNL()
 * To see if the SCNL in the message is already in the table. 
 * Return an index if so, otherwise return a -1.
 *****************************************************************************/
int FindSCNL( TABLE* SCNLtable, TRACE2_HEADER* msg, MSG_LOGO* logo )
{
   int i;
	
   for(i=0;i<TABLELEN;i++)
   {
      if( strcmp(SCNLtable[i].TrHdr.sta,  msg->sta)  != 0 ) continue;
      if( strcmp(SCNLtable[i].TrHdr.chan, msg->chan) != 0 ) continue;
      if( strcmp(SCNLtable[i].TrHdr.net,  msg->net)  != 0 ) continue;
      if( logo->type==TypeTracebuf2  && 
          strcmp(SCNLtable[i].TrHdr.loc,  msg->loc)  != 0 ) continue;
      if( SCNLtable[i].logo.instid != logo->instid )        continue;
      if( SCNLtable[i].logo.mod    != logo->mod    )        continue;
      if( SCNLtable[i].logo.type   != logo->type   )        continue;
      return(i);  /* found a match! */
   }
   return(-1);    /* no match */
}

/******************************************************************************
 * AddSCNL()
 * Add a new SCNL to the table
 *****************************************************************************/
int AddSCNL( TABLE* SCNLtable, TRACE2_HEADER* msg, int msgSize, MSG_LOGO* logo )
{
   if(TableLast == TABLELEN) return(-1); /* no more room */
	
   memcpy( (void*)&(SCNLtable[TableLast].TrHdr), msg,  sizeof(TRACE2_HEADER) );
   memcpy( (void*)&(SCNLtable[TableLast].logo),  logo, sizeof(MSG_LOGO)      );
   SCNLtable[TableLast].maxMsg  = msgSize;
   SCNLtable[TableLast].minMsg  = msgSize;
   SCNLtable[TableLast].totalMsgTime =   
                SCNLtable[TableLast].TrHdr.endtime        /* time of last data point  */
              - SCNLtable[TableLast].TrHdr.starttime      /* time of first data point */
              + 1.0/SCNLtable[TableLast].TrHdr.samprate;  /* one sample period        */
   SCNLtable[TableLast].nMsg    = 1;
   TableLast++;
 
/* Sort the table
 *****************/
   qsort( (void*)SCNLtable, TableLast, sizeof(TABLE), compare);

   return(0);
}

/*************************************************************
 * compare() function for qsort                              *
 *************************************************************/
int compare( const void* s1, const void* s2 )
{
   int rc;
   TABLE* t1 = (TABLE*) s1;
   TABLE* t2 = (TABLE*) s2;
 
   /* Sort by Earthworm logo first */
   if( t1->logo.instid < t2->logo.instid ) return(-1);
   if( t1->logo.instid > t2->logo.instid ) return( 1);

   if( t1->logo.mod   < t2->logo.mod     ) return(-1);
   if( t1->logo.mod   > t2->logo.mod     ) return( 1);

   if( t1->logo.type  < t2->logo.type    ) return(-1);
   if( t1->logo.type  > t2->logo.type    ) return( 1);

   /* Then sort by SCNL: */
   rc = strcmp( t1->TrHdr.sta,  t2->TrHdr.sta);
   if (rc != 0) return(rc);
   rc = strcmp( t1->TrHdr.net,  t2->TrHdr.net);
   if (rc != 0) return(rc);
   rc = strcmp( t1->TrHdr.loc,  t2->TrHdr.loc);
   if (rc != 0) return(rc);
   rc = strcmp( t1->TrHdr.chan, t2->TrHdr.chan);
   return(rc);
}

/******************************************************************************
   UPdateSCNL()
   Update statistics for an existing entry
 *****************************************************************************/
void UpdateSCNL(TABLE* SCNLtable, TRACE2_HEADER* msg, int index, int msgSize )
{
   if( SCNLtable[index].maxMsg < msgSize) SCNLtable[index].maxMsg = msgSize;
   if( SCNLtable[index].minMsg > msgSize) SCNLtable[index].minMsg = msgSize;

   /* Update average message time
    ******************************/
   SCNLtable[index].totalMsgTime =  SCNLtable[index].totalMsgTime 
                                  + SCNLtable[index].TrHdr.endtime
                                  - SCNLtable[index].TrHdr.starttime 
                                  + 1.0/SCNLtable[index].TrHdr.samprate;
   (SCNLtable[index].nMsg)++;

   return;
}

/*****************************************************************************
  RewriteFile()
  Dump the scnl table to the file
 *****************************************************************************/
int RewriteFile( FILE* fp, TABLE* SCNLtable)
{
   int i;
   double bytesPerDay, MBPerDay;
   int    totalMB, usedMB=0;
   int    iRecordSize;
   int    bTruncated;
   double dPercentFull;
   int    iHDCount = 1;

   /* character strings for writing to file */
   char  *inst;
   char  *modid;
   char  *msgtype;
   char   loc[3];

   rewind(fp);
   logit("e","Found %d SCNLs, press <Ctrl C> to halt.\n",TableLast);

   switch (output_type)
   {
   case 'd':
   case 'D':
            {
            /* write the column headings */
               fprintf(fp, "Findwave: Generating debug output:  Found %d SCNLs\n", TableLast);
               fprintf(fp, "                       MessageLen   Samp\n");
               fprintf(fp, "S     C   N  L  Pinno   Max   Min   Rate   nMsg  Mb/day     Earthworm Logo\n");
               break;
            }
   case 'w':
   case 'W':
   default:
            {
              fprintf(fp, "Findwave: Generating wave_serverV output:  Found %d SCNLs\n", TableLast);
              fprintf(fp, "#     Found %d SCNLs, Num Days = %d, Msg Size = %d, HD Size = %d,\n", 
                      TableLast, days_to_save, messageSize, HDSizeMB);  
              fprintf(fp, "#     Enforce 1 Gig limit = %s\n", bEnforceLimit ? "yes" : "no");
              fprintf(fp, "#    SCNL            Record            Logo             File Size   Index Size    File Name\n");
              fprintf(fp, "#    names            size     Instid        ModuleId   (megabytes) (max breaks)  (full path)\n");
            }
   }
	
   for (i=0; i<TableLast; i++)
   {
   /* set up character strings for writing to file */
      inst    = GetInstName (SCNLtable[i].logo.instid );
      modid   = GetModIdName(SCNLtable[i].logo.mod    );
      msgtype = GetTypeName (SCNLtable[i].logo.type   );

      bTruncated = FALSE;

      if( SCNLtable[i].logo.type == TypeTracebuf ) strcpy( loc, "  " );
      else  strcpy( loc, SCNLtable[i].TrHdr.loc );

   /* compute Record size */
      if(messageSize > 0)
        iRecordSize = messageSize;
      else if(messageSize == -2)
      {
        iRecordSize = SCNLtable[i].maxMsg + SCNLtable[i].maxMsg - SCNLtable[i].minMsg;
        if(iRecordSize > MAX_TRACEBUF_SIZ)
          iRecordSize = MAX_TRACEBUF_SIZ;
      }
      else
        iRecordSize = SCNLtable[i].maxMsg;

   /* compute bytes per day of WaveServer tank */
      bytesPerDay = (  (double)(24*60*60)                                /* secs/day */   
                     / (SCNLtable[i].totalMsgTime / SCNLtable[i].nMsg))  /* secs/msg */
                     * iRecordSize;                                      /* bytes/msg */
      MBPerDay = bytesPerDay / 1000000.;
      totalMB  = (int)(MBPerDay * days_to_save + 0.999);


      if (bEnforceLimit)
      {
        if (totalMB > 1073) /* 1 GB = 1073741824, max ws tank size = 1073741823 */
        {
          logit("","# size for (%s:%s:%s:%s) is %dMB.  Truncating to 1GB.\n", 
            SCNLtable[i].TrHdr.sta, SCNLtable[i].TrHdr.chan, 
            SCNLtable[i].TrHdr.net, loc,
            totalMB);
          totalMB = 1073;
          bTruncated = TRUE;
        }
      }
      
      usedMB += totalMB;
      
      if (HDSizeMB > 0)
      {
        if (usedMB > HDSizeMB)
        {
          dPercentFull =  ((double)(usedMB - totalMB)) / HDSizeMB * 100.;
          
          fprintf(fp, "#\n# Harddrive #%d full, %5dMB/%5dMB: %3.2f%% utilization.\n",
            iHDCount, usedMB - totalMB, HDSizeMB, dPercentFull);
          fprintf(fp, "#\n#\n");
          fprintf(fp, "# Findwave: Harddrive #%d:\n#\n", iHDCount+1);
          
          usedMB = totalMB;
          iHDCount++;
        }
      }
      


      switch (output_type)
      {
      case 'd':
      case 'D':
               {
                  if( inst    == NULL ) inst    = Inst_Unlisted;
                  if( modid   == NULL ) modid   = Mod_Unlisted;
                  if( SCNLtable[i].logo.instid != LocalInstId ) modid = Mod_NotLocal;
                  if( msgtype == NULL ) msgtype = Type_Unlisted;

                  fprintf(fp, "%-5s %-3s %-2s %-2s %5d  %4d  %4d  %5.1f  %5d  %6.1f"
                              "  %s %s %s (i%u m%u t%u)\n", 
                           SCNLtable[i].TrHdr.sta, SCNLtable[i].TrHdr.chan, 
                           SCNLtable[i].TrHdr.net, loc, SCNLtable[i].TrHdr.pinno,
                           SCNLtable[i].maxMsg, SCNLtable[i].minMsg, 
                           SCNLtable[i].TrHdr.samprate, SCNLtable[i].nMsg,
                           bytesPerDay, inst, modid, msgtype,
                           SCNLtable[i].logo.instid, SCNLtable[i].logo.mod,
                           SCNLtable[i].logo.type);
                  break;
               }
      case 'w':
      default:
               {
                  char tankloc[4]="_";

                  if( SCNLtable[i].logo.type == TypeTracebuf ) strcpy( tankloc, "" );
                  else  strcat( tankloc, SCNLtable[i].TrHdr.loc );

                  if( inst  == NULL ) inst  = Inst_Wild;
                  if( modid == NULL ) modid = Mod_Wild;
                  if( SCNLtable[i].logo.instid != LocalInstId ) modid = Mod_Wild;

                  fprintf(fp, "Tank %-5s %-3s %-2s %-2s  %4d  %-15s %-15s %5d  %s  <insert path here>%s_%s_%s%s.tnk%s\n", 
                           SCNLtable[i].TrHdr.sta, SCNLtable[i].TrHdr.chan, 
                           SCNLtable[i].TrHdr.net, loc,  
                           SCNLtable[i].maxMsg, inst, modid,
                           totalMB, index_size, SCNLtable[i].TrHdr.sta,
                           SCNLtable[i].TrHdr.chan, SCNLtable[i].TrHdr.net, tankloc,
                           bTruncated ? "  #TRUNCATED" : "");
               }
      }
   }
   return(0);
}
