/***************************************************************************
 *  This code is a part of rayloc_ew / USGS EarthWorm module               *
 *                                                                         *
 *  It is written by ISTI (Instrumental Software Technologies, Inc.)       *
 *          as a part of a contract with CERI USGS.                        *
 * For support contact info@isti.com                                       *
 *   Ilya Dricker (i.dricker@isti.com)                                     *
 *                                                   Aug 2004              *
 ***************************************************************************/

/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: rayloc_message_rw.c 7606 2018-11-26 20:47:34Z alexander $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2006/01/17 21:38:06  friberg
 *     added 2 new .c files for global processing
 *
 *     Revision 1.1  2004/08/05 04:15:11  friberg
 *     First commit of rayloc_ew in EW-CENTRAL CVS
 *
 *     Revision 1.5  2004/08/04 19:27:54  ilya
 *     Towards version 1.0
 *
 *     Revision 1.4  2004/08/03 18:26:05  ilya
 *     Now we use stock EW functions from v6.2
 *
 *     Revision 1.3  2004/08/03 17:51:47  ilya
 *     Finalizing the project: using EW globals
 *
 *     Revision 1.2  2004/07/29 17:28:54  ilya
 *     Fixed makefile.sol; added makefile.sol_gcc; tested cc compilation
 *
 *     Revision 1.1.1.1  2004/06/22 21:12:06  ilya
 *     initial import into CVS
 *
 */
/***************************************************************************
                          rayloc_message_rw.c  -  description
                             -------------------
    begin                : Wed Jun 16 2004
    copyright            : (C) 2004 by Ilya Dricker, ISTI
    email                : i.dricker@isti.com
 ***************************************************************************/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "earthworm.h"
#include "chron3.h"
#include "kom.h"  /* string parsing */

#include "rayloc_message_rw.h"
#include "global_loc_rw.h"

/*
** ===============================================================================
*/
void
	rayloc_epoch17string( double secs, char *c17 )
	{
		double epoch_secs = secs + GSEC1970;
		(void)date17( epoch_secs, c17 );
	}

/*
** ===============================================================================
*/
RAYLOC_MSG_STATUS
	rayloc_fileToRaylocHeader( RAYLOC_MESSAGE_HEADER_STRUCT ** pp_struct,
	                          char * pathname,
	                          int event_id,
	                          unsigned char headerType,
	                          unsigned char headerMod,
	                          unsigned char headerInst,
	                          unsigned char picksType,
	                          unsigned char picksMod,
	                          unsigned char picksInst)
	{
		FILE       *fd;
		int        retVal;
		int        i = 0;
		char       *headerString = NULL;
		char       *picksString = NULL;

		fd = fopen(pathname, "r");
		if (!fd)
		{
			logit( "pt", "ERROR: rayloc_fileToRaylocHeader:  failed to open pathname %s\n", pathname);
			return RAYLOC_MSG_NULL;
		}

		headerString = rayloc_grab_header_stringFromFile(fd);
		if (!headerString)
		{
			logit( "pt", "ERROR: rayloc_fileToRaylocHeader:  failed to read header from RAYLOC TMP file %s\n", pathname);
			fclose(fd);
			return RAYLOC_MSG_NULL;
		}

		retVal = rayloc_stringToRaylocHeader(pp_struct, headerString, event_id, headerType, headerMod, headerInst);
		free(headerString);
		if (RAYLOC_MSG_SUCCESS != retVal)
		{
			logit( "pt", "ERROR: rayloc_fileToRaylocHeader:  failed to create RAYLOC_MESSAGE_HEADER_STRUCT from %s\n",
			    pathname);
			fclose(fd);
			return retVal;
		}

		while(1)
		{
			picksString = rayloc_grab_picks_stringFromFile(fd);
			if (!picksString)
			{
				break;
			}
			retVal = rayloc_stringeToPicks(&(*pp_struct)->picks[i], picksString, picksType, picksMod, picksInst);
			free(picksString);
			if (RAYLOC_MSG_SUCCESS != retVal)
			{
				logit( "pt", "ERROR: rayloc_fileToRaylocHeader:  failed to read PICK info from RAYLOC TMP file %s\n", pathname);
				fclose(fd);
				return retVal;
			}
			i++;
			(*pp_struct)->numPicks = i;
		}

		fclose(fd);
		return RAYLOC_MSG_SUCCESS;
	}


/*
** ===============================================================================
*/
int
	rayloc_how_many_tokens(const char *myString)
	{
		int             i;
		char            prevChar = 'x'; /* THis means the letter is a symbol, mot space */
		int             tokenCounter = 0;

		/* Check if the string is sane */
		if (myString == NULL)
			return 0;
		if (strlen(myString) == 0)
			return 0;

		/* Init */
		if (myString[0] == ' ' || myString[0] == '\t' || myString[0] == '\n')
			prevChar = ' ';
		else
			tokenCounter++;

		/* Main loop */
		for (i = 1; i < (int)strlen(myString); i++)
		{
			if (myString[i] == ' ' || myString[i] == '\t' || myString[i] == '\n')
			{
				if (prevChar == 'x')    /* space after the word */
				{
					prevChar = ' ';
					/* else -> do nothing: space continues */
				}
			}
			else    /* got letter */
			{
				if (prevChar == ' ')    /* word after the space */
				{
					prevChar = 'x';
					tokenCounter++;
					/* else -> do nothing word continues */
				}
			}
		}
		return tokenCounter;
	}

