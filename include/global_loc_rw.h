/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: global_loc_rw.h 1987 2005-08-15 18:35:58Z friberg $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2005/08/15 18:35:58  friberg
 *     Added in global_*.h files from the hydra release. These are used by
 *     some of the CERI modules.
 *
 *     Revision 1.3  2005/02/15 18:57:50  davidk
 *     Changed string lengths from TRACE_*_LEN to TRACE2_*_LEN
 *     to conform to the SCNL based EW Tracebuf2 standard.
 *
 *     Revision 1.2  2004/08/06 01:11:55  davidk
 *     Changed pick and origin times from strings to doubles(secs since 1970)
 *     in the global_loc structs.
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
 *     Revision 1.7  2002/11/17 23:00:07  lombard
 *     Increase GLOBAL_LOC_MAXPHS to a reasonable limit
 *
 *     Revision 1.6  2002/11/03 00:08:29  lombard
 *     Added earthworm RCS header
 *
 *
 *
 */

#ifndef _GLOBAL_LOC_H
#define _GLOBAL_LOC_H
/*---------------------------------------------------------------------------*/

#include <earthworm_defs.h> /* amp types */
#include <trace_buf.h> /*  TRACE_STA_LEN, TRACE_CHAN_LEN, TRACE_NET_LEN, TRACE_LOC_LEN */
#include <global_msg.h>

/*---------------------------------------------------------------------------*/
/* ==========================================================================
**                    SIZE DEFINES AND LIMITS
** ========================================================================= */

/* Global Amp 01  SUPERCEDED
**
** AMP <author> <sequence_number> <version> <station> <comp> <net> <loc> <pick_time> <amplitude_type> <amplitude> <associated_period>
**
** AMP 001002003 sssssssssssssss vv ssssss cccccccc nnnnnnnn llllllll yyyymmddhhmmss.sss T aaaaaaaaaaaaa.aa pppppppp.ppppppp
**
** Global Amp 02  SUPERCEDED
**
** AMP <author> <version> <station> <comp> <net> <loc> <amplitude_type> <amplitude> <associated_period>
**
** AMP 001002003 vv ssssss cccccccc nnnnnnnn llllllll T aaaaaaaaaaaaa.aa pppppppp.ppppppp
**
** Global Amp 03  SUPERCEDED
**
** AMP <author> <pick_sequence> <version> <station> <comp> <net> <loc> <amplitude_type> <amplitude> <associated_period>
**
** AMP 001002003 sssssssssssssss vv ssssss cccccccc nnnnnnnn llllllll T aaaaaaaaaaaaa.aa pppppppp.ppppppp
**
** Global Amp 04
**
** AMP <author> <pick_sequence> <version> <station> <comp> <net> <loc> <amp_time> <amplitude_type> <amplitude> <associated_period>
**
** AMP 001002003 sssssssssssssss vv ssssss cccccccc nnnnnnnn llllllll yyyymmddhhmmss.sss T aaaaaaaaaaaaa.aa pppppppp.ppppppp
**                                                                                                                          \n
**                                                                                                                           \0
**           1         2         3         4         5         6         7         8         9         0         1         2
** 012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012
**
**  NOTE: These are not fixed-width fields, the counter is only to assist with in determining the maximum size
*/

#define GLOBAL_AMPLINE_MAXBUFSIZE (122 + BUFF_OVERRUN_SIZE)

typedef char GLOBAL_AMPLINE_BUFFER[GLOBAL_AMPLINE_MAXBUFSIZE+1];


typedef struct _GLOBAL_AMPLINE_STRUCT
{
   unsigned short     version;  /* only set using InitGlobalAMP() or InitGlobalLoc() */
   MSG_LOGO           logo;
   long               pick_sequence;
   char               station[TRACE2_STA_LEN];
   char               channel[TRACE2_CHAN_LEN]; /* aka "component" */
   char               network[TRACE2_NET_LEN];
   char               location[TRACE2_LOC_LEN];
   char               amp_time[19];
   MAGNITUDE_TYPE     amptype;
   double             adcounts; /* a/d counts */
   double             period;   /* seconds    */
} GLOBAL_AMPLINE_STRUCT;



/* Global Phase 01
**
** PHS <author> <sequence_number> <version> <station> <comp> <net> <loc> <pick_time> <phase_name> <quality> <polarity> <namp>
**
** PHS 001002003 sssssssssssssss vv ssssss cccccccc nnnnnnnn llllllll nnnnnnnn yyyymmddhhmmss.sss qqqqqqqqqqqqq.qq p 10000
**                                                                                                     1         1
**           1         2         3         4         5         6         7         8         9         0         1
** 012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
**
**  NOTE: These are not fixed-width fields, the counter is only to assist in determining the maximum size
*/

