/* rw_mag.c
 *
 * Contains functions in that convert from a
 * TYPE_MAGNITUDE message to a structure and visa versa.
 *
 * written by Pete Lombard   February, 2001
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "earthworm.h"
#include "rw_mag.h"

/* Internal Function Prototypes */
static int strappend( char *s1, int s1max, char *s2 );
static int tokenlength( char *begtok, char c );
int wr_mag_internal( MAG_INFO *pMag, char *buf, int buflen, int scnl_flag);
static char *findItemValue(const char *dataStr, const char *nameStr);
static void copyItemValue(char *destStr, const char *srcStr, int maxLen);

/********************************************************************
 * rd_mag()                                                         *
 * Reads an ascii TYPE_MAGNITUDE message and fills in a MAG_INFO    *
 * structure which must be allocated by the caller                  *
 * If, in addition, the caller allocates space for auxiliary        *
 * structures at pMag->pMagAux (number of bytes in pMag->size_aux), *
 * then rd_mag will call the appropriate function to decode the     *
 * remainder of the message into appropriate structures.            *
 * Currently supported magnitude types are: Ml                      *
 * Returns 0 on success                                             *
 *        -1 on parsing errors                                      *
 *        -2 on insufficient space in pMagAux                       *
 ********************************************************************/
int rd_mag( char *msg, int msgLen, MAG_INFO *pMag)
{
  size_t size_aux;
  char *pMagAux;

  size_aux = pMag->size_aux;
  pMagAux = (char *)pMag->pMagAux;
  memset(pMag, 0, sizeof(MAG_INFO));

  switch ( sscanf( msg
                 , "%s MAG %d %s %lf %s %d %d %lf %lf %lf %d %s %d %d"
                 ,  pMag->qid
                 , &pMag->imagtype
                 ,  pMag->szmagtype
                 , &pMag->mag
                 ,  pMag->algorithm
                 , &pMag->nstations
                 , &pMag->nchannels
                 , &pMag->error
                 , &pMag->quality
                 , &pMag->mindist
                 , &pMag->azimuth
                 ,  pMag->qauthor
                 , &pMag->origin_version
                 , &pMag->qdds_version
         )       )
  {
    case 12:  /* old-style, no origin version, qdds version numbers */
         pMag->origin_version = 0;
         pMag->qdds_version = 0;
         break;
    case 13:  /* new-style, with origin version, no qdds version */
         pMag->qdds_version = 0;
         break;
    case 14:  /* new-style, with both number */
         break;
    default:
         return( -1 );
  }
  
  if (pMagAux && size_aux > 0)
  {
    pMag->size_aux = size_aux;
    pMag->pMagAux = pMagAux;
    
    /* Read the rest of a Mag message */
    if (pMag->pMagAux != NULL)
	{
         return( rd_chan_mag(msg, msgLen, (MAG_CHAN_INFO *)pMag->pMagAux,
                          pMag->size_aux) );
	}
  }
  
  return( 0 );
}

/*****************************************************************
 * rd_chan_mag()                                                 *
 * Reads an ascii TYPE_MAGNITUDE Ml message and fills in         *
 * MAG_CHAN_INFO structures.                                     *
 * This function can be called by rd_mag if sufficient space is  *
 * allocated by its caller. Or it may be called after rd_mag     *
 * when the caller has allocated space for `nchannels' of        *
 * MAG_CHAN_INFO structures.                                     *
 * Returns 0 on success                                          *
 *        -1 on parsing errors                                   *
 *        -2 on insufficient space in pMagAux                    *
 *****************************************************************/
