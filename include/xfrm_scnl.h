  double     inEndtime;                /* end time of last sample in    */
  double     samprate;                 /* sample rate of this SCNL-span */
  char       inSta[TRACE2_STA_LEN];    /* input Station code (name).    */
  char       inChan[TRACE2_CHAN_LEN];  /* input Channel code.           */
  char       inNet[TRACE2_NET_LEN];    /* input Network code.           */
  char       inLoc[TRACE2_LOC_LEN];    /* input Location code.          */
  char       outSta[TRACE2_STA_LEN];   /* output Station code (name).   */
  char       outChan[TRACE2_CHAN_LEN]; /* output Channel code.          */
  char       outNet[TRACE2_NET_LEN];   /* output Network code.          */
  char       outLoc[TRACE2_LOC_LEN];   /* output Location code.         */
  char       datatype[3];              /* datatype of this SCNL-span    */
  signed char status;			/* 0=OK, -ve=uninitialized, +ve=aborted */
  char       *inMsgBuf;            /* Input Message Buffer */
  int        prevSta;              /* Previous Station Index or -1 if none */
  int        nextSta;              /* Next Station Index or -1 if none. */
  double     conversionFactor;     /* Conversion Factor */
