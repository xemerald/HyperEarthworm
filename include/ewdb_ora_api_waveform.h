/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: ewdb_ora_api_waveform.h 7322 2018-04-14 03:27:27Z baker $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2008/12/02 21:37:42  paulf
 *     oracle surgery additions
 *
 *     Revision 1.3  2005/04/25 16:56:01  davidk
 *     Removed references to EWDB_SnippetStruct and ws_clientII.h, because
 *     they were causing problems with the use of ws_clientIIx.h, and
 *     EWDB_SnippetStruct is not in use within the EWDB API.
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
 * definitions and function prototypes for the waveform
 * subschema.   
 * 
 * This file should never be included in an application
 * program.  Instead, include the master include file
 * ewdb_ora_api.h
 * 
 *    Lucky Vidmar, May 20, 2003
 *
 *******************************************************/

#ifndef _EWDB_ORA_API_WAVEFORM_H
#define _EWDB_ORA_API_WAVEFORM_H


/* Needed for TRACE_REQ structure */
#include <ws_clientII.h>


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

SUB_LIBRARY WAVEFORM_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_WaveformStruct
TYPE_DEFINITION struct _EWDB_WaveformStruct
DESCRIPTION Structure containing waveform information and a raw
waveform snippet.

MEMBER idWaveform
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION DB ID of the Waveform snippet.

MEMBER idChan
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of the channel that this pick came from.

MEMBER tStart
MEMBER_TYPE double
MEMBER_DESCRIPTION Beginning time of the waveform snippet.  Expressed
as seconds since 1970.

MEMBER tEnd
MEMBER_TYPE double
MEMBER_DESCRIPTION Ending time of the waveform snippet.  Expressed as
seconds since 1970.

MEMBER iDataFormat
MEMBER_TYPE int
MEMBER_DESCRIPTION Format of the actual waveform snippet data.  See
EWDB_WAVEFORM_FORMAT_UNDEFINED.

MEMBER iByteLen
MEMBER_TYPE int
MEMBER_DESCRIPTION Length of the actual waveform snippet data in bytes.

MEMBER binSnippet
MEMBER_TYPE char *
MEMBER_DESCRIPTION Actual waveform snippet.  Its length is specified by
iByteLen and its format is specified by iDataFormat.  This is just a
pointer.  Space for the waveform snippet must be dynamically allocated.

NOTE There should probably also be an iStorageMethod value that
indicates how the data is stored (in the DB, out of the DB as a file,
at an ftp site, etc.), but it has yet to be implemented.

*************************************************
************************************************/
typedef struct _EWDB_WaveformStruct
{
  EWDBid idWaveform;
  EWDBid idChan;
  double tStart;
  double tEnd;
  int    iDataFormat;
  int    iByteLen;
  char * binSnippet;
} EWDB_WaveformStruct;



/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF

LIBRARY  EWDB_API_LIB

SUB_LIBRARY WAVEFORM_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_SnippetRequestStruct
TYPE_DEFINITION struct _EWDB_SnippetRequestStruct
DESCRIPTION Structure for storing parameters for a waveform request.
This structure is used to store (in the database) a request for
waveform data.  Requests are presumably later gathered by a concierge
type program that recovers as many desired waveforms as possible.  This
structure describes a time interval of interest for which it is
interested in data from a particular channel.  It defines the channel
whose data it is interested in, the time interval for which it wants
data, how many attempts should be made to retrieve the data, how often
each attempt should be made, and what DB idEvent if any that the data
should be associated with when retrieved.

MEMBER idSnipReq
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION DB ID of the waveform request(SnipReq) that this
structure's data is associated with.

MEMBER idChan
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION DB ID of the sesimic data channel for which the
request wants data.

MEMBER tStart
MEMBER_TYPE double
MEMBER_DESCRIPTION Starting time of the desired time interval.

MEMBER tEnd
MEMBER_TYPE double
MEMBER_DESCRIPTION Ending time of the desired time interval.

MEMBER idEvent
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION DB ID of the Event with which the desired data
should be associated.

MEMBER iNumAttempts
MEMBER_TYPE int
MEMBER_DESCRIPTION Number of attempts that should be made to retrieve
the desired data.

MEMBER tNextAttempt
MEMBER_TYPE time_t
MEMBER_DESCRIPTION This field has a dual purpose.  When retrieving a
list of Snippet Requests from the DB, this field will contain the
time that the current snippet request is scheduled for a retrieval
attempt.  When used in the reverse manner (updating a snippet request),
the "data retriever"/scheduler should put the "time till next attempt"
in this field, meaning the time that the scheduler wants the request
to be attempted minus the current time.  (The deltaT).   The number
is done as a delta to ensure that updated requests are not scheduled
for the past.

