
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: ahputaway.c 7093 2018-02-09 18:21:09Z paulf $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.5  2004/07/16 20:44:00  lombard
 *     Modified to provide minimal support for SEED location codes.
 *     The file format produced by this code does not itself support
 *     SEED location codes, so there isn't much this code can do about it.
 *
 *     Revision 1.4  2001/04/17 17:21:18  davidk
 *     Added a _STDIO_H_ conditional definition in order to get a function
 *     prototype declared in an includeded header file.  See comment in code
 *     for more detail.
 *
 *     Revision 1.3  2001/04/12 03:47:36  lombard
 *     Major reorganization of AHPA_next to improve efficiency
 *     Reformatted code; cleaned up lots of comments and logit calls
 *     Removed mapping of NC component codes (only needed at UW)
 *
 *     moved mallocs to AHPA_init instead of in AHPA_next_evt.
 *     Removed different path delimiters; '/' works for both NT and Unix.
 *
 *     Revision 1.2  2000/03/10 23:25:51  davidk
 *     changed the ahputaway routines to match the new PA_XXX routines
 *     in putaway.c.  Moved the AHPA_XXX function prototypes to the ah header file.
 *
 *     Revision 1.1  2000/02/14 18:51:48  lucky
 *     Initial revision
 *
 *
 */

/* ah.c 
These are the five routines which plug into xxtrace_save, and cause the trace
data snippets to be put away into a AH format trace files.  These are the
model for other put away routines, such as flat files of various formats.


  Thu Jul 22 14:56:37 MDT 1999 Lucky Vidmar
    Major structural changes to accomodate making all putaway routines
    into library calls so that they can be shared between trig2disk
    and wave2disk. Eliminated global variables set and allocated outside
    of this file. We should only have static globals from now on, with
    everything else being passed in via the putaway.c calls.
   */


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <earthworm.h>
#include <time.h>
#include <trace_buf.h>
#include <swap.h>
#include <ws_clientII.h>
#include <ahhead.h>
#include <pa_subs.h>
#ifdef _WINNT
# ifdef _INC_STDIO
/* DavidK: the version of Microsoft compiler I have defines _INC_STDIO 
   in stdio.h, instead of _STDIO_H_ which is defined in the sun stdio.h 
   There is an ifdef in rpc_nt/xdr.h that checks for _STDIO_H_ before
   declaring a prototype for xdrstdio_create() */
#  define _STDIO_H_
# endif
    /* undefine '__volatile' to squelch warning from redefine in "cdefs.h" */
#if defined(__STDC__) || defined(__cplusplus) || (!defined(__GNUC__) && !defined(lint))
#undef __volatile
#endif
#include <rpc_nt/types.h>
#include <rpc_nt/xdr.h>
#else
#include <rpc/types.h>
#include <rpc/xdr.h>
#endif

#define MAXTXT           150

static  char    AHDir[2*MAXTXT + 4];
static  ahhed *AhHead;                  /* from ahhead.h */
static  float *mydata;                  /* output data */

/* static functions */
#if 0
static  int  gethead(ahhed *, FILE *);
static  int  puthead (ahhed *, FILE *);
static  int  size (ahhed *);
static  int  tohead (int, FILE *);
static  int  getdata (ahhed *, char *, FILE *);
static  int  putdata (ahhed *, char *, FILE *);
static  int  putrecord (ahhed *, char *, FILE *);
static  int  getrecord (ahhed *, char *, FILE *);
static  int  getrecord2 (ahhed *, char **, FILE *);
static  int  gogethead (int, ahhed *, FILE *);
static  int  gogetrecord (int, ahhed *, char *, FILE *);
static  int  logger (char *, ahhed *);
static  int  out_is_tty (void);
static  int  in_is_tty (void);
static  char *mkdatspace(ahhed *);
static  int  get_null_head (ahhed *);
static  int  acpy (char *, char *, unsigned);
static  void ah_error (char *, char *, int);
static  int  xdr_gethead (ahhed *, XDR *);
static  int  xdr_tohead (int, XDR *);
static  int  xdr_getdata (ahhed *, char *, XDR *);
static  int  xdr_getrecord (ahhed *, char *, XDR *);
static  int  xdr_getrecord2 (ahhed *, char **, XDR *);
static  int  xdr_gogethead (int, ahhed *, XDR *);
static  int  xdr_gogetrecord (int, ahhed *, char *, XDR *);
#endif
static  int  maxamp (ahhed *, char *);
static  int  xdr_puthead (ahhed *, XDR *);
static  int  xdr_putdata (ahhed *, char *, XDR *);
static  int  xdr_putrecord (ahhed *, char *, XDR *);
static  int  xdr_ahhead (XDR *, ahhed *);


/* Initialization function, 
*       This is the Put Away startup intializer. This is called when    *
*       the system first comes up. Here is a chance to look around      *
*       and see if it's possible to do business, and to complain        *
*       if not ,BEFORE an event has to be processed.                    *
 */
int AHPA_init (long *OutBufferLen, char *OutDir, int debug)
{
    
  if ( (AhHead = (ahhed *)calloc((size_t) sizeof(ahhed), (size_t) 1)) == (ahhed *)NULL)
  {
    logit("e", "AHPA_init: out of memory for ahhed\n");
    return EW_FAILURE;
  }
      
  if ( (mydata = (float *)calloc(sizeof(float), 
                                 (size_t)(*OutBufferLen/sizeof(float)))) 
       == (float *)NULL)
  {
    logit("e", "AHPA_init: out of memory for data buffer\n");
    return EW_FAILURE;
  }

  /* Make sure that the top level output directory exists */
  if (CreateDir (OutDir) != EW_SUCCESS)
  {
    logit ("e", "AHPA_init: Call to CreateDir failed\n");
    return EW_FAILURE;
  }

  return EW_SUCCESS; /* how could we do less? */
}

/****************************************************************************
*       This is the Put Away event initializer. It's called when a snippet  *
*       has been received, and is about to be processed.                    *
*       It gets to see the pointer to the TraceRequest array,               *
*       and the number of loaded trace structures.                          *
*****************************************************************************/
int AHPA_next_ev (TRACE_REQ *ptrReq, int nReq, char *OutDir,
                  char *EventDate, char *EventTime, int debug)
{
  /* create directory for this event */
  sprintf (AHDir, "%s/%s%s", OutDir, EventDate, EventTime);
  if (CreateDir (AHDir) != EW_SUCCESS)
  {
    logit ("e", "AHPA_init: Call to CreateDir failed\n");
    return EW_FAILURE;
  }

  return EW_SUCCESS;
}

/*****************************************************************************
 *   This is the working entry point into the disposal system. This routine  *
 *   gets called for each trace snippet which has been recovered. It gets    *
 *   to see the corresponding SNIPPET structure, and the event id            *
 *****************************************************************************/
