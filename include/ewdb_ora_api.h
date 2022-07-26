/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: ewdb_ora_api.h 3471 2008-12-02 21:37:42Z paulf $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2008/12/02 21:37:42  paulf
 *     oracle surgery additions
 *
 *     Revision 1.54  2005/05/12 20:27:09  mark
 *     Added comments constants
 *
 *     Revision 1.53  2005/03/22 17:17:10  davidk
 *     Added constants for SQL max string length, and double-as-string length.
 *
 *     Revision 1.52  2005/03/18 21:57:13  davidk
 *     Changed #define constant for Mw.
 *
 *     Revision 1.51  2005/03/18 00:23:32  davidk
 *     Added #define constant for Mw.
 *
 *     Revision 1.50  2004/11/06 04:21:13  davidk
 *     Added constant for phase-name field size.  (formatted comment included)
 *
 *     Revision 1.49  2004/09/09 17:20:41  davidk
 *     Added DELETE_PARAMS return flags.
 *
 *     Revision 1.48  2004/09/09 05:58:27  davidk
 *     Fixed comments for EWDB_UNASSOCIATED_DATA_* constants.
 *     Added "merge" subdir/schema.
 *
 *     Revision 1.47  2003/08/05 19:46:49  lucky
 *     Removed ioc stuff - it is now under NEIC source tree
 *
 *     Revision 1.46  2003/05/20 20:57:10  lucky
 *     Split up structure definitions and function prototypes into subschema files
 *
 *
 *
 */


/*******************************************************
 *
 * This is now a master include file for all Earthworm
 * API-related definitions.  Within this file are the 
 * #defines and typedefs which are valid throughout
 * the API system.
 *
 * The structure definitions and function prototypes
 * are distributed in the subschema include files, such
 * as ewdb_ora_api_parametric.h.  Those subschema includes
 * are included in this file, so that only this file needs
 * to be included in application programs
 *
 *    Lucky Vidmar, May 20, 2003
 *
 *******************************************************/
 


#ifndef EWDB_ORA_API_H
#define EWDB_ORA_API_H



/**********************************************************
 #########################################################
    INCLUDE Section
 #########################################################
**********************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <rw_strongmotionII.h>



/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW API FORMATTED COMMENT
TYPE LIBRARY

LIBRARY  EWDB_API_LIB

LOCATION THIS_FILE

DESCRIPTION The EWDB_API_LIB is the C API that 
provides access to the Earthworm Database system.
The API provides read and write access to the 
EW DBMS.  It provides a means for application
developers to access the data in the DBMS without
having to learn how to interact with Oracle. 
<br><br>
All access to the Earthworm DBMS must be done via this API. This is
done in order to limit both the code that writes data to the DBMS, and
the amount of code that interacts with the internal structure of the
DBMS.  <br><br> Currently, the only API Available is in C, so to
interact with the DBMS programatically you must have a way of calling C
functions.  <br><br> Unless otherwise specified, all time measurements
are assumed to be seconds since 1/1/1970.  <br><br>

*************************************************
************************************************/


/**********************************************************
 #########################################################
    Constant Section
 #########################################################
**********************************************************/

/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE DEFINE 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY UTILITY

LANGUAGE C

LOCATION THIS_FILE

CONSTANT_GROUP True/False constants

CONSTANT TRUE
VALUE 1

CONSTANT FALSE
VALUE 0

WARNING!!!!!!!!!!
DO NOT CHANGE THESE VALUES.  THEY ARE COPIED FROM
ewdb_cli_base.h!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

*************************************************
************************************************/
#define TRUE 1
#define FALSE 0


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE DEFINE 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY UTILITY

LANGUAGE C

LOCATION THIS_FILE

CONSTANT_GROUP EWDB Return Codes:

CONSTANT EWDB_RETURN_SUCCESS
VALUE 0
DESCRIPTION Value returned by an EWDB function that
executes successfully. 

CONSTANT EWDB_RETURN_FAILURE
VALUE -1
DESCRIPTION Value returned by an EWDB function that
fails.

