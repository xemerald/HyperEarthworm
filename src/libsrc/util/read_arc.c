/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: read_arc.c 7594 2018-10-31 14:43:55Z quintiliani $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.11  2015/06/18 16:09:00  saurel
 *     Added functions to write arc message from Hpck and Hsum structures
 *     Added functions to read arc message without shadow lines to Hpck and Hsum structures
 *
 *     Revision 1.10  2009/08/11 14:21:23  quintiliani
 *     Set numerical value for the origin version from the ARC Summary
 *
 *     Revision 1.9  2004/05/28 22:35:52  dietz
 *     changed read_phs to read loc field from phase line
 *
 *     Revision 1.8  2002/12/06 22:30:45  dietz
 *     Added 3 fields (nphS,nphtot,nPfm) to read_hyp()
 *
 *     Revision 1.7  2002/10/29 18:47:42  lucky
 *     Added origin version number
 *
 *     Revision 1.6  2000/12/06 17:50:32  lucky
 *     Make sure that the onset is read and stored
 *
 *     Revision 1.5  2000/09/12 19:07:34  lucky
 *     Defined separate FM fields for S and P picks
 *
 *     Revision 1.4  2000/08/21 19:49:39  lucky
 *     Fixed read_phs so that the Hpck structure is filled with the information
 *     for both the P and S wave picks.
 *
 *     Revision 1.3  2000/04/04 22:51:23  davidk
 *     changed the starting column position for the read of sumP->e1dp
 *     (the intermediate principal error) from 61 to 64.  Due to the error,
 *     e1dp was being set to an excessively large value that caused an
 *     error to occur in orareport as it inserted the bad value into the DB.
 *
 *     Revision 1.2  2000/03/30 15:43:39  davidk
 *     added code to handle additional arc message fields in both
 *     the summary and phase lines, that are used by orareport,
 *     so that orareport can use the read_arc routines.
 *
 *     Revision 1.1  2000/02/14 18:51:48  lucky
 *     Initial revision
 *
 *
 */

/* Routines for reading HYP2000ARC summary lines and phase lines.
 * These routines are provided for the use of arc2trig and two UW-spcific 
 * modules, uw_report and eqtee. They only read the fields that are
 * required by these modules.
 * If you need to read more info from the HYP2000ARC message, feel free to
 * make additions here. You probably will need to add to the structures in
 * read_arc.h as well.
 * Completely rewritten by Pete Lombard, 12/03/1998
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>  /* for atof() */
#include <string.h>

#include "chron3.h"
#include "read_arc.h"
#include "earthworm_simple_funcs.h"

/********************************
 * Internal function prototypes *
 ********************************/
static int strib( char* );
static void read_fl2( char*, float* );
static void read_fl1( char*, float* );
static void read_i(char*, int* );

/* this used to live in hyp2000_mgr, but it could be used elsewhere - quality for H71Sum card */
char ComputeAverageQuality(float rms, float erh, float erz, float depth, float dmin, int no, int gap)
{
   int qs, qd, qa;
/* Compute qs, qd, and average quality
   ***********************************/
   if     (rms <0.15 && erh<=1.0 && erz <= 2.0) qs=4;  /* qs is A */
   else if(rms <0.30 && erh<=2.5 && erz <= 5.0) qs=3;  /* qs is B */
   else if(rms <0.50 && erh<=5.0)               qs=2;  /* qs is C */
   else                                         qs=1;  /* qs is D */

   if     (no >= 6 && gap <=  90 && (dmin<=depth    || dmin<= 5)) qd=4; /* qd is A */
   else if(no >= 6 && gap <= 135 && (dmin<=2.*depth || dmin<=10)) qd=3; /* qd is B */
   else if(no >= 6 && gap <= 180 &&  dmin<=50)                    qd=2; /* qd is C */
   else                                                           qd=1; /* qd is D */

   qa = (qs+qd)/2; /* use integer truncation to round down */
   if(qa>=4) return('A');
   if(qa==3) return('B');
   if(qa==2) return('C');
   if(qa<=1) return('D');

   return(' '); /* should never reach here */
}
/**************************************************************
 * read_hyp() reads the hypocenter line from an archive msg into an 
 *            Hsum structure (defined in read_arc.h)
 *
 *      Inputs: sumLine - character string holding the summary line
 *              shdw - character string holding the summary shadow line
 *              sumP - pointer to Hsum structure provided by the caller
 *      returns 0 on success, -1 on parsing errors
 *
 ****************************************************************/

