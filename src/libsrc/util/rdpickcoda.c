/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: rdpickcoda.c 1475 2004-05-14 23:47:06Z dietz $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.2  2004/05/14 23:47:05  dietz
 *     unreferenced variable cleanup
 *
 *     Revision 1.1  2004/05/14 17:53:55  dietz
 *     added rdpickcoda.c, moved from pkfilter's source dir
 *
 *     Revision 1.3  2004/04/30 22:52:34  dietz
 *     fixed bugs in parsing new _SCNL msg types
 *
 *     Revision 1.2  2004/04/29 22:01:07  dietz
 *     added capability to process TYPE_PICK_SCNL and TYPE_CODA_SCNL msgs
 *
 *     Revision 1.1  2004/04/22 18:01:56  dietz
 *     Moved pkfilter source from Contrib/Menlo to the earthworm orthodoxy
 *
 *
 *
 */

/*
 * rd_pickcoda.c
 *
 * Functions to convert TYPE_PICK2K and TYPE_CODA2K messages 
 * from ascii strings to a structures.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <chron3.h>
#include <earthworm_simple_funcs.h>
#include <earthworm_defs.h>
#include "rdpickcoda.h"

#define MAXSTR 512
#define MAX_SCNL_LEN (TRACE2_STA_LEN+TRACE2_CHAN_LEN+TRACE2_NET_LEN+TRACE2_LOC_LEN)

static int           Init_rdpickcoda  = 0;
static unsigned char RPC_TypePick2K   = 0;
static unsigned char RPC_TypeCoda2K   = 0;
static unsigned char RPC_TypePickSCNL = 0;
static unsigned char RPC_TypeCodaSCNL = 0;

void trimblank( char *str );

/*******************************************************************
 * Init_MsgTypes()  Initialize msg types recognized by these funcs *
 *                  Returns EW_SUCCESS or EW_FAILURE.              *
 *******************************************************************/
int Init_MsgTypes( void )
{
   if( Init_rdpickcoda ) return( EW_SUCCESS );

   if ( GetType( "TYPE_PICK2K", &RPC_TypePick2K ) != 0 ) {
      logit( "e",
             "Init_rdpickcoda: Invalid message type <TYPE_PICK2K>!\n" );
      return( EW_FAILURE );
   }
   if ( GetType( "TYPE_CODA2K", &RPC_TypeCoda2K ) != 0 ) {
      logit( "e",
             "Init_rdpickcoda: Invalid message type <TYPE_CODA2K>!\n" );
      return( EW_FAILURE );
   }
   if ( GetType( "TYPE_PICK_SCNL", &RPC_TypePickSCNL ) != 0 ) {
      logit( "e",
             "Init_rdpickcoda: Invalid message type <TYPE_PICK_SCNL>!\n" );
      return( EW_FAILURE );
   }
   if ( GetType( "TYPE_CODA_SCNL", &RPC_TypeCodaSCNL ) != 0 ) {
      logit( "e",
             "Init_rdpickcoda: Invalid message type <TYPE_CODA_SCNL>!\n" );
      return( EW_FAILURE );
   }
   Init_rdpickcoda = 1;

   return( EW_SUCCESS );
}

/*******************************************************************
 * rd_pick2k()  Read a TYPE_PICK2K message into an EWPICK struct.  *
 *              Returns EW_SUCCESS or EW_FAILURE.                  *
 *******************************************************************/