CONSTANT EWDB_RETURN_WARNING
VALUE 1
DESCRIPTION Value returned by an EWDB function that experiences
non-fatal problems during execution.  Please see individual function
comments for a description of what this code means for each function.

WARNING!!!!!!!!!!
DO NOT CHANGE THESE VALUES.  THEY ARE COPIED FROM
ewdb_cli_base.h!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

*************************************************
************************************************/
#define EWDB_RETURN_SUCCESS 0
#define EWDB_RETURN_FAILURE -1
#define EWDB_RETURN_WARNING 1


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE DEFINE 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY UTILITY

LANGUAGE C

LOCATION THIS_FILE

CONSTANT_GROUP EWDB Debug Levels

CONSTANT EWDB_DEBUG_DB_BASE_NONE
VALUE 0
DESCRIPTION Log NO EWDB debugging information.

CONSTANT EWDB_DEBUG_DB_BASE_CONNECT_INFO
VALUE 1
DESCRIPTION Log DB connect/disconnect information.

CONSTANT EWDB_DEBUG_DB_BASE_STATEMENT_PARSE_INFO
VALUE 2
DESCRIPTION Log SQL statement parsing information.

CONSTANT EWDB_DEBUG_DB_BASE_FUNCTION_ENTRY_INFO
VALUE 4
DESCRIPTION Log entry/exit of DB Call level functions.

CONSTANT EWDB_DEBUG_DB_API_FUNCTION_ENTRY_INFO
VALUE 8
DESCRIPTION Log entry/exit of EWDB API level functions.

CONSTANT EWDB_DEBUG_DB_BASE_ALL
VALUE -1
DESCRIPTION Log All available debugging information for
the EWDB API.

WARNING!!!!!!!!!!
DO NOT CHANGE THESE VALUES.  THEY ARE COPIED FROM
ewdb_cli_base.h!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

*************************************************
************************************************/
#define EWDB_DEBUG_DB_BASE_NONE                  0
#define EWDB_DEBUG_DB_BASE_CONNECT_INFO          1
#define EWDB_DEBUG_DB_BASE_STATEMENT_PARSE_INFO  2
#define EWDB_DEBUG_DB_BASE_FUNCTION_ENTRY_INFO   4
#define EWDB_DEBUG_DB_API_FUNCTION_ENTRY_INFO    8
#define EWDB_DEBUG_DB_BASE_ALL                  -1


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE DEFINE 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY UTILITY

LANGUAGE C

LOCATION THIS_FILE

CONSTANT_GROUP Criteria Selection Flags

Criteria selection flags, used in determining what criteria to use when
selecting a list of events or other objects.

CONSTANT EWDB_CRITERIA_USE_TIME
VALUE 1
DESCRIPTION Flag used to indicate whether or not a time criteria should
be used in selecting a list of objects.

CONSTANT EWDB_CRITERIA_USE_LAT
VALUE 2
DESCRIPTION Flag used to indicate whether or not
a lattitude criteria should be used.

CONSTANT EWDB_CRITERIA_USE_LON
VALUE 4
DESCRIPTION Flag used to indicate whether or not
a longitude criteria should be used.

CONSTANT EWDB_CRITERIA_USE_DEPTH
VALUE 8
DESCRIPTION Flag used to indicate whether or not
a depth criteria should be used.

CONSTANT EWDB_CRITERIA_USE_SCNL
VALUE 0x10
DESCRIPTION Flag used to indicate whether or not
a channel(SCNL) criteria should be used.

CONSTANT EWDB_CRITERIA_USE_IDEVENT
VALUE 0x20
DESCRIPTION Flag used to indicate whether or not
an Event(using idEvent) criteria should be used.

CONSTANT EWDB_CRITERIA_USE_IDCHAN
VALUE 0x40
DESCRIPTION Flag used to indicate whether or not
a Channel(using idChan) criteria should be used.

*************************************************
************************************************/
#define EWDB_CRITERIA_USE_TIME        1
#define EWDB_CRITERIA_USE_LAT         2
#define EWDB_CRITERIA_USE_LON         4
#define EWDB_CRITERIA_USE_DEPTH       8
#define EWDB_CRITERIA_USE_SCNL     0x10
#define EWDB_CRITERIA_USE_IDEVENT  0x20
#define EWDB_CRITERIA_USE_IDCHAN   0x40


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE DEFINE 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY UTILITY

