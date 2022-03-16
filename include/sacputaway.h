
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: sacputaway.h 1625 2004-07-16 20:30:33Z lombard $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.5  2004/07/16 20:30:33  lombard
 *     Modified to support SCNL (includes SEED location code)
 *
 *     Revision 1.4  2002/06/28 21:06:22  lucky
 *     Lucky's pre-departure checkin. Most changes probably had to do with bug fixes
 *     in connection with the GIOC scaffold.
 *
 *     Revision 1.3  2002/03/22 18:24:23  lucky
 *     Changed prototype for SACPABase_init
 *
 *     Revision 1.2  2001/08/07 16:48:37  lucky
 *     Pre v6.0 checkin
 *
 *     Revision 1.1  2001/04/12 03:06:13  lombard
 *     Initial revision
 *
 *
 *
 */

/*
 * This is sacputaway.h. It contains structures and function prototypes
 * for the SAC putaway routines. If you are looking for SAC header
 * information, see sachead.h
 */

#ifndef SACPUTAWAY_h
#define SACPUTAWAY_h

#include <earthworm.h>
#include <sachead.h>

#if defined (_SPARC)
# define SAC_DIFFERENT_PLATFORM "intel"
#elif defined (_INTEL)
# define SAC_DIFFERENT_PLATFORM "sparc"
#endif

/* ERROR CONSTANTS
*******************************/
#define SACPAB_ALREADY_INIT -4
#define SACPAB_DIRECTORY_TOO_LONG -5
#define SACPAB_NULL_POINTERS -6
#define SACPAB_SCNL_ALREADY_OPEN -7
#define SACPAB_SCNL_NOT_OPEN -8
#define SACPAB_NOT_INIT -9
#define SACPAB_FOPEN_FAILED -11
#define SACPAB_OUTPUT_FORMAT_TOO_LONG -12

/* Other CONSTANTS
*******************************/
#define     MAXTXT           150
#define     MIN_OUTBUFFER_LEN 32000

#define SAC_MAX_POLES_OR_ZEROES 100

/* constant to denote whether we are saving a P or S wave*/
#define		PWAVE	123
#define		SWAVE	456


/* 
 * Global variable:  needed to signal to the caller whether 
 *   any gaps were filled.  Of course, it would be cleaner to 
 *   pass this along as a parameter, but given the proliferation
 *   of these routines, it would take a lot of work to change them all.
 *   So, kludge it is.   lucky 19 June 2002.
 */
int 	GapCount;


typedef struct _SAC_PZNum
{
  double      dReal;
  double      dImag;
} SAC_PZNum;

typedef struct _SAC_ResponseStruct
{
  double    dGain;
  int         iNumPoles;
  int         iNumZeroes;
  SAC_PZNum  Poles[SAC_MAX_POLES_OR_ZEROES];
  SAC_PZNum  Zeroes[SAC_MAX_POLES_OR_ZEROES];
} SAC_ResponseStruct;

/** SACFilelist is used to create an ordered list of files.
    The filelist is used by several SAC macros **/
typedef struct _SACFileList
{
    char    filename[MAXTXT];   /* Sac file name */
    double  sort_param;         /* sort parameter - starttime */
} SACFileListStruct;

typedef struct _SAC_OriginStruct
{
  double dLat;       /* ->evla */
  double dLon;       /* ->evlo */
  double dElev;      /* ->evel */
  double tOrigin;    /* ->o */
} SAC_OriginStruct;

typedef struct _SAC_ArrivalStruct
{
  double tPhase;     /* ->a */
  char   cPhase;     /* ->ka[0] */  /* only P and S are supported by SAC */
  double dCodaLen;   /* ->f */
  float  dDist;      /* ->dist */
  float  dAzm;       /* ->az */
  int    iPhaseWt;   /* ->ka[2] */
  char   cFMotion;   /* ->ka[1] */
  char   cOnset;     /* ->ka[3] */
} SAC_ArrivalStruct;

typedef struct _SAC_AmpPickStruct
{
  int    PickType;   /* 0=zero-to-peak, 1=peak-to-peak */
  double  ZP_time;   /* ->t0 */
  double  ZP_amp;    /* ->user0 */
  double  PPmin_time;/* ->t1 */
  double  PPmax_time;/* ->t2 */
  double  PP_amp;    /* ->user1 */
} SAC_AmpPickStruct;

typedef struct _SAC_StationStruct
{
  float dLat; /* ->stla */
  float dLon; /* ->stlo */
  float dElev; /* ->stel */
  int   bResponseIsValid;
  SAC_ResponseStruct * pResponse;
} SAC_StationStruct;


/* Supports external (urban hazards) channel info */
typedef struct _SAC_ExtChanStruct
{
  int	iGain;					/* user0 */
  float dFullscale; 			/* user1 */
  float dSensitivity;			/* user2 */
  float dDamping; 				/* user3 */
  float dNaturalFrequency; 		/* user4 */
  int	iSensorType;			/* user5 */
  float tLastGPSLock; 		 	/* user6 */
  float dAzm; 					/* cmpaz */
  float dDip;					/* cmpinc */
} SAC_ExtChanStruct;




/************************************/
/** FUNCTION PROTOTYPES            **/
/************************************/

/* SACPABase interface routines.  
   These are the bottom layer routines  
   that convert data into SAC format.
*************************************/
int SACPABase_Debug(int);
int SACPABase_SetOutputFormat(char *);
int SACPABase_init(int, char *, int, char * );
int SACPABase_next_ev(char *, double, SAC_OriginStruct *);
int SACPABase_next_ev_review (char *, int, SAC_OriginStruct *);
int SACPABase_next_scnl(char *, char *, char *, char *);
int SACPABase_write_trace(TRACE_REQ *, double);
int SACPABase_write_parametric(SAC_ArrivalStruct *, int);
int SACPABase_write_amppicks(SAC_AmpPickStruct *); 
int SACPABase_write_stainfo(SAC_StationStruct *);
int SACPABase_write_extinfo(SAC_ExtChanStruct *);
int SACPABase_end_scnl (void);
int SACPABase_end_ev (void);
int SACPABase_close (void);



#endif
