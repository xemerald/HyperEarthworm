
#ifndef COSMOS0PUTAWAY_H
#define COSMOS0PUTAWAY_H
/*
// COSMOS0.h -- General include for COSMOS0 library
*/
#define _COSMOS0_VERSION "1.20"
// DEFINES

// Define code for type of machine COSMOS0 is being compiled on.
// '6' is 80x86 machine

#define MACHINE         '6'

/* // Define symbols for missing data. */
#define NODATA          "-32767."
#define NOTIME          "-2147472000" /* // Dec 14, 1901 00:00 00 GMT, near largest long */
#define NOCHAR          '_'
#define NOSTRG          ""
#define NOLIST          "0"

/* // Integer defines for standard variable types */
#define CHR             1       /* // char 1 byte */
#define MIN             2       /* // char number 1 byte */
#define STR             3       /* // string */
#define BTS             4       /* // char 1 byte */
#define SHT             5       /* // short 2 bytes */
#define LNG             6       /* // long 4 bytes */
#define FLT             7       /* // float 4 bytes */
#define DBL             8       /* // double 8 bytes */
#define STI             9       /* // struct stat_ident 8 bytes */
#define STT             10      /* // st_time 4 bytes */
#define MST             11      /* // ms_time 8 bytes */
#define LLT             12      /* // lon_lat 8 bytes */
#define CAL             13      /* // struct calib 500 bytes */
#define CPX             14      /* // struct complex 16 bytes */
#define BTW             15      /* // unsigned short 2 bytes */

/* // Structure types or identifiers */
#define NO_STRUCT       0
#define STAT_IDENT      1
#define STRUCTTAG       2
#define TERMINATOR      3
#define EQUIPMENT       4
#define STATIONCOMP     5
#define MUXDATA         6
#define DESCRIPTRACE    7
#define LOCTRACE        8
#define CALIBRATION     9
#define FEATURE         10
#define RESIDUAL        11
#define EVENT           12
#define EV_DESCRIPT     13
#define ORIGIN          14
#ifdef ERROR
#undef ERROR   /* stupid NT */
#endif
#define ERROR           15
#define FOCALMECH       16
#define MOMENT          17
#define VELMODEL        18
#define LAYERS          19
#define COMMENT         20
#define PROFILE         21
#define SHOTGATHER      22
#define CALIB           23
#define COMPLEX         24
#define TRIGGERS        25
#define TRIGSETTING     26
#define EVENTSETTING    27
#define DETECTOR        28
#define ATODINFO        29
#define TIMECORRECTION  30
#define INSTRUMENT      31
#define CHANSET         32

#define TOTAL_STRUCTS   32




							 /*
							 //---------------------------------------------------------------------
							 // STRUCTURE DEFINITIONS

							 // COSMOS0_FORM:  Information on each COSMOS0 structure used for input from ascii,
							 //             output to ascii, loading or unloading the database, etc.

							 // antipadding
							 */
#pragma pack(1)





/* // COSMOS0_STRUCTTAG:  Structure to identify structures when archived together */

#define ST_MAGIC   'S'       /* // magic character for sync in structtag */









/* // COSMOS0_CALIBRATION: Calibration information for a station component */

#define NOCALPTS 30










/* // remove anti-padding */
/* // syntax may be wrong - might be #pragma pack instead */

#pragma pack()

/* //--------------------------------------------------------------------- */
/* // Function prototypes for COSMOS0 library functions */

/* // Manifest constants ------------------------------------------------- */
#define COSMOS0_EOF      -1
#define COSMOS0_READONLY  1
#define COSMOS0_READWRITE 2
#define COSMOS0_APPEND    3
#define COSMOS0_CREATE    4

#ifndef TRUE
#define TRUE           1
#endif
#ifndef FALSE
#define FALSE          0
#endif

/* // Input / output functions ------------------------------------------- */

/* // Structure initialization ------------------------------------------- */


/* // Time functions ----------------------------------------------------- */


/* //--------------------------------------------------------------------- */

#endif
