/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: ewdb_ora_api_strong_motion.h 7322 2018-04-14 03:27:27Z baker $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2008/12/02 21:37:42  paulf
 *     oracle surgery additions
 *
 *     Revision 1.2  2004/09/09 17:21:10  davidk
 *     Added ewdb_api_Delete* function protototypes.
 *
 *     Revision 1.1  2003/05/20 20:57:10  lucky
 *     Initial revision
 *
 *
 *
 */


/*******************************************************
 *
 * This include file contains only the structure
 * definitions and function prototypes for the strong
 * motion subschema.   
 * 
 * This file should never be included in an application
 * program.  Instead, include the master include file
 * ewdb_ora_api.h
 * 
 *    Lucky Vidmar, May 20, 2003
 *
 *******************************************************/

#ifndef _EWDB_ORA_API_STRONG_MOTION_H
#define _EWDB_ORA_API_STRONG_MOTION_H


/**********************************************************
 #########################################################
    Struct Definition Section
 #########################################################
**********************************************************/


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF

LIBRARY  EWDB_API_LIB

SUB_LIBRARY STRONG_MOTION_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_SMChanAllStruct
TYPE_DEFINITION struct _EWDB_SMChanAllStruct
DESCRIPTION This struct contains information for
a single strong motion message(SMMessage), including
any associated data, such as idEvent, Channel data.
It is used to retrieve SMMessages from the DB.

MEMBER idSMMessage
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION The DB ID of the SMMessage.

MEMBER idSMMessage
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION The DB ID of the Event with which
the message is associated.

MEMBER Station
MEMBER_TYPE EWDB_StationStruct
MEMBER_DESCRIPTION A structure containing information
about the channel from which the message was generated.

MEMBER SMChan
MEMBER_TYPE SM_INFO
MEMBER_DESCRIPTION The strong motion message itself,
in SM_INFO form.

*************************************************
************************************************/
typedef struct _EWDB_SMChanAllStruct
{
  EWDBid idSMMessage;
  EWDBid idEvent;
  EWDB_StationStruct Station;
  SM_INFO SMChan;
} EWDB_SMChanAllStruct;


/**********************************************************
 #########################################################
    Function Prototype Section
 #########################################################
**********************************************************/

/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY STRONG_MOTION_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_PutSMMessage

SOURCE_LOCATION ewdb_api_PutSMMessage.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Warning.  The message was not inserted 
because the DB had no record of the channel or else the
Lat/Lon of the component from which the channel came.  The
channel check is done based on the SCNL from the message.
See the logfile for the contents of the message, and for more
info on why the message was not inserted.

PARAMETER 1
PARAM_NAME pMessage
PARAM_TYPE SM_INFO *
PARAM_DESCRIPTION  A pointer to a RW_STRONGMOTIONII message
(in SM_INFO form), that the caller wishes to insert into the EW DBMS.  

PARAMETER 2
PARAM_NAME idEvent
PARAM_TYPE EWDBid 
PARAM_DESCRIPTION DB Identifier of the event(that already exists 
in the DB)  that this strong motion message should be associated 
with.  The caller should set idEvent to 0 if they do not want 
this message associated with an Event.  

DESCRIPTION This function stores a strong motion message in the
DB and returns the idSMMessage of the new record.  If idEvent is 
a positive number, then the function will attempt to bind the new 
SMMessage to the Event identified by idEvent.  

NOTE Be sure to properly initialize idEvent.  If a seemingly valid 
idEvent is passed, and no such Event exists, the function will fail, 
even though it can create the SMMessage without problems.
If you do not have an idEvent, but have an author and an author's
EventID, then you can use ewdb_api_CreateEvent() to get an idEvent
from an author and author's EventID.

*************************************************
************************************************/
int ewdb_api_PutSMMessage(SM_INFO * pMessage, EWDBid idEvent);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY STRONG_MOTION_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_DeleteSMMessage

SOURCE_LOCATION ewdb_api_DeleteSMMessage.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