#define MAX_AMPS_PER_GLOBALPHASE  (MAGTYPE_COUNT - 1)

#define GLOBAL_PHSLINE_SUM_MAXSIZE (119+ BUFF_OVERRUN_SIZE)

typedef char GLOBAL_PHSLINE_SUM_BUFFER[GLOBAL_PHSLINE_SUM_MAXSIZE+1];

#define GLOBAL_PHSLINE_MAXBUFSIZE (GLOBAL_PHSLINE_SUM_MAXSIZE + (MAX_AMPS_PER_GLOBALPHASE * GLOBAL_AMPLINE_MAXBUFSIZE) )

typedef char GLOBAL_PHSLINE_BUFFER[GLOBAL_PHSLINE_MAXBUFSIZE+1];

typedef struct _GLOBAL_PHSLINE_STRUCT
{
   unsigned short         version;  /* only set using InitGlobalPhase() or InitGlobalLoc() */
   MSG_LOGO               logo;
   long                   sequence;
   char                   station[TRACE2_STA_LEN];
   char                   channel[TRACE2_CHAN_LEN]; /* aka "component" */
   char                   network[TRACE2_NET_LEN];
   char                   location[TRACE2_LOC_LEN];
   double                 tPhase;
   char                   phase_name[9];
   double                 quality;
   char                   polarity;
   GLOBAL_AMPLINE_STRUCT  amps[MAX_AMPS_PER_GLOBALPHASE]; /* -1 because unknown/invalid not used */
} GLOBAL_PHSLINE_STRUCT;





/* Global Location 02 (SUM line)
**
** SUM <author> <version> <event_id> <origin_id> <origin_time> <lat> <lon> <z> <gap> <dmin> <rms> <id> <pick_count> <nph>
**
** SUM 001002003 vvvvv eeeeeeeeeeeee ooooo YYYYMMDDhhmmss.sss -90.0000 -180.0000 -000.000 000.00 000.00 00.000 100000000000 10000
**
**           1         2         3         4         5         6         7         8         9         0         1         2         3
** 01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123
**
**  NOTE: These are not fixed-width fields, the counter is only to assist in determining the maximum size
*/

#define GLOBAL_LOC_SUM_MAXSIZE (127 + BUFF_OVERRUN_SIZE)

typedef char GLOBAL_LOC_SUM_BUFFER[GLOBAL_LOC_SUM_MAXSIZE+1];


#define GLOBAL_LOC_MAXPHS  500   /* max phases in global loc msg     */

#define GLOBAL_LOC_MAXBUFSIZE (GLOBAL_LOC_SUM_MAXSIZE + (GLOBAL_LOC_MAXPHS * GLOBAL_PHSLINE_MAXBUFSIZE) )

typedef char GLOBAL_LOC_BUFFER[GLOBAL_LOC_MAXBUFSIZE+1];


typedef struct GLOBAL_LOC_DATA
{
   unsigned short         version; /* only set using InitGlobalLoc */
   MSG_LOGO               logo;
   long                   event_id;
   long                   origin_id; /* which edition of event_id contained herein */
   double                 tOrigin;
   double                 lat;
   double                 lon;
   float                  depth;
   float                  gap;
   float                  dmin;
   float                  rms;
   short                  pick_count; /* number of picks used to calculate origin (reported by glass) */
   short                  nphs;       /* number of valid phase structures included */
   GLOBAL_PHSLINE_STRUCT  phases[GLOBAL_LOC_MAXPHS];
} GLOBAL_LOC_STRUCT;









/* ==========================================================================

**                    FUNCTION PROTOTYPES

** ========================================================================= */



/* InitGlobalAmp
** InitGlobalPhase
** InitGlobalLoc
**
**   Used to initialize, including allocate working memory for
**   a global message struct.
**
**
** PARAMETER:
**    p_struct -- pointer to structure to contain the message information
**
** RETURNS:
**    GLOBAL_MSG_SUCCESS
**    GLOBAL_MSG_NULL = structure pointer is NULL
*/
GLOBAL_MSG_STATUS InitGlobalAmpLine( GLOBAL_AMPLINE_STRUCT * p_struct );
GLOBAL_MSG_STATUS InitGlobalPhaseLine( GLOBAL_PHSLINE_STRUCT * p_struct );
GLOBAL_MSG_STATUS InitGlobalLoc( GLOBAL_LOC_STRUCT * p_struct );




