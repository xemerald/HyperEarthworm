/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: ewdb_ora_api_infra.h 7322 2018-04-14 03:27:27Z baker $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2008/12/02 21:37:42  paulf
 *     oracle surgery additions
 *
 *     Revision 1.9  2004/11/18 22:57:36  davidk
 *     Added ewdb_api_GetSelectedPickableStations, a function that retrieves only
 *     stations that have been previously picked(ever) from the DB.
 *
 *     Revision 1.8  2004/07/19 17:45:55  davidk
 *     Added szComment field (char *) to EWDB_ChannelStruct.
 *
 *     Revision 1.7  2004/04/06 17:52:16  davidk
 *     Added ewdb_api_CreateOrUpdateChanParams().
 *
 *     Revision 1.6  2004/03/17 20:37:02  davidk
 *     fixed a couple of html comments.
 *
 *     Revision 1.5  2003/12/04 19:43:18  davidk
 *     Fixed a syntax error (added missing semicolon to func prototype).
 *
 *     Revision 1.4  2003/12/04 19:36:41  davidk
 *     Added lots of function prototypes(needed for metadata tool) and LOTS of comments.
 *
 *     Revision 1.2  2003/08/19 21:23:58  davidk
 *     Added documentation for ewdb_api_GetSelectedStations_w_Response().
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
 * definitions and function prototypes for the cooked
 * infrastructure subschema.   
 * 
 * This file should never be included in an application
 * program.  Instead, include the master include file
 * ewdb_ora_api.h
 * 
 *    Lucky Vidmar, May 20, 2003
 *
 *******************************************************/

#ifndef _EWDB_ORA_API_INFRA_H
#define _EWDB_ORA_API_INFRA_H



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

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_StationStruct
TYPE_DEFINITION struct _EWDB_StationStruct
DESCRIPTION Structure that provides information on seismic station components.
Specifically it describes the name, location, and orientation of the
sensor component for a channel.
ComponentStruct would have been a better name than StationStruct
since there may be multiple components at a Site/Station, and
each component may have a different location/orientation.

MEMBER idChan
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION The Database ID of the Channel.

MEMBER idComp
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION The Database ID of the Component.

MEMBER Sta
MEMBER_TYPE char[10]
MEMBER_DESCRIPTION The Station code of the SCNL for the Component.

MEMBER Comp
MEMBER_TYPE char[10]
MEMBER_DESCRIPTION The Channel/Component code of the SCNL for the Component.

MEMBER Net
MEMBER_TYPE char[10]
MEMBER_DESCRIPTION The Network code of the SCNL for the Component.

MEMBER Loc
MEMBER_TYPE char[10]
MEMBER_DESCRIPTION The Location code of the SCNL for the Component.

MEMBER Lat
MEMBER_TYPE float
MEMBER_DESCRIPTION The lattitude of the Component.  Expressed in Degrees.

MEMBER Lon
MEMBER_TYPE float
MEMBER_DESCRIPTION The longitude of the Component.  Expressed in Degrees.

MEMBER Elev
MEMBER_TYPE float
MEMBER_DESCRIPTION The elevation of the Component above sea level.
Expressed in meters.

MEMBER Azm
MEMBER_TYPE float
MEMBER_DESCRIPTION The horizontal orientation of the Component.
Expressed in degrees clockwise of North.

MEMBER Dip
MEMBER_TYPE float
MEMBER_DESCRIPTION The vertical orientation of the Component.
Expressed in degrees below horizontal.

*************************************************
************************************************/
typedef struct _EWDB_StationStruct
{
  EWDBid  idChan;
  EWDBid  idComp;
  char    Sta[10];
  char    Comp[10];
  char    Net[10];
  char    Loc[10];
  float   Lat;
  float   Lon;
  float   Elev;
  float   Azm;
  float   Dip;
} EWDB_StationStruct;



/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_PZNum
TYPE_DEFINITION struct _EWDB_PZNum
DESCRIPTION Describes a Pole/Zero as a complex number.

MEMBER dReal
MEMBER_TYPE double
MEMBER_DESCRIPTION Real portion of this number.

MEMBER dImag
MEMBER_TYPE double
MEMBER_DESCRIPTION Imaginary portion of this number.

*************************************************
************************************************/
typedef struct _EWDB_PZNum
{
  double      dReal;
  double      dImag;
} EWDB_PZNum;


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_TransformFunctionStruct
TYPE_DEFINITION struct _EWDB_TransformFunctionStruct
DESCRIPTION Structure for describing Poles and Zeros based
transform/transfer functions.

MEMBER idCookedTF
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION DB ID of the transfer function.

MEMBER szCookedTFDesc
MEMBER_TYPE char[50]
MEMBER_DESCRIPTION Function name or description.

MEMBER iNumPoles
MEMBER_TYPE int
MEMBER_DESCRIPTION Number of Poles in the function.

MEMBER iNumZeroes
MEMBER_TYPE int
MEMBER_DESCRIPTION Number of Zeroes in the function.

MEMBER Poles
MEMBER_TYPE EWDB_PZNum [EWDB_MAX_POLES_OR_ZEROES]
MEMBER_DESCRIPTION Poles of the function.

MEMBER Zeroes
MEMBER_TYPE EWDB_PZNum [EWDB_MAX_POLES_OR_ZEROES]
MEMBER_DESCRIPTION Zeroes of the function.

*************************************************
************************************************/
typedef struct _EWDB_TransformFunctionStruct
{
  EWDBid      idCookedTF;
  char        szCookedTFDesc[50];
  int         iNumPoles;
  int         iNumZeroes;
  EWDB_PZNum  Poles[EWDB_MAX_POLES_OR_ZEROES];
  EWDB_PZNum  Zeroes[EWDB_MAX_POLES_OR_ZEROES];
} EWDB_TransformFunctionStruct;


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_ChanTCTFStruct
TYPE_DEFINITION struct _EWDB_ChanTCTFStruct
DESCRIPTION Structure for storing the Poles/Zeroes transfer function
for a channel/time range, including gain and samplerate.

MEMBER idChanT
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION DB ID of the channel validity range(ChanT) for which
the associated transform function is valid.

MEMBER dGain
MEMBER_TYPE double
MEMBER_DESCRIPTION Scalar gain of the channel for the ChanT.

MEMBER dSampRate
MEMBER_TYPE double
MEMBER_DESCRIPTION Digital sample rate of the waveforms exiting the
channel for the ChanT.

MEMBER tfsFunc
MEMBER_TYPE EWDB_TransformFunctionStruct
MEMBER_DESCRIPTION Poles and Zeroes transform function for the channel
time interval.

*************************************************
************************************************/
typedef struct _EWDB_ChanTCTFStruct
{
  EWDBid    idChanT;
  double    dGain;
  double    dSampRate;
  EWDB_TransformFunctionStruct tfsFunc;
} EWDB_ChanTCTFStruct;



/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_ChannelStruct
TYPE_DEFINITION struct _EWDB_ChannelStruct
DESCRIPTION Structure for storing the comprehensive information for
a Site, Component, and/or Channel. (for a single time interval).
EWDB_ChannelStruct represents a unified collection of information
available for a channel.

MEMBER Comp
MEMBER_TYPE EWDB_StationStruct
MEMBER_DESCRIPTION StationStruct that describes properties of the
Channel/Component/Site for the structure.

MEMBER idSite
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION DB ID of the Site for the structure.

MEMBER idSiteT
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION DB ID of the Site time interval(SiteT) for the
structure.

MEMBER idCompT
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION DB ID of the Component time interval(CompT) for the 
structure.

MEMBER idChanT
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION DB ID of the channel time interval(ChanT) for 
the structure.  (Redundant to pChanProps->idChanT)

MEMBER tOn
MEMBER_TYPE double
MEMBER_DESCRIPTION Beginning of the time range for the structure.

MEMBER tOff
MEMBER_TYPE double
MEMBER_DESCRIPTION End of the time range for the structure.

MEMBER tOnCompT
MEMBER_TYPE double
MEMBER_DESCRIPTION If the structure contains information for a 
Channel time interval(ChanT), then the time interval for the 
Channel is stored in tOn/tOff, and the time interval for the
associated Component is stored in tOnCompT/tOffCompT.
tOnComp is the beginning of the time range for the Component
associated with the Channel for the structure.

MEMBER tOffCompT
MEMBER_TYPE double
MEMBER_DESCRIPTION End of the time range for the Component
associated with the Channel for the structure.

MEMBER pChanProps
MEMBER_TYPE EWDB_ChanTCTFStruct *
MEMBER_DESCRIPTION Pointer to an optional EWDB_ChanTCTFStruct that
contains response information for the channel of the structure.

*************************************************
************************************************/
typedef struct _EWDB_ChannelStruct
{
  EWDB_StationStruct Comp;
  EWDBid             idSite;
  EWDBid             idSiteT;
  EWDBid             idCompT;
  EWDBid             idChanT;  /* this is redundant to pChanProps->idChanT */
  double             tOn;
  double             tOff;
  double             tOnCompT;
  double             tOffCompT;
  char              *szComment;
  EWDB_ChanTCTFStruct * pChanProps;
} EWDB_ChannelStruct;



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

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LOCATION THIS_FILE

DESCRIPTION This is the Cooked Infrastructure portion
of the EWDB_API_LIB library.  It provides access to
station/component name, location and orientation,
and response information for each channel in the
Earthworm DB.

*************************************************
************************************************/


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY NEW

FUNCTION ewdb_api_AssociateChanWithComp 

SOURCE_LOCATION src/oracle/schema/src/infra/ewdb_api_AssociateChanWithComp.c

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

PARAMETER 1
PARAM_NAME idComp
PARAM_TYPE EWDBid
PARAM_DESCRIPTION The DB identifier of the component(Comp) record the
caller wants to associate with a channel.

PARAMETER 2
PARAM_NAME idChan
PARAM_TYPE EWDBid
PARAM_DESCRIPTION The DB identifier of the channel(Chan) record the
caller wants to associate with a component.

PARAMETER 3
PARAM_NAME tStart
PARAM_TYPE double
PARAM_DESCRIPTION The start of the time interval for which the caller
wants to associate the component and channel.  Expressed as seconds
since 1970.

PARAMETER 4
PARAM_NAME tEnd
PARAM_TYPE double
PARAM_DESCRIPTION The end of the time interval for which the caller
wants to associate the component and channel.  Expressed as seconds
since 1970.

PARAMETER 5
PARAM_NAME pidCompT
PARAM_TYPE EWDBid *
PARAM_DESCRIPTION A pointer to a EWDBid where the function will write
information if the call fails.

PARAMETER 6
PARAM_NAME pidChanT
PARAM_TYPE EWDBid *
PARAM_DESCRIPTION A pointer to a EWDBid where the function will write
information if the call fails.

