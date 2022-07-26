
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: putaway.c 7901 2019-03-09 01:51:25Z stefan $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.14  2009/06/02 14:11:23  tim
 *     Adding support for wfdisc format
 *
 *     Revision 1.13  2007/03/06 17:52:39  paulf
 *     added psn4 format to putaway for WIN only
 *
 *     Revision 1.12  2007/02/17 01:36:46  stefan
 *     solaris specific ifdefs
 *
 *     Revision 1.11  2007/02/07 05:42:29  stefan
 *     miniseed writer thanks to BGS, Richard Luckett
 *
 *     Revision 1.10  2002/03/20 21:07:01  cjbryan
 *     added EventSubnet to SEISPA_next_ev call
 *
 *
 * 3-feb-2001, S. Flower (British Geological Survey)
 *     Added code for new 'gse_int' and 'seisan' output data types
 *     All new code is prefixed with the initials 'SMF'

 *     Revision 1.8  2001/04/12 03:47:36  lombard
 *     Added include file putaway.h for function prototypes
 *     Removed MaxTraceMsg argument from PA_init(), no longer used.
 *
 *     Revision 1.7  2001/03/22 20:56:29  cjbryan
 *     deleted EventInst from PA_next_ev function call as it is no
 *     longer needed by any of the putaway routines
 *     cleaned up all other relic uses of EventInst and EventMod
 *
 *     Revision 1.6  2001/03/21 16:40:40  alex
 *     deleted now extraneous EventMod variable
 *
 *     Revision 1.5  2001/03/21 02:15:28  alex
 *     added EventSubnet to SUDSPA_next_ev call
 *
 *     Revision 1.4  2000/07/08 19:26:05  lombard
 *     replace `=' in if statements with `=='; two of these were in PA_close().
 *
 *     Revision 1.3  2000/03/14 18:35:11  lucky
 *     *** empty log message ***
 *
 *     Revision 1.2  2000/03/10 23:21:05  davidk
 *     changed the putaway routines so that OutputFormat is specified at
 *     initialization instead of per SCN.  Consolidated all of the EventID's
 *     so that now there is a single EventID string that is passed into
 *     PA_next_ev.
 *     /
 *
 *     Revision 1.1  2000/02/14 18:51:48  lucky
 *     Initial revision
 *
 *
 */

/*
    putaway.c

  Thu Jul 22 13:21:09 MDT 1999 lucky

   Separated the putaway routines for the following output
   formats currently supported:
        ah
        sac
        suds
        seisan
        gse_int
        tank

   This file will now become a library object, so that it can be
   used by both trig2disk and wave2disk. Therefore, adding a new
   output format will only require changing this file.


 Wed Oct 20 09:53:50 MDT 1999 lucky

   Fixed a bug which limited the size of the OutBuffer.


*/

/* Sep 2006 Richard Luckett BGS added miniSEED */

#include <string.h>
#include <earthworm.h>
#include <ws_clientII.h>
#include <ahhead.h>   /* for AHHEADSIZE */
#include <putaway.h>
#include <cosmos0putaway.h>
#include <pa_subs.h>
#include <sachead.h>  /* for SACHEADERSIZE */

/* SMF: definitions for GSE and SEIsan putaway routines */
#include "gsehead.h"
#include "seihead.h"

#ifdef _LINUX
#include "wfdischead.h" /* for wfdisc putaway routines T. Zander */
#endif /* _LINUX */

#ifdef _WINNT
/* L.Cochrane: for psn event files */
#include <psn4putaway.h>
#endif /* _WINNT */


