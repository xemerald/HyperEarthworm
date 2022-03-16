/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: fft99.c 483 2001-03-31 00:46:48Z lombard $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2001/03/31 00:46:48  lombard
 *     Initial revision
 *
 *
 *
 */

/* fft99.f -- translated by f2c (version 19991025).*/
/*
 * Hand-editted after f2c to make it look a little more like C.
 * But there is no hiding the fact that this was once Fortran.
 * Read the comments with caution! Pete Lombard, January 2001
 */
#include <math.h>
#include "fft99.h"

/* Internal function prototypes */
static void fact(long n, long *ifax);
static void cftrig(long n, double *trigs);
static void vpassm(double *a, double *b, double *c, double *d, 
                   double *trigs, long inc1, long inc2, long inc3, long inc4, 
                   long lot, long n, long ifac, long la);
static void fft99a(double *a, double *work, double *trigs, long inc, long jump,
                   long n, long lot);
static void fft99b(double *work, double *a, double *trigs, long inc, long jump,
                   long n, long lot);
static void fax(long *ifax, long n);
static void fftrig(double *trigs, long n);


/* Function definitions */
void cfft99(double *a, double *work, double *trigs, long *ifax, long inc, 
            long jump, long n, long lot, long isign)
{
  /* Local variables */
  static long nfax, i, j, k, l, m, ibase, jbase;
  static double himag, hreal;
  static long ilast, i1, i2, la, nh, nn;
  static long igo, ink, jnk, jum, jst;


  /* PURPOSE      PERFORMS MULTIPLE FAST FOURIER TRANSFORMS.  THIS PACKAGE 
 *              WILL PERFORM A NUMBER OF SIMULTANEOUS COMPLEX PERIODIC 
 *              FOURIER TRANSFORMS OR CORRESPONDING INVERSE TRANSFORMS. 
 *              THAT IS, GIVEN A SET OF COMPLEX GRIDPOINT VECTORS, THE 
 *              PACKAGE RETURNS A SET OF COMPLEX FOURIER 
 *              COEFFICIENT VECTORS, OR VICE VERSA.  THE LENGTH OF THE 
 *              TRANSFORMS MUST BE A NUMBER GREATER THAN 1 THAT HAS 
 *              NO PRIME FACTORS OTHER THAN 2, 3, AND 5. 
 *
 *              THE PACKAGE CFFT99 CONTAINS SEVERAL USER-LEVEL ROUTINES: 
 *
 *            CFTFAX 
 *                AN INITIALIZATION ROUTINE THAT MUST BE CALLED ONCE 
 *                BEFORE A SEQUENCE OF CALLS TO CFFT99 
 *                (PROVIDED THAT N IS NOT CHANGED). 
 *
 *            CFFT99 
 *                THE ACTUAL TRANSFORM ROUTINE ROUTINE, CABABLE OF 
 *                PERFORMING BOTH THE TRANSFORM AND ITS INVERSE. 
 *                HOWEVER, AS THE TRANSFORMS ARE NOT NORMALIZED, 
 *                THE APPLICATION OF A TRANSFORM FOLLOWED BY ITS 
 *                INVERSE WILL YIELD THE ORIGINAL VALUES MULTIPLIED 
 *                BY N. 
 *
 *
 *
 * USAGE        LET N BE OF THE FORM 2**P * 3**Q * 5**R, WHERE P .GE. 0, 
 *              Q .GE. 0, AND R .GE. 0.  THEN A TYPICAL SEQUENCE OF 
 *              CALLS TO TRANSFORM A GIVEN SET OF COMPLEX VECTORS OF 
 *              LENGTH N TO A SET OF (UNSCALED) COMPLEX FOURIER 
 *              COEFFICIENT VECTORS OF LENGTH N IS 
 *
 *                   long ifax[13];
 *                   double trigs[2*n];
 *                   double a[...], work[...]
 *
 *                   cftfax (n, ifax, trigs);
 *                   cfft99 (a,work,trigs,ifax,inc,jump,n,lot,isign);
 *
 *              THE OUTPUT VECTORS OVERWRITE THE INPUT VECTORS, AND 
 *              THESE ARE STORED IN A.  WITH APPROPRIATE CHOICES FOR 
 *              THE OTHER ARGUMENTS, THESE VECTORS MAY BE CONSIDERED 
 *              EITHER THE ROWS OR THE COLUMNS OF THE ARRAY A. 
 *              SEE THE INDIVIDUAL WRITE-UPS FOR CFTFAX AND 
 *              CFFT99 BELOW, FOR A DETAILED DESCRIPTION OF THE 
 *              ARGUMENTS. 
 *
 * HISTORY      THE PACKAGE WAS WRITTEN BY CLIVE TEMPERTON AT ECMWF IN 
 *              NOVEMBER, 1978.  IT WAS MODIFIED, DOCUMENTED, AND TESTED 
 *              FOR NCAR BY RUSS REW IN SEPTEMBER, 1980.  IT WAS 
 *              FURTHER MODIFIED FOR THE FULLY COMPLEX CASE BY DAVE 
 *              FULKER IN NOVEMBER, 1980. 
 *
 * ----------------------------------------------------------------------- 
 *
 *  cftfax (n,ifax,trigs) 
 *
 * PURPOSE      A SET-UP ROUTINE FOR CFFT99.  IT NEED ONLY BE 
 *              CALLED ONCE BEFORE A SEQUENCE OF CALLS TO CFFT99, 
 *              PROVIDED THAT N IS NOT CHANGED. 
 *
 * ARGUMENT     long ifax[13]; double trigs[2*n];
 * DIMENSIONS 
 *
 * ARGUMENTS 
 *
 * ON INPUT     N 
 *               AN EVEN NUMBER GREATER THAN 1 THAT HAS NO PRIME FACTOR 
 *               GREATER THAN 5.  N IS THE LENGTH OF THE TRANSFORMS (SEE 
 *               THE DOCUMENTATION FOR CFFT99 FOR THE DEFINITION OF 
 *               THE TRANSFORMS). 
 *
 *              IFAX 
 *               AN INTEGER ARRAY.  THE NUMBER OF ELEMENTS ACTUALLY USED 
 *               WILL DEPEND ON THE FACTORIZATION OF N.  DIMENSIONING 
 *               IFAX FOR 13 SUFFICES FOR ALL N LESS THAN 1 MILLION. 
 *
 *              TRIGS 
 *               AN ARRAY OF DIMENSION 2*N 
 *
 * ON OUTPUT    IFAX 
 *               CONTAINS THE FACTORIZATION OF N.  IFAX[0] IS THE 
 *               NUMBER OF FACTORS, AND THE FACTORS THEMSELVES ARE STORED 
 *               IN IFAX[1],IFAX[2],...  IF N HAS ANY PRIME FACTORS 
 *               GREATER THAN 5, IFAX[0] IS SET TO -99. 
 *
 *              TRIGS 
 *               AN ARRAY OF TRIGONOMETRIC FUNCTION VALUES SUBSEQUENTLY 
 *               USED BY THE CFT ROUTINES. 
 *
 * ----------------------------------------------------------------------- 
 *
 * cfft99 (a,work,trigs,ifax,inc,jump,n,lot,isign) 
 *
 * PURPOSE      PERFORM A NUMBER OF SIMULTANEOUS (UNNORMALIZED) COMPLEX 
 *              PERIODIC FOURIER TRANSFORMS OR CORRESPONDING INVERSE 
 *              TRANSFORMS.  GIVEN A SET OF COMPLEX GRIDPOINT 
 *              VECTORS, THE PACKAGE RETURNS A SET OF 
 *              COMPLEX FOURIER COEFFICIENT VECTORS, OR VICE 
 *              VERSA.  THE LENGTH OF THE TRANSFORMS MUST BE A 
 *              NUMBER HAVING NO PRIME FACTORS OTHER THAN 
 *              2, 3, AND 5.  THIS ROUTINE IS 
 *              OPTIMIZED FOR USE ON THE CRAY-1. 
 *
 * ARGUMENT     double a[2*n*inc+(lot-1)*jump], work[2*n*lot]
 * DIMENSIONS   double trigs[2*n], long ifax[13]
 *
 * ARGUMENTS 
 *
 * ON INPUT     A 
 *               A COMPLEX ARRAY OF LENGTH N*INC+(LOT-1)*JUMP CONTAINING 
 *               THE INPUT GRIDPOINT OR COEFFICIENT VECTORS.  THIS ARRAY 
 *               OVERWRITTEN BY THE RESULTS. 
 *
 *               N.B. ALTHOUGH THE ARRAY A IS USUALLY CONSIDERED TO BE OF 
 *               TYPE COMPLEX IN THE CALLING PROGRAM, IT IS TREATED AS 
 *               REAL WITHIN THE TRANSFORM PACKAGE.  THIS REQUIRES THAT 
 *               SUCH TYPE CONFLICTS ARE PERMITTED IN THE USER^S 
 *               ENVIRONMENT, AND THAT THE STORAGE OF COMPLEX NUMBERS 
 *               MATCHES THE ASSUMPTIONS OF THIS ROUTINE.  THIS ROUTINE 
 *               ASSUMES THAT THE REAL AND IMAGINARY PORTIONS OF A 
 *               COMPLEX NUMBER OCCUPY ADJACENT ELEMENTS OF MEMORY.  IF 
 *               THESE CONDITIONS ARE NOT MET, THE USER MUST TREAT THE 
 *               ARRAY A AS REAL (AND OF TWICE THE ABOVE LENGTH), AND 
 *               WRITE THE CALLING PROGRAM TO TREAT THE REAL AND 
 *               IMAGINARY PORTIONS EXPLICITLY. 
 *
 *              WORK 
 *               A COMPLEX WORK ARRAY OF LENGTH N*LOT OR A REAL ARRAY 
 *               OF LENGTH 2*N*LOT.  SEE N.B. ABOVE. 
 *
 *              TRIGS 
 *               AN ARRAY SET UP BY CFTFAX, WHICH MUST BE CALLED FIRST. 
 *
 *              IFAX 
 *               AN ARRAY SET UP BY CFTFAX, WHICH MUST BE CALLED FIRST. 
 *
 *
 *               N.B. IN THE FOLLOWING ARGUMENTS, INCREMENTS ARE MEASURED 
 *               IN WORD PAIRS, BECAUSE EACH COMPLEX ELEMENT IS ASSUMED 
 *               TO OCCUPY AN ADJACENT PAIR OF WORDS IN MEMORY. 
 *
 *              INC 
 *               THE INCREMENT (IN WORD PAIRS) BETWEEN SUCCESSIVE ELEMENT 
 *               OF EACH (COMPLEX) GRIDPOINT OR COEFFICIENT VECTOR 
 *               (E.G.  INC=1 FOR CONSECUTIVELY STORED DATA). 
 *
 *              JUMP 
 *               THE INCREMENT (IN WORD PAIRS) BETWEEN THE FIRST ELEMENTS 
 *               OF SUCCESSIVE DATA OR COEFFICIENT VECTORS.  ON THE CRAY- 
 *               TRY TO ARRANGE DATA SO THAT JUMP IS NOT A MULTIPLE OF 8 
 *               (TO AVOID MEMORY BANK CONFLICTS).  FOR CLARIFICATION OF 
 *               INC AND JUMP, SEE THE EXAMPLES BELOW. 
 *
 *              N 
 *               THE LENGTH OF EACH TRANSFORM (SEE DEFINITION OF 
 *               TRANSFORMS, BELOW). 
 *
 *              LOT 
 *               THE NUMBER OF TRANSFORMS TO BE DONE SIMULTANEOUSLY. 
 *
 *              ISIGN 
 *               = -1 FOR A TRANSFORM FROM GRIDPOINT VALUES TO FOURIER 
 *                    COEFFICIENTS. 
 *               = +1 FOR A TRANSFORM FROM FOURIER COEFFICIENTS TO 
 *                    GRIDPOINT VALUES. 
 *
 * ON OUTPUT    A 
 *               IF ISIGN = -1, AND LOT GRIDPOINT VECTORS ARE SUPPLIED, 
 *               EACH CONTAINING THE COMPLEX SEQUENCE: 
 *
 *               G(0),G(1), ... ,G(N-1)  (N COMPLEX VALUES) 
 *
 *               THEN THE RESULT CONSISTS OF LOT COMPLEX VECTORS EACH 
 *               CONTAINING THE CORRESPONDING N COEFFICIENT VALUES: 
 *
 *               C(0),C(1), ... ,C(N-1)  (N COMPLEX VALUES) 
 *
 *               DEFINED BY: 
 *                 C(K) = SUM(J=0,...,N-1)( G(J)*EXP(-2*I*J*K*PI/N) ) 
 *                 WHERE I = SQRT(-1) 
 *
 *
 *               IF ISIGN = +1, AND LOT COEFFICIENT VECTORS ARE SUPPLIED, 
 *               EACH CONTAINING THE COMPLEX SEQUENCE: 
 *
 *               C(0),C(1), ... ,C(N-1)  (N COMPLEX VALUES) 
 *
 *               THEN THE RESULT CONSISTS OF LOT COMPLEX VECTORS EACH 
 *               CONTAINING THE CORRESPONDING N GRIDPOINT VALUES: 
 *
 *               G(0),G(1), ... ,G(N-1)  (N COMPLEX VALUES) 
 *
 *               DEFINED BY: 
 *                 G(J) = SUM(K=0,...,N-1)( G(K)*EXP(+2*I*J*K*PI/N) ) 
 *                 WHERE I = SQRT(-1) 
 *
 *
 *               A CALL WITH ISIGN=-1 FOLLOWED BY A CALL WITH ISIGN=+1 
 *               (OR VICE VERSA) RETURNS THE ORIGINAL DATA, MULTIPLIED 
 *               BY THE FACTOR N. 
 *
 *
 * EXAMPLE       GIVEN A 64 BY 9 GRID OF COMPLEX VALUES, STORED IN 
 *               A 66 BY 9 COMPLEX ARRAY, A, COMPUTE THE TWO DIMENSIONAL 
 *               FOURIER TRANSFORM OF THE GRID.  FROM TRANSFORM THEORY, 
 *               IT IS KNOWN THAT A TWO DIMENSIONAL TRANSFORM CAN BE 
 *               OBTAINED BY FIRST TRANSFORMING THE GRID ALONG ONE 
 *               DIRECTION, THEN TRANSFORMING THESE RESULTS ALONG THE 
 *               ORTHOGONAL DIRECTION. 
 *
 *               COMPLEX A(66,9), WORK(64,9) 
 *               REAL TRIGS1(128), TRIGS2(18) 
 *               INTEGER IFAX1(13), IFAX2(13) 
 *
 *               SET UP THE IFAX AND TRIGS ARRAYS FOR EACH DIRECTION: 
 *
 *               CALL CFTFAX(64, IFAX1, TRIGS1) 
 *               CALL CFTFAX( 9, IFAX2, TRIGS2) 
 *
 *               IN THIS CASE, THE COMPLEX VALUES OF THE GRID ARE 
 *               STORED IN MEMORY AS FOLLOWS (USING U AND V TO 
 *               DENOTE THE REAL AND IMAGINARY COMPONENTS, AND 
 *               ASSUMING CONVENTIONAL FORTRAN STORAGE): 
 *
 *   U(1,1), V(1,1), U(2,1), V(2,1),  ...  U(64,1), V(64,1), 4 NULLS, 
 *
 *   U(1,2), V(1,2), U(2,2), V(2,2),  ...  U(64,2), V(64,2), 4 NULLS, 
 *
 *   .       .       .       .         .   .        .        . 
 *   .       .       .       .         .   .        .        . 
 *   .       .       .       .         .   .        .        . 
 *
 *   U(1,9), V(1,9), U(2,9), V(2,9),  ...  U(64,9), V(64,9), 4 NULLS. 
 *
 *               WE CHOOSE (ARBITRARILY) TO TRANSORM FIRST ALONG THE 
 *               DIRECTION OF THE FIRST SUBSCRIPT.  THUS WE DEFINE 
 *               THE LENGTH OF THE TRANSFORMS, N, TO BE 64, THE 
 *               NUMBER OF TRANSFORMS, LOT, TO BE 9, THE INCREMENT 
 *               BETWEEN ELEMENTS OF EACH TRANSFORM, INC, TO BE 1, 
 *               AND THE INCREMENT BETWEEN THE STARTING POINTS 
 *               FOR EACH TRANSFORM, JUMP, TO BE 66 (THE FIRST 
 *               DIMENSION OF A). 
 *
 *               CALL CFFT99( A, WORK, TRIGS1, IFAX1, 1, 66, 64, 9, -1) 
 *
 *               TO TRANSFORM ALONG THE DIRECTION OF THE SECOND SUBSCRIPT 
 *               THE ROLES OF THE INCREMENTS ARE REVERSED.  THUS WE DEFIN 
 *               THE LENGTH OF THE TRANSFORMS, N, TO BE 9, THE 
 *               NUMBER OF TRANSFORMS, LOT, TO BE 64, THE INCREMENT 
 *               BETWEEN ELEMENTS OF EACH TRANSFORM, INC, TO BE 66, 
 *               AND THE INCREMENT BETWEEN THE STARTING POINTS 
 *               FOR EACH TRANSFORM, JUMP, TO BE 1 
 *
 *               CALL CFFT99( A, WORK, TRIGS2, IFAX2, 66, 1, 9, 64, -1) 
 *
 *               THESE TWO SEQUENTIAL STEPS RESULTS IN THE TWO-DIMENSIONA 
 *               FOURIER COEFFICIENT ARRAY OVERWRITING THE INPUT 
 *               GRIDPOINT ARRAY, A.  THE SAME TWO STEPS APPLIED AGAIN 
 *               WITH ISIGN = +1 WOULD RESULT IN THE RECONSTRUCTION OF 
 *               THE GRIDPOINT ARRAY (MULTIPLIED BY A FACTOR OF 64*9). 
 *
 *
 * ----------------------------------------------------------------------- 
 */

/*     function ^cfft99^ - MULTIPLE FAST COMPLEX FOURIER TRANSFORM 

 *     A IS THE ARRAY CONTAINING INPUT AND OUTPUT DATA 
 *     WORK IS AN AREA OF SIZE N*LOT 
 *     TRIGS IS A PREVIOUSLY PREPARED LIST OF TRIG FUNCTION VALUES 
 *     IFAX IS A PREVIOUSLY PREPARED LIST OF FACTORS OF N 
 *     INC IS THE INCREMENT WITHIN EACH DATA #VECTOR# 
 *         (E.G. INC=1 FOR CONSECUTIVELY STORED DATA) 
 *     JUMP IS THE INCREMENT BETWEEN THE START OF EACH DATA VECTOR 
 *     N IS THE LENGTH OF THE DATA VECTORS 
 *     LOT IS THE NUMBER OF DATA VECTORS 
 *     ISIGN = +1 FOR TRANSFORM FROM SPECTRAL TO GRIDPOINT 
 *           = -1 FOR TRANSFORM FROM GRIDPOINT TO SPECTRAL 
 */

/*  VECTORIZATION IS ACHIEVED ON CRAY BY DOING THE TRANSFORMS IN  PARALLEL. */

    /* Parameter adjustments */
  --work;
  --a;

    /* Function Body */
  nn = n + n;
  ink = inc + inc;
  jum = jump + jump;
  nfax = ifax[0];
  jnk = 2;
  jst = 2;
  if (isign >= 0) {
    goto L30;
  }

  /*     THE INNERMOST TEMPERTON ROUTINES HAVE NO FACILITY FOR THE */
  /*     FORWARD (ISIGN = -1) TRANSFORM.  THEREFORE, THE INPUT MUST BE */
  /*     REARRANGED AS FOLLOWS: */

/*     THE ORDER OF EACH INPUT VECTOR, */

/*     G(0), G(1), G(2), ... , G(N-2), G(N-1) */

/*     IS REVERSED (EXCLUDING G(0)) TO YIELD */

/*     G(0), G(N-1), G(N-2), ... , G(2), G(1). */

/*     WITHIN THE TRANSFORM, THE CORRESPONDING EXPONENTIAL MULTIPLIER */
/*     IS THEN PRECISELY THE CONJUGATE OF THAT FOR THE NORMAL */
/*     ORDERING.  THUS THE FORWARD (ISIGN = -1) TRANSFORM IS */
/*     ACCOMPLISHED */

/*     FOR NFAX ODD, THE INPUT MUST BE TRANSFERRED TO THE WORK ARRAY, */
/*     AND THE REARRANGEMENT CAN BE DONE DURING THE MOVE. */

  jnk = -2;
  jst = nn - 2;
  if (nfax % 2 == 1) {
    goto L40;
  }

  /*     FOR NFAX EVEN, THE REARRANGEMENT MUST BE APPLIED DIRECTLY TO */
  /*     THE INPUT ARRAY.  THIS CAN BE DONE BY SWAPPING ELEMENTS. */

  ibase = 1;
  ilast = (n - 1) * ink;
  nh = n / 2;
  for (l = 1; l <= lot; ++l) {
    i1 = ibase + ink;
    i2 = ibase + ilast;
    /* DIR$ IVDEP */
    for (m = 1; m <= nh; ++m) {
      /*     SWAP REAL AND IMAGINARY PORTIONS */
      hreal = a[i1];
      himag = a[i1 + 1];
      a[i1] = a[i2];
      a[i1 + 1] = a[i2 + 1];
      a[i2] = hreal;
      a[i2 + 1] = himag;
      i1 += ink;
      i2 -= ink;
      /* L10: */
    }
    ibase += jum;
    /* L20: */
  }
  goto L100;

 L30:
  if (nfax % 2 == 0) {
    goto L100;
  }

 L40:

/*     DURING THE TRANSFORM PROCESS, NFAX STEPS ARE TAKEN, AND THE */
/*     RESULTS ARE STORED ALTERNATELY IN WORK AND IN A.  IF NFAX IS */
/*     ODD, THE INPUT DATA ARE FIRST MOVED TO WORK SO THAT THE FINAL */
/*     RESULT (AFTER NFAX STEPS) IS STORED IN ARRAY A. */

  ibase = 1;
  jbase = 1;
  for (l = 1; l <= lot; ++l) {
    /*     MOVE REAL AND IMAGINARY PORTIONS OF ELEMENT ZERO */
    work[jbase] = a[ibase];
    work[jbase + 1] = a[ibase + 1];
    i = ibase + ink;
    j = jbase + jst;
    /* DIR$ IVDEP */
    for (m = 2; m <= n; ++m) {
      /*     MOVE REAL AND IMAGINARY PORTIONS OF OTHER ELEMENTS (POSSIBLY IN */
      /*     REVERSE ORDER, DEPENDING ON JST AND JNK) */
      work[j] = a[i];
      work[j + 1] = a[i + 1];
      i += ink;
      j += jnk;
      /* L50: */
    }
    ibase += jum;
    jbase += nn;
    /* L60: */
  }

 L100:

/*     PERFORM THE TRANSFORM PASSES, ONE PASS FOR EACH FACTOR.  DURING */
/*     EACH PASS THE DATA ARE MOVED FROM A TO WORK OR FROM WORK TO A. */

/*     FOR NFAX EVEN, THE FIRST PASS MOVES FROM A TO WORK */
  igo = 110;
  /*     FOR NFAX ODD, THE FIRST PASS MOVES FROM WORK TO A */
  if (nfax % 2 == 1) {
    igo = 120;
  }
  la = 1;
  for (k = 1; k <= nfax; ++k) {
    if (igo == 120) {
      goto L120;
    }
    /* L110: */
    vpassm(&a[1], &a[2], &work[1], &work[2], trigs, ink, 2, jum, nn, lot, n, 
           ifax[k], la);
    igo = 120;
    goto L130;
  L120:
    vpassm(&work[1], &work[2], &a[1], &a[2], trigs, 2, ink, nn,
           jum, lot, n, ifax[k], la);
    igo = 110;
  L130:
    la *= ifax[k];
    /* L140: */
  }

  /*     AT THIS POINT THE FINAL TRANSFORM RESULT IS STORED IN A. */

  return;
} /* cfft99 */