DESCRIPTION Function creates an association between a channel and a
component for a specified time interval.  The function can fail if a
relationship exists between the specified component and an alternative
channel, or between the specified channel and an alternative component,
during an overlapping time period.
<br><br>
<b>NOTE THIS FUNCTION IS UNSUPPORTED.<br>
Please contact someone in
the Earthworm DB development group if you need to set specific
component/channel time based relationships.  <br>
ewdb_internal_SetChanParams()
is also available(and not recommended for use).</b>
<br><br>
The relationship between a component and a channel is complex to
implement.  I believe the issues that cause the pidCompT and pidChanT
variables to be written to, have been cleared up, and they are no
longer applicable;  however, I am not sure of this, so please be sure
to pass valid pointers until better documentation can be made
available.  davidk 2000/05/04. <br> 
Davidk 07/27/2001
<br><br>

NOTE The following is believed to be out
of date, but is the best information available at this time: 
<br><br>
A channel can be associated with at most one component at a given time.
If the channel that the caller is trying to bind a component to is
already bound to another component during an overlapping time interval,
then the call will fail, and the idComp of the first component found
that has an overlapping association with that channel will be
returned.  Currently, if there are any existing chan relationships
during the time period described by tStart,tEnd, for the given idChan,
then the idChanT of the first ChanT relationship record will be
returned in pidChanT along with an error condition. <br>
Davidk 07/27/2001

*************************************************
************************************************/
int ewdb_api_AssociateChanWithComp(EWDBid idComp, EWDBid idChan, 
                                   double tStart, double tEnd, 
                                   EWDBid * pidCompT, EWDBid * pidChanT);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LANGUAGE C

LOCATION THIS_FILE

STATUS EXPERIMENTAL

FUNCTION ewdb_api_CreateChannel 

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

PARAMETER 1
PARAM_NAME pidChan
PARAM_TYPE EWDBid *
PARAM_DESCRIPTION A pointer to a EWDBid where the
function will write The DB identifier of the Channel
it created.

PARAMETER 2
PARAM_NAME szComment
PARAM_TYPE char *
PARAM_DESCRIPTION Optional comment regarding the component.  It may be
blank, and should be a maximum of 4k characters.  It may not be NULL!

DESCRIPTION Function creates a channel in the DB. A channel identifies
the entire path of a seismic signal from ground motion to completed
waveform.  The function writes the DB ID of the newly created channel
to pidChan.  Because most of a channel's properties are variable over
time, this function does not take much as input, as it only creates a
channel with its fixed attribute(s).  This function should be used to
generate a new idChan that can be used in conjunction with other
functions for creating components and setting time based attributes of
components and channels.

NOTE <b>THIS FUNCTION IS UNSUPPORTED!</b><br>  A channel(Chan) represents a unique 
data path from ground motion to finished waveform.  It is associated with 
a Component sensor, and all of the signal altering devices that the 
sensor's signal passes through before becoming a finished set of waveforms.

*************************************************
************************************************/
int ewdb_api_CreateChannel(EWDBid * pidChan, char * szComment);



/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_CreateCompTForSCNLT

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

PARAMETER 1
PARAM_NAME pidChan
PARAM_TYPE EWDBid *
PARAM_DESCRIPTION Pointer to an EWDBid where the function will
write the idChan of the newly created channel, associated with the SCNL.

PARAMETER 2
PARAM_NAME szSta
PARAM_TYPE char *
PARAM_DESCRIPTION The Station Code of the SCNL.

PARAMETER 3
PARAM_NAME szComp
PARAM_TYPE char *
PARAM_DESCRIPTION The Component(Channel) Code of the SCNL.

PARAMETER 4
PARAM_NAME szNet
PARAM_TYPE char *
PARAM_DESCRIPTION The Network Code of the SCNL.

PARAMETER 5
PARAM_NAME szLoc
PARAM_TYPE char *
PARAM_DESCRIPTION The Location Code of the SCNL.

PARAMETER 6
PARAM_NAME tStart
PARAM_TYPE double
PARAM_DESCRIPTION The start of the validity time range for the given SCNL.
Seconds since 1970.

PARAMETER 7
PARAM_NAME tEnd
PARAM_TYPE double
PARAM_DESCRIPTION The end of the validity time range for the given SCNL.
Seconds since 1970.

DESCRIPTION This function is misnamed.  It actually creates an association
between an SCNL and a channel for a given time period.  It is called 
CreateCompTForSCNLT() because it creates a component time interval(CompT)
record as part of the process of creating a component and a channel, and
associating the two together for a given time period.  The function returns
the idChan of the newly created channel with which the SCNL has been
associated for the supplied time interval.  The DB ID of the component(Comp)
and the component time interval(CompT) are not returned.  The caller can
retrieve the idComp and idCompT by calling ewdb_api_GetComponentInfo() with
the idChan of the newly created channel, and tStart or tEnd.  The caller can
change the location/orientation of the component by calling 
ewdb_api_SetCompParams() after calling ewdb_api_CreateCompTForSCNLT().

*************************************************
************************************************/
int ewdb_api_CreateCompTForSCNLT(EWDBid * pidChan, char * szSta, 
                                 char * szComp, char * szNet, char * szLoc, 
                                 double tStart, double tEnd);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_CreateComponent

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

PARAMETER 1
PARAM_NAME pidComp
PARAM_TYPE EWDBid *
PARAM_DESCRIPTION A pointer to a EWDBid where the function will write
The DB identifier of the Component it created.

PARAMETER 2
PARAM_NAME sSta
PARAM_TYPE char *
PARAM_DESCRIPTION SEEDlike Station code for the component.  It may not
be blank or NULL, and should be a maximum of 7 characters.

PARAMETER 3
PARAM_NAME sComp
PARAM_TYPE char *
PARAM_DESCRIPTION SEEDlike Component code for the component.  The field
may not be blank or NULL and should be a maximum of 9 characters.

PARAMETER 4
PARAM_NAME sNet
PARAM_TYPE char *
PARAM_DESCRIPTION SEEDlike Network code for the component.  It may not
be blank or NULL and should be a maximum of 9 characters.

PARAMETER 5
PARAM_NAME sLoc
PARAM_TYPE char *
PARAM_DESCRIPTION Optional SEEDlike Location code for the component. It
may not be blank or NULL and should be a maximum of 9 characters.

PARAMETER 6
PARAM_NAME szComment
PARAM_TYPE char *
PARAM_DESCRIPTION Optional comment regarding the component.  May be
blank, and should be a maximum of 4k characters.  It may not be NULL!

DESCRIPTION Function creates a component sensor in the DB with an SCN
and optionally L.  It associates the component with a site identified
by (Sta,Net).  It returns the DB ID of the newly created component.  If
a component already exists with the specified SCNL, then a new
component is not created and the idComp of the existing component is
returned.

NOTE A component(Comp) represents a seismic sensor in the field.  It is
represented by SCN and optionaly L, and has location
properties(Lat,Lon, and Elev) and orientation properties(Azimuth and
Dip).  
<br><br>
If a component with matching SCNL already exists, then the call will
succeed and return the idComp of the existing component.  A new
component will not be created!
<br><br>
The comment is currently ignored.

*************************************************
************************************************/
int ewdb_api_CreateComponent(EWDBid *pidComp, char * sSta, char * sComp,
                             char * sNet, char * sLoc, char * szComment);



/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_CreateTransformFunction 

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

PARAMETER 1
PARAM_NAME pidCookedTF
PARAM_TYPE EWDBid *
PARAM_DESCRIPTION Pointer to an EWDBid where the function will write the
DB ID of the newly created poles and zeroes based cooked transform
function(CookedTF).

PARAMETER 2
PARAM_NAME pCookedTF
PARAM_TYPE EWDB_TransformFunctionStruct *
PARAM_DESCRIPTION Pointer to a caller filled
EWDB_TransformFunctionStruct that contains the transform function and
associated information that the caller wants to insert into the DB.

DESCRIPTION Function inserts a caller provided poles/zeroes based
transform function into the DB.  The transform Function is used to
transform recorded waveforms back into ground motion.  The function
writes the DB id of the newly created transform function to pidCookedTF.

NOTE The transform function can be associated with multipe channel time
intervals using EWDB_SetTransformFuncForChanT().

*************************************************
************************************************/
int ewdb_api_CreateTransformFunction(EWDBid * pidCookedTF,
                                     EWDB_TransformFunctionStruct * pCookedTF);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetComponentInfo

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

PARAMETER 1
PARAM_NAME idChan
PARAM_TYPE EWDBid
PARAM_DESCRIPTION The DB identifier of the channel for which the caller
wants information.

PARAMETER 2
PARAM_NAME tParamsTime
PARAM_TYPE int
PARAM_DESCRIPTION Point in time for which the caller wants component
parameter information.  A time is required because some parameters are
variable over time.

PARAMETER 3
PARAM_NAME pStation
PARAM_TYPE EWDB_StationStruct *
PARAM_DESCRIPTION A pointer to a EWDB_StationStruct (allocated by the caller)
where the function will write the component parameter information for 
the given channel and time.

DESCRIPTION Function retrieves a EWDB_StationStruct full of information
about the component associated with a channel at a give time.

*************************************************
************************************************/
int ewdb_api_GetComponentInfo(EWDBid idChan, int tParamsTime,
                              EWDB_StationStruct * pStation);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetStationList

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Returned when the caller passes an invalid set of
criteria.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Partial success.  A list of stations was retrieved,
but the caller's buffer was not large enough to accomadate all of the
stations found.  See pNumStationsFound for the number of stations found
and pNumStationsRetrieved for the number of stations placed in the
caller's buffer.

PARAMETER 1
PARAM_NAME MinLat
PARAM_TYPE double
PARAM_DESCRIPTION Minimum lattitude coordinate of stations
to be retrieved. Lattitude is expressed in degrees.

PARAMETER 2
PARAM_NAME MaxLat
PARAM_TYPE double
PARAM_DESCRIPTION Maximum lattitude coordinate of stations to be
retrieved.  Latitude is expressed in degrees.

PARAMETER 3
PARAM_NAME MinLon
PARAM_TYPE double
PARAM_DESCRIPTION Minimum longitude coordinate of stations to be
retrieved.  Longitude is expressed in degrees.

PARAMETER 4
PARAM_NAME MaxLon
PARAM_TYPE double
PARAM_DESCRIPTION Maximum longitude coordinate of stations to be
retrieved.  Longitude is expressed in degrees.

PARAMETER 5
PARAM_NAME ReqTime
PARAM_TYPE double
PARAM_DESCRIPTION Time of interest.  The list of active components
changes over time.  Reqtime identifies the historical point in time for
which the caller wants a station list.  Time is expressed in seconds
since 1970.

