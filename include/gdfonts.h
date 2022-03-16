
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: gdfonts.h 15 2000-02-14 20:06:34Z lucky $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2000/02/14 20:05:54  lucky
 *     Initial revision
 *
 *
 */

#ifndef GDFONTS_H
#define GDFONTS_H 1

/* gdfonts.h: brings in the smaller of the two provided fonts.
	Also link with gdfonts.c. */

#include "gd.h"

/* 6x12 font derived from a public domain font in the X
        distribution. Only contains the 96 standard ascii characters,
        sorry. Feel free to improve on this. */

extern gdFontPtr gdFontSmall;

#endif
