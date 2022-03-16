/*
** fleng.c
**
** Derived from programs on the CD-ROM:
** "Physics of the Earth and Planetary Interiors."  Appendix to Vol. 96/4.
** (The Flinn-Engdahl Regionalization Scheme: The 1995 revision THE
** standard to be used by seismologists for identifying and specifying
** regions of the earth.)
** Elsvier Science B.V.; Amsterdam; Sept 1996.
** [programs and data are public domain]
*/
#ifndef _FLENG_H_
#define _FLENG_H_

/*
** FlEngLookup()
**
** Either one of grname and srname may be passed as NULL
** without error (obviously, NULL items won't be populated).
*/
void FlEngLookup( double alat
                , double alng
                , char** grname
                , char** srname
                );

#endif