PARAMETER 6
PARAM_NAME pBuffer
PARAM_TYPE EWDB_StationStruct *
PARAM_DESCRIPTION Buffer allocated by the caller, in which the function
will place information for each of the stations matching the caller's 
criteria.

PARAMETER 7
PARAM_NAME pNumStationsFound
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of stations found to match the calller's criteria.

PARAMETER 8
PARAM_NAME pNumStationsRetrieved
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of arrivals placed in the callers buffer(pBuffer).

PARAMETER 9
PARAM_NAME BufferLen
PARAM_TYPE int
PARAM_DESCRIPTION Size of the pBuffer buffer as a multiple of
EWDB_StationStruct. (example: 15 structs)

DESCRIPTION This function retrieves a list of component/channels given
a lat/lon box and time of interest.  Component attributes for the given
time are included.  (MinLat,MinLon) and (MaxLat,MaxLon)
define two vertices of a box.  The function retrieves the list of
stations within the box.  

NOTE Be careful of the effects of negative Lat/Lon on Min/Max!!
<br><br>
This function excludes all stations that have either a Lat or Lon = 0.
Use ewdb_api_GetStationListWithZeroLatLon() to include those stations.
Use ewdb_api_GetStationListWithoutLocation() to retrieve all 
stations available in the database at a given time.  It does not
impose a lat/lon box.

*************************************************
************************************************/
int ewdb_api_GetStationList(double MinLat, double MaxLat, double MinLon,
       double MaxLon, double ReqTime, EWDB_StationStruct * pBuffer,
       int * pNumStationsFound, int * pNumStationsRetrieved, int BufferLen);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetStationList

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Returned when the caller passes an invalid set of
criteria.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Partial success.  A list of stations was retrieved,
but the caller's buffer was not large enough to accomadate all of the
stations found.  See pNumStationsFound for the number of stations found
and pNumStationsRetrieved for the number of stations placed in the
caller's buffer.

PARAMETER 1
PARAM_NAME MinLat
PARAM_TYPE double
PARAM_DESCRIPTION Minimum lattitude coordinate of stations
to be retrieved. Lattitude is expressed in degrees.

PARAMETER 2
PARAM_NAME MaxLat
PARAM_TYPE double
PARAM_DESCRIPTION Maximum lattitude coordinate of stations to be
retrieved.  Latitude is expressed in degrees.

PARAMETER 3
PARAM_NAME MinLon
PARAM_TYPE double
PARAM_DESCRIPTION Minimum longitude coordinate of stations to be
retrieved.  Longitude is expressed in degrees.

PARAMETER 4
PARAM_NAME MaxLon
PARAM_TYPE double
PARAM_DESCRIPTION Maximum longitude coordinate of stations to be
retrieved.  Longitude is expressed in degrees.

PARAMETER 5
PARAM_NAME ReqTime
PARAM_TYPE double
PARAM_DESCRIPTION Time of interest.  The list of active components
changes over time.  Reqtime identifies the historical point in time for
which the caller wants a station list.  Time is expressed in seconds
since 1970.

PARAMETER 6
PARAM_NAME pBuffer
PARAM_TYPE EWDB_StationStruct *
PARAM_DESCRIPTION Buffer allocated by the caller, in which the function
will place information for each of the stations matching the caller's 
criteria.

PARAMETER 7
PARAM_NAME pNumStationsFound
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of stations found to match the calller's criteria.

PARAMETER 8
PARAM_NAME pNumStationsRetrieved
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of arrivals placed in the callers buffer(pBuffer).

PARAMETER 9
PARAM_NAME BufferLen
PARAM_TYPE int
PARAM_DESCRIPTION Size of the pBuffer buffer as a multiple of
EWDB_StationStruct. (example: 15 structs)

DESCRIPTION This function retrieves a list of component/channels given
a lat/lon box and time of interest.  Component attributes for the given
time are included.  (MinLat,MinLon) and (MaxLat,MaxLon)
define two vertices of a box.  The function retrieves the list of
stations within the box.  

NOTE Be careful of the effects of negative Lat/Lon on Min/Max!!
<br><br>
This function includes stations(if within the lat/lon box) that 
have Lat=Lon = 0, meaning their location is undefined.  Use
ewdb_api_GetStationList() to exclude those stations.
Use ewdb_api_GetStationListWithoutLocation() to retrieve all 
stations available in the database at a given time.  It does not
impose a lat/lon box.

*************************************************
************************************************/
int ewdb_api_GetStationListWithZeroLatLon(double MinLat, double MaxLat, double MinLon,
       double MaxLon, double ReqTime, EWDB_StationStruct * pBuffer,
       int * pNumStationsFound, int * pNumStationsRetrieved, int BufferLen);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetStationListWithoutLocation

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Returned when the caller passes an invalid set of
criteria.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Partial success.  A list of stations was retrieved,
but the caller's buffer was not large enough to accomadate all of the
stations found.  See pNumStationsFound for the number of stations found
and pNumStationsRetrieved for the number of stations placed in the
caller's buffer.

PARAMETER 1
PARAM_NAME ReqTime
PARAM_TYPE double
PARAM_DESCRIPTION Time of interest.  The list of active components
changes over time.  Reqtime identifies the historical point in time for
which the caller wants a station list.  Time is expressed in seconds since 1970.

PARAMETER 2
PARAM_NAME pBuffer
PARAM_TYPE EWDB_StationStruct *
PARAM_DESCRIPTION Buffer allocated by the caller, in which the function
will place information for each of the stations matching the caller's criteria.

PARAMETER 3
PARAM_NAME pNumStationsFound
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of stations found to match the calller's criteria.

PARAMETER 4
PARAM_NAME pNumStationsRetrieved
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of arrivals placed in the callers buffer(pBuffer).

PARAMETER 5
PARAM_NAME BufferLen
PARAM_TYPE int
PARAM_DESCRIPTION Size of the pBuffer buffer as a multiple of
EWDB_StationStruct. (example: 15 structs)

DESCRIPTION This function retrieves a list of component/channels given
a time of interest.  This function is the same as ewdb_api_GetStationList(),
except that it does not impose a Lat/Lon box.

*************************************************
************************************************/
int ewdb_api_GetStationListWithoutLocation(double ReqTime, 
      EWDB_StationStruct * pBuffer, int * pNumStationsFound, 
      int * pNumStationsRetrieved, int BufferLen);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetidChanT 

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

PARAMETER 1
PARAM_NAME idChan
PARAM_TYPE EWDBid
PARAM_DESCRIPTION The DB identifier of the channel for which the caller
wants a channel time interval(ChanT) identifier.

PARAMETER 2
PARAM_NAME tTime
PARAM_TYPE double
PARAM_DESCRIPTION Point in time for which the caller wants a time
interval identifier for the channel.

PARAMETER 3
PARAM_NAME pidChanT
PARAM_TYPE EWDBid *
PARAM_DESCRIPTION A pointer to a EWDBid where the function will write
The DB identifier of the requested channel time interval(ChanT).

DESCRIPTION Function retrieves the identifier of the channel time
interval associated with the given channel and time.

*************************************************
************************************************/
int ewdb_api_GetidChanT(EWDBid idChan, double tTime, EWDBid * pidChanT);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetidChansFromSCNLT

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Partial success.  A list of idChans was retrieved,
but the caller's buffer was not large enough to accomadate all of the
idChans found.  See pNumChansFound for the number of idChans found
and pNumChansRetrieved for the number of chans placed in the
caller's buffer.

PARAMETER 1
PARAM_NAME pBuffer
PARAM_TYPE EWDBid *
PARAM_DESCRIPTION Buffer allocated by the caller, where the function
will write the list idChans that match the given SCNL and Time range.

PARAMETER 2
PARAM_NAME IN_szSta
PARAM_TYPE char *
PARAM_DESCRIPTION SEEDlike Station code for the SNCL.  It may not
be blank or NULL, and should be a maximum of 7 characters.  
<br><br>
"*" may be used as a wildcard that will match all Station codes.

PARAMETER 3
PARAM_NAME IN_szComp
PARAM_TYPE char *
PARAM_DESCRIPTION SEEDlike Component code for the SCNL.  The field
may not be blank or NULL and should be a maximum of 9 characters.
<br><br>
"*" may be used as a wildcard that will match all Component codes.

PARAMETER 4
PARAM_NAME IN_szNet
PARAM_TYPE char *
PARAM_DESCRIPTION SEEDlike Network code for the SCNL.  It may not
be blank or NULL and should be a maximum of 9 characters.
<br><br>
"*" may be used as a wildcard that will match all Network codes.