/*
** ===============================================================================
*/
RAYLOC_MSG_STATUS
	rayloc_InitRaylocHeader( RAYLOC_MESSAGE_HEADER_STRUCT ** pp_rlc )
	{

		 RAYLOC_MESSAGE_HEADER_STRUCT *p_rlc = *pp_rlc;

		if (!p_rlc)
		{
			p_rlc = (RAYLOC_MESSAGE_HEADER_STRUCT *) calloc(1, sizeof(RAYLOC_MESSAGE_HEADER_STRUCT));
			if (!p_rlc)
			{
				logit( "pt", "ERROR: rayloc_InitRaylocHeader:  failed to allocated RAYLOC_MESSAGE_HEADER_STRUCT\n");
				return RAYLOC_MSG_NULL;
			}
		}
		else
			memset(p_rlc, 0, sizeof(RAYLOC_MESSAGE_HEADER_STRUCT));
		p_rlc->version     = RAYLOC_MESSAGE_HEADER_VERSION;
		strcpy( p_rlc->origin_time_char      , "?" );

		*pp_rlc = p_rlc;
		return RAYLOC_MSG_SUCCESS;
	}

/*
** ===============================================================================
*/
RAYLOC_MSG_STATUS
	rayloc_InitRaylocPicks( RAYLOC_PICKS_STRUCT ** pp_picks )
	{
		RAYLOC_PICKS_STRUCT *p_picks = *pp_picks;
		if (!p_picks)
		{
			p_picks = (RAYLOC_PICKS_STRUCT *) calloc(1, sizeof(RAYLOC_PICKS_STRUCT));
			if (!p_picks)
			{
				logit( "pt", "ERROR: rayloc_InitRaylocPicks:  failed to allocated RAYLOC_PICKS_STRUCT\n");
				return RAYLOC_MSG_NULL;
			}
		}
		else
			memset(p_picks, 0, sizeof(RAYLOC_PICKS_STRUCT));
		p_picks->version     = RAYLOC_PICKS_VERSION;
		strcpy( p_picks->station      , "?" );
		strcpy( p_picks->channel      , "?" );
		strcpy( p_picks->network      , "?" );
		strcpy( p_picks->location     , "?" );
		strcpy( p_picks->phase_name   , "?" );

		*pp_picks = p_picks;
		return RAYLOC_MSG_SUCCESS;
	}



/*
** ===============================================================================
*/
RAYLOC_MSG_STATUS
	rayloc_stringToRaylocHeader( RAYLOC_MESSAGE_HEADER_STRUCT ** pp_struct,
	                     char * p_string,
	                     int event_id,
	                     unsigned char headerType,
	                     unsigned char headerMod,
	                     unsigned char headerInst)
	{
		int          retVal;
		int          free_flag = 1;
		if (*pp_struct)
			free_flag = 0;
		if (RAYLOC_MSG_SUCCESS != (retVal = rayloc_InitRaylocHeader(pp_struct)))
		{
			logit( "pt", "ERROR: rayloc_stringToRaylocHeader:  rayloc_InitRaylocHeader FAILED\n");
			return retVal;
		}

		if (!p_string)
		{
			if (free_flag)
				rayloc_FreeRaylocHeader(pp_struct);
			logit( "pt", "ERROR: rayloc_stringToRaylocHeader:  INPUT STRING is NULL\n");
			return RAYLOC_MSG_BADPARAM;
		}
		retVal = rayloc_how_many_tokens(p_string);
		if (RAYLOC_HEADER_TOKENS_IN_FILE_STRING != retVal)
		{
			if (free_flag)
				rayloc_FreeRaylocHeader(pp_struct);
			logit( "pt", "ERROR: rayloc_stringToRaylocHeader:  INVALID MESSAGE LINE\n");
			return RAYLOC_MSG_FORMATERROR;
		}

		sscanf(p_string,
		 "%lf %lf %lf %lf %d %d %d %d %lf %lf %c %lf %lf %lf %lf %lf %lf %lf %lf %c %lf %lf %lf %lf %lf %lf %lf %lf %lf",
		          &(*pp_struct)->origin_time,
		          &(*pp_struct)->elat,
		          &(*pp_struct)->elon,
		          &(*pp_struct)->edepth,
		          &(*pp_struct)->nsta,
		          &(*pp_struct)->npha,
		          &(*pp_struct)->suse,
		          &(*pp_struct)->puse,
		          &(*pp_struct)->gap,
		          &(*pp_struct)->dmin,
		          &(*pp_struct)->depth_flag,
		          &(*pp_struct)->oterr,
		          &(*pp_struct)->laterr,
		          &(*pp_struct)->lonerr,
		          &(*pp_struct)->deperr,
		          &(*pp_struct)->se,
		          &(*pp_struct)->errh,
		          &(*pp_struct)->errz,
		          &(*pp_struct)->avh,
		          &(*pp_struct)->q,
		          &(*pp_struct)->axis[0],
		          &(*pp_struct)->az[0],
		          &(*pp_struct)->dp[0],
		          &(*pp_struct)->axis[1],
		          &(*pp_struct)->az[1],
		          &(*pp_struct)->dp[1],
		          &(*pp_struct)->axis[2],
		          &(*pp_struct)->az[2],
		          &(*pp_struct)->dp[2]);

		(*pp_struct)->logo.type = headerType;
		(*pp_struct)->logo.mod = headerMod;
		(*pp_struct)->logo.instid = headerInst;
		rayloc_epoch17string((*pp_struct)->origin_time, (*pp_struct)->origin_time_char);
		(*pp_struct)->event_id = event_id;

		return RAYLOC_MSG_SUCCESS;
	}

