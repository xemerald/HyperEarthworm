
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: decode.h 15 2000-02-14 20:06:34Z lucky $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2000/02/14 20:05:54  lucky
 *     Initial revision
 *
 *
 */

#ifndef DECODE_H
#define DECODE_H

#ifndef MAXBUF
#define MAXBUF          20
#endif

char    dcbuf[MAXBUF];

/* ASCII-to-number conversion (for Fortran-like formatted reading) */
/* char *s is beginning of field        */
/* int  j  is field length              */
/*      f is conversion function        */
#define DECODE(s, j, f) (dcbuf[j]='\0', f(strncpy(dcbuf, s, j)))
#define HASDECPT        index(dcbuf, '.')

#endif