/* Process one channel of data */
int AHPA_next (TRACE_REQ *getThis, double GapThresh, 
               long OutBufferLen, int debug)
{
  FILE *fid;                      /* xdr routines don't like creat */
  XDR xdr;
  TRACE2_HEADER *wf;
  char        *msg_p;        /* pointer into tracebuf data */
  short       *s_data;
  int32_t     *l_data;
  float       *f_data;
  float        fill = 0.0;
  int          j;
  int          gap_count = 0;
  int32_t      nsamp, nfill;
  int32_t      nfill_max = 0;
  int32_t      nsamp_this_scn = 0;
  long         this_size;
  double       starttime, endtime; /* times for current scn         */
  double       samprate;
  double       AH_starttime;
  struct tm    *time;
  time_t       ltime;
  char         datatype; /* 's' for short, 'l' for long */
  char         ahfile[MAXTXT];    /* the name of the ah file */
  
  if ( (msg_p = getThis->pBuf) == NULL)   /* pointer to first message */
  {
    logit("e", "AHPA_next: message buffer is NULL\n");
    return EW_FAILURE;
  }
  
  wf = (TRACE2_HEADER *) msg_p;
  
  sprintf (ahfile, "%s/%s.%s.%s.%s", AHDir, wf->sta, wf->chan, wf->net, wf->loc);

  if ( (fid = fopen(ahfile,"w+")) == NULL)
  {
    logit("e", "AHPA_next: cannot create %s\n", ahfile);
    return(EW_FAILURE);
  }

  if (WaveMsg2MakeLocal(wf) < 0)
  {
    logit("e", "AHPA_next: unknown trace data type: %s\n",
          wf->datatype);
    return( EW_FAILURE );
  }

  nsamp = wf->nsamp;
  starttime = wf->starttime;
  endtime = wf->endtime;
  samprate = wf->samprate;
  if (samprate < 0.01)
  {
    logit("et", "unreasonable samplerate (%f) for <%s.%s.%s.%s>\n",
          samprate, wf->sta, wf->chan, wf->net, wf->loc);
    return( EW_FAILURE );
  }
  AH_starttime = starttime;
  datatype = 'n';
  if (wf->datatype[0] == 's' || wf->datatype[0] == 'i')
  {
    if (wf->datatype[1] == '2') datatype = 's';
    else if (wf->datatype[1] == '4') datatype = 'l';
  }
  else if (wf->datatype[0] == 't' || wf->datatype[0] == 'f')
  {
    if (wf->datatype[1] == '4') datatype = 'f';
  }
  if (datatype == 'n')
  {
    logit("et", "AHPA_next: unsupported datatype: %c\n", datatype);
    return( EW_FAILURE );
  }

  /* loop through all the messages for this s-c-n */
  while (1) 
  {
    /* advance message pointer to the data */
    msg_p += sizeof(TRACE_HEADER);

    /* check for sufficient memory in output buffer */
    this_size = (nsamp_this_scn + nsamp ) * sizeof(float);
    if ( OutBufferLen < (this_size + (long)sizeof(ahhed) ))
    {
      logit( "e", "out of space for <%s.%s.%s.%s>; saving short trace.\n",
             wf->sta, wf->chan, wf->net, wf->loc);
      break;
    }
  
    switch( datatype )
    {
    case 's':
      s_data = (short *)msg_p;
      for ( j = 0; j < nsamp ; j++, nsamp_this_scn++ )
        mydata[nsamp_this_scn] = (float) s_data[j];
      msg_p += sizeof(short) * nsamp;
      break;
    case 'l':
      l_data = (int32_t *)msg_p;
      for ( j = 0; j < nsamp; j++, nsamp_this_scn++ )
        mydata[nsamp_this_scn] = (float) l_data[j];
      msg_p += sizeof(int32_t) * nsamp;
      break;
    case 'f':
      f_data = (float *)msg_p;
      for ( j = 0; j < nsamp; j++, nsamp_this_scn++ )
        mydata[nsamp_this_scn] = f_data[j];
      msg_p += sizeof(float) * nsamp;
      break;
    }
  
    /* End-check based on length of snippet buffer */
    if ((size_t) msg_p >= ((size_t) getThis->actLen + (size_t) getThis->pBuf))
    {
      if (debug == 1)
        logit ("", "Setting done for <%s.%s.%s.%s>\n", wf->sta, wf->chan, 
               wf->net, wf->loc);
      break; /* Break out of the while(1) loop 'cuz we're done */
    }
  
    /* msg_p has been advanced to the next TRACE_BUF; localize bytes *
     * and check for gaps.                                           */
    wf = (TRACE2_HEADER *) msg_p;
    if (WaveMsg2MakeLocal(wf) < 0)
    {
      logit("e", "AHPA_next: unknown trace data type: %s\n",
            wf->datatype);
      return( EW_FAILURE );
    }
    nsamp = wf->nsamp;
    starttime = wf->starttime; 
    /* starttime is set for new packet; endtime is still set for old packet */
    if ( endtime + ( 1.0/samprate ) * GapThresh < starttime ) 
    {
      /* there's a gap, so fill it */
      logit("e", "gap in %s.%s.%s.%s: %lf: %lf\n", wf->sta, wf->chan, wf->net,
            wf->loc, endtime, starttime - endtime);
      nfill = (int32_t) (samprate * (starttime - endtime) - 1);
      if ( (nsamp_this_scn + nfill) * (long)sizeof(float) > OutBufferLen ) 
      {
        logit("e", 
              "bogus gap (%d); skipping\n", (int)nfill);
        return(EW_FAILURE);
      }
      /* do the filling */
      for ( j = 0; j < nfill; j++, nsamp_this_scn ++ ) 
        mydata[nsamp_this_scn] = fill;
      /* keep track of how many gaps and the largest one */
      gap_count++;
      if (nfill_max < nfill) 
        nfill_max = nfill;
    }
    /* Advance endtime to the new packet;        *
     * process this packet in the next iteration */
    endtime = wf->endtime;
  } /* while(1) */

  /*  All data read.  Now fill in header and write AH file */
  AhHead->record.ndata = nsamp_this_scn;           /* samples in trace */
  AhHead->record.delta = (float) (1.0/samprate);   /* sample period */
  ltime = (time_t)AH_starttime;
  /* gmt time makes months from 0 - 11 */
  time = gmtime( &ltime );
  AhHead->record.abstime.yr = (short) (time->tm_year + 1900l);
  AhHead->record.abstime.mo = (short)(time->tm_mon + 1);  
  AhHead->record.abstime.day = (short)time->tm_mday;
  AhHead->record.abstime.hr = (short)time->tm_hour;
  AhHead->record.abstime.mn = (short)time->tm_min;
  AhHead->record.abstime.sec = (float)time->tm_sec + 
    (float)(starttime - (long)starttime);
  AhHead->record.type = FLOAT;

  /* set the maxamp value */
  if (maxamp(AhHead, (char *)mydata) < 0)
  {
    logit("e", "AHPA_next: bad data type for maxamp\n");
    return EW_FAILURE;
  }

  strcpy(AhHead->station.code, wf->sta);   /* station name */
  strcpy(AhHead->station.chan, wf->chan);

  /* here's a kludge using sensor type for network */
  strcpy(AhHead->station.stype, wf->net);
  
  /* We don't know where to put SEED location code except in the file name */

  /* Write an AH file */
  xdrstdio_create(&xdr, fid, XDR_ENCODE);
  xdr_putrecord(AhHead, (char *) mydata, &xdr);
  fclose(fid);
  XDR_DESTROY(&xdr);
  
  if (gap_count) 
    logit("e", "AHPA_next: %d gaps; largest %ld for <%s><%s><%s><%s>\n",
          gap_count, (long)nfill_max, wf->sta, wf->chan, wf->net, wf->loc);

  return(EW_SUCCESS);
}

