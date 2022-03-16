#ifndef COSMOS0PUTAWAY_h
#define COSMOS0PUTAWAY_h

#ifndef MINUTE
#define MINUTE        1
#endif
#ifndef HOUR
#define HOUR          0
#endif
#define COSMOSLINELEN		  83 /* Cosmos lines are 80 characters long, but we leave extra characters for the line ending \r\n\0 */
/* see pa_subs.h for more prototypes you might expect to be here*/
int FindTabularBase(int, long *, TRACE_REQ *, double, int);
long Median(int , long *);
int longCompareForQsort(const void *, const void *);

#define TAG_FILE        '.tag'        /* file containing the last known file tag */
#define GAP_FILL        "NaN" // this is what we want, but for the moment we'll use 0, because Nan breaks PRISM
/* #define GAP_FILL        "0" */ 
#define MAXTXT           150

#include "read_arc.h"

/* COSMOS info
*************************/
/* Some text path variables we'd like to pass along to the COSMOS0PA_next without passing too many parameters */
typedef struct
{
	/* the request portion */
	char    OutDir[4 * MAXTXT];	        /* save files to this directory */
	char    COSMOS0File[4 * MAXTXT];	/* save files to this filename */
	char    LibDir[4 * MAXTXT];     	/* v0 library location */
	char    EventArcFile[4 * MAXTXT];	/* name of the single arcfile we'll work with */
	HypoArc			arcmsg;             /* Hypocenter info goes here */
	char EventDate[MAXTXT];
	char EventTime[MAXTXT + 4];
}
COSMOS;

FILE    *COSMOS0fp;                      /* file pointer for the COSMOS0 output file */
FILE    *COSMOS0Libfp;                   /* file pointer for the COSMOS0 library file */
FILE    *EVENTARCfp;                     /* file pointer to the seismic event file */

static COSMOS *cosmos_info;
#endif /* COSMOS0PUTAWAY_h */