PARAMETER 5
PARAM_NAME IN_szLoc
PARAM_TYPE char *
PARAM_DESCRIPTION Optional SEEDlike Location code for the component. It
may not be NULL (Use a blank string "" to indicate a NULL location code.
It should be a maximum of 9 characters.
<br><br>
"*" may be used as a wildcard that will match all Location codes.

PARAMETER 6
PARAM_NAME IN_tOff
PARAM_TYPE double *
PARAM_DESCRIPTION End time of the time range of interest.  
Expressed as seconds since 1970.

PARAMETER 7
PARAM_NAME IN_tOn
PARAM_TYPE double *
PARAM_DESCRIPTION Start time of the time range of interest.  
Expressed as seconds since 1970.

PARAMETER 8
PARAM_NAME pNumChansFound
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of idChans found to match the SCNL and time range.

PARAMETER 9
PARAM_NAME pNumChansRetrieved
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of idChans placed in the callers buffer(pBuffer).

PARAMETER 10
PARAM_NAME BufferLen
PARAM_TYPE int
PARAM_DESCRIPTION Size of the pBuffer buffer as a multiple of
EWDBid. (example: 15 EWDBids)


DESCRIPTION The function retrieves a list of idChans that are
valid for the given SCNL and time range.  Wildcards ("*") may
be used for any of the S, C, N, and L codes.  The function will
return all idChans that are associated with the components that
have codes matching the given SCNL, during a time period that
overlaps with (IN_tOn - IN_tOff).

*************************************************
************************************************/
int ewdb_api_GetidChansFromSCNLT(EWDBid * pBuffer,  
      char * IN_szSta, char * IN_szComp, char * IN_szNet, char * IN_szLoc,
      double IN_tOff, double IN_tOn, int * pNumChansFound, 
      int * pNumChansRetrieved, int BufferLen);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_SetCompParams

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

PARAMETER 1
PARAM_NAME pStation
PARAM_TYPE EWDB_StationStruct *
PARAM_DESCRIPTION Pointer to a EWDB_StationStruct that contains the
parameters that the caller wishes to set for the component for the
given time interval.

PARAMETER 2
PARAM_NAME tOn
PARAM_TYPE double
PARAM_DESCRIPTION The start of the time interval for which the caller
wants to the component parameters to be valid.  Expressed as seconds
since 1970.

PARAMETER 3
PARAM_NAME tOff
PARAM_TYPE double
PARAM_DESCRIPTION The end of the time interval for which the caller
wants to the component parameters to be valid.  Expressed as seconds
since 1970.

PARAMETER 4
PARAM_NAME pidCompT
PARAM_TYPE EWDBid *
PARAM_DESCRIPTION A pointer to a EWDBid where the function will write
the DB ID of the updated or newly created Component Time Interval.

PARAMETER 5
PARAM_NAME szComment
PARAM_TYPE char *
PARAM_DESCRIPTION Optional comment regarding the Component Time
Interval.  It may be blank, and should be a maximum of 4k characters.
It may not be NULL!

DESCRIPTION Function sets the variable parameters of a component for a
given time interval.  The caller specifies the component by filling in
the SCNL codes in pStation.  Variable parameters include lat, lon, elevation,
and the orientation of the component.  It will write the DB ID of the
newly created/updated Component Time Interval record to to pidCompT.
  
NOTE pStation->idComp is ignored by this function.  The component must
be specified using the SCNL codes, not by a DB idComp.

*************************************************
************************************************/
int ewdb_api_SetCompParams(EWDB_StationStruct * pStation, 
                           double tOn, double tOff,
                           EWDBid * pidCompT, char * szComment);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_SetTransformFuncForChanT 

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

PARAMETER 1
PARAM_NAME idChanT
PARAM_TYPE EWDBid
PARAM_DESCRIPTION The DB identifier of the channel time interval(ChanT)
with which the caller wants to associate a Poles and Zeroes transform
function(CookedTF).

PARAMETER 2
PARAM_NAME dGain
PARAM_TYPE double
PARAM_DESCRIPTION The signal gain for the channel time interval.
Expressed as a double floating point multiplicative scalar.

PARAMETER 3
PARAM_NAME idCookedTF
PARAM_TYPE EWDBid
PARAM_DESCRIPTION The DB identifier of the existing poles and zeroes
based cooked transfer function, that the caller wants to associate with
a given channel time interval(ChanT).

PARAMETER 4
PARAM_NAME dSampRate
PARAM_TYPE double
PARAM_DESCRIPTION Digital sample rate of the data spouted out of the
channel.  Expressed as samples per second.

DESCRIPTION Function associates an existing transform function with a
Channel for a given time period.  Records the gain and sample-rate that
are specific to the channel.

NOTE To create a transform function for a channel, you must call two 
API functions.  You must first call ewdb_api_CreateTransformFunction() to
create the transfer function.  Then you must call 
ewdb_api_SetTransformFuncForChanT() (using the idCookedTF that you got back 
from the create function call) for each "channel time interval" with 
which you wish to associate the transfer function.

*************************************************
************************************************/
int ewdb_api_SetTransformFuncForChanT(EWDBid idChanT, double dGain,
                                      EWDBid idCookedTF, double dSampRate);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY NEW

FUNCTION ewdb_api_GetTransformFunctionForChan

SOURCE_LOCATION src/oracle/schema/src/infra/ewdb_api_GetTransformFunctionForChan.c

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

PARAMETER 1
PARAM_NAME idChan
PARAM_TYPE EWDBid
PARAM_DESCRIPTION The DB ID of the channel for
which the caller wants to retrieve a cooked
transform function.

PARAMETER 2
PARAM_NAME tTime
PARAM_TYPE time_t
PARAM_DESCRIPTION Point in time for which the caller wants
a cooked transform function for the channel.
A time is required because the transform function varies
over time as device properties are changed or the channel is altered.

PARAMETER 3
PARAM_NAME pChanCTF
PARAM_TYPE EWDB_ChanTCTFStruct *
PARAM_DESCRIPTION Pointer to a caller allocated
EWDB_ChanTCTFStruct where the function will write
the cooked transform function and associated "channel
specific response information" requested by the caller.

DESCRIPTION Function retrieves the poles/zeroes based
transform function, gain and sample rate for a given
channel and time.
The transform Function is used to transform recorded
waveforms back into ground motion.

*************************************************
************************************************/
int ewdb_api_GetTransformFunctionForChan(EWDBid idChan, time_t tTime,
                                         EWDB_ChanTCTFStruct * pChanCTF);



/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LOCATION THIS_FILE

LANGUAGE C

STABILITY NEW

FUNCTION ewdb_api_GetSelectedStations 


SOURCE_LOCATION src/oracle/schema/src/infra/ewdb_api_GetSelectedStations.c

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Warning: The buffer (pBuffer) was too small, and more records 
were found than could be written to the buffer.  check to see if 
NumRecordsFound > NumRecordsRetrieved.

PARAMETER 1
PARAM_NAME EWDB_ChannelStruct * 
PARAM_TYPE pBuffer 
PARAM_DESCRIPTION Buffer allocated by the caller of iBufferLen EWDB_ChannelStructs.
The call fills the buffer with the results of the desired query.

PARAMETER 2
PARAM_NAME iBufferLen
PARAM_TYPE int 
PARAM_DESCRIPTION The size of pBuffer in terms of EWDB_ChannelStructs.

PARAMETER 3
PARAM_NAME EWDB_ChannelStruct * 
PARAM_TYPE pCSCriteria 
PARAM_DESCRIPTION Pointer to an EWDB_ChannelStruct that the caller has filled in 
with criteria for the desired query.  pCSCriteria->tOn/tOff control the time interval 
for the query, pCSCriteria->Comp.Lat/Lon in conjunction with 
pCSMaxCriteria->Comp.Lat/Lon, define the lat/lon box for the query,
pCSMaxCriteria->Comp.Sta/Comp/Net/Loc define the SCNL criteria for the query and may
include wildcards, and pCSCriteria->Comp.idChan controls the Channel ID for the query.
Note, the pCSCriteria fields examined are dependent upon the value of iCriteria.

PARAMETER 4
PARAM_NAME EWDB_ChannelStruct * 
PARAM_TYPE pCSMaxCriteria 
PARAM_DESCRIPTION Pointer to an EWDB_ChannelStruct that the caller has filled in 
with criteria for the desired query.  pCSMaxCriteria->Comp.Lat/Lon are used in conjunction
with pCSCriteria->Comp.Lat/Lon to define the lat/lon box for the query.

PARAMETER 5
PARAM_NAME iCriteria
PARAM_TYPE int 
PARAM_DESCRIPTION A flag array that controls the criteria used for the query.  The function checks 
the iCriteria value for a combination of the following EWDB Criteria Selection Flags: <br>
EWDB_CRITERIA_USE_TIME   constrain the query by time  <br>
EWDB_CRITERIA_USE_LAT    constrain the query by lat/lon <br>
EWDB_CRITERIA_USE_SCNL   constrain the query by some combo of SCNL (wildcards supported as "*")<br>
EWDB_CRITERIA_USE_IDCHAN constrain the query by a channel ID(idChan)<br>
<br>

PARAMETER 6
PARAM_NAME pNumStationsFound
PARAM_TYPE int *
PARAM_DESCRIPTION Number of stations(channels) found by the query.

PARAMETER 7
PARAM_NAME pNumStationsRetrieved
PARAM_TYPE int *
PARAM_DESCRIPTION Number of stations(channels) returned by the query.

DESCRIPTION Retrieves a list of stations(actually channel time intervals) based upon
the given criteria.  Response information (pChanProps) is not retrieved as part of the query.
<br>DK 2003/12/03

NOTE  The differences between ewdb_api_GetSelectedStations(), ewdb_api_GetSelectedChannels(),
ewdb_api_GetSelectedStations_w_Response(), ewdb_api_GetCompTInfo(), and ewdb_api_GetSelectedSites() are:
<br><br>
ewdb_api_GetSelectedSites() provides only site information.  It provides no component, channel,
or response information.  It's information is based upon Site time interval granularity.
<br><br>
ewdb_api_GetCompTInfo() provides component information.  It provides no channel,
or response information.  It's information is based upon Component time interval granularity.
<br><br>
ewdb_api_GetSelectedStations() provides all Component level information, along with basic
Channel time interval(ChanT) information.  It provides NO "channel response" information.
It's information is based upon Channel time interval granularity.
<br><br>
ewdb_api_GetSelectedChannels() provides all information provided by ewdb_api_GetSelectedStations(),
plus it also provides partial Channel response information, if it is available, and the CALLER HAS ALLOCATED
SPACE(pChanProps for each applicable record).  If possible, the query includes the Gain and SampleRate
of the Channel, plus the DB ID(idCookedTF) of channel's Transform Function for the time interval.
It's information is based upon Channel time interval granularity.
<br><br>
ewdb_api_GetSelectedStations_w_Response() provides all information provided by ewdb_api_GetSelectedStations(),
plus it also provides full Channel response information where available.  This call ALLOCATES 
space for pChanProps for each record for which it finds response information.  It includes the
Gain, SampleRate, and Poles/Zeroes response function for the Channel.
It's information is based upon Channel time interval granularity.
<br>

*************************************************
************************************************/
int ewdb_api_GetSelectedStations(EWDB_ChannelStruct * pBuffer, int iBufferLen,
                                 EWDB_ChannelStruct * pCSCriteria, 
                                 EWDB_ChannelStruct * pCSMaxCriteria, 
                                 int iCriteria,
                                 int * pNumStationsFound, 
                                 int * pNumStationsRetrieved);


int ewdb_api_GetSelectedPickableStations(EWDB_ChannelStruct * pBuffer, int iBufferLen,
                                 EWDB_ChannelStruct * pCSCriteria, 
                                 EWDB_ChannelStruct * pCSMaxCriteria, 
                                 int iCriteria,
                                 int * pNumStationsFound, 
                                 int * pNumStationsRetrieved);



/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LOCATION THIS_FILE

LANGUAGE C

STABILITY NEW

FUNCTION ewdb_api_GetCompTInfo 


SOURCE_LOCATION src/oracle/schema/src/infra/ewdb_api_GetCompTInfo.c

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Warning: The buffer (pBuffer) was too small, and more records 
were found than could be written to the buffer.  check to see if 
NumRecordsFound > NumRecordsRetrieved.

PARAMETER 1
PARAM_NAME EWDB_ChannelStruct * 
PARAM_TYPE pBuffer 
PARAM_DESCRIPTION Buffer allocated by the caller of iBufferLen EWDB_ChannelStructs.
The call fills the buffer with the results of the desired query.

PARAMETER 2
PARAM_NAME iBufferLen
PARAM_TYPE int 
PARAM_DESCRIPTION The size of pBuffer in terms of EWDB_ChannelStructs.

PARAMETER 3
PARAM_NAME EWDB_ChannelStruct * 
PARAM_TYPE pCSCriteria 
PARAM_DESCRIPTION Pointer to an EWDB_ChannelStruct that the caller has filled in 
with criteria for the desired query.  pCSCriteria->tOn/tOff control the time interval 
for the query, pCSCriteria->Comp.Lat/Lon in conjunction with 
pCSMaxCriteria->Comp.Lat/Lon, define the lat/lon box for the query,
and pCSMaxCriteria->Comp.Sta/Comp/Net/Loc define the SCNL criteria for the query and may
include wildcards.
Note, the pCSCriteria fields examined are dependent upon the value of iCriteria.

PARAMETER 4
PARAM_NAME EWDB_ChannelStruct * 
PARAM_TYPE pCSMaxCriteria 
PARAM_DESCRIPTION Pointer to an EWDB_ChannelStruct that the caller has filled in 
with criteria for the desired query.  pCSMaxCriteria->Comp.Lat/Lon are used in conjunction
with pCSCriteria->Comp.Lat/Lon to define the lat/lon box for the query.

PARAMETER 5
PARAM_NAME iCriteria
PARAM_TYPE int 
PARAM_DESCRIPTION A flag array that controls the criteria used for the query.  The function checks 
the iCriteria value for a combination of the following EWDB Criteria Selection Flags: <br>
EWDB_CRITERIA_USE_TIME   constrain the query by time  <br>
EWDB_CRITERIA_USE_LAT    constrain the query by lat/lon <br>
EWDB_CRITERIA_USE_SCNL   constrain the query by some combo of SCNL (wildcards supported as "*")<br>
<br>

PARAMETER 6
PARAM_NAME pNumStationsFound
PARAM_TYPE int *
PARAM_DESCRIPTION Number of stations(components) found by the query.

PARAMETER 7
PARAM_NAME pNumStationsRetrieved
PARAM_TYPE int *
PARAM_DESCRIPTION Number of stations(components) returned by the query.

DESCRIPTION Retrieves a list of stations(actually component time intervals) based upon
the given criteria.  No Channel information is retrieved as part of the query.
<br>DK 2003/12/03

NOTE  See ewdb_api_GetSelectedStations for a comparison of this an other similar functions.

*************************************************
************************************************/
int ewdb_api_GetCompTInfo(EWDB_ChannelStruct * pBuffer, int iBufferLen,
                          EWDB_ChannelStruct * pCSCriteria, 
                          EWDB_ChannelStruct * pCSMaxCriteria, 
                          int iCriteria,
                          int * pNumStationsFound, 
                          int * pNumStationsRetrieved);

/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LOCATION THIS_FILE

LANGUAGE C

STABILITY NEW

FUNCTION ewdb_api_GetSelectedChannels 


SOURCE_LOCATION src/oracle/schema/src/infra/ewdb_api_GetSelectedChannels.c

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Warning: The buffer (pBuffer) was too small, and more records 
were found than could be written to the buffer.  check to see if 
NumRecordsFound > NumRecordsRetrieved.

PARAMETER 1
PARAM_NAME EWDB_ChannelStruct * 
PARAM_TYPE pBuffer 
PARAM_DESCRIPTION Buffer allocated by the caller of iBufferLen EWDB_ChannelStructs.
The call fills the buffer with the results of the desired query.

PARAMETER 2
PARAM_NAME iBufferLen
PARAM_TYPE int 
PARAM_DESCRIPTION The size of pBuffer in terms of EWDB_ChannelStructs.

PARAMETER 3
PARAM_NAME EWDB_ChannelStruct * 
PARAM_TYPE pCSCriteria 
PARAM_DESCRIPTION Pointer to an EWDB_ChannelStruct that the caller has filled in 
with criteria for the desired query.  pCSCriteria->tOn/tOff control the time interval 
for the query, pCSCriteria->Comp.Lat/Lon in conjunction with 
pCSMaxCriteria->Comp.Lat/Lon, define the lat/lon box for the query,
pCSMaxCriteria->Comp.Sta/Comp/Net/Loc define the SCNL criteria for the query and may
include wildcards, and pCSMaxCriteria->Comp.idChan controls the Channel ID for the query.
Note, the pCSCriteria fields examined are dependent upon the value of iCriteria.

PARAMETER 4
PARAM_NAME EWDB_ChannelStruct * 
PARAM_TYPE pCSMaxCriteria 
PARAM_DESCRIPTION Pointer to an EWDB_ChannelStruct that the caller has filled in 
with criteria for the desired query.  pCSMaxCriteria->Comp.Lat/Lon are used in conjunction
with pCSCriteria->Comp.Lat/Lon to define the lat/lon box for the query.

PARAMETER 5
PARAM_NAME iCriteria
PARAM_TYPE int 
PARAM_DESCRIPTION A flag array that controls the criteria used for the query.  The function checks 
the iCriteria value for a combination of the following EWDB Criteria Selection Flags: <br>
EWDB_CRITERIA_USE_TIME   constrain the query by time  <br>
EWDB_CRITERIA_USE_LAT    constrain the query by lat/lon <br>
EWDB_CRITERIA_USE_SCNL   constrain the query by some combo of SCNL (wildcards supported as "*")<br>
EWDB_CRITERIA_USE_IDCHAN constrain the query by a channel ID(idChan)<br>
<br>

PARAMETER 6
PARAM_NAME pNumStationsFound
PARAM_TYPE int *
PARAM_DESCRIPTION Number of stations(channels) found by the query.

PARAMETER 7
PARAM_NAME pNumStationsRetrieved
PARAM_TYPE int *
PARAM_DESCRIPTION Number of stations(channels) returned by the query.

DESCRIPTION Retrieves a list of channels(actually channel time intervals) based upon
the given criteria.  Partial response information (EWDB_ChannelStruct.pChanProps->dGain/dSampRate/idCookedTF) 
is retrieved, if BOTH it is available in DB, and if THE CALLER HAS ALLOCATED a pChanProps structure for the channel.
<br>DK 2003/12/03

NOTE  See ewdb_api_GetSelectedStations() for a comparison of this and other similar functions.

*************************************************
************************************************/
int ewdb_api_GetSelectedChannels(EWDB_ChannelStruct * pBuffer, int iBufferLen,
                                 EWDB_ChannelStruct * pCSCriteria, 
                                 EWDB_ChannelStruct * pCSMaxCriteria, 
                                 int iCriteria,
                                 int * pNumStationsFound, 
                                 int * pNumStationsRetrieved);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LOCATION THIS_FILE

LANGUAGE C

STABILITY NEW

FUNCTION ewdb_api_GetSelectedSites 


SOURCE_LOCATION src/oracle/schema/src/infra/ewdb_api_GetSelectedSites.c

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Warning: The buffer (pBuffer) was too small, and more records 
were found than could be written to the buffer.  check to see if 
NumRecordsFound > NumRecordsRetrieved.

PARAMETER 1
PARAM_NAME EWDB_ChannelStruct * 
PARAM_TYPE pBuffer 
PARAM_DESCRIPTION Buffer allocated by the caller of iBufferLen EWDB_ChannelStructs.
The call fills the buffer with the results of the desired query.

PARAMETER 2
PARAM_NAME iBufferLen
PARAM_TYPE int 
PARAM_DESCRIPTION The size of pBuffer in terms of EWDB_ChannelStructs.

PARAMETER 3
PARAM_NAME EWDB_ChannelStruct * 
PARAM_TYPE pCSCriteria 
PARAM_DESCRIPTION Pointer to an EWDB_ChannelStruct that the caller has filled in 
with criteria for the desired query.  pCSCriteria->tOn/tOff control the time interval 
for the query, pCSCriteria->Comp.Lat/Lon in conjunction with 
pCSMaxCriteria->Comp.Lat/Lon, define the lat/lon box for the query,
pCSMaxCriteria->Comp.Sta/Net define the SN criteria for the query and may
include wildcards, and pCSMaxCriteria->Comp.idChan controls the Channel ID for the query.
Note, the pCSCriteria fields examined are dependent upon the value of iCriteria.

PARAMETER 4
PARAM_NAME EWDB_ChannelStruct * 
PARAM_TYPE pCSMaxCriteria 
PARAM_DESCRIPTION Pointer to an EWDB_ChannelStruct that the caller has filled in 
with criteria for the desired query.  pCSMaxCriteria->Comp.Lat/Lon are used in conjunction
with pCSCriteria->Comp.Lat/Lon to define the lat/lon box for the query.

PARAMETER 5
PARAM_NAME iCriteria
PARAM_TYPE int 
PARAM_DESCRIPTION A flag array that controls the criteria used for the query.  The function checks 
the iCriteria value for a combination of the following EWDB Criteria Selection Flags: <br>
EWDB_CRITERIA_USE_TIME   constrain the query by time  <br>
EWDB_CRITERIA_USE_LAT    constrain the query by lat/lon <br>
EWDB_CRITERIA_USE_SCNL   constrain the query by some combo of SN (C and L are ignored) (wildcards supported as "*")<br>
<br>

PARAMETER 6
PARAM_NAME pNumStationsFound
PARAM_TYPE int *
PARAM_DESCRIPTION Number of stations found by the query.

PARAMETER 7
PARAM_NAME pNumStationsRetrieved
PARAM_TYPE int *
PARAM_DESCRIPTION Number of stations returned by the query.

DESCRIPTION Retrieves a list of stations(actually site time intervals) based upon
the given criteria.  No component or channel information is retrieved.
<br>DK 2003/12/03

NOTE  See ewdb_api_GetSelectedStations() for a comparison of this and other similar functions.

*************************************************
************************************************/
int ewdb_api_GetSelectedSites(EWDB_ChannelStruct * pBuffer, int iBufferLen,
                              EWDB_ChannelStruct * pCSCriteria, 
                              EWDB_ChannelStruct * pCSMaxCriteria, 
                              int iCriteria,
                              int * pNumStationsFound, 
                              int * pNumStationsRetrieved);



/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LOCATION THIS_FILE

LANGUAGE C

STABILITY NEW

FUNCTION ewdb_api_CreateSite 


SOURCE_LOCATION src/oracle/schema/src/infra/ewdb_api_CreateSite.c

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION  Warning, Site could not be created beacuse a matching Site already exists.

PARAMETER 1
PARAM_NAME pChan
PARAM_TYPE EWDB_ChannelStruct *
PARAM_DESCRIPTION Pointer to the Channel Structure which the caller has filled
in with the Site time interval(SiteT) parameters.

PARAMETER 2
PARAM_NAME szComment
PARAM_TYPE char *
PARAM_DESCRIPTION Null terminated string that represents a comment that the caller
wishes to associate with the Site.

DESCRIPTION Attempts to create a new Site in the DB, using the
caller supplied parameters: pChan->Comp.Sta/Net and szComment.  
The resulting idSite of the new Site record is written to pChan->idSite.
If a matching Site already exists(matching Sta and Net), EWDB_RETURN_WARNING is
returned and the idSite of the existing Site is written to pChan->idSite.
<br>
In the event of a SQL error.  EWDB_RETURN_FAILURE is returned, and the SQL error is written
to pChan->idSite.
<br>DK 2003/12/03

*************************************************
************************************************/
int ewdb_api_CreateSite(EWDB_ChannelStruct * pChan, char * szComment);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LOCATION THIS_FILE

LANGUAGE C

STABILITY NEW

FUNCTION ewdb_api_CreateCompT 


SOURCE_LOCATION src/oracle/schema/src/infra/ewdb_api_CreateCompT.c

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Warning a record with matching idComp and overlapping 
time intervall (tOn - tOff) already exists.  New record could not be 
created.  The idCompT of the first overlapping record(there may be multiple)
is written to pChan->idCompT.

PARAMETER 1
PARAM_NAME pChan
PARAM_TYPE EWDB_ChannelStruct *
PARAM_DESCRIPTION Pointer to the Channel Structure which the caller has filled
in with the Component time interval(CompT) parameters.

DESCRIPTION Attempts to create a new Component time interval(CompT) in the DB, using the
caller supplied parameters: pChan->Comp.idComp, pChan->tOn/tOff, pChan->Comp.Lat/Lon/Elev,
and pChan->Comp.Azm,Dip.  If successful, the DB idCompT of the resulting record is
wirtten to pChan->idCompT.  If a matching record already exists (same idComp) with overlapping 
time interval(tOn - tOff), the function will not create the new record.  Instead
EWDB_RETURN_WARNING will be returned and the idCompT of the first overlapping record
will be written to pChan->idCompT.
<br>
In the event of a SQL error.  EWDB_RETURN_FAILURE is returned, and the SQL error is written
to pChan->idCompT.
<br>DK 2003/12/03

*************************************************
************************************************/
int ewdb_api_CreateCompT(EWDB_ChannelStruct * pChan);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LOCATION THIS_FILE

LANGUAGE C

STABILITY NEW

FUNCTION ewdb_api_CreateSiteT 


SOURCE_LOCATION src/oracle/schema/src/infra/ewdb_api_CreateSiteT.c

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

PARAMETER 1
PARAM_NAME pChan
PARAM_TYPE EWDB_ChannelStruct *
PARAM_DESCRIPTION Pointer to the Channel Structure which the caller has filled
in with the Site time interval(SiteT) parameters.

DESCRIPTION Attempts to create a new Site time interval(SiteT) in the DB, using the
caller supplied parameters: pChan->idSite, pChan->tOn/tOff, and pChan->Comp.Lat/Lon/Elev.
<br>
In the event of a SQL error.  EWDB_RETURN_FAILURE is returned, and the SQL error is written
to pChan->idSiteT.
<br>DK 2003/12/03

*************************************************
************************************************/
int ewdb_api_CreateSiteT(EWDB_ChannelStruct * pChan);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LOCATION THIS_FILE

LANGUAGE C

STABILITY NEW

FUNCTION ewdb_api_DeleteChanT 

SOURCE_LOCATION src/oracle/schema/src/infra/ewdb_api_DeleteChanT.c

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

PARAMETER 1
PARAM_NAME idCompT
PARAM_TYPE EWDBid
PARAM_DESCRIPTION DB ID of the Channel time interval(ChanT) the caller wishes to delete.

DESCRIPTION Function deletes the DB record for the Channel time interval(ChanT) specified
by the caller.  
<br>
In the event that the function cannot delete the ChanT record because of a 
DB foreign key constraint error, EWDB_RETURN_WARNING will be returned
and a warning will be  written to the logfile.  In the event of a 
different SQL error, EWDB_RETURN_FAILURE is returned, and the SQL 
error is written to the logfile.  The SQL error is not returned by the function.
<br>DK 2003/12/03

*************************************************
************************************************/
int ewdb_api_DeleteChanT(EWDBid idChanT);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LOCATION THIS_FILE

LANGUAGE C

STABILITY NEW

FUNCTION ewdb_api_DeleteComp 

SOURCE_LOCATION src/oracle/schema/src/infra/ewdb_api_DeleteComp.c

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

PARAMETER 1
PARAM_NAME idCompT
PARAM_TYPE EWDBid
PARAM_DESCRIPTION DB ID of the Component(Comp) the caller wishes to delete.

DESCRIPTION Function deletes the DB record for the Component(Comp) specified
by the caller.  
<br>
In the event that the function cannot delete the Comp record because of a 
DB foreign key constraint error, EWDB_RETURN_WARNING will be returned
and a warning will be  written to the logfile.  In the event of a 
different SQL error, EWDB_RETURN_FAILURE is returned, and the SQL 
error is written to the logfile.  The SQL error is not returned by the function.
<br>DK 2003/12/03

*************************************************
************************************************/
int ewdb_api_DeleteComp(EWDBid idComp);



/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LOCATION THIS_FILE

LANGUAGE C

STABILITY NEW

FUNCTION ewdb_api_DeleteCompT 

SOURCE_LOCATION src/oracle/schema/src/infra/ewdb_api_DeleteCompT.c

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

PARAMETER 1
PARAM_NAME idCompT
PARAM_TYPE EWDBid
PARAM_DESCRIPTION DB ID of the Component time interval(CompT) the caller wishes to delete.

DESCRIPTION Function deletes the DB record for the Component time interval(CompT) specified
by the caller.  
<br>
In the event that the function cannot delete the CompT record because of a 
DB foreign key constraint error, EWDB_RETURN_WARNING will be returned
and a warning will be  written to the logfile.  In the event of a 
different SQL error, EWDB_RETURN_FAILURE is returned, and the SQL 
error is written to the logfile.  The SQL error is not returned by the function.
<br>DK 2003/12/03

*************************************************
************************************************/
int ewdb_api_DeleteCompT(EWDBid idCompT);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LOCATION THIS_FILE

LANGUAGE C

STABILITY NEW

FUNCTION ewdb_api_DeleteSiteT 

SOURCE_LOCATION src/oracle/schema/src/infra/ewdb_api_DeleteSiteT.c

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

PARAMETER 1
PARAM_NAME idSiteT
PARAM_TYPE EWDBid
PARAM_DESCRIPTION DB ID of the Site time interval(SiteT) the caller wishes to delete.

DESCRIPTION Function deletes the DB record for the Site time interval(SiteT) specified
by the caller.  
<br>
In the event that the function cannot delete the SiteT record because of a 
DB foreign key constraint error, EWDB_RETURN_WARNING will be returned
and a warning will be  written to the logfile.  In the event of a 
different SQL error, EWDB_RETURN_FAILURE is returned, and the SQL 
error is written to the logfile.  The SQL error is not returned by the function.
<br>DK 2003/12/03

*************************************************
************************************************/
int ewdb_api_DeleteSiteT(EWDBid idSiteT);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LOCATION THIS_FILE

LANGUAGE C

STABILITY NEW

FUNCTION ewdb_api_DeleteSite 

SOURCE_LOCATION src/oracle/schema/src/infra/ewdb_api_DeleteSite.c

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

PARAMETER 1
PARAM_NAME idSiteT
PARAM_TYPE EWDBid
PARAM_DESCRIPTION DB ID of the Site the caller wishes to delete.

DESCRIPTION Function deletes the DB record for the Site  specified
by the caller.  
<br>
In the event that the function cannot delete the Site record because of a 
DB foreign key constraint error, EWDB_RETURN_WARNING will be returned
and a warning will be  written to the logfile.  In the event of a 
different SQL error, EWDB_RETURN_FAILURE is returned, and the SQL 
error is written to the logfile.  The SQL error is not returned by the function.
<br>DK 2003/12/03

*************************************************
************************************************/
int ewdb_api_DeleteSite(EWDBid idSite);




/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LOCATION THIS_FILE

LANGUAGE C

STABILITY NEW

FUNCTION ewdb_api_GetChanParams 


SOURCE_LOCATION src/oracle/schema/src/infra/ewdb_api_GetChanParams.c

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

PARAMETER 1
PARAM_NAME pChan
PARAM_TYPE EWDB_ChannelStruct *
PARAM_DESCRIPTION Pointer to the Channel Structure which the caller wishes
filled with parameters for the given Channel time interval(ChanT).

DESCRIPTION Function retrieves the parameters for the given Channel time interval(ChanT).
The interval can be specified one of two ways.  The first and overriding way is by setting
pChan->idChanT to the DB ID of the interval.  The second way is by setting 
pChan->Comp.idChan to the DB ID of the channel, and by setting the pchan->tOn/tOff to the
desired time range.  Note: if the second method is used, it will find all ChanT records
that overlap with that range.  If more than one record is found, the function will return
EWDB_RETURN_WARNING, and the parameters of the first(smallest idChanT) will be written to the
pChan structure.

<br>
In the event of a SQL error.  EWDB_RETURN_FAILURE is returned, and the SQL error is written
to the logfile.  The SQL error is not returned by the function.
<br>DK 2003/12/03

*************************************************
************************************************/
int ewdb_api_GetChanParams(EWDB_ChannelStruct * pChan);



/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LOCATION THIS_FILE

LANGUAGE C

STABILITY NEW

FUNCTION ewdb_api_GetCompTParams 


SOURCE_LOCATION src/oracle/schema/src/infra/ewdb_api_GetCompTParams.c

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

PARAMETER 1
PARAM_NAME pChan
PARAM_TYPE EWDB_ChannelStruct *
PARAM_DESCRIPTION Pointer to the Channel Structure which the caller wishes
filled with parameters for the given Component time interval(CompT).

DESCRIPTION Function retrieves the parameters for the given Component time interval(CompT).
The interval can be specified one of two ways.  The first and overriding way is by setting
pChan->idCompT to the DB ID of the interval.  The second way is by setting 
pChan->Comp.idComp to the DB ID of the component, and by setting the pchan->tOn/tOff to the
desired time range.  Note: if the second method is used, it will find all CompT records
that overlap with that range.  If more than one record is found, the function will return
EWDB_RETURN_WARNING, and the parameters of the first(smallest idCompT) will be written to the
pChan structure.

<br>
In the event of a SQL error.  EWDB_RETURN_FAILURE is returned, and the SQL error is written
to the logfile.  The SQL error is not returned by the function.
<br>DK 2003/12/03

*************************************************
************************************************/
int ewdb_api_GetCompTParams(EWDB_ChannelStruct * pChan);





/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LOCATION THIS_FILE

LANGUAGE C

STABILITY NEW

FUNCTION ewdb_api_GetidSite 


SOURCE_LOCATION src/oracle/schema/src/infra/ewdb_api_GetidSite.c

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

PARAMETER 1
PARAM_NAME pChan
PARAM_TYPE EWDB_ChannelStruct *
PARAM_DESCRIPTION Pointer to the Channel Structure which the caller has
filled with the Sta and Net codes for which they want the idSite.

DESCRIPTION Retrieves the idSite(pChan->idSite) for the given pChan->Comp.Sta/Net.<br>
In the event of a SQL error EWDB_RETURN_FAILURE is returned, and the SQL error is written
to pChan->idSite.  If a Site with the given Sta/Net does not exist in the DB, the function
will fail.
<br>DK 2003/12/03

*************************************************
************************************************/
int ewdb_api_GetidSite(EWDB_ChannelStruct * pChan);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LOCATION THIS_FILE

LANGUAGE C

STABILITY NEW

FUNCTION ewdb_api_GetidComp 


SOURCE_LOCATION src/oracle/schema/src/infra/ewdb_api_GetidComp.c

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

PARAMETER 1
PARAM_NAME pChan
PARAM_TYPE EWDB_ChannelStruct *
PARAM_DESCRIPTION Pointer to the Channel Structure which the caller has
filled with the SCNL codes for which they want the idComp.

DESCRIPTION Retrieves the idComp(pChan->Comp.idComp) for the given pChan->Comp.Sta/Comp/Net/Loc.<br>
In the event of a SQL error EWDB_RETURN_FAILURE is returned, and the SQL error is written
to pChan->idSite.  If a Site with the given SCNL does not exist in the DB, the function
will fail.
<br>DK 2003/12/03

*************************************************
************************************************/
int ewdb_api_GetidComp(EWDB_ChannelStruct * pChan);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LOCATION THIS_FILE

LANGUAGE C

STABILITY NEW

FUNCTION ewdb_api_GetSelectedStations_w_Response 


SOURCE_LOCATION src/oracle/schema/src/infra/ewdb_api_GetSelectedStations_w_Response.c

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Warning: Not all data was retrieved.  A warning is returned in 
one of two situations: <br>
1)The buffer (pBuffer) was too small, and more records were found than could be 
written to the buffer.  check to see if NumRecordsFound > NumRecordsRetrieved.
<br>
2)Response information could not be found for all of the channels retrieved.
Check the pChanProps pointer for NULL, before attempting to process each record.