int read_hyp( char *sumLine, char *shdw, struct Hsum *sumP )
{
  char   str[200];
  float  deg, min;
  float  sign;
  int    len;

  str[0]=0;

  /* Initialize the output structure
  **********************************/
  memset( sumP, 0, (size_t) sizeof(struct Hsum) ); 
 
  /* Copy incoming string to a local buffer so we can alter it.
  ************************************************************/
  strncpy ( str, sumLine, (size_t) 165 ); /* this should be enough chars for us */
  len = strlen( sumLine );       /* note length of original summary line */

/*------------------------------------------------------------------------
Sample HYP2000ARC archive summary line and its shadow.  The summary line
may be up to 188 characters long.  Its full format is described in
documentation (hyp2000.shadow.doc) by Fred Klein.
199204290117039536 2577120 2407  475  0 18 98 17  16 5975 128175 6  58343COA  38    0  57 124 21   0 218  0  8COA WW D 24X   0  0L  0  0     10123D343 218Z  0   0  \n
YYYYMMDDHHMMSSssLLsLLLLlllsllllDDDDDMMMpppGGGnnnRRRRAAADDEEEEAAADDMMMMCCCrrreeeerrssshhhhvvvvpppssssddddsssdddmmmapdacpppsaeeewwwmaaawwwIIIIIIIIIIlmmmwwwwammmwwwwVvN
$1                                                                                0343   0   0\n
0123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 12345
0         1         2         3         4         5         6         7         8         9        10        11        12        13        14        15        16
------------------------------------------------------------------------*/
  
  /* Read origin time
  ******************/
  strncpy( sumP->cdate,    str,     (size_t) 14 ); /* YYYYMMDDHHMMSS */
  sumP->cdate[14] = '.';
  strncpy( sumP->cdate+15, str+14,  (size_t) 2 );  /* hundreths of seconds */
  sumP->cdate[17] = '\0';
  sumP->ot = julsec17( sumP->cdate );
  if ( sumP->ot == 0.0 ) 
  {
    logit( "t", "read_hyp: Error decoding origin time: %s\n",
	   sumP->cdate );
    return( -1 );
  }

  /* Read the "version" of the information,
   * i.e. what stage of processing or completeness the event is in. */
  if(str[162] >= '0'  &&  str[162] <= '9') {
      sumP->version = str[162] - '0';
  } else {
      /* Set version to 1 as default */
      sumP->version = 1;
  }
  
  
  /* Read the rest of the card, back to front
  ******************************************/
  if( len >= 154 )
  {
  /* We're not reading fields beyond str[154] (Fortran column 155) */
    str[154] = '\0';
    read_fl1( str+150, &sumP->wtpref );
    str[150] = '\0';
    read_fl2( str+147, &sumP->Mpref );
    sumP->labelpref = str[146];
  }
  str[146] = '\0';  sumP->qid  = atol( str+136 );
  
  str[121] = '\0';  
  read_i( str+118, &sumP->nphtot );
 
  /* Added by davidk 20000317 for use by DB PutArc */
  sumP->mdtype=str[117]; /* coda dur mag type */

  str[107+3]  = '\0';  
  read_fl2( str+107, &sumP->mdmad ); /* Mag XX */

  str[100+4]  = '\0';  
  read_fl1( str+100, &sumP->mdwt ); /* Mag Wt. */
  /* End: Added by davidk 20000317  */

  str[96] = '\0';  
  read_i( str+93, &sumP->nPfm );
  str[93]  = '\0';  
  read_fl2( str+89, &sumP->erz );
  str[89]  = '\0';
  read_fl2( str+85, &sumP->erh );
  str[85]  = '\0';
  read_i( str+82, &sumP->nphS );
  str[80]  = '\0';  
  read_fl2( str+76, &sumP->e2 );
  str[76]  = '\0';
  strncpy( sumP->reg, str+73, (size_t) 4 );
  str[73]  = '\0';
  read_fl2( str+70, &sumP->Md );
  str[70]  = '\0';
  read_fl2( str+66, &sumP->e1 );
  str[66]  = '\0';  
  read_i( str+64, &sumP->e1dp );
  str[64]  = '\0';
  read_i( str+61, &sumP->e1az );
  str[61]  = '\0';
  read_fl2( str+57, &sumP->e0 );
  str[57]  = '\0';
  read_i( str+55, &sumP->e0dp );
  str[55]  = '\0';
  read_i( str+52, &sumP->e0az );
  str[52]  = '\0';
  read_fl2( str+48, &sumP->rms );
  str[48]  = '\0';
  read_i( str+45, &sumP->dmin );
  str[45]  = '\0';
  read_i( str+42, &sumP->gap );
  str[42]  = '\0';
  read_i( str+39, &sumP->nph );
  str[36]  = '\0';
  read_fl2( str+31, &sumP->z );
  
  /* longitude */
  str[31]  = '\0';  min        = (float)(atof( str+27 )/100.0);
  if ( str[26] == ' ' || str[26] == 'W' || str[26] == 'w' ) 
    sign = -1.0;
  else
    sign = 1.0;
  str[26]  = '\0';  deg        = (float)atof( str+23 );
  sumP->lon = (float)(sign * ( deg + min / 60.0 ));

  /* latitude */
  str[23]  = '\0';  min        = (float)(atof( str+19 )/100.0);
  if ( str[18] == 'S' || str[18] == 's' )
    sign = -1.0;
  else
    sign = 1.0;
  str[18]  = '\0';  deg        = (float)atof( str+16 );
  sumP->lat = (float)(sign * ( deg + min / 60.0 ));

  return ( 0 );
}


