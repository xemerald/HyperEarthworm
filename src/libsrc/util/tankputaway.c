
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: tankputaway.c 6803 2016-09-09 06:06:39Z et $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.6  2004/07/16 20:44:21  lombard
 *     Modified to provide minimal support for SEED location codes.
 *
 *     Revision 1.5  2001/04/12 03:47:36  lombard
 *     revised putaway include files; cleaned up some comments
 *
 *     Revision 1.4  2001/03/28 04:50:27  lombard
 *     ripped out the junk that was attempting to multiplex traces.
 *     It works much better now, and uses less memory.
 *
 *     Revision 1.3  2000/09/20 18:06:24  lucky
 *     Removed fixed number of MAX_MSGS. Instead we allocate as many
 *     slots for messages as our max buffer can hold, and we remember
 *     that number so that we can free at the end.
 *
 *     Revision 1.2  2000/03/10 23:19:03  davidk
 *     changed the tankputaway interface to match the new PA interface in
 *     putaway.c
 *
 *     Revision 1.1  2000/02/14 18:51:48  lucky
 *     Initial revision
 *
 *
 */

/* tankputaway.c.c 

  Lucky Vidmar Mon Aug  2 10:15:46 MDT 1999

	These routines are called from putaway.c. They implement writing
  selected trace data into files which can be read by tankplayer.
*/

/*
  Tank putaway routines revised to make them work. The idea now is simple:
  Write a single file containing all the TRACEBUF2 messages for one SCN,
  followed by all the TRACEBUF2 messages for the next. The order of the
  SCNs or TRACEBUF2 messages doesn't matter.
  Then to make this into a tankplayer file, use Lynn Dietz' remux_tbuf
  program, currently in Contrib/Menlo.  Pete Lombard, March 2001
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <earthworm.h>
#include <trace_buf.h>
#include <swap.h>
#include <pa_subs.h>

#define 	MAXTXT           	150

static char     TankFile[MAXTXT];
static FILE     *fpTank;

/* Initialization function, 
*       This is the Put Away startup intializer. This is called when    *
*       the system first comes up. Here is a chance to look around      *
*       and see if it's possible to do business, and to complain        *
*       if not, BEFORE an event has to be processed.                    *
*/
int TANKPA_init(char *OutDir, int debug)
{
  if (OutDir == NULL)
  {
    logit ("e", "TANKPA_init: Invalid arguments passed in.\n");
    return EW_FAILURE;
  }

  if (debug == 1)
    logit ("", "In TANKPA_init\n");


  /* Make sure that the output directory exists */
  if (CreateDir (OutDir) != EW_SUCCESS)
  {
    logit ("e", "TANKPA_init: Call to CreateDir failed\n");
    return EW_FAILURE;
  }


  if (debug == 1)
    logit ("", "Done TANKPA_init\n");

  return EW_SUCCESS; 

}

/****************************************************************************
*       This is the Put Away event initializer. It's called when a snippet  *
*       has been received, and is about to be processed.                    *
*       It gets to see the pointer to the TraceRequest array,               *
*       and the number of loaded trace structures.                          *
*****************************************************************************/
int TANKPA_next_ev(char * EventID, char *OutDir, char *EventDate, 
                   char *EventTime, int debug)
{

  char	tmp1[256];

  if ((OutDir == NULL) || (EventDate == NULL) || 
      (EventTime == NULL) || (EventID == NULL))
  {
    logit ("e", "TANKPA_next_ev: Invalid parameters passed in.\n");
    return EW_FAILURE;
  }

  if (debug == 1)
    logit ("", "In TANKPA_next_ev\n");


  /* Tank files will be written in the following format:
   *   TankFile = OutDir/yyyymmdd_hhmmss-iiii.tnk
   */

  /* build the event file name */
  sprintf (tmp1, "%s_%s_%s", EventDate, EventTime, EventID);

  /* NT can handle "/" as a path separator here;        *
   *  it's only the NT command-shell that requires "\"  */
  sprintf (TankFile, "%s/%s", OutDir, tmp1); 

  /* open the file */
  if ((fpTank = fopen (TankFile, "wb")) == NULL)
  {
    logit ("e", "TANKPA_init: Can't open %s.\n", TankFile);
    return EW_FAILURE;
  }

  if (debug == 1)
    logit ("", "opened %s\n", TankFile);

  if (debug == 1)
    logit ("", "Done TANKPA_next_ev\n");

  return EW_SUCCESS;

}

/*****************************************************************************
 *   This is the working entry point into the disposal system. This routine  *
 *   gets called for each trace snippet which has been recovered. It gets    *
 *   to see the corresponding SNIPPET structure, and the event id            *
 *****************************************************************************/
/* Process one channel of data */
int TANKPA_next(TRACE_REQ *getThis, int debug)
{
  TRACE2_HEADER 	*in_head = NULL;
  char        	*in_msg; 
  int          byte_per_sample;
  long         this_size, ret;

  if (debug == 1)
    logit ("", "In TANKPA_next\n");

  if ((in_msg = getThis->pBuf) == NULL)
  {
    logit ("e", "TANKPA_next: in_msg buffer is NULL\n");
    return EW_FAILURE;
  }

  /* loop through all the messages for this s-c-n */
  while ((size_t) in_msg < (size_t) getThis->actLen + (size_t) getThis->pBuf)
  {
    /* read the header inforamtion for each TRACE_BUF message */
    in_head = (TRACE2_HEADER *) in_msg;

    /* Swap bytes, if necessary, to make this message 
     * readable on this architecture
     *************************************************/
    WaveMsg2MakeLocal (in_head);


    byte_per_sample  = atoi (&in_head->datatype[1]); /* input sample size */

    /* how big is the current message? */
    this_size = sizeof (TRACE2_HEADER) + (byte_per_sample * in_head->nsamp);


    /* Copy the message */
    ret = (long)fwrite (in_msg, sizeof (char), this_size, fpTank);
    if (ret != this_size)
    {
      logit ("e", "TANKPA_next: Write to %s failed.\n", TankFile);
      return EW_FAILURE;
    }

  /* Increment incoming message */
  in_msg += this_size;

  }
  if (debug == 1)
    logit ("", "TANKPA_next: done with <%s.%s.%s.%s>\n", in_head->sta, 
           in_head->chan, in_head->net, in_head->loc);

  return EW_SUCCESS;

}

/************************************************************************
*       This is the Put Away end event routine. It's called after we've     *
*       finished processing one event.                                  *
*************************************************************************/
int TANKPA_end_ev(int debug)
{

 fclose (fpTank);

 if (debug == 1)
   logit ("", "Closed %s. Done TANKPA_end_ev\n", TankFile);

 return EW_SUCCESS;
}


/************************************************************************
*       This is the Put Away close routine. It's called after when      *
*       we're being shut down.                                          *
*************************************************************************/
int TANKPA_close(int debug)
{

  if (debug == 1)
    logit ("", "In TANKPA_close, doing nothing useful.\n");

  return EW_SUCCESS;
}