PARAMETER 1
PARAM_NAME EWDB_ChannelStruct * 
PARAM_TYPE pBuffer 
PARAM_DESCRIPTION Buffer allocated by the caller of iBufferLen EWDB_ChannelStructs.
The call fills the buffer with the results of the desired query.

PARAMETER 2
PARAM_NAME iBufferLen
PARAM_TYPE int 
PARAM_DESCRIPTION The size of pBuffer in terms of EWDB_ChannelStructs.

PARAMETER 3
PARAM_NAME EWDB_ChannelStruct * 
PARAM_TYPE pCSCriteria 
PARAM_DESCRIPTION Pointer to an EWDB_ChannelStruct that the caller has filled in 
with criteria for the desired query.  pCSCriteria->tOn/tOff control the time interval 
for the query, pCSCriteria->Comp.Lat/Lon in conjunction with 
pCSMaxCriteria->Comp.Lat/Lon, define the lat/lon box for the query,
pCSMaxCriteria->Comp.Sta/Comp/Net/Loc define the SCNL criteria for the query and may
include wildcards, and pCSMaxCriteria->Comp.idChan controls the Channel ID for the query.
Note, the pCSCriteria fields examined are dependent upon the value of iCriteria.

PARAMETER 4
PARAM_NAME EWDB_ChannelStruct * 
PARAM_TYPE pCSMaxCriteria 
PARAM_DESCRIPTION Pointer to an EWDB_ChannelStruct that the caller has filled in 
with criteria for the desired query.  pCSMaxCriteria->Comp.Lat/Lon are used in conjunction
with pCSCriteria->Comp.Lat/Lon to define the lat/lon box for the query.