/* Initialization function,
*       This is the Put Away startup intializer. This is called when    *
*       the system first comes up. Here is a chance to look around      *
*       and see if it's possible to do business, and to complain        *
*       if not ,BEFORE an event has to be processed.                    *
*/
int PA_init (char *DataFormat, long TraceBufferLen, long *OutBufferLen,
             int *FormatInd, char *OutDir, char * OutputFormat, int debug)
{

  if ((DataFormat == NULL) || (TraceBufferLen <= 0) ||
      (OutBufferLen == NULL) || (FormatInd == NULL))
  {
    logit ("e", "putaway: invalid parameters passed in.\n");
    return EW_FAILURE;
  }

  /* size OutBuffer to  include header info */
  if (strcmp (DataFormat,"ah") == 0)
  {
    *FormatInd = AH_FORMAT;
    *OutBufferLen = TraceBufferLen + AHHEADSIZE;
    if (AHPA_init (OutBufferLen, OutDir, debug) != EW_SUCCESS)
    {
      logit("e", "putaway: Call to AHPA_init failed!\n" );
      return EW_FAILURE;
    }

  }
  else if (strcmp (DataFormat,"sac") == 0)
  {
    *FormatInd = SAC_FORMAT;
    *OutBufferLen = TraceBufferLen + SACHEADERSIZE;
    if (SACPA_init (*OutBufferLen, OutDir, OutputFormat, debug) != EW_SUCCESS)
    {
      logit("e", "putaway: Call to SACPA_init failed!\n" );
      return EW_FAILURE;
    }
  }
  else if (strcmp (DataFormat,"suds") == 0)
  {
    *FormatInd = SUDS_FORMAT;
    *OutBufferLen = TraceBufferLen;
    if (SUDSPA_init (*OutBufferLen, OutDir, OutputFormat, debug) != EW_SUCCESS)
    {
      logit("e", "putaway: Call to SUDSPA_init failed!\n" );
      return EW_FAILURE;
    }
  }
  else if (strcmp (DataFormat,"tank") == 0)
  {
    *FormatInd = TANK_FORMAT;
    if (TANKPA_init (OutDir, debug) != EW_SUCCESS)
    {
      logit("e", "putaway: Call to TANKPA_init failed!\n" );
      return EW_FAILURE;
    }
  }
  /* SMF: new code for GSE */
  else if (strcmp (DataFormat,"gse_int") == 0)
  {
    *FormatInd = GSE_INT_FORMAT;
    *OutBufferLen = TraceBufferLen;
    if (GSEPA_init (OutDir, OutputFormat, debug) != EW_SUCCESS)
    {
      logit("e", "putaway: Call to GSEPA_init failed!\n" );
            return EW_FAILURE;
    }
  }
  /* SMF: new code for SEIsan */
  else if (strcmp (DataFormat,"seisan") == 0)
  {
    *FormatInd = SEI_FORMAT;
    *OutBufferLen = TraceBufferLen;
    if (SEIPA_init (OutDir, OutputFormat, debug) != EW_SUCCESS)
    {
      logit("e", "putaway: Call to SEIPA_init failed!\n" );
      return EW_FAILURE;
    }
  }
  /* Cosmos V0 format */
  else if (strcmp(DataFormat, "cosmos0") == 0)
  {
	  *FormatInd = COSMOS0_FORMAT;
	  *OutBufferLen = TraceBufferLen;
	  if (COSMOS0PA_init(*OutBufferLen, OutDir, OutputFormat, debug) != EW_SUCCESS)
	  {
		  logit("e", "putaway: Call to COSMOS0PA_init failed!\n");
		  return EW_FAILURE;
	  }
  }
#ifdef _LINUX
  /* T. Zander: new code for wfdisc */
  else if (strcmp (DataFormat,"wfdisc") == 0)
  {
    *FormatInd = WFDISC_FORMAT;
    *OutBufferLen = TraceBufferLen;
    if (WFDISCPA_init (OutDir, OutputFormat, debug) != EW_SUCCESS)
    {
      logit("e", "putaway: Call to WFDISCPA_init failed!\n" );
      return EW_FAILURE;
    }
  }
#endif /* _LINUX */
#ifdef _UNIX
  else if (strcmp (DataFormat,"uw") == 0)
  {
    *FormatInd = UW_FORMAT;
    *OutBufferLen = TraceBufferLen;
    if (UWPA_init (TraceBufferLen, OutDir, OutputFormat, debug) != EW_SUCCESS)
    {
      logit("e", "putaway: Call to UWPA_init failed!\n" );
      return EW_FAILURE;
    }
  }
#endif /* _UNIX */

#ifdef _WINNT
  /* L. Cochrane: new code for PSN Type 4 Event Files */
  else if (strcmp (DataFormat,"psn4") == 0)
  {
    *FormatInd = PSN4_FORMAT;
    *OutBufferLen = TraceBufferLen;
    if (PSN4PA_init (*OutBufferLen, OutDir, OutputFormat, debug) != EW_SUCCESS)
    {
      logit("", "waveputaway: Call to PSN4PA_init failed!\n" );
      return EW_FAILURE;
    }
  }
#endif /* _WINNT */

  /* RL: code for miniSEED */
  else if (strcmp (DataFormat,"mseed") == 0)
  {
    *FormatInd = MSEED_FORMAT;
    *OutBufferLen = TraceBufferLen;
    if (MSEEDPA_init (OutDir, OutputFormat, debug) != EW_SUCCESS)
    {
      logit("e", "putaway: Call to MSEEDPA_init failed!\n" );
      return EW_FAILURE;
    }
  }

  else
  {
    logit("e","putaway: undefined DataFormat: %s\n", DataFormat);
    return EW_FAILURE;
  }

  return EW_SUCCESS;

}


