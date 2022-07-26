/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: global_amp_rw.h 1987 2005-08-15 18:35:58Z friberg $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2005/08/15 18:35:58  friberg
 *     Added in global_*.h files from the hydra release. These are used by
 *     some of the CERI modules.
 *
 *     Revision 1.2  2004/04/09 22:59:25  davidk
 *     Added corrected AMPTYPE_xxx definitions.
 *     (This should be the only place where AMPTYPE_xxx constants
 *     are now defined.)
 *
 *     Revision 1.1.1.1  2004/03/31 18:43:18  michelle
 *     New Hydra Import
 *
 *     Revision 1.2  2003/09/04 20:59:40  lucky
 *     Replaced AMPLITUDE_TYPE with MAGNITUDE_TYPE
 *
 *     Revision 1.1  2003/09/03 15:29:40  lucky
 *     Initial revision
 *
 *     Revision 1.4  2002/11/03 00:08:04  lombard
 *     Added earthworm RCS header
 *
 *
 *
 */

#ifndef _GLOBAL_AMP_H
#define _GLOBAL_AMP_H
/*---------------------------------------------------------------------------*/


#include <trace_buf.h>      /*  TRACE_STA_LEN, TRACE_CHAN_LEN, TRACE_NET_LEN, TRACE_LOC_LEN */
#include <earthworm_defs.h> /* for amp types */
#include <global_msg.h>     /* for return codes */


/*---------------------------------------------------------------------------*/


/* ==========================================================================
**                    SIZE DEFINES AND LIMITS
** ========================================================================= */

/* Global Amp 01
**
** <author> <sequence_number> <version> <station> <comp> <net> <loc> <pick_time> <magnitude_type> <amplitude> <associated_period>
**
** 001002003 sssssssssssssss vv ssssss cccccccc nnnnnnnn llllllll yyyymmddhhmmss.sss T aaaaaaaaaaaaa.aa pppppppppppp.ppp
**                                                                                                                      \n
**                                                                                                                       \0
**           1         2         3         4         5         6         7         8         9         0         1         2
** 01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901
*/

#define GLOBAL_AMP_MAXBUFSIZE (118 + BUFF_OVERRUN_SIZE)

typedef char GLOBAL_AMP_BUFFER[GLOBAL_AMP_MAXBUFSIZE+1];

/* RAY PICKER'S AMPLITUDE DEFINITIONS */

#define AMPTYPE_NONE  0 /* invalid or unknown ('?') */
#define AMPTYPE_MB    1
#define AMPTYPE_ML    2
#define AMPTYPE_MBLG  3



typedef struct GLOBAL_AMP_MSG
{
   short              version;  /* only set using InitGlobalAmp() or StringToAmp() */
   MSG_LOGO           logo;
   long               sequence;       /* pick sequence number */
   char               station[TRACE_STA_LEN];
   char               channel[TRACE_CHAN_LEN];
   char               network[TRACE_NET_LEN];
   char               location[TRACE_LOC_LEN];
   char               amp_time[19];
   MAGNITUDE_TYPE     amptype;
   double             adcounts; /* a/d counts */
   double             period;   /* seconds    */
} GLOBAL_AMP_STRUCT;



/* ==========================================================================
**                    FUNCTION PROTOTYPES
** ========================================================================= */


/* InitGlobalAmp
**
**   Used to initialize, including allocate working memory for
**   a global amp message struct.
**
** PARAMETER:
**    p_struct -- pointer to structure to contain the message information
**
** RETURNS:
**    GLOBAL_MSG_SUCCESS
**    GLOBAL_MSG_NULLSTRUCT = structure pointer is NULL
*/
GLOBAL_MSG_STATUS InitGlobalAmp( GLOBAL_AMP_STRUCT * p_struct );




/* WriteAmpToBuffer
**
**    Writes a global amp message structure to a buffer.
**
** PARAMETERS:
**    p_struct -- the source structure
**    p_buffer -- the target buffer
**                (on successful return, buffer will be null-terminated)
**    p_length -- the available buffer size
**
** RETURNS:
**    GLOBAL_MSG_SUCCESS
**    GLOBAL_MSG_NULLSTRUCT = structure pointer is NULL
**    GLOBAL_MSG_VERSINVALID = invalid message version
**    GLOBAL_MSG_BADPARAM = buffer pointer is NULL
**    GLOBAL_MSG_TOOSMALL = buffer not large enough to contain message
**                          (may be partly written anyway)
**    GLOBAL_MSG_DEFINESMALL = max buffer size too small for normal write
*/
GLOBAL_MSG_STATUS WriteAmpToBuffer( GLOBAL_AMP_STRUCT * p_struct, char * p_buffer, unsigned int p_length );



/* StringToAmp
**
**   Parses any global amp message from a supplied string.
**
** RETURNS:
**    GLOBAL_MSG_SUCCESS
**    GLOBAL_MSG_NULLSTRUCT = structure pointer is NULL
**    GLOBAL_MSG_VERSINVALID = invalid message version
**    GLOBAL_MSG_BADPARAM    = string pointer is NULL
**    GLOBAL_MSG_FORMATERROR = invalid string format
**
*/
GLOBAL_MSG_STATUS StringToAmp( GLOBAL_AMP_STRUCT * p_struct, char * p_string );


#endif