LANGUAGE C

LOCATION THIS_FILE

CONSTANT_GROUP EWDB Time constants

These define the max/min time values and thus the
allowable time range.  Absolute times are seconds since 1970.

CONSTANT EWDB_MAX_TIME
VALUE 9999999999.9999
DESCRIPTION Maximum time value supported for EWDB times.
Times are seconds since 1970.

CONSTANT EWDB_MIN_TIME
VALUE 0
DESCRIPTION Minimum time value supported for EWDB times.
Times are seconds since 1970.

*************************************************
************************************************/
#define EWDB_MAX_TIME 9999999999.9999
#define EWDB_MIN_TIME 0


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE DEFINE 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

CONSTANT_GROUP Parametric Misc. Data Constants

CONSTANT EWDB_MAXIMUM_AMPS_PER_CODA
VALUE 6
DESCRIPTION Maximum number of coda average amplitude values per coda.
6 is derived from the maximum number of coda average amplitude windows
tracked by Earthworm(Pick_EW).

*************************************************
************************************************/
#define EWDB_MAXIMUM_AMPS_PER_CODA 6

/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE DEFINE 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

CONSTANT_GROUP Event Types

ala Quake, Blast, Mudslide?

CONSTANT EWDB_EVENT_TYPE_UNKNOWN
VALUE 0
DESCRIPTION Flag indicating that the type of a DB Event is unknown.

CONSTANT EWDB_EVENT_TYPE_QUAKE
VALUE 2
DESCRIPTION Flag indicating that a DB Event is a Quake

CONSTANT EWDB_EVENT_TYPE_COINCIDENCE
VALUE 3
DESCRIPTION Flag indicating that a DB Event is a Coincidence Trigger

*************************************************
************************************************/
#define EWDB_EVENT_TYPE_COINCIDENCE 3
#define EWDB_EVENT_TYPE_QUAKE 2
#define EWDB_EVENT_TYPE_UNKNOWN 0

/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE DEFINE 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

CONSTANT_GROUP Event Update Types


CONSTANT EWDB_UPDATE_EVENT_NONE
VALUE 0
DESCRIPTION Perform no update.

CONSTANT EWDB_UPDATE_EVENT_EVENTTYPE
VALUE 1
DESCRIPTION Update the EventType of the Event.

CONSTANT EWDB_UPDATE_EVENT_DUBIOCITY
VALUE 2
DESCRIPTION Update the Dubiocity of the Event.

CONSTANT EWDB_UPDATE_EVENT_ARCHIVED
VALUE 4
DESCRIPTION Update the "Archived" status of the Event.

CONSTANT EWDB_UPDATE_EVENT_COMMENT
VALUE 8
DESCRIPTION Update the comment IDs of the Event.

*************************************************
************************************************/
#define EWDB_UPDATE_EVENT_NONE 0
#define EWDB_UPDATE_EVENT_EVENTTYPE 1
#define EWDB_UPDATE_EVENT_DUBIOCITY 2
#define EWDB_UPDATE_EVENT_ARCHIVED  4
#define EWDB_UPDATE_EVENT_COMMENT   8




/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE DEFINE 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

CONSTANT_GROUP Event Update Types


CONSTANT EWDB_UNASSOCIATED_DATA_NONE
VALUE 0
DESCRIPTION Flag to indicate no data.

CONSTANT EWDB_UNASSOCIATED_DATA_PICKS
VALUE 1
DESCRIPTION Flag to indicate pick data.

CONSTANT EWDB_UNASSOCIATED_DATA_WAVEFORMS
VALUE 2
DESCRIPTION Flag to indicate waveform data.

CONSTANT EWDB_UNASSOCIATED_DATA_PEAKAMPS
VALUE 4
DESCRIPTION Flag to indicate Peak Amplitude data.

CONSTANT EWDB_UNASSOCIATED_DATA_SMMESSAGES
VALUE 8
DESCRIPTION Flag to indicate Strong Motion data.

