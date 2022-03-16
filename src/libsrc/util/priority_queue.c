/*
** priority_queue.c
**
** This implements a priority transfer queue.
**
** Priority levels are defined in the .h file.
**
** The actual sorting is of the pointers in array queue->sorted.
** Bear in mind these are pointers to the PRI_QUEUE elements in
** queue->objects.
** 
** The queue must first be initialized to a specified size
** using the init_pri_queue function.  After that, the queue
** is ready for use through add_item() and pop_next_item().
** (pop_next_item always returns the next item of the highest
** priority).
**
** SEE priority_queue.h for FUNCTION DESCRIPTIONS and RETURN CODES.
**
** CAUTION: This queue manages its own mutex, and thus the function
**          calls do not need to be wrapped in such by the caller.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "earthworm.h"
#include "earthworm_complex_funcs.h"
#include "priority_queue.h"

/* #define LOG_DEBUG 1  */

#ifdef LOG_DEBUG
/* #define DEBUG_DETAILS 1 */
#include <stdio.h>
#endif

/* Private definitions used to simplify the code by keeping the array
** shifting in a single location.
*/
#define EW_PRI_SHIFT_DOWN -1
#define EW_PRI_SHIFT_NONE  0
#define EW_PRI_SHIFT_UP    1


/*********************************************************
**
**********************************************************/
int init_pri_queue( PRI_QUEUE   * p_queue
                  , unsigned long p_max_items
                  , unsigned long p_max_item_size )
{
   int r_status = EW_PRI_RETNORMAL;

   if ( p_queue == NULL )
   {
      r_status = EW_PRI_RETQNULL;   
   }
   else
   {
      /*
      ** Initialize structure in case deallocation needed later
      */
      p_queue->queuesize = 0;
      p_queue->itemsused = 0;
      p_queue->sorted    = NULL;
      p_queue->entries   = NULL;
      p_queue->data      = NULL;
   }

   if ( p_max_items < 1 )
   {
      r_status = EW_PRI_RETPARAM;
   }

   if ( r_status == 0 )
   {
      unsigned long _idx;

      p_queue->queuesize = p_max_items;
      p_queue->itemmaxsize = p_max_item_size;

      /*
      ** Allocate space for the sorted pointers to the object storage
      */
      if ( r_status == EW_PRI_RETNORMAL )
      {
         if ( ( p_queue->sorted = (PRI_QUEUE_ENTRY **) malloc( sizeof(PRI_QUEUE_ENTRY*) * p_max_items) ) ==  NULL )
         {
            r_status = EW_PRI_RETMALLOC;
         }
      }

      /*
      ** Allocate space for the object storage
      */
      if ( r_status == EW_PRI_RETNORMAL )
      {
         if ( ( p_queue->entries = (PRI_QUEUE_ENTRY *) malloc( sizeof(PRI_QUEUE_ENTRY) * p_max_items) ) ==  NULL )
         {
            r_status = EW_PRI_RETMALLOC;
         }
      }


      /*
      ** Allocate space for the object storage
      */
      if ( r_status == EW_PRI_RETNORMAL )
      {
         if ( ( p_queue->data = (char *) malloc(sizeof(char) * p_max_item_size * p_max_items) ) ==  NULL )
         {
            r_status = EW_PRI_RETMALLOC;
         }
      }
      

      /*
      ** Initialize the priority containers,
      ** Load the container pointers into the sorted list
      */
      if ( r_status == EW_PRI_RETNORMAL )
      {
         PRI_QUEUE_ENTRY  * _entry  = p_queue->entries;
         PRI_QUEUE_ENTRY ** _sorted = p_queue->sorted;
         for ( _idx = 0 ; _idx < p_max_items ; _idx++, _entry++, _sorted++ ) 
         {
            /* set priority to indicate no valid data */
            _entry->pri = EW_PRIORITY_NONE;
            _entry->length = 0;
            /* assign data storage location to queue entry */
            _entry->data = (char *)(p_queue->data + (_idx * p_max_item_size));
            /* grab storage pointer for sorted array */
            *_sorted = _entry;
         }
      }


      /*
      ** Initialize priority index locations to point to the first (zero)
      ** location in the sorted array
      */
      if ( r_status == EW_PRI_RETNORMAL )
      {
         for ( _idx = 0 ; _idx < EW_PRIORITY_COUNT ; _idx++ ) {
            p_queue->insert_indices[_idx] = 0;
         }
      }
      CreateSpecificMutex( &(p_queue->lock) );
   }


   if ( r_status != EW_PRI_RETNORMAL )
   {
      release_pri_queue( p_queue );
   }
#ifdef LOG_DEBUG
   else
   {
      char dbgstr[120];
            sprintf( dbgstr
                   , "DEBUG init: %d  %d  %d\n"
                   , p_queue->sorted, p_queue->entries, p_queue->data
                   );
            logit( "t" 
                 , dbgstr
                 , "export_pri"
                 , "MOD_EXPORT_SCN"
                 );
   }
#endif
   return r_status;
}