void cftfax(long n, long *ifax, double *trigs)
{
  /* Local variables */
  static long k;

/*     THIS ROUTINE WAS MODIFIED FROM TEMPERTON^S ORIGINAL */
/*     BY DAVE FULKER.  IT NO LONGER PRODUCES FACTORS IN ASCENDING */
/*     ORDER, AND THERE ARE NONE OF THE ORIGINAL #MODE# OPTIONS. */

/* ON INPUT     N */
/*               THE LENGTH OF EACH COMPLEX TRANSFORM TO BE PERFORMED */

/*               N MUST BE GREATER THAN 1 AND CONTAIN NO PRIME */
/*               FACTORS GREATER THAN 5. */

/* ON OUTPUT    IFAX */
/*               IFAX(1) */
/*                 THE NUMBER OF FACTORS CHOSEN OR -99 IN CASE OF ERROR */
/*               IFAX(2) THRU IFAX( IFAX(1)+1 ) */
/*                 THE FACTORS OF N IN THE FOLLOWIN ORDER:  APPEARING */
/*                 FIRST ARE AS MANY FACTORS OF 4 AS CAN BE OBTAINED. */
/*                 SUBSEQUENT FACTORS ARE PRIMES, AND APPEAR IN */
/*                 ASCENDING ORDER, EXCEPT FOR MULTIPLE FACTORS. */

/*              TRIGS */
/*               2N SIN AND COS VALUES FOR USE BY THE TRANSFORM ROUTINE */

    /* Function Body */
  fact(n, ifax);
  k = ifax[0];
  if (k < 1 || ifax[k] > 5) {
    ifax[0] = -99;
  }
  if (ifax[0] <= 0)
    return;
      
  cftrig(n, trigs);
  return;
} /* cftfax */

