/*
**  worm_msgtype.h
**
**  Hard-coded definitions of common worm message types that are sacred
**  (thus never-changing) in the [earth]worm system.
*
**  Generally, do not put any domain-specific types (i.e. seismology-
**  or geomag-specific) herein.   Should use a separate file for such.
*/
//---------------------------------------------------------------------------
#ifndef _WORM_MSG_TYPES_H
#define _WORM_MSG_TYPES_H
//---------------------------------------------------------------------------

#include <worm_types.h>  // WORM_MSGTYPE_ID

#define TYPE_WILDCARD (WORM_MSGTYPE_ID)0

#define TYPE_ERROR (WORM_MSGTYPE_ID)2

#define TYPE_HEARTBEAT (WORM_MSGTYPE_ID)3


#endif

 