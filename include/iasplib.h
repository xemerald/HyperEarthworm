/****************************************************************************
IASPLIB.H

These declarartions are to be included in any program calling the utility
functions in taulib or emiasp91.
---------------------------------------------------------------------------*/

#include "ttlim.h"

/* Function declarations (from taulib) */
void   bkin( FILE *, int, int, double [] );
int    brnset( int, char [10][PHASE_LENGTH], int [] );
void   depcor( int, FILE * );
void   depset( double, double [], FILE * );
void   findtt( int, double [], int *, double [], double [], 
               double [], double [], char [60][PHASE_LENGTH] );
void   fitspl( int, int, double [JOUT][4], double, double, double [5][JOUT] );
void   pdecu( int, int *, double, double, double, int );
void   r4sort( int, double [], int [] );
void   spfit( int, int );
int    tabin( FILE **, char * );
void   tauint( double, double, double, double, double, double *, double * ); 
void   tauspl( int, int, double [], double [5][JOUT] );
void   trtm( double, int *, double [], double [], double [], double [], 
             char [MAX][PHASE_LENGTH] );
double umod( double, int *, int );
double zmod( double, int, int );
