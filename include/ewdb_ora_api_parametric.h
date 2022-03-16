/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: ewdb_ora_api_parametric.h 7322 2018-04-14 03:27:27Z baker $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2008/12/02 21:37:42  paulf
 *     oracle surgery additions
 *
 *     Revision 1.28  2005/06/17 21:07:11  davidk
 *     Added API function for retrieving CodaDurs by PickID.
 *
 *     Revision 1.27  2005/05/12 20:28:40  mark
 *     Added comments functions
 *
 *     Revision 1.26  2005/03/31 18:51:07  davidk
 *     Changed the EWDB_MwStruct:
 *       Added dM0 (doh!)
 *       Changed dPercentCVLD to dPercentCLVD
 *
 *     Revision 1.25  2005/03/30 18:25:22  davidk
 *     Added ewdb_api_GetUnassocPicksByTime() and ewdb_api_GetUnassocPicksByTimeWChan()
 *
 *     Revision 1.24  2005/03/22 23:41:07  davidk
 *     Changed a variable name in Mw function prototype.
 *
 *     Revision 1.23  2005/03/22 17:18:33  davidk
 *     Changed some Mw structure field names, function attributes, and comments.
 *
 *     Revision 1.22  2005/03/19 01:54:57  davidk
 *     Updated field names in Mw structures.
 *
 *     Revision 1.21  2005/03/18 21:58:40  davidk
 *     Modified structures for Mw.
 *     Added prototypes for functions for retrieving Mw related data.
 *
 *     Revision 1.20  2005/03/18 00:31:39  davidk
 *     Fixed compiler errors in the previous version.
 *
 *     Revision 1.19  2005/03/18 00:24:39  davidk
 *     Added structures and function prototypes for Mw.
 *     Added redesigned structures and function prototypes for Mwp.
 *
 *     Revision 1.18  2004/11/06 04:22:57  davidk
 *     Updated EWDB_ArrivalStruct to utilize the new constant EWDB_PHASENAME_SIZE
 *     for it's field sizes for szObsPhase and szCalcPhase.
 *     (The current value of EWDB_PHASENAME_SIZE  is 10)  The old size in the struct was 6.
 *
 *     Revision 1.17  2004/09/27 21:52:33  davidk
 *     Added EWDB_AuthorStruct for source/author information.
 *     Added ewdb_api_GetSourceByID()
 *
 *     Revision 1.16  2004/09/09 17:21:10  davidk
 *     Added ewdb_api_Delete* function protototypes.
 *
 *     Revision 1.15  2004/09/09 06:00:23  davidk
 *     Moved Merge/Coincidence structures/functions from parametric to newly added
 *     "merge" subdir/schema.
 *
 *     Revision 1.14  2004/08/20 21:43:33  davidk
 *     Added prototype for ewdb_api_GetIDSource().
 *
 *     Revision 1.13  2004/08/18 21:36:00  davidk
 *     Added prototype for ewdb_api_GetPicksByTime()
 *
 *     Revision 1.12  2004/08/10 17:28:56  michelle
 *     added declarations for
 *     ewdb_api_CreateUnassocPick
 *     ewdb_api_GetEventID
 *     ewdb_api_CreatePeakAmp
 *     ewdb_api_GetPickByExternalID
 *     ewdb_api_GetidOriginFromVersionNum
 *
 *     Revision 1.11  2004/07/19 17:48:55  davidk
 *     Added prototypes for ewdb_api_GetAmpsForOriginByPick(), and
 *     ewdb_api_CreateStaMag().  CreateStaMag is not new but the prototype had been missing.
 *
 *     Revision 1.10  2004/05/22 18:40:12  davidk
 *     changed iMagType to MAGNITUDE_TYPE from int.
 *
 *     Revision 1.9  2004/05/11 01:09:13  davidk
 *     Added prototypes for two new functions used to retrieve amplitude info for an origin.
 *     ewdb_api_GetAmpsForOriginWChanInfo() and ewdb_api_GetAmpsForOriginMagWChanInfo()
 *
 *     Revision 1.8  2004/05/05 16:22:26  davidk
 *     Added prototype w/comment for ewdb_api_GetArrivalsWChanInfo().
 *     Modified comment for ewdb_api_GetArrivals().
 *
 *     Revision 1.7  2004/01/19 22:01:08  davidk
 *     Added prototype and comment for ewdb_ora_api_parametric.h().  Modified the
 *     comment for ewdb_GetEventInfo(), to indicate that some fields of the
 *     EWDB_EventStruct are not filled in by the function.
 *
 *     Revision 1.6  2003/09/04 19:56:05  lucky
 *     Changed AMPLITUDE_TYPE to MAGNITUDE_TYPE
 *
 *     Revision 1.5  2003/08/28 22:55:04  lucky
 *     Added prototype for ewdb_api_GetAmpsByPick
 *
 *     Revision 1.4  2003/08/25 17:59:14  lucky
 *     Added tStartInterval and tEndInterval to PeakAmpStruct
 *
 *     Revision 1.3  2003/08/25 17:57:15  michelle
 *     added function declaration for GetPickByID
 *
 *     Revision 1.2  2003/06/03 18:43:15  lucky
 *     Removed Starting lat, lon, depth
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
 * definitions and function prototypes for the parametric
 * subschema.   
 * 
 * This file should never be included in an application
 * program.  Instead, include the master include file
 * ewdb_ora_api.h
 * 
 *    Lucky Vidmar, May 20, 2003
 *
 *******************************************************/

#ifndef _EWDB_ORA_API_PARAMETRIC_H
#define _EWDB_ORA_API_PARAMETRIC_H



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

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_ArrivalStruct
TYPE_DEFINITION struct _EWDB_ArrivalStruct
DESCRIPTION Arrival structure describing information about a phase,
possibly with information relating the phase to an Origin.

MEMBER idPick
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of the Pick.

MEMBER idOriginPick
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of the OriginPick, the record relating
the Pick(phase) to an Origin.

MEMBER idOrigin
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of the Origin with which this phase is associated.

MEMBER idChan
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of the channel that this pick came from.

MEMBER pStation
MEMBER_TYPE EWDB_StationStruct
MEMBER_DESCRIPTION Station/Component information for the channel
associated with this phase.

NOTE The fields szCalcPhase - tResPick are only legitimate if a phase
is associated with an Origin.  If idOrigin is not set then these feels
should be ignored.

MEMBER szCalcPhase
MEMBER_TYPE char[EWDB_PHASENAME_SIZE]
MEMBER_DESCRIPTION The calculated type of the Phase: P, S, PKP(sp?),
and others up to 5 chars.  This values is assigned by the phase
associator or locator that created the Origin that this phase is
associated with.  This is not the phase type given by the picker.

MEMBER tCalcPhase
MEMBER_TYPE double
MEMBER_DESCRIPTION The onset time of the phase, according to the
associator/locator.

MEMBER dWeight
MEMBER_TYPE float
MEMBER_DESCRIPTION The weight of the phase pick in calculating the Origin.

MEMBER dWeight
MEMBER_TYPE float
MEMBER_DESCRIPTION The weight of the phase pick in calculating the Origin.

MEMBER dDist
MEMBER_TYPE float
MEMBER_DESCRIPTION The epicentral distance to the station where the phase
was observed.

MEMBER dAzm
MEMBER_TYPE float
MEMBER_DESCRIPTION Azimuthal direction from the station where the phase
was observed to the epicenter.  Expressed as degrees east of North.

MEMBER dTakeoff
MEMBER_TYPE float
MEMBER_DESCRIPTION Takeoff angle of the Phase from the hypocenter.

MEMBER tResPick
MEMBER_TYPE float
MEMBER_DESCRIPTION Pick residual in seconds.

MEMBER szExtSource
MEMBER_TYPE char[16]
MEMBER_DESCRIPTION Reserved

MEMBER szExternalPickID
MEMBER_TYPE char[16]
MEMBER_DESCRIPTION Reserved

MEMBER szExtPickTabName
MEMBER_TYPE char[16]
MEMBER_DESCRIPTION Reserved

MEMBER szObsPhase
MEMBER_TYPE char[EWDB_PHASENAME_SIZE]
MEMBER_DESCRIPTION The type of the Phase:  P, S, PKP(sp?), and others
up to 5 chars.  This values is assigned by the picker(the entity that
observed the phase arrival).

MEMBER tObsPhase
MEMBER_TYPE double
MEMBER_DESCRIPTION The onset time of the phase, according to the picker.

MEMBER cMotion
MEMBER_TYPE char
MEMBER_DESCRIPTION The initial motion of the phase: U (up), D (down).  

MEMBER cOnset
MEMBER_TYPE char
MEMBER_DESCRIPTION The onset type of the phase:  

MEMBER dSigma
MEMBER_TYPE double
MEMBER_DESCRIPTION Potential Phase error.  Expressed in seconds.

*************************************************
************************************************/
typedef struct _EWDB_ArrivalStruct
{    
	EWDBid  idPick;
	EWDBid  idOriginPick;
	EWDBid  idOrigin;
	EWDBid  idChan;
  EWDB_StationStruct * pStation;
  char    szCalcPhase[EWDB_PHASENAME_SIZE]; /* name of phase */
  double  tCalcPhase;  /* time of phase */
  float   dWeight;   /* weight of pick in Origin calc */
  float   dDist;     /* distance(km) from epicenter */
  float   dAzm;      /* Azimuth(deg from N.) to epicenter */
  float   dTakeoff;  /* Takeoff angle of incoming waves */
  double  tResPick;  /* Pick residual in seconds */
  char    szExtSource[16];  /* Source that produced this pick */
  char    szExternalPickID[16];  /* Source's Pick ID */
  char    szExtPickTabName[16];  /* Source's Pick Table */
  char    szObsPhase[EWDB_PHASENAME_SIZE]; /* name of phase */
  double  tObsPhase;  /* time of phase */
  char    cMotion; /* first motion U,D,X */
  char    cOnset;  /* how steep the first wave was? */
  double  dSigma;  /* quality of the pick sigma interval(secs) */
} EWDB_ArrivalStruct;


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_CodaAmplitudeStruct
TYPE_DEFINITION struct _EWDB_CodaAmplitudeStruct
DESCRIPTION Information for average amplitude measurments of a
coda in support of a coda termination measurement/calculation.

MEMBER tOn
MEMBER_TYPE double[EWDB_MAXIMUM_AMPS_PER_CODA]
MEMBER_DESCRIPTION Array of window starting times for coda average
amplitude measurements.  Expressed as seconds since 1970.

MEMBER tOff
MEMBER_TYPE double[EWDB_MAXIMUM_AMPS_PER_CODA]
MEMBER_DESCRIPTION Array of window ending times for coda average
amplitude measurements.  Expressed as seconds since 1970.

MEMBER iAvgAmp
MEMBER_TYPE int[EWDB_MAXIMUM_AMPS_PER_CODA]
MEMBER_DESCRIPTION Array of average amplitude values for coda average
amplitude measurements.  Expressed as digital counts.  tOn[x] and
tOff[x] describe a time window during a coda at a component. AvgAmp[x]
is the average wave amplitude during that window.  A series of timed
average amplitude measurements can be combined with a decay algorithm
to estimate the coda termination time.

*************************************************
************************************************/
typedef struct _EWDB_CodaAmplitudeStruct
{
  /* AmpCoda part */
  double  tOn[EWDB_MAXIMUM_AMPS_PER_CODA];
  double  tOff[EWDB_MAXIMUM_AMPS_PER_CODA];
  int     iAvgAmp[EWDB_MAXIMUM_AMPS_PER_CODA];
}  EWDB_CodaAmplitudeStruct;


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_PeakAmpStruct
TYPE_DEFINITION struct _EWDB_PeakAmpStruct
DESCRIPTION Information for the Peak Amplitude measurement
associated with a station magnitude calculation.

MEMBER idPeakAmp
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION DB ID of the Peak Amplitude record for this amplitude
structure.

MEMBER idChan
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of the channel that this pick came from.

MEMBER szExtSource
MEMBER_TYPE char[16]
MEMBER_DESCRIPTION Source where this amplitude was produced

MEMBER szExternalAmpID
MEMBER_TYPE char[16]
MEMBER_DESCRIPTION ID of the amplitude at the source

MEMBER dAmp1
MEMBER_TYPE float
MEMBER_DESCRIPTION 1st Amplitude measurement(millimeters).

MEMBER dAmpPeriod1
MEMBER_TYPE float
MEMBER_DESCRIPTION Time period of the first measurement.