static void fact(long n, long *ifax)
{
  /* Local variables */
  static long k, l, nn, inc, max;

/*     FACTORIZATION ROUTINE THAT FIRST EXTRACTS ALL FACTORS OF 4 */
    /* Function Body */
  if (n > 1) {
    goto L10;
  }
  ifax[0] = 0;
  if (n < 1) {
    ifax[0] = -99;
  }
  return;
 L10:
  nn = n;
  k = 0;
  /*     TEST FOR FACTORS OF 4 */
 L20:
  if (nn % 4 != 0) {
    goto L30;
  }
  ++k;
  ifax[k] = 4;
  nn /= 4;
  if (nn == 1) {
    goto L80;
  }
  goto L20;
  /*     TEST FOR EXTRA FACTOR OF 2 */
 L30:
  if (nn % 2 != 0) {
    goto L40;
  }
  ++k;
  ifax[k] = 2;
  nn /= 2;
  if (nn == 1) {
    goto L80;
  }
  /*     TEST FOR FACTORS OF 3 */
 L40:
  if (nn % 3 != 0) {
    goto L50;
  }
  ++k;
  ifax[k] = 3;
  nn /= 3;
  if (nn == 1) {
    goto L80;
  }
  goto L40;
  /*     NOW FIND REMAINING FACTORS */
 L50:
  l = 5;
  max = sqrt((double) nn);
  inc = 2;
  /*     INC ALTERNATELY TAKES ON VALUES 2 AND 4 */
 L60:
  if (nn % l != 0) {
    goto L70;
  }
  ++k;
  ifax[k] = l;
  nn /= l;
  if (nn == 1) {
    goto L80;
  }
  goto L60;
 L70:
  if (l > max) {
    goto L75;
  }
  l += inc;
  inc = 6 - inc;
  goto L60;
 L75:
  ++k;
  ifax[k] = nn;
 L80:
  ifax[0] = k;
  /*     IFAX(0) NOW CONTAINS NUMBER OF FACTORS */
  return;
} /* fact */

static void cftrig(long n, double *trigs)
{
  /* Local variables */
  static long i, n2;
  static double angle, del;

  /* Function Body */
  del = (PI + PI) / (double) (n);
  n2 = n + n;
  for (i = 0; i < n2; i += 2) {
    angle = (double) i * 0.5 * del;
    trigs[i] = cos(angle);
    trigs[i + 1] = sin(angle);
  }
  return;
} /* cftrig */

