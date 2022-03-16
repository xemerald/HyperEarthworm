/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: global_pick_rw.h 1987 2005-08-15 18:35:58Z friberg $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2005/08/15 18:35:58  friberg
 *     Added in global_*.h files from the hydra release. These are used by
 *     some of the CERI modules.
 *
 *     Revision 1.2  2005/02/15 18:59:06  davidk
 *     Changed string lengths from TRACE_*_LEN to TRACE2_*_LEN
 *     to conform to the SCNL based EW Tracebuf2 standard.
 *
 *     Revision 1.1.1.1  2004/03/31 18:43:18  michelle
 *     New Hydra Import
 *
 *     Revision 1.1  2003/09/03 15:29:40  lucky
 *     Initial revision
 *
 *     Revision 1.4  2002/11/03 00:09:01  lombard
 *     Added earthworm RCS header
 *
 *
 *
 */

#ifndef _GLOBAL_PICK_H
#define _GLOBAL_PICK_H
/*---------------------------------------------------------------------------*/

#include <global_msg.h>
#include <trace_buf.h> /*  TRACE_STA_LEN, TRACE_CHAN_LEN, TRACE_NET_LEN, TRACE_LOC_LEN */

/*---------------------------------------------------------------------------*/



/* ==========================================================================
**                    SIZE DEFINES AND LIMITS
** ========================================================================= */

/* Global Pick 01
**
** <author> <sequence_number> <version> <station> <comp> <net> <loc> <pick_time> <phase_name> <quality> <polarity>
**
** 001002003 sssssssssssssss vv ssssss cccccccc nnnnnnnn llllllll yyyymmddhhmmss.sss nnnnnnnn qqqqqqqqqqqqq.qq p
**                                                                                                     1   
**           1         2         3         4         5         6         7         8         9         0
** 01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
*/

#define GLOBAL_PICK_MAXBUFSIZE (108 + BUFF_OVERRUN_SIZE)

typedef char GLOBAL_PICK_BUFFER[GLOBAL_PICK_MAXBUFSIZE+1];


typedef struct GLOBAL_PICK_MSG
{
   short              version;  /* only set using InitGlobalPick() or StringToPick() */
   MSG_LOGO           logo;
   long               sequence;
   char               station[TRACE2_STA_LEN];  /* aka "site"      */
   char               channel[TRACE2_CHAN_LEN]; /* aka "component" */
   char               network[TRACE2_NET_LEN];
   char               location[TRACE2_LOC_LEN];
   char               pick_time[19];
   char               phase_name[9];
   double             quality;
   char               polarity;
} GLOBAL_PICK_STRUCT;




/* ==========================================================================
**                    FUNCTION PROTOTYPES
** ========================================================================= */

/* InitGlobalPick
**
**   Used to initialize, including allocate working memory for
**   a global pick struct.
**
**
** PARAMETER:
**    p_struct -- pointer to structure to contain the message information
**
** RETURNS:
**    GLOBAL_MSG_SUCCESS
**    GLOBAL_MSG_NULLSTRUCT = structure pointer is NULL
*/
GLOBAL_MSG_STATUS InitGlobalPick( GLOBAL_PICK_STRUCT * p_struct );




/* WritePickToBuffer
**
**    Writes a global pick structure to a buffer.
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
GLOBAL_MSG_STATUS WritePickToBuffer( GLOBAL_PICK_STRUCT * p_struct, char * p_buffer, unsigned int p_length );



/* StringToPick
**
**   Parses a global pick message from a supplied string.
**
** RETURNS:
**    GLOBAL_MSG_SUCCESS
**    GLOBAL_MSG_NULLSTRUCT = structure pointer is NULL
**    GLOBAL_MSG_VERSINVALID = invalid message version
**    GLOBAL_MSG_BADPARAM    = string pointer is NULL
**    GLOBAL_MSG_FORMATERROR = invalid string format
**
*/
GLOBAL_MSG_STATUS StringToPick( GLOBAL_PICK_STRUCT * p_pick, char * p_string );



#endif

