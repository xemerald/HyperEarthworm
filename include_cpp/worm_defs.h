#ifndef _WORM_DEFS_H
#define _WORM_DEFS_H

/*
** PRIORITY LEVELS
**
** CAUTION: Note that the highest priority (most important)
**          items are assigned WORM_PRIORITY_MIN.
**          That is, these defines are for the programming
**          domain, not the work domain.
*/
#define WORM_PRIORITY_NONE    0 /* state when no message present */
#define WORM_PRIORITY_COMMAND 1 /* priority for command messages */
#define WORM_PRIORITY_MIN     1 /* the highest priority */
#define WORM_PRIORITY_MAX     9 /* the lowest priority */
#define WORM_PRIORITY_COUNT  10 /* simplifies loops, DO NOT USE AS A PRIORITY */

#define WORM_PRIORITY_DEF     9 /* default priority => lowest work priority */

typedef short WORM_PRIORITY;


enum WORM_LOGGING_LEVEL
{
    WORM_LOG_MU       = -1 // mu is a Japanese term "not relevant to this domain"
                           // this is used for initialization, thus this value
                           // should never be used, especially never in the
                           // configuration (.d) files.
  , WORM_LOG_NONE     =  0
  , WORM_LOG_ERRORS   =  1
  , WORM_LOG_STATUS   =  2
  , WORM_LOG_TRACKING =  3
  , WORM_LOG_DETAILS  =  4
  , WORM_LOG_DEBUG    =  5
};



#if   defined(_Windows)
#define PathDelim "\\"
#elif defined(_Solaris)
#define PathDelim "/"
#else
#error worm_defs.h not completed for this O/S
#endif


#endif