/************************************************************************
*       This is the Put Away end event routine. It's called after we've     *
*       finished processing one event.                                  *
*************************************************************************/
int AHPA_end_ev (int debug)
{
  /* what else is there to do? */
  return( EW_SUCCESS );
}


/************************************************************************
*       This is the Put Away close routine. It's called after when      *
*       we're being shut down.                                          *
*************************************************************************/
int AHPA_close (int debug)
{
  free(AhHead);
  free(mydata);

  return( EW_SUCCESS );
}


/*      low level i/o routines for ah format records

 *              -- witte        6 june 85
 *
 *      gethead routine was altered so it can read the ah structure 
 *      on a sun4 machine  - Mahdad Parsi June 4 1992.
 */

/* Function prototypes */
extern  int             isatty (int);

/* ah error processing */

int     ah_errno = 0;
int     ah_nerr = 10;

/* ah error numbers */
#define AE_RHED         1       /* error reading header */
#define AE_DTYPE        2       /* bad data type        */
#define AE_WHED         3       /* error writing header */
#define AE_RDATA        4       /* error reading data   */
#define AE_WDATA        5       /* error writing data   */
#define AE_WRECORD      6       /* error writing record */
#define AE_RRECORD      7       /* error reading record */
#define AE_TTYOUT       8       /* binary going to tty  */
#define AE_TTYIN        9       /* binary coming from tty       */

/* ah errlist */

char    *ah_errlist[] = {
  "no error",           /* 0    no error        */
  "read header error",  /* 1    AE_RHED         */
  "bad data type",      /* 2    AE_DTYPE        */
  "write header error", /* 3    AE_WHED         */
  "read data error",    /* 4    AE_RDATA        */
  "write data error",   /* 5    AE_WDATA        */
  "write record error", /* 6    AE_WRECORD      */
  "read record error",  /* 7    AE_RRECORD      */
  "tty can't get binary",       /* 8    AE_TTYOUT       */
  "tty can't send binary"       /* 9    AE_TTYIN        */
};

/*      gethead
 *              gets the next header from the stream pointed to by
 *              file_pt and returns this header in the structure head.
 *              file_pt is assumed to be positioned at the next header,
 *              and does not search.
 *
 *      added:  Reads the input fiule byte by byte and sets the appropriate
 *              structure fields in the ah header. June 4 1992.
 *
 *      returns:
 *                      1               ->      no error
 *                      -1              ->      not enough head to read
 *                      -2              ->      bad data type
 */

#if 0

static  int     gethead(ahhed *head, FILE *file_pt)
{
  int   ierr=0,    /* error indicator */
    i;         /* counter variable */
  size_t     sizefloat,  /* sizeofloat()  */
    sizedouble,  /* sizeodouble()  */
    sizeint32,  /* sizeoint32()  */
    sizeshort;  /* sizeoshort()  */

  sizedouble = sizeof(double) ;
  sizeint32 = sizeof(int32_t) ;
  sizefloat = sizeof(float) ;
  sizeshort = sizeof(short) ;
        
  /* station info */
  if ((ierr = (int)fread(head->station.code, (size_t) CODESIZE, (size_t) 1, file_pt)) == -1) goto CH4ERROR;
  if ((ierr = (int)fread(head->station.chan, (size_t) CHANSIZE, (size_t) 1, file_pt)) == -1) goto CH4ERROR;
  if ((ierr = (int)fread(head->station.stype, (size_t) STYPESIZE, (size_t) 1, file_pt)) == -1) goto CH4ERROR;
  if ((ierr = (int)fread(&head->station.slat, sizefloat, (size_t) 1, file_pt)) == -1) goto CH4ERROR;
  if ((ierr = (int)fread(&head->station.slon, sizefloat, (size_t) 1, file_pt)) == -1) goto CH4ERROR;
  if ((ierr = (int)fread(&head->station.elev, sizefloat, (size_t) 1, file_pt)) == -1) goto CH4ERROR;
  if ((ierr = (int)fread(&head->station.DS, sizefloat, (size_t) 1, file_pt)) == -1) goto CH4ERROR;
  if ((ierr = (int)fread(&head->station.A0, sizefloat, (size_t) 1, file_pt)) == -1) goto CH4ERROR;
  for (i=0; i< NOCALPTS ; i++) {
    if ((ierr=(int)fread(&head->station.cal[i].pole.r,sizefloat,(size_t) 1,file_pt)) == -1) goto CH4ERROR;
    if ((ierr=(int)fread(&head->station.cal[i].pole.i,sizefloat,(size_t) 1,file_pt)) == -1) goto CH4ERROR;
    if ((ierr=(int)fread(&head->station.cal[i].zero.r,sizefloat,(size_t) 1,file_pt)) == -1) goto CH4ERROR;
    if ((ierr=(int)fread(&head->station.cal[i].zero.i,sizefloat,(size_t) 1,file_pt)) == -1) goto CH4ERROR;
  } /* for */

  /* event info */
  if ((ierr=(int)fread(&head->event.lat,sizefloat,(size_t) 1,file_pt)) == -1) goto CH4ERROR;
  if ((ierr=(int)fread(&head->event.lon,sizefloat,(size_t) 1,file_pt)) == -1) goto CH4ERROR;
  if ((ierr=(int)fread(&head->event.dep,sizefloat,(size_t) 1,file_pt)) == -1) goto CH4ERROR;
  if ((ierr=(int)fread(&head->event.ot.yr,sizeshort,(size_t) 1,file_pt)) == -1) goto CH4ERROR;
  if ((ierr=(int)fread(&head->event.ot.mo,sizeshort,(size_t) 1,file_pt)) == -1) goto CH4ERROR;
  if ((ierr=(int)fread(&head->event.ot.day,sizeshort,(size_t) 1,file_pt)) == -1) goto CH4ERROR;
  if ((ierr=(int)fread(&head->event.ot.hr,sizeshort,(size_t) 1,file_pt)) == -1) goto CH4ERROR;
  if ((ierr=(int)fread(&head->event.ot.mn,sizeshort,(size_t) 1,file_pt)) == -1) goto CH4ERROR;
  if ((ierr=(int)fread(&head->event.ot.sec,sizefloat,(size_t) 1,file_pt)) == -1) goto CH4ERROR;
  if ((ierr=(int)fread(head->event.ecomment,(size_t) COMSIZE,(size_t) 1,file_pt)) == -1) goto CH4ERROR;


  /* record info */
  if ((ierr=(int)fread(&head->record.type,sizeshort,(size_t) 1,file_pt)) == -1) goto CH4ERROR;
  if ((ierr=(int)fread(&head->record.ndata,sizeint32,(size_t) 1,file_pt)) == -1) goto CH4ERROR;
  if ((ierr=(int)fread(&head->record.delta,sizefloat,(size_t) 1,file_pt)) == -1) goto CH4ERROR;
  if ((ierr=(int)fread(&head->record.maxamp,sizefloat,(size_t) 1,file_pt)) == -1) goto CH4ERROR;
  if ((ierr=(int)fread(&head->record.abstime.yr,sizeshort,(size_t) 1,file_pt)) == -1) goto CH4ERROR;
  if ((ierr=(int)fread(&head->record.abstime.mo,sizeshort,(size_t) 1,file_pt)) == -1) goto CH4ERROR;
  if ((ierr=(int)fread(&head->record.abstime.day,sizeshort,(size_t) 1,file_pt)) == -1) goto CH4ERROR;
  if ((ierr=(int)fread(&head->record.abstime.hr,sizeshort,(size_t) 1,file_pt)) == -1) goto CH4ERROR;
  if ((ierr=(int)fread(&head->record.abstime.mn,sizeshort,(size_t) 1,file_pt)) == -1) goto CH4ERROR;
  if ((ierr=(int)fread(&head->record.abstime.sec,sizefloat,(size_t) 1,file_pt)) == -1) goto CH4ERROR;
  if ((ierr=(int)fread(&head->record.rmin,sizefloat,(size_t) 1,file_pt)) == -1) goto CH4ERROR;
  if ((ierr=(int)fread(head->record.rcomment,(size_t) COMSIZE,(size_t) 1,file_pt)) == -1) goto CH4ERROR;
  if ((ierr=(int)fread(head->record.log,(size_t) LOGSIZE,(size_t) 1,file_pt)) == -1) goto CH4ERROR;


  /* extra */
  for (i=0; i < NEXTRAS ; i++) {
    if ((ierr=(int)fread(&head->extra[i],sizefloat, (size_t) 1,file_pt)) == -1) goto CH4ERROR;
  }


  if((head->record.type < TYPEMIN) || (head->record.type > TYPEMAX))
  {
    get_null_head(head);
    ierr = -2;  
    ah_errno= AE_DTYPE;
  }

 CH4ERROR:  if (ierr == -1)
 {
   get_null_head(head);
   ierr = -1;
   ah_errno= AE_RHED;
 }
  return(ierr);

}