CONSTANT EWDB_UNASSOCIATED_DATA_ALL
VALUE -1
DESCRIPTION Flag to indicate ALL data.

*************************************************
************************************************/
#define EWDB_UNASSOCIATED_DATA_NONE       0
#define EWDB_UNASSOCIATED_DATA_PICKS      1
#define EWDB_UNASSOCIATED_DATA_WAVEFORMS  2
#define EWDB_UNASSOCIATED_DATA_PEAKAMPS   4
#define EWDB_UNASSOCIATED_DATA_SMMESSAGES 8
#define EWDB_UNASSOCIATED_DATA_ALL       -1


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW API FORMATTED COMMENT
TYPE LIBRARY

LIBRARY  EWDB_API_LIB

SUB_LIBRARY WAVEFORM_API

LOCATION THIS_FILE

DESCRIPTION This is the Waveform portion
of the EWDB_API_LIB library.  It provides access to
binary waveform data in the Earthworm DB.

*************************************************
************************************************/

/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE DEFINE 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY WAVEFORM_API

LANGUAGE C

LOCATION THIS_FILE

CONSTANT_GROUP Waveform Data Formats

CONSTANT EWDB_WAVEFORM_FORMAT_UNDEFINED
VALUE 0
DESCRIPTION Indicates that the data format of a waveform
snippet is unknown.

CONSTANT EWDB_WAVEFORM_FORMAT_EW_TRACE_BUF
VALUE 1
DESCRIPTION Indicates that a waveform snippet is of type Earthworm
Trace Buf.  Please see Earthworm documentation of TRACE_BUF messages
for more info.

*************************************************
************************************************/
#define EWDB_WAVEFORM_FORMAT_UNDEFINED 0
#define EWDB_WAVEFORM_FORMAT_EW_TRACE_BUF 1


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE DEFINE 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LANGUAGE C

LOCATION THIS_FILE

CONSTANT_GROUP Poles and Zeroes Constants


CONSTANT EWDB_MAX_POLES_OR_ZEROES
VALUE 100
DESCRIPTION Maximum number of poles or zeroes
allowed for a transfer function

CONSTANT EWDB_PZTYPE_POLE
VALUE 1
DESCRIPTION Flag indicating that a Pole/Zero is a Pole.

CONSTANT EWDB_PZTYPE_ZERO
VALUE 0
DESCRIPTION Flag indicating that a Pole/Zero is a Zero.

*************************************************
************************************************/
#define EWDB_MAX_POLES_OR_ZEROES 100
#define EWDB_PZTYPE_POLE 1
#define EWDB_PZTYPE_ZERO 0


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE DEFINE 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY STRONG_MOTION_API

LANGUAGE C

LOCATION THIS_FILE

CONSTANT_GROUP Strong Motion Message Search Codes

CONSTANT EWDB_SM_SEARCH_UNDEFINED
VALUE 0
DESCRIPTION Code for no type of search defined.  
This is not supported by any functions, and 
exists to prevent 0(the default value) from 
being used as a meaningful code.

CONSTANT EWDB_SM_SEARCH_FOR_ALL_SMMESSAGES
VALUE 1
DESCRIPTION Code that indicates that all strong
motion messages should be retrieved, regardless of
whether they are associated with an event or not.

CONSTANT EWDB_SM_SEARCH_FOR_ALL_UNASSOCIATED_MESSAGES
VALUE 2
DESCRIPTION Code that indicates that only strong
motion messages that are not already associated 
with an event, should be retrieved.

*************************************************
************************************************/
#define EWDB_SM_SEARCH_UNDEFINED                      0
#define EWDB_SM_SEARCH_FOR_ALL_SMMESSAGES             1
#define EWDB_SM_SEARCH_FOR_ALL_UNASSOCIATED_MESSAGES  2


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE DEFINE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

CONSTANT_GROUP Utility constants for the alarm system

CONSTANT EWDB_ALARMS_MAX_RULES_PER_RECIPIENT
VALUE 20
DESCRIPTION Maximum number of rules that a recipient 
can have.

