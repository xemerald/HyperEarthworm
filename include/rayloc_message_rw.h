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
 *    $Id: rayloc_message_rw.h 2050 2006-01-17 17:27:29Z friberg $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2006/01/17 17:27:28  friberg
 *     added from MWithers work on global routines
 *
 *     Revision 1.1  2004/08/05 04:15:11  friberg
 *     First commit of rayloc_ew in EW-CENTRAL CVS
 *
 *     Revision 1.6  2004/08/04 19:27:54  ilya
 *     Towards version 1.0
 *
 *     Revision 1.5  2004/08/03 17:51:47  ilya
 *     Finalizing the project: using EW globals
 *
 *     Revision 1.4  2004/07/29 17:28:54  ilya
 *     Fixed makefile.sol; added makefile.sol_gcc; tested cc compilation
 *
 *     Revision 1.3  2004/06/24 16:47:05  ilya
 *     Version compiles
 *
 *     Revision 1.2  2004/06/24 16:32:06  ilya
 *     global_msg.h
 *
 *     Revision 1.1.1.1  2004/06/22 21:12:07  ilya
 *     initial import into CVS
 *
 */

/***************************************************************************
                          rayloc_message_rw.h  -  description
                             -------------------
    begin                : Wed Jun 16 2004
    copyright            : (C) 2004 by Ilya Dricker, ISTI
    email                : i.dricker@isti.com
 ***************************************************************************/

 
#ifndef RAYLOC_MESSAGE_RW_H
#define RAYLOC_MESSAGE_RW_H
/*---------------------------------------------------------------------------*/

#include <global_msg.h>

#ifndef RAYLOC_TRACE_STA_LEN
#define     RAYLOC_TRACE_STA_LEN   7
#endif

#ifndef RAYLOC_TRACE_CHAN_LEN
#define     RAYLOC_TRACE_CHAN_LEN  9
#endif

#ifndef RAYLOC_TRACE_NET_LEN
#define     RAYLOC_TRACE_NET_LEN   9
#endif

#ifndef RAYLOC_TRACE_LOC_LEN
#define     RAYLOC_TRACE_LOC_LEN   3
#endif

#define RAYLOC_PICKS_VERSION (short)1
#define RAYLOC_MESSAGE_HEADER_VERSION (short)1

#define RAYLOC_PICKS_TOKENS_IN_FILE_STRING 10
#define RAYLOC_HEADER_TOKENS_IN_FILE_STRING 29

#define RAYLOC_MSG_SUCCESS 0
#define RAYLOC_MSG_NULL -1
#define RAYLOC_MSG_VERSINVALID -2
#define RAYLOC_MSG_BADPARAM -5
#define RAYLOC_MSG_FORMATERROR -3
#define RAYLOC_MSG_MAXCHILDREN -4
#define RAYLOC_MSG_BADCHILD -7

typedef int RAYLOC_MSG_STATUS;

/*---------------------------------------------------------------------------*/
/* ==========================================================================
**                    SIZE DEFINES AND LIMITS
** ========================================================================= */


/* rayloc pick PCK 01
**
**
**          1         2         3         4         5         6         7         8         9         0         1
** 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
** PCK 001002003 vv xxxxxxxxxx.xxx +xx.xxxx +xxx.xxxx xxx.xx xxxx xxxx xxxx xxxx xxx xxx.xx l
** PCK author version event_id    Origin        elat     elon    depth nsta npha suse puse gap  dmin  f
**
**
**  NOTE: These are not fixed-width fields, the counter is only to assist with in determining the maximum size

         1         2         3         4         5         6         7         8           9         0         1
1234567890123456789012345678901234567890123456789012345678901234567890123456789090123456789012345678901234567890
PCK 001002003 vv xxxxxxxxxx aaaaa aaa aa aa aaaaaaaa +xx.x xxx.x xxx l
PCK author version PickID    sta  cmp nt lc  phase    res   dist azm w

Where:
  PickID   DBMS ID of pick (unsigned long)
  Sta      Station code (upper case)
  Cmp      Component code (upper case)
  Nt       Network code (upper case)
  Lc       Location code
  Phase    Phase code (e.g., 'P', 'S', or 'PKP')
  Res      Travel-time residual in seconds
  Dist     Source-receiver distance in decimal degrees
  Azm      Receiver azimuth in degrees (from the source)
  W        Phase weight flag (i.e., T for 1, F for 0)*
*/

