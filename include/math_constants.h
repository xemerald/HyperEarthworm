/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: math_constants.h 4473 2011-08-04 15:12:34Z kevin $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.2  2006/03/09 21:24:06  luetgert
 *     .
 *
 *     Revision 1.1  2006/01/30 19:41:10  friberg
 *     added in hydra includes for raypicker module
 *
 *     Revision 1.2  2005/10/21 22:56:29  cjbryan
 *     added definition of FOUR_PI
 *
 *     Revision 1.1.1.1  2005/06/22 19:30:37  michelle
 *     new directory tree built from files in HYDRA_NEWDIR_2005-06-20 tagged hydra and earthworm projects
 *
 *     Revision 1.2  2005/01/04 16:15:50  cjbryan
 *     added DEG2RAd and RAD2DEG
 *
 *     Revision 1.1  2004/04/21 20:15:12  cjbryan
 *     *** empty log message ***
 *
 *
 *
 */

/* Useful constants */

#ifndef MATH_CONSTANTS_H
#define MATH_CONSTANTS_H

/* Constants rounded for 21 decimals. */
#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif
#define HALF_PI     (M_PI / 2.0)
#define TWO_PI      (M_PI * 2.0)
#define FOUR_PI     (M_PI * 4.0)
#define EIGHT_PI    (M_PI * 8.0)

#define PI           M_PI
#define DEG2RAD      PI/180.
#define RAD2DEG      180./PI

#endif  /*  MATH_CONSTANTS_H  */