MEMBER tAmp1
MEMBER_TYPE double
MEMBER_DESCRIPTION Starting time of the first amplitude measurement.  
Expressed as seconds since 1970.

MEMBER dAmp2
MEMBER_TYPE float
MEMBER_DESCRIPTION 2nd Amplitude measurement(millimeters).

MEMBER dAmpPeriod2
MEMBER_TYPE float
MEMBER_DESCRIPTION Time period of the second measurement.

MEMBER tAmp2
MEMBER_TYPE double
MEMBER_DESCRIPTION Starting time of the second amplitude measurement.  
Expressed as seconds since 1970.

MEMBER iAmpType
MEMBER_TYPE int
MEMBER_DESCRIPTION Type of amplitude.  


*************************************************
************************************************/
typedef struct _EWDB_PeakAmpStruct
{
  /* PeakAmp part */
  EWDBid   idPeakAmp;  /* same as idDatum */
  EWDBid   idChan;
  EWDBid   idPick;
  char    szExtSource[16];  		/* Source that produced this amp */
  char    szExternalAmpID[16];  	/* Source's amplitude ID */
  float   dAmp1;
  float   dAmpPeriod1;
  double  tAmp1;
  float   dAmp2;
  float   dAmpPeriod2;
  double  tAmp2;
  int     iAmpType;

  /* 
   * We (optionally) keep track of the interval over which this
   * amp record was computed
   */
  double  tStartInterval;
  double  tEndInterval;
} EWDB_PeakAmpStruct;


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_CodaDurationStruct
TYPE_DEFINITION struct _EWDB_CodaDurationStruct
DESCRIPTION Information for the Coda Duration measurement
associated with a station magnitude calculation.

MEMBER idChan
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of the channel that this pick came from.

MEMBER idTCoda
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION DB ID of the coda termination record.

MEMBER tCodaTermObs
MEMBER_TYPE double
MEMBER_DESCRIPTION Time when the coda associated with this station
duration magnitude, was observed to have ended.  Time is expressed as
seconds since 1970.  *NOTE*: In general, this value should not be used.  
It is provided as a reference in case tCodaTermXtp contains an extrapolated
value.  The value of tCodaTermXtp should always be a better value.  When
recalculating a coda termination time, the results should be written to
tCodaTermXtp.)

MEMBER tCodaTermXtp
MEMBER_TYPE double
MEMBER_DESCRIPTION Time when the coda associated with this station
duration magnitude ended.  This will be the better of either an
observed coda termination, or one calculated based on average amplitude
values and a decay rate.  Time is expressed as seconds since 1970.

Coda Duration

MEMBER tCodaDurObs
MEMBER_TYPE double
MEMBER_DESCRIPTION Length of the coda (in seconds) from the time of the
P-pick to the time of the OBSERVED coda termination.  This value should
not be used for the coda estimate, use tCodaDurXtp instead.  
See the tCodaTermObs comment.

MEMBER tCodaDurXtp
MEMBER_TYPE double
MEMBER_DESCRIPTION Length of the coda (in seconds) from the time of the
P-pick to the time of the coda termination.  This value is based upon
tCodaTermXtp and P-pick times.

MEMBER idCodaDur
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION DB ID of the coda duration record calculated by
associated a Phase pick with a coda termination.

MEMBER idPick
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION DB ID of the Phase arrival(pick) that is associated
with a Coda termination from the same channel, to produce the coda
duration(s) that this station magniutde is calculated from.

MEMBER pCodaAmp
MEMBER_TYPE EWDB_CodaAmplitudeStruct *
MEMBER_DESCRIPTION Pointer to a structure of Coda Average
Amplitude Values, used to calculate the coda termination time.

*************************************************
************************************************/
typedef struct _EWDB_CodaDurationStruct
{

  EWDBid   idChan;

  /* TCoda part */
  EWDBid   idTCoda;
  double   tCodaTermObs;  /* IGNORE */
  double   tCodaTermXtp; 

  double   tCodaDurObs;   /* IGNORE */
  double   tCodaDurXtp;

  /* CodaDur part */
  EWDBid  idCodaDur;  /* same as idDatum */
  EWDBid  idPick;

  /* Coda Amplitude Information for extrapolated codas */
  EWDB_CodaAmplitudeStruct * pCodaAmp;

}  EWDB_CodaDurationStruct;



typedef struct _EWDB_IntDispStruct
{
  EWDBid idIntDisp;
  int    iMagType;
  double dIntDisp;
  double tIntPeak;
  double tIntWindow;
}  EWDB_IntDispStruct;



/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_StationMagUnion
TYPE_DEFINITION union _EWDB_StationMagUnion
DESCRIPTION Structure of Station Magnitude information.

MEMBER PeakAmp
MEMBER_TYPE EWDB_PeakAmpStruct
MEMBER_DESCRIPTION Measurement information for an Amplitude
measurement.

MEMBER CodaDur
MEMBER_TYPE EWDB_CodaDurationStruct
MEMBER_DESCRIPTION Measurement information for a Coda
Duration measurement.

*************************************************
************************************************/
typedef union _EWDB_StationMagUnion   /* existing - modified for Mwp */
{
  EWDB_PeakAmpStruct PeakAmp;
  EWDB_CodaDurationStruct CodaDur;
  EWDB_IntDispStruct IntDisp;
}  EWDB_StationMagUnion;


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_StationMagStruct
TYPE_DEFINITION struct _EWDB_StationMagStruct
DESCRIPTION Structure of Station Magnitude information.

MEMBER pStationInfo
MEMBER_TYPE EWDB_StationStruct *
MEMBER_DESCRIPTION Station information for the station for which
the magnitude was calculated.

MEMBER idChan
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of the channel that this pick came from.

MEMBER pStationInfo
MEMBER_TYPE EWDB_StationStruct
MEMBER_DESCRIPTION Station/Component information for the channel
associated with this phase.

MEMBER idMagLink
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION DB ID of the station magnitude.

MEMBER idMagnitude
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION DB ID of the summary magnitude.

MEMBER iMagType
MEMBER_TYPE int
MEMBER_DESCRIPTION Type of magnitude.  See rw_mag.h for a list of Magnitude types.

MEMBER dMag
MEMBER_TYPE float
MEMBER_DESCRIPTION Station magnitude.

MEMBER dWeight
MEMBER_TYPE float
MEMBER_DESCRIPTION Weight of the Station Magnitude in calculating the
summary magnitude this Station Magnitude is associated with.

MEMBER idDatum
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION DB ID of the measurement that the station magnitude
is calculated from.  The measurement could be a Peak Amplitude or Coda Duration.

MEMBER StaMagUnion
MEMBER_TYPE EWDB_StationMagUnion
MEMBER_DESCRIPTION Information about the measurement upon which the 
magnitude is based.  This could be either Peak Amplitude information
for an Amplitude based magnitude, or Coda Duration information for
a Duration based magnitude.

NOTE: When both an observed(dMeasurementObs) and a
calculated(dMeasurementCalc) measurement exist the method for
determining which one the magnitude is derived from is undefined.

*************************************************
************************************************/
typedef struct _EWDB_StationMagStruct
{
  EWDBid  idChan;
  EWDB_StationStruct * pStationInfo;

  /* MagLink part */
  EWDBid  idMagLink;
  EWDBid  idMagnitude;
  int     iMagType;
  float   dMag;
  float   dWeight;
  EWDBid  idDatum;

  /* Datum Info */
  EWDB_StationMagUnion StaMagUnion;

} EWDB_StationMagStruct;


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_EventStruct
TYPE_DEFINITION struct _EWDB_EventStruct
DESCRIPTION Structure containing information about a Database Event.

MEMBER idEvent
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID for the event

MEMBER iEventType
MEMBER_TYPE int
MEMBER_DESCRIPTION Human assigned event-type (quake, avalanche, 
volcano, nuclear detination).  Currently the only types supported
are Quake and Unknonwn.  See EWDB_EVENT_TYPE_QUAKE for more information.

MEMBER iDubiocity
MEMBER_TYPE int
MEMBER_DESCRIPTION Describes the perceived validity of the Event.  The
Dubiocity range is currently defined as TRUE/FALSE with TRUE indicating
a dubious event.

MEMBER bArchived
MEMBER_TYPE int
MEMBER_DESCRIPTION Indicates whether or not the Event has already been
archived from the database to disk.

MEMBER szComment
MEMBER_TYPE char[512]
MEMBER_DESCRIPTION Description of and comments about the Event.  If present, it will
be the first 512 characters of the comment stored in idPublicComment.  For the full
public comment, call ewdb_apps_GetFullComment(idPublicComment), or repeatedly call
ewdb_api_GetComment(idPublicComment);

MEMBER szSource
MEMBER_TYPE char[256]
MEMBER_DESCRIPTION Identifier of the source/author that declared the
Event.  Only one Source is listed here.  If more than one Source
contributed to the declaration of the Event, then the manner by which
this Source is chosen as THE Source is undefined.  This could be the
Logo of the Earthworm module that generated the Event, or the or the
login name of an analyst that reviewed it.

MEMBER szSourceName
MEMBER_TYPE char[256]
MEMBER_DESCRIPTION Name of the source/author that declared the Event.
The SourceName is a human readable version of the Source.  The
SourceName could be the name of the network that generated the event,
or the name of an Analyst that reviewed it.

MEMBER szSourceEventID
MEMBER_TYPE char[256]
MEMBER_DESCRIPTION Event identifer assigned to this event by the
Source.  This would be the EventID given by and Earthworm Binder Module
to an automatic event that was generated by an Earthworm.

MEMBER idPublicComment
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID for comment associated with this event.  This comment is
	public, and may be published with the event.

MEMBER idInternalComment
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID for comment associated with this event.  This comment is
	private and internal, and will not be published with the event.

MEMBER idContribMagComment
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID for comment associated with this event.  This comment is
	any contributed magnitudes for the event; this will be one or more of:
		<mag type> <magnitude> <contributing organization>
	all in one string.

*************************************************
************************************************/
typedef struct _EWDB_EventStruct
{
    EWDBid  idEvent; 
    int     iEventType; 
    int     iDubiocity;
    int     bArchived;
    char    szComment[512]; 
    char    szSource[256];  
    char    szSourceName[256];  
    char    szSourceEventID[256];
	EWDBid	idPublicComment;
	EWDBid	idInternalComment;
	EWDBid	idContribMagComment;

} EWDB_EventStruct;



/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_MagStruct
TYPE_DEFINITION struct _EWDB_MagStruct
DESCRIPTION Summary magnitude struct.

MEMBER idMag
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID for the Magnitude.

MEMBER iMagType
MEMBER_TYPE int
MEMBER_DESCRIPTION Magnitude type: Duration, Local, BodyAmp, Surface, etc.
This is an enumerated value that identifies a magnitude type.  The possible
enumerated values are defined in rw_mag.h.

MEMBER dMagAvg
MEMBER_TYPE float
MEMBER_DESCRIPTION The summary magnitude.  For most magnitude types, this
is computed as a weighted average of a bunch of station magnitudes.

MEMBER iNumMags
MEMBER_TYPE int
MEMBER_DESCRIPTION Number of station magnitudes used to compute the
dMagAvg.  For a Moment magnitude, this maybe the number of reporting
stations that were used in calculating the moment.

MEMBER dMagErr
MEMBER_TYPE float
MEMBER_DESCRIPTION The potential error of the summary magnitude.
CLEANUP!  Detailed units, meaning?  I believe this is the Median
Absolute Deviation of the station mags.

MEMBER idEvent
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database identifier of the Event with which this
Magnitude is associated.

MEMBER bBindToEvent
MEMBER_TYPE int
MEMBER_DESCRIPTION Boolean flag indicating whether or not this
Magnitude should be bound to an Event when it is inserted into the DB.
If yes, it is associated with the Event identified by idEvent upon
insertion into the DB.

MEMBER bSetPreferred
MEMBER_TYPE int
MEMBER_DESCRIPTION Boolean flag indicating whether or not this
Magnitude should be set as the Preferred Magnitude for the Event
identified by idEvent.  This flag is ignored if bBindToEvent is FALSE.

MEMBER idOrigin
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Optional Database identifier of the Origin 
that this Magnitude was calculated from.

MEMBER szSource
MEMBER_TYPE char[256]
MEMBER_DESCRIPTION Identifier of the source/author that declared the
Origin.  This could be the Logo of the Earthworm module that generated
the Origin, or the login name of an analyst that reviewed it.  CLEANUP
Should this be the human readable version of the source name??  It
should be consistent with Origin and Mechanism.