int  rd_pick2k( char *msg, int msglen, EWPICK *pk )
{
   char  str[MAXSTR];
   char  stime[20];
   int   instid,modid,msgtype;
   int   i;

/*-------------------------------------------------------------------------
Here's a sample TYPE_PICK2K message (72 chars long):
 10  4  3 2133 CMN  NCVHZ U1  19950831183134.90     953    1113     968\n
0123456789 123456789 123456789 123456789 123456789 123456789 123456789 12
---------------------------------------------------------------------------*/

/* Initialize variables
 **********************/
   if( Init_MsgTypes() != EW_SUCCESS ) return EW_FAILURE;
   memset( str, 0, MAXSTR );
   memset( pk,  0, sizeof(EWPICK) );

/* Make sure it's a TYPE_PICK2K message!
 ***************************************/
   strncpy( str, msg, 3 );
   msgtype = atoi( str );
   if( msgtype != (int)RPC_TypePick2K ) 
   {
      logit("","rd_pick2k error: given msgtype:%d but "
               "can only decode TYPE_PICK2K:%d\n",
                msgtype, (int)RPC_TypePick2K );
      return EW_FAILURE;
   }

/* Copy the incoming string to a local buffer so alter it.
 *********************************************************/
   if( msglen > MAXSTR ) 
   {
      logit("","rd_pick2k error: msg[%d] too long for local buffer[%d]\n",
             msglen, MAXSTR );
      return EW_FAILURE;
   }
   strncpy( str, msg, msglen );
 /*logit("e","pick msg:    %s", str );*/  /*DEBUG*/

/* Read PICK2K string (column oriented).
   Start at the end and work backwards.
 ***************************************/
   str[71] = '\0';   pk->pamp[2] = atol(str+63);
   str[63] = '\0';   pk->pamp[1] = atol(str+55);
   str[55] = '\0';   pk->pamp[0] = atol(str+47);
   str[47] = '\0';   strcpy(stime, str+30);
   str[30] = '\0';   pk->wt      = str[27];
                     pk->fm      = str[26];
   str[25] = '\0';   strcpy(pk->comp, str+22);
   str[22] = '\0';   strcpy(pk->net,  str+20);
   str[20] = '\0';   strcpy(pk->site, str+15);
   str[14] = '\0';   pk->seq     = (short) atoi(str+10);
   str[9]  = '\0';   instid      = atoi(str+6);
   str[6]  = '\0';   modid       = atoi(str+3);
   str[3]  = '\0';   msgtype     = atoi(str+0);
 
/* Trim blanks off of site,comp,net codes 
 ****************************************/
   trimblank(pk->site);
   trimblank(pk->net);
   trimblank(pk->comp);
   strcpy(pk->loc, LOC_NULL_STRING);

/* Check for valid values of instid, modid, type
 ***********************************************/
   if( instid<0  || instid>255 ||
       modid<0   || modid>255  ||
       msgtype<0 || msgtype>255   )
   {
      logit("","rd_pick2k: Invalid msgtype, modid, or instid\n" );
      return EW_FAILURE;
   }
   pk->msgtype = (unsigned char) msgtype;
   pk->modid   = (unsigned char) modid;
   pk->instid  = (unsigned char) instid;
 
/* Calculate pick time (tpick) from character string
 ***************************************************/
   for(i=0; i<18; i++) if(stime[i] == ' ') stime[i] = '0';
   epochsec17( &(pk->tpick), stime );
 
   if ( pk->tpick == 0.0 )
   {
      logit("","rd_pick2k: Error decoding time: %s\n", stime );
      return EW_FAILURE;
   }
  
 /*logit("e","pick struct: %u %u %u %hd %s.%s.%s.%s %c%c %.2lf %ld %ld %ld\n",
              pk->msgtype, pk->modid, pk->instid, pk->seq, 
              pk->site, pk->comp, pk->net, pk->loc, pk->fm, pk->wt,
              pk->tpick, pk->pamp[0], pk->pamp[1], pk->pamp[2] );*/  /*DEBUG*/

   return EW_SUCCESS;
}


/*******************************************************************
 * rd_coda2k()  Read a TYPE_CODA2K message into an EWCODA struct.  *
 *              Returns EW_SUCCESS or EW_FAILURE.                  *
 *******************************************************************/

