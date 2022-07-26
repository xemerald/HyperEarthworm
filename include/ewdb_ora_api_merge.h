/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: ewdb_ora_api_merge.h 3471 2008-12-02 21:37:42Z paulf $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2008/12/02 21:37:42  paulf
 *     oracle surgery additions
 *
 *     Revision 1.2  2004/11/23 17:58:05  mark
 *     Added newline to end of file
 *
 *     Revision 1.1  2004/09/09 06:00:23  davidk
 *     Moved Merge/Coincidence structures/functions from parametric to newly added
 *     "merge" subdir/schema.
 *
 *
 ***********************************************************/

/*******************************************************
 *
 * This include file contains only the structure
 * definitions and function prototypes for the merge
 * subschema.   
 * 
 * This file should never be included in an application
 * program.  Instead, include the master include file
 * ewdb_ora_api.h
 * 
 *    Lucky Vidmar, May 20, 2003
 *
 *******************************************************/

#ifndef _EWDB_ORA_API_MERGE_H
#define _EWDB_ORA_API_MERGE_H

/**********************************************************
 #########################################################
    Struct Definition Section
 #########################################################
**********************************************************/
/***************************** COINCIDENCE EVENT SECTION *****************************/

/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_CoincEventStruct
TYPE_DEFINITION struct _EWDB_CoincEventStruct
DESCRIPTION Coincidence Trigger event structure.

MEMBER idCoincidence
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID for the Coincidence.

MEMBER tCoincTrigger
MEMBER_TYPE double
MEMBER_DESCRIPTION Time of the event trigger

MEMBER idEvent
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database identifier of the Event with which this
Coincidence is associated.

MEMBER bBindToEvent
MEMBER_TYPE int
MEMBER_DESCRIPTION Boolean flag indicating whether or not this
Magnitude should be bound to an Event when it is inserted into the DB.
If yes, it is associated with the Event identified by idEvent upon
insertion into the DB.


MEMBER szSource
MEMBER_TYPE char[256]
MEMBER_DESCRIPTION Identifier of the source/author that declared the
Coincidence.  This could be the Logo of the Earthworm module that generated
the Coincidence, or the login name of an analyst that reviewed it.  

MEMBER szComment
MEMBER_TYPE char *
MEMBER_DESCRIPTION Optional comment associated with the magnitude.

*************************************************
************************************************/
typedef struct _EWDB_CoincEventStruct
{
	EWDBid  idCoincidence;
	EWDBid  idEvent;
	double	tCoincidence;
	int		bBindToEvent;
	char	szSource[256]; 
	char	szHumanReadable[256]; 
	char 	*szComment;
} EWDB_CoincEventStruct;



/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_TriggerStruct
TYPE_DEFINITION struct _EWDB_TriggerStruct
DESCRIPTION Single Channel Triggerstructure.

MEMBER idTrigger
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID for the Coincidence.

MEMBER tTrigger
MEMBER_TYPE double
MEMBER_DESCRIPTION Time of the trigger

MEMBER idCoincidence
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database identifier of the Coincidence Event with 
which this trigger is associated.

MEMBER idChan
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database identifier of the channel with which this
trigger is associated.

*************************************************
************************************************/
typedef struct _EWDB_TriggerStruct
{
	EWDBid  idTrigger;
	EWDBid  idCoincidence;
	EWDBid  idChan;
	double	tTrigger;
	double	dDist;
} EWDB_TriggerStruct;


/***************************** EVENT MERGE SECTION *****************************/

/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_MergeStruct
TYPE_DEFINITION struct _EWDB_MergeStruct
DESCRIPTION Event Merge info structure.

MEMBER idMerge
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID for the Merge.

MEMBER idEvent
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database identifier of the Event which is to be merged. 

MEMBER idPh
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database identifier of the Phenomenon which is to 
be merged with the event.

MEMBER szSource
MEMBER_TYPE char[256]
MEMBER_DESCRIPTION Identifier of the source/author that declared the
Merge.  This could be the Logo of the Earthworm module that generated
the Merge, or the login name of an analyst that reviewed it.  