MEMBER szComment
MEMBER_TYPE char *
MEMBER_DESCRIPTION Optional comment associated with the magnitude.
This is provided primarily as an option for later development.  Most
functions that handle the EWDB_MagStruct, do not know/care about comments.

*************************************************
************************************************/
typedef struct _EWDB_MagStruct
{
	EWDBid  idMag;          /* database ID for this magnitude */
	int     iMagType;       /* enumerated magnitude type.  See rw_mag.h for the 
                             list of possible values */
	float	  dMagAvg;			  /* summary magnitude value */
	int		  iNumMags;		    /* number of stations used to compute magAvg */
	float	  dMagErr;
	EWDBid  idEvent;		    /* database ID of the associated event */
	int		  bBindToEvent;	  /* should magnitude be bound to this event */
	int		  bSetPreferred;  /* is this the preferred magnitude */
	EWDBid  idOrigin;       /* database ID of the Origin upon which
                             this magnitude is based */
	char	  szSource[256];  /* Author of this magnitude */
	char *  szComment;      /* Optional comment for this magnitude.
                             PROBABLY NOT SUPPORTED */
} EWDB_MagStruct;


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_OriginStruct
TYPE_DEFINITION struct _EWDB_OriginStruct
DESCRIPTION Structure containing information about an Event Origin.
Includes error information.

MEMBER idOrigin
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database identifier of the Origin.

MEMBER idEvent
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database identifier of the Event with which this
origin is associated.

MEMBER BindToEvent
MEMBER_TYPE int
MEMBER_DESCRIPTION Boolean flag indicating whether or not this Origin
should be bound to an Event when it is inserted into the DB.  If yes,
it is associated with the Event identified by idEvent upon insertion
into the DB.

MEMBER SetPreferred
MEMBER_TYPE int
MEMBER_DESCRIPTION Boolean flag indicating whether or not this Origin
should be set as the Preferred Origin for the Event identified by
idEvent.  This flag is ignored if BindToEvent is FALSE.

MEMBER tOrigin
MEMBER_TYPE double
MEMBER_DESCRIPTION Origin time in seconds since 1970.

MEMBER dLat
MEMBER_TYPE float
MEMBER_DESCRIPTION Lattitude of the Origin.  Expressed in degrees.
(North=positive)

MEMBER dLon
MEMBER_TYPE float
MEMBER_DESCRIPTION Longitude of the Origin.  Expressed in degrees.
(East=positive)

MEMBER dDepth
MEMBER_TYPE float
MEMBER_DESCRIPTION Depth of the Origin.
Expressed in kilometers below surface.

MEMBER dLatStart
MEMBER_TYPE float
MEMBER_DESCRIPTION Lattitude of the starting location.  

MEMBER dLonStart
MEMBER_TYPE float
MEMBER_DESCRIPTION Longitude of the starting location. 

MEMBER dDepthStart
MEMBER_TYPE float
MEMBER_DESCRIPTION Depth of the starting location .

MEMBER dErrLat
MEMBER_TYPE float
MEMBER_DESCRIPTION Potential lattitude error of the Origin.  Expressed
in degrees.

MEMBER dErrLon
MEMBER_TYPE float
MEMBER_DESCRIPTION Potential longitude error of the Origin.  Expressed
in degrees.

MEMBER dErZ
MEMBER_TYPE float
MEMBER_DESCRIPTION Potential vertical(depth) error of the Origin.
Expressed in km.

MEMBER ExternalTableName
MEMBER_TYPE char[256]
MEMBER_DESCRIPTION Reserved

MEMBER xidExternal
MEMBER_TYPE char[256]
MEMBER_DESCRIPTION Reserved

MEMBER sRealSource
MEMBER_TYPE char[51]
MEMBER_DESCRIPTION Identifier of the source/author that declared the
Origin.  This could be the Logo of the Earthworm module that generated
the Origin, or the login name of an analyst that reviewed it.

MEMBER sSource
MEMBER_TYPE char[101]
MEMBER_DESCRIPTION Human readable name of the source/author of the origin.

MEMBER Comment
MEMBER_TYPE char[4000]
MEMBER_DESCRIPTION Comment about the Origin. Currently unused.

MEMBER iGap
MEMBER_TYPE int
MEMBER_DESCRIPTION Largest gap between stations used to compute the
origin.  The gap is the largest arc between stations, based on a
epicentral circle.  Expressed in degrees.

MEMBER dMin
MEMBER_TYPE float
MEMBER_DESCRIPTION Epicentral distance to nearest station used in
origin computation.

MEMBER dRms
MEMBER_TYPE float
MEMBER_DESCRIPTION RMS misfit for the origin.  What does RMS mean?
CLEANUP dk 05/01/00.

MEMBER iAssocRD
MEMBER_TYPE int
MEMBER_DESCRIPTION Number of readings associated with the Origin.

MEMBER iAssocPh
MEMBER_TYPE int
MEMBER_DESCRIPTION Number of phases associated with the Origin.

MEMBER iUsedRd
MEMBER_TYPE int
MEMBER_DESCRIPTION Number of readings used to calculate the Origin.

MEMBER iUsedPh
MEMBER_TYPE int
MEMBER_DESCRIPTION Number of phases used to calculate the Origin.

MEMBER iE0Azm
MEMBER_TYPE int
MEMBER_DESCRIPTION Azimuth of the smallest principal error of the error
elipse.  Expressed as Degrees east of North.

MEMBER iE1Azm
MEMBER_TYPE int
MEMBER_DESCRIPTION Azimuth of the intermediate principal error of the
error elipse.  Expressed as Degrees east of North.

MEMBER iE2Azm
MEMBER_TYPE int
MEMBER_DESCRIPTION Azimuth of the largest principal error of the error
elipse.  Expressed as Degrees east of North.

MEMBER iE0Dip
MEMBER_TYPE int
MEMBER_DESCRIPTION Dip of the smallest principal error of the error
elipse.  Expressed as Degrees below horizontal.

MEMBER iE1Dip
MEMBER_TYPE int
MEMBER_DESCRIPTION Dip of the intermediate principal error of the error
elipse.  Expressed as Degrees below horizontal.

MEMBER iE2Dip
MEMBER_TYPE int
MEMBER_DESCRIPTION Dip of the largest principal error of the error
elipse.  Expressed as Degrees below horizontal.

MEMBER dE0
MEMBER_TYPE float
MEMBER_DESCRIPTION Magnitude of smallest principle error of the error
elipse.

MEMBER dE1
MEMBER_TYPE float
MEMBER_DESCRIPTION Magnitude of intermediate principle error of the
error elipse.

MEMBER dE2
MEMBER_TYPE float
MEMBER_DESCRIPTION Magnitude of largest principle error of the error
elipse.

MEMBER dMCI
MEMBER_TYPE double
MEMBER_DESCRIPTION Confidence interval.  CLEANUP???  Help? Units,
detailed meaning?

MEMBER iFixedDepth
MEMBER_TYPE int
MEMBER_DESCRIPTION Flag indicating whether a fixed depth model was used
to calculate the Origin.

MEMBER iVersionNum
MEMBER_TYPE int
MEMBER_DESCRIPTION Integer > 0 indicating the version number of the current
origin.  The idea, is that external system will produce multiple revisions
of event solutions as time goes on.  Each new origin will have an increasing
version number assigned to it.  That way when you process origin data to
produce other summary data (such as magnitudes) you can associate it with
the correct origin version.

MEMBER szSourceEventID
MEMBER_TYPE char[256]
MEMBER_DESCRIPTION Event identifer assigned to this origin's event by the
Source.  This would be the EventID given by and Earthworm Binder Module
to an automatic event that was generated by an Earthworm.

*************************************************
************************************************/
typedef struct _EWDB_OriginStruct
{

	EWDBid    idOrigin;		/* database ID of the associated event */
	EWDBid    idEvent;		/* database ID of the associated event */
	int		  BindToEvent;	/* should origin be bound to this event */
	int		  SetPreferred;	/* is this the preferred origin */
	double	  tOrigin;		/* origin time */
	float	  dLat;
	float	  dLon;
	float	  dDepth;
	float	  dErrLat;
	float	  dErrLon;
	float	  dErZ;
	char	  ExternalTableName[256];
	char	  xidExternal[256];
	char	  sSource[101];		/* Human readable Source string */
	char	  sRealSource[51]; /* sSource in the Source table */
	char	  Comment[4000];
	int		  iGap;
	float	  dDmin;
	float	  dRms;
	int		  iAssocRd;
	int		  iAssocPh;
	int		  iUsedRd;
	int		  iUsedPh;
	int		  iE0Azm;
	int		  iE1Azm;
	int		  iE2Azm;
	int		  iE0Dip;
	int		  iE1Dip;
	int		  iE2Dip;
	float	  dE0;
	float	  dE1;
	float	  dE2;
	double	  dMCI;
	int		  iFixedDepth;
	int     iVersionNum;
	char    szSourceEventID[256];
} EWDB_OriginStruct;



/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_EventListStruct
TYPE_DEFINITION struct _EWDB_EventListStruct DESCRIPTION Structure to
provide summary information for one of a list of Events.

MEMBER Event
MEMBER_TYPE EWDB_EventStruct
MEMBER_DESCRIPTION Event summary information for the current event.

MEMBER dOT
MEMBER_TYPE double
MEMBER_DESCRIPTION Time of the Event's preferred origin of in seconds
since 1970.

MEMBER dLat
MEMBER_TYPE float
MEMBER_DESCRIPTION Latitude of the preferred Origin 
of the Event. Expressed in degrees.  (North=positive)

MEMBER dLon
MEMBER_TYPE float
MEMBER_DESCRIPTION Longitude of the preferred Origin 
of the Event. Expressed in degrees.  (East=positive)

MEMBER dDepth
MEMBER_TYPE float
MEMBER_DESCRIPTION Depth of the preferred Origin 
of the Event. Expressed in kilometers below surface.

MEMBER dPrefMag
MEMBER_TYPE float
MEMBER_DESCRIPTION preferred Magnitude of the Event.

NOTE There was overlap between EWDB_EventListStruct and
EWDB_EventStruct, so the overlapping fields from
EWDB_EventListStruct were removed and EWDB_EventStruct 
was incorporated.

*************************************************
************************************************/
typedef struct _EWDB_EventListStruct
{
    EWDB_EventStruct Event;
    EWDBid  idOrigin;             
    double  dOT;             
    float   dLat;           
    float   dLon;          
    float   dDepth;       
    float   dPrefMag;  
} EWDB_EventListStruct;


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_CriteriaStruct
TYPE_DEFINITION struct _EWDB_CriteriaStruct
DESCRIPTION Structure for specifying criteria for object searches.
Criteria can be used to retrieve a list of certian objects, like Events
or Stations.

MEMBER Criteria
MEMBER_TYPE int
MEMBER_DESCRIPTION Set of flags specifying which criteria to use.  See
EWDB_CRITERIA_USE_TIME for a list of the possible criteria flags.  The
flags are not mutually exclusive.  The values of the remaning members
will be ignored if their matching criteria flags are not set.

MEMBER MinTime
MEMBER_TYPE int
MEMBER_DESCRIPTION The minimum acceptable time for a list value.
Specified as seconds since 1970.  If time were a criteria for selecting
a list of events, then all events in the returned list would have to
have occurred after MinTime.

MEMBER MaxTime
MEMBER_TYPE int
MEMBER_DESCRIPTION The maximum acceptable time for a list value.
Comparable to MinTime.  Expressed as seconds since 1970.

MEMBER MinLon
MEMBER_TYPE float
MEMBER_DESCRIPTION The minimum acceptable longitude for a list value.
Same principal as MinTime.  Expressed as degrees. (West=negative)

MEMBER MaxLon
MEMBER_TYPE float
MEMBER_DESCRIPTION The maximum acceptable longitude for a list value.
Same principal as MinTime.  Expressed as degrees. (West=negative)

MEMBER MinLat
MEMBER_TYPE float
MEMBER_DESCRIPTION The minimum acceptable lattitude for a list value.
Same principal as MinTime.  Expressed as degrees. (South=negative)

MEMBER MaxLat
MEMBER_TYPE float
MEMBER_DESCRIPTION The maximum acceptable lattitude for a list value.
Same principal as MinTime.  Expressed as degrees. (South=negative)