/*
** ===============================================================================
*/
RAYLOC_MSG_STATUS
	rayloc_stringeToPicks( RAYLOC_PICKS_STRUCT ** pp_struct,
	                     char * p_string,
	                     unsigned char picksType,
	                     unsigned char picksMod,
	                     unsigned char picksInst)
	{
		int retVal;
		if (RAYLOC_MSG_SUCCESS != (retVal = rayloc_InitRaylocPicks(pp_struct)))
		{
			logit( "pt", "ERROR: rayloc_stringeToPicks:  rayloc_InitRaylocPicks FAILED\n");
			return retVal;
		}

		if (!p_string)
		{
			logit( "pt", "ERROR: rayloc_stringeToPicks: INPUT string is NULL\n");
			rayloc_FreeRaylocPicks(pp_struct);
			return RAYLOC_MSG_BADPARAM;
		}
		retVal = rayloc_how_many_tokens(p_string);
		if (RAYLOC_PICKS_TOKENS_IN_FILE_STRING == retVal)
			sscanf(p_string, "%ld %s %s %s %s %s %lf %lf %lf %c",
			          &(*pp_struct)->pick_id,
			          (*pp_struct)->station,
			          (*pp_struct)->channel,
			          (*pp_struct)->network,
			          (*pp_struct)->location,
			          (*pp_struct)->phase_name,
			          &(*pp_struct)->residual,
			          &(*pp_struct)->dist,
			          &(*pp_struct)->az,
			          &(*pp_struct)->weight_flag);
		else if (RAYLOC_PICKS_TOKENS_IN_FILE_STRING - 1  == retVal)
			sscanf(p_string, "%ld %s %s %s %s %lf %lf %lf %c",
			          &(*pp_struct)->pick_id,
			          (*pp_struct)->station,
			          (*pp_struct)->channel,
			          (*pp_struct)->network,
			          (*pp_struct)->phase_name,
			          &(*pp_struct)->residual,
			          &(*pp_struct)->dist,
			          &(*pp_struct)->az,
			          &(*pp_struct)->weight_flag);
		else
		{
			rayloc_FreeRaylocPicks(pp_struct);
			logit( "pt", "ERROR: rayloc_stringeToPicks: INPUT string is INVALID\n");
			return RAYLOC_MSG_FORMATERROR;
		}

		(*pp_struct)->logo.type = picksType;
		(*pp_struct)->logo.mod = picksMod;
		(*pp_struct)->logo.instid = picksInst;


		return RAYLOC_MSG_SUCCESS;
	}

/*
** ===============================================================================
*/
RAYLOC_MSG_STATUS
	rayloc_ClearRayLocHeader( RAYLOC_MESSAGE_HEADER_STRUCT * p_struct )
	{
		logit( "pt", "ERROR: rayloc_ClearRayLocHeader: FUNCTION iS NOT IMPLEMENTED\n");
		return RAYLOC_MSG_SUCCESS;
	}

/*
** ===============================================================================
*/
RAYLOC_MSG_STATUS
	rayloc_ClearRaylocPicks( RAYLOC_PICKS_STRUCT * p_struct )
{
	logit( "pt", "WARNING: rayloc_ClearRaylocPicks: FUNCTION iS NOT TESTED: Do not use it\n");
	if ( p_struct == NULL )
	{
		return RAYLOC_MSG_NULL;
	}
	if ( p_struct->version != RAYLOC_PICKS_VERSION )
	{
		return RAYLOC_MSG_VERSINVALID;
	}
	memset(p_struct, 0, sizeof(RAYLOC_MESSAGE_HEADER_STRUCT));
	return RAYLOC_MSG_SUCCESS;
}

/*
** ===============================================================================
*/
void
	rayloc_FreeRaylocPicks( RAYLOC_PICKS_STRUCT ** pp_struct )
	{
		if (NULL == *pp_struct)
			return;
		free(*pp_struct);
		*pp_struct = NULL;
	}