static void vpassm(double *a, double *b, double *c, double *d, 
                   double *trigs, long inc1, long inc2, long inc3, long inc4, 
                   long lot, long n, long ifac, long la)
{
  /* Initialized data */

  static double sin36 = 0.587785252292473;
  static double cos36 = 0.809016994374947;
  static double sin72 = 0.951056516295154;
  static double cos72 = 0.309016994374947;
  static double sin60 = 0.866025403784437;

    /* Local variables */
  static long iink, jink, jump, i, j, k, l, m, ibase, jbase;
  static double c1, c2, c3, c4, s1, s2, s3, s4;
  static long ia, ja, ib, jb, kb, ic, jc, kc, id, jd, kd, ie, je, ke, 
    la1, ijk, igo;


/*     SUBROUTINE ^VPASSM^ - MULTIPLE VERSION OF ^VPASSA^ */
/*     PERFORMS ONE PASS THROUGH DATA */
/*     AS PART OF MULTIPLE COMPLEX (INVERSE) FFT ROUTINE */
/*     A IS FIRST REAL INPUT VECTOR */
/*     B IS FIRST IMAGINARY INPUT VECTOR */
/*     C IS FIRST REAL OUTPUT VECTOR */
/*     D IS FIRST IMAGINARY OUTPUT VECTOR */
/*     TRIGS IS PRECALCULATED TABLE OF SINES \ COSINES */
/*     INC1 IS ADDRESSING INCREMENT FOR A AND B */
/*     INC2 IS ADDRESSING INCREMENT FOR C AND D */
/*     INC3 IS ADDRESSING INCREMENT BETWEEN A^S \ B^S */
/*     INC4 IS ADDRESSING INCREMENT BETWEEN C^S \ D^S */
/*     LOT IS THE NUMBER OF VECTORS */
/*     N IS LENGTH OF VECTORS */
/*     IFAC IS CURRENT FACTOR OF N */
/*     LA IS PRODUCT OF PREVIOUS FACTORS */

    /* Parameter adjustments */
  --d;
  --c;
  --b;
  --a;

    /* Function Body */

  m = n / ifac;
  iink = m * inc1;
  jink = la * inc2;
  jump = (ifac - 1) * jink;
  ibase = 0;
  jbase = 0;
  igo = ifac - 1;
  if (igo > 4) {
    return;
  }
  switch ((int)igo) {
  case 1:  goto L10;
  case 2:  goto L50;
  case 3:  goto L90;
  case 4:  goto L130;
  }

  /*     CODING FOR FACTOR 2 */

 L10:
  ia = 1;
  ja = 1;
  ib = ia + iink;
  jb = ja + jink;
  for (l = 1; l <= la; ++l) {
    i = ibase;
    j = jbase;
    /* DIR$ IVDEP */
    for (ijk = 1; ijk <= lot; ++ijk) {
      c[ja + j] = a[ia + i] + a[ib + i];
      d[ja + j] = b[ia + i] + b[ib + i];
      c[jb + j] = a[ia + i] - a[ib + i];
      d[jb + j] = b[ia + i] - b[ib + i];
      i += inc3;
      j += inc4;
      /* L15: */
    }
    ibase += inc1;
    jbase += inc2;
    /* L20: */
  }
  if (la == m) {
    return;
  }
  la1 = la + 1;
  jbase += jump;
  for (k = la1; la < 0 ? k >= m : k <= m; k += la) {
    kb = k + k - 2;
    c1 = trigs[kb];
    s1 = trigs[kb + 1];
    for (l = 1; l <= la; ++l) {
      i = ibase;
      j = jbase;
      /* DIR$ IVDEP */
      for (ijk = 1; ijk <= lot; ++ijk) {
        c[ja + j] = a[ia + i] + a[ib + i];
        d[ja + j] = b[ia + i] + b[ib + i];
        c[jb + j] = c1 * (a[ia + i] - a[ib + i]) - s1 * 
          (b[ia +i] - b[ib + i]);
        d[jb + j] = s1 * (a[ia + i] - a[ib + i]) + c1 * 
          (b[ia +i] - b[ib + i]);
        i += inc3;
        j += inc4;
        /* L25: */
      }
      ibase += inc1;
      jbase += inc2;
      /* L30: */
    }
    jbase += jump;
    /* L40: */
  }
  return;

/*     CODING FOR FACTOR 3 */

 L50:
  ia = 1;
  ja = 1;
  ib = ia + iink;
  jb = ja + jink;
  ic = ib + iink;
  jc = jb + jink;
  for (l = 1; l <= la; ++l) {
    i = ibase;
    j = jbase;
    /* DIR$ IVDEP */
    for (ijk = 1; ijk <= lot; ++ijk) {
      c[ja + j] = a[ia + i] + (a[ib + i] + a[ic + i]);
      d[ja + j] = b[ia + i] + (b[ib + i] + b[ic + i]);
      c[jb + j] = a[ia + i] - (a[ib + i] + a[ic + i]) * 0.5 - 
        sin60 * (b[ib + i] - b[ic + i]);
      c[jc + j] = a[ia + i] - (a[ib + i] + a[ic + i]) * 0.5 + 
        sin60 * (b[ib + i] - b[ic + i]);
      d[jb + j] = b[ia + i] - (b[ib + i] + b[ic + i]) * 0.5 + 
        sin60 * (a[ib + i] - a[ic + i]);
      d[jc + j] = b[ia + i] - (b[ib + i] + b[ic + i]) * 0.5 - 
        sin60 * (a[ib + i] - a[ic + i]);
      i += inc3;
      j += inc4;
      /* L55: */
    }
    ibase += inc1;
    jbase += inc2;
    /* L60: */
  }
  if (la == m) {
    return;
  }
  la1 = la + 1;
  jbase += jump;
  for (k = la1; la < 0 ? k >= m : k <= m; k += la) {
    kb = k + k - 2;
    kc = kb + kb;
    c1 = trigs[kb];
    s1 = trigs[kb + 1];
    c2 = trigs[kc];
    s2 = trigs[kc + 1];
    for (l = 1; l <= la; ++l) {
      i = ibase;
      j = jbase;
      /* DIR$ IVDEP */
      for (ijk = 1; ijk <= lot; ++ijk) {
        c[ja + j] = a[ia + i] + (a[ib + i] + a[ic + i]);
        d[ja + j] = b[ia + i] + (b[ib + i] + b[ic + i]);
        c[jb + j] = c1 * (a[ia + i] - (a[ib + i] + a[ic + i]) * 0.5 - 
                          sin60 * (b[ib + i] - b[ic + i])) - 
          s1 * (b[ia + i] - (b[ib + i] + b[ic + i]) * 0.5 + 
                sin60 * (a[ib + i] - a[ic + i]));
        d[jb + j] = s1 * (a[ia + i] - (a[ib + i] + a[ic + i]) * 0.5 - 
                          sin60 * (b[ib + i] - b[ic + i])) + 
          c1 * (b[ia + i] - (b[ib + i] + b[ic + i]) * 0.5 + 
                sin60 * (a[ib + i] - a[ic + i]));
        c[jc + j] = c2 * (a[ia + i] - (a[ib + i] + a[ic + i]) * 0.5 + 
                          sin60 * (b[ib + i] - b[ic + i])) - 
          s2 * (b[ia + i] - (b[ib + i] + b[ic + i]) * 0.5 - 
                sin60 * (a[ib + i] - a[ic + i]));
        d[jc + j] = s2 * (a[ia + i] - (a[ib + i] + a[ic + i]) * 0.5 + 
                          sin60 * (b[ib + i] - b[ic + i])) + 
          c2 * (b[ia + i] - (b[ib + i] + b[ic + i]) * 0.5 - 
                sin60 * (a[ib + i] - a[ic + i]));
        i += inc3;
        j += inc4;
        /* L65: */
      }
      ibase += inc1;
      jbase += inc2;
      /* L70: */
    }
    jbase += jump;
    /* L80: */
  }
  return;

/*     CODING FOR FACTOR 4 */

 L90:
  ia = 1;
  ja = 1;
  ib = ia + iink;
  jb = ja + jink;
  ic = ib + iink;
  jc = jb + jink;
  id = ic + iink;
  jd = jc + jink;
  for (l = 1; l <= la; ++l) {
    i = ibase;
    j = jbase;
    /* DIR$ IVDEP */
    for (ijk = 1; ijk <= lot; ++ijk) {
      c[ja + j] = a[ia + i] + a[ic + i] + (a[ib + i] + a[id + i]);
      c[jc + j] = a[ia + i] + a[ic + i] - (a[ib + i] + a[id + i]);
      d[ja + j] = b[ia + i] + b[ic + i] + (b[ib + i] + b[id + i]);
      d[jc + j] = b[ia + i] + b[ic + i] - (b[ib + i] + b[id + i]);
      c[jb + j] = a[ia + i] - a[ic + i] - (b[ib + i] - b[id + i]);
      c[jd + j] = a[ia + i] - a[ic + i] + (b[ib + i] - b[id + i]);
      d[jb + j] = b[ia + i] - b[ic + i] + (a[ib + i] - a[id + i]);
      d[jd + j] = b[ia + i] - b[ic + i] - (a[ib + i] - a[id + i]);
      i += inc3;
      j += inc4;
      /* L95: */
    }
    ibase += inc1;
    jbase += inc2;
    /* L100: */
  }
  if (la == m) {
    return;
  }
  la1 = la + 1;
  jbase += jump;
  for (k = la1; la < 0 ? k >= m : k <= m; k += la) {
    kb = k + k - 2;
    kc = kb + kb;
    kd = kc + kb;
    c1 = trigs[kb];
    s1 = trigs[kb + 1];
    c2 = trigs[kc];
    s2 = trigs[kc + 1];
    c3 = trigs[kd];
    s3 = trigs[kd + 1];
    for (l = 1; l <= la; ++l) {
      i = ibase;
      j = jbase;
      /* DIR$ IVDEP */
      for (ijk = 1; ijk <= lot; ++ijk) {
        c[ja + j] = a[ia + i] + a[ic + i] + (a[ib + i] + a[id + i]);
        d[ja + j] = b[ia + i] + b[ic + i] + (b[ib + i] + b[id + i]);
        c[jc + j] = c2 * (a[ia + i] + a[ic + i] - (a[ib + i] + a[id + i])) - 
          s2 * (b[ia + i] + b[ic + i] - (b[ib + i] + b[id + i]));
        d[jc + j] = s2 * (a[ia + i] + a[ic + i] - (a[ib + i] + a[id + i])) + 
          c2 * (b[ia + i] + b[ic + i] - (b[ib + i] + b[id + i]));
        c[jb + j] = c1 * (a[ia + i] - a[ic + i] - (b[ib + i] - b[id + i])) - 
          s1 * (b[ia + i] - b[ic + i] + (a[ib + i] - a[id + i]));
        d[jb + j] = s1 * (a[ia + i] - a[ic + i] - (b[ib + i] - b[id + i])) + 
          c1 * (b[ia + i] - b[ic + i] + (a[ib + i] - a[id + i]));
        c[jd + j] = c3 * (a[ia + i] - a[ic + i] + (b[ib + i] - b[id + i])) - 
          s3 * (b[ia + i] - b[ic + i] - (a[ib + i] - a[id + i]));
        d[jd + j] = s3 * (a[ia + i] - a[ic + i] + (b[ib + i] - b[id + i])) + 
          c3 * (b[ia + i] - b[ic + i] - (a[ib + i] - a[id + i]));
        i += inc3;
        j += inc4;
        /* L105: */
      }
      ibase += inc1;
      jbase += inc2;
      /* L110: */
    }
    jbase += jump;
    /* L120: */
  }
  return;

/*     CODING FOR FACTOR 5 */

 L130:
  ia = 1;
  ja = 1;
  ib = ia + iink;
  jb = ja + jink;
  ic = ib + iink;
  jc = jb + jink;
  id = ic + iink;
  jd = jc + jink;
  ie = id + iink;
  je = jd + jink;
  for (l = 1; l <= la; ++l) {
    i = ibase;
    j = jbase;
    /* DIR$ IVDEP */
    for (ijk = 1; ijk <= lot; ++ijk) {
      c[ja + j] = a[ia + i] + (a[ib + i] + a[ie + i]) + 
        (a[ic + i] + a[id + i]);
      d[ja + j] = b[ia + i] + (b[ib + i] + b[ie + i]) + 
        (b[ic + i] + b[id + i]);
      c[jb + j] = a[ia + i] + cos72 * (a[ib + i] + a[ie + i]) - 
        cos36 * (a[ic + i] + a[id + i]) - (sin72 * (b[ib + i] - b[ie + i]) + 
                                           sin36 * (b[ic + i] - b[id + i]));
      c[je + j] = a[ia + i] + cos72 * (a[ib + i] + a[ie + i]) -
        cos36 * (a[ic + i] + a[id + i]) + (sin72 * (b[ib + i] - b[ie + i]) + 
                                           sin36 * (b[ic + i] - b[id + i]));
      d[jb + j] = b[ia + i] + cos72 * (b[ib + i] + b[ie + i]) - 
        cos36 * (b[ic + i] + b[id + i]) + (sin72 * (a[ib + i] - a[ie + i]) + 
                                           sin36 * (a[ic + i] - a[id + i]));
      d[je + j] = b[ia + i] + cos72 * (b[ib + i] + b[ie + i]) - 
        cos36 * (b[ic + i] + b[id + i]) - (sin72 * (a[ib + i] - a[ie + i]) + 
                                           sin36 * (a[ic + i] - a[id + i]));
      c[jc + j] = a[ia + i] - cos36 * (a[ib + i] + a[ie + i]) + 
        cos72 * (a[ic + i] + a[id + i]) - (sin36 * (b[ib + i] - b[ie + i]) -
                                           sin72 * (b[ic + i] - b[id + i]));
      c[jd + j] = a[ia + i] - cos36 * (a[ib + i] + a[ie + i]) + 
        cos72 * (a[ic + i] + a[id + i]) + (sin36 * (b[ib + i] - b[ie + i]) - 
                                           sin72 * (b[ic + i] - b[id + i]));
      d[jc + j] = b[ia + i] - cos36 * (b[ib + i] + b[ie + i]) + 
        cos72 * (b[ic + i] + b[id + i]) + (sin36 * (a[ib + i] - a[ie + i]) - 
                                           sin72 * (a[ic + i] - a[id + i]));
      d[jd + j] = b[ia + i] - cos36 * (b[ib + i] + b[ie + i]) + 
        cos72 * (b[ic + i] + b[id + i]) - (sin36 * (a[ib + i] - a[ie + i]) - 
                                           sin72 * (a[ic + i] - a[id + i]));
      i += inc3;
      j += inc4;
      /* L135: */
    }
    ibase += inc1;
    jbase += inc2;
    /* L140: */
  }
  if (la == m) {
    return;
  }
  la1 = la + 1;
  jbase += jump;
  for (k = la1; la < 0 ? k >= m : k <= m; k += la) {
    kb = k + k - 2;
    kc = kb + kb;
    kd = kc + kb;
    ke = kd + kb;
    c1 = trigs[kb];
    s1 = trigs[kb + 1];
    c2 = trigs[kc];
    s2 = trigs[kc + 1];
    c3 = trigs[kd];
    s3 = trigs[kd + 1];
    c4 = trigs[ke];
    s4 = trigs[ke + 1];
    for (l = 1; l <= la; ++l) {
      i = ibase;
      j = jbase;
      /* DIR$ IVDEP */
      for (ijk = 1; ijk <= lot; ++ijk) {
        c[ja + j] = a[ia + i] + (a[ib + i] + a[ie + i]) + 
          (a[ic + i] + a[id + i]);
        d[ja + j] = b[ia + i] + (b[ib + i] + b[ie + i]) + 
          (b[ic + i] + b[id + i]);
        c[jb + j] = 
          c1 * (a[ia + i] + cos72 * (a[ib + i] + a[ie + i]) - 
                cos36 * (a[ic + i] + a[id + i]) - 
                (sin72 * (b[ib + i] - b[ie + i]) + 
                 sin36 * (b[ic + i] - b[id + i]))) - 
          s1 * (b[ia + i] + cos72 * (b[ib + i] + b[ie + i]) - 
                cos36 * (b[ic + i] + b[id + i]) + 
                (sin72 * (a[ib + i] - a[ie + i]) + 
                 sin36 * (a[ic + i] - a[id + i])));
        d[jb + j] = 
          s1 * (a[ia + i] + cos72 * (a[ib + i] + a[ie + i]) - 
                cos36 * (a[ic + i] + a[id + i]) - 
                (sin72 * (b[ib + i] - b[ie + i]) + 
                 sin36 * (b[ic + i] - b[id + i]))) + 
          c1 * (b[ia + i] + cos72 * (b[ib + i] + b[ie + i]) - 
                cos36 * (b[ic + i] + b[id + i]) + 
                (sin72 * (a[ib + i] - a[ie + i]) +
                 sin36 * (a[ic + i] - a[id + i])));
        c[je + j] = 
          c4 * (a[ia + i] + cos72 * (a[ib + i] + a[ie + i]) - 
                cos36 * (a[ic + i] + a[id + i]) + 
                (sin72 * (b[ib + i] - b[ie + i]) + 
                 sin36 * (b[ic + i] - b[id + i]))) - 
          s4 * (b[ia + i] + cos72 * (b[ib + i] + b[ie + i]) - 
                cos36 * (b[ic + i] + b[id + i]) - 
                (sin72 * (a[ib + i] - a[ie + i]) + 
                 sin36 * (a[ic + i] - a[id + i])));
        d[je + j] = 
          s4 * (a[ia + i] + cos72 * (a[ib + i] + a[ie + i]) - 
                cos36 * (a[ic + i] + a[id + i]) + 
                (sin72 * (b[ib + i] - b[ie + i]) + 
                 sin36 * (b[ic + i] - b[id + i]))) + 
          c4 * (b[ia + i] + cos72 * (b[ib + i] + b[ie + i]) - 
                cos36 * (b[ic + i] + b[id + i]) - 
                (sin72 * (a[ib + i] - a[ie + i]) +
                 sin36 * (a[ic + i] - a[id + i])));
        c[jc + j] = 
          c2 * (a[ia + i] - cos36 * (a[ib + i] + a[ie + i]) + 
                cos72 * (a[ic + i] + a[id + i]) - 
                (sin36 * (b[ib + i] - b[ie + i]) - 
                 sin72 * (b[ic + i] - b[id + i]))) - 
          s2 * (b[ia + i] - cos36 * (b[ib + i] + b[ie + i]) + 
                cos72 * (b[ic + i] + b[id + i]) + 
                (sin36 * (a[ib + i] - a[ie + i]) -
                 sin72 * (a[ic + i] - a[id + i])));
        d[jc + j] = 
          s2 * (a[ia + i] - cos36 * (a[ib + i] + a[ie + i]) + 
                cos72 * (a[ic + i] + a[id + i]) - 
                (sin36 * (b[ib + i] - b[ie + i]) - 
                 sin72 * (b[ic + i] - b[id + i]))) + 
          c2 * (b[ia + i] - cos36 * (b[ib + i] + b[ie + i]) + 
                cos72 * (b[ic + i] + b[id + i]) + 
                (sin36 * (a[ib + i] - a[ie + i]) - 
                 sin72 * (a[ic + i] - a[id + i])));
        c[jd + j] = 
          c3 * (a[ia + i] - cos36 * (a[ib + i] + a[ie + i]) + 
                cos72 * (a[ic + i] + a[id + i]) + 
                (sin36 * (b[ib + i] - b[ie + i]) - 
                 sin72 * (b[ic +i] - b[id + i]))) - 
          s3 * (b[ia + i] - cos36 * (b[ib + i] + b[ie + i]) + 
                cos72 * (b[ic + i] + b[id + i]) - 
                (sin36 * (a[ib + i] - a[ie + i]) - 
                 sin72 * (a[ic + i] - a[id + i])));
        d[jd + j] = 
          s3 * (a[ia + i] - cos36 * (a[ib + i] + a[ie + i]) + 
                cos72 * (a[ic + i] + a[id + i]) + 
                (sin36 * (b[ib + i] - b[ie + i]) - 
                 sin72 * (b[ic + i] - b[id + i]))) + 
          c3 * (b[ia + i] - cos36 * (b[ib + i] + b[ie + i]) + 
                cos72 * (b[ic + i] + b[id + i]) - 
                (sin36 * (a[ib + i] - a[ie + i]) -
                 sin72 * (a[ic + i] - a[id + i])));
        i += inc3;
        j += inc4;
        /* L145: */
      }
      ibase += inc1;
      jbase += inc2;
      /* L150: */
    }
    jbase += jump;
    /* L160: */
  }
  return;
} /* vpassm */