MEMBER MinDepth
MEMBER_TYPE float
MEMBER_DESCRIPTION The minimum acceptable depth for a list value.  Same
principal as MinTime.  Expressed as kilometers(km) below surface.

MEMBER MaxDepth
MEMBER_TYPE float
MEMBER_DESCRIPTION The maximum acceptable depth for a list value.  Same
principal as MinTime.  Expressed as kilometers(km) below surface.

*************************************************
************************************************/
typedef struct _EWDB_CriteriaStruct
{
  int   Criteria;
  int   MinTime;
  int   MaxTime;
  float MinLon;
  float MaxLon;
  float MinLat;
  float MaxLat;
  float MinDepth;
  float MaxDepth;
} EWDB_CriteriaStruct;


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_AuthorStruct
TYPE_DEFINITION struct _EWDB_AuthorStruct
DESCRIPTION Structure for specifying information about an author/source.

MEMBER idSource
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION EW DB id of the Source.

MEMBER szSource
MEMBER_TYPE char[256]
MEMBER_DESCRIPTION Identifier of the source/author that declared the
Event.  Only one Source is listed here.  If more than one Source
contributed to the declaration of the Event, then the manner by which
this Source is chosen as THE Source is undefined.  This could be the
Logo of the Earthworm module that generated the Event, or the or the
login name of an analyst that reviewed it.

MEMBER szSourceName
MEMBER_TYPE char[256]
MEMBER_DESCRIPTION Name of the source/author that declared the Event.
The SourceName is a human readable version of the Source.  The
SourceName could be the name of the network that generated the event,
or the name of an Analyst that reviewed it.

MEMBER idInst
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION EW DB id of the Installation with which this source
is associated.

MEMBER iEWInstID
MEMBER_TYPE int
MEMBER_DESCRIPTION The EW INST_ID of the Installation with which this 
source is associated.

MEMBER iSourceType
MEMBER_TYPE int
MEMBER_DESCRIPTION The type of source.  See constants for definition
types.  This is a flag that describes the type of source, including
whether or not it is human or automatic. 

MEMBER szNote
MEMBER_TYPE char[256]
MEMBER_DESCRIPTION Freeform auxiliary information about this source.