CONSTANT EWDB_ALARMS_MAX_RECIPIENT_DELIVERIES
VALUE 20
DESCRIPTION Maximum number of deliveries that a recipient 
can have.

CONSTANT EWDB_ALARMS_MAX_FORMAT_LEN
VALUE 4000
DESCRIPTION Largest (in bytes) format string.

*************************************************
************************************************/
#define 	EWDB_ALARMS_MAX_RULES_PER_RECIPIENT    	 20
#define 	EWDB_ALARMS_MAX_RECIPIENT_DELIVERIES     20
#define 	EWDB_ALARMS_MAX_FORMAT_LEN 		   	 	4000


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE DEFINE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

CONSTANT_GROUP Delivery Mechanisms

CONSTANT EWDB_ALARMS_DELIVERY_NUM_MECHANISMS
VALUE 5
DESCRIPTION Number of defined delivery mechanisms.

CONSTANT EWDB_ALARMS_DELIVERY_IND_EMAIL
VALUE 0
DESCRIPTION Index of the email delivery mechanism.

CONSTANT EWDB_ALARMS_DELIVERY_IND_PAGER
VALUE 1
DESCRIPTION Index of the pager delivery mechanism.

CONSTANT EWDB_ALARMS_DELIVERY_IND_PHONE
VALUE 2
DESCRIPTION Index of the phone delivery mechanism.

CONSTANT EWDB_ALARMS_DELIVERY_IND_QDDS
VALUE 3
DESCRIPTION Index of the qdds delivery mechanism.

CONSTANT EWDB_ALARMS_DELIVERY_IND_CUSTOM
VALUE 4
DESCRIPTION Index of the custom delivery mechanism.

*************************************************
************************************************/
#define	    EWDB_ALARMS_DELIVERY_NUM_MECHANISMS 5
#define     EWDB_ALARMS_DELIVERY_IND_EMAIL      0
#define     EWDB_ALARMS_DELIVERY_IND_PAGER  		1
#define     EWDB_ALARMS_DELIVERY_IND_PHONE      2
#define     EWDB_ALARMS_DELIVERY_IND_QDDS       3
#define     EWDB_ALARMS_DELIVERY_IND_CUSTOM     4



/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE DEFINE 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY RAW_INFRASTRUCTURE_API

LANGUAGE C

LOCATION THIS_FILE

CONSTANT_GROUP Raw Infrastructure Misc. Data Constants

CONSTANT EWDB_RAW_INFRA_NAME_LEN
VALUE 40
DESCRIPTION Standard name length for raw infrastructure applications,
such as DeviceName, SlotName, and SlotTypeName.

*************************************************
************************************************/
#define EWDB_RAW_INFRA_NAME_LEN 40


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE DEFINE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY WAVEFORM_REQUEST_API

LANGUAGE C

LOCATION THIS_FILE

CONSTANT_GROUP Attempt Param Flags

Attempt Parameters and Scheduling of Snippet-Retrieval Attempts

The concierge system resolves around the concept of having a
background process periodically attempt to retrieve trace from
wave_servers.  There exists within the concierge, a scheduling
system, that schedules requests for retrieval.  The requests
are scheduled based upon a scheduling algorithm, as well as
feedback from the process that is trying to retrieve the data.
These "attempt params" flags, allow the retrieval process
to provide feedback to the scheduling logic that runs within
the database.
There are currently 3 flags:
Flag 1:  Do not update.
This flag says, "I made a retrieval attempt, and for some
reason, I don't want to record that retrieval attempt, 
so just pretend (for scheduling purposes) that there was 
no attempt."  For the most part, this flag will only be used
when a SPECIAL attempt was made, that was not prompted by
the scheduler.  Maybe the retrieval process decided on its
own to attempt to retrieve data, and it doesn't want to interfere
with the official attempt schedule.

Flag 2:  Update and reschedule.
This flag says, "I made an attempt, record that I attempted,
and schedule the next attempt, based upon the scheduling logic."
This would be the standard flag to use when an attempt failed.
It says that you tried, but had no luck, and so it should be 
scheduled for another try.