void fft99(double *a, double *work, double *trigs, long *ifax, long inc, 
           long jump, long n, long lot, long isign)
{
  /* Local variables */
  static long nfax, i, j, k, l, m, ibase, jbase;
  static long ia, ib, la, nh, nx;
  static long igo, ink;


/* PURPOSE      PERFORMS MULTIPLE FAST FOURIER TRANSFORMS.  THIS PACKAGE 
 *              WILL PERFORM A NUMBER OF SIMULTANEOUS REAL/HALF-COMPLEX 
 *              PERIODIC FOURIER TRANSFORMS OR CORRESPONDING INVERSE 
 *              TRANSFORMS, I.E.  GIVEN A SET OF REAL DATA VECTORS, THE 
 *              PACKAGE RETURNS A SET OF #HALF-COMPLEX# FOURIER 
 *              COEFFICIENT VECTORS, OR VICE VERSA.  THE LENGTH OF THE 
 *              TRANSFORMS MUST BE AN EVEN NUMBER GREATER THAN 4 THAT HAS 
 *              NO OTHER FACTORS EXCEPT POSSIBLY POWERS OF 2, 3, AND 5. 
 *
 *              THE PACKAGE FFT99F CONTAINS SEVERAL USER-LEVEL ROUTINES: 
 *
 *             fftfax 
 *                AN INITIALIZATION ROUTINE THAT MUST BE CALLED ONCE 
 *                BEFORE A SEQUENCE OF CALLS TO THE FFT ROUTINES 
 *                (PROVIDED THAT N IS NOT CHANGED). 
 *
 *             fft99 AND fft991 
 *                TWO FFT ROUTINES THAT RETURN SLIGHTLY DIFFERENT 
 *                ARRANGEMENTS OF THE DATA IN GRIDPOINT SPACE. 
 *
 * USAGE        LET N BE OF THE FORM 2**P * 3**Q * 5**R, WHERE P .GE. 1, 
 *              Q .GE. 0, AND R .GE. 0.  THEN A TYPICAL SEQUENCE OF 
 *              CALLS TO TRANSFORM A GIVEN SET OF REAL VECTORS OF LENGTH 
 *              N TO A SET OF #HALF-COMPLEX# FOURIER COEFFICIENT VECTORS 
 *              OF LENGTH N IS 
 *
 *                   long ifax[13];
 *                   double trigs[3*n/2+1],a[m*(n+2)], work[m*(n+1)];
 *
 *                    fftfax (n, ifax, trigs);
 *                    fft99 (a,work,trigs,ifax,inc,jump,n,m,isign);
 *
 *              SEE THE INDIVIDUAL WRITE-UPS FOR FFTFAX, FFT99, AND 
 *              FFT991 BELOW, FOR A DETAILED DESCRIPTION OF THE 
 *              ARGUMENTS. 
 *
 * HISTORY      THE PACKAGE WAS WRITTEN BY CLIVE TEMPERTON AT ECMWF IN 
 *              NOVEMBER, 1978.  IT WAS MODIFIED, DOCUMENTED, AND TESTED 
 *              FOR NCAR BY RUSS REW IN SEPTEMBER, 1980. 
 *
 * ----------------------------------------------------------------------- 
 *
 *  fftfax (n,ifax,trigs) 
 *
 * PURPOSE      A SET-UP ROUTINE FOR FFT99 AND FFT991.  IT NEED ONLY BE 
 *              CALLED ONCE BEFORE A SEQUENCE OF CALLS TO THE FFT 
 *              ROUTINES (PROVIDED THAT N IS NOT CHANGED). 
 *
 * ARGUMENT     IFAX(13),TRIGS(3*N/2+1) 
 * DIMENSIONS 
 *
 * ARGUMENTS 
 *
 * ON INPUT     N 
 *               AN EVEN NUMBER GREATER THAN 4 THAT HAS NO PRIME FACTOR 
 *               GREATER THAN 5.  N IS THE LENGTH OF THE TRANSFORMS (SEE 
 *               THE DOCUMENTATION FOR FFT99 AND FFT991 FOR THE 
 *               DEFINITIONS OF THE TRANSFORMS). 
 *
 *              IFAX 
 *               AN INTEGER ARRAY.  THE NUMBER OF ELEMENTS ACTUALLY USED 
 *               WILL DEPEND ON THE FACTORIZATION OF N.  DIMENSIONING 
 *               IFAX FOR 13 SUFFICES FOR ALL N LESS THAN A MILLION. 
 *
 *              TRIGS 
 *               A FLOATING POINT ARRAY OF DIMENSION 3*N/2 IF N/2 IS 
 *               EVEN, OR 3*N/2+1 IF N/2 IS ODD. 
 *
 * ON OUTPUT    IFAX 
 *               CONTAINS THE FACTORIZATION OF N/2.  IFAX(1) IS THE 
 *               NUMBER OF FACTORS, AND THE FACTORS THEMSELVES ARE STORED 
 *               IN IFAX(2),IFAX(3),...  IF FFTFAX IS CALLED WITH N ODD, 
 *               OR IF N HAS ANY PRIME FACTORS GREATER THAN 5, IFAX(1) 
 *               IS SET TO -99. 
 *
 *              TRIGS 
 *               AN ARRAY OF TRIGNOMENTRIC FUNCTION VALUES SUBSEQUENTLY 
 *               USED BY THE FFT ROUTINES. 
 *
 * ----------------------------------------------------------------------- 
 *
 *  fft991 (a,work,trigs,ifax,inc,jump,n,m,isign) 
 *                       AND 
 *  fft99 (a,work,trigs,ifax,inc,jump,n,m,isign) 
 *
 * PURPOSE      PERFORM A NUMBER OF SIMULTANEOUS REAL/HALF-COMPLEX 
 *              PERIODIC FOURIER TRANSFORMS OR CORRESPONDING INVERSE 
 *              TRANSFORMS, USING ORDINARY SPATIAL ORDER OF GRIDPOINT 
 *              VALUES (FFT991) OR EXPLICIT CYCLIC CONTINUITY IN THE 
 *              GRIDPOINT VALUES (FFT99).  GIVEN A SET 
 *              OF REAL DATA VECTORS, THE PACKAGE RETURNS A SET OF 
 *              #HALF-COMPLEX# FOURIER COEFFICIENT VECTORS, OR VICE 
 *              VERSA.  THE LENGTH OF THE TRANSFORMS MUST BE AN EVEN 
 *              NUMBER THAT HAS NO OTHER FACTORS EXCEPT POSSIBLY POWERS 
 *              OF 2, 3, AND 5.  THESE VERSION OF FFT991 AND FFT99 ARE 
 *              OPTIMIZED FOR USE ON THE CRAY-1. 
 *
 * ARGUMENT     A(M*(N+2)), WORK(M*(N+1)), TRIGS(3*N/2+1), IFAX(13) 
 * DIMENSIONS 
 *
 * ARGUMENTS 
 *
 * ON INPUT     A 
 *               AN ARRAY OF LENGTH M*(N+2) CONTAINING THE INPUT DATA 
 *               OR COEFFICIENT VECTORS.  THIS ARRAY IS OVERWRITTEN BY 
 *               THE RESULTS. 
 *
 *              WORK 
 *               A WORK ARRAY OF DIMENSION M*(N+1) 
 *
 *              TRIGS 
 *               AN ARRAY SET UP BY FFTFAX, WHICH MUST BE CALLED FIRST. 
 *
 *              IFAX 
 *               AN ARRAY SET UP BY FFTFAX, WHICH MUST BE CALLED FIRST. 
 *
 *              INC 
 *               THE INCREMENT (IN WORDS) BETWEEN SUCCESSIVE ELEMENTS OF 
 *               EACH DATA OR COEFFICIENT VECTOR (E.G.  INC=1 FOR 
 *               CONSECUTIVELY STORED DATA). 
 *
 *              JUMP 
 *               THE INCREMENT (IN WORDS) BETWEEN THE FIRST ELEMENTS OF 
 *               SUCCESSIVE DATA OR COEFFICIENT VECTORS.  ON THE CRAY-1, 
 *               TRY TO ARRANGE DATA SO THAT JUMP IS NOT A MULTIPLE OF 8 
 *               (TO AVOID MEMORY BANK CONFLICTS).  FOR CLARIFICATION OF 
 *               INC AND JUMP, SEE THE EXAMPLES BELOW. 
 *
 *              N 
 *               THE LENGTH OF EACH TRANSFORM (SEE DEFINITION OF 
 *               TRANSFORMS, BELOW). 
 *
 *              M 
 *               THE NUMBER OF TRANSFORMS TO BE DONE SIMULTANEOUSLY. 
 *
 *              ISIGN 
 *               = +1 FOR A TRANSFORM FROM FOURIER COEFFICIENTS TO 
 *                    GRIDPOINT VALUES. 
 *               = -1 FOR A TRANSFORM FROM GRIDPOINT VALUES TO FOURIER 
 *                    COEFFICIENTS. 
 *
 * ON OUTPUT    A 
 *               IF ISIGN = +1, AND M COEFFICIENT VECTORS ARE SUPPLIED 
 *               EACH CONTAINING THE SEQUENCE: 
 *
 *               A(0),B(0),A(1),B(1),...,A(N/2),B(N/2)  (N+2 VALUES) 
 *
 *               THEN THE RESULT CONSISTS OF M DATA VECTORS EACH 
 *               CONTAINING THE CORRESPONDING N+2 GRIDPOINT VALUES: 
 *
 *               FOR FFT991, X(0), X(1), X(2),...,X(N-1),0,0. 
 *               FOR FFT99, X(N-1),X(0),X(1),X(2),...,X(N-1),X(0). 
 *                   (EXPLICIT CYCLIC CONTINUITY) 
 *
 *               WHEN ISIGN = +1, THE TRANSFORM IS DEFINED BY: 
 *                 X(J)=SUM(K=0,...,N-1)(C(K)*EXP(2*I*J*K*PI/N)) 
 *                 WHERE C(K)=A(K)+I*B(K) AND C(N-K)=A(K)-I*B(K) 
 *                 AND I=SQRT (-1) 
 *
 *               IF ISIGN = -1, AND M DATA VECTORS ARE SUPPLIED EACH 
 *               CONTAINING A SEQUENCE OF GRIDPOINT VALUES X(J) AS 
 *               DEFINED ABOVE, THEN THE RESULT CONSISTS OF M VECTORS 
 *               EACH CONTAINING THE CORRESPONDING FOURIER COFFICIENTS 
 *               A(K), B(K), 0 .LE. K .LE N/2. 
 *
 *               WHEN ISIGN = -1, THE INVERSE TRANSFORM IS DEFINED BY: 
 *                 C(K)=(1/N)*SUM(J=0,...,N-1)(X(J)*EXP(-2*I*J*K*PI/N)) 
 *                 WHERE C(K)=A(K)+I*B(K) AND I=SQRT(-1) 
 *
 *               A CALL WITH ISIGN=+1 FOLLOWED BY A CALL WITH ISIGN=-1 
 *               (OR VICE VERSA) RETURNS THE ORIGINAL DATA. 
 *
 *               NOTE: THE FACT THAT THE GRIDPOINT VALUES X(J) ARE REAL 
 *               IMPLIES THAT B(0)=B(N/2)=0.  FOR A CALL WITH ISIGN=+1, 
 *               IT IS NOT ACTUALLY NECESSARY TO SUPPLY THESE ZEROS. 
 *
 * EXAMPLES      GIVEN 19 DATA VECTORS EACH OF LENGTH 64 (+2 FOR EXPLICIT 
 *               CYCLIC CONTINUITY), COMPUTE THE CORRESPONDING VECTORS OF 
 *               FOURIER COEFFICIENTS.  THE DATA MAY, FOR EXAMPLE, BE 
 *               ARRANGED LIKE THIS: 
 *
 * FIRST DATA   A(1)=    . . .                A(66)=             A(70) 
 * VECTOR       X(63) X(0) X(1) X(2) ... X(63) X(0)  (4 EMPTY LOCATIONS) 
 *
 * SECOND DATA  A(71)=   . . .                                  A(140) 
 * VECTOR       X(63) X(0) X(1) X(2) ... X(63) X(0)  (4 EMPTY LOCATIONS) 
 *
 *               AND SO ON.  HERE INC=1, JUMP=70, N=64, M=19, ISIGN=-1, 
 *               AND FFT99 SHOULD BE USED (BECAUSE OF THE EXPLICIT CYCLIC 
 *               CONTINUITY). 
 *
 *               ALTERNATIVELY THE DATA MAY BE ARRANGED LIKE THIS: 
 *
 *                FIRST         SECOND                          LAST 
 *                DATA          DATA                            DATA 
 *                VECTOR        VECTOR                          VECTOR 
 *
 *                 A(1)=         A(2)=                           A(19)= 
 *
 *                 X(63)         X(63)       . . .               X(63) 
 *        A(20)=   X(0)          X(0)        . . .               X(0) 
 *        A(39)=   X(1)          X(1)        . . .               X(1) 
 *                  .             .                               . 
 *                  .             .                               . 
 *                  .             .                               . 
 *
 *               IN WHICH CASE WE HAVE INC=19, JUMP=1, AND THE REMAINING 
 *               PARAMETERS ARE THE SAME AS BEFORE.  IN EITHER CASE, EACH 
 *               COEFFICIENT VECTOR OVERWRITES THE CORRESPONDING INPUT 
 *               DATA VECTOR. 
 *
 * ----------------------------------------------------------------------- 
 *
 *     SUBROUTINE ^FFT99^ - MULTIPLE FAST REAL PERIODIC TRANSFORM 
 *     CORRESPONDING TO OLD SCALAR ROUTINE FFT9 
 *     PROCEDURE USED TO CONVERT TO HALF-LENGTH COMPLEX TRANSFORM 
 *     IS GIVEN BY COOLEY, LEWIS AND WELCH (J. SOUND VIB., VOL. 12 
 *     (1970), 315-337) 
 *
 *     A IS THE ARRAY CONTAINING INPUT AND OUTPUT DATA 
 *     WORK IS AN AREA OF SIZE (N+1)*LOT 
 *     TRIGS IS A PREVIOUSLY PREPARED LIST OF TRIG FUNCTION VALUES 
 *     IFAX IS A PREVIOUSLY PREPARED LIST OF FACTORS OF N/2 
 *     INC IS THE INCREMENT WITHIN EACH DATA #VECTOR# 
 *         (E.G. INC=1 FOR CONSECUTIVELY STORED DATA) 
 *     JUMP IS THE INCREMENT BETWEEN THE START OF EACH DATA VECTOR 
 *     N IS THE LENGTH OF THE DATA VECTORS 
 *     LOT IS THE NUMBER OF DATA VECTORS 
 *     ISIGN = +1 FOR TRANSFORM FROM SPECTRAL TO GRIDPOINT 
 *           = -1 FOR TRANSFORM FROM GRIDPOINT TO SPECTRAL 
 *
 *     ORDERING OF COEFFICIENTS: 
 *         A(0),B(0),A(1),B(1),A(2),B(2),...,A(N/2),B(N/2) 
 *         WHERE B(0)=B(N/2)=0; (N+2) LOCATIONS REQUIRED 
 *
 *     ORDERING OF DATA: 
 *         X(N-1),X(0),X(1),X(2),...,X(N),X(0) 
 *         I.E. EXPLICIT CYCLIC CONTINUITY; (N+2) LOCATIONS REQUIRED 
 *
 *     VECTORIZATION IS ACHIEVED ON CRAY BY DOING THE TRANSFORMS IN 
 *     PARALLEL 
 *
 *     *** N.B. N IS ASSUMED TO BE AN EVEN NUMBER 
 *
 *     DEFINITION OF TRANSFORMS: 
 *     ------------------------- 
 *
 *     ISIGN=+1: X(J)=SUM(K=0,...,N-1)(C(K)*EXP(2*I*J*K*PI/N)) 
 *         WHERE C(K)=A(K)+I*B(K) AND C(N-K)=A(K)-I*B(K) 
 *
 *     ISIGN=-1: A(K)=(1/N)*SUM(J=0,...,N-1)(X(J)*COS(2*J*K*PI/N)) 
 *               B(K)=-(1/N)*SUM(J=0,...,N-1)(X(J)*SIN(2*J*K*PI/N)) 
 */

    /* Parameter adjustments */
  --work;
  --a;

    /* Function Body */
  nfax = ifax[0];
  nx = n + 1;
  nh = n / 2;
  ink = inc + inc;
  if (isign == 1) {
    goto L30;
  }

  /*     IF NECESSARY, TRANSFER DATA TO WORK AREA */
  igo = 50;
  if (nfax % 2 == 1) {
    goto L40;
  }
  ibase = inc + 1;
  jbase = 1;
  for (l = 1; l <= lot; ++l) {
    i = ibase;
    j = jbase;
    /* DIR$ IVDEP */
    for (m = 1; m <= n; ++m) {
      work[j] = a[i];
      i += inc;
      ++j;
      /* L10: */
    }
    ibase += jump;
    jbase += nx;
    /* L20: */
  }

  igo = 60;
  goto L40;

  /*     PREPROCESSING (ISIGN=+1) */
  /*     ------------------------ */

 L30:
  fft99a(&a[1], &work[1], trigs, inc, jump, n, lot);
  igo = 60;

/*     COMPLEX TRANSFORM */
/*     ----------------- */

 L40:
  ia = inc + 1;
  la = 1;
  for (k = 1; k <= nfax; ++k) {
    if (igo == 60) {
      goto L60;
    }
    /* L50: */
    vpassm(&a[ia], &a[ia + inc], &work[1], &work[2], trigs, ink, 2, jump, nx, 
           lot, nh, ifax[k], la);
    igo = 60;
    goto L70;
  L60:
    vpassm(&work[1], &work[2], &a[ia], &a[ia + inc], trigs, 2, ink, nx, jump, 
           lot, nh, ifax[k], la);
    igo = 50;
  L70:
    la *= ifax[k];
    /* L80: */
  }

  if (isign == -1) {
    goto L130;
  }

  /*     IF NECESSARY, TRANSFER DATA FROM WORK AREA */
  if (nfax % 2 == 1) {
    goto L110;
  }
  ibase = 1;
  jbase = ia;
  for (l = 1; l <= lot; ++l) {
    i = ibase;
    j = jbase;
    /* DIR$ IVDEP */
    for (m = 1; m <= n; ++m) {
      a[j] = work[i];
      ++i;
      j += inc;
      /* L90: */
    }
    ibase += nx;
    jbase += jump;
    /* L100: */
  }

  /*     FILL IN CYCLIC BOUNDARY POINTS */
 L110:
  ia = 1;
  ib = n * inc + 1;
  /* DIR$ IVDEP */
  for (l = 1; l <= lot; ++l) {
    a[ia] = a[ib];
    a[ib + inc] = a[ia + inc];
    ia += jump;
    ib += jump;
    /* L120: */
  }
  goto L140;

/*     POSTPROCESSING (ISIGN=-1): */
/*     -------------------------- */

 L130:
  fft99b(&work[1], &a[1], trigs, inc, jump, n, lot);

 L140:
  return;
} /* fft99 */