/*
** ===============================================================================
*/
void
	rayloc_FreeRaylocHeader( RAYLOC_MESSAGE_HEADER_STRUCT ** pp_struct )
	{
		int          i;

		if (NULL == *pp_struct)
			return;
		for (i = 0; i < RAYLOC_MAX_PICKS; i++)
			rayloc_FreeRaylocPicks(&(*pp_struct)->picks[i]);

		free(*pp_struct);
		*pp_struct = NULL;
	}


/*
** ===============================================================================
*/
char *
	rayloc_grab_header_stringFromFile(FILE *fd)
	{
		char                    tmpString1[RAYLOC_MESSAGE_HEADER_MAXBUFSIZE];
		char                    tmpString2[RAYLOC_MESSAGE_HEADER_MAXBUFSIZE];
		char                    tmpString3[RAYLOC_MESSAGE_HEADER_MAXBUFSIZE];
		int                     bufLen;
		char                    *out = NULL;

		if (!fgets(tmpString1, RAYLOC_MESSAGE_HEADER_MAXBUFSIZE, fd))
			return NULL;
		if (!fgets(tmpString2, RAYLOC_MESSAGE_HEADER_MAXBUFSIZE, fd))
			return NULL;
		if (!fgets(tmpString3, RAYLOC_MESSAGE_HEADER_MAXBUFSIZE, fd))
			return NULL;

		bufLen = strlen(tmpString1) + strlen(tmpString2) + strlen(tmpString3) + 3;
		if (bufLen < 100 )
			return NULL;

		out = (char *) calloc(bufLen, 1);
		if (!out)
			return NULL;

		sprintf(out, "%s %s %s", tmpString1, tmpString2, tmpString3);

		return out;
	}

/*
** ===============================================================================
*/
char *
	rayloc_grab_picks_stringFromFile(FILE *fd)
	{
		char                    *out = NULL;
		char                    tmpString[RAYLOC_MESSAGE_PICKS_MAXBUFFER];

		if (!fgets(tmpString, RAYLOC_MESSAGE_PICKS_MAXBUFFER, fd))
			return NULL;

		if (strlen(tmpString) < 20 )
			return NULL;

		out = (char *) calloc(strlen(tmpString) + 1, 1);
		if (!out)
			return NULL;

		sprintf(out, "%s", tmpString);

		return out;
	}

/*
** ===============================================================================
*/
char
	*rayloc_WriteRaylocHeaderBuffer( RAYLOC_MESSAGE_HEADER_STRUCT * p_struct, unsigned int *p_length, int *errNo )
  {
		char *out = NULL;
		char tmp[RAYLOC_MESSAGE_HEADER_MAXBUFSIZE];
		char _author[10];
		char *picksString = NULL;
		int i;
		unsigned int picksLen;
		int picksErrNo;
		int outLen;

		if ( p_struct == NULL )
		{
			logit( "pt", "ERROR: rayloc_WriteRaylocHeaderBuffer: INPUT RAYLOC_MESSAGE_HEADER_STRUCT is NULL\n");
			*errNo = RAYLOC_MSG_NULL;
			return NULL;
		}

		if ( p_struct->version != RAYLOC_MESSAGE_HEADER_VERSION )
		{
			logit( "pt",
			  "ERROR: rayloc_WriteRaylocHeaderBuffer: INPUT RAYLOC_MESSAGE_HEADER_STRUCT  version (%d) is different from supported %d\n",
			  p_struct->version, RAYLOC_MESSAGE_HEADER_VERSION);
			*errNo = RAYLOC_MSG_VERSINVALID;
			return NULL;
		}
		EncodeAuthor( p_struct->logo, _author );
		sprintf(tmp,
		  "RLC %s %d %ld %s %.4f %.4f %.3f %d %d %d %d %.3f %.3f %c %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %c %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f %.3f\n",
		          _author,
		          p_struct->version,
		          p_struct->event_id ,
		          p_struct->origin_time_char ,
		          p_struct->elat ,
		          p_struct->elon ,
		          p_struct->edepth ,
		          p_struct->nsta ,
		          p_struct->npha ,
		          p_struct->suse ,
		          p_struct->puse ,
		          p_struct->gap ,
		          p_struct->dmin ,
		          p_struct->depth_flag ,
		          p_struct->oterr ,
		          p_struct->laterr ,
		          p_struct->lonerr ,
		          p_struct->deperr ,
		          p_struct->se ,
		          p_struct->errh ,
		          p_struct->errz ,
		          p_struct->avh ,
		          p_struct->q ,
		          p_struct->axis[0] ,
		          p_struct->az[0] ,
		          p_struct->dp[0] ,
		          p_struct->axis[1] ,
		          p_struct->az[1] ,
		          p_struct->dp[1] ,
		          p_struct->axis[2] ,
		          p_struct->az[2] ,
		          p_struct->dp[2]);

		out = strdup(tmp);
		if (!out)
		{
			logit( "pt", "ERROR: rayloc_WriteRaylocHeaderBuffer: FAILED to copy OUTPUT MESSAGE STRING\n");
			*errNo = RAYLOC_MSG_NULL;
			return NULL;
		}

		outLen = strlen(out);
		for (i = 0; i < p_struct->numPicks; i++)
		{
			picksString = rayloc_WriteRaylocPicksBuffer(p_struct->picks[i], &picksLen, &picksErrNo);
			if (!picksString)
			{
				logit( "pt", "ERROR: rayloc_WriteRaylocHeaderBuffer: rayloc_WriteRaylocPicksBuffer FAILED\n");
				free(out);
				*errNo = RAYLOC_MSG_NULL;
				return NULL;
			}

			out = realloc(out, strlen(out) + picksLen + 3);
			if (!out)
			{
				logit( "pt", "ERROR: rayloc_WriteRaylocHeaderBuffer: FAILED to REALLOC OUTPUT STRING\n");
				free(picksString);
				*errNo = RAYLOC_MSG_NULL;
				return NULL;
			}
			strcat(out, picksString);
			outLen = outLen + picksLen;
			free(picksString);
		}
		strcat(out, "\n");
		outLen++;
		*p_length = strlen(out);
		return out;
	}