Flag 3:  Update only the next attempt time.
This flag says, "I made an attempt, but for some reason, I don't
want to record that attempt, and I want to attempt again at
the time I specify."  This flag could be used if you made an attempt
and got a partial response, and you want to attempt again in the
near future, instead of waiting for whenever the next 
scheduling-algorithm based attempt.

CONSTANT EWDB_WAVEFORM_REQUEST_DO_NOT_MODIFY_ATTEMPT_PARAMS
VALUE 0
DESCRIPTION Do not modify attempt params at all

CONSTANT EWDB_WAVEFORM_REQUEST_RECORD_ATTEMPT_AND_UPDATE_PARAMS
VALUE 1
DESCRIPTION Record the current attempt (adjust the number of
attempts remaining), and schedule the next attempt based upon
the scheduling algorithm.

CONSTANT EWDB_WAVEFORM_REQUEST_UPDATE_NEXT_ATTEMPT
VALUE 2
DESCRIPTION Do not record the current attempt.  Schedule the
next attempt at the time given by tNow(calculated by the API
call), plus tAttemptInterval seconds, as specified in the 
SnippetRequestStruct by the application.


*************************************************
************************************************/
#define EWDB_WAVEFORM_REQUEST_DO_NOT_MODIFY_ATTEMPT_PARAMS 0
#define EWDB_WAVEFORM_REQUEST_RECORD_ATTEMPT_AND_UPDATE_PARAMS 1
#define EWDB_WAVEFORM_REQUEST_UPDATE_NEXT_ATTEMPT 2


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE DEFINE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY WAVEFORM_REQUEST_API

LANGUAGE C

LOCATION THIS_FILE

CONSTANT_GROUP Delete Params Result Flags

Param Delete Result Flags

There is a collection of calls used to delete old non-association
data, such as Picks, PeakAmps, Waveforms, SnippetRequests.
The functions do not differentiate between data that was and
wasn't associated, they just delete whatever data they can.
The class of functions return 1 of 4 codes from their underlying
SQL functions:  SUCCES, FAILURE, WARNING_FK, WARNING_TOO_MANY_RECS

CONSTANT EWDB_DELETE_PARAMS_SUCCESS
VALUE 0
DESCRIPTION The function completed successfully, deleting any
records that it found.

CONSTANT EWDB_DELETE_PARAMS_FAILURE
VALUE -1
DESCRIPTION The function failed to complete for an unknown
reason.  No records were deleted.

CONSTANT EWDB_DELETE_PARAMS_WARNING_FK
VALUE 1
DESCRIPTION The function completed, but was not able to delete
some records due to unknown Foreign Key constraint violations.

CONSTANT EWDB_DELETE_PARAMS_WARNING_TOO_MANY
VALUE 2
DESCRIPTION The function completed, but was not able to delete
some records, because there were more records found to delete
than the "maximum records to delete" param that the function
was given.

*************************************************
************************************************/
#define EWDB_DELETE_PARAMS_SUCCESS 0
#define EWDB_DELETE_PARAMS_FAILURE -1
#define EWDB_DELETE_PARAMS_WARNING_FK 1
#define EWDB_DELETE_PARAMS_WARNING_TOO_MANY 2


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE DEFINE 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

CONSTANT_GROUP PhaseName Length Constant

CONSTANT EWDB_PHASENAME_SIZE
VALUE 10
DESCRIPTION Size of PhaseName field.  Maximum 
allowable phasename length length is 1 less than
this value.  So if the size is 6, then "PKIKP" is 
valid(5 chars), but "PKPPKP"(6 chars) is not.

*************************************************
************************************************/
#define EWDB_PHASENAME_SIZE 10


#define EWDB_MAX_SAMPLES_PER_MW_TIMESERIES 285


#define EWDB_MAX_SQL_STRING_LEN 4000
#define EWDB_SQL_DOUBLE_LEN       14

/**********************************************************
 #########################################################
    Typedef & Struct Section
 #########################################################
**********************************************************/


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY UTILITY

LANGUAGE C

LOCATION THIS_FILE