static void fft99a(double *a, double *work, double *trigs, long inc, long jump,
                   long n, long lot)
{
  /* Local variables */
  static double c;
  static long k, l;
  static double s;
  static long ia, ib, ja, jb, iabase, ibbase, nh, jabase, jbbase, nx, 
    ink;


/*     SUBROUTINE FFT99A - PREPROCESSING STEP FOR FFT99, ISIGN=+1 */
/*     (SPECTRAL TO GRIDPOINT TRANSFORM) */

    /* Parameter adjustments */
  --work;
  --a;

    /* Function Body */
  nh = n / 2;
  nx = n + 1;
  ink = inc + inc;

  /*     A(0) AND A(N/2) */
  ia = 1;
  ib = n * inc + 1;
  ja = 1;
  jb = 2;
  /* DIR$ IVDEP */
  for (l = 1; l <= lot; ++l) {
    work[ja] = a[ia] + a[ib];
    work[jb] = a[ia] - a[ib];
    ia += jump;
    ib += jump;
    ja += nx;
    jb += nx;
    /* L10: */
  }

  /*     REMAINING WAVENUMBERS */
  iabase = (inc << 1) + 1;
  ibbase = (n - 2) * inc + 1;
  jabase = 3;
  jbbase = n - 1;

  for (k = 3; k <= nh; k += 2) {
    ia = iabase;
    ib = ibbase;
    ja = jabase;
    jb = jbbase;
    c = trigs[n + k - 1];
    s = trigs[n + k];
    /* DIR$ IVDEP */
    for (l = 1; l <= lot; ++l) {
      work[ja] = a[ia] + a[ib] - (s * (a[ia] - a[ib]) + 
                                  c * (a[ia + inc] + a[ib + inc]));
      work[jb] = a[ia] + a[ib] + (s * (a[ia] - a[ib]) + 
                                  c * (a[ia + inc] + a[ib + inc]));
      work[ja + 1] = c * (a[ia] - a[ib]) - 
        s * (a[ia + inc] + a[ib + inc]) + (a[ia + inc] - a[ib + inc]);
      work[jb + 1] = c * (a[ia] - a[ib]) - 
        s * (a[ia + inc] + a[ib + inc]) - (a[ia + inc] - a[ib + inc]);
      ia += jump;
      ib += jump;
      ja += nx;
      jb += nx;
      /* L20: */
    }
    iabase += ink;
    ibbase -= ink;
    jabase += 2;
    jbbase += -2;
    /* L30: */
  }

  if (iabase != ibbase) {
    goto L50;
  }
  /*     WAVENUMBER N/4 (IF IT EXISTS) */
  ia = iabase;
  ja = jabase;
  /* DIR$ IVDEP */
  for (l = 1; l <= lot; ++l) {
    work[ja] = a[ia] * 2.0;
    work[ja + 1] = a[ia + inc] * -2.0;
    ia += jump;
    ja += nx;
    /* L40: */
  }

 L50:
  return;
} /* fft99a */