MEMBER szHumanReadable
MEMBER_TYPE char[256]
MEMBER_DESCRIPTION Huamn readable string identifying the person or
program that declared the Merge.

MEMBER szComment
MEMBER_TYPE char *
MEMBER_DESCRIPTION Optional comment associated with the magnitude.

*************************************************
************************************************/
typedef struct _EWDB_MergeStruct
{
	EWDBid  idMerge;
	EWDBid  idPh; 			/* phenomenon */
	EWDBid  idEvent;
	char	szSource[256]; 
	char	szHumanReadable[256]; 
	char 	*szComment;
} EWDB_MergeStruct;



/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_PhenomenonStruct
TYPE_DEFINITION struct _EWDB_PhenomenonStruct
DESCRIPTION Phenomenon structure

MEMBER idPh
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID for the phenomenon.

MEMBER idEvent
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database identifier of the Event which first 
gave rise to this phenomenon. 

MEMBER szSource
MEMBER_TYPE char[256]
MEMBER_DESCRIPTION Identifier of the source/author that declared the
Merge.  This could be the Logo of the Earthworm module that generated
the Merge, or the login name of an analyst that reviewed it.  

MEMBER szHumanReadable
MEMBER_TYPE char[256]
MEMBER_DESCRIPTION Huamn readable string identifying the person or
program that declared the Merge.

MEMBER szComment
MEMBER_TYPE char *
MEMBER_DESCRIPTION Optional comment associated with the merge.

*************************************************
************************************************/
typedef struct _EWDB_PhenomenonStruct
{
	EWDBid  idPh;
	EWDBid  idPrefEvent; 		/* preferred solution of this phenomenon */
	char	szSource[256]; 
	char	szHumanReadable[256]; 
	char 	*szComment;
} EWDB_PhenomenonStruct;


/* Newest stuff -- TO BE COMMENTED */
typedef struct _MergeInfo
{
    EWDBid  idEvent;
    EWDBid  idPrefEvent; /* event giving rise to this phenomenon */
    EWDBid  idPh;
    EWDBid  idMerge;
    double  tOrigin;
    float   dLat;
    float   dLon;
    float   dDepth;
    float   dPrefMag;
	char	szSource[256]; 
	char	szHumanReadable[256]; 
} EWDB_MergeList;


/* defines needed for the merging code */
#define CORE_TABLE_ORIGIN		4
#define CORE_TABLE_MAGNITUDE 	5
#define CORE_TABLE_MECHFM 		6
#define CORE_TABLE_WAVEFORMDESC	27
#define CORE_TABLE_COINCIDENCE	50


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

SUB_LIBRARY MERGE_API

LOCATION THIS_FILE

DESCRIPTION This is a portion of the EWDB_API_LIB
that deals with merging parametric events.

*************************************************
************************************************/



/* Merge/Coincidence */
int ewdb_api_CreateCoincidence (EWDB_CoincEventStruct *pCoinc);
int ewdb_api_CreateTrigger(EWDB_TriggerStruct *pTrigger);
int ewdb_api_GetCoincidence(EWDBid idEvent, int EvtType, 
				EWDB_CoincEventStruct *pCoinc,
				int NumAlloc, double StartTime, double EndTime, 
				int *pNumFound, int *pNumRetr);
int ewdb_api_GetTriggerList (EWDBid idCoincidence, EWDB_TriggerStruct *pTrigs,
                            int NumAlloc, double StartTime, double EndTime,
                            int *pNumFound, int *pNumRetr);
int ewdb_api_CreateMerge (EWDB_MergeStruct *pMerge);
int ewdb_api_DeleteMerge (EWDBid idPh, EWDBid idEvent);
int ewdb_api_DeletePhenomenon (EWDBid idPh);
int ewdb_api_CreatePhenomenon (EWDB_PhenomenonStruct *pPhenomenon);
int ewdb_api_GetMergeList (EWDB_MergeList *pMerge, int BufferLen, EWDBid idPh,
                          EWDB_MergeList *pMin, EWDB_MergeList *pMax,
                            int *NumFound, int *NumRetr);

#endif /* EWDB_ORA_API_MERGE_H */