/* WriteAmpToBuffer
** WritePhaseToBuffer
** WriteLocToBuffer
**
**    Writes a global message structure to a buffer.
**
** PARAMETERS:
**    p_struct -- the source structure
**    p_buffer -- the target buffer
**                (on successful return, buffer will be null-terminated)
**    p_length -- the available buffer size
**
** RETURNS:
**    GLOBAL_MSG_SUCCESS
**    GLOBAL_MSG_NULL = structure pointer is NULL
**    GLOBAL_MSG_VERSINVALID = invalid message version
**    GLOBAL_MSG_BADPARAM = buffer pointer is NULL
**    GLOBAL_MSG_TOOSMALL = buffer not large enough to contain message
**                          (may be partly written anyway)
**    GLOBAL_MSG_DEFINESMALL = max buffer size too small for normal write
*/
GLOBAL_MSG_STATUS WriteAmpLineToBuffer( GLOBAL_AMPLINE_STRUCT * p_struct, char * p_buffer, unsigned int p_length );
GLOBAL_MSG_STATUS WritePhaseLineToBuffer( GLOBAL_PHSLINE_STRUCT * p_struct, char * p_buffer, unsigned int p_length );
GLOBAL_MSG_STATUS WriteLocToBuffer( GLOBAL_LOC_STRUCT * p_struct, char * p_buffer, unsigned int p_length );



/* StringToAmp
** StringToPhase
** StringToLoc
**
**   Parses any global message from a supplied string.
**
**   Do not include the "PHS " or "AMP " line id strings.
**
** RETURNS:
**    GLOBAL_MSG_SUCCESS
**    GLOBAL_MSG_NULL = structure pointer is NULL
**    GLOBAL_MSG_VERSINVALID = invalid message version
**    GLOBAL_MSG_BADPARAM    = string pointer is NULL
**    GLOBAL_MSG_FORMATERROR = invalid string format
**    GLOBAL_MSG_MAXCHILDREN = too many children [StringToLoc() only]
**    GLOBAL_MSG_BADCHILD = invalid child line [StringToLoc() only]
*/
GLOBAL_MSG_STATUS StringToAmpLine( GLOBAL_AMPLINE_STRUCT * p_amp, char * p_string );
GLOBAL_MSG_STATUS StringToPhaseLine( GLOBAL_PHSLINE_STRUCT * p_phase, char * p_string );
GLOBAL_MSG_STATUS StringToLoc( GLOBAL_LOC_STRUCT * p_loc, char * p_string );


/* AddAmpToPhase
** AddPhaseToLoc
**
**    Add an Amp to a Phase, or a Phase to a Location
**
** RETURNS
**    number 0 - n = index of successfully added item
**    GLOBAL_MSG_NULL = location structure pointer is NULL
**    GLOBAL_MSG_VERSINVALID = invalid message version (location)
**    GLOBAL_MSG_BADPARAM = child pointer is NULL, or version invalid
**    GLOBAL_MSG_MAXCHILDREN = too many children (of this type)
*/
int AddAmpLineToPhase( GLOBAL_PHSLINE_STRUCT * p_phase
                     , GLOBAL_AMPLINE_STRUCT * p_amp
                     );
int AddPhaseLineToLoc( GLOBAL_LOC_STRUCT     * p_loc
                     , GLOBAL_PHSLINE_STRUCT * p_phase
                     );



/* GetLocPhaseIndex
**
**    Find the location index of an Amp or Phase or Amp in another structure
**
** RETURNS
**   p_loc.nphs = item does not exist in the location
**   other = index of item in the nphs[] or namp[] array
*/
int GetLocPhaseIndex( GLOBAL_LOC_STRUCT     * p_loc
                    , GLOBAL_PHSLINE_STRUCT * p_phase
                    );

/* ClearAmps
** ClearPhases
**
**    Used to clear work structure of children's data
**
** RETURNS
**    GLOBAL_MSG_SUCCESS
**    GLOBAL_MSG_NULL = location structure pointer is NULL
**    GLOBAL_MSG_VERSINVALID = invalid message version
*/
GLOBAL_MSG_STATUS ClearAmpLines( GLOBAL_PHSLINE_STRUCT * p_phase );
GLOBAL_MSG_STATUS ClearPhaseLines( GLOBAL_LOC_STRUCT * p_loc );

#endif