int rd_chan_mag ( char *msg, int msglen, MAG_CHAN_INFO *pMci, int size_ml)
{
  char     *line;
  char     *begtok;
  char     *tmpMsg;     /* local copy of msg */
  int       len;       /* length (bytes) of the next token */
  int       nchan;     /* number of channels read from msg so far */
  int       rc;
  int       index;
  
  memset(pMci, 0, size_ml);

  /* tmpMsg = (char *) malloc (msglen); 
     strcpy (tmpMsg, msg);
*/
  tmpMsg = msg;

  nchan = 0;

  /* Skip the first line */
  len = tokenlength (tmpMsg, '\n'); 

  index = len + 1;

  /* Read next line from the message into working buffer */
  while(index < (msglen-1))
  {
	/* Process next line */
	line = tmpMsg + index;
    len = tokenlength(line, '\n' ); 
	line[len] = '\0';
    index = index + len + 1;

    begtok = line;
    while( begtok[0] == ' ' && begtok[0] != '\0') begtok++;    /* go to 1st non-blank char */
    if( begtok[0] == '\0' ) return( -1 );
    len = tokenlength( begtok, '.' );
    if( len >= TRACE_STA_LEN ) return( -1 );
    strncpy( pMci[nchan].sta, begtok, len );
    pMci[nchan].sta[len] = '\0';
    begtok += len + 1;

    len = tokenlength( begtok, '.' );
    if( len >= TRACE_CHAN_LEN ) return( -1 );
    strncpy( pMci[nchan].comp, begtok, len );
    pMci[nchan].comp[len] = '\0';
    begtok += len + 1;

    len = tokenlength( begtok, '.' );
    if (len == 2) {
        /* we have a location code! */
        if( len >= TRACE_NET_LEN ) return( -1 );
        strncpy( pMci[nchan].net, begtok, len );
        pMci[nchan].net[len] = '\0';
        begtok += len + 1;
        pMci[nchan].loc[0] = '\0';
        len = tokenlength( begtok, ' ' );
        if (len == 2) {
            strncpy( pMci[nchan].loc, begtok, len );
            pMci[nchan].loc[len] = '\0';
            begtok += len + 1;
        }
    } else {
        /* we are probably looking at a decimal point further down in the line 
           indicating that this is just a SCN (old style message) */
        len = tokenlength( begtok, ' ' );
        if( len >= TRACE_NET_LEN ) return( -1 );
        strncpy( pMci[nchan].net, begtok, len );
        pMci[nchan].net[len] = '\0';
        begtok += len + 1;
    }

    while( begtok[0] == ' ' && begtok[0] != '\0' ) begtok++;    /* go to next non-blank char */
    if ( (rc = sscanf(begtok, "%lf %lf %lf %E %lf %f %E %lf %f",
                      &(pMci[nchan].mag), &(pMci[nchan].dist), &(pMci[nchan].corr),
                      &(pMci[nchan].Amp1), &(pMci[nchan].Time1), &(pMci[nchan].Period1),
                      &(pMci[nchan].Amp2), &(pMci[nchan].Time2), 
                      &(pMci[nchan].Period2))) != 9)
    {
      if (rc == 6)
      {
        pMci[nchan].Amp2 = pMci[nchan].Period2 = -1.0;
        pMci[nchan].Time2 = -1.0;
        continue;
      }
      return( -1 );
    }

    nchan = nchan + 1;

  }

  /* free(tmpMsg); */
  return 0;
}



/*******************************************************************
 * wr_mag()                                                        *
 * Reads a MAG_INFO structure and writes an ascii TYPE_MAGNITUDE   *
 * message (null terminated)                                       *
 * Returns 0 on success, -1 on failure (potential buffer overflow) *
 *******************************************************************/