PARAMETER 1
PARAM_NAME idSMMessage
PARAM_TYPE EWDBid 
PARAM_DESCRIPTION The DB id of the message that the caller 
wishes to delete.

DESCRIPTION This function dissassociates a strong motion message
with any Events, and then deletes the message.

*************************************************
************************************************/
int ewdb_api_DeleteSMMessage(EWDBid idSMMessage);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY STRONG_MOTION_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetSMData

SOURCE_LOCATION ewdb_api_GetSMData.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Partial Success.  The function successfully
executed, but either 1)the caller's buffer wasn't large enough 
to retrieve all of the messages, and/or 2) there was a problem
retrieving one or more of the messages.  If the value written
to pNumRecordsFound at the completion of the function is greater
than the iBufferRecLen parameter passed to the function, then
more records were found than the caller's buffer could hold.
If the value written to pNumRecordsRetrieved is less than 
the iBufferRecLen parameter passed to the function, then the
function failed to retrieve atleast one of the messages that it found.

PARAMETER 1
PARAM_NAME pcsCriteria
PARAM_TYPE EWDB_CriteriaStruct * 
PARAM_DESCRIPTION  The criteria struct for specifying criteria
types and values for the query to retrieve messages.  
pcsCriteria->Criteria, contains the flags which indicate
which criteria to use in searching for messages.  NOTE: The 
Depth criteria is not supported by this function!  See 
EWDB_CriteriaStruct for more detail on criteria.

PARAMETER 2
PARAM_NAME szSta
PARAM_TYPE char * 
PARAM_DESCRIPTION The Station name criteria for retrieving 
messages.  This param is only used for criteria if 
pcsCriteria->Criteria includes the EWDB_CRITERIA_USE_SCNL
flag.  '*' may be used as a wildcard.  This param must 
always be a valid pointer, the function may fail if it is set to NULL.

PARAMETER 3
PARAM_NAME szComp
PARAM_TYPE char * 
PARAM_DESCRIPTION The Component name criteria for retrieving 
messages.  This param is only used for criteria if 
pcsCriteria->Criteria includes the EWDB_CRITERIA_USE_SCNL
flag.  '*' may be used as a wildcard.  This param must 
always be a valid pointer, the function may fail if it is set to NULL.

PARAMETER 4
PARAM_NAME szNet
PARAM_TYPE char * 
PARAM_DESCRIPTION The Network name criteria for retrieving 
messages.  This param is only used for criteria if 
pcsCriteria->Criteria includes the EWDB_CRITERIA_USE_SCNL
flag.  '*' may be used as a wildcard.  This param must 
always be a valid pointer, the function may fail if it is set to NULL.

PARAMETER 5
PARAM_NAME szLoc
PARAM_TYPE char * 
PARAM_DESCRIPTION The Location name criteria for retrieving 
messages.  This param is only used for criteria if 
pcsCriteria->Criteria includes the EWDB_CRITERIA_USE_SCNL
flag.  '*' may be used as a wildcard.  This param must 
always be a valid pointer, the function may fail if it is set to NULL.

PARAMETER 6
PARAM_NAME idEvent
PARAM_TYPE EWDBid 
PARAM_DESCRIPTION The Event criteria for retrieving 
messages.  This param is only used for criteria if 
pcsCriteria->Criteria includes the EWDB_CRITERIA_USE_IDEVENT
flag.  This param must be set to a valid(existing) idEvent,
if pcsCriteria->Criteria includes the EWDB_CRITERIA_USE_IDEVENT 
flag, or the function will fail.

PARAMETER 7
PARAM_NAME iEventAssocFlag
PARAM_TYPE int 
PARAM_DESCRIPTION  A flag indicating how the function should 
retrieve strong motion messages with respect to event association.
If pcsCriteria->Criteria includes the EWDB_CRITERIA_USE_IDEVENT flag,
then this parameter is ignored;  otherwise, if this parameter is set
to EWDB_SM_SEARCH_FOR_ALL_SMMESSAGES, then all messages meeting the
criteria will be retrieved, otherwise if this param is set to 
EWDB_SM_SEARCH_FOR_ALL_UNASSOCIATED_MESSAGES, then only messages
(meeting the criteria) that are NOT associated with an event, will
be retrieved.