/***************************************************************
 * read_phs() reads a phase line & its shadow from archive msg 
 *            into an Hpck structure (defined in read_arc.h)
 *      Inputs: phs - character string holding one phase line
 *              shdw - character string holding the phase shadow line
 *              pckP - pointer to Hpck structure provided by the caller
 *     
 *      returns 0 on success, -1 on parsing errors
 *
 ***************************************************************/

int read_phs( char *phs, char *shdw, struct Hpck *pckP )
{
	char 	str[120];
	float  	Psec, Ssec;
	int 	i;
	double	Cat; /* common arrival time up to yyyymmddhhmm */

   
/*------------------------------------------------------------------------
Sample TYPE_HYP2000ARC station archive card (P-arrival) and its shadow. 
  Phase card is 116 chars (including newline); 
  Shadow is up to 96 chars (including newline).
PWM  NC VVHZ  PD0199204290117  877  -8136    0   0   0      0 0  0  61   0 169 8400  0   77 88325  0 932   0WD 01  \n
SSSSSNNxcCCCxrrFWYYYYMMDDHHMMPPPPPRRRRWWWSSSSSrrxWRRRRAAAAAAAuuWWWDDDDddddEEEEAAAwwppprDDDDAAAMMMmmmppppssssSDALLaa\
$   6 5.49 1.80 7.91 3.30 0.10 PSN0   77 PHP3 1853 39 340 47 245 55 230 63  86 71  70 77  48   \n
$xnnnAA.AAQQ.QQaa.aaqq.qqRR.RRxccccDDDDDxPHpwAAAAATTTAAAAtttaaaaTTTAAAAtttaaaaTTTAAAAtttaaaaMMM\
0123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456
--------------------------------------------------------------------------*/


  /* Copy phase string to a local buffer so we can alter it.
  ************************************************************/
  memset(str,0,(size_t) 120);
  strncpy ( str, phs, (size_t) 120 );

  /* Data source code */
  pckP->datasrc = str[108];

  /* Duration magnitude */
  str[94+3] = '\0';
  read_fl2( str+94, &pckP->Md);

  /* azimuth to station */
  str[91+3] = '\0';
  read_i( str+91, &pckP->azm);

  /* Coda duration */
  str[87+4] = '\0';
  read_i( str+87, &pckP->codalen );

  /* duration magnitude weight code */
  str[82+1] = '\0';
  read_i( str+82, &pckP->codawt );

  /* emergence angle at source */
  str[78+3] = '\0';
  read_i( str+78, &pckP->takeoff);

  /* epicentral distance */
  str[74+4] = '\0';
  read_fl1( str+74, &pckP->dist);

  /* S weight actually used */
  str[63+3]  = '\0';
  read_fl2( str+63, &pckP->Swt);

  /* S travel time residual */
  str[50+4]  = '\0';
  read_fl2( str+50, &pckP->Sres );

  /* Assigned S weight code */
  str[49+1]  = '\0';
  read_i( str+49, &pckP->Squal );

  pckP->Sfm = str[48];

  /* S phase label */
  pckP->Slabel = str[47];

  /* S phase onset */
  pckP->Sonset = str[46];

  /* Second of S arrival */
  str[41+5]  = '\0';  
  Ssec = (float)(atof( str+41 )/100.0 );

  /* P weight actually used */
  str[38+3]  = '\0';
  read_fl2( str+38, &pckP->Pwt);

  /* P travel time residual */
  str[34+4]  = '\0';
  read_fl2( str+34, &pckP->Pres );

  /* Seconds of P arrival */
  str[29+5]  = '\0';  
  Psec = (float)(atof( str+29 )/100.0 );

  /* Figure out the common time */
  strncpy( pckP->cdate, str+17, (size_t) 12 ); /* YYYYMMDDHHMM */
  pckP->cdate[12] = '\0';
  strcat(pckP->cdate, "00.00" );
  Cat = julsec17( pckP->cdate );
  if ( Cat == 0.0 ) 
  {
    logit( "t", "read_phs: Error decoding arrival time: %s\n",
                    pckP->cdate );
    return( -1 );
  }

  /* Calculate P and S arrival times */
  pckP->Pat = Cat + Psec;
  pckP->Sat = Cat + Ssec;

  /* Assigned P weight code */
  str[16+1]  = '\0';
  read_i( str+16, &pckP->Pqual );

  pckP->Pfm = str[15];

  /* P phase label */
  pckP->Plabel = str[14];

  /* P phase onset */
  pckP->Ponset = str[13];

  /* Location code */
  str[111+2]  = '\0';  
  strcpy(pckP->loc, str+111); 
  strib(pckP->loc);

  /* Component code */
  str[9+3]  = '\0';  
  strcpy(pckP->comp, str+9); 
  strib(pckP->comp);

  /* network code */
  str[5+2]   = '\0';  
  strcpy(pckP->net,  str +5); 
  strib(pckP->net);

  /* station site code */
  str[5]   = '\0';  
  strcpy(pckP->site, str); 
  strib(pckP->site);


  /* Copy shadow string to a local buffer so we can alter it.
  ************************************************************/
  memset(str,0,(size_t) 120);
  strncpy ( str, shdw, (size_t) 120 );

  str[88+4] = '\0';
  read_i( str+88,  &(pckP->caav[5]) );

  str[85+3] = '\0';
  read_i( str+85, &(pckP->ccntr[5]) );

  str[81+4] = '\0';
  read_i( str+81,  &(pckP->caav[4]) );

  str[78+3] = '\0';
  read_i( str+78, &(pckP->ccntr[4]) );

  str[74+4] = '\0';
  read_i( str+74,  &(pckP->caav[3]) );

  str[71+3] = '\0';
  read_i( str+71, &(pckP->ccntr[3]) );

  str[67+4] = '\0';
  read_i( str+67,  &(pckP->caav[2]) );

  str[64+3] = '\0';
  read_i( str+64, &(pckP->ccntr[2]) );

  str[60+4] = '\0';
  read_i( str+60,  &(pckP->caav[1]) );

  str[57+3] = '\0';
  read_i( str+57, &(pckP->ccntr[1]) );

  str[53+4] = '\0';
  read_i( str+53,  &(pckP->caav[0]) );

  str[50+3] = '\0';
  read_i( str+50, &(pckP->ccntr[0]) );

  for( i=0; i<6; i++ ) 
  {
    if( pckP->ccntr[i]==0 ) 
    {
	    pckP->ccntr[i]    = (long)NULL;
      pckP->caav[i]     = (long)NULL;
    }
  }

  str[45+5] = '\0';
  read_i( str+45, &pckP->pamp );

  str[35+5] = '\0';
  read_i( str+35, &pckP->codalenObs);


  return ( 0 );
}