TYPEDEF EWDBid
TYPE_DEFINITION int
DESCRIPTION The C format of a EWDB Database ID.

NOTE Last seen, EWDBids were capable of being larger than an integer,
and this could create problems for the storing of EWDBids in C.  There
is a move to change the type from int to either 64-bit int or string.
DavidK 05/01/00  This should not cause problems with operations on
existing EW5 databases, but in the existing format, it does prevent
existing nodes from coexisting in a tightly coupled manner.

*************************************************
************************************************/
typedef int EWDBid;


/**********************************************************
 #########################################################
    UTILITY 
 #########################################################
**********************************************************/

/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW API FORMATTED COMMENT
TYPE LIBRARY

LIBRARY  EWDB_API_LIB

SUB_LIBRARY UTILITY

LOCATION THIS_FILE

DESCRIPTION This is a portion of the EWDB_API_LIB
that contains constants and functions that are
used by the other portions of the EWDB_API_LIB
library.  It contains basic constants, and also
functions that deal with the setup and teardown
of the API, not with retrieval or insertion of
scientific data.

*************************************************
************************************************/


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY UTILITY

LANGUAGE C

LOCATION THIS_FILE

STABILITY MATURE

FUNCTION ewdb_api_Init

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION The API environment was successfully initialized.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION The API environment failed to initialize.  Please
see stderr or a logfile for details of the failure.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION The API environment was already initialized.  If the
client needs to reinitialize the API environment with different
parameters, they must first call ewdb_api_Shutdown to uninitialize
the existing environment.

PARAMETER 1
PARAM_NAME DBuser
PARAM_TYPE char *
PARAM_DESCRIPTION The database user that the program should run as.

PARAMETER 2
PARAM_NAME DBpassword
PARAM_TYPE char *
PARAM_DESCRIPTION The password for user DBuser.

PARAMETER 3
PARAM_NAME DBservice
PARAM_TYPE char *
PARAM_DESCRIPTION The network serviceID of the database to connect to.
In Oracle environments, the serviceID is usually defined in either a
tnsnames.ora file or by an Oracle Names server.

DESCRIPTION Initialization function for the API.  This function should
be called prior to any other API calls.  It initializes the API
environment and validates the database connection parameters (DBuser,
DBpassword, DBservice).

*************************************************
************************************************/
int ewdb_api_Init(char * DBuser, char * DBpassword, char * DBservice);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY UTILITY

LANGUAGE C

LOCATION THIS_FILE

STABILITY MATURE

FUNCTION ewdb_api_Shutdown

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION The API environment was successfully
de-initialized.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION The API environment failed to de-initialize.  Please
see stderr or a logfile for details of the failure.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION The API environment had not been initialized using
ewdb_api_Init.

DESCRIPTION Shutdown the API environment and free any related memory
and handles.

*************************************************
************************************************/
int ewdb_api_Shutdown(void);



/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY UTILITY

LANGUAGE C

LOCATION THIS_FILE

STABILITY MATURE

FUNCTION ewdb_api_Set_Debug

RETURN_TYPE void

PARAMETER 1
PARAM_NAME IN_iDebug
PARAM_TYPE int
PARAM_DESCRIPTION Flag indicating the level of debug
information that the caller wishes the API to log.
See EWDB_DEBUG_DB_BASE_NONE for information on the 
various debug level constants.  Constants can be 
bitwise OR'd together to form the proper debug level.

DESCRIPTION Set the debug level for the API (and
supporting) functions.

*************************************************
************************************************/
void ewdb_api_Set_Debug(int IN_iDebug);




/* Subschema include files -- NOTE:  ORDER MATTERS!!  */
#include <ewdb_ora_api_infra.h>
#include <ewdb_ora_api_parametric.h>
#include <ewdb_ora_api_alarms.h>
#include <ewdb_ora_api_external.h>
#include <ewdb_ora_api_misc.h>
#include <ewdb_ora_api_raw_infra.h>
#include <ewdb_ora_api_strong_motion.h>
#include <ewdb_ora_api_waveform.h>
#include <ewdb_ora_api_merge.h>

#endif   /* undef EWDB_ORA_API_H */
