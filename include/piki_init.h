/* Detailed picker parameters follow */
#define FWIN 75		/* Forward window length */
#define BWIN 75		/* Backward window length */
#define PIKTHR 5.0  /* f/b ratio curve must exceed for valid 0-wt pick*/
#define PIK9THR 2.0  /* f/b ratio curve must exceed for valid 9-wt pick*/
#define OFFTHR 1.5  /* f/b ratio curve must go below or no pick */
#define MINPROD 1.0	/* f/b ratio curve min*max must be greater or no pick */
#define PICKCORR FALSE	/* Apply pick correction if TRUE */
#if 0
#define PIKTHR 1.8  /* f/b ratio curve must exceed for valid 0-wt pick*/
#define PIK9THR 1.5  /* f/b ratio curve must exceed for valid 9-wt pick*/
#define OFFTHR 1.5  /* f/b ratio curve must go below or no pick */
#define PIKTHR 3.0  /* f/b ratio curve must exceed for valid pick*/
#define OFFTHR 2.0  /* f/b ratio curve must go below or no pick */
#define PICKCORR FALSE	/* Apply pick correction if TRUE */
#endif
#define RNG 150		/* Look within this many points of target */
#define GTCOR_EXP -.36 /* Correction parameter from Eric Crosson */
#define GTERR_EXP -.75 /* Correction parameter from Eric Crosson */
#define SEIS_SCALE 10.	/* Arbitrary scaling for integerizing seismogram */