PARAMETER 5
PARAM_NAME iCriteria
PARAM_TYPE int 
PARAM_DESCRIPTION A flag array that controls the criteria used for the query.  The function checks 
the iCriteria value for a combination of the following EWDB Criteria Selection Flags: <br>
EWDB_CRITERIA_USE_TIME   constrain the query by time  <br>
EWDB_CRITERIA_USE_LAT    constrain the query by lat/lon <br>
EWDB_CRITERIA_USE_SCNL   constrain the query by some combo of SCNL (wildcards supported as "*")<br>
EWDB_CRITERIA_USE_IDCHAN constrain the query by a channel ID(idChan)<br>
<br>

PARAMETER 6
PARAM_NAME pNumStationsFound
PARAM_TYPE int *
PARAM_DESCRIPTION Number of stations(channels) found by the query.

PARAMETER 7
PARAM_NAME pNumStationsRetrieved
PARAM_TYPE int *
PARAM_DESCRIPTION Number of stations(channels) returned by the query.



DESCRIPTION Retrieves a list of stations(actually channel time intervals) based upon
the given criteria.  For each channel it attempts to retrieve response information
in addition to component information.  Be sure to ensure pChanProps is not NULL for
each record before attempting to process response information.<br>
<br>DK 2003/12/03

NOTE  <b>POTENTIAL MEMORY LEAK</b> ewdb_api_GetSelectedStations_w_Response() allocates(calloc()) 
one EWDB_ChanTCTFStruct for each Station record it retrieves.  It is the <b>CALLER'S RESPONSIBILITY</b> 
to free the allocated memory via:<br>
<pre><br>
              for(i=0; i<*pNumStationsRetrieved; i++)<br>
               if(pBuffer[i].pChanProps)<br>
               {<br>
                 free(pBuffer[i].pChanProps);<br>
                 pBuffer[i].pChanProps = NULL;<br>
               }<br>