NOTE Only the idSource, szSource, and szSourceName fields are currently
well defined and supported.  iSourceType of -1 indicates and unknown
source type.  szSource corresponds to the official source name. (this 
would likely be an EW Logo for an earthworm module, or some similar
notation for other automatic processors.  For human authors, this
would be some sort of login ID.  The szSourceName would be the 
human readable version of the source.  Such as the name of a module,
or the full name of a human author.  DK 092704

*************************************************
************************************************/
typedef struct _EWDB_AuthorStruct
{
  EWDBid idSource;
  char   szSource[256];
  char   szSourceName[256];
  EWDBid idInst;
  int    iEWInstID;
  int    iSourceType;
  char   szNote[1024];
} EWDB_AuthorStruct;



/* ##########################
 * ##########################
 * ## C STRUCTURES for Mw
 * ##########################
 * ########################## */

typedef enum _MwTimeSeriesDataType
{
  MWTS_UNKNOWN = 0,
  MWTS_PROCESSED = 1,
  MWTS_SYNTHETIC = 2
} MwTimeSeriesDataType;

typedef struct _EWDB_MwFilterStruct
{
  EWDBid idMwFilter;
  float dLowCutHz;
  float dLowTaperHz;
  float dHighTaperHz;
  float dHighCutHz;
} EWDB_MwFilterStruct;

typedef struct _EWDB_MwStruct
{
  EWDBid idMw;
  double dMxx,dMyy,dMzz, dMxy, dMxz, dMyz;
  int    iScalarExp;
  double dM0;
  double dPFPStrike, dPFPDip, dPFPRake;  /* Primary Fault Plane */
  double dAFPStrike, dAFPDip, dAFPRake;  /* Alternate Fault Plane */
  double dDepth;
  EWDBid idOrigin;
  EWDBid idMag;  /* should only be set for the final Mw */
  int    iNumStations;
  double dMisfit;
  double dPercentCLVD;
  double dPercentDC;
  EWDB_MwFilterStruct Filter;
} EWDB_MwStruct;


typedef struct _EWDB_MwTimeSeriesStruct
{
  EWDBid idMwCTS;
  double dStart; /* - standard epoch time */
  float  dFreq;  /* - normally 1 hz */
  int    iLen;   /* - number of samples in the time series */
  EWDBid idChan;
  MwTimeSeriesDataType   iTSType; /* - Flag indicating whether the data is SYNTHETIC/REAL */
  EWDB_MwFilterStruct *pFilter;
  EWDBid idMwFilter;
  double TS[EWDB_MAX_SAMPLES_PER_MW_TIMESERIES];
} EWDB_MwTimeSeriesStruct;


typedef struct _EWDB_MwChanStruct
{
  EWDBid idMwChan;
  EWDBid idMw;
  EWDBid idPick;
  EWDBid idChan;
  int iOffset;        /* (offset in samples the synthetic is slid ahead of the real - can be negative) */
  double dMisfit;     /* (size, units, precision?)    */
  EWDB_MwTimeSeriesStruct SynthTS;
                      /* (pointer to the synthetic time series) */
  EWDB_MwTimeSeriesStruct RealTS; 
                      /* (pointer to the synthetic time series) */
} EWDB_MwChanStruct;

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

SUB_LIBRARY PARAMETRIC_API

LOCATION THIS_FILE

DESCRIPTION This is a portion of the EWDB_API_LIB
that deals with weak-motion parametric event data.
It provides access to parametric data, such as
event data, origin's, magnitudes, and mechanisms
as well as their supporting parametric data.

*************************************************
************************************************/

/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY MATURE

FUNCTION ewdb_api_CreateArrival

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

PARAMETER 1
PARAM_NAME pArrival
PARAM_TYPE EWDB_ArrivalStruct *
PARAM_DESCRIPTION Pointer to a EWDB_ArrivalStruct filled by the caller.

DESCRIPTION Function creates an arrival from the caller supplied
information in pArrival.  The function assumes that the caller has
supplied all information for an arrival including origin-phase
information.  The pArrival->idOrigin must be a valid existing origin DB
ID.  Upon successful completion, the function writes the DB ID of the
newly created phase pick(Pick) and the newly created OriginPick to
pArrival->idPick and pArrival->idOriginPick respectively.

*************************************************
************************************************/
int ewdb_api_CreateArrival(EWDB_ArrivalStruct *pArrival);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY NEW

FUNCTION ewdb_api_CreatePick

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

PARAMETER 1
PARAM_NAME pArrival
PARAM_TYPE EWDB_ArrivalStruct *
PARAM_DESCRIPTION Pointer to a EWDB_ArrivalStruct filled by the caller.

DESCRIPTION Function creates an unassociated pick from the caller supplied
information in pArrival.  This function creates a Pick in the DB
but it does not associate the Pick with an Origin.  To associate Picks with 
an Origin, either call ewdb_api_CreateArrival() to create the entire arrival
at once, or call ewdb_api_CreateOriginPick() after creating a naked pick..  

The function assumes that the 
caller has supplied phase type(szObsPhase), phase arrival time (tObsPhase),
identification of the channel for which the phase was detected(idChan), 
phase first motion(cMotion), phase onset(cOnset), and pick error(dSigma).
All other members of the EWDB_ArrivalStruct are ignored by the function.

Upon successful completion, the function writes the DB ID of the
newly created phase pick(Pick) to pArrival->idPick.

*************************************************
************************************************/
int ewdb_api_CreatePick(EWDB_ArrivalStruct *pArrival);  


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY NEW

FUNCTION ewdb_api_CreateOriginPick

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

PARAMETER 1
PARAM_NAME pArrival
PARAM_TYPE EWDB_ArrivalStruct *
PARAM_DESCRIPTION Pointer to a EWDB_ArrivalStruct filled by the caller.

DESCRIPTION Function creates an association between a pick and an origin.  
The information must be supplied by the caller in pArrival. 

Upon successful completion, the function writes the DB ID of the
newly created OriginPick to pArrival->idOriginPick

*************************************************
************************************************/
int ewdb_api_CreateOriginPick (EWDB_ArrivalStruct *pArrival, EWDBid idOrigin);  


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY MATURE


FUNCTION ewdb_api_CreateEvent

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

PARAMETER 1
PARAM_NAME pEvent
PARAM_TYPE EWDB_EventStruct *
PARAM_DESCRIPTION Pointer to an EWDB_EventStruct filled by the caller.

DESCRIPTION Function creates an event in the DB using the information
provided by the caller in pEvent.  Upon successful completion, the
function writes the DB ID of the newly created event to pEvent->idEvent.
<br>
If the caller specifies a Source and a SourceEventID, and there is 
an existing Event with that same Source/SourceEventID, then the 
idEvent of the existing event will be returned, instead of 
creating a new event.  The function will return success in this case
even though it has not actually created a new Event.  If a matching
Event already exists, then the Comment passed by the caller is ignored.

*************************************************
************************************************/
int ewdb_api_CreateEvent(EWDB_EventStruct *pEvent);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY MODIFIED

FUNCTION ewdb_api_CreateMagnitude

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

PARAMETER 1
PARAM_NAME pMag
PARAM_TYPE EWDB_MagStruct *
PARAM_DESCRIPTION Pointer to an EWDB_MagStruct filled by the caller.

DESCRIPTION Function creates a magnitude in the DB using the
information provided by the caller in pMag.  If pMag->bBindToEvent is
set to TRUE, then the function will attempt to associate the new
magnitude with an existing event(pMag->idEvent).  If both
pMag->bBindToEvent and pMag->bSetPreferred are TRUE, then the function
will attempt to set the magnitude as the preferred one for the Event.
Upon successful completion, the function writes the DB ID of the newly
created magnitude to pMag->idMag.  <br>
The call will fail under any of the following circumstances:
If bBindToEvent is TRUE, and idEvent is not a valid existing idEvent 
in the DB <br>
If idOrigin is set to a non-zero value, and it does not match 
an existing idOrigin in the DB<br>
szSource is not a valid source/author string.

*************************************************
************************************************/
int ewdb_api_CreateMagnitude(EWDB_MagStruct *pMag);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY MATURE

FUNCTION ewdb_api_CreateOrigin

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

PARAMETER 1
PARAM_NAME pOrigin
PARAM_TYPE EWDB_OriginStruct *
PARAM_DESCRIPTION Pointer to an EWDB_OriginStruct filled by the caller.

DESCRIPTION Function creates an origin in the DB using the information
provided by the caller in pOrigin.  If pOrigin->BindToEvent is set to
TRUE, then the function will attempt to associate the new origin with
an existing event(pOrigin->idEvent).  If both pOrigin->BindToEvent and
pOrigin->SetPreferred are TRUE, then the function will attempt to set
the origin as the preferred one for the Event.  Upon successful
completion, the function writes the DB ID of the newly created origin
to pOrigin->idOrigin.

*************************************************
************************************************/
int ewdb_api_CreateOrigin(EWDB_OriginStruct *pOrigin);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY MODIFIED

FUNCTION ewdb_api_DeleteEvent

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

PARAMETER 1
PARAM_NAME IN_idEvent
PARAM_TYPE int
PARAM_DESCRIPTION DB id of the event to delete.

PARAMETER 2
PARAM_NAME IN_bDeleteTraceOnly
PARAM_TYPE int
PARAM_DESCRIPTION If set to TRUE, delete only trace, otherwise delete both
trace and parametric data for this event.

DESCRIPTION Deletes trace and/or parametric data for an event from the
database. Note that this means that the deleted data is permanently
gone.

*************************************************
************************************************/
int ewdb_api_DeleteEvent(int IN_idEvent, int IN_bDeleteTraceOnly);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMmENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY MATURE

FUNCTION ewdb_api_GetArrivals

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Partial success.  A list of arrivals was retrieved,
but the caller's buffer was not large enough to accomadate all of the
arrivals found.  See pNumArrivalsFound for the number of arrivals found
and pNumArrivalsRetrieved for the number of arrivals placed in the
caller's buffer.

PARAMETER 1
PARAM_NAME idOrigin
PARAM_TYPE EWDBid
PARAM_DESCRIPTION This function retrieves a list of arrivals associated
with an origin.  idOrigin is the database ID of the origin for which
the caller wants a list of arrivals.

PARAMETER 2
PARAM_NAME pArrivals
PARAM_TYPE EWDB_ArrivalStruct *
PARAM_DESCRIPTION Buffer allocated by the caller, where the function
will write the list of arrivals.

PARAMETER 3
PARAM_NAME pNumArrivalsFound
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of found arrivals.

PARAMETER 4
PARAM_NAME pNumArrivalsRetrieved
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of arrivals placed in the callers buffer(pArrivals).

PARAMETER 5
PARAM_NAME BufferLen
PARAM_TYPE int
PARAM_DESCRIPTION Size of the pArrivals buffer as a multiple of
EWDB_ArrivalStruct. (example: 15 structs)

DESCRIPTION The function retrieves a list of arrivals that are
associated with a given origin.  See EWDB_ArrivalStruct for a
description of the information retrieved for each associated arrival.

NOTE  This function does not fill in the pStation member of the
Arrival structure.  To retrieve channel information in addition to 
arrival information, use the ewdb_api_GetArrivalsWChanInfo function.

*************************************************
************************************************/
int ewdb_api_GetArrivals(EWDBid idOrigin, EWDB_ArrivalStruct * pArrivals,
                         int * pNumArrivalsFound, int * pNumArrivalsRetrieved,
                         int BufferLen);

/**********************************************
 * ewdb_api_GetPickByID
 * gets a single pick from the Pick table based on the passed in
 * pick ID and puts the results into an ArrivalStruct
 ***********************************************/
int ewdb_api_GetPickByID(EWDBid idPick, EWDB_ArrivalStruct *pArrival);

/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY MATURE

FUNCTION ewdb_api_GetEventInfo

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

PARAMETER 1
PARAM_NAME idEvent
PARAM_TYPE EWDBid
PARAM_DESCRIPTION The DB identifier of the Event for which the caller
wants information.

PARAMETER 2
PARAM_NAME pEvent
PARAM_TYPE EWDB_EventStruct *
PARAM_DESCRIPTION Pointer to a EWDB_EventStruct allocated by the
caller, where the function will write the information about the Event.

DESCRIPTION Function retrieves information about the Event identified
by idEvent.  See EWDB_EventStruct for a description of the information
provided.

NOTE  The szSource and szSourceEventID fields of the EWDB_EventStruct
are not filled in by this call.  Instead use ewdb_api_GetEventSummaryInfo().


*************************************************
************************************************/
int ewdb_api_GetEventInfo(EWDBid idEvent, EWDB_EventStruct * pEvent);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY MATURE

FUNCTION ewdb_api_GetEventList

DESCRIPTION Function retrieves summary information for a list of
events, based on selection criteria provided by the caller.  See
EWDB_EventListStruct for a description of the information returned for
each event in the list.

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION A list of events was successfully retrieved.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION The function experienced a fatal error while
retrieving a list of events.

RETURN_VALUE -x < -1
RETURN_DESCRIPTION (-x) is returned where x > 1 when the size of the
caller's buffer is insufficient to hold all of the events that matched
the caller's criteria.  x is the size of the buffer required to
retrieve all of the events that were found.  The function places as
many events as possible into the caller's existing buffer.  This is not
a failure condition, just a warning that the caller did not receive all
of the requested data.

PARAMETER 1
PARAM_NAME pBuffer
PARAM_TYPE EWDB_EventListStruct *
PARAM_DESCRIPTION Buffer allocated by the caller, in which the function
will place the list of events matching the caller's criteria.

PARAMETER 2
PARAM_NAME len
PARAM_TYPE int
PARAM_DESCRIPTION Length of pBuffer as a multiple of
EWDB_EventListStruct.

PARAMETER 3
PARAM_NAME StartTime
PARAM_TYPE int
PARAM_DESCRIPTION Minimum(oldest) time acceptable for an event.
Expressed as seconds since 1970(time_t).

PARAMETER 4
PARAM_NAME EndTime
PARAM_TYPE int
PARAM_DESCRIPTION Maximum(youngest) time acceptable for an event.
Expressed as seconds since 1970(time_t).

PARAMETER 5
PARAM_NAME pCriteria
PARAM_TYPE EWDB_EventListStruct *
PARAM_DESCRIPTION Pointer to the EWDB_EventListStruct
that contains part of the selection criteria for the
list, including: minimum lattitude, minimum longitude,
minimum depth, minimum magnitude, and source
limitations.  Source criteria is:             <br>
         "*"  events from all sources             <br>
         "**" events from only human sources      <br>
         ""   (blank string)all automatic sources <br>
Any other string will be matched against the source identifier, such as
the logo of the earthworm module that created a solution, or the
dblogin of a human reviewer.

PARAMETER 6
PARAM_NAME pMaxEvent
PARAM_TYPE EWDB_EventListStruct *
PARAM_DESCRIPTION Pointer to the EWDB_EventListStruct that contains
part of the selection criteria for the list, including: maximum
lattitude, maximum longitude, maximum depth, and maximum magnitude.

PARAMETER 7
PARAM_NAME pNumEvents
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer where the function writes the
number of events it put in pBuffer.

DESCRIPTION Returns the list of all events in the database, in a given
time range, that meet the criteria specified in pCriteria.  For each Event
found to meet the given criteria, an EWDB_EventListStruct is returned.
This is the primary function used for searching for a subset of the events
in the DB.

*************************************************
************************************************/
int ewdb_api_GetEventList(EWDB_EventListStruct * pBuffer, int len,
                          unsigned int StartTime, unsigned int EndTime,
                          EWDB_EventListStruct * pCriteria, 
                          EWDB_EventListStruct * pMaxEvent, int * pNumEvents);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY MODIFIED

FUNCTION ewdb_api_GetMagnitude

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

PARAMETER 1
PARAM_NAME idMag
PARAM_TYPE EWDBid
PARAM_DESCRIPTION The DB identifier of the Magnitude for which the
caller wants information.

PARAMETER 2
PARAM_NAME pMag
PARAM_TYPE EWDB_MagStruct *
PARAM_DESCRIPTION Pointer to an EWDB_MagStruct allocated by the caller,
where the function will write the information about the Magnitude.

DESCRIPTION Function retrieves information about the Magnitude
identified by idMag.  See EWDB_MagStruct for a description of the
information provided.

*************************************************
************************************************/
int ewdb_api_GetMagnitude(EWDBid idMag, EWDB_MagStruct * pMag);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY MATURE

FUNCTION ewdb_api_GetOrigin

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

PARAMETER 1
PARAM_NAME idOrigin
PARAM_TYPE EWDBid
PARAM_DESCRIPTION The DB identifier of the Origin for which the caller
wants information.

PARAMETER 2
PARAM_NAME pOrigin
PARAM_TYPE EWDB_OriginStruct *
PARAM_DESCRIPTION Pointer to a EWDB_OriginStruct allocated by the
caller, where the function will write the information about the Origin.

DESCRIPTION Function retrieves information about the Origin identified
by idOrigin.  See EWDB_OriginStruct for a description of the
information provided.

*************************************************
************************************************/
int ewdb_api_GetOrigin(EWDBid idOrigin, EWDB_OriginStruct * pOrigin);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY MATURE

FUNCTION ewdb_api_GetPreferredSummaryInfo

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

PARAMETER 1
PARAM_NAME idEvent
PARAM_TYPE EWDBid
PARAM_DESCRIPTION The DB identifier for the Event for which the caller
wants summary info.

PARAMETER 2
PARAM_NAME pidOrigin
PARAM_TYPE EWDBid *
PARAM_DESCRIPTION Pointer to a EWDBid where the function will write the
DB ID of the preferred origin for the Event.  0 will be written as the
ID if there is no preferred origin for the given Event.

PARAMETER 3
PARAM_NAME pidMagnitude
PARAM_TYPE EWDBid *
PARAM_DESCRIPTION Pointer to a EWDBid where the function will write the
DB ID of the preferred magnitude for the Event.  0 will be written as
the ID if there is no preferred magnitude for the given event.

PARAMETER 4
PARAM_NAME pidMech
PARAM_TYPE EWDBid *
PARAM_DESCRIPTION Pointer to a EWDBid where the function will write the
DB ID of the preferred mechanism for the Event.  0 will be written as
the ID if there is no preferred mechanism for the given event.  (Note:
Mechanism are currently only supported in a trivial manner)

DESCRIPTION Function retrieves the DB IDs for the preferred summary
information(origin, magnitude, and mechanism) for the Event identified
by idEvent.

*************************************************
************************************************/
int ewdb_api_GetPreferredSummaryInfo(EWDBid idEvent, EWDBid *pidOrigin,
                                     EWDBid *pidMagnitude, EWDBid *pidMech);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY NEW

FUNCTION ewdb_api_GetStaMags

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Partial success.  A list of station magnitudes was
retrieved, but the caller's buffer was not large enough to accomadate
all of the station magnitudes found.  See pNumStaMagsFound for the
number of station magnitudes found and pNumStaMagsRetrieved for the
number of station magnitudes placed in the caller's buffer.

PARAMETER 1
PARAM_NAME idMagnitude
PARAM_TYPE EWDBid
PARAM_DESCRIPTION This function retrieves a list of station magnitudes
associated with an summary magnitude.  idMagnitude is the database ID
of the magnitude for which the caller wants a list of station
magnitudes.

PARAMETER 2
PARAM_NAME pStaMags
PARAM_TYPE EWDB_StationMagStruct *
PARAM_DESCRIPTION Buffer allocated by the caller, where the function
will write the list of station magnitudes.

PARAMETER 3
PARAM_NAME pNumStaMagsFound
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of station magnitudes found.

PARAMETER 4
PARAM_NAME pNumStaMagsRetrieved
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of station magnitudes placed in the caller's
buffer(pStaMags).

PARAMETER 5
PARAM_NAME BufferLen
PARAM_TYPE int
PARAM_DESCRIPTION Size of the pStaMags buffer as a multiple of
EWDB_StationMagStruct. (example: 15 structs)

DESCRIPTION The function retrieves a list of station magnitudes that
are associated with a given summary magnitude.  See
EWDB_StationMagStruct for a description of the information
retrieved for each associated station magnitude.

*************************************************
************************************************/
int ewdb_api_GetStaMags(EWDBid idMagnitude, 
                        EWDB_StationMagStruct * pStaMags,
                        int * pNumStaMagsFound, int * pNumStaMagsRetrieved,
                        int BufferLen);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY NEW

FUNCTION ewdb_api_UpdateComment

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

PARAMETER 1
PARAM_NAME szFieldTypeName
PARAM_TYPE char *
PARAM_DESCRIPTION Name of the table to updated

PARAMETER 2
PARAM_NAME idCore
PARAM_TYPE EWDBid
PARAM_DESCRIPTION id of the record to update

PARAMETER 3
PARAM_NAME szUpdatedComment
PARAM_TYPE char *
PARAM_DESCRIPTION Text of the updated comment. 

DESCRIPTION  Updates a comment for any record in the DB.  Enter the
tablename and the record ID for the table that the comment belongs to,
and the function will update the comment for that record.  This
requires that the caller know DB layout, which is a bad thing.  What we
need is a lookup table for the comment types, and we can perform a
conversion from the lookup table to the actual DB table type within
the DB.  

*************************************************
************************************************/
int ewdb_api_UpdateComment (char * szFieldTypeName, EWDBid idCore,
                            char * szUpdatedComment);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY NEW

FUNCTION ewdb_api_CreateOrUpdateComment

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

PARAMETER 1
PARAM_NAME idComment
PARAM_TYPE EWDBid *
PARAM_DESCRIPTION Ignored on entry.
				On exit, contains comment ID of created comment

PARAMETER 2
PARAM_NAME szComment
PARAM_TYPE char *
PARAM_DESCRIPTION Comment to add.


DESCRIPTION  Creates a comment in the DB.  If the comment is too long for one DB entry,
	the comment will span multiple DB entries linked together.  idComment will return
	the ID for the first segment of this comment.

*************************************************
************************************************/
int ewdb_api_CreateComment(EWDBid *idComment, char *szComment);

int ewdb_api_DeleteComment(EWDBid idComment);

/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY NEW

FUNCTION ewdb_api_GetComment

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION No comment found with the given DB ID

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

PARAMETER 1
PARAM_NAME idComment
PARAM_TYPE EWDBid
PARAM_DESCRIPTION DB ID of the comment to retrieve

PARAMETER 2
PARAM_NAME szComment
PARAM_TYPE char *
PARAM_DESCRIPTION On exit, contains the comment fresh from the DB,
					with that new-comment smell.

PARAMETER 1
PARAM_NAME idNextComment
PARAM_TYPE EWDBid *
PARAM_DESCRIPTION On exit, contains the DB ID of the next comment snippet, or 0 if this
	is the end of the comment.

DESCRIPTION  Retrieves a comment from DB with the given ID.  This will only retrieve one
	comment "snippet"; it is up to the caller to repeatedly call this function to retrieve
	an entire comment that's spread out over multiple DB entries.  Alternatively, the function
	ewdb_apps_GetFullComment() will handle this.

*************************************************
************************************************/
int ewdb_api_GetComment(EWDBid idComment, char *szComment, EWDBid *idNextComment);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY NEW

FUNCTION ewdb_api_GetOriginsForEvent

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION  Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Origin records found successfully, but one
or more records were unable to be retrieved.  Most likely 
indicates that the caller's buffer is not large enough to hold
all of the records found.

PARAMETER 1
PARAM_NAME IN_idEvent
PARAM_TYPE EWDBid
PARAM_DESCRIPTION Event for which the caller wishes to 
retrieve origins.

PARAMETER 2
PARAM_NAME pOrigins
PARAM_TYPE EWDB_OriginStruct *
PARAM_DESCRIPTION Pointer to a buffer (allocated by the caller)
where the function will put information for the Origins found 
for the given Event.  

PARAMETER 3
PARAM_NAME pNumOriginsFound
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer (caller allocated) where
the function will write the number of origins found for the given
idEvent.  Note:  this is not the number actually retrieved.

PARAMETER 4
PARAM_NAME pNumOriginsRetrieved
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer (caller allocated) where
the function will write the number of origins retrieved into the
caller's buffer for the given idEvent

PARAMETER 5
PARAM_NAME IN_BufferRecLen
PARAM_TYPE int 
PARAM_DESCRIPTION The size of the pOrigins buffer allocated
by the caller.  (In terms of number of EWDB_OriginStruct records)

DESCRIPTION  This function retrieves information for each Origin that
is associated with the event(idEvent) given by the caller.  

NOTE  Then entire EWDB_OriginStruct is not filled!  Only the following
fields are filled by this function:  idOrigin, tOrigin, dLat, dLon, 
dDepth, iAssocPh, iUsedPh, iFixedDepth, sSource, and sRealSource.

*************************************************
************************************************/
int ewdb_api_GetOriginsForEvent(EWDBid IN_idEvent, EWDB_OriginStruct * pOrigins,
                                int * pNumOriginsFound, 
                                int * pNumOriginsRetrieved,
                                int IN_BufferRecLen);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY  PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY NEW

FUNCTION ewdb_api_InsertCodaWithMag 

SOURCE_LOCATION src/oracle/schema-working/src/parametric/ewdb_api_InsertcodaWithMag.c

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

PARAMETER 1
PARAM_NAME pCodaDur
PARAM_TYPE EWDB_StationMagStruct *
PARAM_DESCRIPTION Pointer to a EWDB_StationMagStruct filled by the caller.

DESCRIPTION Function creates a coda duration based station magnitude
from the caller supplied information in pCodaDur.  The function assumes
that the caller has supplied all station magnitude information
including summary magnitude data in the EWDB_StationMagStruct.  The
caller need not fill in PeakAmp related information as it is
inappropriate.  The pCodaDur->idMagnitude and 
pCodaDur->StaMagUnion.CodaDur.idPick values
must be valid existing Magnitude and Pick DB IDs respectively.  The
pCodaDur->idChan value must also be filled in with the DB ID of a valid
existing channel.  Upon successful completion, the function writes the
DB IDs of the newly created coda duration, coda termination, and
station magnitude to pCodaDur->idCodaDur, pCodaDur->idTCoda, and
pCodaDur->idMagLink respectively.
This function performs a full insertion of a coda,
complete with how that coda (duration) contributed to an overall 
duration magnitude.  

NOTE You should use this function instead of using ewdb_internal_CreateTCoda(),
ewdb_internal_CreateCodaDur, and ewdb_internal_CreateStaMag(), unless you are 
VERY SURE that you know what you are doing, and know exactly how 
to use those functions.

*************************************************
************************************************/
int ewdb_api_InsertCodaWithMag(EWDB_StationMagStruct *pCodaDur);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY  PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY NEW

FUNCTION ewdb_api_InsertPeakAmpWithMag 

SOURCE_LOCATION src/oracle/schema-working/src/parametric/ewdb_api_InsertPeakAmpWithMag.c

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

PARAMETER 1
PARAM_NAME pPeakAmp
PARAM_TYPE EWDB_StationMagStruct *
PARAM_DESCRIPTION Pointer to a EWDB_PhaseAmpStruct filled by the caller.

DESCRIPTION Function creates a peak amplitude based station magnitude
from the caller supplied information in pPeakAmp.  The function assumes
that the caller has supplied all station magnitude information
including summary magnitude data in the EWDB_StationMagStruct.  The
caller need not fill in CodaDur related information as it is
inappropriate. 
This function performs a full insertion of an amplitude,
complete with how that amplitude contributed to an overall 
magnitude.  

NOTE You should use this function instead of using ewdb_internal_CreatePeakAmp(),
and ewdb_internal_CreateStaMag(), unless you are VERY SURE that you know what
you are doing, and know exactly how to use those functions.

*************************************************
************************************************/
int ewdb_api_InsertPeakAmpWithMag(EWDB_StationMagStruct *pPeakAmp);  


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetMagsForOrigin

STABILITY NEW

SOURCE_LOCATION THIS_FILE

RETURN_TYPE int

RETURN_VALUE Default_Return_Value
RETURN_DESCRIPTION Description of the default return value

PARAMETER 1
PARAM_NAME IN_idOrigin
PARAM_TYPE EWDBid
PARAM_DESCRIPTION Optional description of (IN_idOrigin)

PARAMETER 2
PARAM_NAME pMags
PARAM_TYPE EWDB_MagStruct *
PARAM_DESCRIPTION Optional description of (pMags)

PARAMETER 3
PARAM_NAME pNumMagsFound
PARAM_TYPE int *
PARAM_DESCRIPTION Optional description of (pNumMagsFound)

PARAMETER 4
PARAM_NAME pNumMagsRetrieved
PARAM_TYPE int *
PARAM_DESCRIPTION Optional description of (pNumMagsRetrieved)

PARAMETER 5
PARAM_NAME IN_BufferRecLen
PARAM_TYPE int
PARAM_DESCRIPTION Optional description of (IN_BufferRecLen)

DESCRIPTION Optionally, write a description of what the function does
and how it behaves.

NOTE Optionally, write a note about
the function here.

*************************************************
************************************************/
int ewdb_api_GetMagsForOrigin(EWDBid IN_idOrigin, EWDB_MagStruct * pMags,
                                int * pNumMagsFound, 
                                int * pNumMagsRetrieved,
                                int IN_BufferRecLen);




/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_DeleteDataBeforeTime

STABILITY NEW

SOURCE_LOCATION THIS_FILE

RETURN_TYPE int

RETURN_VALUE Default_Return_Value
RETURN_DESCRIPTION Description of the default return value

PARAMETER 1
PARAM_NAME IN_tTime
PARAM_TYPE int
PARAM_DESCRIPTION Optional description of (IN_tTime)

PARAMETER 2
PARAM_NAME IN_iDatatypes
PARAM_TYPE int
PARAM_DESCRIPTION Optional description of (IN_iDatatypes)

PARAMETER 3
PARAM_NAME IN_iMaxRecsToDelete
PARAM_TYPE int
PARAM_DESCRIPTION Maximum number of records to delete in this pass.
Use -1 to indicate delete-all.

PARAMETER 4
PARAM_NAME pSQLRetCode
PARAM_TYPE int *
PARAM_DESCRIPTION Return code from underlying SQL function indicating
more precise result.  See EWDB_DELETE_PARAMS_SUCCESS return codes
for more information.

DESCRIPTION Delete "datum" data from the database based on time.


*************************************************
************************************************/
int ewdb_api_DeleteDataBeforeTime(int IN_tTime, int IN_iDatatypes, 
                                  int IN_iMaxRecsToDelete, int * pSQLRetCode);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_UpdateEvent

STABILITY NEW

SOURCE_LOCATION THIS_FILE

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Error updating DB; see logfile for details.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Nothing updated in the DB - iUpdateInfoType was NONE

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.


PARAMETER 1
PARAM_NAME pEvent
PARAM_TYPE EWDB_EventStruct *
PARAM_DESCRIPTION Event structure to update in the DB.  Contains a valid event ID, as well
		as any other valid info that is to be updated.

PARAMETER 2
PARAM_NAME iUpdateInfoType
PARAM_TYPE int
PARAM_DESCRIPTION Constants OR-ed together, describing which fields in the DB to update
		(see EWDB_UPDATE_EVENT_NONE in ewdb_ora_api.h)

DESCRIPTION Updates one or more fields in the DB for a given event.  This is used to update
	an event's type, dubiocity, "archived" flag, and/or comments.


*************************************************
************************************************/
int ewdb_api_UpdateEvent(EWDB_EventStruct * pEvent, int iUpdateInfoType);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetAmpsByTime

STABILITY NEW

SOURCE_LOCATION THIS_FILE

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Partial success.  A list of amplitudes was retrieved,
but the caller's buffer was not large enough to accomadate all of the
amplitudes found.  See pNumAmpsFound for the number of arrivals found
and pNumAmpsRetrieved for the number of amplitudes placed in the
caller's buffer.

PARAMETER 1
PARAM_NAME tStart
PARAM_TYPE time_t
PARAM_DESCRIPTION Start time of criteria window.  (Secs since 1970)

PARAMETER 2
PARAM_NAME tEnd
PARAM_TYPE time_t
PARAM_DESCRIPTION End time of criteria window.  (Secs since 1970)

PARAMETER 3
PARAM_NAME iAmpType
PARAM_TYPE MAGNITUDE_TYPE
PARAM_DESCRIPTION Type of amplitude the caller is searching for.
"Amplitude Type" constants are defined in <earthworm_defs.h>.

PARAMETER 4
PARAM_NAME pAmpBuffer
PARAM_TYPE EWDB_PeakAmpStruct *
PARAM_DESCRIPTION Buffer allocated by the caller, where the function
will write the list of amplitudes retrieved.

PARAMETER 5
PARAM_NAME pStationBuffer
PARAM_TYPE EWDB_StationStruct *
PARAM_DESCRIPTION Buffer allocated by the caller, where the function
will write the list of station/channel info associated with
the amplitudes retrieved.  If pStationBuffer is set to NULL by the 
caller, then the function will not attempt to retrieve Station information
for the retrieved amplitudes.  If pStationBuffer is NOT NULL, then 
the function will fill out an EWDB_StationStruct for the channel 
associated with each amplitude.

PARAMETER 6
PARAM_NAME pNumAmpsFound
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of amplitudes found.

PARAMETER 7
PARAM_NAME pNumAmpsRetrieved
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of amplitudes retrieved and placed in the caller's buffer(pAmpBuffer).

PARAMETER 8
PARAM_NAME iBufferLen
PARAM_TYPE int
PARAM_DESCRIPTION Size of the pAmpBuffer buffer in as a multiple of
EWDB_PeakAmpStruct. (example: 15 structs)  If pStationBuffer is
not NULL, then it must also contain space for this many 
EWDB_StationStruct records.

DESCRIPTION Function retrieves a list of amplitudes of a given type
for a given time window.  Use tStart and tEnd to specify the time 
window for which you are interested in amplitudes.  If you want station
information for the channel associated with each amplitude, allocate 
space for pStationBuffer, otherwise set pStationBuffer to NULL, and
station information will not be retrieved.


*************************************************
************************************************/
int ewdb_api_GetAmpsByTime(time_t tStart, time_t tEnd, MAGNITUDE_TYPE iMagType,
      EWDB_PeakAmpStruct * pAmpBuffer, EWDB_StationStruct * pStationBuffer,
      int * pNumAmpsFound, int * pNumAmpsRetrieved, int iBufferLen);

int ewdb_api_GetAmpsByPick (EWDBid, MAGNITUDE_TYPE, EWDB_PeakAmpStruct *, int *, int *, int);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetAmpsByEvent

STABILITY NEW

SOURCE_LOCATION THIS_FILE

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Partial success.  A list of amplitudes was retrieved,
but the caller's buffer was not large enough to accomadate all of the
amplitudes found.  See pNumAmpsFound for the number of arrivals found
and pNumAmpsRetrieved for the number of amplitudes placed in the
caller's buffer.

PARAMETER 1
PARAM_NAME idEvent
PARAM_TYPE EWDBid
PARAM_DESCRIPTION DB identifier of the Event for which the caller
wants a list of associated amplitudes.

PARAMETER 2
PARAM_NAME iAmpType
PARAM_TYPE MAGNITUDE_TYPE
PARAM_DESCRIPTION Type of amplitude the caller is searching for.
"Amplitude Type" constants are defined in <earthworm_defs.h>.

PARAMETER 3
PARAM_NAME pAmpBuffer
PARAM_TYPE EWDB_PeakAmpStruct *
PARAM_DESCRIPTION Buffer allocated by the caller, where the function
will write the list of amplitudes retrieved.

PARAMETER 4
PARAM_NAME pStationBuffer
PARAM_TYPE EWDB_StationStruct *
PARAM_DESCRIPTION Buffer allocated by the caller, where the function
will write the list of station/channel info associated with
the amplitudes retrieved.  If pStationBuffer is set to NULL by the 
caller, then the function will not attempt to retrieve Station information
for the retrieved amplitudes.  If pStationBuffer is NOT NULL, then 
the function will fill out an EWDB_StationStruct for the channel 
associated with each amplitude.

PARAMETER 5
PARAM_NAME pNumAmpsFound
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of amplitudes found.

PARAMETER 6
PARAM_NAME pNumAmpsRetrieved
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of amplitudes retrieved and placed in the caller's buffer(pAmpBuffer).

PARAMETER 7
PARAM_NAME iBufferLen
PARAM_TYPE int
PARAM_DESCRIPTION Size of the pAmpBuffer buffer in as a multiple of
EWDB_PeakAmpStruct. (example: 15 structs)  If pStationBuffer is
not NULL, then it must also contain space for this many 
EWDB_StationStruct records.

DESCRIPTION Function retrieves a list of amplitudes of a given type
that are associated with a given DB Event.  Use idEvent to define the
DB Event for which you want a list of associated amplitudes.  If you want 
station information for the channel associated with each amplitude, 
allocate space for pStationBuffer, otherwise set pStationBuffer to NULL, 
and station information will not be retrieved.

NOTE This function only retrieves amplitudes that are directly associated
with an Event.  It will not retrieve amplitudes that have been associated
with an Event via a Magnitude.


*************************************************
************************************************/
int ewdb_api_GetAmpsByEvent(EWDBid idEvent, MAGNITUDE_TYPE iMagType,
       EWDB_PeakAmpStruct * pAmpBuffer, EWDB_StationStruct * pStationBuffer,
       int * pNumAmpsFound, int * pNumAmpsRetrieved, int iBufferLen);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetAmpsByOrigin

STABILITY NEW

SOURCE_LOCATION THIS_FILE

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Partial success.  A list of amplitudes was retrieved,
but the caller's buffer was not large enough to accomadate all of the
amplitudes found.  See pNumAmpsFound for the number of arrivals found
and pNumAmpsRetrieved for the number of amplitudes placed in the
caller's buffer.

PARAMETER 1
PARAM_NAME idOrigin
PARAM_TYPE EWDBid
PARAM_DESCRIPTION DB identifier of the Origin for which the caller
wants a list of associated amplitudes.

PARAMETER 2
PARAM_NAME iAmpType
PARAM_TYPE MAGNITUDE_TYPE
PARAM_DESCRIPTION Type of amplitude the caller is searching for.
"Amplitude Type" constants are defined in <earthworm_defs.h>.

PARAMETER 3
PARAM_NAME pAmpBuffer
PARAM_TYPE EWDB_PeakAmpStruct *
PARAM_DESCRIPTION Buffer allocated by the caller, where the function
will write the list of amplitudes retrieved.

PARAMETER 4
PARAM_NAME pStationBuffer
PARAM_TYPE EWDB_StationStruct *
PARAM_DESCRIPTION Buffer allocated by the caller, where the function
will write the list of station/channel info associated with
the amplitudes retrieved.  If pStationBuffer is set to NULL by the 
caller, then the function will not attempt to retrieve Station information
for the retrieved amplitudes.  If pStationBuffer is NOT NULL, then 
the function will fill out an EWDB_StationStruct for the channel 
associated with each amplitude.

PARAMETER 5
PARAM_NAME pNumAmpsFound
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of amplitudes found.

PARAMETER 6
PARAM_NAME pNumAmpsRetrieved
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of amplitudes retrieved and placed in the caller's buffer(pAmpBuffer).

PARAMETER 7
PARAM_NAME iBufferLen
PARAM_TYPE int
PARAM_DESCRIPTION Size of the pAmpBuffer buffer in as a multiple of
EWDB_PeakAmpStruct. (example: 15 structs)  If pStationBuffer is
not NULL, then it must also contain space for this many 
EWDB_StationStruct records.

DESCRIPTION Function retrieves a list of amplitudes of a given type
that are associated with a given DB Origin.  Use idOrigin to define the
DB Origin for which you want a list of associated amplitudes.  If you want 
station information for the channel associated with each amplitude, 
allocate space for pStationBuffer, otherwise set pStationBuffer to NULL, 
and station information will not be retrieved.

NOTE This function only retrieves amplitudes that are directly associated
with an Origin, or the Event with which that Origin is associated.  
It will not retrieve amplitudes that have been associated
with an Origin via a Magnitude.


*************************************************
************************************************/
int ewdb_api_GetAmpsByOrigin(EWDBid idOrigin, MAGNITUDE_TYPE iMagType,
      EWDB_PeakAmpStruct * pAmpBuffer, EWDB_StationStruct * pStationBuffer,
      int * pNumAmpsFound, int * pNumAmpsRetrieved, int iBufferLen);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetEventForOrigin

STABILITY NEW

SOURCE_LOCATION THIS_FILE

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

PARAMETER 1
PARAM_NAME idOrigin
PARAM_TYPE EWDBid
PARAM_DESCRIPTION The DB identifier of the Origin for
which the caller wants to obtain the DB Event.

PARAMETER 2
PARAM_NAME idEvent
PARAM_TYPE EWDBid *
PARAM_DESCRIPTION Pointer to an EWDBid value, where the 
function will write the DB ID of the Event associated 
with the given Origin.

DESCRIPTION Function returns the DB ID of the Event associated
with the given Origin.  If "zero" or "more than one" Events
are associated with the given origin, then an error is returned.


*************************************************
************************************************/
int ewdb_api_GetEventForOrigin(EWDBid idOrigin, EWDBid * idEvent);




/* TO BE COMMENTED */
int ewdb_api_GetNumStationsForEvent(EWDBid idEvent, int *pNumStations); 
int ewdb_api_BindToEvent (EWDBid idEvent, int CoreTable, EWDBid idCore, EWDBid *Out_idBind);
int ewdb_api_SetPrefer (EWDBid idEvent, int CoreTable, EWDBid idCore, EWDBid *Out_idPrefer);




/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetEventSummaryInfo

STABILITY NEW

SOURCE_LOCATION THIS_FILE

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Data for the given idEvent was not found.

PARAMETER 1
PARAM_NAME IN_idEvent
PARAM_TYPE EWDBid
PARAM_DESCRIPTION The DB identifier of the Event for
which the caller wants to obtain summary information.

PARAMETER 2
PARAM_NAME pEvent
PARAM_TYPE EWDB_EventListStruct *
PARAM_DESCRIPTION Pointer to an EWDB_EventListStruct that the
function will fill with the summary information for the given
Event.  All fields are filled.

DESCRIPTION Function returns the summary information(Event 
Info(including Source and Source-EventID of author of the 
preferred Origin), preferred Origin info, size of 
preferred Magnitude) for the given idEvent.


*************************************************
************************************************/
int ewdb_api_GetEventSummaryInfo(EWDBid IN_idEvent, EWDB_EventListStruct * pEvent);



/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMmENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY NEW

FUNCTION ewdb_api_GetArrivalsWChanInfo

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Partial success.  A list of arrivals was retrieved,
but the caller's buffer was not large enough to accomadate all of the
arrivals found.  See pNumArrivalsFound for the number of arrivals found
and pNumArrivalsRetrieved for the number of arrivals placed in the
caller's buffer.

PARAMETER 1
PARAM_NAME idOrigin
PARAM_TYPE EWDBid
PARAM_DESCRIPTION This function retrieves a list of arrivals associated
with an origin.  idOrigin is the database ID of the origin for which
the caller wants a list of arrivals.

PARAMETER 2
PARAM_NAME pArrivals
PARAM_TYPE EWDB_ArrivalStruct *
PARAM_DESCRIPTION Buffer allocated by the caller, where the function
will write the list of arrivals.

PARAMETER 3
PARAM_NAME pChannelBuffer
PARAM_TYPE EWDB_ChannelStruct *
PARAM_DESCRIPTION Buffer allocated by the caller, where the function
will write the list of channel structures that parallels the list
of arrivals.

PARAMETER 4
PARAM_NAME pNumArrivalsFound
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of found arrivals.

PARAMETER 5
PARAM_NAME pNumArrivalsRetrieved
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of arrivals placed in the callers buffer(pArrivals).

PARAMETER 6
PARAM_NAME iBufferLen
PARAM_TYPE int
PARAM_DESCRIPTION Size of the pArrivals (and pChannelBuffer) buffer as a 
multiple of EWDB_ArrivalStruct (and EWDB_ChannelStruct). (example: 15 structs)

DESCRIPTION The function retrieves a list of arrivals that are
associated with a given origin.  Along with each arrival struct the function
retrieves a parallel channel struct that contains channel information for
the given arrival.  Each Arrival.pStation is linked to each Channel.Comp
structure.  See EWDB_ArrivalStruct and EWDB_ChannelStruct for a
description of the information retrieved for each associated arrival/channel.

*************************************************
************************************************/
int ewdb_api_GetArrivalsWChanInfo(EWDBid IN_idOrigin,
                                  EWDB_ArrivalStruct * pArrivals,
                                  EWDB_ChannelStruct * pChannelBuffer,
                                  int * pNumArrivalsFound, int * pNumArrivalsRetrieved,
                                  int iBufferLen);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMmENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY NEW

FUNCTION ewdb_api_GetAmpsForOriginWChanInfo

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Partial success.  A list of amplitudes was retrieved,
but the caller's buffer was not large enough to accomadate all of the
records found.  See pNumItemsFound for the number of amplitudes found
and pNumItemsRetrieved for the number of records placed in the
caller's buffer.

PARAMETER 1
PARAM_NAME idOrigin
PARAM_TYPE EWDBid
PARAM_DESCRIPTION This function retrieves a list of amplitudes associated
with an origin, via origin -> pick -> amplitude association.  
idOrigin is the database ID of the origin for which the caller 
wants a list of amplitudes.

PARAMETER 2
PARAM_NAME iMagType
PARAM_TYPE int
PARAM_DESCRIPTION iMagType specifies a specific magnitude/amplitude
type that the caller wishes to retrieve(Mb,Ml,Ms,Mblg, etc.).
If iMagType <= 0, then all mag/amp types will be retrieved.  If
iMagtype > 0, then only amplitudes of the specified type will be
retrieved.

PARAMETER 3
PARAM_NAME pAmpBuffer
PARAM_TYPE EWDB_PeakAmpStruct *
PARAM_DESCRIPTION Buffer allocated by the caller, where the function
will write the list of peak amplitudes.

PARAMETER 4
PARAM_NAME pArrBuffer
PARAM_TYPE EWDB_ArrivalStruct *
PARAM_DESCRIPTION Buffer allocated by the caller, where the function
will write the list of arrivals that parallels the peak amplitude
buffer.

PARAMETER 5
PARAM_NAME pChanBuffer
PARAM_TYPE EWDB_ChannelStruct *
PARAM_DESCRIPTION Buffer allocated by the caller, where the function
will write the list of channel structures that parallels the list
of amplitudes.

PARAMETER 6
PARAM_NAME pNumItemsFound
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of items found.

PARAMETER 7
PARAM_NAME pNumItemsRetrieved
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of records placed in the callers buffers).