static void fft99b(double *work, double *a, double *trigs, long inc, long jump,
                   long n, long lot)
{
  /* Local variables */
  static double c;
  static long k, l;
  static double s, scale;
  static long ia, ib, ja, jb, iabase, ibbase, nh, jabase, jbbase, nx, 
    ink;


/*     SUBROUTINE FFT99B - POSTPROCESSING STEP FOR FFT99, ISIGN=-1 */
/*     (GRIDPOINT TO SPECTRAL TRANSFORM) */

    /* Parameter adjustments */
  --a;
  --work;

    /* Function Body */
  nh = n / 2;
  nx = n + 1;
  ink = inc + inc;

  /*     A(0) AND A(N/2) */
  scale = 1.0 / (double) n;
  ia = 1;
  ib = 2;
  ja = 1;
  jb = n * inc + 1;
  /* DIR$ IVDEP */
  for (l = 1; l <= lot; ++l) {
    a[ja] = scale * (work[ia] + work[ib]);
    a[jb] = scale * (work[ia] - work[ib]);
    a[ja + inc] = 0.0;
    a[jb + inc] = 0.0;
    ia += nx;
    ib += nx;
    ja += jump;
    jb += jump;
    /* L10: */
  }

  /*     REMAINING WAVENUMBERS */
  scale *= 0.5;
  iabase = 3;
  ibbase = n - 1;
  jabase = (inc << 1) + 1;
  jbbase = (n - 2) * inc + 1;

  for (k = 3; k <= nh; k += 2) {
    ia = iabase;
    ib = ibbase;
    ja = jabase;
    jb = jbbase;
    c = trigs[n + k - 1];
    s = trigs[n + k];
    /* DIR$ IVDEP */
    for (l = 1; l <= lot; ++l) {
      a[ja] = scale * (work[ia] + work[ib] + 
                       (c * (work[ia + 1] + work[ib + 1]) + 
                        s * (work[ia] - work[ib])));
      a[jb] = scale * (work[ia] + work[ib] - 
                       (c * (work[ia + 1] + work[ib + 1]) + 
                        s * (work[ia] - work[ib])));
      a[ja + inc] = scale * (c * (work[ia] - work[ib]) - 
                             s * (work[ia + 1] + work[ib + 1]) + 
                             (work[ib + 1] - work[ia + 1]));
      a[jb + inc] = scale * (c * (work[ia] - work[ib]) - 
                             s * (work[ia + 1] + work[ib + 1]) - 
                             (work[ib + 1] - work[ia + 1]));
      ia += nx;
      ib += nx;
      ja += jump;
      jb += jump;
      /* L20: */
    }
    iabase += 2;
    ibbase += -2;
    jabase += ink;
    jbbase -= ink;
    /* L30: */
  }

  if (iabase != ibbase) {
    goto L50;
  }
  /*     WAVENUMBER N/4 (IF IT EXISTS) */
  ia = iabase;
  ja = jabase;
  scale *= 2.0;
  /* DIR$ IVDEP */
  for (l = 1; l <= lot; ++l) {
    a[ja] = scale * work[ia];
    a[ja + inc] = -scale * work[ia + 1];
    ia += nx;
    ja += jump;
    /* L40: */
  }

 L50:
  return;
} /* fft99b */