#define RAYLOC_MESSAGE_PICKS_MAXBUFFER (50000 + BUFF_OVERRUN_SIZE)

typedef char RAYLOC_MESSAGE_PICKS_BUFFER[RAYLOC_MESSAGE_PICKS_MAXBUFFER+1];

typedef struct _RAYLOC_PICKS_STRUCT
{
   unsigned short     version; /* only set using InitGlobalLoc */
   MSG_LOGO           logo;
   long               pick_id;
   char               station[RAYLOC_TRACE_STA_LEN];
   char               channel[RAYLOC_TRACE_CHAN_LEN]; /* aka "component" */
   char               network[RAYLOC_TRACE_NET_LEN];
   char               location[RAYLOC_TRACE_LOC_LEN];
   char               phase_name[9];
   double             residual;
   double             dist;
   double             az;
   char               weight_flag;
} RAYLOC_PICKS_STRUCT;




/* rayloc header rlc 01
**
**
**          1         2         3         4         5         6         7         8         9         0         1
** 12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
** RLC 001002003 vv xxxxxxxxxx.xxx +xx.xxxx +xxx.xxxx xxx.xx xxxx xxxx xxxx xxxx xxx xxx.xx l
** RLC author version event_id    Origin        elat     elon    depth nsta npha suse puse gap  dmin  f
**
** xxx.xx xxxx.x xxxx.x xxxx.x xxx.xx xxxx.x xxxx.x xxxx.x a
**  oterr laterr lonerr deperr   se    errh   errz    avh  q
**
** xxxx.x xxx +xx xxxx.x xxx +xx xxxx.x xxx +xx
**  axis1 az1 dp1  axis2 az2 dp2  axis3 az3 dp3
**
**  NOTE: These are not fixed-width fields, the counter is only to assist with in determining the maximum size
**
**
** Where:
**   Origin   Event origin time in epoch seconds
**   Elat     Geographical event latitude in signed decimal degrees (WGS84)
**   Elon     Geographical event longitude in signed decimal degrees (WGS84)
**   Depth    Event depth in kilometers (WGS84)
**   Gap      Largest azimuthal gap in degrees
**   Nsta     Number of stations associated
**   Npha     Number of phases associated
**   Suse     Number of stations used in the solution
**   Puse     Number of phases used in the solution
**   Dmin     Distance to the nearest station in degrees
**   F        Fixed depth flag (T if depth was held, F otherwise)*
**   Oterr    90% marginal confidence interval for origin time
**   Laterr   90% marginal confidence interval for latitude
**   Lonerr   90% marginal confidence interval for longitude
**   Deperr   90% marginal confidence interval for depth
**   Se       Standard error of the residuals in seconds
**   Errh     Maximum horizontal projection of the error ellipsoid in kilometers
**   Errz     Maximum vertical projection of the error ellipsoid in kilometers
**   Avh      Equivalent radius of the horizontal error ellipse in kilometers
**   Q        Quality flag (i.e., 'A', 'B', 'C', 'D')
**   Axis1-3  Length in kilometers of the principle axies of the error ellipsoid
**   Az1-3    Azimuth in degrees of the principle axies of the error ellipsoid
**   Dp1-3    Dip in degrees of the principle axies of the error ellipsoid
*
*/


#define RAYLOC_MAX_PICKS  500   /* max pcks in rayloc message */

#define RAYLOC_MESSAGE_HEADER_MAXBUFSIZE (50000 + BUFF_OVERRUN_SIZE)