/***************************************************************
 * read_phs_no_shdw() reads a phase line without its shadow from archive msg 
 *            into an Hpck structure (defined in read_arc.h)
 *      Inputs: phs - character string holding one phase line

 *              pckP - pointer to Hpck structure provided by the caller
 *     
 *      returns 0 on success, -1 on parsing errors
 *
 ***************************************************************/

int read_phs_no_shdw( char *phs, struct Hpck *pckP )
{
	char 	str[120];
	float  	Psec, Ssec;
	double	Cat; /* common arrival time up to yyyymmddhhmm */

   
/*------------------------------------------------------------------------
Sample TYPE_HYP2000ARC station archive card (P-arrival) and its shadow. 
  Phase card is 116 chars (including newline); 
  Shadow is up to 96 chars (including newline).
PWM  NC VVHZ  PD0199204290117  877  -8136    0   0   0      0 0  0  61   0 169 8400  0   77 88325  0 932   0WD 01  \n
SSSSSNNxcCCCxrrFWYYYYMMDDHHMMPPPPPRRRRWWWSSSSSrrxWRRRRAAAAAAAuuWWWDDDDddddEEEEAAAwwppprDDDDAAAMMMmmmppppssssSDALLaa\
$   6 5.49 1.80 7.91 3.30 0.10 PSN0   77 PHP3 1853 39 340 47 245 55 230 63  86 71  70 77  48   \n
$xnnnAA.AAQQ.QQaa.aaqq.qqRR.RRxccccDDDDDxPHpwAAAAATTTAAAAtttaaaaTTTAAAAtttaaaaTTTAAAAtttaaaaMMM\
0123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456
--------------------------------------------------------------------------*/


  /* Copy phase string to a local buffer so we can alter it.
  ************************************************************/
  memset(str,0,(size_t) 120);
  strncpy ( str, phs, (size_t) 120 );

  if( strlen(str) < 40 )
  {
    logit( "t", "read_phs_no_shdw: invalid phase line, skipped\n");
    return( -1 );
  }

  /* Data source code */
  pckP->datasrc = str[108];

  /* Duration magnitude */
  str[94+3] = '\0';
  read_fl2( str+94, &pckP->Md);

  /* azimuth to station */
  str[91+3] = '\0';
  read_i( str+91, &pckP->azm);

  /* Coda duration */
  str[87+4] = '\0';
  read_i( str+87, &pckP->codalen );

  /* duration magnitude weight code */
  str[82+1] = '\0';
  read_i( str+82, &pckP->codawt );

  /* emergence angle at source */
  str[78+3] = '\0';
  read_i( str+78, &pckP->takeoff);

  /* epicentral distance */
  str[74+4] = '\0';
  read_fl1( str+74, &pckP->dist);

  /* S weight actually used */
  str[63+3]  = '\0';
  read_fl2( str+63, &pckP->Swt);

  /* S travel time residual */
  str[50+4]  = '\0';
  read_fl2( str+50, &pckP->Sres );

  /* Assigned S weight code */
  str[49+1]  = '\0';
  read_i( str+49, &pckP->Squal );

  pckP->Sfm = str[48];

  /* S phase label */
  pckP->Slabel = str[47];

  /* S phase onset */
  pckP->Sonset = str[46];

  /* Second of S arrival */
  str[41+5]  = '\0';  
  Ssec = (float)(atof( str+41 )/100.0 );

  /* P weight actually used */
  str[38+3]  = '\0';
  read_fl2( str+38, &pckP->Pwt);

  /* P travel time residual */
  str[34+4]  = '\0';
  read_fl2( str+34, &pckP->Pres );

  /* Seconds of P arrival */
  str[29+5]  = '\0';  
  Psec = (float)(atof( str+29 )/100.0 );

  /* Figure out the common time */
  strncpy( pckP->cdate, str+17, (size_t) 12 ); /* YYYYMMDDHHMM */
  pckP->cdate[12] = '\0';
  strcat(pckP->cdate, "00.00" );
  Cat = julsec17( pckP->cdate );
  if ( Cat == 0.0 ) 
  {
    logit( "t", "read_phs_no_shdw: Error decoding arrival time: %s\n",
                    pckP->cdate );
    return( -1 );
  }

  /* Calculate P and S arrival times */
  pckP->Pat = Cat + Psec;
  pckP->Sat = Cat + Ssec;

  /* Assigned P weight code */
  str[16+1]  = '\0';
  read_i( str+16, &pckP->Pqual );

  pckP->Pfm = str[15];

  /* P phase label */
  pckP->Plabel = str[14];

  /* P phase onset */
  pckP->Ponset = str[13];

  /* Location code */
  str[111+2]  = '\0';  
  strcpy(pckP->loc, str+111); 
  strib(pckP->loc);

  /* Component code */
  str[9+3]  = '\0';  
  strcpy(pckP->comp, str+9); 
  strib(pckP->comp);

  /* network code */
  str[5+2]   = '\0';  
  strcpy(pckP->net,  str +5); 
  strib(pckP->net);

  /* station site code */
  str[5]   = '\0';  
  strcpy(pckP->site, str); 
  strib(pckP->site);

  return ( 0 );
}

