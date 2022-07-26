/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: global_msg.h 1987 2005-08-15 18:35:58Z friberg $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2005/08/15 18:35:58  friberg
 *     Added in global_*.h files from the hydra release. These are used by
 *     some of the CERI modules.
 *
 *     Revision 1.1.1.1  2004/03/31 18:43:18  michelle
 *     New Hydra Import
 *
 *     Revision 1.2  2003/09/04 20:44:58  lucky
 *      Replaced AMPLITUDE_TYPE with MAGNITUDE_TYPE
 *
 *     Revision 1.1  2003/09/03 15:29:40  lucky
 *     Initial revision
 *
 *     Revision 1.5  2002/11/03 00:08:42  lombard
 *     Added earthworm RCS header
 *
 *
 *
 */
/*
**
*/
#ifndef _GLOBAL_MSG_H
#define _GLOBAL_MSG_H

#include <earthworm_defs.h> /* amp types */
#include <transport.h>      /*  MSG_LOGO  */

#define BUFF_OVERRUN_SIZE  20


typedef int GLOBAL_MSG_STATUS;

/*
** Ensure that error numbers are negative to avoid
** collision with some functions that return 0 - n for
** success
*/
enum GLOBAL_MSG_STATUS_TYPES
{
    GLOBAL_MSG_UNKNOWN      =   1 /* value is undetermined or not supplied (?)                    */
  , GLOBAL_MSG_SUCCESS      =   0 /* success                                                      */
  , GLOBAL_MSG_NULL         =  -1 /* item pointer is NULL                                         */
  , GLOBAL_MSG_VERSINVALID  =  -2 /* invalid version                                              */
  , GLOBAL_MSG_FORMATERROR  =  -3 /* invalid format (while parsing)                               */
  , GLOBAL_MSG_MAXCHILDREN  =  -4 /* too many children (applies to global location, phase)        */
  , GLOBAL_MSG_BADPARAM     =  -5 /* parameter error (e.g. string too long, index out of range)   */
  , GLOBAL_MSG_TOOSMALL     =  -6 /* [buffer] too small to contain message                        */
  , GLOBAL_MSG_BADCHILD     =  -7 /* invalid child line                                           */
  , GLOBAL_MSG_DEFINESMALL  =  -9 /* max buffer size too small for normal write                   */
  , GLOBAL_MSG_BADAMPTYPE   = -10 /* amp magtype not defined                                      */
};



/* ==========================================================================
**                    FUNCTION DECLARATIONS
** ========================================================================= */

/*
** DTStringToTime()
**
**  "YYYYMMDDHHMMss.sss" --> double (secs since 1/1/1970)
*/
double DTStringToTime( const char * p_datestring );

/*
** DTStringToTime()
**
**  double --> "YYYYMMDDHHMMss.sss"
**
**  p_buffer must be at least 19 chars long
**
**  RETURNS
**     GLOBAL_MSG_SUCCESS
**     GLOBAL_MSG_NULL -- p_buffer is null
*/
int TimeToDTString( const double p_time, char * p_buffer );


/* EncodeAuthor() -- encodes author from logo
**
** RETURNS
**     GLOBAL_MSG_SUCCESS
**     GLOBAL_MSG_NULLSTRUCT -- buffer pointer is NULL
*/
GLOBAL_MSG_STATUS EncodeAuthor( MSG_LOGO   p_logo
                              , char     * r_buffer
                              );

/* DecodeAuthor() -- decodes logo from author
**
** RETURNS
**     GLOBAL_MSG_SUCCESS
**     GLOBAL_MSG_NULLSTRUCT -- buffer pointer is NULL
**     GLOBAL_MSG_BADPARAM -- buffer string is invalid length
**     GLOBAL_MSG_FORMATERROR -- number conversion error
*/
GLOBAL_MSG_STATUS DecodeAuthor( MSG_LOGO * p_logo
                              , char     * r_buffer
                              );


/* GetGlobalAmpTypeName
**
**   A little helper function to obtain the character string
**   for the specified type code
*/
const char * GetGlobalAmpTypeName( MAGNITUDE_TYPE p_type );


#endif