MEMBER idExistingWaveform
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION DB ID of a Waveform in the DB that contains partial
data for the original request.

MEMBER tInitialRequest
MEMBER_TYPE time_t
MEMBER_DESCRIPTION Time of the initial snippet request.  That is the
time that it is created in the DB.  Seconds since 1970.

MEMBER iRequestGroup
MEMBER_TYPE int
MEMBER_DESCRIPTION Request Group of the snippet request.  Requests can
be placed in a Request Group so that different requests can be handled
differently, or can be handled by separate data retrievers(like
ora_trace_fetch).

MEMBER iNumAlreadyAtmptd
MEMBER_TYPE int
MEMBER_DESCRIPTION Number of times that a data retriever has already
attempted to retrieve the subject of the request.  It is up to the
retriever to update this number.  This number field is used for
informational purposes.  It is up to the data retriever to delete
the snippet request when it feels it has been attempted enough
times.  Theoretically, a request could be attempted 100 times, even
if iNumAttempts was only set to 5, since it is up to the retriever
to delete the request.

MEMBER iRetCode
MEMBER_TYPE int
MEMBER_DESCRIPTION This is the return code that the retriever gives
to the request after it attempts to process it.  There are currently
no generally agreed upon return codes.

MEMBER iLockTime
MEMBER_TYPE int
MEMBER_DESCRIPTION The Lock value associated with the snippet request.
In order to provide a mechanism for concurrent data retrievers,
snippet requests must be locked before being processed.  iLockTime
is the key to the Lock for the snippet request.  After the request
has been processed, the key must be used to access the locked
snippet request, so that the request can be updated.

MEMBER szNote
MEMBER_TYPE char[100]
MEMBER_DESCRIPTION This is a note that the retriever may include
with the snippet request.  It is free to put whatever it likes
in this field, such as its name, strategy params, a log, etc.

MEMBER ComponentInfo
MEMBER_TYPE EWDB_StationStruct
MEMBER_DESCRIPTION Station/Component information for the channel
associated with this snippet request.

*************************************************
************************************************/
typedef struct _EWDB_SnippetRequestStruct
{
  EWDBid idSnipReq;
  EWDBid idChan;
  double tStart;
  double tEnd;
  EWDBid idEvent;
  int    iNumAttempts;
  time_t tNextAttempt;
  EWDBid idExistingWaveform;
  time_t tInitialRequest;
  int    iRequestGroup;
  int    iNumAlreadyAtmptd;
  int    iRetCode;
  int    iLockTime;
  char   szNote[100];
  EWDB_StationStruct ComponentInfo;
} EWDB_SnippetRequestStruct;




/**********************************************************
 #########################################################
    Function Prototype Section
 #########################################################
**********************************************************/


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
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY WAVEFORM_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetWaveformDesc

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

PARAMETER 1
PARAM_NAME pWaveformDesc
PARAM_TYPE EWDB_WaveformStruct
PARAM_DESCRIPTION Structure containing information about the waveform 
descriptor.  The function will read pWaveformDesc->idWaveform, and 
attempt to retrieve information about that descriptor.  Upon successfull 
completion, the function will write the retrieved information into the
pWaveformDesc struct.  The structure pointed to by pWaveformDesc must
be allocated by the client.

DESCRIPTION Retrieve information about the waveform descriptor pointed
to by pWaveformDesc->idWaveform from the database.

*************************************************
************************************************/
int ewdb_api_GetWaveformDesc(EWDB_WaveformStruct * pWaveformDesc);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY WAVEFORM_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetWaveformListByEvent

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

PARAMETER 1
PARAM_NAME IN_idEvent
PARAM_TYPE EWDBid
PARAM_DESCRIPTION Database ID of the Event whose waveform
snippets the caller is interested in.

PARAMETER 2
PARAM_NAME pWaveformBuffer
PARAM_TYPE EWDB_WaveformStruct
PARAM_DESCRIPTION Pointer to a  caller allocated buffer of waveform structs
which will be filled by this function.

PARAMETER 3
PARAM_NAME pStationBuffer
PARAM_TYPE EWDB_WaveformStruct
PARAM_DESCRIPTION Pointer to a  caller allocated buffer of station structs
which will be filled by this function.  NOTE:  This buffer is only filled
if bIncludeStationInfo is set to TRUE.  This param can be set to NULL
if bIncludeStationInfo is FALSE.