/****************************************************************************
*       This is the Put Away event initializer. It's called when a snippet
*       has been received, and is about to be processed.
*       It gets to see the pointer to the TraceRequest array,
*       and the number of loaded trace structures.
*****************************************************************************/
int PA_next_ev (char *EventID, TRACE_REQ *trace_req, int num_req,
                int FormatInd, char *OutDir, char *LibDir, char *EventDate,
                char *EventTime, char *EventSubnet, int debug)
{


  if ((trace_req == NULL) || (OutDir == NULL) || (EventDate == NULL) ||
      (EventTime == NULL) || (EventID == NULL))
  {
    logit ("", "waveputaway: Invalid parameters passed in!\n");
    return EW_FAILURE;
  }


  if (FormatInd == AH_FORMAT)
  {
    if (AHPA_next_ev (trace_req, num_req, OutDir,
                      EventDate, EventTime, debug) != EW_SUCCESS)
    {
      logit ("", "waveputaway: Call to AHPA_next_ev failed!\n");
      return EW_FAILURE;
    }
  }
  else if (FormatInd == SAC_FORMAT)
  {
    if (SACPA_next_ev (EventID, trace_req, num_req, OutDir,
                       EventDate, EventTime, debug) != EW_SUCCESS)
    {
      logit ("", "waveputaway: Call to SACPA_next_ev failed!\n");
      return EW_FAILURE;
    }
  }
  else if (FormatInd == SUDS_FORMAT)
  {
    if (SUDSPA_next_ev(EventID, trace_req, num_req, OutDir, EventDate,
                       EventTime, EventSubnet, debug) != EW_SUCCESS)
    {
      logit ("", "waveputaway: Call to SUDSPA_next_ev failed!\n");
      return EW_FAILURE;
    }

  }
  else if (FormatInd == TANK_FORMAT)
  {
    if (TANKPA_next_ev (EventID, OutDir, EventDate,
                        EventTime, debug) != EW_SUCCESS)
    {
      logit ("", "waveputaway: Call to TANKPA_next_ev failed!\n");
      return EW_FAILURE;
    }
  }
  /* SMF: new code for GSE */
  else if (FormatInd == GSE_INT_FORMAT)
  {
    if (GSEPA_next_ev (trace_req, num_req, OutDir,
               EventDate, EventTime, debug) != EW_SUCCESS)
    {
      logit ("", "waveputaway: Call to GSEPA_next_ev failed!\n");
      return EW_FAILURE;
    }
  }
  /* SMF: new code for SEIsan */
  else if (FormatInd == SEI_FORMAT)
  {
    if (SEIPA_next_ev (trace_req, num_req, OutDir,
                   EventDate, EventTime, EventSubnet, debug) != EW_SUCCESS)
    {
      logit ("", "waveputaway: Call to SEIPA_next_ev failed!\n");
      return EW_FAILURE;
    }
  }
  else if (FormatInd == COSMOS0_FORMAT)
  {
	  if (COSMOS0PA_next_ev(trace_req, OutDir, LibDir, EventDate, EventTime, 
		  1, debug, 1.0) != EW_SUCCESS)
	  {
		  logit("", "waveputaway: Call to COSMOS0PA_next_ev failed!\n");
		  return EW_FAILURE;
	  }

  }
#ifdef _WINNT
  /* L. Cochrane: new code for PSN Type 4 Event Files */
  else if (FormatInd == PSN4_FORMAT)
  {
    if (PSN4PA_next_ev( trace_req, EventDate, EventTime, debug ) != EW_SUCCESS)
    {
      logit ("", "waveputaway: Call to PSN4PA_next_ev failed!\n");
      return EW_FAILURE;
    }
  }
#endif /* _WINNT */

  /* RL: code for miniSEED */
  else if (FormatInd == MSEED_FORMAT)
  {
    if (MSEEDPA_next_ev (OutDir, EventDate, EventTime, debug) != EW_SUCCESS)
    {
      logit ("", "waveputaway: Call to MSEEDPA_next_ev failed!\n");
      return EW_FAILURE;
    }
  }

#ifdef _LINUX
  /* T. Zander: new code for wfdisc */
  else if (FormatInd == WFDISC_FORMAT)
  {
    if (WFDISCPA_next_ev (trace_req, num_req, OutDir,
                   EventDate, EventTime, EventSubnet, debug) != EW_SUCCESS)
    {
      logit ("", "waveputaway: Call to WFDISCPA_next_ev failed!\n");
      return EW_FAILURE;
    }
  }
#endif /* _LINUX */
#ifdef _UNIX
  else if (FormatInd == UW_FORMAT)
  {
    if (UWPA_next_ev (EventID, trace_req, num_req, OutDir,
                   EventDate, EventTime, EventSubnet, NULL, debug) != EW_SUCCESS)
    {
      logit ("", "waveputaway: Call to UWPA_next_ev failed!\n");
      return EW_FAILURE;
    }
  }
#endif /* _UNIX */
  else
  {
    logit ("", "waveputaway: undefined DataFormat: %d!\n", FormatInd);
    return EW_FAILURE;
  }


  return EW_SUCCESS;
}



