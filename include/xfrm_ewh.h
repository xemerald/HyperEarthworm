  unsigned char myInstId;       /* Installation running this module.    */
  unsigned char myModId;        /* ID of this module.                   */
  unsigned char readInstId[MAX_LOGO];  /* Retrieve trace messages from  */
                                /*   specified installation(s).         */
  unsigned char readModId[MAX_LOGO];   /* Retrieve trace messages from  */
                                /*    specified module(s).              */
  unsigned char readMsgType[MAX_LOGO];  /* Retrieve trace messages of   */
                                /*    specified type.                   */
  unsigned char typeError;      /* Error message type.                  */
  unsigned char typeHeartbeat;  /* Heartbeat message type.              */
  long  ringInKey;                      /* Key to input shared memory   */
                                        /*   region.                    */
  long  ringOutKey;                     /* Key to output shared memory  */
                                        /*   region.                    */
  unsigned char typeTrace;      /* Original trace message type (SCN).   */
  unsigned char typeTrace2;     /* New trace message type (SCNL).       */