PARAMETER 4
PARAM_NAME bIncludeStationInfo
PARAM_TYPE int
PARAM_DESCRIPTION Flag(TRUE/FALSE) indicating whether the function should
retrieve station/component information for each waveform found.

PARAMETER 5
PARAM_NAME pWaveformDescsFound
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of waveforms found.

PARAMETER 6
PARAM_NAME pWaveformDescsRetrieved
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of waveform descriptors placed in the caller's 
buffer(pWaveformBuffer).

PARAMETER 7
PARAM_NAME BufferRecLen
PARAM_TYPE int
PARAM_DESCRIPTION Size of the pWaveformBuffer buffer as a multiple of
EWDB_WaveformStruct. (example: 15 structs)  The pStationBuffer is also
assumed to be of atleast this size if bIncludeStationInfo is set to TRUE.


DESCRIPTION The function retrieves a list of waveforms that are
associated with a given event.  See EWDB_WaveformStruct for a
description of the information retrieved for each associated waveform.
If bIncludeStationInfo is set to TRUE, then the function also retrieves
station/component information for each waveform and writes it into 
the pStationBuffer buffer, in matching order with the waveforms.

NOTE This function only retrieves the waveform DESCRIPTORS, it does not
actually retrieve the binary waveform.

*************************************************
************************************************/
int ewdb_api_GetWaveformListByEvent(EWDBid idEvent, 
        EWDB_WaveformStruct * pWaveformBuffer, EWDB_StationStruct * pStationBuffer,
        int bIncludeStationInfo, int * pWaveformDescsFound,
        int * pWaveformDescsRetrieved, int BufferRecLen);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY WAVEFORM_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY MATURE

FUNCTION ewdb_api_GetWaveformSnippet

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION The Actual Length of the snippet retrieved
was 0 bytes. (empty snippet)

PARAMETER 1
PARAM_NAME IN_idWaveform
PARAM_TYPE EWDBid
PARAM_DESCRIPTION Database ID of the waveform descriptor whose
snippet the caller is interested in.

PARAMETER 2
PARAM_NAME IN_pWaveform
PARAM_TYPE char *
PARAM_DESCRIPTION Buffer allocated (prior to calling this function)
by the caller to hold the retrieved snippet.  The function writes the
retrieved snippet to the buffer.

PARAMETER 3
PARAM_NAME IN_WaveformLength
PARAM_TYPE int
PARAM_DESCRIPTION Length of the IN_pWaveform buffer.

DESCRIPTION Retrieves the snippet associated with IN_idWaveform and
returns it in the IN_pWaveform buffer.

*************************************************
************************************************/
int ewdb_api_GetWaveformSnippet(EWDBid IN_idWaveform, char * IN_pWaveform,
                                 int IN_WaveformLength);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY WAVEFORM_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY MATURE

FUNCTION ewdb_api_CreateWaveform

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

PARAMETER 1
PARAM_NAME pWaveform
PARAM_TYPE EWDB_WaveformStruct *
PARAM_DESCRIPTION Pointer to an EWDB_WaveformStruct filled by the caller
with both waveform descriptor information and the actual binary waveform.

PARAMETER 2
PARAM_NAME idEvent
PARAM_TYPE EWDBid
PARAM_DESCRIPTION DB ID of an existing Event in the database that the
caller wishes to associate with the new Waveform.

DESCRIPTION  Function inserts a waveform into the database, including
both descriptor information and the actual binary snippet.  Optionally
(if idEvent is not 0), the function will associate the new Waveform
with the existing DB Event identified by idEvent.

*************************************************
************************************************/
int ewdb_api_CreateWaveform(EWDB_WaveformStruct * pWaveform, EWDBid idEvent);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY WAVEFORM_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY NEW

FUNCTION ewdb_api_UpdateWaveform

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

PARAMETER 1
PARAM_NAME pWaveform
PARAM_TYPE EWDB_WaveformStruct *
PARAM_DESCRIPTION Pointer to an EWDB_WaveformStruct filled by the caller
with both waveform descriptor information and the actual binary waveform.

DESCRIPTION  Function updates an existing waveform in the database, including
both descriptor information and the actual binary snippet.

*************************************************
************************************************/
int ewdb_api_UpdateWaveform(EWDB_WaveformStruct * pWaveform);




/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY WAVEFORM_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_DeleteSnippetRequest

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