#endif

/*      puthead
 *              writes the header head onto the stream pointed to by
 *              file_pt.
 *      returns:
 *                      1               ->      no error
 *                      -1              ->      error writing header
 */

#if 0

static  int     puthead (ahhed *head, FILE *file_pt)
{
  int   ierr = 0;

  if((ierr= (int)fwrite((char *)head,(size_t)sizeof(ahhed),(size_t)1,file_pt)) != 1)
  {
    ah_errno= AE_WHED;
    ierr= -1;
  }
  return(ierr);
}

#endif

/*      size
 *              returns the size (in bytes) of the data type given by
 *              head->record.type.
 *      returns:
 *                      size of data type       ->      no error
 *                      -1                      ->      unknown data type
 */

#if 0

static  int     size (ahhed *head)
{
  int   type_size = 0;

  switch(head->record.type)
  {
  case 1:               /* real time series */
    type_size= sizeof(float);
    break;
  case 2:               /* complex time series */
    type_size= sizeof(complex);
    break;
  case 3:               /* real x,y pairs */
    type_size= sizeof(vector);
    break;
  case 4:               /* x real, y complex, or real x,y,z */
    type_size= sizeof(tensor);
    break;
  case 5:               /* complex x,y pairs */
    type_size= 2*sizeof(complex);
    break;
  case 6:               /* double */
    type_size=sizeof(double);
    break;
  default:      /* unknown data type */
    type_size= -1;
    ah_errno= AE_DTYPE;
    break;
  }
  return(type_size);
}

#endif

/*      tohead
 *              positions the read/write head to the beginning of the
 *              n-th header in the file pointed to by file_pt.
 *      returns:
 *                      n       ->      no error
 *                      -1      ->      not enough heads
 *                      -2      ->      bad seek
 */

#if 0

static  int     tohead (int n, FILE *file_pt)
{
  ahhed head;
  int   i,ierr;

  rewind(file_pt);
  for(i=1; i<n; ++i)
  {
    if(gethead(&head,file_pt) == 1)
    {
      if(fseek(file_pt,(long)(head.record.ndata)*(size(&head)),1) == -1)
      {
        ierr = -2;      /* bad seek */
        ah_errno= AE_RHED;
        return(ierr);
      }
    }
    else
    {
      ierr = -1;        /* not enough head */
      ah_errno= AE_RHED;
      return(ierr);
    }
  }
  return(i);    /* success */
}

#endif

/*      getdata
 *              reads from the file pointed to by file_pt into
 *              the array pointed to by array.  It assumes that
 *              the read/write head is positioned correctly 
 *              (i.e., right after the header), and does not
 *              search.  Works for any allowed data type.
 *      returns:
 *                      number of elements read ->      OK
 *                      -1                      ->      error
 */

#if 0

static  int     getdata (ahhed *head, char *array, FILE *file_pt)
{
  int ierr = 0;

  if((ierr = (int)fread(array,(size_t) size(head),(size_t) head->record.ndata,file_pt)) != (int)head->record.ndata)
  {
    ah_errno= AE_RDATA;
    ierr = -1;
  }

  return(ierr);
}

#endif

/*      putdata
 *              writes array to the file pointed to by
 *              file_pt.  Works for any allowed data type.
 *      returns:
 *                      number of elements written      ->      OK
 *                      -1                      ->      error
 */

#if 0

static  int     putdata (ahhed *head, char *array, FILE *file_pt)
{
  int   ierr = 0;

  if((ierr = (int)fwrite(array,(size_t) size(head),(size_t)head->record.ndata,file_pt)) != (int)head->record.ndata)
  {
    ah_errno= AE_WDATA;
    ierr = -1;
  }
  return(ierr);
}

#endif

/*      putrecord
 *              writes head and array to the file pointed to by
 *              file_pt.  Works for any allowed data type.
 *      returns:
 *                      0       ->      OK
 *                      -1      ->      error writing header
 *                      -2      ->      error writing data
 */

#if 0

