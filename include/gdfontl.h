
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: gdfontl.h 15 2000-02-14 20:06:34Z lucky $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2000/02/14 20:05:54  lucky
 *     Initial revision
 *
 *
 */

#ifndef GDFONTL_H
#define GDFONTL_H 1

/* gdfontl.h: brings in the larger of the two provided fonts.
	Also link with gdfontl.c. */

#include "gd.h"

/* 8x16 font derived from a public domain font in the X
        distribution. Only contains the 96 standard ascii characters,
        sorry. Feel free to improve on this. */

extern gdFontPtr gdFontLarge;

#endif