PARAMETER 1
PARAM_NAME IN_idSnipReq
PARAM_TYPE EWDBid
PARAM_DESCRIPTION DB ID of an existing SnippetRequest which the
caller wishes to delete.

PARAMETER 2
PARAM_NAME IN_iLockTime
PARAM_TYPE int
PARAM_DESCRIPTION The lock value of the given request.  This
should be the time the Request was locked.  This value can 
be obtained via the EWDB_SnippetRequestStruct that was used
in the call to lock the request.  If the request is not locked,
then this value is ignored.


DESCRIPTION The Function deletes the snippet request and any
supporting information from the database.

*************************************************
************************************************/
int ewdb_api_DeleteSnippetRequest (EWDBid IN_idSnipReq, int IN_iLockTime);




/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY WAVEFORM_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetListOfOldSnipReqs

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Partial success.  A list of snippet requests was retrieved,
but the caller's buffer was not large enough to accomadate all of the
requests found.  See pNumItemsFound for the number of arrivals found
and pNumItemsRetrieved for the number of requests placed in the
caller's buffer.


PARAMETER 1
PARAM_NAME pBuffer
PARAM_TYPE EWDB_SnippetRequestStruct *
PARAM_DESCRIPTION Buffer allocated by the caller, where the function
will write the list of snippet requests that are active as of tThreshold.

PARAMETER 2
PARAM_NAME iLockTimeThreshold
PARAM_TYPE int
PARAM_DESCRIPTION Threshold time for determining whether snippet requests
are old or not.  If the lock time of the request is earlier than 
iLockTimeThreshold, then the request is considered old. 
(Expressed as seconds since 1970)

 
PARAMETER 3
PARAM_NAME pNumItemsFound
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of snippet reuqests found.

PARAMETER 4
PARAM_NAME pNumItemsRetrieved
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of snippet requests placed in the caller's buffer(pBuffer).

PARAMETER 5
PARAM_NAME BufferLen
PARAM_TYPE int
PARAM_DESCRIPTION Size of the pBuffer buffer as a multiple of
EWDB_SnippetRequestStruct. (example: 15 structs)

DESCRIPTION THIS FUNCTION IS NOT FOR GENERAL CONSUMPTION.  The function 
retrieves a list of snippet requests with OLD locks.  It is for use
by programs auditing the operation of the concierge system and should
not be used in the mainstream processing of data.

NOTE  THIS FUNCTION IS NOT FOR GENERAL CONSUMPTION.  It is for auditing
an problem detection. The function retrieves only the SCNL portion of the
EWDB_SnippetRequestStruct.ComponentInfo.


*************************************************
************************************************/
int ewdb_api_GetListOfOldSnipReqs(EWDB_SnippetRequestStruct * pBuffer,
      int iLockTimeThreshold, int * pNumItemsFound, 
      int * pNumItemsRetrieved, int BufferLen);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY WAVEFORM_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY NEW

FUNCTION ewdb_api_GetSnippetRequestList

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Partial success.  A list of snippet requests was retrieved,
but the caller's buffer was not large enough to accomadate all of the
requests found.  See pNumItemsFound for the number of arrivals found
and pNumItemsRetrieved for the number of requests placed in the
caller's buffer.

PARAMETER 1
PARAM_NAME pBuffer
PARAM_TYPE EWDB_SnippetRequestStruct *
PARAM_DESCRIPTION Buffer allocated by the caller, where the function
will write the list of snippet requests that are active as of tThreshold.
If you want to retrieve a list of request, then you MUST initialize 
pBuffer[0].idSnipReq to 0.  Otherwise, the function will only attempt to 
retrieve the one SnippetRequest identified by pBuffer[0].idSnipReq 
regardless of the other parameters.

PARAMETER 2
PARAM_NAME tThreshold
PARAM_TYPE time_t
PARAM_DESCRIPTION Threshold time for determining whether snippet requests
are active or not.  If the next scheduled attempt for retrieving a 
snippet requests is earlier than tThreshold, then the request is 
considered active.  Only active requests are returned, in increasing order 
of next scheduled attempt.  (Expressed as seconds since 1970)

PARAMETER 3
PARAM_NAME iRequestGroup
PARAM_TYPE int
PARAM_DESCRIPTION Request group for which the caller is interested
in snippet requests.  Set this to -1 to get all Snippet Requests regardless
of Request Group.  See EWDB_SnippetRequestStruct for a description of RequestGroup.

PARAMETER 4
PARAM_NAME pNumItemsFound
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of snippet reuqests found.