static  int     putrecord (ahhed *head, char *array, FILE *file_pt)
{
  int   ierr = 0;

  (puthead(head,file_pt) == 1) ? ((putdata(head,array,file_pt) < 0) ? (ierr = -2) : (ierr = 0)) : (ierr = -1);
  if(ierr)
    ah_errno= AE_WRECORD;

  return(ierr);
}

#endif

/*      getrecord
 *              gets header and data from the file pointed to by
 *              file_pt and puts them in head and array.  It assumes
 *              that the read/write head is positioned at the beginning
 *              of the header, and does not search.  Obviously, calling
 *              routine must have allocated enough space.
 *      returns:
 *                      0       ->      OK
 *                      -1      ->      error reading header
 *                      -2      ->      error reading data
 */

#if 0

static  int     getrecord (ahhed *head, char *array, FILE *file_pt)
{
  int   ierr = 0;

  (gethead(head,file_pt) == 1) ? ((getdata(head,array,file_pt) < 0) ? (ierr = -2) : (ierr = 0)) : (ierr = -1);
  if(ierr)
    ah_errno= AE_RRECORD;
  return(ierr);
}

#endif

/*
 *      getrecord2
 *              gets header and data from the file pointed to by
 *              file_pt and puts them in head and array.  It assumes
 *              that the read/write head is positioned at the beginning
 *              of the header, and does not search (although it does
 *              some error checking).  Space for array is allocated, so
 *              be sure to pass a pointer to the data pointer. Got it?
 *      returns:
 *                      0       ->      ok
 *                      -1      ->      error reading record
 *                      -2      ->      error allocating space for data
 */

#if 0

static  int     getrecord2 (ahhed *head, char **array, FILE *file_pt)
{
  int   ierr = 0;
  int   gethead();
  char  *mkdatspace();

  if(gethead(head, file_pt) != 1) {
    ierr = -1;
    return(ierr);
  }

  *array= mkdatspace(head);
  if(*array == NULL) {
    ierr= -2;
    return(ierr);
  }

  if(getdata(head,*array,file_pt) < 0)
    ierr= -1;

  return(ierr);
}

#endif

/*      gogethead
 *              gets n-th header from the stream pointed to by
 *              file_pt and returns this header in the structure
 *              head.
 *      returns:
 *                      0       ->      OK
 *                      -1      ->      stream not long enough
 *                      -2      ->      error reading header
 */

#if 0

static  int     gogethead (int n, ahhed *head, FILE *file_pt)
{
  int   ierr = 0;

  (tohead(n,file_pt) == n) ? ((gethead(head,file_pt) < 1) ? (ierr = -2) : (ierr = 0)) : (ierr = -1);
  return(ierr);
}

#endif

/*      gogetrecord
 *              gets n-th record (header and data) from the stream
 *              pointed to by file_pt and places it in head and array.
 *              Calling routine must allocate enough space.
 *      returns:
 *                      0       ->      OK
 *                      -1      ->      stream not long enough
 *                      -2      ->      error reading record
 */

#if 0

static  int     gogetrecord (int n, ahhed *head, char *array, FILE *file_pt)
{
  int   ierr = 0;

  (tohead(n,file_pt) == n) ? ((getrecord(head,array,file_pt) < 0) ? (ierr = -2) : (ierr = 0)) : (ierr = -1);
  return(ierr);
}

#endif

/* logger adds a 10 character comment to the log section of the header
 * comment should be passed as a character pointer must be terminated
 * by a ';' and a `/0`
 * returns:
 *      logger =  0  -> log info added to header structure
 *      logger = -1  -> no ';', added
 *      logger = -2  -> input string greater than LOGENT
 *                      input truncated to allowable limit
 *      logger = -3  -> attempt to make log string greater than LOGSIZE
 *                      input comment truncated to fit 
 *
 *                      written by Tom Boyd   6/10/85
 */

#if 0

static  int logger (char *char_pt, ahhed *head_pt)
{
  int org,in,err,diff;


  err=0;

  /* find length of log array and input array  */

  org=(int)strlen(head_pt->record.log);  /*log array*/
  in=(int)strlen(char_pt);  /*input array*/

  /* check for a terminating ':' in the input array */

  if(*(char_pt+in-1) != ';')
  {                   /* no semicolon----add it*/
    err=(-1);
    *(char_pt+in)=';';
    *(char_pt+in+1)='\0';
    in+=1;
  }

  /* check the length of the input array */

  if(in > LOGENT)
  {                  /* entry length too long-----truncate it*/
    err=(-2);
    *(char_pt+LOGENT-1)=';';
    *(char_pt+LOGENT)='\0';
    in=LOGENT;
  }

  /* check combined length of array and new input and add it */

  diff=LOGSIZE-(org+in);
  if(diff == -in) return(-3);  /* no room left in log array */
  if(diff < 0)diff*=(-1),err=(-3);  /*partial room left----use it */    
  strncat(head_pt->record.log,char_pt,(size_t) diff); /* cat two strings */

  return(err);
}

#endif

/*      out_is_tty
 *              determines whether stdout is being sent to screen.
 *      returns:
 *                      0       ->      stdout is not tty
 *                      1       ->      stdout is tty
 */

#if 0

static  int     out_is_tty (void)
{

  if(isatty(1)) /* sun specific --- stdout */
  {
    ah_errno= AE_TTYOUT;
    return(1);
  }
  return(0);
}

#endif

/*      in_is_tty
 *              determines whether stdin is tty
 *      returns:
 *                      0       ->      stdin is not tty
 *                      1       ->      stdin is tty
 */

#if 0

static  int     in_is_tty (void)
{

  if(isatty(0)) /* sun specific --- stdin */
  {
    ah_errno= AE_TTYIN;
    return(1);
  }
  return(0);
}

#endif

/*      mkdatspace
 *              allocates enough space for the data array, and
 *              returns a pointer to the memory location, or
 *              NULL if failure.
 *      returns:
 *                      character pointer       ->      success
 *                      NULL                    ->      failure
 */

#if 0

static  char *mkdatspace(ahhed *head)
{
  /*      dont redefine calloc (NT doesn't like it), return the type expected
          withers 9/98

          char  *calloc();
        
          return(calloc((size_t)head->record.ndata,(size_t)size(head)));
  */
  return((char *)calloc((size_t)head->record.ndata,(size_t)size(head)));
}

#endif

#if 0