/*********************************************************
**
**********************************************************/
void release_pri_queue( PRI_QUEUE * p_queue )
{
   p_queue->queuesize = 0;
   p_queue->itemsused = 0;

   CloseSpecificMutex( &p_queue->lock );

   if ( p_queue->sorted != NULL )
   {
      free( p_queue->sorted );
      p_queue->sorted = NULL;
   }

   if ( p_queue->entries != NULL )
   {
      free( p_queue->entries );
      p_queue->entries = NULL;
   }

   if ( p_queue->data != NULL )
   {
      free( p_queue->data );
      p_queue->data = NULL;
   }
}

/*********************************************************
**
**********************************************************/
int getNumOfElementsInQueue( PRI_QUEUE * p_queue )
{
   if ( p_queue == NULL )
   {
      return 0;
   }
   return p_queue->itemsused;
}

/*********************************************************
**
**********************************************************/
int add_item( PRI_QUEUE * p_queue
            , EW_PRIORITY p_priority
            , MSG_LOGO    p_logo
            , long        p_size
            , PRI_DATA    p_data
            )
{
#ifdef LOG_DEBUG
   char dbgstr[120];
#endif

   int r_status = EW_PRI_RETNORMAL;

   if ( p_queue == NULL )
   {
      return( EW_PRI_RETQNULL );
   }

   if ( p_queue->queuesize < 1 )
   {
      return( EW_PRI_RETNOTREADY );
   }

   RequestSpecificMutex( &p_queue->lock );

   if ( p_size < 0 || p_queue->itemmaxsize < p_size )
   {
      return ( EW_PRI_RETMSGSIZE );
   }
   else
   {
/*    EW_PRIORITY _usePri = p_priority; */

      long _queuesize = p_queue->queuesize
         , _ins_index = p_queue->queuesize - 1 /* array location to insert new object */
         , _src_index   /* array location to obtain container for insertion */
         ;
      PRI_QUEUE_ENTRY * _wrk_pointer;  /* item to be shifted */
      int _shift_direction = EW_PRI_SHIFT_NONE
         , _shift_stt
         , _shift_end
         , _idx             /* work index for loops */
         ;
      int _doSwap  = 0  /* swap old object for new */
      , _updateIdx = 0  /* update the priority insert location indices */
      ;

      if ( p_priority < EW_PRIORITY_MIN || EW_PRIORITY_MAX < p_priority )
      {
         /* priority out of range, fall back to default */
/*       _usePri = EW_PRIORITY_DEF; */
         /* keep this return status unless worse one arises */
         r_status = EW_PRI_RETBADPRI;
      }


      if ( p_queue->insert_indices[p_priority] < _queuesize )
      {
         /*
         ** The insert location is within the array
         */
         _ins_index = p_queue->insert_indices[p_priority];


         /* Check state of item at insert location */
         _wrk_pointer = p_queue->sorted[_ins_index];

         if (   _wrk_pointer->pri == 0           /* container at insert location unused */
             || _ins_index == (_queuesize - 1) /* insert location is last position in the array */
            )
         {
            /*
            ** Use the item at the insert location as the source container
            **
            ** _shift_direction = 0;
            */
            _doSwap = 1;
            _updateIdx = 1;
            if ( p_queue->itemsused < _queuesize )
            {
               p_queue->itemsused++;  /* increment number of items used */
            }
         }
         else
         {  /*
            ** the insert location is within the array space
            ** the item at the insert location is used
            ** the insert location is not the last position in the array
            */
            
            /* check if the array is full */
            if ( p_queue->itemsused < _queuesize )
            {
               /*
               ** The array is not full, therefore there is at least one
               ** unused item in the array.
               **
               ** Shift items down from insert point to first unused item
               */
               _src_index = p_queue->itemsused++;  /* increment number of items used */

               /* GET THE SOURCE CONTAINER */
               _wrk_pointer = p_queue->sorted[_src_index];

               /* SHIFT ITEMS DOWN */
               _shift_end = _ins_index;
               _shift_stt = _src_index;
               _shift_direction = EW_PRI_SHIFT_DOWN;

               _doSwap = 1;
               _updateIdx = 1;
            }
            else
            {
               /*
               ** The array is full.
               **
               ** Since we've already ascertained that the insert point is within the array
               ** we know that the item to be inserted is less than the highest priority
               ** stored in the array because there is at least one item of a higher priority
               ** following the items at the priority of the new item.
               **
               ** Get the priority of the last item in the array, use the earliest item
               ** of that priority to make space for the new item.
               ** This equates to shifting items from the insert location down to the
               ** insert location of the priority before the last (the first item of
               ** the last priority in the array).
               */
               _src_index = p_queue->insert_indices[ p_queue->sorted[_queuesize - 1]->pri - 1 ];

               /* GET THE SOURCE CONTAINER */
               _wrk_pointer = p_queue->sorted[_src_index];

               /* SHIFT ITEMS DOWN */
               _shift_end = _ins_index;
               _shift_stt = _src_index;
               _shift_direction = EW_PRI_SHIFT_DOWN;

               _doSwap = 1;
               _updateIdx = 1;

               r_status = EW_PRI_RETDROP;
            }
         }
      }  /*  insert_indices[p_priority] < _queuesize  */
      else
      {  /*  insert_indices[p_priority] == _queuesize
         **
         ** Insert index for the priority of the new item is past the end of
         ** the array.  (The array is full.)
         **
         ** Therefore, this item can only be inserted if this item has the
         ** same priority as the last item in the array and the priority
         ** of the last item in the array is not of the minimum priority.
         **
         ** In such a case, will drop the earliest item of the same priority,
         ** shift all other of the priority up, and tack the new item on at the end.
         **
         ** (That way, if the bandwidth opens up again and no others of this
         ** priority are dropped, we can send a continuous stream of this
         ** priority from the earliest point at which there was sufficient
         ** bandwidth).
         */
         if (   p_queue->sorted[_queuesize - 1]->pri != EW_PRIORITY_MIN
             && p_queue->insert_indices[p_priority - 1] < _queuesize
            )
         {
            /*
            ** The insert point for the priority immediately prior to this one
            ** is before the end of the array, indicating that this item is of
            ** the same priority as the last priority in the array.
            */
            /* GET THE SOURCE CONTAINER */
            _wrk_pointer = p_queue->sorted[ p_queue->insert_indices[p_priority - 1] ];

            /* SHIFT ITEMS UP */
            _shift_stt = p_queue->insert_indices[p_priority - 1];
            _shift_end = _queuesize - 1;
            _shift_direction = EW_PRI_SHIFT_UP;

            _doSwap = 1;
            _updateIdx = 1;

            r_status = EW_PRI_RETDROP;

         }
         else
         {
            r_status = EW_PRI_RETREJECT;
         }
      }


      if ( _doSwap == 1 )
      {

         _wrk_pointer->pri           = p_priority;
         (_wrk_pointer->logo).type   = p_logo.type;
         (_wrk_pointer->logo).mod    = p_logo.mod;
         (_wrk_pointer->logo).instid = p_logo.instid;
         _wrk_pointer->length        = p_size;
         /*
         ** copy message text
         **/
         if ( _wrk_pointer->data != NULL ) /* avoid invalid write at termination */
         {
            memcpy( _wrk_pointer->data, p_data, p_size );
         }
      }


      if ( _shift_direction != EW_PRI_SHIFT_NONE && p_queue->sorted != NULL )
      {
         PRI_QUEUE_ENTRY * _temp = p_queue->sorted[_shift_stt];

#ifdef DEBUG_DETAILS
         sprintf( dbgstr
                , "DEBUG add_item() shifting %d -> %d (%d)\n"
                , _shift_stt, _shift_end, _shift_direction
                );
         logit( "t" 
              , dbgstr
              , "export_pri"
              , "MOD_EXPORT_SCN"
              );
#endif
         for ( _idx = _shift_stt
             ; ( _shift_direction == EW_PRI_SHIFT_UP ? _idx < _shift_end : _idx > _shift_end )
             ; _idx += _shift_direction
             )
         {
#ifdef DEBUG_DETAILS
            sprintf( dbgstr
                   , "DEBUG shifting [%d] = [%d]\n"
                   , _idx, _idx + _shift_direction
                   );
            logit( "t" 
                 , dbgstr
                 , "export_pri"
                 , "MOD_EXPORT_SCN"
                 );
#endif
            p_queue->sorted[ _idx ] = p_queue->sorted[ _idx + _shift_direction ];
         }
         p_queue->sorted[_shift_end] = _temp;
      }


      if ( _updateIdx == 1 )
      {
         /* Adjust insert indices */
         for ( _idx = p_priority ; _idx < EW_PRIORITY_COUNT ; _idx++ )
         {
            if ( ( p_queue->insert_indices[_idx] + 1 ) <= _queuesize )
            {
               p_queue->insert_indices[_idx] = p_queue->insert_indices[_idx] + 1;
            }
         }
      }
   }

   ReleaseSpecificMutex( &p_queue->lock );

   return r_status;
}