typedef char RAYLOC_MESSAGE_HEADER_BUFFER[RAYLOC_MESSAGE_HEADER_MAXBUFSIZE+1];

typedef struct _RAYLOC_MESSAGE_HEADER_STRUCT
{
   unsigned short         version;  /* only set using InitGlobalAMP() or InitGlobalLoc() */
   MSG_LOGO               logo;
   long                   event_id;
   long                   origin_id; /* which edition of event_id contained herein */
   double                 origin_time;
   char                   origin_time_char[19];
   double                 elat;
   double                 elon;
   double                 edepth;
   double                 gap;
   int                    nsta;
   int                    npha;
   int                    suse;
   int                    puse;
   double                 dmin;
   char                   depth_flag;
   double                 oterr;
   double                 laterr;
   double                 lonerr;
   double                 deperr;
   double                 se;
   double                 errh;
   double                 errz;
   double                 avh;
   char                   q;
   double                 axis[3];
   double                 az[3];
   double                 dp[3];
   int                    numPicks;
   RAYLOC_PICKS_STRUCT    *picks[RAYLOC_MAX_PICKS];     
} RAYLOC_MESSAGE_HEADER_STRUCT;


/* ==========================================================================

**                    FUNCTION PROTOTYPES

** ========================================================================= */



/* rayloc_InitRaylocHeader
** rayloc_InitRaylocPicks
**
**   Used to initialize, including allocate working memory for
**   a rayloc message struct.
**
**
** PARAMETER:
**    p_struct -- pointer to structure to contain the message information
**
** RETURNS:
**    RAYLOC_MSG_SUCCESS
**    RAYLOC_MSG_NULL = structure pointer is NULL
*/
RAYLOC_MSG_STATUS rayloc_InitRaylocHeader( RAYLOC_MESSAGE_HEADER_STRUCT ** pp_rlc );
RAYLOC_MSG_STATUS rayloc_InitRaylocPicks( RAYLOC_PICKS_STRUCT ** pp_picks );


/* rayloc_WriteRaylocHeaderBuffer
** rayloc_WriteRaylocPicksBuffer
**
**    Writes a global message structure to a buffer.
**
** PARAMETERS:
**    p_struct -- the source structure
**    p_length -- the available buffer size
**    errNo -- pointer to error number (Check it if return string is NULL)
**  
** errNo possible values:
**    RAYLOC_MSG_SUCCESS
**    RAYLOC_MSG_NULL = structure pointer is NULL
**    RAYLOC_MSG_VERSINVALID = invalid message version
**    RAYLOC_MSG_BADPARAM = buffer pointer is NULL

**
** RETURNS:
**    Character buffer containing message for placing in the RING or NULL;
*/

char *
	rayloc_WriteRaylocHeaderBuffer( RAYLOC_MESSAGE_HEADER_STRUCT * p_struct, unsigned int *p_length, int *errNo);
char *
	rayloc_WriteRaylocPicksBuffer( RAYLOC_PICKS_STRUCT * p_struct, unsigned int *p_length, int *errNo);


/* rayloc_MessageToRaylocHeader
** rayloc_MessageToPicksBuffer
**
**
**   Parses rayloc message from a supplied string.

** PARAMETERS:
**    pp_struct - second pointer to the output structure;
**    p_message - message string;
**
**
** RETURNS:
**    RAYLOC_MSG_SUCCESS
**    RAYLOC_MSG_NULL = structure pointer is NULL
**    RAYLOC_MSG_VERSINVALID = invalid message version
**    RAYLOC_MSG_BADPARAM    = string pointer is NULL
**    RAYLOC_MSG_FORMATERROR = invalid string format
**    GLOBAL_MSG_MAXCHILDREN = too many children
**    GLOBAL_MSG_BADCHILD = invalid child line
*/
RAYLOC_MSG_STATUS
	rayloc_MessageToRaylocHeader( RAYLOC_MESSAGE_HEADER_STRUCT ** pp_struct, char * p_message);

