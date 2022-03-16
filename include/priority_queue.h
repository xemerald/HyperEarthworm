/*   priority_queue.h
**
**   These are the defines and prototypes for a priority-sorted,
**   stack-based queue of messages.
**
**   CAUTION: RETURN CODES DIFFER SLIGHTLY FROM MEM_CIR_QUEUE AND
**            SOME NON-ZERO RETURN CODES ARE NORMAL (see defs, below)
**
**   CAUTION: THIS QUEUE TYPE MANAGES ITS OWN MUTEX FOR ACCESS CONTROL
**            THERE IS NO NEED FOR THE CALLER TO DUPLICATE SUCH.
**
**   20020318 dbh - created
*/

#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <priority_level.h>  /* for MSG_LOGO */
#include <transport.h>       /* for MSG_LOGO */

/*
** RETURN CODES
*/
#define EW_PRI_NOITEM       3 /* no item found in queue */
#define EW_PRI_RETREJECT    2 /* no space for this priority */
#define EW_PRI_RETDROP      1 /* earlier item dropped for space */
#define EW_PRI_RETNORMAL    0 /* normal */
#define EW_PRI_RETNOTREADY -1 /* queue not ready */
#define EW_PRI_RETMSGSIZE  -2 /* message too large (or < 0) */
#define EW_PRI_RETQNULL    -3 /* PRI_QUEUE pointer is NULL */
#define EW_PRI_RETMALLOC   -4 /* memory allocation problem */
#define EW_PRI_RETBADPRI   -5 /* invalid priority */
#define EW_PRI_RETPARAM    -6 /* invalid parameter value */

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


typedef short EW_PRIORITY;


typedef char*  PRI_DATA;


typedef struct {
   EW_PRIORITY    pri;
   MSG_LOGO       logo;
   long           length;
   PRI_DATA       data;
} PRI_QUEUE_ENTRY;

typedef struct
{
   int                queuesize;
   int                itemsused;
   long               itemmaxsize;
   int insert_indices[EW_PRIORITY_COUNT]; /* sort management */
   PRI_QUEUE_ENTRY ** sorted;  /* sorted pointers to objects */
   PRI_QUEUE_ENTRY *  entries; /* queue entries */
   char            *  data;    /* data storage */
   mutex_t            lock;
} PRI_QUEUE;


/*********************************************************
** init_pri_queue( PRI_QUEUE * p_queue
**               , unsigned long p_max_items
**               , unsigned long p_max_item_size )
**
** Allocates and initializes all arrays and grabs a mutex
** for access control.
**
** parameters
**    p_max_items:     the maximum number of objects stored
**                     prior to dropping lower priorities.
**    p_max_item_size: size to use for storage of items.
**
** returns: EW_PRI_RETNORMAL
**			   EW_PRI_RETQNULL
**          EW_PRI_RETPARAM  (p_max_items < 1)
**          EW_PRI_RETMALLOC
**********************************************************/
int init_pri_queue( PRI_QUEUE * p_queue
                  , unsigned long p_max_items
                  , unsigned long p_max_item_size );

/*********************************************************
** release_pri_stack() releases allocated memory
**********************************************************/
void release_pri_queue( PRI_QUEUE * p_queue );

/*********************************************************
** getNumOfElementsInQueue() returns number of items in queue
**********************************************************/
int getNumOfElementsInQueue( PRI_QUEUE * p_queue );

/*********************************************************
** add_item( PRI_QUEUE * p_queue
**         , EW_PRIORITY p_priority
**         , MSG_LOGO    p_logo
**         , long        p_size
**         , DATA        p_data )
**
** Adds an object to the stack in priority order
** (The object is dropped if insufficient space available,
** item of lesser priority or of same priority but earlier
** may also be dropped to make space).
**
** parameters
**    p_queue:     PRI_QUEUE to act upon
**    p_priority:  Priority of the object
**    p_logo:      Message logo
**    p_size:      Length of message data
**    p_data:      Message data
**
** returns:  EW_PRI_RETREJECT
**           EW_PRI_RETDROP
**           EW_PRI_RETNORMAL
**           EW_PRI_RETNOTREADY
**           EW_PRI_RETMSGSIZE
**           EW_PRI_RETQNULL
**           EW_PRI_RETBADPRI
**********************************************************/
int add_item( PRI_QUEUE * p_queue
            , EW_PRIORITY p_priority
            , MSG_LOGO    p_logo
            , long        p_size
            , PRI_DATA    p_data
            );

/*********************************************************
** peek_next_item( PRI_QUEUE   * p_queue
**               , MSG_LOGO    * p_logoptr  )
**               , EW_PRIORITY * p_priptr  )
**
** Gets the highest available object currently stored on
** the stack.
**
** parameters
**    p_queue:     PRI_QUEUE to act upon
**    p_logo:      Pointer to Message logo to fill
**    p_priptr:    Pointer to EQ_PRIORITY to fill
**
** returns:  EW_PRI_RETNORMAL (data ready)
**           EW_PRI_RETNOTREADY
**           EW_PRI_NOITEM    (no item was in queue)
**           EW_PRI_RETQNULL
*********************************************************/
int peek_next_item( PRI_QUEUE   * p_queue
                  , MSG_LOGO    * p_logoptr
                  , EW_PRIORITY * p_priptr
                  );

/*********************************************************
** pop_next_item( PRI_QUEUE * p_queue
**              , MSG_LOGO  * p_logoptr
**              , long      * p_sizeptr
**              , DATA        p_data    )
**
** Gets the highest available object currently stored on
** the stack.
**
** parameters
**    p_queue:     PRI_QUEUE to act upon
**    p_logo:      Pointer to Message logo to fill
**    p_size:      Pointer to long to fill with message length
**    p_data:      Message data
**
** returns:  EW_PRI_RETNORMAL (data ready)
**           EW_PRI_RETNOTREADY
**           EW_PRI_NOITEM    (no item was in queue)
**           EW_PRI_RETQNULL
*********************************************************/
int pop_next_item( PRI_QUEUE * p_queue
                 , MSG_LOGO  * p_logoptr
                 , long      * p_sizeptr
                 , PRI_DATA    p_data
                 );


#endif
