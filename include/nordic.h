#ifndef NORDIC_H
#define NORDIC_H



#include <read_arc.h>
#include <rw_mag.h>


/* Some constants necessary for conversion */
#define MAX_NOR_PHASES		1000
#define MAX_NOR_LOCATIONS	100
//#define LOC_AGENCY	"ARC"	/* Location agency, 3 chars */
#define CHANNEL_ID_COMMAND	"CHANNELID"
#define BINDER_ID_COMMAND "BINDERID"
#define VERSION_ID_COMMAND "VERSIONID"





/* Structures to store a location - This corresponds to the type 1 lines */

/* Check SEISAN manual for details */
typedef struct NordicMag_ {
	int isValid; /* Magnitude is valid if 1 */
	float mag; /* Value of the magnitude */
	char magtype; /* Type of magnitude */
	char agency[4]; /* Magnitude reporting agency */
} NordicMag;

typedef struct NordicLoc_ {
	int isValid; /* Location may be invalid. Set to 1 if it is valid */
	double ot; /* Origin time */
	char locModel; /* Location model */
	char distIndicator; /* Distance indicator */
	char eventType; /* Event type */
	float lat; /* Latitude */
	float lon; /* Longitude */
	float z; /* Depth */
	int hasZ; /* 1 Indicates the existance of a depth in the solution */
	char zIndicator; /* Indicates a fixed depth with 'F' */
	char lIndicator; /* Indicates a fixed location with '*' */
	char agency[4]; /* Location agency - 3 char and null termination */
	int nsta; /* Number of stations */
	float rms; /* RMS of solution */
	NordicMag magnitudes[3]; /* Array of magnitudes */
//	size_t nmagnitudes; /* Number of magnitudes */
} NordicLoc;

typedef struct NordicHypError_ {
	int gap;
	float ot_err;
	float lat_err;
	float lon_err;
	float z_err;
	float cov_xy;
	float cov_xz;
	float cov_yz;
} NordicHypError;

typedef struct NordicPhase_ {
	char station[6];
	char channel[4];
	char network[3];
	char location[3];
	double at;
	int weight;
	char qualityIndicator;
	char phaseIndicator[9];
	char autoIndicator; /* A for automatic picks */
	char firstMotion;
	int duration; /* In seconds */
	double amplitude;
	double period;
	double dirApproach;
	double velocity;
	int azimuth;
	double atRMS;
	double distance;
} NordicPhase;

/* Structure for nordic event */
typedef struct Nordic_ {
	NordicLoc* locations[MAX_NOR_LOCATIONS]; /* Type 1 lines - 1st is primary */
	NordicLoc* primloc; /* Pointer to the primary location */
	size_t nlocations; /* Number of locations */
	NordicHypError* hypErr; /* Hypocenter error line */
	NordicPhase* phases[MAX_NOR_PHASES];
	size_t nphases;
	char* waverefs[MAX_NOR_PHASES]; /* Reference to waveforms */
	size_t nwaveref;
	char** comments;
	size_t ncomments;
} Nordic;






/* Functions */

int parseNordic( Nordic** event, char* buf, int nbuf );
int NorAddWaveRef( Nordic* event, char* waveref );

/******************************************************************************
 *    fwriteNor                                                               *
 * Write a seisan event to a stream.                                          *
 *                                                                            *
 * Input arguments:                                                           *
 * FILE* str: Output stream                                                   *
 * Nordic* event: Seisan event                                                *
 * NordicOptions* useroptions: Pointer to options structure, Optional         *
 *                                                                            *
 * Output arguments:                                                          *
 * - Integer number of characters written                                     *
 ******************************************************************************/
int fwriteNor( FILE* str, Nordic* event );


/******************************************************************************
 *    arc2nor                                                                 *
 * Convert a hyp2000arc event to a seisan event                               *
 *                                                                            *
 * Input arguments:                                                           *
 * Nordic **event: Null pointer to the new seisan event                       *
 * HypoArc *arc: Pointer to the hyp2000arc structure                          *
 * MAG_INFO *mag: Optional - Pointer to a magnitude structure to include an ML*
 *                Set null if it is not required                              *
 *                                                                            *
 * Output arguments:                                                          *
 * - 1 or -1 if conversion was Ok or not                                      *
 ******************************************************************************/
int arc2nor( Nordic **event, HypoArc *arc, MAG_INFO *mag, char* agency );

/******************************************************************************
 *    nor2arc                                                                 *
 * Convert a seisan event into a hyp2000arc message                           *
 *                                                                            *
 * Input arguments:                                                           *
 * HypoArc *arc: Pointer to the hyp2000arc structure to be filled with data   *
 * Nordic *event: Pointer to the seisan event                                 *
 *                                                                            *
 * Output arguments:                                                          *
 * - 1 or -1 if conversion was Ok or not                                      *
 ******************************************************************************/
int nor2arc( HypoArc* arc, Nordic *event );


/******************************************************************************
 *    freeNordic                                                              *
 * Free the memory allocated when creating a seisan event                     *
 *                                                                            *
 * Input arguments:                                                           *
 * Nordic **event: Pointer to the seisan event                                *
 ******************************************************************************/
void freeNordic(Nordic *event);





#endif

