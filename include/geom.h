/*
 *
 *   Functions for geometric computations
 */

#ifndef GEOM_H
#define GEOM_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int 	geom_area (int *, int, float *, float *, float *, float *);
int 	geom_cntsct (int, float *, float *, float *, float *);
int 	geom_isect (float, float, float, float, float *, float *);

#endif  /* GEOM_H */