/*****************************************************************************
*   This is the working entry point into the disposal system. This routine
*   gets called for each trace snippet which has been recovered. It gets
*   to see the corresponding SNIPPET structure, and the event id
*****************************************************************************/
int PA_next (TRACE_REQ *getThis, int FormatInd,
             double GapThresh, long OutBufferLen, int debug)
{

  if (getThis == NULL)
  {
    logit ("e", "Invalid arguments passed in.\n");
    return EW_FAILURE;
  }

  if (FormatInd == AH_FORMAT)
  {
    if (AHPA_next (getThis, GapThresh, OutBufferLen, debug) != EW_SUCCESS)
    {
      logit("", "waveputaway: Call to AHPA_next failed!\n");
      return EW_FAILURE;
    }
  }
  else if (FormatInd == SAC_FORMAT)
  {
    if (SACPA_next (getThis, GapThresh, OutBufferLen, debug) != EW_SUCCESS)
    {
      logit("", "waveputaway: Call to SACPA_next failed!\n");
      return EW_FAILURE;
    }
  }
  else if (FormatInd == SUDS_FORMAT)
  {
    if (SUDSPA_next (getThis, GapThresh, OutBufferLen, debug) != EW_SUCCESS)
    {
      logit("", "waveputaway: Call to SUDSPA_next failed!\n");
      return EW_FAILURE;
    }
  }
  else if (FormatInd == TANK_FORMAT)
  {
    if (TANKPA_next (getThis, debug) != EW_SUCCESS)
    {
      logit("", "waveputaway: Call to TANKPA_next failed!\n");
      return EW_FAILURE;
    }
  }
  /* SMF: new code for GSE */
  else if (FormatInd == GSE_INT_FORMAT)
  {
    if (GSEPA_next (getThis, GapThresh, debug) != EW_SUCCESS)
    {
      logit("", "waveputaway: Call to GSEPA_next failed!\n");
      return EW_FAILURE;
    }
  }
  /* SMF: new code for SEIsan */
  else if (FormatInd == SEI_FORMAT)
  {
    if (SEIPA_next (getThis, GapThresh, debug) != EW_SUCCESS)
    {
      logit("", "waveputaway: Call to SEIPA_next failed!\n");
      return EW_FAILURE;
    }
  }
#ifdef _WINNT
  /* L.Cochrane: new code for PSN Type 4 Event Files */
  else if (FormatInd == PSN4_FORMAT)
  {
    if (PSN4PA_next (getThis, GapThresh, OutBufferLen, debug) != EW_SUCCESS)
    {
      logit("", "waveputaway: Call to PSN4PA_next failed!\n");
      return EW_FAILURE;
    }
  }
#endif /* _WINNT */

  /* RL: code for miniSEED */
  else if (FormatInd == MSEED_FORMAT)
  {
    if (MSEEDPA_next (getThis, GapThresh, debug) != EW_SUCCESS)
    {
      logit("", "waveputaway: Call to MSEEDPA_next failed!\n");
      return EW_FAILURE;
    }
  }

#ifdef _LINUX
  /* T. Zander: new code for wfdisc */
  else if (FormatInd == WFDISC_FORMAT)
  {
    if (WFDISCPA_next (getThis, GapThresh, debug) != EW_SUCCESS)
    {
      logit("", "waveputaway: Call to WFDISCPA_next failed!\n");
      return EW_FAILURE;
    }
  }
#endif /* _LINUX */
#ifdef _UNIX
  else if (FormatInd == UW_FORMAT)
  {
    if (UWPA_next (getThis, GapThresh, OutBufferLen, debug) != EW_SUCCESS)
    {
      logit("", "waveputaway: Call to UWPA_next failed!\n");
      return EW_FAILURE;
    }
  }
#endif /* _UNIX */
  else if (FormatInd == COSMOS0_FORMAT)
  {
	  if (COSMOS0PA_next(getThis, GapThresh, OutBufferLen, debug, 1, 1.0) != EW_SUCCESS)
	  {
		  logit("", "waveputaway: Call to COSMOS0PA_next failed!\n");
		  return EW_FAILURE;
	  }
  }
  else
  {
    logit("", "waveputaway: undefined DataFormat: %d\n", FormatInd);
    return EW_FAILURE;
  }

  return EW_SUCCESS;

}


