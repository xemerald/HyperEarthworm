#pragma once

/* Some generally useful functions */
#ifndef MIN
# define MIN(a,b) ((a) <= (b) ? (a) : (b))
#endif
# ifndef MAX
#define MAX(a,b) ((a) >= (b) ? (a) : (b))
#endif
#ifndef ABS
# define ABS(x) ((x) >= (0) ? (x) : -(x))
#endif
#ifndef ROUND
# define ROUND(x)	((int)((x) + ((x) < 0 ? -.5 : .5)))
#endif 
#ifndef TRUE
# define TRUE (1)
#endif
#ifndef FALSE
# define FALSE (0)
#endif

	/* flags for type of characteristic funcion */
#define SELF		0	/* the function itself */
#define ABSVAL		1	/* absolute value */
#define	ENVELOPE	2	/* envelope function */

extern int piki(int *, int *, int *, void *, char, int, int, int, int);
extern void fbrat(float *, float *, int, int, int, int );
extern void do_abs(float *, int);
extern void do_env(float *, int);