int wr_mag( MAG_INFO *pMag, char *buf, int buflen )
{
	return(wr_mag_internal(pMag, buf, buflen, 0));
}
int wr_mag_scnl( MAG_INFO *pMag, char *buf, int buflen )
{
	return(wr_mag_internal(pMag, buf, buflen, 1));
}
int wr_mag_internal( MAG_INFO *pMag, char *buf, int buflen, int scnl_flag)
{
  char     tmp[256]; /* working buffer */
  char     qid[256]; 
  char     szmagtype[256]; 
  char     algorithm[256]; 
  char     qauthor[256]; 

  memset( buf, 0, (size_t)buflen );    /* zero output buffer */

  /* 
   * We should check for null strings, if for nothing else than to 
   * avoid mis-formatted message trickling down the system. LV 6/2001
   */
	if (strlen (pMag->qid) == 0)
		strcpy (qid, "UNKNOWN");
	else
		strcpy (qid, pMag->qid);
 
	if (strlen (pMag->szmagtype) == 0)
		strcpy (szmagtype, "UNKNOWN");
	else
		strcpy (szmagtype, pMag->szmagtype);
 
	if (strlen (pMag->algorithm) == 0)
		strcpy (algorithm, "UNKNOWN");
	else
		strcpy (algorithm, pMag->algorithm);
 
	if (strlen (pMag->qauthor) == 0)
		strcpy (qauthor, "UNKNOWN");
	else
		strcpy (qauthor, pMag->qauthor);
 
  sprintf(tmp, "%s MAG %d %s %0.2f %s %d %d %0.2f %0.2f %0.2f %d %s %d %d\n",
          qid, pMag->imagtype, szmagtype, pMag->mag, algorithm, 
          pMag->nstations, pMag->nchannels, pMag->error, pMag->quality,
          pMag->mindist, pMag->azimuth, qauthor, pMag->origin_version
          , pMag->qdds_version
          );
   
  if ( strappend( buf, buflen, tmp ) ) return( -1 );
   
  if (pMag->pMagAux != NULL)
  {
      return( wr_chan_mag( (MAG_CHAN_INFO *)pMag->pMagAux, pMag->nchannels, 
                           buf, buflen, scnl_flag) );
  }
   
  return( 0 );
}

/*******************************************************************
 * wr_chan_mag()                                                   *
 * Reads MAG_CHAN_INFO structures and appends to an ascii          *
 * TYPE_MAGNITUDE message (null terminated)                        *
 * Normally this would be called by wr_mag().                      *
 * Returns 0 on success, -1 on failure (potential buffer overflow) *
 *******************************************************************/
int wr_chan_mag( MAG_CHAN_INFO *pMci, int nchannels, char *buf, int buflen , int scnl_flag)
{
  int i;
  char line[256];
  
  for (i = 0; i < nchannels; i++)
  {
    if (scnl_flag) {
        sprintf(line, 
            "%s.%s.%s.%s %.2f %.2f %.2f %9.2E %9.2f %9.2f %9.2E %9.2f %9.2f\n",
            /*s  c  n l mag  dist corr Amp1 Time1 Period1 Amp2 Time2 Period2 */
            pMci[i].sta, pMci[i].comp, pMci[i].net, pMci[i].loc,
			pMci[i].mag, pMci[i].dist, pMci[i].corr, 
            pMci[i].Amp1,
            ( (pMci[i].Time1 < 0.0) ? -1.0 : pMci[i].Time1),
            ( (pMci[i].Period1 < 0.0) ? -1.0 : pMci[i].Period1),
            pMci[i].Amp2,
            ( (pMci[i].Time2 < 0.0) ? -1.0 : pMci[i].Time2),
            ( (pMci[i].Period2 < 0.0) ? -1.0 : pMci[i].Period2));
    } else {
        sprintf(line, 
            "%s.%s.%s %.2f %.2f %.2f %9.2E %9.2f %9.2f %9.2E %9.2f %9.2f\n",
            /*s  c  n mag  dist corr Amp1 Time1 Period1 Amp2 Time2 Period2 */
            pMci[i].sta, pMci[i].comp, pMci[i].net, 
			pMci[i].mag, pMci[i].dist, pMci[i].corr, 
            pMci[i].Amp1,
            ( (pMci[i].Time1 < 0.0) ? -1.0 : pMci[i].Time1),
            ( (pMci[i].Period1 < 0.0) ? -1.0 : pMci[i].Period1),
            pMci[i].Amp2,
            ( (pMci[i].Time2 < 0.0) ? -1.0 : pMci[i].Time2),
            ( (pMci[i].Period2 < 0.0) ? -1.0 : pMci[i].Period2));
    }
    
    if ( strappend(buf, buflen, line))
        return( -1 );
  }
  return( 0 );
}


/***********************************************************************
 * rd_nomag_msg() reads values from a "No local magnitude available"   *
 * message.                                                            *
 *   msg - message string to parse                                     *
 *   eventIdStr - event-ID string buffer, or NULL for none             *
 *   magTypeStr - magnitude-type string buffer, or NULL for none       *
 *   pMagTypeIdx - pointer to magnitude-type index integer, or NULL    *
 *                 for none                                            *
 *   maxLen - maximum length for 'eventIdStr' and 'magTypeStr' buffers *
 * Returns 0 on success; -1 if parsing error                           *
 ***********************************************************************/