int  rd_coda2k( char *msg, int msglen, EWCODA *cd )
{
   char str[MAXSTR];
   int  instid,modid,msgtype;
   int  i,n;

/*----------------------------------------------------------------------------
Here's a sample TYPE_CODA2K message (79 characters long):
 11  6  3 7893 NMW  NCVHZ      48     106     211     182     148     133  15 \n
0123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789
------------------------------------------------------------------------------*/

/* Initialize variables
 **********************/
   if( Init_MsgTypes() != EW_SUCCESS ) return EW_FAILURE;
   memset( str, 0, MAXSTR );
   memset( cd,  0, sizeof(EWCODA) );

/* Make sure it's a TYPE_CODA2K message!
 ***************************************/
   strncpy( str, msg, 3 );
   msgtype = atoi( str );
   if( msgtype != (int)RPC_TypeCoda2K ) 
   {
      logit("","rd_coda2k error: given msgtype:%d but "
               "can only decode TYPE_CODA2K:%d\n",
                msgtype, (int)RPC_TypeCoda2K );
      return EW_FAILURE;
   }
   
/* Copy incoming string to a local buffer so we can alter it.
 ************************************************************/
   if( msglen > MAXSTR ) 
   {
      logit("","rd_coda2k error: msg[%d] too long for local buffer[%d]\n",
             msglen, MAXSTR );
      return EW_FAILURE;
   }
   strncpy( str, msg, msglen );
 /*logit("e","coda msg:    %s", str );*/   /*DEBUG*/
 
/* Read CODA2K string (column oriented).
   Start at the end and work backwards.
 ***************************************/
   str[77] = '\0';   cd->dur = atoi(str+73);
   for (i=5, n=73 ; i>=0 ; i--, n-=8)
   {
      str[n] = '\0'; cd->caav[i] = atol(str+n-8);
   }
   str[25] = '\0';   strcpy(cd->comp, str+22);
   str[22] = '\0';   strcpy(cd->net,  str+20);
   str[20] = '\0';   strcpy(cd->site, str+15);
   str[14] = '\0';   cd->seq = (short) atoi(str+10);
   str[9]  = '\0';   instid  = atoi(str+6);
   str[6]  = '\0';   modid   = atoi(str+3);
   str[3]  = '\0';   msgtype = atoi(str+0);
 
/* Trim blanks off of site,comp,net codes 
 ****************************************/
   trimblank(cd->site);
   trimblank(cd->net);
   trimblank(cd->comp);
   strcpy(cd->loc, LOC_NULL_STRING);

/* Check for valid values of instid, modid, type
 ***********************************************/
   if( instid<0  || instid>255  ||
       modid<0   || modid>255   ||
       msgtype<0 || msgtype>255   )  
   {
      logit("","rd_coda2k: Invalid msgtype, modid, or instid\n" );
      return EW_FAILURE;
   }
   cd->msgtype = (unsigned char) msgtype;
   cd->modid   = (unsigned char) modid;
   cd->instid  = (unsigned char) instid;

 /*logit("e","coda struct: %u %u %u %hd %s %s %s %ld %ld %ld %ld %ld %ld %d\n",
              cd->msgtype, cd->modid, cd->instid, cd->seq, 
              cd->site, cd->net, cd->comp, cd->caav[0], cd->caav[1],
              cd->caav[2], cd->caav[3], cd->caav[4], cd->caav[5],
              cd->dur );*/   /*DEBUG*/

   return EW_SUCCESS;
}

void trimblank( char *str ) 
{
   char *c = strchr( str, ' ' );
   if( c != (char *)NULL ) *c= '\0';
   return;
}

/*******************************************************************
 * rd_pick_scnl() Read a TYPE_PICK_SCNL msg into an EWPICK struct. *
 *                Returns EW_SUCCESS or EW_FAILURE.                *
 *******************************************************************/

