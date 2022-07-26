/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: rw_strongmotionII.c 8025 2019-06-19 13:13:24Z kevin $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.5  2004/02/13 22:55:22  dietz
 *     Now both "?" and "-" are interpreted as NULL values for
 *     qid and qauthor on the QID: line when reading TYPE_STRONGMOTIONII msgs
 *
 *     Revision 1.4  2003/06/09 19:53:50  lombard
 *     Rewrote rd_strongmotionII to be more fault-tolerant and to allow reading of
 *     multiple channels from a single message.
 *
 *     Revision 1.3  2001/06/19 17:26:02  dietz
 *     Fixed wr_strongmotionII to print SM_NULL values properly (it had been
 *     taking the absolute value of them by mistake)
 *
 *     Revision 1.2  2001/04/18 17:56:27  dietz
 *     wr_strongmotionII: made sure pga,pgv,pgd are written as absolute values
 *
 *     Revision 1.1  2001/04/17 17:25:46  davidk
 *     Initial revision
 *
 *
 *
 */

/* rw_strongmotionII.c
 *
 * Contains functions in that convert from a
 * TYPE_STRONGMOTION message to a structure and visa versa.
 *
 * written by Lynn Dietz   October, 1999
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <chron3.h>
#include <time_ew.h>
#include <rw_strongmotionII.h>
#include <earthworm.h>

#define ABS(X) (((X) >= 0) ? (X) : -(X))

static int   strappend( char *s1, int s1max, char *s2 );
static int   tokenlength( char *begtok, char c );
static char *datestr24( double t, char *pbuf, int len );
static int   addtimestr( char *buf, int buflen, double t );

static char *sNullDate = "0000/00/00 00:00:00.000";
static char *sUnknown  = "?";
#define SMLINE 512

/* Where we are in the message */
#define HAVE(ln) (ln&smState)
#define SM_SCNL 1<<0
#define SM_TIME 1<<1
#define SM_ALT  1<<2
#define SM_PGA  1<<3
#define SM_PGV  1<<4
#define SM_PGD  1<<5
#define SM_RSA  1<<6
#define SM_QID  1<<7
#define SM_AI   1<<8
#define SM_ALL (SM_SCNL | SM_TIME | SM_ALT | SM_PGA | SM_PGV | SM_PGD | SM_RSA | SM_QID)
#define SM_REQ (SM_SCNL | SM_TIME)

/*************************************************************
 * sm_init()                                                 *
 * Initialize a SM_INFO structure in an appropriate fashion. *
 *************************************************************/
static void sm_init( SM_INFO *sm)
{
    memset( sm, 0, sizeof(SM_INFO) );     /* zero output structure */
    sm->altcode = SM_ALTCODE_NONE;
    sm->pga = SM_NULL;
    sm->pgv = SM_NULL;
    sm->pgd = SM_NULL;
    
    return;
}



/********************************************************************
 * rd_strongmotionII()                                              *
 * Reads an one ascii TYPE_STRONGMOTION2 message and fills in a     *
 * SM_INFO structure.                                               *
 * Since there may be more than one TYPE_STRONGMOTION2 message in   *
 * a buffer, pass in a pointer to the message buffer. On return,    *
 * this pointer will get moved to the start of the next message,    *
 * or to the position of the terminating NULL byte if there is no   *
 * following message.                                               *
 * Returns: 1 on success                                            *
 *          0 on no message found                                   *
 *         -1 on failure                                            *
 ********************************************************************/