int rd_nomag_msg(const char *msg, char *eventIdStr, char *magTypeStr,
                                               int *pMagTypeIdx, int maxLen)
{
  char *eidptr, *mtsptr, *mtiptr;

  if((eidptr=findItemValue(msg, EVENT_ID_MSTR)) != NULL &&
                       (mtsptr=findItemValue(msg, MAG_TYPE_MSTR)) != NULL &&
                         (mtiptr=findItemValue(msg, MAG_TIDX_MSTR)) != NULL)
  {  /* all three "name=value" items found */
    if(eventIdStr != NULL)
      copyItemValue(eventIdStr, eidptr, maxLen);
    if(magTypeStr != NULL)
      copyItemValue(magTypeStr, mtsptr, maxLen);
    if(pMagTypeIdx != NULL)
      sscanf(mtiptr, "%d", pMagTypeIdx);
    return 0;
  }
  return -1;
}

/********************************************************************
 * wr_nomag_msg() writes a "No local magnitude available" message   *
 * to the given buffer.                                             *
 *   buf - destination buffer                                       *
 *   eventIdStr - event-ID string                                   *
 *   magTypeStr - magnitude-type string                             *
 *   magTypeIdx - magnitude-type index value                        *
 ********************************************************************/
void wr_nomag_msg(char *buf, const char *eventIdStr, const char *magTypeStr,
                                                             int magTypeIdx)
{
  sprintf(buf,"%s, %s=%s, %s=%s, %s=%d", NO_MAGAVAIL_MSTR, EVENT_ID_MSTR,
          eventIdStr, MAG_TYPE_MSTR, magTypeStr, MAG_TIDX_MSTR, magTypeIdx);
}

 
/********************************************************************
 * strappend() append second null-terminated character string to    *
 * the first as long as there's enough room in the target buffer    * 
 * for both strings and the null-byte                                *
 ********************************************************************/
static int strappend( char *s1, int s1max, char *s2 )
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
static int tokenlength( char *begtok, char c )
{
  char    *endtok;   /* points to the end of this token */

  endtok = strchr( begtok, c );
  if( endtok == NULL ) return( (int)strlen(begtok) );
  return( (int)(endtok-begtok) );
}

/***********************************************************************
 * findItemValue() finds the 'value' part of a "name=value" data       *
 * string.                                                             *
 *   dataStr - data string to parse                                    *
 *   nameStr - item name string to find in the data string             *
 * Returns a pointer to the start of the item 'value' string, or NULL  *
 *  if the 'name' or 'value' data could not be parsed.                 *
 ***********************************************************************/
static char *findItemValue(const char *dataStr, const char *nameStr)
{
  char *ptr;

  if((ptr=strstr(dataStr,nameStr)) == NULL)      /* find item name */
    return NULL;
  if(*(ptr+=strlen(nameStr)) != '=')             /* find '=' char */
    return NULL;
  if(*(++ptr) == '\0')
    return NULL;        /* if end of data string then indicate not found */
  return ptr;           /* return pointer to start of value data */
}

/***********************************************************************
 * findItemValue() finds the 'value' part of a "name=value" data       *
 * string.  The value is delimited by a comma, a space, or end of      *
 * string.                                                             *
 *   destStr - destination string buffer to receive 'value' data       *
 *   srcStr - source string to copy 'value' data from                  *
 *   maxLen - maximum length for 'destStr' buffer                      *
 ***********************************************************************/
static void copyItemValue(char *destStr, const char *srcStr, int maxLen)
{
  char *ptr;
  int len;

  if((ptr=strchr(srcStr,',')) != NULL || (ptr=strchr(srcStr,' ')) != NULL)
    len = (int)(ptr - srcStr);         /* found delimiter */
  else
    len = strlen(srcStr);              /* no delimiter, use whole string */
  if(len > maxLen-1)
    len = maxLen-1;               /* make sure length value not too long */
  else if(len < 0)
    len = 0;                      /* make sure length value is sane */
  strncpy(destStr, srcStr, len);       /* copy value data */
  destStr[len] = '\0';                 /* null terminate string */
}