PARAMETER 5
PARAM_NAME pNumItemsRetrieved
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of snippet requests placed in the caller's buffer(pBuffer).

PARAMETER 6
PARAM_NAME BufferLen
PARAM_TYPE int
PARAM_DESCRIPTION Size of the pBuffer buffer as a multiple of
EWDB_SnippetRequestStruct. (example: 15 structs)

DESCRIPTION The function retrieves the list of active snippet 
requests (as of time tThreshold) from the DB.  A Snippet Request
is deemed active if its next scheduled retrieval attempt is 
prior to tThreshold.  Requests are returned in order of 
increasing time of next retrieval attempt, so the most overdue
ones should be at the top of the list.  Note that only the snippets
for the given RequestGroup are returned, unless the RequestGroup
is set to a special value.
<p>
The function retrieves only the SCNL portion of the
EWDB_SnippetRequestStruct.ComponentInfo.
<p>
The function can be used to retrieve a single SnippetRequest by
idSnipReq.  See the (PARAMETER 1 pBuffer) documentation for more info.

NOTE  THIS CALL LOCKS THE SnippetRequests THAT IT RETRIEVES.
You MUST UNLOCK these requests using ewdb_api_UpdateSnippetRequest()
or ewdb_api_DeleteSnippetRequest() after you have finished with them.
*************************************************
************************************************/
int ewdb_api_GetSnippetRequestList(EWDB_SnippetRequestStruct * pBuffer,
      time_t tThreshold, int iRequestGroup,
      int * pNumItemsFound, int * pNumItemsRetrieved, int BufferLen);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY WAVEFORM_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY NEW

FUNCTION ewdb_api_UpdateSnippetRequest

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

PARAMETER 1
PARAM_NAME IN_pSnipReq
PARAM_TYPE EWDB_SnippetRequestStruct *
PARAM_DESCRIPTION Pointer to a EWDB_SnippetRequestStruct, where the
caller puts the parameters of the SnippetRequest that they wish to update.

PARAMETER 2
PARAM_NAME IN_bModifyAttemptParams
PARAM_TYPE int
PARAM_DESCRIPTION TRUE/FALSE  flag indicating whether the attempt should
be recorded.  If true, the DB will increment the counter that tracks
how many times the request has been attempted. (VERY SIMPLE)

PARAMETER 3
PARAM_NAME IN_bModifyResultParams
PARAM_TYPE int
PARAM_DESCRIPTION TRUE/FALSE flag indicating whether or not any new
snippet data was found, and thus whether the snippetrequest should 
be updated (new tStart or tEnd, idExistingWaveform).

DESCRIPTION The function updates a snippet request.  This function
acts as a recording mechanism for the request processor.  It allows
the processor to record that it tried to retrieve a snippet, and it
either got part or none of the desired snippet.  <br>
The function always updates the tNextAttempt time in the database 
with the value of IN_pSnipReq->tNextAttempt.<br>
If IN_bModifyAttemptParams is true, then the DB updates the counter
that track the number of attempts so far. <br>
If IN_bModifyResultParams is true, then the DB will update tStart,
tEnd, and idExistingWaveform if appropriate.

NOTE  Set IN_pSnipReq->idExistingWaveform to 0 if a partial snippet was 
not retrieved.  If a partial snippet was retrieved, first call 
ewdb_api_CreateWaveform() to store the snippet in the database, 
then call this function using the idWaveform that you got back 
from the create call.

*************************************************
************************************************/
int ewdb_api_UpdateSnippetRequest(EWDB_SnippetRequestStruct * IN_pSnipReq,  
      int IN_bModifyAttemptParams, int IN_bModifyResultParams);



/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY WAVEFORM_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY NEW

FUNCTION ewdb_api_UpdateSnippetRequestControlParams

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

PARAMETER 1
PARAM_NAME IN_pSnipReq
PARAM_TYPE EWDB_SnippetRequestStruct *
PARAM_DESCRIPTION Pointer to a EWDB_SnippetRequestStruct, where the
caller puts the parameters of the SnippetRequest that they wish to update.

DESCRIPTION The function updates the control parameters for a snippet request.  
At this time, the control params are:<br>
 iNumAttempts(the number of times the request will attempt to be fulfilled), and<br>
 iRequestGroup(the request group in which the request has been categorized).<br>
 
NOTE IN_pSnipReq->iLockTime must match the LockTime for the request (and the request
must be locked).  (You must lock the request before you can modify the control params
and you must remember to unlock it afterwards.)