void fft991(double *a, double *work, double *trigs, long *ifax, long inc, 
            long jump, long n, long lot, long isign)
{
  /* Local variables */
  static long nfax, i, j, k, l, m, ibase, jbase;
  static long ia, ib, la, nh, nx;
  static long igo, ink;


/*     SUBROUTINE ^FFT991^ - MULTIPLE REAL/HALF-COMPLEX PERIODIC */
/*     FAST FOURIER TRANSFORM */

/*     SAME AS FFT99 EXCEPT THAT ORDERING OF DATA CORRESPONDS TO */
/*     THAT IN MRFFT2 */

/*     PROCEDURE USED TO CONVERT TO HALF-LENGTH COMPLEX TRANSFORM */
/*     IS GIVEN BY COOLEY, LEWIS AND WELCH (J. SOUND VIB., VOL. 12 */
/*     (1970), 315-337) */

/*     A IS THE ARRAY CONTAINING INPUT AND OUTPUT DATA */
/*     WORK IS AN AREA OF SIZE (N+1)*LOT */
/*     TRIGS IS A PREVIOUSLY PREPARED LIST OF TRIG FUNCTION VALUES */
/*     IFAX IS A PREVIOUSLY PREPARED LIST OF FACTORS OF N/2 */
/*     INC IS THE INCREMENT WITHIN EACH DATA #VECTOR# */
/*         (E.G. INC=1 FOR CONSECUTIVELY STORED DATA) */
/*     JUMP IS THE INCREMENT BETWEEN THE START OF EACH DATA VECTOR */
/*     N IS THE LENGTH OF THE DATA VECTORS */
/*     LOT IS THE NUMBER OF DATA VECTORS */
/*     ISIGN = +1 FOR TRANSFORM FROM SPECTRAL TO GRIDPOINT */
/*           = -1 FOR TRANSFORM FROM GRIDPOINT TO SPECTRAL */

/*     ORDERING OF COEFFICIENTS: */
/*         A(0),B(0),A(1),B(1),A(2),B(2),...,A(N/2),B(N/2) */
/*         WHERE B(0)=B(N/2)=0; (N+2) LOCATIONS REQUIRED */

/*     ORDERING OF DATA: */
/*         X(0),X(1),X(2),...,X(N-1) */

/*     VECTORIZATION IS ACHIEVED ON CRAY BY DOING THE TRANSFORMS IN */
/*     PARALLEL */

/*     *** N.B. N IS ASSUMED TO BE AN EVEN NUMBER */

/*     DEFINITION OF TRANSFORMS: */
/*     ------------------------- */

/*     ISIGN=+1: X(J)=SUM(K=0,...,N-1)(C(K)*EXP(2*I*J*K*PI/N)) */
/*         WHERE C(K)=A(K)+I*B(K) AND C(N-K)=A(K)-I*B(K) */

/*     ISIGN=-1: A(K)=(1/N)*SUM(J=0,...,N-1)(X(J)*COS(2*J*K*PI/N)) */
/*               B(K)=-(1/N)*SUM(J=0,...,N-1)(X(J)*SIN(2*J*K*PI/N)) */

    /* Parameter adjustments */
  --work;
  --a;

    /* Function Body */
  nfax = ifax[0];
  nx = n + 1;
  nh = n / 2;
  ink = inc + inc;
  if (isign == 1) {
    goto L30;
  }

  /*     IF NECESSARY, TRANSFER DATA TO WORK AREA */
  igo = 50;
  if (nfax % 2 == 1) {
    goto L40;
  }
  ibase = 1;
  jbase = 1;
  for (l = 1; l <= lot; ++l) {
    i = ibase;
    j = jbase;
    /* DIR$ IVDEP */
    for (m = 1; m <= n; ++m) {
      work[j] = a[i];
      i += inc;
      ++j;
      /* L10: */
    }
    ibase += jump;
    jbase += nx;
    /* L20: */
  }

  igo = 60;
  goto L40;

  /*     PREPROCESSING (ISIGN=+1) */
  /*     ------------------------ */

 L30:
  fft99a(&a[1], &work[1], trigs, inc, jump, n, lot);
  igo = 60;

/*     COMPLEX TRANSFORM */
/*     ----------------- */

 L40:
  ia = 1;
  la = 1;
  for (k = 1; k <= nfax; ++k) {
    if (igo == 60) {
      goto L60;
    }
    /* L50: */
    vpassm(&a[ia], &a[ia + inc], &work[1], &work[2], trigs, ink, 2, jump, nx, 
           lot, nh, ifax[k], la);
    igo = 60;
    goto L70;
  L60:
    vpassm(&work[1], &work[2], &a[ia], &a[ia + inc], trigs, 2, ink, nx, jump, 
           lot, nh, ifax[k], la);
    igo = 50;
  L70:
    la *= ifax[k];
    /* L80: */
  }

  if (isign == -1) {
    goto L130;
  }

  /*     IF NECESSARY, TRANSFER DATA FROM WORK AREA */
  if (nfax % 2 == 1) {
    goto L110;
  }
  ibase = 1;
  jbase = 1;
  for (l = 1; l <= lot; ++l) {
    i = ibase;
    j = jbase;
    /* DIR$ IVDEP */
    for (m = 1; m <= n; ++m) {
      a[j] = work[i];
      ++i;
      j += inc;
      /* L90: */
    }
    ibase += nx;
    jbase += jump;
    /* L100: */
  }

  /*     FILL IN ZEROS AT END */
 L110:
  ib = n * inc + 1;
  /* DIR$ IVDEP */
  for (l = 1; l <= lot; ++l) {
    a[ib] = 0.0;
    a[ib + inc] = 0.0;
    ib += jump;
    /* L120: */
  }
  goto L140;

/*     POSTPROCESSING (ISIGN=-1): */
/*     -------------------------- */

 L130:
  fft99b(&work[1], &a[1], trigs, inc, jump, n, lot);

 L140:
  return;
} /* fft991 */

void fftfax(long n, long *ifax, double *trigs)
{
    /* Local variables */
  static long i;

  /* Function Body */
  fax(ifax, n);
  i = ifax[0];
  if (ifax[i] > 5 || n <= 4) {
    ifax[0] = -99;
  }
  if (ifax[0] <= 0) 
    return;
    
  fftrig(trigs, n);
  return;
} /* fftfax */

static void fax(long *ifax, long n)
{
  /* Local variables */
  static long nfax, item, i, k, l, istop, ii, nn, inc;

    /* Function Body */
  nn = n / 2;
  if (nn + nn == n) {
    goto L10;
  }
  ifax[0] = -99;
  return;
 L10:
  k = 0;
  /*     TEST FOR FACTORS OF 4 */
 L20:
  if (nn % 4 != 0) {
    goto L30;
  }
  ++k;
  ifax[k] = 4;
  nn /= 4;
  if (nn == 1) {
    goto L80;
  }
  goto L20;
  /*     TEST FOR EXTRA FACTOR OF 2 */
 L30:
  if (nn % 2 != 0) {
    goto L40;
  }
  ++k;
  ifax[k] = 2;
  nn /= 2;
  if (nn == 1) {
    goto L80;
  }
  /*     TEST FOR FACTORS OF 3 */
 L40:
  if (nn % 3 != 0) {
    goto L50;
  }
  ++k;
  ifax[k] = 3;
  nn /= 3;
  if (nn == 1) {
    goto L80;
  }
  goto L40;
  /*     NOW FIND REMAINING FACTORS */
 L50:
  l = 5;
  inc = 2;
  /*     INC ALTERNATELY TAKES ON VALUES 2 AND 4 */
 L60:
  if (nn % l != 0) {
    goto L70;
  }
  ++k;
  ifax[k] = l;
  nn /= l;
  if (nn == 1) {
    goto L80;
  }
  goto L60;
 L70:
  l += inc;
  inc = 6 - inc;
  goto L60;
 L80:
  ifax[0] = k;
  /*     IFAX(0) CONTAINS NUMBER OF FACTORS */
  nfax = ifax[0];
  /*     SORT FACTORS INTO ASCENDING ORDER */
  if (nfax == 1) 
    return;
  
  for (ii = 2; ii <= nfax; ++ii) {
    istop = nfax + 2 - ii;
    for (i = 2; i <= istop; ++i) {
      if (ifax[i] < ifax[i - 1]) {
        item = ifax[i - 1];
        ifax[i - 1] = ifax[i];
        ifax[i] = item;
      }
    }
  }
  return;
} /* fax */

static void fftrig(double *trigs, long n)
{
  /* Local variables */
  static long i, n2;
  static double angle;
  static long la, nh;
  static long nn;
  static double del;

    /* Function Body */
  nn = n / 2;
  del = (PI + PI) / (double) nn;
  n2 = nn + nn;
  for (i = 0; i < n2; i += 2) {
    angle = (double) i * 0.5 * del;
    trigs[i] = cos(angle);
    trigs[i + 1] = sin(angle);
  }
  del *= 0.5;
  nh = (nn + 1) / 2;
  n2 = nh + nh;
  la = nn + nn;
  for (i = 0; i < n2; i += 2) {
    angle = (double) i * 0.5 * del;
    trigs[la + i] = cos(angle);
    trigs[la + i + 1] = sin(angle);
  }

  return;
} /* fftrig */

