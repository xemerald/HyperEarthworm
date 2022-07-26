/*
   The "j" parameters (1 st line) are intended to be user settable:
        jsrc   Maximum number of discrete model slowness samples above
               the maximum source depth of interest.
        jseg   Maximum number of different types of travel-times
               considered.
        jbrn   Maximum number of different travel-time branches to be
               searched.
        jout   Maximum length of all travel-time branches strung
               together.
        jtsm   Maximum length of the tau depth increments.
        jxsm   Maximum number of x-values needed for the depth
               increments.
        jbrnu  Maximum length of the up-going branches.
        jbrna  Maximum length of branches which may need
               re-interpolation.
*/
#define PSZ     char *
#define	JSRC	150
#define	JSEG	30
#define	JBRN	100
#define	JOUT	2500
#define	JTSM	350
#define	JXSM	JBRN
#define	JBRNU	JBRN
#define	JBRNA	JBRN
#define	JREC	JTSM+JXSM
#define	JTSM0	JTSM+1

#define RAD          0.017453292519943	// Convert degrees to radians
#define DEG         57.295779513083     // Convert radians to degrees
//#define PI           4.*atan (1.0)
//#define TWOPI        4.*atan (1.0)*2.

#define MAX		60
#define PHASE_LENGTH	8