RAYLOC_MSG_STATUS rayloc_MessageToPicksBuffer( RAYLOC_PICKS_STRUCT ** pp_struct, char * p_message );



/* rayloc_logRayloc
**
**
**   Logs the content of RAYLOC_MESSAGE_HEADER_STRUCT structure using LOGIT
*/
void
	rayloc_logRayloc(RAYLOC_MESSAGE_HEADER_STRUCT *pp_struct);


/*
** rayloc_fileToRaylocHeader
**
** Reads text file (output of Ray Bulland FORTRAN rayloc processing)
** and places it into RAYLOC_MESSAGE_HEADER_STRUCT structure
**
** PARAMETERS:
**    pp_struct - second pointer to the output structure
**    pathname - path name to rayloc file - output of FORTRAn processing
**    event_id - integer containing event_id
**    headerType -
**    headerMod
**    headerInst
**    picksType
**    picksMode
**    picksInst
**
** RETURNS:
**    RAYLOC_MSG_SUCCESS
**    RAYLOC_MSG_NULL = string pointer is NULL
**    RAYLOC_MSG_BADPARAM = string pointer is NULL in the low-level functions
**    RAYLOC_MSG_FORMATERROR = invalid string format
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
	                          unsigned char picksInst);


/*
** rayloc_FreeRaylocPicks
** rayloc_FreeRaylocHeader
**
** Deallocates and ZEROs RAYLOC_PICKS_STRUCT and RAYLOC_MESSAGE_HEADER_STRUCT
** correspondingly
**
** PARAMETERS:
** Second pointers to the above structures
**
** RETURNS:
**    void
*/

void rayloc_FreeRaylocPicks( RAYLOC_PICKS_STRUCT ** pp_struct );
void rayloc_FreeRaylocHeader( RAYLOC_MESSAGE_HEADER_STRUCT ** pp_struct );


/* ===============================================
 * FUNCTIONS BELOW ARE NOT PART OF RAYLOC_MSG API.
 * DO NOT USE THEM DIRECTLY
 *================================================ 
*/

/*
** rayloc_stringToRaylocHeader
** rayloc_stringeToPicks
** rayloc_grab_header_stringFromFile
** rayloc_grab_picks_stringFromFile
**
** Low-level service functions used by rayloc_fileToRaylocHeader()
** Do not directly call these functions 
** 
*/

RAYLOC_MSG_STATUS rayloc_stringToRaylocHeader( RAYLOC_MESSAGE_HEADER_STRUCT ** pp_struct,
	                     char * p_string,
	                     int event_id,
	                     unsigned char headerType,
	                     unsigned char headerMod,
	                     unsigned char headerInst);
	                     
RAYLOC_MSG_STATUS rayloc_stringeToPicks( RAYLOC_PICKS_STRUCT ** p_struct,
	                     char * p_string,
	                     unsigned char picksType,
	                     unsigned char picksMod,
	                     unsigned char picksInst);

char *rayloc_grab_header_stringFromFile(FILE *fd);
char *rayloc_grab_picks_stringFromFile(FILE *fd);


/*
** rayloc_ClearRayLocHeader
** rayloc_ClearRaylocPicks
**
** Those functions are implemented but not recommended;
** use rayloc_FreeRaylocPicks() and rayloc_FreeRaylocHeader() instead
*/

RAYLOC_MSG_STATUS rayloc_ClearRayLocHeader( RAYLOC_MESSAGE_HEADER_STRUCT * p_struct );
RAYLOC_MSG_STATUS rayloc_ClearRaylocPicks( RAYLOC_PICKS_STRUCT * p_struct );

/*
** Utulity finctions
*/
int rayloc_how_many_tokens(const char *myString);
char * rayloc_grab_header_stringFromFile(FILE *fd);
char * rayloc_grab_picks_stringFromFile(FILE *fd);
void rayloc_epoch17string( double secs, char *c17 );
	
#endif
/* EOF */

