/*
 * This is psn4putaway.h. It contains structures and function prototypes
 * for the PSN Type 4 putaway routines. If you are looking for the PSN header
 * information, see psn4head.h
 *
 * Created Dec 14 2005 by Larry Cochrane, Redwood City, PSN
 */

#ifndef PSN4PUTAWAY_h
#define PSN4PUTAWAY_h

#include <earthworm.h>
#include <psn4head.h>

int PSN4PA_init( long, char *, char *, int );
int PSN4PA_next_ev( TRACE_REQ *, char *, char *, int );
int PSN4PA_next( TRACE_REQ *, double, long, int );
int PSN4PA_end_ev( int );
int PSN4PA_close( int );

#endif