/*
** ===============================================================================
*/
char *
	rayloc_WriteRaylocPicksBuffer( RAYLOC_PICKS_STRUCT * p_struct, unsigned int *p_length, int *errNo)
	{
		char *out = NULL;
		char _author[10];
		char tmp[RAYLOC_MESSAGE_PICKS_MAXBUFFER];

		if ( p_struct == NULL )
		{
			logit( "pt", "ERROR: rayloc_WriteRaylocPicksBuffer: FAILED to ALLOC RAYLOC_PICKS_STRUCT\n");
			*errNo = RAYLOC_MSG_NULL;
			return NULL;
		}

		if ( p_struct->version != RAYLOC_PICKS_VERSION )
		{
			logit( "pt", "ERROR: rayloc_WriteRaylocPicksBuffer: RAYLOC_PICKS_STRUCT version %d is different from supported %d\n",
			    p_struct->version, RAYLOC_PICKS_VERSION);
			*errNo = RAYLOC_MSG_VERSINVALID;
			return NULL;
		}

		EncodeAuthor( p_struct->logo, _author );
		sprintf(tmp,
		  "PCK %s %d %ld %s %s %s %s %s %.3f %.3f %.3f %c\n",
		          _author,
		          p_struct->version ,
		          p_struct->pick_id ,
		          p_struct->station ,
		          p_struct->channel ,
		          p_struct->network ,
		          p_struct->location ,
		          p_struct->phase_name ,
		          p_struct->residual ,
		          p_struct->dist ,
		          p_struct->az ,
		          p_struct->weight_flag);

		out = strdup(tmp);
		if (!out)
		{
			logit( "pt", "ERROR: rayloc_WriteRaylocPicksBuffer: FAILED to copy output string\n");
			*errNo = RAYLOC_MSG_NULL;
			return NULL;
		}
		*p_length = strlen(out);

		return out;

	}

/*
** ===============================================================================
*/

void
	rayloc_logRayloc(RAYLOC_MESSAGE_HEADER_STRUCT *p_struct)
	{
		int i;
		logit( "", "============= PRINTING OUTPUT RAYLOC MESSAGE =============\n");
		logit( "",
		    "RAYLOC MSG (v. %d) message (type %d ; mod %d instId %d) event id = %ld origin = %s\n",
		   p_struct->version, p_struct->logo.type, p_struct->logo.mod,
		   p_struct->logo.instid, p_struct->event_id, p_struct->origin_time_char);

		logit("",    "     Origin        elat     elon    depth nsta npha suse puse gap  dmin  f\n");
		logit("", "%14.3f %+7.4f %+8.4f %6.2f %4d %4d %4d %4d %3ld %6.2f ",
		           p_struct->origin_time, p_struct->elat, p_struct->elon, p_struct->edepth,
		           p_struct->nsta, p_struct->npha, p_struct->suse, p_struct->puse,
		           lround(p_struct->gap), p_struct->dmin);
		logit("", "%c\n", p_struct->depth_flag);

		logit("",    " oterr laterr lonerr deperr   se    errh   errz    avh  q\n");
		logit("",    "%6.2f %6.1f %6.1f %6.1f %6.2f %6.1f %6.1f %6.1f\n",
		   p_struct->oterr , p_struct->laterr , p_struct->lonerr , p_struct->deperr ,
			p_struct->se , p_struct->errh , p_struct->errz , p_struct->avh );

		logit("",    " axis1 az1 dp1  axis2 az2 dp2  axis3 az3 dp3\n");
		logit("",    "%6.1f %3lf %+2lf %6.1lf %3lf %+2lf %6.1lf %3lf %+2lf \n",
		    p_struct->axis[0] , p_struct->az[0] , p_struct->dp[0] ,
		    p_struct->axis[1] , p_struct->az[1] , p_struct->dp[1] ,
		    p_struct->axis[2] , p_struct->az[2] , p_struct->dp[2]);

		logit("",    "  PickID    sta  cmp nt lc  phase    res   dist azm w\n");
		for (i = 0; i < p_struct->numPicks; i++)
		{
			logit("", "%10ld %5s %3s %2s %2s %8s %+4.1f %5.1f %3ld ",
			    (p_struct->picks[i])->pick_id ,
			    (p_struct->picks[i])->station ,
			    (p_struct->picks[i])->channel ,
			    (p_struct->picks[i])->network ,
			    (p_struct->picks[i])->location ,
			    (p_struct->picks[i])->phase_name ,
			    (p_struct->picks[i])->residual ,
			    (p_struct->picks[i])->dist ,
			    lround((p_struct->picks[i])->az) );
			logit("", "%c\n", (p_struct->picks[i])->weight_flag );
		}
		logit( "", "============================================================\n");
		return;
	}