/*
 * Strip trailing blanks and newlines from string.
 */
static int strib( char *string )
{
  int i, length;
  
  length = strlen( string );
  if ( length )
  {
    for ( i = length-1; i >= 0; i-- )
    {
      if ( string[i] == ' ' || string[i] == '\n' )
	string[i] = '\0';
      else
	return ( i+1 );
    }
  } 
  else
    return length;
  return ( i+1 );
}

static void read_fl2( char* str, float* quant )
{
  if ( strib( str ) )
    *quant = (float)(atof( str)/100.0 );
  else
    *quant = NO_FLOAT_VAL;
}

static void read_fl1( char* str, float* quant )
{
  if ( strib( str ) )
    *quant = (float)(atof( str)/10.0 );
  else
    *quant = NO_FLOAT_VAL;
}

static void read_i( char* str, int* quant )
{
  if ( strib( str ) )
    *quant = atoi( str );
  else
    *quant = NO_INT_VAL;
}

/* Function to search in string src, the pointer of next characters \0 or \n
 * return the pointer or NULL if src is NULL */
char *parse_arc_next_shadow(char *src) {
    char *ret = NULL;
    if(src) {
        ret = src;
        while( (*ret != 0)  &&  (*ret != '\n') ) {
            ret++;
        }
        if(*ret == 0) {
            ret = NULL;
        } else {
            ret++;
        }
    }
    return ret;
}

int free_phases(HypoArc *arc)
{
  int i = 0;
  
  for(i=0; i< arc->num_phases; i++) 
  {
     free(arc->phases[i]);
  }
  return(0);
}

 /* parse_arc: parse the hyp2000 archive message into the Arc structure
 *   Returns: 0 on success
 *           -1 on parse errors
 */