int rd_strongmotionII( char **msgP, SM_INFO *sm, int logErr)
{
   char     line[SMLINE]; /* pointer to current line in msg */
   char     *nextline;  /* working pointer into msg */
   int       nfreq;
   int       i;
   struct tm stm;       /* time structure for timestamp */
   time_t    tsec;      /* timestamp in whole sec since 1970 */    
   int       msec;      /* thousandths of seconds field of timestamp */
   int       smState;   /* keep track of our message state */
   int       status; 

   nextline = *msgP;
   nfreq = 0;
   smState = 0;
   status = 0;
   /* Loop over lines in the message */
   while( nextline && *nextline )
   {
       char *lineP, *mP;
       char *token;
       int lc;

       /* Read a line from the message into working buffer, without overflow */
       lineP = line;
       for (mP = nextline, lc = 0; 
	    *mP && *mP != '\n' && lc < SMLINE-1; 
	    mP++, lineP++, lc++) {
	   *lineP = *mP;
       }
       if (line[lc-1] == '\r') lc--;
       line[lc++] = '\n'; /* terminate with newline, for strtok */
       line[lc] = '\0';
       /* Did we get to the end of the line in our message? */
       if (lc == SMLINE -1 ) {
	   if (logErr)
	       logit("t", "SM message long line truncated: <%s>\n", line);
	   for ( ; *mP && *mP != '\n'; mP++) {
	       /* skip to end of line */
	   }
       }
       /* Don't advance nextline pointer until         *
	* we're sure we will process this current line */

       /* We have a copy of the line, chop it into tokens */
       if (strlen(line)==0) goto NextLine;
       token = strtok(line, " \t");
       if (token == NULL) {
	   if (logErr)
	       logit("t", "no token found in SM line <%s>\n", line);
	   status = -1;
	   goto NextLine;
       }
       
       /* We are looking for the SCNL line that starts a message */
       if (strcmp(token, "SCNL:") == 0) {
	   if (! HAVE(SM_SCNL) ) {
	       sm_init(sm);  /* initialize for new SM message */
	       smState = 0;  /* reset state for new SM message */
	       
	       token = strtok(NULL, ".");
	       if (token == NULL) {
		   if (logErr) 
		       logit("t", "error parsing SCNL from SM line <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       if( strlen(token) >= TRACE_STA_LEN ) {
		   if (logErr)
		       logit("t", "station name <%s> too long; max %d\n",
			     token, TRACE_STA_LEN);
		   status = -1;
		   goto NextLine;
	       }
	       strcpy( sm->sta, token); 
	       token = strtok(NULL, ".");
	       if (token == NULL) {
		   if (logErr) 
		       logit("t", "error parsing SCNL from SM line <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       if( strlen(token) >= TRACE_CHAN_LEN ) {
		   if (logErr)
		       logit("t", "channel name <%s> too long; max %d\n", 
			     token, TRACE_CHAN_LEN);
		   status = -1;
		   goto NextLine;
	       }
	       strcpy( sm->comp, token ); 
	       token = strtok(NULL, ".");
	       if (token == NULL) {
		   if (logErr) 
		       logit("t", "error parsing SCNL from SM line <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       if( strlen(token) >= TRACE_NET_LEN ) {
		   if (logErr)
		       logit("t", "network name <%s> too long; max %d\n",
			     token, TRACE_NET_LEN);
		   status = -1;
		   goto NextLine;
	       }
	       strcpy( sm->net, token );
	       /* location code might be empty, so we treat this differently */
	       token += strlen(token) + 1;
	       if ( *token == '\n' || *token == ' ' || *token == '\t' ) {
		   sm->loc[0] = '\0';
	       } else {
		   token = strtok(NULL, " \t\n");
		   if (token == NULL) {
		       if (logErr) 
			   logit("t", "error parsing SM SCNL line <%s>\n", line);
		       status = -1;
		       goto NextLine;
		   }
		   if( strlen(token) >= TRACE_LOC_LEN ) {
		       if (logErr)
			   logit("t", "location name <%s> too long; max %d\n",
				 token, TRACE_LOC_LEN);
		       status = -1;
		       goto NextLine;
		   }
		   strcpy( sm->loc, token );
		   if (sm->loc[0] == '-' || sm->loc[0] == '?')
		       sm->loc[0] = '\0';
	       }
	       
	       smState |= SM_SCNL;

	       /* There might be more on this line, but we ignore it today */

	   } else {
	       /* We found another SCNL line, so we past the end of our message */
	       if ( ! HAVE(SM_REQ) ) {
		   if (logErr) 
		       logit("t", "incomplete SM message for <%s.%s.%s.%s>\n",
			     sm->sta, sm->comp, sm->net, sm->loc);
		   status = -1;
	       }
	       goto LastLine;
	   }
	   /* end of SCNL line processing */
       }
       else if (strcmp(token, "TIME:") == 0) {
	   if (HAVE(SM_SCNL) ) {
	       memset( &stm, 0, sizeof(struct tm) );
	       token = strtok(NULL, " ");
	       if (token == NULL) {
		   if (logErr) 
		       logit("t", "error parsing SM TIME line <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       if( sscanf( token, "%d/%d/%d", 
			   &stm.tm_year, &stm.tm_mon, &stm.tm_mday) != 3) {
		   if (logErr) 
		       logit("t", "error parsing date from <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       token = strtok(NULL, " \t\n");
	       if (token == NULL) {
		   if (logErr) 
		       logit("t", "error parsing SM TIME line <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       if ( sscanf( token, "%d:%d:%d.%d", &stm.tm_hour,
                  &stm.tm_min, &stm.tm_sec, &msec ) != 4 ) {
		   if (logErr) 
		       logit("t", "error parsing date from <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       stm.tm_year -= 1900;  /* convert to definition of struct tm */
	       stm.tm_mon  -= 1;     /* convert to definition of struct tm */
	       tsec  = timegm( &stm );
	       sm->t = (double)tsec + 0.001*(double)msec;
	       smState |= SM_TIME;
	   } else {
	       /* still looking for SCNL line to start message */
	       goto NextLine;
	   }
	   /* End of TIME line processing */
       }
       else if ( strcmp(token, "ALT:") == 0) {
	   if (HAVE(SM_SCNL) ) {
	       memset( &stm, 0, sizeof(struct tm) );
	       token = strtok(NULL, " \t");
	       if (token == NULL) {
		   if (logErr) 
		       logit("t", "error parsing SM ALT line <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       if( sscanf( token, "%d/%d/%d", 
			   &stm.tm_year, &stm.tm_mon, &stm.tm_mday) != 3) {
		   if (logErr) 
		       logit("t", "error parsing date from <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       token = strtok(NULL, " \t");
	       if (token == NULL) {
		   if (logErr) 
		       logit("t", "error parsing SM ALT line <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       if ( sscanf( token, "%d:%d:%d.%d", &stm.tm_hour,
                  &stm.tm_min, &stm.tm_sec, &msec ) != 4 ) {
		   if (logErr) 
		       logit("t", "error parsing date from <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       token = strtok(NULL, " \t");
	       if (token == NULL) {
		   if (logErr) 
		       logit("t", "error parsing SM ALT line <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       if (strcmp(token, "CODE:") != 0) {
		   if (logErr) 
		       logit("t", "error parsing SM ALT line <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       token = strtok(NULL, " \t\n");
	       if (token == NULL) {
		   if (logErr) 
		       logit("t", "error parsing SM ALT line <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       if ( sscanf( token, "%d", &sm->altcode) != 1 ) {
		   if (logErr) 
		       logit("t", "error parsing ALT code from <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       stm.tm_year -= 1900;  /* convert to definition of struct tm */
	       stm.tm_mon  -= 1;     /* convert to definition of struct tm */
	       tsec  = timegm( &stm );
	       sm->talt = (double)tsec + 0.001*(double)msec;
	       smState |= SM_ALT;
	   } else {
	       /* still looking for SCNL line to start message */
	       goto NextLine;
	   }
	   /* End of ALT line processing */
       }
       else if (strcmp(token, "PGA:") == 0) {
	   if (HAVE(SM_SCNL) ) {
	       token = strtok(NULL, " \t");
	       if (token == NULL) {
		   if (logErr) 
		       logit("t", "error parsing SM PGA line <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       if( sscanf( token, "%lf", &sm->pga) != 1) {
		   if (logErr) 
		       logit("t", "error parsing PGA from <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       memset( &stm, 0, sizeof(struct tm) );
	       token = strtok(NULL, " \t");
	       if (token == NULL) {
		   if (logErr) 
		       logit("t", "error parsing SM PGA line <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       if (strcmp(token, "TPGA:") != 0) {
		   if (logErr) 
		       logit("t", "error parsing SM PGA line <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       token = strtok(NULL, " \t");
	       if (token == NULL) {
		   if (logErr) 
		       logit("t", "error parsing SM PGA line <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       if( sscanf( token, "%d/%d/%d", 
			   &stm.tm_year, &stm.tm_mon, &stm.tm_mday) != 3) {
		   if (logErr) 
		       logit("t", "error parsing date from <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       token = strtok(NULL, " \t\n");
	       if (token == NULL) {
		   if (logErr) 
		       logit("t", "error parsing SM PGA line <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       if ( sscanf( token, "%d:%d:%d.%d", &stm.tm_hour,
                  &stm.tm_min, &stm.tm_sec, &msec ) != 4 ) {
		   if (logErr) 
		       logit("t", "error parsing date from <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       if( stm.tm_year == 0 ) {
		   sm->tpga = 0.0;
	       } else {
		   stm.tm_year -= 1900;
		   stm.tm_mon  -= 1;
		   tsec  = timegm( &stm );
		   sm->tpga = (double)tsec + 0.001*(double)msec;
	       }
	       smState |= SM_PGA;
	   } else {
	       /* still looking for SCNL line to start message */
	       goto NextLine;
	   }
	   /* End of PGA line processing */
       }
       else if (strcmp(token, "PGV:") == 0) {
	   if (HAVE(SM_SCNL) ) {
	       token = strtok(NULL, " \t");
	       if (token == NULL) {
		   if (logErr) 
		       logit("t", "error parsing SM PGV line <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       if( sscanf( token, "%lf", &sm->pgv) != 1) {
		   if (logErr) 
		       logit("t", "error parsing PGV from <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       memset( &stm, 0, sizeof(struct tm) );
	       token = strtok(NULL, " \t");
	       if (token == NULL) {
		   if (logErr) 
		       logit("t", "error parsing SM PGV line <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       if (strcmp(token, "TPGV:") != 0) {
		   if (logErr) 
		       logit("t", "error parsing SM PGV line <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       token = strtok(NULL, " \t");
	       if (token == NULL) {
		   if (logErr) 
		       logit("t", "error parsing SM PGV line <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       if( sscanf( token, "%d/%d/%d", 
			   &stm.tm_year, &stm.tm_mon, &stm.tm_mday) != 3) {
		   if (logErr) 
		       logit("t", "error parsing date from <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       token = strtok(NULL, " \t\n");
	       if (token == NULL) {
		   if (logErr) 
		       logit("t", "error parsing SM PGV line <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       if ( sscanf( token, "%d:%d:%d.%d", &stm.tm_hour,
                  &stm.tm_min, &stm.tm_sec, &msec ) != 4 ) {
		   if (logErr) 
		       logit("t", "error parsing date from <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       if( stm.tm_year == 0 ) {
		   sm->tpgv = 0.0;
	       } else {
		   stm.tm_year -= 1900;
		   stm.tm_mon  -= 1;
		   tsec  = timegm( &stm );
		   sm->tpgv = (double)tsec + 0.001*(double)msec;
	       }
	       smState |= SM_PGV;
	   } else {
	       /* still looking for SCNL line to start message */
	       goto NextLine;
	   }
	   /* End of PGV line processing */
       }
       else if (strcmp(token, "PGD:") == 0) {
	   if (HAVE(SM_SCNL) ) {
	       token = strtok(NULL, " \t");
	       if (token == NULL) {
		   if (logErr) 
		       logit("t", "error parsing SM PGD line <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       if( sscanf( token, "%lf", &sm->pgd) != 1) {
		   if (logErr) 
		       logit("t", "error parsing PGD from <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       memset( &stm, 0, sizeof(struct tm) );
	       token = strtok(NULL, " \t");
	       if (token == NULL) {
		   if (logErr) 
		       logit("t", "error parsing SM PGD line <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       if (strcmp(token, "TPGD:") != 0) {
		   if (logErr) 
		       logit("t", "error parsing SM PGD line <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       token = strtok(NULL, " \t");
	       if (token == NULL) {
		   if (logErr) 
		       logit("t", "error parsing SM PGD line <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       if( sscanf( token, "%d/%d/%d", 
			   &stm.tm_year, &stm.tm_mon, &stm.tm_mday) != 3) {
		   if (logErr) 
		       logit("t", "error parsing date from <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       token = strtok(NULL, " \t\n");
	       if (token == NULL) {
		   if (logErr) 
		       logit("t", "error parsing SM PGD line <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       if ( sscanf( token, "%d:%d:%d.%d", &stm.tm_hour,
                  &stm.tm_min, &stm.tm_sec, &msec ) != 4 ) {
		   if (logErr) 
		       logit("t", "error parsing date from <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       if( stm.tm_year == 0 ) {
		   sm->tpgd = 0.0;
	       } else {
		   stm.tm_year -= 1900;
		   stm.tm_mon  -= 1;
		   tsec  = timegm( &stm );
		   sm->tpgd = (double)tsec + 0.001*(double)msec;
	       }
	       smState |= SM_PGD;
	   } else {
	       /* still looking for SCNL line to start message */
	       goto NextLine;
	   }
	   /* End of PGD line processing */
       }
       else if (strcmp(token, "RSA:") == 0) {
	   if (HAVE(SM_SCNL) ) {
	       token = strtok(NULL, "/");
	       if (token == NULL) {
		   if (logErr) 
		       logit("t", "error parsing SM RSA line <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       if( sscanf( token, "%d", &sm->nrsa) != 1) {
		   if (logErr) 
		       logit("t", "error parsing RSA from <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       for (i = 0; i < sm->nrsa; i++) {
		   token = strtok(NULL, "/");
		   if (token == NULL) {
		       if (logErr) 
			   logit("t", "error parsing SM RSA line <%s>\n", line);
		       status = -1;
		       goto NextLine;
		   }
		   if( sscanf( token, "%lf %lf", 
			       &(sm->pdrsa[i]), &(sm->rsa[i]) ) != 2 ) {
		       if (logErr) 
			   logit("t", "error parsing RSA from <%s>\n", line);
		       status = -1;
		       goto NextLine;
		   }
	       }
	       smState |= SM_RSA;
	   } else {
	       /* still looking for SCNL line to start message */
	       goto NextLine;
	   }
	   /* End of RSA line processing */
       }
       else if (strcmp(token, "QID:") == 0) {
	   if (HAVE(SM_SCNL) ) {
	       token = strtok(NULL, " ");
	       if (token == NULL) {
		   if (logErr) 
		       logit("t", "error parsing SM QID line <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       if (strlen(token) >= EVENTID_SIZE) {
		   if (logErr) 
		       logit("t", "event id too long in QID line <%s>, max %d\n",
			     line, EVENTID_SIZE-1);
		   status = -1;
		   goto NextLine;
	       }
	       if( strcmp(token, sUnknown)==0 ||
                   strcmp(token, "-"     )==0   )  /* CGS's NULL string */
		   sm->qid[0] = '\0';
	       else
		   strcpy(sm->qid, token);

	       token = strtok(NULL, " \n");
	       if (token == NULL) {
		   if (logErr) 
		       logit("t", "error parsing SM QID line <%s>\n", line);
		   status = -1;
		   goto NextLine;
	       }
	       if (strlen(token) >= EVENTID_SIZE) {
		   if (logErr) 
		       logit("t", "author id too long in QID line <%s>, max %d\n",
			     line, AUTHOR_FIELD_SIZE-1);
		   status = -1;
		   goto NextLine;
	       }
	       if (strcmp(token, sUnknown)==0 ||
                   strcmp(token, "-"     )==0   )  /* CGS's NULL string */
		   sm->qauthor[0] = '\0';
	       else
		   strcpy(sm->qauthor, token);
	       smState |= SM_QID;
	   } else {
	       /* still looking for SCNL line to start message */
	       goto NextLine;
	   }
	   /* End of QID line processing */
       } else if (strcmp(token, "AI:") == 0) {
           if (HAVE(SM_SCNL) ) {
               token = strtok(NULL, " \n");
               if (token == NULL) {
                   if (logErr)
                       logit("t", "error parsing SM AI line <%s>\n", line);
                   status = -1;
                   goto NextLine;
               }
               if( sscanf( token, "%lf", &sm->ai) != 1) {
                   if (logErr)
                       logit("t", "error parsing AI from <%s>\n", line);
                   status = -1;
                   goto NextLine;
               }
               smState |= SM_AI;
           } else {
               /* still looking for SCNL line to start message */
               goto NextLine;
           }
           /* End of AI line processing */
       } else {
	   /* unknown line; skip it for now */
	   goto NextLine;
       }

   NextLine:
       nextline = mP+1;
   } /* end of loop over message */
       
 LastLine:
   *msgP = nextline;
   if (HAVE(SM_REQ) )
       status = 1;
   
   return( status );
}

/********************************************************************
 * wr_strongmotionII()                                              *
 * Reads a SM_INFO structure and writes an ascii TYPE_STRONGMOTION2 *
 * message (null terminated)                                        *
 * Returns 0 on success, -1 on failure (buffer overflow)            *
 ********************************************************************/
int wr_strongmotionII( SM_INFO *sm, char *buf, int buflen )
{
   char     tmp[256]; /* working buffer */
   char    *qid;
   char    *qauthor;
   int      i;

   memset( buf, 0, (size_t)buflen );    /* zero output buffer */

/* channel codes */
   sprintf( buf, "SCNL: %s.%s.%s.%s", 
            sm->sta, sm->comp, sm->net, sm->loc );

/* field time */
   if( strappend( buf, buflen, "\nTIME: " ) ) return ( -1 );
   datestr24( sm->t, tmp, 256 );  
   if( strappend( buf, buflen, tmp ) ) return( -1 );

/* alternate time & its code */
   if( strappend( buf, buflen, "\nALT: " ) ) return ( -1 );
   if( addtimestr( buf, buflen, sm->talt ) ) return ( -1 );
   sprintf( tmp, " CODE: %d", sm->altcode );
   if( strappend( buf, buflen, tmp ) ) return( -1 );

/* Print peak acceleration value & time */
   sprintf( tmp, "\nPGA: %.6lf TPGA: ", (sm->pga!=SM_NULL ? ABS(sm->pga) : sm->pga) );
   if( strappend( buf, buflen, tmp ) ) return( -1 );
   if( addtimestr( buf, buflen, sm->tpga ) ) return ( -1 );
      
/* Print peak velocity value & time */
   sprintf( tmp, "\nPGV: %.6lf TPGV: ", (sm->pgv!=SM_NULL ? ABS(sm->pgv) : sm->pgv) );
   if( strappend( buf, buflen, tmp ) ) return( -1 );
   if( addtimestr( buf, buflen, sm->tpgv ) ) return ( -1 );
      
/* Print peak displacement value & time */
   sprintf( tmp, "\nPGD: %.6lf TPGD: ", (sm->pgd!=SM_NULL ? ABS(sm->pgd) : sm->pgd) );
   if( strappend( buf, buflen, tmp ) ) return( -1 );
   if( addtimestr( buf, buflen, sm->tpgd ) ) return ( -1 );

/* Print the response spectrum */
   sprintf( tmp, "\nRSA: %d", sm->nrsa );
   if( strappend( buf, buflen, tmp ) ) return( -1 );
   for( i=0; i<sm->nrsa; i++ )
   {
     sprintf( tmp, "/%.2lf %.6lf", sm->pdrsa[i], sm->rsa[i] );
     if( strappend( buf, buflen, tmp ) ) return( -1 );
   }   

/* Print the eventid & event author */
   qid     = sm->qid;
   qauthor = sm->qauthor;
   if( strlen(qid)     == 0 ) qid     = sUnknown;
   if( strlen(qauthor) == 0 ) qauthor = sUnknown;
   sprintf( tmp, "\nQID: %s %s\n", qid, qauthor );
   if( strappend( buf, buflen, tmp ) ) return( -1 );

/* Print the AI (Arias Intensity) */
   if( sm->ai != 0. )
   {
     sprintf( tmp, "AI: %.6lf\n", sm->ai );
     if( strappend( buf, buflen, tmp ) ) return( -1 );
   }

   return( 0 );
}



/********************************************************************
 * log_strongmotionII()                                             *
 * Writes the contents of a SM_INFO structure to an Earthworm       *
 * log file                                                         *
 ********************************************************************/
void log_strongmotionII( SM_INFO *sm )
{
   char date[30];
   int  i;

   logit("", "SCNL: %s.%s.%s.%s\n", 
           sm->sta, sm->comp, sm->net, sm->loc );
  
/* Log time values */
   datestr24( sm->t, date, 30 );
   logit("", "Time: %s  (%.3lf)\n", date, sm->t );
   datestr24( sm->talt, date, 30 );
   logit("", "Alt:  %s  (%.3lf)  code: %d\n", 
            date, sm->talt, sm->altcode );


/* Print peak acceleration values */
   datestr24( sm->tpga, date, 30 );
   logit("", "PGA: %.6lf  TGPA: %s (%.3lf)\n",
            sm->pga, date, sm->tpga );
      
/* Print peak velocity values */
   datestr24( sm->tpgv, date, 30 );
   logit("", "PGV: %.6lf  TGPV: %s (%.3lf)\n",
            sm->pgv, date, sm->tpgv );

/* Print peak displacement values */
   datestr24( sm->tpgd, date, 30 );
   logit("", "PGD: %.6lf  TGPD: %s (%.3lf)\n",
            sm->pgd, date, sm->tpgd );


/* Print number of points in the response spectrum */
   logit("","RSA: %d ", sm->nrsa );
   for( i=0; i<sm->nrsa; i++ )
   {
      logit("", " /%.2lf %.6lf", sm->pdrsa[i], sm->rsa[i] );
   }   

/* Print eventid & author */
   logit("","\nQID: %s %s\n", sm->qid, sm->qauthor);

/* Print the AI (Arias Intensity) */
   if( sm->ai != 0. )
   {
     logit("","AI: %.6lf\n", sm->ai );
   }

   return;
}


/**********************************************************
 * Converts time (double, seconds since 1970:01:01) to    *
 * a 23-character, null-terminated string in the form of  *
 *            yyyy/mm/dd hh:mm:ss.sss                     *
 * Time is displayed in UTC                               *
 * Target buffer must be 24-chars long to have room for   *
 * null-character                                         *
 **********************************************************/
char *datestr24( double t, char *pbuf, int len )
{
   time_t    tt;       /* time as time_t                  */
   struct tm stm;      /* time as struct tm               */
   int       t_msec;   /* milli-seconds part of time      */

/* Make sure target is big enough
 ********************************/
   if( len < 24 ) return( (char *)NULL );

/* Convert double time to other formats
 **************************************/
   t += 0.0005;  /* prepare to round to the nearest 1000th */
   tt     = (time_t) t;
   t_msec = (int)( (t - tt) * 1000. );
   gmtime_ew( &tt, &stm );

/* Build character string
 ************************/
   sprintf( pbuf,
           "%04d/%02d/%02d %02d:%02d:%02d.%03d",
            stm.tm_year+1900,
            stm.tm_mon+1,
            stm.tm_mday,
            stm.tm_hour,
            stm.tm_min,
            stm.tm_sec,
            t_msec );

   return( pbuf );
}


/********************************************************************
 * addtimestr() append a date string to the end of existing string  *
 *   Return -1 if result would overflow the target,                 *
 *           0 if everything went OK                                *
 ********************************************************************/
int addtimestr( char *buf, int buflen, double t )
{
   char tmp[30];

   if( t == 0.0 )
   {
     if( strappend( buf, buflen, sNullDate ) ) return( -1 );
   } else {
     datestr24( t, tmp, 30 );  
     if( strappend( buf, buflen, tmp ) ) return( -1 );
   }
   return( 0 );
}

/********************************************************************
 * strappend() append second null-terminated character string to    *
 * the first as long as there's enough room in the target buffer    * 
 * for both strings and the null-byte                               *
 ********************************************************************/
int strappend( char *s1, int s1max, char *s2 )
{
   if( (int)strlen(s1)+(int)strlen(s2)+1 > s1max ) return( -1 );
   strcat( s1, s2 );
   return( 0 );
}

/********************************************************************
 * tokenlength() given a null-terminated character string and a     *
 * character that delimits the end of a token, tokenlength returns  * 
 * the length (in bytes) of the next token. If the character wasn't * 
 * found, tokenlength returns the length of the string.             *
 ********************************************************************/
int tokenlength( char *begtok, char c )
{
   char    *endtok;   /* points to the end of this token */

   endtok = strchr( begtok, c );
   if( endtok == NULL ) return( (int)strlen(begtok) );
   return( (int)(endtok-begtok) );
}


/* Sample TYPE_STRONGMOTION2 message (between ------):
------------------------------------------------
SCNL: CMB.BHZ.BK.
TIME: 2001/02/25 02:37:00.000
ALT:  2001/02/25 02:40:40.000 CODE: 1
PGA: 6.846210 TPGA: 2001/02/25 02:37:00.000
PGV: 0.140000 TPGV: 2001/02/25 02:37:00.000
PGD: 0.000000 TPGD: 2001/02/25 02:37:00.000
RSA: 3/0.30 4.415404/1.00 0.925639/3.00 0.297907
QID: 41059467 014024003:UCB
------------------------------------------------
*/