/*
** ===============================================================================
*/
RAYLOC_MSG_STATUS
        rayloc_MessageToRaylocHeader( RAYLOC_MESSAGE_HEADER_STRUCT ** pp_struct, char * p_message)
	{
		int retVal;
		RAYLOC_MESSAGE_HEADER_BUFFER workbuff;
		char _str[80];
		int i;
		char * _readpoint = p_message;
		char * _linemark;
		char _readingchildren = 1;
		int count = 0;
		int  _index;



		if ( *pp_struct == NULL )
		{
			*pp_struct = (RAYLOC_MESSAGE_HEADER_STRUCT *) calloc(1, sizeof(RAYLOC_MESSAGE_HEADER_STRUCT));
			if (! (*pp_struct ))
			{
				logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: Failed to allocate RAYLOC_MESSAGE_HEADER_STRUCT \n");
				return RAYLOC_MSG_NULL;
			}
		}

		if ( p_message == NULL )
		{
			logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: RAYLOC HEADER MESSAGE is NULL\n");
			return RAYLOC_MSG_BADPARAM;
		}

		retVal = rayloc_InitRaylocHeader( pp_struct);
		if (RAYLOC_MSG_SUCCESS != retVal)
		{
			logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: rayloc_InitRaylocHeader FAILED: retVal = %d\n", retVal);
			return retVal;
		}

		/* Find the end of the SUM line */
		if ( (_linemark = strchr( _readpoint, '\n' )) == NULL )
		{
			/* didn't find the newline delimiter */
			logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: Failed to find the end of message string A\n");
			return GLOBAL_MSG_FORMATERROR;
		}
		_index = _linemark - _readpoint;

		strncpy( workbuff , _readpoint , _index );
		workbuff[_index] = '\0';


		k_put(workbuff);

		strcpy( _str , k_str() );

		if (0 != strcmp(_str, "RLC"))
		{
			logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: Message is not a RLC (RAYLOC_TYPE) \n");
			return RAYLOC_MSG_FORMATERROR;

		}
		/*  Author  */
		strcpy( _str , k_str() );

		if ( strlen(_str) == 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: failed to read (author) \n");
			return RAYLOC_MSG_FORMATERROR;
		}

		if ( DecodeAuthor( &((*pp_struct)->logo), _str ) != RAYLOC_MSG_SUCCESS )
		{
			logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: failed to decode (logo) \n");
			return RAYLOC_MSG_FORMATERROR;
		}
		/*  version number  */
		(*pp_struct)->version = (short)k_long();
		if ( k_err() != 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: failed to read (version) \n");
			return RAYLOC_MSG_FORMATERROR;
		}

		/*  event id  */
		(*pp_struct)->event_id = k_long();
		if ( k_err() != 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: failed to read (event id) \n");
			return RAYLOC_MSG_FORMATERROR;
		}

		/*  Origin Time  */
		strcpy( (*pp_struct)->origin_time_char , k_str() );
		if ( strlen( (*pp_struct)->origin_time_char) == 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: failed to read (oringin time) \n");
			return RAYLOC_MSG_FORMATERROR;
		}
		(*pp_struct)->origin_time = julsec17( (*pp_struct)->origin_time_char );

		/*  Latitude  */
		(*pp_struct)->elat = k_val();
		if ( k_err() != 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: failed to read (Latitude) \n");
			return RAYLOC_MSG_FORMATERROR;
		}

		/*  Longitude  */
		(*pp_struct)->elon = k_val();
		if ( k_err() != 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: failed to read (Longitude) \n");
			return RAYLOC_MSG_FORMATERROR;
		}

		/*  Depth  */
		(*pp_struct)->edepth = k_val();
		if ( k_err() != 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: failed to read (Depth) \n");
			return RAYLOC_MSG_FORMATERROR;
		}

		/*  number of stations  */
		(*pp_struct)->nsta = k_long();
		if ( k_err() != 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: failed to read (number of stations) \n");
			return RAYLOC_MSG_FORMATERROR;
		}

		/*  number of phases  */
		(*pp_struct)->npha = k_long();
		if ( k_err() != 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: failed to read (number of phases) \n");
			return RAYLOC_MSG_FORMATERROR;
		}

		/*  number of stations used  */
		(*pp_struct)->suse = k_long();
		if ( k_err() != 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: failed to read (number of stations used) \n");
			return RAYLOC_MSG_FORMATERROR;
		}

		/*  Number of phases used */
		(*pp_struct)->puse = k_long();
		if ( k_err() != 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: failed to read (Number of phases used) \n");
			return RAYLOC_MSG_FORMATERROR;
		}

		/*  Gap  */
		(*pp_struct)->gap = k_val();
		if ( k_err() != 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: failed to read (Gap) \n");
			return RAYLOC_MSG_FORMATERROR;
		}

		/*  dmin  */
		(*pp_struct)->dmin = k_val();
		if ( k_err() != 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: failed to read (dmin) \n");
			return RAYLOC_MSG_FORMATERROR;
		}

		/*  depth flag  */
		strncpy( &(*pp_struct)->depth_flag , k_str(), 1);

		/*  oterr  */
		(*pp_struct)->oterr = k_val();
		if ( k_err() != 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: failed to read (oterr) \n");
			return RAYLOC_MSG_FORMATERROR;
		}

		/*  laterr  */
		(*pp_struct)->laterr = k_val();
		if ( k_err() != 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: failed to read (laterr) \n");
			return RAYLOC_MSG_FORMATERROR;
		}

		/*  lonerr  */
		(*pp_struct)->lonerr = k_val();
		if ( k_err() != 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: failed to read (lonerr) \n");
			return RAYLOC_MSG_FORMATERROR;
		}

		/*  deperr  */
		(*pp_struct)->deperr = k_val();
		if ( k_err() != 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: failed to read (deperr) \n");
			return RAYLOC_MSG_FORMATERROR;
		}

		/*  se  */
		(*pp_struct)->se = k_val();
		if ( k_err() != 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: failed to read (se) \n");
			return RAYLOC_MSG_FORMATERROR;
		}

		/*  errh  */
		(*pp_struct)->errh = k_val();
		if ( k_err() != 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: failed to read (errh) \n");
			return RAYLOC_MSG_FORMATERROR;
		}

		/*  errz  */
		(*pp_struct)->errz = k_val();
		if ( k_err() != 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: failed to read (Latitude) \n");
			return RAYLOC_MSG_FORMATERROR;
		}

		/*  avherr  */
		(*pp_struct)->avh = k_val();
		if ( k_err() != 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: failed to read (avherr) \n");
			return RAYLOC_MSG_FORMATERROR;
		}

		/*  q */
		strncpy( &(*pp_struct)->q , k_str(), 1);


		for (i = 0; i < 3; i++)
		{
			/*  axis  */
			(*pp_struct)->axis[i] = k_val();
			if ( k_err() != 0 )
			{
				logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: failed to read (axis) \n");
				return RAYLOC_MSG_FORMATERROR;
			}

			/*  az  */
			(*pp_struct)->az[i] = k_val();
			if ( k_err() != 0 )
			{
				logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: failed to read (az) \n");
				return RAYLOC_MSG_FORMATERROR;
			}

			/*  dp  */
			(*pp_struct)->dp[i] = k_val();
			if ( k_err() != 0 )
			{
				logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: failed to read (dp) \n");
				return RAYLOC_MSG_FORMATERROR;
			}
		}

		do
		{
			/* Set read point to start of next child line */
			_readpoint = _linemark + 1;

			/*
			** need the \r to handle file2ring
			*/
			if ( *_readpoint == '\n' || *_readpoint == '\r' )
			{
				/* next char in file is newline, this indicated file termination */
				_readingchildren = 0;
				break;
			}

			if ( strncmp( _readpoint, "PCK ", 4 ) == 0 )
			{
				/* Phase Line */
				if (count  ==  RAYLOC_MAX_PICKS )
				{
					/* no space for any more Phases */
					logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: more phases than space allocated \n");
					return RAYLOC_MSG_MAXCHILDREN;
				}

				/* Advance the pointer to the first data location */
				_readpoint += 4;

				/* Find the end of the PCK line */
				if ( (_linemark = strchr( _readpoint, '\n' )) == NULL )
				{
					/* didn't find the newline delimiter */
					logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: Failed to find the end of PICKS line \n");
					return RAYLOC_MSG_FORMATERROR;
				}

				/* null-terminate the PHS line (into a string) */
				*_linemark = '\0';

				/* parse the working buffer into the location's next Phase struct */
				if ( (retVal = rayloc_MessageToPicksBuffer(&(((*pp_struct)->picks)[count]), _readpoint )) != RAYLOC_MSG_SUCCESS )
				{
					logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: rayloc_MessageToPicksBuffer FAILED \n");
					return retVal;
				}

				/* increment the phase count */
				count++;
				/* End of PHS Line */
			}
			else
			{
				/* Unrecognized Line start tag */
				logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: Unrecognized Line start tag \n");
				return RAYLOC_MSG_BADCHILD;
			}
		} while( _readingchildren == 1 );
		if ( (*pp_struct)->npha != count )
		{
			logit( "pt", "ERROR: rayloc_MessageToRaylocHeader: Actual number of phases in message different from nominal \n");
			return RAYLOC_MSG_FORMATERROR;
			}

		(*pp_struct)->numPicks = count;
		return RAYLOC_MSG_SUCCESS;
	}