</pre><br>
<br>

*************************************************
************************************************/
int ewdb_api_GetSelectedStations_w_Response(EWDB_ChannelStruct * pBuffer, int iBufferLen,
                                            EWDB_ChannelStruct * pCSCriteria, 
                                            EWDB_ChannelStruct * pCSMaxCriteria, 
                                            int iCriteria,
                                            int * pNumStationsFound, 
                                            int * pNumStationsRetrieved);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LOCATION THIS_FILE

LANGUAGE C

STABILITY NEW

FUNCTION ewdb_api_SetTransformFuncForChanT 


SOURCE_LOCATION src/oracle/schema/src/infra/ewdb_api_SetTransformFuncForChanT.c

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

PARAMETER 1
PARAM_NAME idChanT
PARAM_TYPE EWDBid 
PARAM_DESCRIPTION DB ID of the Channel time interval(ChanT) record for which the 
caller wishes to set the response parameters.

PARAMETER 2
PARAM_NAME dGain
PARAM_TYPE double 
PARAM_DESCRIPTION Overall gain for the Channel.

PARAMETER 3
PARAM_NAME idCookedTF
PARAM_TYPE EWDBid 
PARAM_DESCRIPTION DB ID of the response function (Poles/Zeroes) for the 
Channel time interval.  Set this parameter to 0, if you do not wish to 
associate a response function with the Channel time interval.

PARAMETER 4
PARAM_NAME dSampRate
PARAM_TYPE double 
PARAM_DESCRIPTION Sample rate for the Channel time interval.


DESCRIPTION Sets the response information for a given Channel time interval(ChanT record).<br>
<br>
In the event of a SQL error.  EWDB_RETURN_FAILURE is returned, and the SQL error is written
to the logfile.  The SQL error is not returned by the function.  
<br>DK 2003/12/03
*************************************************
************************************************/
int ewdb_api_SetTransformFuncForChanT(EWDBid idChanT, double dGain,
                                      EWDBid idCookedTF, double dSampRate);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LOCATION THIS_FILE

LANGUAGE C

STABILITY NEW

FUNCTION ewdb_api_SplitSiteT 


SOURCE_LOCATION src/oracle/schema/src/infra/ewdb_api_SplitSiteT.c

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

PARAMETER 1
PARAM_NAME pChan
PARAM_TYPE EWDB_ChannelStruct *
PARAM_DESCRIPTION Pointer to the Channel Structure which the caller has
filled with the parameters for the given Site time interval(SiteT).
pChan->tOn is the desired split time.


PARAMETER 2
PARAM_NAME pidNewSiteT
PARAM_TYPE EWDBid *
PARAM_DESCRIPTION Pointer to an EWDBid, where the call will write the DB ID of the
newly created Site time interval(SiteT) record, that resulted from the split 
of the existing one.  On error, the call will write the SQL error to the EWDBid.

DESCRIPTION Attempts to split an existing Site time interval(SiteT record) into two
pieces.  The first resulting interval is from Original.tOn - tSplit, and keeps 
the Original.idSiteT.  The second interval is from tSplit - Original.tOff, and 
receives a new idSiteT, which is returned to pidNewSiteT.<br>
<br>
The function will fail if the given idSiteT (pChan->idSiteT) does not exist in the
DB, or if tSplit(pChan->tOn) is outside of the timeinterval for pChan->idSiteT record.<br>
<br>
In the event of a SQL error.  EWDB_RETURN_FAILURE is returned, and the SQL error is written
to pidNewSiteT.
<br>DK 2003/12/03

NOTE  This function is only for updating the actual time interval of 
a given Channel time interval(ChanT).  To update the parameters of the 
Channel during the given time interval (Lat/Lon/Elev etc.)
use the ewdb_internal_SetChanParams() function.

*************************************************
************************************************/
int ewdb_api_SplitSiteT(EWDB_ChannelStruct * pChan, EWDBid * pidNewSiteT);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LOCATION THIS_FILE

LANGUAGE C

STABILITY NEW

FUNCTION ewdb_api_GetSiteTParams 


SOURCE_LOCATION src/oracle/schema/src/infra/ewdb_api_GetSiteTParams.c

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

PARAMETER 1
PARAM_NAME pChan
PARAM_TYPE EWDB_ChannelStruct *
PARAM_DESCRIPTION Pointer to the Channel Structure which the call will
fill with the parameters for the given Site time interval(SiteT), specified by
pChan->idSiteT.

DESCRIPTION Retrieves the Site time interval(SiteT) parameters for a given SiteT 
record, identified by pChan->idSiteT.  Writes the retrieved parameters to structure
pointed to by pChan.
<br>
In the event of a SQL error.  EWDB_RETURN_FAILURE is returned, and the SQL error is written
to the logfile.  the SQL error is not returned by the function.
<br>DK 2003/12/03

*************************************************
************************************************/
int ewdb_api_GetSiteTParams(EWDB_ChannelStruct * pChan);



/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LOCATION THIS_FILE

LANGUAGE C

STABILITY NEW

FUNCTION ewdb_api_UpdateChanTTime 


