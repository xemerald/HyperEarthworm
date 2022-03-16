
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: gdfontt.h 15 2000-02-14 20:06:34Z lucky $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2000/02/14 20:05:54  lucky
 *     Initial revision
 *
 *
 */

#ifndef GDFONTT_H
#define GDFONTT_H 1

/* gdfontt.h: brings in the tinyest of the provided fonts.
	Also link with gdfontt.c. */

#include "gd.h"

/* 5x8  font derived from a public domain font in the X
        distribution. Contains the 127 standard ascii characters. */

extern gdFontPtr gdFontTiny;

#endif
