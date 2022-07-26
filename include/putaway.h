
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: putaway.h 7683 2019-01-18 00:48:34Z stefan $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.2  2007/02/17 02:37:11  stefan
 *     miniseed putaway prototypes
 *
 *     Revision 1.1  2001/04/12 03:08:12  lombard
 *     Initial revision
 *
 *
 *
 */

#ifndef PUTAWAY_H
#define PUTAWAY_H

/* 10/22/2012 Ilya Dricker Moved from putaway.c file */
#define         AH_FORMAT               0
#define         SAC_FORMAT              1
#define         SUDS_FORMAT             2
#define         TANK_FORMAT             3
/* SMF: new GSE format code - we use GSE_INT to allow future expansion
 *      to the other GSE sub-format types (CM6 and CM8) */
#define         GSE_INT_FORMAT          4
/* SMF: new SEIsan format code */
#define         SEI_FORMAT              5
#define         MSEED_FORMAT            6   /* RL */
#define         PSN4_FORMAT	            7
#define         WFDISC_FORMAT           8   /* T. Zander */
#define         UW_FORMAT               9   /*  Chris Wood's USBoR code */
/* 10/22/2012 Ilya Dricker: end of new code */
#define         COSMOS0_FORMAT         10   /* Stefan adding for Erol Kalkan */

/* Function prototypes */
int PA_init (char *DataFormat, long TraceBufferLen, long *OutBufferLen,
             int *FormatInd, char *OutDir, char * OutputFormat, int debug);

int PA_next_ev (char *EventID, TRACE_REQ *trace_req, int num_req,
                int FormatInd, char *OutDir, char *LibDir, char *EventDate,
                char *EventTime, char *EventSubnet, int debug);

int PA_next (TRACE_REQ *getThis, int FormatInd,
             double GapThresh, long OutBufferLen, int debug);

int PA_end_ev (int, int);
int PA_close (int, int);

#endif
int MSEEDPA_init (char *, char *, int);
int MSEEDPA_next_ev (char *,char *, char *, int);
int MSEEDPA_next (TRACE_REQ *, double, int);
int MSEEDPA_end_ev (int);
int MSEEDPA_close (int);


#ifdef _UNIX
int UWPA_init (int traceBufLen, char *OutDir,
                char *OutputFormat, int debug);
int UWPA_next_ev (char *EventID, TRACE_REQ *ptrReq, int nReq,
                char *OutDir, char *EventDate, char *EventTime,
                char *EventInst, char *EventMod, int debug);
int UWPA_next (TRACE_REQ *getThis, double GapThresh,
                int traceBufLen, int debug);
int UWPA_end_ev (int debug);
#endif