PARAMETER 8
PARAM_NAME pSMI
PARAM_TYPE SM_INFO * 
PARAM_DESCRIPTION  A buffer allocated by the caller, where the
function will place the retrieved messages.  

PARAMETER 9
PARAM_NAME BufferRecLen
PARAM_TYPE int 
PARAM_DESCRIPTION  The length of the buffer(pBuffer) in terms of
SM_INFO records.

PARAMETER 10
PARAM_NAME pNumRecordsFound
PARAM_TYPE int * 
PARAM_DESCRIPTION  A pointer to an int where the function will
write the number of messages found to meet the given criteria.
NOTE:  The number found is not the number retrieved and written
to the caller's buffer.  That is NumRecordsRetrieved.  If the
number of messages found is greater than the caller's buffer will
hold, then the function will return a warning.

PARAMETER 11
PARAM_NAME pNumRecordsRetrieved
PARAM_TYPE int * 
PARAM_DESCRIPTION  A pointer to an int where the function will
write the number of messages actualy retrieved by the function.
This number will be less than or equal to the number found.

DESCRIPTION This function retrieves strong motion messages 
from the DB that meet the criteria given by the caller.  

*************************************************
************************************************/
int ewdb_api_GetSMData(EWDB_CriteriaStruct * pcsCriteria,
                       char * szSta, char * szComp,
                       char * szNet, char * szLoc,
                       EWDBid idEvent, int iEventAssocFlag,
                       SM_INFO * pSMI, int BufferRecLen,
                       int * pNumRecordsFound, int * pNumRecordsRetrieved);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY STRONG_MOTION_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetSMDataWithChannelInfo

SOURCE_LOCATION ewdb_api_GetSMDataWithChannelInfo.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Partial Success.  See ewdb_api_GetSMData().

PARAMETER 1
PARAM_NAME pcsCriteria
PARAM_TYPE EWDB_CriteriaStruct * 

PARAMETER 2
PARAM_NAME szSta
PARAM_TYPE char * 

PARAMETER 3
PARAM_NAME szComp
PARAM_TYPE char * 

PARAMETER 4
PARAM_NAME szNet
PARAM_TYPE char * 

PARAMETER 5
PARAM_NAME szLoc
PARAM_TYPE char * 

PARAMETER 6
PARAM_NAME idEvent
PARAM_TYPE EWDBid 

PARAMETER 7
PARAM_NAME iEventAssocFlag
PARAM_TYPE int 

PARAMETER 8
PARAM_NAME pSMCAS
PARAM_TYPE EWDB_SMChanAllStruct * 
PARAM_DESCRIPTION  A buffer allocated by the caller, where the
function will place the retrieved messages.  

PARAMETER 9
PARAM_NAME BufferRecLen
PARAM_TYPE int 

PARAMETER 10
PARAM_NAME pNumRecordsFound
PARAM_TYPE int * 

PARAMETER 11
PARAM_NAME pNumRecordsRetrieved
PARAM_TYPE int * 

DESCRIPTION This function retrieves strong motion messages and
associated info from the DB, for all messages that meet 
the criteria given by the caller.  This function retrieves the 
strong motion messages and Channel and Event association 
information for each message.

NOTE This function is the same as ewdb_api_GetSMData(), except where
noted above.  Please see ewdb_api_GetSMData() for better documentation
on each of the parameters.

*************************************************
************************************************/
int ewdb_api_GetSMDataWithChannelInfo(EWDB_CriteriaStruct * pcsCriteria,
      char * szSta, char * szComp, char * szNet, char * szLoc,
      EWDBid idEvent, int iEventAssocFlag, EWDB_SMChanAllStruct * pSMCAS, 
      int BufferRecLen, int * pNumRecordsFound, int * pNumRecordsRetrieved);


int ewdb_api_DeleteSMMessagesBeforeTime(int IN_tTime, int IN_bForce);

#endif /* _EWDB_ORA_API_STRONG_MOTION_H */