int parse_arc(char *msg, HypoArc *arc)
{
  char *cur_msg = NULL;
  char *cur_sdw = NULL;
  int error = 0;
  int nline = 0;

  cur_msg = msg;
  cur_sdw = parse_arc_next_shadow(cur_msg);
  while( error != -1 && cur_msg != NULL && cur_sdw != NULL ) {

    if(cur_msg[0] != '$') {
      cur_sdw = parse_arc_next_shadow(cur_msg);
      nline++;

      if(cur_sdw == NULL) {
        logit("", "parse_arc: error reading arc shadow line.\n");
        error = -1;
      } else if(cur_sdw[0] != '$') {
        logit("", "parse_arc: error reading arc shadow line:\n%s\n", cur_sdw);
        error = -1;
      } else {

        if (nline == 1) {
          /* Summary ARC line */

          /* Process the hypocenter card (1st line of msg) */
          if ( read_hyp( cur_msg, cur_sdw, &(arc->sum) ) < 0 )
          {  /* Should we send an error message? */
            logit( "", "parse_arc: error parsing HYPO summary message.\n" );
            error = -1;
          } else {

            logit( "", "parse_arc: parsed HYPO summary message.\n" );
            arc->num_phases = 0;
          }
        } else {
          /* Station ARC line */
          if(arc->num_phases < MAX_PHASES) {
            arc->phases[arc->num_phases] = (struct Hpck *) calloc(1, sizeof(struct Hpck));
            if (read_phs (cur_msg, cur_sdw, arc->phases[arc->num_phases]) < 0) {
              logit("", "parse_arc: error reading phase line info.\n");
              error = -1;
              free(arc->phases[arc->num_phases]);
            } else {
              if(arc->phases[arc->num_phases]->site[0] != 0 ) {
                logit( "", "parse_arc: parsed HYPO phase line (%s.%s).\n", 
			arc->phases[arc->num_phases]->site, arc->phases[arc->num_phases]->comp );
                arc->num_phases++;
              } else {
                logit( "", "parse_arc: parsing HYPO phase lines is terminated.\n");
                cur_msg = NULL;
                cur_sdw = NULL;
                free(arc->phases[arc->num_phases]);
              }
            }
          } else {
            logit("", "parse_arc: WARNING number of arc phases execeds number of allocated %d\n", MAX_PHASES );
            cur_msg = NULL;
            cur_sdw = NULL;
          }
        }

      }
    } else {
      logit("", "parse_arc: error reading arc no shadow line.\n");
      error = -1;
    }

    if(error != -1) {
      cur_msg = parse_arc_next_shadow(cur_sdw);
      nline++;
    }
  }

  if(error != -1) {
      logit("", "parse_arc: read %d lines and %d phases of the arc message.\n", nline, arc->num_phases);
  } else {
      logit("", "parse_arc: error reading arc message (%d lines and %d phases).\n", nline, arc->num_phases);
  }

  return 0;
}

 /* parse_arc_no_shdw: parse an hyp2000 archive message without shadow lines into the Arc structure
 *   Returns: 0 on success
 *           -1 on parse errors
 */
int parse_arc_no_shdw(char *msg, HypoArc *arc)
{
  char *cur_msg = NULL;
  int error = 0;
  int nline = 1;

  cur_msg = msg;
  while( error != -1 && cur_msg != NULL && strlen(cur_msg) > 40 ) {

        if (nline == 1) {
          /* Summary ARC line */

          /* Process the hypocenter card (1st line of msg) */
          if ( read_hyp( cur_msg, NULL, &(arc->sum) ) < 0 )
          {  /* Should we send an error message? */
            logit( "", "parse_arc_no_shdw: error parsing HYPO summary message.\n" );
            error = -1;
          } else {

            logit( "", "parse_arc_no_shdw: parsed HYPO summary message.\n" );
            arc->num_phases = 0;
          }
        } else {
          /* Station ARC line */
          if(arc->num_phases < MAX_PHASES) {
            arc->phases[arc->num_phases] = (struct Hpck *) calloc(1, sizeof(struct Hpck));
            if (read_phs_no_shdw (cur_msg, arc->phases[arc->num_phases]) < 0) {
              logit("", "parse_arc_no_shdw: error reading phase line info.\n");
              error = -1;
              free(arc->phases[arc->num_phases]);
            } else {
              if(arc->phases[arc->num_phases]->site[0] != 0 ) {
                logit( "", "parse_arc_no_shdw: parsed HYPO phase line (%s.%s).\n", 
			arc->phases[arc->num_phases]->site, arc->phases[arc->num_phases]->comp );
                arc->num_phases++;
              } else {
                logit( "", "parse_arc_no_shdw: parsing HYPO phase lines is terminated.\n");
                cur_msg = NULL;
                free(arc->phases[arc->num_phases]);
              }
            }
          } else {
            logit("", "parse_arc_no_shdw: WARNING number of arc phases execeds number of allocated %d\n", MAX_PHASES );
            cur_msg = NULL;
          }
        }

    if(error != -1) {
      cur_msg = parse_arc_next_shadow(cur_msg);
      nline++;
    }
  }

  if(error != -1) {
      logit("", "parse_arc_no_shdw: read %d lines and %d phases of the arc message.\n", nline, arc->num_phases);
  } else {
      logit("", "parse_arc_no_shdw: error reading arc message (%d lines and %d phases).\n", nline, arc->num_phases);
  }

  return 0;
}

 /* write_arc: write a hyp2000 archive message from the Arc structure
 *   Returns: 0 on success
 *           -1 on parse errors
 */
int write_arc(char *msg, HypoArc *arc)
{
  char *out_msg = NULL;
  char line[251];
  int i;

  out_msg = msg;
  write_hyp( out_msg, arc->sum );
  for(i=0;i<arc->num_phases;i++) {
    write_phs( line, *arc->phases[i] );
    strcat( out_msg, line);
  }
  write_term( out_msg, arc->sum.qid );

  return 0;
}