static int      get_null_head (ahhed *hed)
{
  int   i;


  strcpy(hed->station.code,"null");
  strcpy(hed->station.chan,"null");
  strcpy(hed->station.stype,"null");
  hed->station.slat= 0.0;
  hed->station.slon= 0.0;
  hed->station.elev= 0.0;
  hed->station.DS= 0.0;
  hed->station.A0= 0.0;
  for(i=0; i< NOCALPTS; ++i)
  {
    hed->station.cal[i].pole.r= 0.0;
    hed->station.cal[i].pole.i= 0.0;
    hed->station.cal[i].zero.r= 0.0;
    hed->station.cal[i].zero.i= 0.0;
  }

  hed->event.lat= 0.0;
  hed->event.lon= 0.0;
  hed->event.dep= 0.0;
  hed->event.ot.yr= (short)0;
  hed->event.ot.mo= (short)0;
  hed->event.ot.day= (short)0;
  hed->event.ot.hr= (short)0;
  hed->event.ot.mn= (short)0;
  hed->event.ot.sec= 0.0;
  strcpy(hed->event.ecomment,"null");

  hed->record.type= (short)0;
  hed->record.ndata= 0;
  hed->record.delta= 0.0;
  hed->record.maxamp= 0.0;
  hed->record.abstime.yr= (short)0;
  hed->record.abstime.mo= (short)0;
  hed->record.abstime.day= (short)0;
  hed->record.abstime.hr= (short)0;
  hed->record.abstime.mn= (short)0;
  hed->record.abstime.sec= 0.0;
  hed->record.rmin= 0.0;
  strcpy(hed->record.rcomment,"null");
  strcpy(hed->record.log,"null");

  for(i=0; i< NEXTRAS; ++i)
    hed->extra[i]= 0.0;

  return 0;
}

#endif

/* acpy(from,to,nbytes) copies nbytes from the array "from" to the
 *      array "to".
 */

#if 0

static int      acpy (char *from, char *to, unsigned nbytes)
{
  while(nbytes--)
    *from++ = *to++;
  return 0;
}

#endif

/* print ah format error message and die */

#if 0

static  void ah_error (char *s1, char *s2, int status)
{
  fprintf(stderr,s1,s2);
  if(ah_errno > 0 && ah_errno < ah_nerr)
    fprintf(stderr," (%s)",ah_errlist[ah_errno]);
  fprintf(stderr,"\n");
  exit(status);
}

#endif

#ifndef MAX
#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#endif

/*
 *      maxamp
 *              determines the maximum absolute amplitude of the data array, and
 *              places that number in head.record.maxamp.
 *      returns:
 *                      0       ->      ok
 *                      -1      ->      error
 */
static  int     maxamp (ahhed *head, char *data)
{
  float *fpt;
  double *dpt, dmin, dmax;
  float max,min;
  long  n_data_pts;

  switch(head->record.type)
  {
  case FLOAT:
    n_data_pts= head->record.ndata;
    break;
  case COMPLEX:
  case VECTOR:
    n_data_pts= 2 * head->record.ndata;
    break;
  case TENSOR:
    n_data_pts= 3 * head->record.ndata;
    break;
  case 5:
    n_data_pts= 4 * head->record.ndata;
    break;
  case DOUBLE:
    n_data_pts= head->record.ndata;
    break;
  default:
    ah_errno= AE_DTYPE;
    return(-1);
    break;
  }

  if (head->record.type == DOUBLE) {
    dpt= (double *)data;
    dmax= dmin= *dpt;
    while(n_data_pts--)
    {
      dmax= MAX(dmax,*dpt);
      dmin= MIN(dmin,*dpt);
      ++dpt;
    }
    ((fabs(dmax) > fabs(dmin)) ? (head->record.maxamp= (float) dmax) : (head->record.maxamp= (float) -dmin));
  }
  else {
    fpt= (float *)data;
    max= min= *fpt;
    while(n_data_pts--)
    {
      max= MAX(max,*fpt);
      min= MIN(min,*fpt);
      ++fpt;
    }
    ((fabs((double)max) > fabs((double)min)) ? (head->record.maxamp= max) : (head->record.maxamp= -min));
  }

  return(0);
}
/*      xdr_gethead
 *              gets the next header from the xdr stream pointed to by
 *              xdrs and returns this header in the structure head.
 *              xdrs is assumed to be positioned at the next header,
 *              and does not search.
 *      returns:
 *                      1               ->      no error
 *                      -1              ->      not enough head to read
 *                      -2              ->      bad data type
 */

#if 0

static  int     xdr_gethead (ahhed *head, XDR *xdrs)
{
  int   ierr = 0;

  if((ierr = xdr_ahhead(xdrs, head)) == 1)
  {
    if((head->record.type < TYPEMIN) || (head->record.type > TYPEMAX))
    {
      get_null_head(head);
      ierr = -2;        /* bad data type */
      ah_errno= AE_DTYPE;
    }
  }
  else          /* not enough head */
  {
    get_null_head(head);
    ierr = -1;
    ah_errno= AE_RHED;
  }
  return(ierr);
}

#endif

/*      xdr_puthead
 *              writes the header head onto the xdr stream pointed to by
 *              xdrs.
 *      returns:
 *                      1               ->      no error
 *                      -1              ->      error writing header
 */
static  int     xdr_puthead (ahhed *head, XDR *xdrs)
{
  int   ierr = 0;

  if((ierr= xdr_ahhead(xdrs, head)) != 1)
  {
    ah_errno= AE_WHED;
    ierr= -1;
  }
  return(ierr);
}



/*      xdr_tohead
 *              positions the read/write head to the beginning of the
 *              n-th header in the xdr stream pointed to by xdrs.
 *      returns:
 *                      n       ->      no error
 *                      -1      ->      not enough heads
 *                      -2      ->      bad seek
 */

#if 0

static  int     xdr_tohead (int n, XDR *xdrs)
{
  ahhed head;
  int   i,ierr,j;
  float float_dum;
  double        double_dum;
  complex complex_dum;
  tensor        tensor_dum;

  /* be warned: the following xdr_setpos call may not work at all       */
  /* depending on the stream.  The use of 0 to get to the beginning     */
  /* works empirically, but is not documented  ... sigh - dws           */
  xdr_setpos(xdrs, (u_int) 0);

  for(i=1; i<n; ++i)
  {
    if(xdr_gethead(&head,xdrs) == 1)
    {
      switch (head.record.type) {
      case FLOAT:
        for (j = 0; j < head.record.ndata; j++) {
          if (!xdr_float(xdrs, &float_dum)) {
            ierr = -2;  /* bad seek */
            ah_errno= AE_RHED;
            return(ierr);
          }
        }

        break;
      case COMPLEX:
      case VECTOR:
        for (j = 0; j < head.record.ndata; j++) {
          if (!xdr_float(xdrs, &complex_dum.i) ||
              !xdr_float(xdrs, &complex_dum.r)) {
            ierr = -2;  /* bad seek */
            ah_errno= AE_RHED;
            return(ierr);
          }
        }

        break;
      case TENSOR:
        for (j = 0; j < head.record.ndata; j++) {
          if (!xdr_float(xdrs, &tensor_dum.xx) ||
              !xdr_float(xdrs, &tensor_dum.yy) ||
              !xdr_float(xdrs, &tensor_dum.xy)) {
            ierr = -2;  /* bad seek */
            ah_errno= AE_RHED;
            return(ierr);
          }
        }
        break;
      case 5:
        for (j = 0; j < 4 * head.record.ndata; j++) {
          if (!xdr_float(xdrs, &float_dum)) {
            ierr = -2;  /* bad seek */
            ah_errno= AE_RHED;
            return(ierr);
          }
        }
        break;
      case DOUBLE:
        for (j = 0; j < head.record.ndata; j++) {
          if (!xdr_double(xdrs, &double_dum)) {
            ierr = -2;  /* bad seek */
            ah_errno= AE_RHED;
            return(ierr);
          }
        }
        break;
      default:
        ierr = -2;      /* bad seek */
        ah_errno= AE_DTYPE;
        return(ierr);
      }
    }
    else
    {
      ierr = -1;        /* not enough head */
      ah_errno= AE_RHED;
      return(ierr);
    }
  }
  return(i);    /* success */
}

