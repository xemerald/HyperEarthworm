
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: site.h 1466 2004-05-14 17:45:45Z dietz $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.2  2004/05/14 17:45:45  dietz
 *     Added loc field (location code) and index field.
 *     Changed SCNL string lengths to TRACE2 defined lengths.
 *
 *     Revision 1.1  2000/02/14 20:05:54  lucky
 *     Initial revision
 *
 *
 */

/*
 * site.h : Network parameter definitions.
 *
 *$ 95Aug31 LDD Added net & comp to SITE structure definition
 *$ 95Sep19 KL  Added staname & chanid to SITE structure
 *$ 95Oct19 LDD Added prototypes for functions in site.c
 *
 */
#ifndef SITE_H
#define SITE_H

#include <trace_buf.h>

/* Define the structure that will hold the site table
 ****************************************************/
typedef struct {
        char   name[TRACE2_STA_LEN];  /* shorted from 8 to 6 for "universal" names */
        char   net[TRACE2_NET_LEN];   /* added for "universal" naming convention   */
        char   comp[TRACE2_CHAN_LEN]; /* 950831:ldd                                */
        char   loc[TRACE2_LOC_LEN];   /* added 2004/5/12:ldd                       */
        char   staname[50];
        int    chanid;
        int    index;
        double lat;
        double lon;
        double elev;
} SITE;

int   nSite;
SITE *Site;

/* Prototypes for functions in site.c
 ************************************/
int  site_com  ( void );                   /* process recognized commands     */
void site_read ( char * );                 /* read in a HYPOINV site file     */
int  site_load ( char * );                 /* process a kom.c command file    */
int  site_index( char *, char *, char *, char * ); /* return index in the Site table  */
                                           /*   of the given SCNL code        */
#endif