PARAMETER 8
PARAM_NAME iBufferLen
PARAM_TYPE int
PARAM_DESCRIPTION Size of the buffers as a 
multiple of the buffer's structure size. (example: 15 structs)

DESCRIPTION The function retrieves a list of amplitudes that are
associated with a given origin, by mapping the arrivals that are
associated with each origin, and then mapping the amplitude(s) 
associated with each pick.  <br>
Along with each amplitude struct the function
retrieves a parallel arrival and channel struct that contain 
arrival and channel information for the given peakamp.  
Each Arrival.pStation is linked to each Channel.Comp
structure.  See EWDB_PeakAmpStruct, EWDB_ArrivalStruct and 
EWDB_ChannelStruct for a description of the information 
retrieved for each associated amp/arrival/channel.

*************************************************
************************************************/
int ewdb_api_GetAmpsForOriginWChanInfo(EWDBid idOrigin, MAGNITUDE_TYPE iMagType,
                                       EWDB_PeakAmpStruct * pAmpBuffer,
                                       EWDB_ArrivalStruct * pArrBuffer,
                                       EWDB_ChannelStruct * pChanBuffer,
                                       int * pNumItemsFound, int * pNumItemsRetrieved,
                                       int iBufferLen);

/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMmENT
TYPE FUNCTION_PROTOTYPE

