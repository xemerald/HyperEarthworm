/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: glevt_2_ewevent.c 1988 2005-08-15 18:59:49Z friberg $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2005/08/15 18:59:48  friberg
 *     updated global files from hydra
 *
 *     Revision 1.4  2005/02/15 19:04:56  davidk
 *     SCNL Conversion.
 *     Changed code to copy the Location code from the globlal event format
 *     to the EW event format, instead of just copying a blank string.
 *
 *     Revision 1.3  2004/09/13 23:17:14  davidk
 *     Changed effective hard-coding of pick-association-weight to 1.0 from 0.0.
 *
 *     Revision 1.2  2004/08/06 01:17:04  davidk
 *     Changed pick and origin times from strings to doubles(secs since 1970)
 *     in the global_loc structs.
 *
 *     Revision 1.1.1.1  2004/03/31 18:43:18  michelle
 *     New Hydra Import
 *
 *     Revision 1.3  2003/10/21 21:04:52  lucky
 *     Fixed the transfer of magnitude types in PeakAmp records
 *
 *     Revision 1.2  2003/09/04 20:59:04  lucky
 *     Replaced AMPLITUDE_TYPE with MAGNITUDE_TYPE
 *
 *     Revision 1.1  2003/09/03 15:30:13  lucky
 *     Initial revision
 *
 *     Revision 1.6  2003/06/10 14:41:25  lucky
 *     *** empty log message ***
 *
 *     Revision 1.5  2003/01/28 17:14:34  lucky
 *     Added Starting location fields to origins
 *
 *     Revision 1.4  2002/09/10 17:21:21  lucky
 *     Stable scaffold
 *
 *     Revision 1.3  2002/07/16 20:03:20  davidk
 *     removed //comments
 *
 *     Revision 1.2  2002/07/16 19:48:12  davidk
 *     Changed the code that parses the global event message, so
 *     that it reads the message one line at a time, instead of a fixed
 *     number of bytes (supposing a line) at a time.
 *     By reading exactly 96 bytes for the first line, the code bombed
 *     when reading the message as a text file on NT because of the
 *     0x0d 0x0a (CR LF) 2 character EOL sequence.  The 2 char sequence
 *     caused there to be 1 extra char on each line, and thus the parser
 *     was off by 1 extra char(byte) each line.
 *     The new code extracts one line per time from the message, no matter
 *     the line's length, but then uses the same sscanf() parsing code
 *     as before.
 *
 *     Revision 1.1  2002/06/28 21:06:22  lucky
 *     Initial revision
 *
 *
 *
 */