/*********************************************************
**
**********************************************************/
int peek_next_item( PRI_QUEUE   * p_queue
                  , MSG_LOGO    * p_logoptr
                  , EW_PRIORITY * p_priptr
                  )
{
   int r_status = EW_PRI_NOITEM;  /* no items in queue */

   if ( p_queue == NULL )
   {
      return( EW_PRI_RETQNULL );
   }

   if ( p_queue->queuesize < 1  )
   {
      return( EW_PRI_RETNOTREADY );
   }

   RequestSpecificMutex( &p_queue->lock );

   if ( p_queue->sorted != NULL )
   {
      PRI_QUEUE_ENTRY * _wrk_pointer;  /* item to be shifted */
      /*
      ** Check the priority of the object referenced by the
      ** first pointer in the array.
      **
      ** If the priority is none, then there are no items in the list
      */
      _wrk_pointer = p_queue->sorted[0];

      if ( _wrk_pointer->pri != EW_PRIORITY_NONE )
      {  
         /* copy the logo */
         p_logoptr->type   = (_wrk_pointer->logo).type;
         p_logoptr->mod    = (_wrk_pointer->logo).mod ;
         p_logoptr->instid = (_wrk_pointer->logo).instid;
  
         *p_priptr  = _wrk_pointer->pri;  /* message priority */

         r_status = EW_PRI_RETNORMAL;
      }
   }

   ReleaseSpecificMutex( &p_queue->lock );

   return r_status;
}


