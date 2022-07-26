

/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: neic2scn.h 1111 2002-11-03 00:22:11Z lombard $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.3  2002/11/03 00:13:33  lombard
 *     Protected from multiple includes.
 *
 *     Revision 1.2  2001/07/01 22:03:19  davidk
 *     Added function prototypes from libsrc/util/neic2scn.c.
 *     Previously only structures and constants were defined,
 *     but no prototypes.
 *
 *     Revision 1.1  2000/03/30 18:23:38  lucky
 *     Initial revision
 *
 *
 *
 */

#ifndef NEIC2SCN_H
#define NEIC2SCN_H

#define	MAXLEN		10

typedef struct neic2scn
{

	char	neic_sta[MAXLEN];
	char	sta[MAXLEN];
	char	comp[MAXLEN];
	char	net[MAXLEN];

} NEIC2SCN;

int GetNEICStaList(NEIC2SCN **Sta, int *Nsta, char *filename);
int MatchNeic2SCN(char *neic_sta, char *sta, char *comp, 
                  char *net, NEIC2SCN *neic2scn, int nSta);


#endif