LIBRARY  EWDB_API_LIB

SUB_LIBRARY PARAMETRIC_API

LANGUAGE C

LOCATION THIS_FILE

STABILITY NEW

FUNCTION ewdb_api_GetAmpsForOriginMagWChanInfo

RETURN_TYPE int

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Fatal error.  See logfile for details.

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Partial success.  A list of amplitudes was retrieved,
but the caller's buffer was not large enough to accomadate all of the
records found.  See pNumItemsFound for the number of amplitudes found
and pNumItemsRetrieved for the number of records placed in the
caller's buffer.

PARAMETER 1
PARAM_NAME idOrigin
PARAM_TYPE EWDBid
PARAM_DESCRIPTION This function retrieves a list of amplitudes associated
with an origin, via origin -> magnitude -> amplitude association.  
idOrigin is the database ID of the origin for which the caller 
wants a list of amplitudes.

PARAMETER 2
PARAM_NAME iMagType
PARAM_TYPE int
PARAM_DESCRIPTION iMagType specifies a specific magnitude/amplitude
type that the caller wishes to retrieve(Mb,Ml,Ms,Mblg, etc.).
If iMagType <= 0, then all mag/amp types will be retrieved.  If
iMagtype > 0, then only amplitudes of the specified type will be
retrieved.