/***************************************************************************/
/* write_hyp() builds a hyp2000 (hypoinverse) summary card & its shadow   */
/***************************************************************************/
void write_hyp( char *hypcard, struct Hsum sumP )
{
   char line[170];   /* temporary working place */
   char shdw[170];   /* temporary shadow card   */
   int  i;
   int  dlat,dlon;
   float mlon,mlat;

/* Put all blanks into line and shadow card
 ******************************************/
   for ( i=0; i<170; i++ )  line[i] = shdw[i] = ' ';

/*----------------------------------------------------------------------------------
Sample HYP2000 (HYPOINVERSE) hypocenter and shadow card.  Binder's eventid is stored 
in cols 136-145.  Many fields will be blank due to lack of information from binder.
(summary is 165 chars, including newline; shadow is 81 chars, including newline):
199806262007418537 3557118 4836  624 00 26 94  2   633776  5119810  33400MOR  15    0  32  50 99
   0 999  0 11MAM WW D189XL426 80         51057145L426  80Z464  102 \n
$1                                                                              \n
0123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 12345
6789 123456789 123456789 123456789 123456789 123456789 123456789 123456789
-----------------------------------------------------------------------------------*/

/* Write out hypoinverse summary card
 ************************************/
   strncpy( line,     sumP.cdate,     14 );
   strncpy( line+14,  sumP.cdate+15,  2  );
   dlat = (int) sumP.lat;
   mlat =  (sumP.lat - dlat)*60.0;
   if(sumP.lat>0)  sprintf( line+16,  "%2d ", abs(dlat) );
   else            sprintf( line+16,  "%2dS", abs(dlat) );
   sprintf( line+19,  "%4.0f", fabs( mlat*100.0 ) );
   dlon = (int) sumP.lon;
   mlon = (sumP.lon - dlon)*60.0;
   if(sumP.lon<0)  sprintf( line+23,  "%3d ", abs(dlon) );
   else            sprintf( line+23,  "%3dE", abs(dlon) );
   sprintf( line+27,  "%4.0f", fabs( mlon*100.0 ) );
   sprintf( line+31,  "%5.0f", sumP.z*100.0 ); 
   sprintf( line+39,  "%3d"  , sumP.nph );
   sprintf( line+42,  "%3d"  , sumP.gap );
   sprintf( line+45,  "%3d"  , sumP.dmin );
   sprintf( line+48,  "%4.0f", sumP.rms*100.0 );
   sprintf( line+52,  "%3d"  , sumP.e0az );
   sprintf( line+55,  "%2d"  , sumP.e0dp );
   sprintf( line+57,  "%4.0f", sumP.e0*100.0 );
   sprintf( line+61,  "%3d"  , sumP.e1az );
   sprintf( line+64,  "%2d"  , sumP.e1dp );
   sprintf( line+66,  "%4.0f", sumP.e1*100.0 );
   sprintf( line+70,  "%3.0f", sumP.Md*100.0 );
   sprintf( line+73,  "%3s"  , sumP.reg );
   sprintf( line+76,  "%4.0f", sumP.e2*100.0 );
   sprintf( line+82,  "%3d"  , sumP.nphS );
   sprintf( line+85,  "%4.0f", sumP.erh*100.0 );
   sprintf( line+89,  "%4.0f", sumP.erz*100.0 );
   sprintf( line+93,  "%3d"  , sumP.nPfm );
   sprintf( line+100, "%4.0f", sumP.mdwt*10.0 );
   sprintf( line+107, "%4.0f", sumP.mdmad*100.0 );
   sprintf( line+117, "%c"   , sumP.mdtype );
   sprintf( line+118, "%3d"  , sumP.nphtot );
   sprintf( line+136, "%10ld", sumP.qid );
   sprintf( line+146, "%c"   , sumP.labelpref );
   sprintf( line+147, "%3.0f", sumP.Mpref*100.0 );
   sprintf( line+150, "%4.0f", sumP.wtpref*10.0 );
   sprintf( line+162, "%1d"  , (int)sumP.version );
   for( i=0; i<164; i++ ) if( line[i]=='\0' ) line[i] = ' ';
   sprintf( line+164, "\n" );

/* Write out summary shadow card
 *******************************/
   sprintf( shdw,     "$1"   );
   for( i=0; i<80; i++ ) if( shdw[i]=='\0' ) shdw[i] = ' ';
   sprintf( shdw+80,  "\n" );

/* Copy both to the target address
 *********************************/
   strcpy( hypcard, line );
   strcat( hypcard, shdw );

   return;
}

