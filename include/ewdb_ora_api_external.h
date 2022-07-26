/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: ewdb_ora_api_external.h 7322 2018-04-14 03:27:27Z baker $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2008/12/02 21:37:42  paulf
 *     oracle surgery additions
 *
 *     Revision 1.2  2004/08/10 17:25:18  michelle
 *     added declaration for ewdb_api_SelectExternalStation
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
 * definitions and function prototypes for the external
 * subschema.   
 * 
 * This file should never be included in an application
 * program.  Instead, include the master include file
 * ewdb_ora_api.h
 * 
 *    Lucky Vidmar, May 20, 2003
 *
 *******************************************************/

#ifndef _EWDB_ORA_API_EXTERNAL_H
#define _EWDB_ORA_API_EXTERNAL_H



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

SUB_LIBRARY EXTERNAL_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_External_StationStruct
TYPE_DEFINITION struct _EWDB_External_StationStruct DESCRIPTION
Structure for inserting station information into a side "Cheater Table"
area of the DB.

MEMBER Station
MEMBER_TYPE EWDB_StationStruct
MEMBER_DESCRIPTION Station information.

MEMBER StationID
MEMBER_TYPE int
MEMBER_DESCRIPTION The DB StationID of the current station.

MEMBER Description
MEMBER_TYPE char[100]
MEMBER_DESCRIPTION A text description of the station, usually
the long name of the station.


*************************************************
************************************************/
typedef struct _EWDB_External_StationStruct
{
	EWDB_StationStruct 	Station;
	int     			StationID;
	char    			Description[100];   /* station description (for nsn) */
} EWDB_External_StationStruct;


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF

LIBRARY  EWDB_API_LIB

SUB_LIBRARY EXTERNAL_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_External_UH_InfraInfo
TYPE_DEFINITION struct _EWDB_External_UH_InfraInfo
DESCRIPTION Structure for inserting additional infrastructure information
into a side table. This information is needed by the Urban Hazards team.

MEMBER idUHInfo
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION DB id of this record.

MEMBER idChanT
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION DB id of the channel to which this auxiliary
data belongs.

MEMBER dFullscale
MEMBER_TYPE double
MEMBER_DESCRIPTION fullscale.

MEMBER dSensitivity
MEMBER_TYPE double
MEMBER_DESCRIPTION sensitivity.

MEMBER dNaturalFrequency
MEMBER_TYPE double
MEMBER_DESCRIPTION natural frequency.

MEMBER dDamping
MEMBER_TYPE double
MEMBER_DESCRIPTION damping.

MEMBER dAzm
MEMBER_TYPE double
MEMBER_DESCRIPTION The horizontal orientation of the Component.
Expressed in degrees clockwise of North.

MEMBER dDip
MEMBER_TYPE double
MEMBER_DESCRIPTION The vertical orientation of the Component.
Expressed in degrees below horizontal.

MEMBER iGain
MEMBER_TYPE int
MEMBER_DESCRIPTION gain.

MEMBER iSensorType
MEMBER_TYPE int
MEMBER_DESCRIPTION numeric sensor type.

*************************************************
************************************************/
typedef struct _EWDB_External_UH_InfraInfo
{

    EWDBid          idUHInfo;
    EWDBid          idChanT;
    double          dFullscale;
    double          dSensitivity;
    double          dNaturalFrequency;
    double          dDamping;
    double          dAzm;
    double          dDip;
    int             iGain;
    int             iSensorType;
} EWDB_External_UH_InfraInfo;






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

SUB_LIBRARY EXTERNAL_API

LOCATION THIS_FILE

DESCRIPTION This is a portion of the EWDB_API_LIB
that contains references to external systems and
data.  This includes things like a cheater table
to go from Earthworm SCN to EWDB idChan.

*************************************************
************************************************/


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY EXTERNAL_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY MATURE

FUNCTION ewdb_api_CreateOrAlterExternalStation

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

PARAMETER 1
PARAM_NAME pStation
PARAM_TYPE EWDB_External_StationStruct *
PARAM_DESCRIPTION Pointer to an EWDB_External_StationStruct filled by the caller.

DESCRIPTION Function creates or updates a component and associated
channel in the DB using an external station table and a a cheater table
to go from external station to chan.  Upon successful completion it
writes the DB ID of the new External Station to pStation->StationID.
This function ignores the pStation->idComp parameter.

NOTE   If station with the scnl is already in the table,
its STATIONID will be set in the pStation struct, AND all
non-zero, and non-empty character values will be updated.
<br><br>
If this is a new record, all values from the pStation struct
will be stored in the table.


*************************************************
************************************************/
int ewdb_api_CreateOrAlterExternalStation(EWDB_External_StationStruct *pStation);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY EXTERNAL_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetIdChanFromStationExternal

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

PARAMETER 1
PARAM_NAME pidChan
PARAM_TYPE EWDBid *
PARAM_DESCRIPTION Pointer to EWDBid where the function will write
the idChan for this external station.

PARAMETER 2
PARAM_NAME IN_StationID
PARAM_TYPE int
PARAM_DESCRIPTION Database StationID of the external station.

DESCRIPTION Given the DB StationID of the external station(component), 
the function will return find and return the idChan for it.

NOTE This function uses a "cheater table" mechanism to convert
from a StationID to an idChan.  This "cheater table" has theoretically
been setup beforehand by the DB operator, to map the names on the data
channels coming in, to idChans.  This is a task that must be done by
the operator, because it usually requires a priori knowledge about
what names refer to what channels(since more than one channel may
have the same name).

*************************************************
************************************************/
int ewdb_api_GetIdChanFromStationExternal(EWDBid * pidChan, int IN_StationID);



/* TO BE COMMENTED */
int ewdb_api_UH_CreateExternalInfraRecord (EWDB_External_UH_InfraInfo *pUHInfo);
int     ewdb_api_UH_GetExternalInfraRecord (int idChanT,
        EWDB_External_UH_InfraInfo *pUHInfo, int BufferLen,
        int *NumFound, int *NumRetrieved);

int ewdb_api_SelectExternalStation (EWDB_External_StationStruct *pStation);


#endif /* _EWDB_ORA_API_EXTERNAL_H */
