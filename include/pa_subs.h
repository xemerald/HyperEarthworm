
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: pa_subs.h 7902 2019-03-09 01:51:56Z stefan $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2001/04/12 03:06:13  lombard
 *     Initial revision
 *
 *
 *
 */

#ifndef PA_SUBS_H
#define PA_SUBS_H

/*
 * The various putaway routines used for AH, SAC, SUDS and tankplayer
 * formats. Normally these will be accessed through putaway.c, which
 * is a selector for the various formats.
 */

#include <ws_clientII.h>   /* for TRACE_REQ */

/* Function prototypes */
int AHPA_init(long *OutBufferLen, char *OutDir, int debug);
int AHPA_next_ev(TRACE_REQ *ptrReq, int nReq, char *OutDir,
                 char *EventDate, char *EventTime, int debug);
int AHPA_next(TRACE_REQ *getThis, double GapThresh, 
              long OutBufferLen, int debug);
int AHPA_end_ev(int debug);
int AHPA_close(int debug);

int COSMOS0PA_init(int, char *, char *, int);
int COSMOS0PA_next_ev(TRACE_REQ *, char *, char *, char*, char*, int, int, double);
int COSMOS0PA_next(TRACE_REQ *, double, long, int, int, double);
int COSMOS0PA_end_ev(int);
int COSMOS0PA_close(int);

int SACPA_init(long OutBufferLen, char *OutDir, char * OutputFormat, int debug);
int SACPA_next_ev(char *EventID, TRACE_REQ *ptrReq, int nReq, 
                  char *OutDir, char *EventDate, char *EventTime, int debug);
int SACPA_next(TRACE_REQ *getThis, double GapThresh, 
               long OutBufferLen, int debug);
int SACPA_end_ev (int);
int SACPA_close (int);

int SUDSPA_init (int OutBufferLen, char *OutDir, char *OutputFormat, 
                 int debug);
int SUDSPA_next_ev (char *EventID, TRACE_REQ *ptrReq, int nReq, 
                    char *OutDir, char *EventDate, char *EventTime,
                    char *EventSubnet, int debug);
int SUDSPA_next (TRACE_REQ *getThis, double GapThresh,
                 long OutBufferLen, int debug);
int SUDSPA_end_ev(int debug);
int SUDSPA_close(int debug);

int TANKPA_init(char *OutDir, int debug);
int TANKPA_next_ev(char * EventID, char *OutDir, char *EventDate, 
                   char *EventTime, int debug);
int TANKPA_next(TRACE_REQ *getThis, int debug);
int TANKPA_end_ev (int debug);
int TANKPA_close(int debug);

#endif
