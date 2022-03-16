/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: ewdb_ora_api_misc.h 7322 2018-04-14 03:27:27Z baker $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2008/12/02 21:37:42  paulf
 *     oracle surgery additions
 *
 *     Revision 1.3  2004/12/01 20:51:33  mark
 *     Additional polygon functions
 *
 *     Revision 1.2  2004/11/23 17:27:24  mark
 *     Added polygon structs and prototypes
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
 * definitions and function prototypes for the misc
 * subschema.   
 * 
 * This file should never be included in an application
 * program.  Instead, include the master include file
 * ewdb_ora_api.h
 * 
 *    Lucky Vidmar, May 20, 2003
 *
 *******************************************************/

#ifndef _EWDB_ORA_API_MISC_H
#define _EWDB_ORA_API_MISC_H



/**********************************************************
 #########################################################
    Struct Definition Section
 #########################################################
**********************************************************/


/***************************** NEAREST TOWN SECTION *****************************/

/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY MISC_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_PlaceStruct
TYPE_DEFINITION struct _EWDB_PlaceStruct
DESCRIPTION Towns and Places structure

MEMBER idPlace
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID for the place.

MEMBER szState
MEMBER_TYPE char[3]
MEMBER_DESCRIPTION two-letter State code 

MEMBER szPlaceName
MEMBER_TYPE char[70]
MEMBER_DESCRIPTION  Name of the place

MEMBER szPlaceType
MEMBER_TYPE char[20]
MEMBER_DESCRIPTION  Type of the place

MEMBER szCountry
MEMBER_TYPE char[30]
MEMBER_DESCRIPTION  Country of the place

MEMBER szCounty
MEMBER_TYPE char[30]
MEMBER_DESCRIPTION  County of the place

MEMBER dLat
MEMBER_TYPE double
MEMBER_DESCRIPTION  Latitude

MEMBER dLon
MEMBER_TYPE double
MEMBER_DESCRIPTION  Longitude

MEMBER dElev
MEMBER_TYPE double
MEMBER_DESCRIPTION  Elevation

MEMBER iPopulation
MEMBER_TYPE int
MEMBER_DESCRIPTION  Population

MEMBER iPlaceMajorType
MEMBER_TYPE int
MEMBER_DESCRIPTION  Major Type (City, Dam, etc...)

MEMBER iPlaceMinorType
MEMBER_TYPE int
MEMBER_DESCRIPTION  Minor Type (large city, small city...)

*************************************************
************************************************/
typedef struct _EWDB_PlaceStruct
{
	int    idPlace;
	char   szState[3];
	char   szPlaceName[70];
	char   szPlaceType[20];
	char   szCounty[30];
	char   szCountry[30];
	double dLat;
	double dLon;
	double dElev;
	int    iPopulation;
	int    iPlaceMajorType;
	int    iPlaceMinorType;

	/* These are not in the DB, but are filled upon retrieval and
		comparisson with the origin */
	double dDist;
	double dAzm;
}  EWDB_PlaceStruct;


#define	EWDB_PLACE_TYPE_GENERIC_CITY	1

#define EWDB_CITY_TYPE_UNKNOWN 		0
#define EWDB_CITY_TYPE_CDP     		5
#define EWDB_CITY_TYPE_CITY    		4
#define EWDB_CITY_TYPE_LARGE_CITY 	3
#define EWDB_CITY_TYPE_METROPOLIS 	2
#define EWDB_CITY_TYPE_MEGALOPOLIS  1


/*****************************	POLYGON SECTION *****************************/


typedef struct _EWDB_PolygonStruct
{
	EWDBid idPolygon;
	char   szPolygonName[1024];
} EWDB_PolygonStruct;


typedef struct _EWDB_PolygonVertexStruct
{
	EWDBid idVertex;
	EWDBid idPolygon;
	double dLat;
	double dLon;
	int    iOrder;
} EWDB_PolygonVertexStruct;


/**********************************************************
 #########################################################
    Function Prototype Section
 #########################################################
**********************************************************/

/* TO COMMENT */
int ewdb_api_CreatePlace (EWDB_PlaceStruct *pPlace);
int ewdb_api_GetPlaceList (EWDB_PlaceStruct *pBuffer, int BufferLen,
		EWDB_PlaceStruct *pMinPlace, EWDB_PlaceStruct *pMaxPlace, 
		int *pNumRet, int *pNumFound);

int ewdb_api_CreatePolygonVert(EWDB_PolygonVertexStruct *pPolygonVert);
int ewdb_api_CreatePolygon(EWDB_PolygonStruct *pPolygon);
int ewdb_api_GetPolygonVertices(EWDBid idPolygon, EWDB_PolygonVertexStruct *pBuffer,
                              int *pNumFound, int *pNumRetrieved, int BufferLen);
int ewdb_api_GetPolygons(EWDB_PolygonStruct *pBuffer,
                              int *pNumFound, int *pNumRetrieved, int BufferLen);
int ewdb_api_DeletePolygon(EWDBid idPolygon);

#endif /* _EWDB_ORA_API_MISC_H */