SOURCE_LOCATION src/oracle/schema/src/infra/ewdb_api_UpdateChanTTime.c

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

PARAMETER 1
PARAM_NAME OUT_pRetCode
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer, where the call will write the return 
code from the SQL function.

PARAMETER 2
PARAM_NAME pChan
PARAM_TYPE EWDB_ChannelStruct *
PARAM_DESCRIPTION Pointer to the Channel Structure which the caller has
filled with the parameters for the given Channel time interval(ChanT).

DESCRIPTION Updates the time parameters for a given Channel time interval(ChanT record) in the DB.
pChan->idChanT is the DB ID of the given Channel time interval.  pChan->tOn/tOff define the
updated time interval. 
<br>
In the event of a SQL error.  EWDB_RETURN_FAILURE is returned, and the SQL error is written
to OUT_pRetCode.
<br>DK 2003/12/03

NOTE  This function is only for updating the actual time interval of 
a given Channel time interval(ChanT).  To update the parameters of the 
Channel during the given time interval (Lat/Lon/Elev etc.)
use the ewdb_internal_SetChanParams() function.

*************************************************
************************************************/
int ewdb_api_UpdateChanTTime(int * OUT_pRetCode, EWDB_ChannelStruct * pChan);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LOCATION THIS_FILE

LANGUAGE C

STABILITY NEW

FUNCTION ewdb_api_ModifySiteParams 


SOURCE_LOCATION src/oracle/schema/src/infra/ewdb_api_ModifySiteParams.c

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

PARAMETER 1
PARAM_NAME OUT_pRetCode
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer, where the call will write the return 
code from the SQL function.

PARAMETER 2
PARAM_NAME pChan
PARAM_TYPE EWDB_ChannelStruct *
PARAM_DESCRIPTION Pointer to the Channel Structure which the caller has
filled with the parameters for the given Site time interval(SiteT).

PARAMETER 3
PARAM_NAME szComment
PARAM_TYPE char *
PARAM_DESCRIPTION Null terminated string, containing the comment for the updated
Site time interval(SiteT) record.  The max acceptable length is 1023 characters.
If the string is blank, it will not update the comment field for the record.

DESCRIPTION Updates the non-time parameters for a given Site time interval(SiteT record) in the DB,
including lat, lon, and elevation.
pChan->idSiteT is the DB ID of the given Site time interval.  pChan->Comp.Lat/Lon/Elev define the
updated params of the Site time interval.  szComment defines the updated comment for the record.
<br>
In the event of a SQL error.  EWDB_RETURN_FAILURE is returned, and the SQL error is written
to OUT_pRetCode.
<br>DK 2003/12/03

NOTE  This function is only for updating the parameters of a given Site time interval(SiteT).  
To update the actual time intervalof the Site during the given time interval 
use the ewdb_api_UpdateSiteT() function.

*************************************************
************************************************/
int ewdb_api_ModifySiteParams(int * OUT_pRetCode, EWDB_ChannelStruct * pChan, 
                              char * szComment);



/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LOCATION THIS_FILE

LANGUAGE C

STABILITY NEW

FUNCTION ewdb_api_UpdateSiteT 


SOURCE_LOCATION src/oracle/schema/src/infra/ewdb_api_UpdateSiteT.c

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

PARAMETER 1
PARAM_NAME OUT_pRetCode
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer, where the call will write the return 
code from the SQL function.

PARAMETER 2
PARAM_NAME pChan
PARAM_TYPE EWDB_ChannelStruct *
PARAM_DESCRIPTION Pointer to the Channel Structure which the caller has
filled with the parameters for the given Site time interval(SiteT).

PARAMETER 3
PARAM_NAME szComment
PARAM_TYPE char *
PARAM_DESCRIPTION Null terminated string, containing the comment for the updated
Site time interval(SiteT) record.  The max acceptable length is 1023 characters.
If the string is blank, it will not update the comment field for the record.

DESCRIPTION Updates the time parameters for a given Site time interval(SiteT record) in the DB.
pChan->idSiteT is the DB ID of the given Site time interval.  pChan->tOn/tOff define the
updated time interval.  szComment defines the updated comment for the record.
<br>
In the event of a SQL error.  EWDB_RETURN_FAILURE is returned, and the SQL error is written
to OUT_pRetCode.
<br>DK 2003/12/03

NOTE  This function is only for updating the actual time interval of a given Site time interval(SiteT).  
To update the parameters of the Site during the given time interval (Lat/Lon/Elev etc.)
use the ewdb_api_ModifySiteParams() function.

*************************************************
************************************************/
int ewdb_api_UpdateSiteT(int * OUT_pRetCode, EWDB_ChannelStruct * pChan, 
                         char * szComment);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LOCATION THIS_FILE

LANGUAGE C

STABILITY NEW

FUNCTION ewdb_api_SetComptParams 


SOURCE_LOCATION src/oracle/schema/src/infra/ewdb_api_SetComptParams.c

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

PARAMETER 1
PARAM_NAME pChan
PARAM_TYPE EWDB_ChannelStruct *
PARAM_DESCRIPTION Pointer to the Channel Structure which the caller has
filled with the parameters for the given Component time interval(CompT).

PARAMETER 2
PARAM_NAME szComment
PARAM_TYPE char *
PARAM_DESCRIPTION Null terminated string, containing the comment for the 
Component time interval(CompT) record.  The max acceptable length is 4000 characters.
If the string is blank, the comment for the CompT record will not be updated.

DESCRIPTION Updates the parameters for a given Component time interval(CompT record) in the DB.
pChan->idCompT is the DB ID of the given Component time interval.  pChan->Comp.Lat/Lon/Elev/Azm/Dip
are the parameters used to update the record.  All parameters except for time are updated, so if
you wish to update only one of the parameters, you must retrieve the existing params from the DB,
modify the one you are interested in updating, and then call this function to update the record,
otherwise the old params will be wiped out.

<br>
In the event of a SQL error.  EWDB_RETURN_FAILURE is returned, and the SQL error is written
to the logfile.  The SQL error is not returned by the function.
<br>DK 2003/12/03

NOTE  This function is only for updating the parameters of a given Component time interval(CompT).  
To update the actual time interval of the Component, use the  ewdb_api_UpdateCompTTime() function.
This function only updates an existing Component time interval.  To create a new one, use the
ewdb_api_SetCompParams() function.

*************************************************
************************************************/
int ewdb_api_SetComptParams(EWDB_ChannelStruct * IN_pChan, char * IN_szComment);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LOCATION THIS_FILE

LANGUAGE C

STABILITY NEW

FUNCTION ewdb_api_UpdateCompTTime 


SOURCE_LOCATION src/oracle/schema/src/infra/ewdb_api_UpdateCompTTime.c

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

PARAMETER 1
PARAM_NAME OUT_pRetCode
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer, where the call will write the return 
code from the SQL function.

PARAMETER 2
PARAM_NAME pChan
PARAM_TYPE EWDB_ChannelStruct *
PARAM_DESCRIPTION Pointer to the Channel Structure which the caller has
filled with the parameters for the given Component time interval(CompT).

DESCRIPTION Updates the time parameters for a given Component time interval(CompT record) in the DB.
pChan->idCompT is the DB ID of the given Component time interval.  pChan->tOn/tOff define the
updated time interval. 
<br>
In the event of a SQL error.  EWDB_RETURN_FAILURE is returned, and the SQL error is written
to OUT_pRetCode.
<br>DK 2003/12/03

NOTE  This function is only for updating the actual time interval of 
a given Component time interval(CompT).  To update the parameters of the 
Component during the given time interval (Lat/Lon/Elev etc.)
use the ewdb_api_SetComptParams() function.

*************************************************
************************************************/
int ewdb_api_UpdateCompTTime(int * OUT_pRetCode, EWDB_ChannelStruct * pChan);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LOCATION THIS_FILE

LANGUAGE C

STABILITY NEW

FUNCTION ewdb_api_UpdateTransformFunction 


SOURCE_LOCATION src/oracle/schema/src/infra/ewdb_api_UpdateTransformFunction.c

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

PARAMETER 1
PARAM_NAME pCookedTF
PARAM_TYPE EWDB_TransformFunctionStruct *
PARAM_DESCRIPTION Pointer to the TransformFunctionStruct which the caller has
filled with the parameters for the given transform function.

PARAMETER 2
PARAM_NAME IN_bUpdatePZ
PARAM_TYPE int
PARAM_DESCRIPTION Boolean flag indicating whether or not the poles/zeroes of
the transform function should be updated.

DESCRIPTION Updates the parameters for a given transform function, identified
by the DB ID pCookedTF->idCookedTF.  The description of the function is always
updated with the value pCookedTF->szCookedTFDesc.  If IN_bUpdatePZ is set to true, 
then the poles and zeroes of the function are also updated based on the values 
in the pCookedTF structure.

<br>
In the event of a SQL error.  EWDB_RETURN_FAILURE is returned.  See the logfile for the SQL error,
as it is not returned by the function.
<br>DK 2003/12/03

*************************************************
************************************************/
int ewdb_api_UpdateTransformFunction(EWDB_TransformFunctionStruct * pCookedTF,
                                     int IN_bUpdatePZ);




/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY COOKED_INFRASTRUCTURE_API

LOCATION THIS_FILE

LANGUAGE C

STABILITY NEW

FUNCTION ewdb_api_CreateOrUpdateChanParams 


SOURCE_LOCATION src/oracle/schema/src/infra/ewdb_api_CreateOrUpdateChanParams.c

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

PARAMETER 1
PARAM_NAME pChan
PARAM_TYPE EWDB_ChannelStruct *
PARAM_DESCRIPTION Pointer to the ChannelStruct which the caller has
filled with the parameters for the given channel.

DESCRIPTION Sets the parameters for a channel for a given time period.
pChan->Comp.idChan defines the channel.  pChan->tOn/tOff define the
time period.  pchan->idCompT defines the Component(time-interval) with
which the channel is associated for the given time period.  All other
pChan members are ignored.<br>
Upon Successfull completion, the DB ID of the new ChanT(channel time-interval)
record is written back to pChan->idChanT.<br>
<br>
In the event of a SQL error.  EWDB_RETURN_FAILURE is returned, and the SQL error
is written to pChan->idChanT.
<br>DK 2004/04/03

NOTE  ewdb_api_CreateOrUpdateChanParams() is very dangerous because
it forces the overwriting of existing channel data.  If you have
existing channel settings defined within the time period (tOn-tOff),
the function will overwrite those settings, and they will be lost.
Use this function CAREFULLY!

*************************************************
************************************************/
int ewdb_api_CreateOrUpdateChanParams(EWDB_ChannelStruct * pChan);





#endif /* _EWDB_ORA_API_INFRA_H */