#endif

/*      xdr_getdata
 *              reads from the xdr stream pointed to by xdrs into
 *              the array pointed to by array.  It assumes that
 *              the read/write head is positioned correctly 
 *              (i.e., right after the header), and does not
 *              search.  Works for any allowed data type.
 *      returns:
 *                      number of elements read ->      OK
 *                      -1                      ->      error
 */

#if 0

static  int     xdr_getdata (ahhed *head, char *array, XDR *xdrs)
{
  int ierr = 0;
  float *pfloat;
  double        *pdouble;
  complex *pcomplex;
  tensor        *ptensor;
  int i;

  switch(head->record.type) {
  case FLOAT:
    pfloat = (float *) array;
    for (i = 0; i < head->record.ndata; i++) {
      if (! xdr_float(xdrs, pfloat++) ) {
        ah_errno= AE_RDATA;
        ierr = -1;
        return(ierr);
      }
      ++ierr;
    }
    break;
  case COMPLEX:
  case VECTOR:
    pcomplex = (complex *) array;
    for (i = 0; i < head->record.ndata; i++) {
      if (!xdr_float(xdrs, &(pcomplex->r)) ||
          !xdr_float(xdrs, &(pcomplex++->i))) {
        ah_errno= AE_RDATA;
        ierr = -1;
        return(ierr);
      }
      ++ierr;
    }
    break;
  case TENSOR:
    ptensor = (tensor *) array;
    for (i = 0; i < head->record.ndata; i++) {
      if (!xdr_float(xdrs, &(ptensor->xx)) ||
          !xdr_float(xdrs, &(ptensor->yy)) ||
          !xdr_float(xdrs, &(ptensor++->xy))) {
        ah_errno= AE_RDATA;
        ierr = -1;
        return(ierr);
      }
      ++ierr;
    }
    break;
  case 5:
    pfloat = (float *) array;
    for (i = 0; i < 4 * head->record.ndata; i++) {
      if (! xdr_float(xdrs, pfloat++) ) {
        ah_errno= AE_RDATA;
        ierr = -1;
        return(ierr);
      }
      ++ierr;
    }
    break;
  case DOUBLE:
    pdouble = (double *) array;
    for (i = 0; i < head->record.ndata; i++) {
      if (! xdr_double(xdrs, pdouble++) ) {
        ah_errno= AE_RDATA;
        ierr = -1;
        return(ierr);
      }
      ++ierr;
    }
    break;
  default:
    ierr = -1;
    ah_errno= AE_DTYPE;
    return(ierr);
  }
  return(ierr);
}

#endif

/*      xdr_putdata
 *              writes array to the xdr stream pointed to by xdrs.
 *              Works for any allowed data type.
 *      returns:
 *                      number of elements written      ->      OK
 *                      -1                      ->      error
 */
static  int     xdr_putdata (ahhed *head, char *array, XDR *xdrs)
{
  int   ierr = 0;
  float *pfloat;
  double        *pdouble;
  complex *pcomplex;
  tensor        *ptensor;
  int i;

  switch(head->record.type) {
  case FLOAT:
    pfloat = (float *) array;
    for (i = 0; i < head->record.ndata; i++) {
      if (! xdr_float(xdrs, pfloat++) ) {
        ah_errno= AE_RDATA;
        ierr = -1;
        return(ierr);
      }
      ++ierr;
    }
    break;
  case COMPLEX:
  case VECTOR:
    pcomplex = (complex *) array;
    for (i = 0; i < head->record.ndata; i++) {
      if (!xdr_float(xdrs, &(pcomplex->r)) ||
          !xdr_float(xdrs, &(pcomplex++->i))) {
        ah_errno= AE_RDATA;
        ierr = -1;
        return(ierr);
      }
      ++ierr;
    }
    break;
  case TENSOR:
    ptensor = (tensor *) array;
    for (i = 0; i < head->record.ndata; i++) {
      if (!xdr_float(xdrs, &(ptensor->xx)) ||
          !xdr_float(xdrs, &(ptensor->yy)) ||
          !xdr_float(xdrs, &(ptensor++->xy))) {
        ah_errno= AE_RDATA;
        ierr = -1;
        return(ierr);
      }
      ++ierr;
    }
    break;
  case 5:
    pfloat = (float *) array;
    for (i = 0; i < 4 * head->record.ndata; i++) {
      if (! xdr_float(xdrs, pfloat++) ) {
        ah_errno= AE_RDATA;
        ierr = -1;
        return(ierr);
      }
      ++ierr;
    }
    break;
  case DOUBLE:
    pdouble = (double *) array;
    for (i = 0; i < head->record.ndata; i++) {
      if (! xdr_double(xdrs, pdouble++) ) {
        ah_errno= AE_RDATA;
        ierr = -1;
        return(ierr);
      }
      ++ierr;
    }
    break;

  default:
    ierr = -1;
    ah_errno= AE_DTYPE;
    return(ierr);
  }
  return(ierr);
}


/*      xdr_putrecord
 *              writes head and array to the xdr stream pointed to by xdrs.
 *              Works for any allowed data type.
 *      returns:
 *                      0       ->      OK
 *                      -1      ->      error writing header
 *                      -2      ->      error writing data
 */
static  int     xdr_putrecord (ahhed *head, char *array, XDR *xdrs)
{
  int   ierr = 0;

  (xdr_puthead(head,xdrs) == 1) ? ((xdr_putdata(head,array,xdrs) < 0) ? (ierr = -2) : (ierr = 0)) : (ierr = -1);
  if(ierr)
    ah_errno= AE_WRECORD;

  return(ierr);
}

/*      xdr_getrecord
 *              gets header and data from the xdr stream pointed to by
 *              xdrs and puts them in head and array.  It assumes
 *              that the read/write head is positioned at the beginning
 *              of the header, and does not search.  Obviously, calling
 *              routine must have allocated enough space.
 *      returns:
 *                      0       ->      OK
 *                      -1      ->      error reading header
 *                      -2      ->      error reading data
 */

#if 0

