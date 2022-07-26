/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: priority_level.h 1023 2002-07-19 22:29:42Z dietz $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.2  2002/07/19 22:29:42  dietz
 *     Contains priority level definitions previously in priority_queue.h
 *
 *
 */

/*   priority_level.h
**
**   Definitions of Priority level used in priority_queue.c.
**   Pulled from priority_queue.h by LDD so the priority
**   definitions could be used by mem_circ_queue programs too.
**
**   created 2002/07/19 LDD
*/

#ifndef PRIORITY_LEVEL_H
#define PRIORITY_LEVEL_H

/*
** PRIORITY LEVELS
**
** CAUTION: Note that the highest priority (most important)
**          items are assigned EW_PRIORITY_MIN.
**          That is, these defines are for the programming
**          domain, not the work domain.
*/
#define EW_PRIORITY_NONE   0 /* state when no message present */
#define EW_PRIORITY_MIN    1 /* the highest priority */
#define EW_PRIORITY_MAX    9 /* the lowest priority */
#define EW_PRIORITY_COUNT 10 /* simplifies loops */

#define EW_PRIORITY_DEF    9 /* default priority => lowest work priority */

#endif
