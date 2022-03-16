  char  myModName[MAX_MOD_STR];         /* Name of this instance of the */
                                        /*   Decimate module -          */
                                        /*   REQUIRED.                  */
  char  readInstName[MAX_LOGO][MAX_INST_STR];  /* Name of installation  */
                                        /*   that is producing trace    */
                                        /*   data messages.             */
  char  readModName[MAX_LOGO][MAX_MOD_STR];   /* Name of module at the  */
                                        /*   above installation that is */
                                        /*   producing the trace data   */
                                        /*   messages.                  */
  char  readTypeName[MAX_LOGO][MAX_TYPE_STR]; /* Type of trace data msg */
                                        /*   above module is producing. */
  char  ringIn[MAX_RING_STR];           /* Name of ring from which      */
                                        /*   trace data will be read -  */
                                        /* REQUIRED.                    */
  char  ringOut[MAX_RING_STR];          /* Name of ring to which        */
                                        /*   triggers will be written - */
                                        /* REQUIRED.                    */
  int   nlogo;                          /* number of instid/modid pairs */
  int   heartbeatInt;                   /* Heartbeat Interval(seconds). */
  int   logSwitch;                      /* Write logs?                  */
                                        /*   ( 0 = No, 1 = Yes )        */
  int   debug;                          /* Write out debug messages?    */
                                        /*   ( 0 = No, 1 = Yes )        */
  int   cleanStart;                     /* Clean Start flag             */
                                        /*   ( 0 = No, 1 = Yes )        */
  int   testMode;                       /* Test mode flag               */
                                        /*   ( 0 = No, 1 = Yes )        */
  int   processRejected;                /* Process rejected packets flag*/
                                        /*   ( 0 = No, 1 = Yes )        */
  double maxGap;                        /* Gap between tracedata points */
  int   getAllSCNLs;                    /* As if GetSNCL for every      */
                                        /* possible SCNL not already    */
                                        /* covered a GetSCNL w/ in=out  */
                                        /*   ( 0 = No, 1 = Yes )        */