/*
 * glevt_2_ewevent.c :  Routines to produce and parse global solution
 *   messages of type TYPE_EVENTGLOBAL
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <chron3.h>
#include <earthworm.h>
#include <rw_glevt.h>
#include <global_loc_rw.h>

/******************************************************************************
 GlEvt2EWEvent() fills EWEvent structure from a string in GlobalEvent format 
******************************************************************************/
int 	GlEvt2EWEvent (EWEventInfoStruct *pEWEvent, char *pGlEvt, int GlEvtLen)
{

    int i, j, ind, rc = 0;
    static GLOBAL_LOC_STRUCT LocData; /* not thread safe */


	if ((pEWEvent == NULL) || (pGlEvt == NULL) || (GlEvtLen <= 0))
	{
		logit ("", "Invalid arguments passed in.\n");
		return EW_FAILURE;
	}


    InitGlobalLoc (&LocData);

   switch ((rc = StringToLoc (&LocData, pGlEvt)))
   {
     case GLOBAL_MSG_SUCCESS:

         /* 
          * KLUDGE CITY:  copy elements from local structures into the
          *   super-mondo API event structure 
          */
	        /* Initialize the Event structure */
        	if (InitEWEvent (pEWEvent) != EW_SUCCESS)
        	{
        		logit ("", "Call to InitEWEvent failed.\n");
        		return EW_FAILURE;
        	}


	        /* 
             * Set magnitude values to 0 as we don't have any info on it here
	         */
	        pEWEvent->iNumMags = 0;
        	pEWEvent->iPrefMag = -1;
        	pEWEvent->iMd = -1;
        	pEWEvent->iML = -1;

        	pEWEvent->iNumChans = LocData.nphs;
        	pEWEvent->Event.idEvent = LocData.event_id;

        	pEWEvent->PrefOrigin.tOrigin = LocData.tOrigin;
			 
        	pEWEvent->PrefOrigin.dLat = (float) LocData.lat;
        	pEWEvent->PrefOrigin.dLon = (float) LocData.lon;
        	pEWEvent->PrefOrigin.dDepth = (float) LocData.depth;

        	pEWEvent->PrefOrigin.dRms = LocData.rms;
	        pEWEvent->PrefOrigin.dDmin = (float) LocData.dmin;
	        pEWEvent->PrefOrigin.iGap = (int) LocData.gap;
	        pEWEvent->PrefOrigin.iVersionNum = LocData.origin_id;

	        pEWEvent->PrefOrigin.iAssocRd = LocData.nphs;
        	pEWEvent->PrefOrigin.iAssocPh = LocData.nphs;
        	pEWEvent->PrefOrigin.iUsedRd = LocData.nphs;
        	pEWEvent->PrefOrigin.iUsedPh = LocData.nphs;

        	pEWEvent->PrefOrigin.BindToEvent = TRUE;
        	pEWEvent->PrefOrigin.SetPreferred = TRUE;


        	if (pEWEvent->iNumChans > pEWEvent->iNumAllocChans)
        	{
        		free (pEWEvent->pChanInfo);

	        	if ((pEWEvent->pChanInfo = (EWChannelDataStruct *) malloc 
	    					(pEWEvent->iNumChans * sizeof (EWChannelDataStruct))) == NULL)
	        	{
	        		logit ("", "Could not malloc %d channel info structs\n", pEWEvent->iNumChans);
	        		return EW_FAILURE;
	        	}

	        	pEWEvent->iNumAllocChans = pEWEvent->iNumChans;

	        	for (i = 0; i < pEWEvent->iNumAllocChans; i++)
	        	{
	        		InitEWChan(&pEWEvent->pChanInfo[i]);
	        	}
	        }


            /* Copy over channel specific stuff */
           	for (i = 0; i < pEWEvent->iNumChans; i++)
            {

           		strcpy (pEWEvent->pChanInfo[i].Station.Sta, LocData.phases[i].station);
	            strcpy (pEWEvent->pChanInfo[i].Station.Comp, LocData.phases[i].channel);
	            strcpy (pEWEvent->pChanInfo[i].Station.Net, LocData.phases[i].network);
	            strcpy (pEWEvent->pChanInfo[i].Station.Loc, LocData.phases[i].location);
   
                /* Arrival pick info */
	            pEWEvent->pChanInfo[i].iNumArrivals = 1;
	            pEWEvent->pChanInfo[i].Arrivals[0].cMotion  = LocData.phases[i].polarity;
                pEWEvent->pChanInfo[i].Arrivals[0].dSigma  = LocData.phases[i].quality;
            	
                pEWEvent->pChanInfo[i].Arrivals[0].tCalcPhase  = LocData.phases[i].tPhase; 
	           	strcpy (pEWEvent->pChanInfo[i].Arrivals[0].szCalcPhase, LocData.phases[i].phase_name);
		        pEWEvent->pChanInfo[i].Arrivals[0].tObsPhase  = LocData.phases[i].tPhase;
		        strcpy (pEWEvent->pChanInfo[i].Arrivals[0].szObsPhase, LocData.phases[i].phase_name);

            /* hardcoded weight because it is not supported by global_loc msg.  Assumed all phases 
               are equal weight associated 
             ***********************************************************************/
            pEWEvent->pChanInfo[i].Arrivals[0].dWeight = (float)1.0;

                /* Build the author string */
		        sprintf (pEWEvent->pChanInfo[i].Arrivals[0].szExtSource, "%03d%03d%03d",
							LocData.phases[i].logo.type,
							LocData.phases[i].logo.mod, 
                            LocData.phases[i].logo.instid);

		        sprintf (pEWEvent->pChanInfo[i].Arrivals[0].szExternalPickID, "%ld", LocData.phases[i].sequence);

	            /* Amplitude information */
                
                /*
                 * HACK ALERT:  How do we know how many amps we have for this phase?????
                 */
                pEWEvent->pChanInfo[i].iNumStaMags = 0;
                for (j = 0; j < MAX_AMPS_PER_GLOBALPHASE; j++)
                {
                    ind =  pEWEvent->pChanInfo[i].iNumStaMags;

                    if (LocData.phases[i].amps[j].pick_sequence == LocData.phases[i].sequence)
                    {

                        pEWEvent->pChanInfo[i].Stamags[ind].StaMagUnion.PeakAmp.tAmp1 = 
                                       DTStringToTime (LocData.phases[i].amps[j].amp_time);


                        pEWEvent->pChanInfo[i].Stamags[ind].StaMagUnion.PeakAmp.dAmp1 = 
                                                    (float) LocData.phases[i].amps[j].adcounts;                      

         	       	   	pEWEvent->pChanInfo[i].Stamags[ind].StaMagUnion.PeakAmp.dAmpPeriod1 = 
                                                         (float) LocData.phases[i].amps[j].period;

	                    pEWEvent->pChanInfo[i].Stamags[ind].StaMagUnion.PeakAmp.dAmp2 = (float) MAG_NULL;
	                    pEWEvent->pChanInfo[i].Stamags[ind].StaMagUnion.PeakAmp.tAmp2 = (float) MAG_NULL;
	                    pEWEvent->pChanInfo[i].Stamags[ind].StaMagUnion.PeakAmp.dAmpPeriod2 = (float) MAG_NULL;

                        pEWEvent->pChanInfo[i].Stamags[ind].dMag = 0.0;
                        pEWEvent->pChanInfo[i].Stamags[ind].dWeight = 0.0;
                            
						pEWEvent->pChanInfo[i].Stamags[ind].iMagType = LocData.phases[i].amps[j].amptype;

						pEWEvent->pChanInfo[i].iNumStaMags = pEWEvent->pChanInfo[i].iNumStaMags + 1;

                    } /* is this the correct amp? */

                } /* loop over amp picks */

   			} /* loop over channels */

	      rc = 0;
          break;

     case GLOBAL_MSG_VERSINVALID:
          logit( "e", "GlEvt2EWEvent: global message version invalid.\n%s\n",pGlEvt);
          rc = -1;
          break;

     case GLOBAL_MSG_FORMATERROR:
          logit( "e", "GlEvt2EWEvent: global message format invalid.\n%s\n",pGlEvt);
          rc = -1;
          break;

     default:
          logit( "e", "GlEvt2EWEvent: error %d.\n%s<\n", rc, pGlEvt);
          rc = -1;
          break;
   }

   if (rc < 0)
       return EW_FAILURE;
   else
	return EW_SUCCESS;
}