/*
** ===============================================================================
*/
RAYLOC_MSG_STATUS
	rayloc_MessageToPicksBuffer( RAYLOC_PICKS_STRUCT ** pp_struct, char * p_message )
	{
		int retVal;
		char _str[80];
		RAYLOC_MESSAGE_PICKS_BUFFER workbuff;

		if ( *pp_struct == NULL )
		{
			*pp_struct = (RAYLOC_PICKS_STRUCT *) calloc(1, sizeof(RAYLOC_PICKS_STRUCT));
			if (! (*pp_struct ))
			{
				logit( "pt", "ERROR: rayloc_MessageToPicksBuffer: failed to allocate RAYLOC_PICKS_STRUCT  \n");
				return RAYLOC_MSG_NULL;
			}
		}

		if ( p_message == NULL )
		{
			logit( "pt", "ERROR: rayloc_MessageToPicksBuffer: PCK MESSAGE string is NULL  \n");
			return RAYLOC_MSG_BADPARAM;
		}
		if(RAYLOC_MSG_SUCCESS != (retVal = rayloc_InitRaylocPicks(pp_struct)))
		{
			logit( "pt", "ERROR: rayloc_MessageToPicksBuffer: rayloc_InitRaylocPicks FAILED  \n");
			return retVal;
		}

		if ( RAYLOC_MESSAGE_PICKS_MAXBUFFER < strlen( p_message ) )
		{
			strncpy( workbuff , p_message , RAYLOC_MESSAGE_PICKS_MAXBUFFER );
			workbuff[RAYLOC_MESSAGE_PICKS_MAXBUFFER] = '\0';
		}
		else
		{
			strcpy( workbuff, p_message );
		}

		k_put(workbuff);

		/*  Author  */
		strcpy( _str , k_str() );
		if ( strlen(_str) == 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToPicksBuffer:  failed to read (AUTHOR) \n");
			return GLOBAL_MSG_FORMATERROR;
		}

		if ( DecodeAuthor( &((*pp_struct)->logo), _str ) != RAYLOC_MSG_SUCCESS )
		{
			logit( "pt", "ERROR: rayloc_MessageToPicksBuffer:  failed to decode (LOGO) \n");
			return RAYLOC_MSG_FORMATERROR;
		}

		/*  version number  */
		(*pp_struct)->version = (short)k_long();
		if ( k_err() != 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToPicksBuffer:  failed to read (VERSION) \n");
			return RAYLOC_MSG_FORMATERROR;
		}

		/*  pick id  */
		(*pp_struct)->pick_id = k_long();
		if ( k_err() != 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToPicksBuffer:  failed to read (PICK ID) \n");
			return RAYLOC_MSG_FORMATERROR;
		}

		/*  station  */
		strcpy( (*pp_struct)->station , k_str() );
		if ( strlen( (*pp_struct)->station) == 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToPicksBuffer:  failed to read (STATION) \n");
			return RAYLOC_MSG_FORMATERROR;
		}

		/*  channel  */
		strcpy( (*pp_struct)->channel , k_str() );
		if ( strlen( (*pp_struct)->channel) == 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToPicksBuffer:  failed to read (CHANNEL) \n");
			return RAYLOC_MSG_FORMATERROR;
		}

		/*  network  */
		strcpy( (*pp_struct)->network , k_str() );
		if ( strlen( (*pp_struct)->network) == 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToPicksBuffer:  failed to read (NETWORK) \n");
			return RAYLOC_MSG_FORMATERROR;
		}

		/*  location  */
		strcpy( (*pp_struct)->location , k_str() );
		if ( strlen( (*pp_struct)->location) == 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToPicksBuffer:  failed to read (LOcATION ID) \n");
			return RAYLOC_MSG_FORMATERROR;
		}

		/*  phase_name  */
		strcpy( (*pp_struct)->phase_name , k_str() );
		if ( strlen( (*pp_struct)->phase_name) == 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToPicksBuffer:  failed to read (PHASE NAME) \n");
			return RAYLOC_MSG_FORMATERROR;
		}

		/*  residual  */
		(*pp_struct)->residual = k_val();
		if ( k_err() != 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToPicksBuffer:  failed to read (RESIDUAL) \n");
			return RAYLOC_MSG_FORMATERROR;
		}

		/*  dist  */
		(*pp_struct)->dist = k_val();
		if ( k_err() != 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToPicksBuffer:  failed to read (DISTANCE) \n");
			return RAYLOC_MSG_FORMATERROR;
		}

		/*  az */
		(*pp_struct)->az = k_val();
		if ( k_err() != 0 )
		{
			logit( "pt", "ERROR: rayloc_MessageToPicksBuffer:  failed to read (AZIMUTH) \n");
			return RAYLOC_MSG_FORMATERROR;
		}

		/*  weight_flag */
		strncpy( &(*pp_struct)->weight_flag , k_str(), 1);

		return RAYLOC_MSG_SUCCESS;
	}