int  rd_pick_scnl( char *msg, int msglen, EWPICK *pk )
{
   char  stime[20];
   char  scnl[MAX_SCNL_LEN];
   char *ptr;
   int   instid,modid,msgtype;
   int   goodparse, toklen;
   int   rc;

/*-------------------------------------------------------------------------
Here's a sample TYPE_PICK_SCNL message (whitespace delimited):
8 6 3 7226 PCC.SHZ.NC.01 U2 20040430210553.360 161 200 137\n
---------------------------------------------------------------------------*/

/* Initialize variables
 **********************/
   if( Init_MsgTypes() != EW_SUCCESS ) return EW_FAILURE;
   memset( pk,  0, sizeof(EWPICK) );
   memset( stime, 0, 20 );

/* Make sure it's a TYPE_PICK_SCNL message!
 ******************************************/
   if( sscanf( msg, "%d", &msgtype ) != 1 ) 
   {
      logit("","rd_pick_scnl: error reading msgtype\n" );
      return EW_FAILURE;
   }
   if( msgtype != (int)RPC_TypePickSCNL ) 
   {
      logit("","rd_pick_scnl error: given msgtype:%d but "
               "can only decode TYPE_PICK_SCNL:%d\n",
                msgtype, (int)RPC_TypePickSCNL );
      return EW_FAILURE;
   }

/* Scan the incoming string (field oriented)
 *******************************************/
   rc = sscanf( msg, 
               "%d %d %d %d %s %c%c %18s %ld %ld %ld\n",
                &msgtype,
                &modid,
                &instid,
                &pk->seq,
                 scnl,
                &pk->fm,
                &pk->wt,
                 stime,
                &pk->pamp[0],
                &pk->pamp[1],
                &pk->pamp[2] );

 /*logit("e","pick_scnl sscanf: %d %d %d %d %s %c%c %s %ld %ld %ld\n",
              msgtype, modid, instid, pk->seq, 
              scnl, pk->fm, pk->wt, stime,
              pk->pamp[0], pk->pamp[1], pk->pamp[2] );*/  /*DEBUG*/
   if( rc != 11 )
   {
      logit("","rd_pick_scnl error: read only %d of 11 fields in msg\n",rc);
      return EW_FAILURE;
   }

/* Parse SCNL string
 *******************/
   goodparse = 0;
   ptr       = scnl;      
   toklen    = strcspn( ptr, "." );
   if( toklen>0 && toklen<TRACE2_STA_LEN ) {
      strncpy( pk->site, ptr, toklen );
      ptr += toklen+1;
      toklen = strcspn( ptr, "." );
      if( toklen>0 && toklen<TRACE2_CHAN_LEN ) {
         strncpy( pk->comp, ptr, toklen );
         ptr += toklen+1;
         toklen = strcspn( ptr, "." );
         if( toklen>0 && toklen<TRACE2_NET_LEN ) {
            strncpy( pk->net, ptr, toklen );
            ptr += toklen+1;
            toklen = strlen( ptr );
            if( toklen>0 && toklen<TRACE2_LOC_LEN ) {
               strcpy( pk->loc, ptr );
               goodparse = 1;
            }
         }
      }
   }
   if( !goodparse ) {
      logit("","rd_pick_scnl error parsing SCNL string: %s\n",scnl);
      return EW_FAILURE;
   } 

/* Check for valid values of instid, modid, type
 ***********************************************/
   if( instid<0  || instid>255 ||
       modid<0   || modid>255  ||
       msgtype<0 || msgtype>255   )
   {
      logit("","rd_pick_scnl error: Invalid msgtype, modid, or instid\n" );
      return EW_FAILURE;
   }
   pk->msgtype = (unsigned char) msgtype;
   pk->modid   = (unsigned char) modid;
   pk->instid  = (unsigned char) instid;
    
/* Calculate pick time (tpick) from character string
 ***************************************************/
   epochsec18( &(pk->tpick), stime );
 
   if ( pk->tpick == 0.0 )
   {
      logit("","rd_pick_scnl: Error decoding time: %s\n", stime );
      return EW_FAILURE;
   }
  
 /*logit("e","pick_scnl EWPICK: %u %u %u %d %s.%s.%s.%s %c%c %.3lf %ld %ld %ld\n",
              pk->msgtype, pk->modid, pk->instid, pk->seq, 
              pk->site, pk->comp, pk->net, pk->loc, pk->fm, pk->wt,
              pk->tpick, pk->pamp[0], pk->pamp[1], pk->pamp[2] );*/  /*DEBUG*/

   return EW_SUCCESS;
}


/*******************************************************************
 * rd_coda_scnl() Read a TYPE_CODA_SCNL msg into an EWCODA struct. *
 *                Returns EW_SUCCESS or EW_FAILURE.                *
 *******************************************************************/