/************************************************************************
*       This is the Put Away end event routine. It's called after we've *
*                                                                       *
*       finished processing one event.                                  *
*************************************************************************/
int PA_end_ev (int FormatInd, int debug)
{

  if (FormatInd == AH_FORMAT)
  {
    if (AHPA_end_ev (debug) != EW_SUCCESS)
    {
      logit ("", "waveputaway: Call to AHPA_end_ev failed!\n");
      return EW_FAILURE;
    }
  }
  else if (FormatInd == SAC_FORMAT)
  {
    if (SACPA_end_ev (debug) != EW_SUCCESS)
    {
      logit ("", "waveputaway: Call to SACPA_end_ev failed!\n");
      return EW_FAILURE;
    }
  }
  else if (FormatInd == SUDS_FORMAT)
  {
    if (SUDSPA_end_ev (debug) != EW_SUCCESS)
    {
      logit ("", "waveputaway: Call to SUDSPA_end_ev failed!\n");
      return EW_FAILURE;
    }
  }
  else if (FormatInd == TANK_FORMAT)
  {
    if (TANKPA_end_ev (debug) != EW_SUCCESS)
    {
      logit ("", "waveputaway: Call to TANKPA_end_ev failed!\n");
      return EW_FAILURE;
    }
  }
  /* SMF: new code for GSE */
  else if (FormatInd == GSE_INT_FORMAT)
  {
    if (GSEPA_end_ev (debug) != EW_SUCCESS)
    {
      logit ("", "waveputaway: Call to GSEPA_end_ev failed!\n");
      return EW_FAILURE;
    }
  }
  /* SMF: new code for SEIsan */
  else if (FormatInd == SEI_FORMAT)
  {
    if (SEIPA_end_ev (debug) != EW_SUCCESS)
    {
      logit ("", "waveputaway: Call to SEIPA_end_ev failed!\n");
      return EW_FAILURE;
    }
  }
#ifdef _WINNT
  /* L.Cochrane: new code for PSN Type 4 Event Files */
  else if (FormatInd == PSN4_FORMAT)
  {
    if (PSN4PA_end_ev (debug) != EW_SUCCESS)
    {
      logit ("", "waveputaway: Call to PSN4PA_end_ev failed!\n");
      return EW_FAILURE;
    }
  }
#endif /* _WINNT */
  /* RL: code for miniSEED */
  else if (FormatInd == MSEED_FORMAT)
  {
    if (MSEEDPA_end_ev (debug) != EW_SUCCESS)
    {
      logit ("", "waveputaway: Call to MSEEDPA_end_ev failed!\n");
      return EW_FAILURE;
    }
  }

#ifdef _LINUX
  /* T. Zander: new code for wfdisc */
  else if (FormatInd == WFDISC_FORMAT)
  {
    if (WFDISCPA_end_ev (debug) != EW_SUCCESS)
    {
      logit ("", "waveputaway: Call to WFDISCPA_end_ev failed!\n");
      return EW_FAILURE;
    }
  }
#endif /* _LINUX */
#ifdef _UNIX
  else if (FormatInd == UW_FORMAT)
  {
    if (UWPA_end_ev (debug) != EW_SUCCESS)
    {
      logit ("", "waveputaway: Call to UWPA_end_ev failed!\n");
      return EW_FAILURE;
    }
  }
#endif /* _UNIX */
  else if (FormatInd == COSMOS0_FORMAT)
  {
	  if (COSMOS0PA_end_ev(debug) != EW_SUCCESS)
	  {
		  logit("", "waveputaway: Call to COSMOS0PA_end_ev failed!\n");
		  return EW_FAILURE;
	  }
  }
  else
  {
    logit ("", "waveputaway: undefined DataFormat: %d\n", FormatInd);
    return EW_FAILURE;
  }

  return EW_SUCCESS;
}