static  int     xdr_getrecord (ahhed *head, char *array, XDR *xdrs)
{
  int   ierr = 0;

  (xdr_gethead(head,xdrs) == 1) ? ((xdr_getdata(head,array,xdrs) < 0) ? (ierr = -2) : (ierr = 0)) : (ierr = -1);
  if(ierr)
    ah_errno= AE_RRECORD;
  return(ierr);
}

#endif

/*
 *      xdr_getrecord2
 *              gets header and data from the xdr stream pointed to by
 *              xdrs and puts them in head and array.  It assumes
 *              that the read/write head is positioned at the beginning
 *              of the header, and does not search (although it does
 *              some error checking).  Space for array is allocated, so
 *              be sure to pass a pointer to the data pointer. Got it?
 *      returns:
 *                      0       ->      ok
 *                      -1      ->      error reading record
 *                      -2      ->      error allocating space for data
 */

#if 0

static  int     xdr_getrecord2 (ahhed *head, char **array, XDR *xdrs)
{
  int   ierr = 0;
  int   xdr_gethead();
  char  *mkdatspace();

  if(xdr_gethead(head, xdrs) != 1) {
    ierr = -1;
    return(ierr);
  }

  *array= mkdatspace(head);
  if(*array == NULL)
  {
    ierr= -2;
    return(ierr);
  }

  if(xdr_getdata(head,*array,xdrs) < 0)
    ierr= -1;

  return(ierr);
}

#endif

/*      xdr_gogethead
 *              gets n-th header from the xdr stream pointed to by
 *              xdrs and returns this header in the structure
 *              head.
 *      returns:
 *                      0       ->      OK
 *                      -1      ->      stream not long enough
 *                      -2      ->      error reading header
 */
 
#if 0
 
static  int     xdr_gogethead (int n, ahhed *head, XDR *xdrs)
{
  int   ierr = 0;

  (xdr_tohead(n,xdrs) == n) ? ((xdr_gethead(head,xdrs) < 1) ? (ierr = -2) : (ierr = 0)) : (ierr = -1);
  return(ierr);
}

#endif

/*      xdr_gogetrecord
 *              gets n-th record (header and data) from the xdr stream
 *              pointed to by xdrs and places it in head and array.
 *              Calling routine must allocate enough space.
 *      returns:
 *                      0       ->      OK
 *                      -1      ->      stream not long enough
 *                      -2      ->      error reading record
 */
 
#if 0

static  int     xdr_gogetrecord (int n, ahhed *head, char *array, XDR *xdrs)
{
  int   ierr = 0;

  (xdr_tohead(n,xdrs) == n) ? ((xdr_getrecord(head,array,xdrs) < 0) ? (ierr = -2) : (ierr = 0)) : (ierr = -1);
  return(ierr);
}

#endif

static int xdr_ahhead (XDR *xdrsp, ahhed *ahheadp)
{
  u_int l;
  char **pp, *p;
  float **ppf, *pf;

  l = CODESIZE;
  p = ahheadp->station.code;
  pp = &p;
  if (!xdr_bytes(xdrsp, pp, &l, (u_int) CODESIZE))
    return(0);
  l = CHANSIZE;
  p = ahheadp->station.chan;
  pp = &p;
  if (!xdr_bytes(xdrsp, pp, &l, CHANSIZE))
    return(0);
  l = STYPESIZE;
  p = ahheadp->station.stype;
  pp = &p;
  if (!xdr_bytes(xdrsp, pp, &l, STYPESIZE))
    return(0);
  if (!xdr_float(xdrsp, &ahheadp->station.slat))
    return(0);
  if (!xdr_float(xdrsp, &ahheadp->station.slon))
    return(0);
  if (!xdr_float(xdrsp, &ahheadp->station.elev))
    return(0);
  if (!xdr_float(xdrsp, &ahheadp->station.DS))
    return(0);
  if (!xdr_float(xdrsp, &ahheadp->station.A0))
    return(0);
  for (l = 0; l < NOCALPTS; l++) {
    if (!xdr_float(xdrsp, &ahheadp->station.cal[l].pole.r))
      return(0);
    if (!xdr_float(xdrsp, &ahheadp->station.cal[l].pole.i))
      return(0);
    if (!xdr_float(xdrsp, &ahheadp->station.cal[l].zero.r))
      return(0);
    if (!xdr_float(xdrsp, &ahheadp->station.cal[l].zero.i))
      return(0);
  }
  if (!xdr_float(xdrsp, &ahheadp->event.lat))
    return(0);
  if (!xdr_float(xdrsp, &ahheadp->event.lon))
    return(0);
  if (!xdr_float(xdrsp, &ahheadp->event.dep))
    return(0);
  if (!xdr_short(xdrsp, &ahheadp->event.ot.yr))
    return(0);
  if (!xdr_short(xdrsp, &ahheadp->event.ot.mo))
    return(0);
  if (!xdr_short(xdrsp, &ahheadp->event.ot.day))
    return(0);
  if (!xdr_short(xdrsp, &ahheadp->event.ot.hr))
    return(0);
  if (!xdr_short(xdrsp, &ahheadp->event.ot.mn))
    return(0);
  if (!xdr_float(xdrsp, &ahheadp->event.ot.sec))
    return(0);
  l = COMSIZE;
  p = ahheadp->event.ecomment;
  pp = &p;
  if (!xdr_bytes(xdrsp, pp, &l, COMSIZE))
    return(0);
  if (!xdr_short(xdrsp, &ahheadp->record.type))
    return(0);
  if (!xdr_int32_t(xdrsp, &ahheadp->record.ndata))
    return(0);
  if (!xdr_float(xdrsp, &ahheadp->record.delta))
    return(0);
  if (!xdr_float(xdrsp, &ahheadp->record.maxamp))
    return(0);
  if (!xdr_short(xdrsp, &ahheadp->record.abstime.yr))
    return(0);
  if (!xdr_short(xdrsp, &ahheadp->record.abstime.mo))
    return(0);
  if (!xdr_short(xdrsp, &ahheadp->record.abstime.day))
    return(0);
  if (!xdr_short(xdrsp, &ahheadp->record.abstime.hr))
    return(0);
  if (!xdr_short(xdrsp, &ahheadp->record.abstime.mn))
    return(0);
  if (!xdr_float(xdrsp, &ahheadp->record.abstime.sec))
    return(0);
  if (!xdr_float(xdrsp, &ahheadp->record.rmin))
    return(0);
  l = COMSIZE;
  p = ahheadp->record.rcomment;
  pp = &p;
  if (!xdr_bytes(xdrsp, pp, &l, COMSIZE))
    return(0);
  l = LOGSIZE;
  p = ahheadp->record.log;
  pp = &p;
  if (!xdr_bytes(xdrsp, pp, &l, LOGSIZE))
    return(0);
  l = NEXTRAS;
  pf = ahheadp->extra;
  ppf = &pf;
  if (!xdr_array(xdrsp, (char **)ppf, &l, NEXTRAS, (u_int) sizeof(float),
                 (xdrproc_t)xdr_float))
    return(0);
        

  return(1);
}