/*******************************************************************************/
/* write_phs() builds a hyp2000 (hypoinverse) archive phase card & its shadow */
/*******************************************************************************/
void write_phs( char *phscard, struct Hpck pckP )
{
   char line[125];     /* temporary phase card    */
   char shdw[125];     /* temporary shadow card   */
   int   nblank;
   int   offset;
   float Psec, Ssec;
   int   i;
   double Cat; /* common arrival time up to yyyymmddhhmm */

/* Put all blanks into line and shadow card
 ******************************************/
   for ( i=0; i<125; i++ )  line[i] = shdw[i] = ' ';

/*-----------------------------------------------------------------------------
Sample Hyp2000 (hypoinverse) station archive cards (for a P-arrival and 
an S-arrival) and a sample shadow card. Many fields are blank due to lack 
of information from binder. Station phase card is 121 chars, including newline; 
shadow is 96 chars, including newline:
MTC  NC  VLZ  PD0199806262007 4507                                                0      79                 W  --       \n
MTC  NC  VLZ    4199806262007 9999        5523 S 1                                4      79                 W  --       \n
$   6 5.27 1.80 4.56 1.42 0.06 PSN0   79 PHP2 1197 39 198 47 150 55 137 63 100 71  89 79  48   \n
0123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 12
-------------------------------------------------------------------------------*/
/* Build station archive card
 ****************************/
   sprintf( line,     "%-5.5s", pckP.site );
   sprintf( line+5,   "%-2.2s", pckP.net );
   sprintf( line+9,   "%-3.3s", pckP.comp );
   line[13] = pckP.Ponset;  
   line[14] = pckP.Plabel;  
   line[15] = pckP.Pfm;  
   sprintf( line+16,  "%1d", pckP.Pqual );
   /* Figure out the common time */
   Cat = julsec17( pckP.cdate );
   if ( Cat == 0.0 ) 
   {
     logit( "t", "read_phs: Error decoding arrival time: %s\n",
                     pckP.cdate );
     return;
   }
   /* Calculate P and S arrival times */
   Psec = pckP.Pat - Cat;
   Ssec = pckP.Sat - Cat;
   strncpy( line+17, pckP.cdate,   12 );        /* yyyymmddhhmm    */
   sprintf( line+29,  "%5.0f", Psec*100.0 );
   sprintf( line+34,  "%4.0f", pckP.Pres*100.0 );
   sprintf( line+38,  "%3.0f", pckP.Pwt*100.0 );
   sprintf( line+41,  "%5.0f", Ssec*100.0 );
   line[46] = pckP.Sonset;  
   line[47] = pckP.Slabel;  
   line[48] = pckP.Sfm;  
   sprintf( line+49,  "%1d"  , pckP.Squal );
   sprintf( line+50,  "%4.0f", pckP.Sres*100.0 );
   sprintf( line+63,  "%3.0f", pckP.Swt*100.0 );
   sprintf( line+74,  "%4.0f", pckP.dist*10.0 );
   sprintf( line+78,  "%3d"  , pckP.takeoff );
   sprintf( line+82,  "%1d"  , pckP.codawt );
   sprintf( line+87,  "%4d"  , pckP.codalen );
   sprintf( line+91,  "%3d"  , pckP.azm );
   sprintf( line+94,  "%3.0f", pckP.Md*100.0 );
   sprintf( line+108, "%c"   , pckP.datasrc );
   sprintf( line+111, "%2s"  , pckP.loc );
   for( i=0; i<113; i++ ) if( line[i]=='\0' ) line[i] = ' ';
   sprintf( line+113, "\n" );


/* Build the shadow card
 ***********************/
   sprintf( shdw,  "$ " );
   sprintf( shdw+35, "%5d "  , pckP.codalenObs );
   sprintf( shdw+45, "%5d"   ,   pckP.pamp );
   offset = 50;
   nblank = 0;
   for ( i=5; i>=0; i-- ) {  /* add coda aav's in increasing time order */
      if( pckP.ccntr[i] == 0. ) {
          nblank++;
          continue;
      }
      sprintf( shdw+offset, "%3d%4d", 
              (int) pckP.ccntr[i], (int) pckP.caav[i] );
      offset += 7;
   }
   if( nblank ) {      /* put unused (blank) coda aav's at end of line */
      for( i=0; i<nblank; i++ ) {
         sprintf( shdw+offset, "%3d%4d", 
                 (int) pckP.ccntr[5-i], (int) pckP.caav[5-i] );
         offset += 7;
      }
   }
   for( i=0; i<95; i++ ) if( shdw[i]=='\0' ) shdw[i] = ' ';
   sprintf( shdw+95, "\n" );

/* Copy both to the target address
 *********************************/
   strcpy( phscard, line );
   strcat( phscard, shdw );

   return;
}

/***************************************************************************/
/* write_term() builds a hypoinverse event terminator card & its shadow   */
/***************************************************************************/
void write_term( char *termcard, long HypId )
{
        char line[100];   /* temporary working place */
        char shdw[100];   /* temporary shadow card   */
        int  i;

/* Put all blanks into line and shadow card
 ******************************************/
        for ( i=0; i<100; i++ )  line[i] = shdw[i] = ' ';

/* Build terminator
 ******************/
        sprintf( line+62, "%10ld\n",  HypId );

/* Build shadow card
 *******************/
        sprintf( shdw, "$" );
        shdw[1] = ' ';
        sprintf( shdw+62, "%10ld\n",  HypId );

/* Copy both to the target address
 *********************************/
        strcat( termcard, line );
        strcat( termcard, shdw );
        return;
}
