  SHM_INFO      regionOut;      /* Output shared memory region info     */
  MSG_LOGO      hrtLogo;        /* Logo of outgoing heartbeat message.  */
  MSG_LOGO      errLogo;        /* Logo of outgoing error message.      */
  MSG_LOGO      trcLogo;        /* Logo of outgoing tracebuf message.   */
  QUEUE 	    MsgQueue;	    /* The message queue                    */
  pid_t         MyPid;          /* My pid for restart from startstop    */
  int           XfrmStatus;     /* 0=> Decimator thread ok. <0 => dead  */
  char          *scnls;         /* Array of SCNLs to process            */
  int           nSCNL;          /* How many SCNLs do we have            */  
  char          *mod_name;      /* Name of module                       */
  int           (*completionFcn)(); /* Function called when done        */
  int           scnlRecSize;    /* Size of each SCNL record */
  int           useInBufPerSCNL;/* 0=shared input buffer, 1=input buffer for each SCNL */
  int           nInSCNL;        /* How many input SCNLs do we have      */
  char          *version;       /* Version of module                    */
  int			version_magic;	/* Version "magic number"               */