PARAMETER 3
PARAM_NAME pStaMagBuffer
PARAM_TYPE EWDB_StationMagStruct *
PARAM_DESCRIPTION Buffer allocated by the caller, where the function
will write the list of peak amplitudes, and associated magnitude
information.

PARAMETER 4
PARAM_NAME pChanBuffer
PARAM_TYPE EWDB_ChannelStruct *
PARAM_DESCRIPTION Buffer allocated by the caller, where the function
will write the list of channel structures that parallels the list
of amplitudes.

PARAMETER 5
PARAM_NAME pNumItemsFound
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of items found.

PARAMETER 6
PARAM_NAME pNumItemsRetrieved
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of records placed in the callers buffers).

PARAMETER 7
PARAM_NAME iBufferLen
PARAM_TYPE int
PARAM_DESCRIPTION Size of the buffers as a 
multiple of the buffer's structure size. (example: 15 structs)

DESCRIPTION The function retrieves a list of amplitudes that are
associated with a given origin, by mapping the magnitudes that are
associated with each origin, and then mapping the amplitude(s) 
associated with each magnitude.  <br>
Along with each amplitude struct the function
retrieves a parallel channel struct that contain 
channel information for the given peakamp.  
Each StaMag.pStation is linked to each Channel.Comp
structure.  See EWDB_StationMagStruct and 
EWDB_ChannelStruct for a description of the information 
retrieved for each associated amp/mag/channel.

*************************************************
************************************************/
int ewdb_api_GetAmpsForOriginMagWChanInfo(EWDBid IN_idOrigin, int iMagType,
                                          EWDB_StationMagStruct * pStaMagBuffer,
                                          EWDB_ChannelStruct * pChanBuffer,
                                          int * pNumItemsFound, int * pNumItemsRetrieved,
                                          int iBufferLen);

int ewdb_api_GetAmpsForOriginByPick(EWDBid IN_idOrigin, MAGNITUDE_TYPE IN_iMagType,
                                    EWDB_PeakAmpStruct * pAmpBuffer,
                                    int * pNumAmpsFound, int * pNumAmpsRetrieved,
                                    int iBufferLen);


int ewdb_api_CreateStaMag(EWDB_StationMagStruct *pStaMag);

int ewdb_api_CreateUnassocPick(EWDB_ArrivalStruct *pArrival);
int ewdb_api_GetEventID (EWDBid *idEvent, char *sSource, char *sSourceID);
int ewdb_api_CreatePeakAmp(EWDB_PeakAmpStruct *pPeakAmp);
int ewdb_api_GetPickByExternalID (char *sExtPickId, EWDBid *pidPick);
int ewdb_api_GetidOriginFromVersionNum (EWDB_OriginStruct *pOrigin);

/* ewdb_api_GetPicksByTime() returns a list of picks from the DB based on
   time and optionally idChan.
   Set IN_idChan <= 0 to query by time only.
   Set IN_idChan to a valid idChan to query by time/chan.
 **************************************************************************/
int ewdb_api_GetPicksByTime(time_t IN_tStart, time_t IN_tEnd, EWDBid idChan,
							              EWDB_ArrivalStruct * pArrivals,
                            int * pNumItemsFound, int * pNumItemsRetrieved,
                            int iBufferLen);

int ewdb_api_GetIDSource(EWDBid *pidSource, char * IN_szSource, 
                         int IN_iEWInstID, EWDBid IN_idInst);

/* Deletion Routines */
int ewdb_api_DeleteAllDataBeforeTime(int IN_tTime, int IN_iMaxRecsToDelete, 
                                     int * pSQLRetCode);
int ewdb_api_DeleteDataBeforeTime(int IN_tTime, int IN_iDatatypes, 
                                  int IN_iMaxRecsToDelete, int * pSQLRetCode);
int ewdb_api_DeletePeakAmpsBeforeTime(int IN_tTime, int IN_iMaxRecsToDelete, 
                                      int * pSQLRetCode);
int ewdb_api_DeletePicksBeforeTime(int IN_tTime, int IN_iMaxRecsToDelete, 
                                   int * pSQLRetCode);

int ewdb_api_GetSourceByID(EWDB_AuthorStruct * pAuthor);


int ewdb_api_GetMwFilterByID(EWDB_MwFilterStruct * pMwFilter);
/* fill in pMwFilter->idMwFilter, get the associated filter back in pMwFilter
   EWDB_RETURN_WARNING indicates function was unable to find filter for given ID
   EWDB_RETURN_FAILURE indicates function encountered unspecified error
  */

int ewdb_api_GetMwFilterByProperties(EWDB_MwFilterStruct * pMwFilter);
/* fill in pMwFilter filter properties and the idMwFilter back (as pMwFilter->idMwFilter), 
   If a matching filter does not already exist in the DB, one will be created
   and the ID returned.
   EWDB_RETURN_FAILURE indicates function encountered unspecified error
  */

int ewdb_api_CreateMwChanWithTS(EWDB_MwChanStruct * pMwChan);
/* creates an MwTimeSeries record based on the info in pMwTS.
   idChan and idMwFilter, as well as the non db-specific fields must
   all be filled out.
   EWDB_RETURN_FAILURE indicates function encountered unspecified error
  */


int ewdb_api_GetMwTimeSeriesByPick(EWDBid idPick, EWDB_MwTimeSeriesStruct * pMwTS);
/* fill in idPick, and get back the time series that matches the pick channel, and most clostly 
   matches the pick time.  If no records exist that start within 10 seconds of Pick.tPhase, then
   a warning will be returned.  Be sure to fill in pMwTS->cTSType in order to get the
   appropriate type(synthetic/real) data.
   Also fill in pMwTS->idMwFilter, in order to grab only data based on the proper filter.
   EWDB_RETURN_WARNING indicates function was unable to find a time series record that matched
   the criteria.
   EWDB_RETURN_FAILURE indicates function encountered unspecified error
  */

int ewdb_api_GetMwTimeSeriesByChan(double tReqStart, double tReqEnd, EWDB_MwTimeSeriesStruct * pMwTS);
/* fill in pMwTS->idChan and pMwTS->dStart, and tReqStart and tReqEnd, and get back the time series 
   that matches the given channel, and most clostly matches the pMwTS->dStart time.  
   If no records exist that start prior to tReqStart and end after tReqEnd then
   a warning will be returned.  Be sure to fill in pMwTS->cTSType in order to get the
   appropriate type(synthetic/real) data.
   EWDB_RETURN_WARNING indicates function was unable to find a time series record that matched
   the criteria.
   EWDB_RETURN_FAILURE indicates function encountered unspecified error
  */

int ewdb_api_CreateMw(EWDB_MwStruct * pMw);
/* creates an Mw record based on the info in pMw.
   idOrigin must be filled in.
   idMag must be valid if filled in.
   EWDB_RETURN_FAILURE indicates function encountered unspecified error
  */

int ewdb_api_AssocMwWithMag(EWDBid idMw, EWDBid idMag);
/* updates an Mw record, so that it becomes associated with a Magnitude, indicating
   that it is the final Mw for the Origin, and the Magnitude is derived from it.
   idMw and idMag must be valid.
   EWDB_RETURN_WARNING indicates that either idMw was invalid, idMag was invalid,
                       or Mw.idOrigin did not match Mag.idOrigin.
   EWDB_RETURN_FAILURE indicates function encountered unspecified error
  */


int ewdb_api_GetMwSolutionsForOrigin(EWDBid IN_idOrigin, EWDB_MwStruct * pMwBuffer, 
                                     int * pNumItemsFound, int * pNumItemsRetrieved, int iBufferLen);
/* retrieves all of the Mw records for a given Origin.  One of those Mw records
   should have a valid idMag value, which indicates it is the "final solution". 
   EWDB_RETURN_WARNING indicates the pMwBuffer wasn't large enough.
   EWDB_RETURN_FAILURE indicates function encountered unspecified error, including idOrigin was invalid.
 */

int ewdb_api_GetMwChans(EWDBid idMw, EWDB_MwChanStruct * pMwChansBuffer, 
                        int * pNumItemsFound, int * pNumItemsRetrieved, int iBufferLen);
/* retrieves all of the Mw Channel entires for a given Mw.  Each MwChan returned includes
   both sets of time series data associated with the MwChan computation. 
   EWDB_RETURN_WARNING indicates the pMwChansBuffer wasn't large enough.
   EWDB_RETURN_FAILURE indicates function encountered unspecified error, including idMw was invalid.
*/


/**************************
 *## C API Functions for Mwp
 **************************/
int ewdb_api_CreateIntDisp(EWDB_IntDispStruct *pIntDisp);
int ewdb_api_CreateStaMag (EWDB_StationMagStruct *pStaMag);
int ewdb_api_DeleteIntDispsBeforeTime(int IN_tTime, int IN_iMaxRecsToDelete, int * pSQLRetCode);
int ewdb_api_GetIntDispsByPick(EWDBid IN_idPick, MAGNITUDE_TYPE IN_iMagType,
                               EWDB_IntDispStruct* pIntDispBuffer,
                               int * pNumIntDispsFound, int * pNumIntDispsRetrieved, int iBufferLen);

int ewdb_api_GetIntDispsForOriginByPick(EWDBid IN_idOrigin, MAGNITUDE_TYPE IN_iMagType,
                                        EWDB_IntDispStruct* pAmpBuffer,
                                        int * pNumIntDispsFound, int * pNumIntDispsRetrieved,
                                        int iBufferLen);

int ewdb_apps_InsertIntDispWithMag (EWDB_StationMagStruct *pStaMag);

int ewdb_api_GetUnassocPicksByTime(time_t IN_tStart, time_t IN_tEnd, EWDBid IN_idChan,
                            EWDB_ArrivalStruct * pArrivals,
                            int * pNumItemsFound, int * pNumItemsRetrieved,
                            int iBufferLen);

int ewdb_api_GetUnassocPicksByTimeWChan(time_t IN_tStart, time_t IN_tEnd,
                                  EWDB_ArrivalStruct * pBuffer,
                                  EWDB_ChannelStruct * pChannelBuffer,
                                  int * pNumItemsFound, int * pNumItemsRetrieved,
                                  int iBufferLen);

int ewdb_api_GetCodaDurByPickID(EWDB_CodaDurationStruct * pCodaDur);

#endif /* _EWDB_ORA_API_PARAMETRIC_H */