int  rd_coda_scnl( char *msg, int msglen, EWCODA *cd )
{
   char  scnl[MAX_SCNL_LEN];
   char *ptr;
   int   goodparse, toklen;
   int   instid, modid, msgtype, rc;

/*----------------------------------------------------------------------------
Here's a sample TYPE_CODA_SCNL message (whitespace delimited):
9 6 3 7226 PCC.SHZ.NC.01 39 54 85 0 0 0 5\n
------------------------------------------------------------------------------*/

/* Initialize variables
 **********************/
   if( Init_MsgTypes() != EW_SUCCESS ) return EW_FAILURE;
   memset( cd,  0, sizeof(EWCODA) );

/* Make sure it's a TYPE_CODA_SCNL message!
 ******************************************/
   if( sscanf( msg, "%d", &msgtype ) != 1 ) 
   {
      logit("","rd_coda_scnl: error reading msgtype\n" );
      return EW_FAILURE;
   }
   if( msgtype != (int)RPC_TypeCodaSCNL ) 
   {
      logit("","rd_coda_scnl error: given msgtype:%d but "
               "can only decode TYPE_CODA_SCNL:%d\n",
                msgtype, (int)RPC_TypeCodaSCNL );
      return EW_FAILURE;
   }

/* Scan the incoming string (field oriented)
 *******************************************/
   rc = sscanf( msg, 
               "%d %d %d %d %s %ld %ld %ld %ld %ld %ld %d\n",
                &msgtype,
                &modid,
                &instid,
                &cd->seq,
                 scnl,
                &cd->caav[0],
                &cd->caav[1],
                &cd->caav[2],
                &cd->caav[3],
                &cd->caav[4],
                &cd->caav[5],
                &cd->dur );

 /*logit("e","coda_scnl sscanf: %d %d %d %d %s %ld %ld %ld %ld %ld %ld %d\n",
              msgtype, modid, instid, cd->seq, scnl, 
              cd->caav[0], cd->caav[1], cd->caav[2], 
              cd->caav[3], cd->caav[4], cd->caav[5], cd->dur );*/   /*DEBUG*/
   if( rc != 12 )
   {
      logit("","rd_coda_scnl error: read only %d of 12 fields in msg\n",rc);
      return EW_FAILURE;
   }

/* Parse SCNL string
 *******************/
   goodparse = 0;
   ptr       = scnl;      
   toklen    = strcspn( ptr, "." );
   if( toklen>0 && toklen<TRACE2_STA_LEN ) {
      strncpy( cd->site, ptr, toklen );
      ptr += toklen+1;
      toklen = strcspn( ptr, "." );
      if( toklen>0 && toklen<TRACE2_CHAN_LEN ) {
         strncpy( cd->comp, ptr, toklen );
         ptr += toklen+1;
         toklen = strcspn( ptr, "." );
         if( toklen>0 && toklen<TRACE2_NET_LEN ) {
            strncpy( cd->net, ptr, toklen );
            ptr += toklen+1;
            toklen = strlen( ptr );
            if( toklen>0 && toklen<TRACE2_LOC_LEN ) {
               strcpy( cd->loc, ptr );
               goodparse = 1;
            }
         }
      }
   }
   if( !goodparse ) {
      logit("","rd_coda_scnl error parsing SCNL string: %s\n",scnl);
      return EW_FAILURE;
   } 

/* Check for valid values of instid, modid, type
 ***********************************************/
   if( instid<0  || instid>255  ||
       modid<0   || modid>255   ||
       msgtype<0 || msgtype>255   )  
   {
      logit("","rd_coda_scnl error: Invalid msgtype, modid, or instid\n" );
      return EW_FAILURE;
   }
   cd->msgtype = (char) msgtype;
   cd->modid   = (char) modid;
   cd->instid  = (char) instid;

 /*logit("e","coda_scnl EWCODA: %u %u %u %d %s.%s.%s.%s %ld %ld %ld %ld %ld %ld %d\n",
              cd->msgtype, cd->modid, cd->instid, cd->seq, 
              cd->site, cd->comp, cd->net, cd->loc, cd->caav[0], cd->caav[1],
              cd->caav[2], cd->caav[3], cd->caav[4], cd->caav[5],
              cd->dur );*/   /*DEBUG*/

   return EW_SUCCESS;
}
