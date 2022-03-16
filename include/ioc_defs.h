/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: ioc_defs.h 2068 2006-01-30 19:41:11Z friberg $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2006/01/30 19:41:10  friberg
 *     added in hydra includes for raypicker module
 *
 *     Revision 1.2  2005/12/17 22:37:58  davidk
 *     moved non ioc-specific definitions to hydra_defs.h
 *
 *     Revision 1.1  2005/11/16 18:43:52  davidk
 *     Split former ioc_defs.h up into two parts:
 *     common/hydra_defs.h which includes hydra common definitions
 *     and
 *     resp/ioc_defs.h which includes only neic resp definitions.
 *
 *
 *
 */


#ifndef IOC_DEFS_H
#define IOC_DEFS_H

#include <hydra_defs.h>



/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE DEFINE 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY IOC_API

LANGUAGE C

LOCATION THIS_FILE

CONSTANT_GROUP EVENT_QUALITY_FLAG constants

CONSTANT EVENT_QUALITY_FLAG_AUTOMATIC_DUBIOUS
VALUE 1
DESCRIPTION Flag used by an automatic module to indicate
that it believes an EWDB Event is bogus.

CONSTANT EVENT_QUALITY_FLAG_AUTOMATIC_VALID
VALUE 0
DESCRIPTION Flag used by an automatic module to indicate
that it believes an EWDB Event is valid

CONSTANT EVENT_QUALITY_FLAG_HUMAN_DUBIOUS
VALUE -1
DESCRIPTION Flag used by a human analyst to indicate
that it believes an EWDB Event is bogus.

CONSTANT EVENT_QUALITY_FLAG_HUMAN_VALID
VALUE 0
DESCRIPTION Flag used by a human analyst to indicate
that it believes an EWDB Event is valid

*************************************************
************************************************/
#define   EVENT_QUALITY_FLAG_AUTOMATIC_DUBIOUS  1
#define   EVENT_QUALITY_FLAG_AUTOMATIC_VALID    0
#define   EVENT_QUALITY_FLAG_HUMAN_DUBIOUS     -1
#define   EVENT_QUALITY_FLAG_HUMAN_VALID        0



#define		STATE_DO_LOCATE  "From Glass"
#define		STATE_FROM_GLASS  STATE_DO_LOCATE
#define		STATE_DO_ML			"Ready for ML"
#define		STATE_DO_MB			"Ready for MB"
#define		STATE_DO_MBLG		"Ready for MBLG"
#define		STATE_DO_MS			"Ready for MS"
#define		STATE_DO_MD			"Ready for MD"
#define		STATE_DO_MWP		"Ready for MWP"
#define		STATE_DO_ALARMS	"Ready for notifications"
#define		STATE_CHECK_FIN "Determine Automatic processing status"
#define		STATE_PROCESS_LOCATED_ORIGIN  "Process Located Origin"
#define		STATE_FROM_LOC		            STATE_PROCESS_LOCATED_ORIGIN
#define		STATE_DO_RAYPELLETS	          "Ready for RayPellets"
#define		STATE_DO_MW 	              	"Ready for MW"
#define		STATE_DETERMINE_MW_CALC	    	"Checking Mw Criteria"
#define		STATE_DO_ASSOCIATE_NEW		    "Associate Picks With Event"
#define		STATE_DO_RAYPELLETS_FORCE		"Force RayPellet"

/*******************
* #define		STATE_DO_LOCATE  "From Glass"
* #define		STATE_FROM_GLASS  STATE_DO_LOCATE
* #define		STATE_LOC_PROB		"Problem with Locator"
* 
* #define		STATE_ML_PROB		"Problem with ML Computation"
* 
* #define		STATE_MB_PROB		"Problem with MB Computation"
* 
* #define		STATE_MBLG_PROB		"Problem with MBLG Computation"
* 
* #define		STATE_MS_PROB		"Problem with MS Computation"
* 
* #define		STATE_MD_PROB		"Problem with MD Computation"
* 
* #define		STATE_MWP_PROB		"Problem with MWP Computation"
* 
* #define		STATE_ALARMS_PROB	"Problem with notifications"
* 
* #define		STATE_AUTO_PROC_DONE "Automatic processing completed"
*************/


/*
 * Status notification messages  
 *
 *   STORY:  We (will) have displays that automatically update themselves
 *    based on the goings-on in the database.  For example, new origins
 *    will be shown in the list of events, as well as in the active locator
 *    display.
 *
 *    To support these displays, the state manager (and conceivably other
 *    modules in the future) will write UDP messages to a private network
 *    where various displays (and potentially other state managers and modules)
 *    can listen to those messages and act accordingly.
 *
 *    The following are the "blessed" messages.  NOTE:  the message text
 *    must be a single word in order to make token parsing easier.
 *
 */

//#define	MAX_NOTIFY_MSG_LEN			4096

#define	STATUS_MSG_COUNT			15

#define	STAMSG_UNDEFINED			0
#define	STAMSG_NEW_GLASS_ORIGIN		1
#define	STAMSG_NEW_LOC_ORIGIN		2
#define	STAMSG_ML_DONE		 		3
#define	STAMSG_MB_DONE		 		4
#define	STAMSG_MBLG_DONE		 	5
#define	STAMSG_MS_DONE		 		6
#define	STAMSG_MWP_DONE		 		7
#define	STAMSG_ALARMS_DONE	 		8
#define STAMSG_ERROR            	9
#define STAMSG_AUTO_PROC_DONE      	10
#define STAMSG_RAYPELLETS_DONE		11
#define STAMSG_MW_DONE				12
#define STAMSG_MD_DONE				13
#define STAMSG_RAYPELLET_FORCED		14

static char* StatusMsgs[STATUS_MSG_COUNT] =
{
   "Unknown"    		   		/* 0  */
 , "NewGlassOrigin"    	   		/* 1  */
 , "NewLocatorOrigin"      		/* 2  */
 , "MlDoneForOrigin"       		/* 3  */
 , "MbDoneForOrigin"       		/* 4  */
 , "MblgDoneForOrigin"     		/* 5  */
 , "MsDoneForOrigin"       		/* 6  */
 , "MwpDoneForOrigin"      		/* 7  */
 , "AlarmsDoneForOrigin"   		/* 8  */
 , "Error!"                		/* 9  */
 , "AutomaticProcessingDone"   	/* 10 */
 , "RayPelletsDone"				/* 11 */
 , "MwDoneForOrigin"			/* 12 */
 , "MdDoneForOrigin"			/* 13 */
 , "RayPelletForcedDone"		/* 14 */
};

#endif /* IOC_DEFS_H */