/*********************************************************
**
**********************************************************/
int pop_next_item( PRI_QUEUE * p_queue
                 , MSG_LOGO  * p_logoptr
                 , long      * p_sizeptr
                 , PRI_DATA    p_data
                 )
{
#ifdef LOG_DEBUG
   char dbgstr[120];
#endif
   
   int r_status = EW_PRI_NOITEM;  /* no items in queue */

   PRI_QUEUE_ENTRY * _wrk_pointer;  /* item to be shifted */
   unsigned long _idx
               , _sz
               ;

   if ( p_queue == NULL )
   {
      return( EW_PRI_RETQNULL );
   }

   if ( p_queue->queuesize < 1  )
   {
      return( EW_PRI_RETNOTREADY );
   }

   RequestSpecificMutex( &(p_queue->lock) );


   /*
   ** Check the priority of the object referenced by the
   ** first pointer in the array.
   **
   ** If the priority is none, then there are no items in the list
   */
   _wrk_pointer = p_queue->sorted[0];

   if ( _wrk_pointer->pri != EW_PRIORITY_NONE )
   { 
      /* copy the logo */
      p_logoptr->type   = (_wrk_pointer->logo).type;
      p_logoptr->mod    = (_wrk_pointer->logo).mod ;
      p_logoptr->instid = (_wrk_pointer->logo).instid;
  
      *p_sizeptr  = _wrk_pointer->length;  /* message length */

      /*
      ** copy text of message to caller's memory 
      */
      memcpy( p_data, _wrk_pointer->data, (size_t)(_wrk_pointer->length) );

      /* shift pointers to the other queue entries up */
      for ( _idx = 0, _sz = p_queue->queuesize - 1 ; _idx < _sz ; _idx++ )
      {
         p_queue->sorted[_idx] = p_queue->sorted[_idx + 1];
      }

      /* clear the queue entry item */
      _wrk_pointer->pri    = EW_PRIORITY_NONE;
      _wrk_pointer->length = 0;

      /*
      ** Put the pointer to the now empty queue entry
      ** at the end of the sorted array
      */
      p_queue->sorted[p_queue->queuesize - 1] = _wrk_pointer;

      /* update the insert indices */
      for ( _idx = 0 ; _idx < EW_PRIORITY_COUNT ; _idx++ )
      {
         if ( 0 <= ( p_queue->insert_indices[_idx] - 1 ) )
         {
            p_queue->insert_indices[_idx] = p_queue->insert_indices[_idx] - 1;
         }
      }

      /* update the used count */
      (p_queue->itemsused)--;
      
      r_status = EW_PRI_RETNORMAL;
   }

#ifdef DEBUG_DETAILS
   sprintf( dbgstr
          , "%%s(%%s): DEBUG pop_next_item() releasing mutex; q state: %d of %d\n"
          , p_queue->itemsused
          , p_queue->queuesize
          );
   logit( "t" 
        , dbgstr
        , "export_pri"
        , "MOD_EXPORT_SCN"
        );
#endif

   ReleaseSpecificMutex( &p_queue->lock );

   return r_status;
}
