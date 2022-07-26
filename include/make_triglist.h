/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: make_triglist.h 1111 2002-11-03 00:22:11Z lombard $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.2  2002/11/03 00:12:41  lombard
 *     Protected from multiple includes.
 *
 *     Revision 1.1  2001/07/01 22:01:48  davidk
 *     Initial revision
 *
 *
 *
 ************************************************************/
#ifndef MAKE_TRIGLIST_H
#define MAKE_TRIGLIST_H

/* DK Added prototypes for make_triglist library functions  06/27/2001 */
/* Functions in make_triglist.c */
int writetrig_init( char* trigFileBase, char* outputDir );
/******************************************************************
 * writetrig_init(): 
 *
 * trigFileBase: The base filename used for each trigger file. 
 *
 * outputDir:    The directory used for each trigger file.
 * so the trigger filenames will look something like:
 * <outputDir>/<trigFileBase>.trg_<Date>
 *
 * Function initializes the trigger writing environment.
 ******************************************************************/

int writetrig( char *note, char* filename, char* outDir );
/******************************************************************
 * writetrig(): 
 *
 * note:     The text of the trigger that is written out to the
 * trigger file.
 *
 * filename:  Passed to writetrig_init() as trigFileBase if the
 * environment has not yet been initialized.  See writetrig_init()
 * for a description. 
 *
 * outDir:   Passed to writetrig_init() as outputDir if the
 * environment has not yet been initialized.  See writetrig_init()
 * for a description. 
 *
 * Function writes information to a trigger file.  The function will
 * initialize the trigger writing environment, if it is not already
 * initialized.
 ******************************************************************/

void writetrig_close();
/******************************************************************
 * writetrig_close(): 
 * 
 * Function shuts down the trigger writing environment.(closes
 * file ptr)
 ******************************************************************/


void bldtrig_head( char* trigmsg, double otime, char* evId, char* author);
/******************************************************************
 * bldtrig_head(): 
 *
 * trigmsg:  Trigger message EVENT line created by the function.
 *
 * otime:    Origin time. (seconds since 1600)
 *
 * evId:     EVENT ID that the author gave to the event.
 *
 * author:   Author of the event.
 *
 * Function builds the EVENT line of a trigger message   
 * Modified for author id by alex 
 ******************************************************************/



void bldtrig_phs(char *trigmsg, char* sta, char* comp, char* net, 
                 char ph, double pickTime, double saveStart, double durSave);
/******************************************************************
 * bldtrig_phs(): 
 *
 * trigmsg:  Trigger message PHASE line created by the function.
 *
 * sta:      SEEDesque Station code of the channel for which this 
 * line is being built.
 *
 * comp:     SEEDesque Component code of the channel for which this 
 * line is being built.
 *
 * net:      SEEDesque Network code of the channel for which this 
 * line is being built.
 *
 * ph:       One character phase label.
 *
 * pickTime: Pick time of the phase.  (Seconds since 1600)
 *
 * saveStart:Time that data should begin to be saved. 
 * (Seconds since 1600)
 *
 * durSave:  Seconds of data to save starting at saveStart.
 *
 * Function builds the "phase" lines of a trigger message.  One 
 * line per call.
 *
 * DavidK 06/28/2001  Does anyone even use this function.  Arc2trig
 * and Arc2trigII each have their own.
 ******************************************************************/

char *make_datestr(double t, char *datestr);
/*********************************************************************
 * make_datestr()  takes a time in seconds since 1600 and converts   *
 *                 it into a character string in the form of:        *
 *                   "19880123 12:34:12.21"                          *
 *                 It returns a pointer to the new character string  *
 *                                                                   *
 *    NOTE: this requires an output buffer >=21 characters long      *
 *                                                                   *
 *  Y2K compliance:                                                  *
 *     date format changed to YYYYMMDD                               *
 *     date15() changed to date17()                                  *
 *                                                                   *
 *********************************************************************/

/* End of Functions in make_triglist.c */

#endif