*************************************************
************************************************/
int ewdb_api_UpdateSnippetRequestControlParams(EWDB_SnippetRequestStruct * IN_pSnipReq);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY WAVEFORM_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_ProcessSnippetReqs

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

PARAMETER 1
PARAM_NAME szSta
PARAM_TYPE char *
PARAM_DESCRIPTION SEEDlike Station code of the channels for which
the caller wishes to create snippet requests.  <br>
This param CANNOT be NULL or blank.
Use the wildcard string "*" to match all possible Station codes.

PARAMETER 2
PARAM_NAME szComp
PARAM_TYPE char *
PARAM_DESCRIPTION SEEDlike Component code of the channels for which
the caller wishes to create snippet requests.  <br>
This param CANNOT be NULL or blank.
Use the wildcard string "*" to match all possible Component codes.

PARAMETER 3
PARAM_NAME szNet
PARAM_TYPE char *
PARAM_DESCRIPTION SEEDlike Network code of the channels for which
the caller wishes to create snippet requests.  <br>
This param CANNOT be NULL or blank.
Use the wildcard string "*" to match all possible Network codes.

PARAMETER 4
PARAM_NAME szLoc
PARAM_TYPE char *
PARAM_DESCRIPTION SEEDlike Location code of the channels for which
the caller wishes to create snippet requests.  <br>
This param CANNOT be NULL.  It may be blank("").
Use the wildcard string "*" to match all possible Location codes.

PARAMETER 5
PARAM_NAME tStart
PARAM_TYPE double
PARAM_DESCRIPTION The start of the time interval for which
the caller wants a snippet for the given SCNL.
Expressed as seconds since 1970.

PARAMETER 6
PARAM_NAME tEnd
PARAM_TYPE double
PARAM_DESCRIPTION The end of the time interval for which
the caller wants a snippet for the given SCNL.
Expressed as seconds since 1970.

PARAMETER 7
PARAM_NAME idEvent
PARAM_TYPE EWDBid
PARAM_DESCRIPTION The EWDB event identifer that the
any resulting snippets should be associated with.

PARAMETER 8
PARAM_NAME iNumAttempts
PARAM_TYPE int
PARAM_DESCRIPTION The number of times that the caller 
wishes to have the request attempted.  It is up to the
data retriever to enforce/interpret this number.

PARAMETER 9
PARAM_NAME iRequestGroup
PARAM_TYPE int
PARAM_DESCRIPTION The Request Group that the caller wishes
the request associated with.  Requests may be divided into 
groups for more efficient retrieval.  This param may be later
modified/ignored by the data retriever.

DESCRIPTION The Function creates a Snippet Request record in the DB, for
the given time range, for each channel that matches the given SCNL codes.
The record describes snippet data that the caller wants stored in the DB.
(This call does not retrieve snippet data, it only records the request.)
A snippet is essentially a piece of data from a given channel for a given
time interval.  If idEvent is specified as non 0, then any snippets
recovered as a result of the requests will be associated
the given idEvent.

NOTE  This function does not retrieve any waveform data!
It only creates requests for waveform data.  The requests
must be handled by another program(concierge).

*************************************************
************************************************/
int ewdb_api_ProcessSnippetReqs(char * szSta,char * szComp, 
       char * szNet, char * szLoc, double tStart, double tEnd, EWDBid idEvent,
                                int iNumAttempts, int iRequestGroup);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY WAVEFORM_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY NEW

FUNCTION ewdb_api_GetNextScheduledSnippetRetrievalAttempt

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION There were no pending snippet requests in the DB.

PARAMETER 1
PARAM_NAME ptNextAttempt
PARAM_TYPE time_t *
PARAM_DESCRIPTION Pointer to a time_t variable supplied by the caller
where the function will write the next scheduled attempt time of all
the pending snippet requests in the DB.  (assuming that the function
executes successfully and returns EWDB_RETURN_SUCCESS)

DESCRIPTION The function checks the snippet requests in the database
to see when the next scheduled attempt is among all of the pending
requests.  The requests are schedule by the DB to be attempted at
intervals.  The function looks up the first scheduled attempt time.

*************************************************
************************************************/
int ewdb_api_GetNextScheduledSnippetRetrievalAttempt(time_t * ptNextAttempt);


int ewdb_api_DeleteWaveformsBeforeTime(int IN_tTime, int IN_iMaxRecsToDelete, 
                                       int * pSQLRetCode);

#endif /* _EWDB_ORA_API_WAVEFORM_H */