/************************************************************************
*       This is the Put Away close routine. It's called after when      *
*       we're being shut down.                                          *
*************************************************************************/
int PA_close (int FormatInd, int debug)
{

  if (FormatInd == AH_FORMAT)
  {
    if (AHPA_close (debug) != EW_SUCCESS)
    {
      logit ("", "waveputaway: Call to AHPA_close failed!\n");
      return EW_FAILURE;
    }
  }
  else if (FormatInd == SAC_FORMAT)
  {
    if (SACPA_close (debug) != EW_SUCCESS)
    {
      logit ("", "waveputaway: Call to SACPA_close failed!\n");
      return EW_FAILURE;
    }
  }
  else if (FormatInd == SUDS_FORMAT)
  {
    if (SUDSPA_close (debug) != EW_SUCCESS)
    {
      logit ("", "waveputaway: Call to SUDSPA_close failed!\n");
      return EW_FAILURE;
    }
  }
  else if (FormatInd == TANK_FORMAT)
  {
    if (TANKPA_close (debug) != EW_SUCCESS)
    {
      logit ("", "waveputaway: Call to TANKPA_close failed!\n");
      return EW_FAILURE;
    }
  }
  /* SMF: new code for GSE */
  else if (FormatInd == GSE_INT_FORMAT)
  {
    if (GSEPA_close (debug) != EW_SUCCESS)
    {
      logit ("", "waveputaway: Call to GSEPA_close failed!\n");
      return EW_FAILURE;
    }
  }
  /* SMF: new code for SEIsan */
  else if (FormatInd == SEI_FORMAT)
  {
    if (SEIPA_close (debug) != EW_SUCCESS)
    {
      logit ("", "waveputaway: Call to SEIPA_close failed!\n");
      return EW_FAILURE;
    }
  }
#ifdef _WINNT
  /* L.Cochrane: new code for PSN Type 4 Event Files */
  else if (FormatInd == PSN4_FORMAT)
  {
    if (PSN4PA_close (debug) != EW_SUCCESS)
    {
      logit ("", "waveputaway: Call to PSN4PA_close failed!\n");
      return EW_FAILURE;
    }
  }
#endif /* _WINNT */
  /* RL: code for miniSEED */
  else if (FormatInd == MSEED_FORMAT)
  {
    if (MSEEDPA_close (debug) != EW_SUCCESS)
    {
      logit ("", "waveputaway: Call to MSEEDPA_close failed!\n");
      return EW_FAILURE;
    }
  }

#ifdef _LINUX
  /* T. Zander: new code for wfdisc */
  else if (FormatInd == WFDISC_FORMAT)
  {
    if (WFDISCPA_close (debug) != EW_SUCCESS)
    {
      logit ("", "waveputaway: Call to WFDISCPA_close failed!\n");
      return EW_FAILURE;
    }
  }
#endif /* _LINUX */

  else if (FormatInd == UW_FORMAT)
  {
      /* do nothing */
  }
  else if (FormatInd == COSMOS0_FORMAT)
  {
	  if (COSMOS0PA_close(debug) != EW_SUCCESS)
	  {
		  logit("", "waveputaway: Call to COSMOS0PA_close failed!\n");
		  return EW_FAILURE;
	  }
  }
  else
  {
    logit ("", "waveputaway: undefined DataFormat: %d\n", FormatInd);
    return EW_FAILURE;
  }

  return EW_SUCCESS;

}
