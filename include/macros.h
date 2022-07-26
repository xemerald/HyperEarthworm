/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: macros.h 2068 2006-01-30 19:41:11Z friberg $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2006/01/30 19:41:10  friberg
 *     added in hydra includes for raypicker module
 *
 *     Revision 1.1.1.1  2005/06/22 19:30:36  michelle
 *     new directory tree built from files in HYDRA_NEWDIR_2005-06-20 tagged hydra and earthworm projects
 *
 *     Revision 1.2  2005/03/29 15:43:50  michelle
 *     added #define SIGN per carol
 *
 *     Revision 1.1  2004/04/21 20:13:51  cjbryan
 *     *** empty log message ***
 *
 *
 *
 */
/*
 * A couple of useful macros.
 * 
 */

#ifndef MACROS_H
#define MACROS_H

/* returns the maximum of two values */
#define MAX(a,b) ((a) < (b) ? (b) : (a))

/* returns the minimum of two values */
#define MIN(a,b) ((a) < (b) ? (a) : (b))

/* signature */
#define SIGN(a, b) (((b) < 0) ? -fabs(a) : fabs(a))

#endif /* MACROS_H  */
