/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: residue.h 2093 2006-03-09 21:24:06Z luetgert $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.2  2006/03/09 21:24:06  luetgert
 *     .
 *
 *     Revision 1.1  2006/01/30 19:41:10  friberg
 *     added in hydra includes for raypicker module
 *
 *     Revision 1.1.1.1  2005/06/22 19:30:37  michelle
 *     new directory tree built from files in HYDRA_NEWDIR_2005-06-20 tagged hydra and earthworm projects
 *
 *     Revision 1.1  2004/04/22 14:28:52  patton
 *     Missing .h file that was preventing libsrc from compiling.
 *
 *
 *
 */
 
#ifndef RESIDUE_H
#define RESIDUE_H

#include <complex_math.h>
#include <transferFunction.h>

Complex residue(TransferFnStruct tf, const Complex s, const int i);

#endif /*  RESIDUE_H  */
