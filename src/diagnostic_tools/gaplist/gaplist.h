/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: gaplist.h 2704 2007-02-23 16:48:21Z paulf $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.13  2007/02/23 16:46:11  paulf
 *     fixed long warning for time_t
 *
 *     Revision 1.12  2005/04/11 17:56:32  dietz
 *       Added new optional command: GetLogo <instid> <modid> <msgtype>
 *     If no GetLogo cmds are given, gaplist will process all TYPE_TRACEBUF
 *     and TYPE_TRACEBUF2 messages (previous behavior).
 *       Modified to allow processing of TYPE_TRACE_COMP_UA and TYPE_TRACE2_COMP_UA
 *     messages (headers are NOT compressed!).
 *
 *     Revision 1.11  2004/05/14 22:44:55  kohler
 *     Yet more defensive programming. WMK
 *
 *     Revision 1.10  2004/05/14 22:08:48  kohler
 *     More defensive programming.  WMK
 *
 *     Revision 1.9  2004/05/11 17:04:24  kohler
 *     Now, the scnl array is allocated using realloc, so any number of Scnl and
 *     Label lines are permitted in the config file.
 *
 *     Fixed two bugs:
 *     Label strings longer than 31 characters weren't null-terminated in the
 *     program.  Now, they are, so long labels are correctly truncated.
 *
 *     The label array, defined in gaplist.h, had a length of 31 characters,
 *     which caused some values in the scnl array to get clobbered.  This resulted
 *     in continuous dead/alive messages being sent to statmgr.
 *
 *     Revision 1.8  2004/04/21 19:00:56  kohler
 *     gaplist now looks for gaps in TYPE_TRACEBUF2 message, ie trace messages
 *     that contain location codes.  The program ignores TYPE_TRACEBUF messages.
 *     WMK 4/21/04
 *
 *     Revision 1.7  2004/01/09 00:52:27  kohler
 *     Added totalGap variable.
 *
 *     Revision 1.6  2001/12/07 22:46:57  dietz
 *     added support for optional Label command
 *
 *     Revision 1.5  2001/04/19 19:16:49  kohler
 *     Removed two parameters from gparm.
 *
 *     Revision 1.4  2000/11/20 19:22:16  kohler
 *     Added variable ReportDeadChan to struct GPARM
 *
 *     Revision 1.3  2000/07/24 19:19:00  lucky
 *     Implemented global limits to module, installation, ring, and message type strings.
 *
 *     Revision 1.2  2000/04/18 23:26:10  dietz
 *     added new struct element for MyModId config parameter
 *
 *     Revision 1.1  2000/02/14 17:39:05  lucky
 *     Initial revision
 *
 *
 */

/******************************************************************
 *                         File gaplist.h                         *
 ******************************************************************/

#include <earthworm.h>
#include <trace_buf.h>

#define MAGIC 11676096000.       /* Jan 1, 1970 minus Jan 1, 1600, in sec */
#define NWINDOW    6
#define LABEL_LEN 32             /* Must be a multiple of 4 */

/* Structure definitions
   *********************/
typedef struct
{
   int    isLabel;               /* 1 if label; 0 if scnl */
   char   label[LABEL_LEN];      /* Label line */
   char   sta[TRACE2_STA_LEN];   /* Station name */
   char   chan[TRACE2_CHAN_LEN]; /* Component */
   char   net[TRACE2_NET_LEN];   /* Network */
   char   loc[TRACE2_LOC_LEN];   /* Location code */
   int    count[NWINDOW];        /* Number of hits within a time window */
   int    alive;                 /* 1 if alive, 0 if dead */
   int    status;                /* 0, 1, or 2 */
   int    statusp;               /* 0, 1, or 2 */
   time_t timeOfDeath;           /* When a channel died */
   double EndTimePrev;           /* Time of last sample in previous message */
   double totalGap;              /* Total gap time, in seconds */
} SCNL;

typedef struct
{
   char   InRing[MAX_RING_STR];   /* Name of ring containing tracebuf messages */
   char   MyModName[MAX_MOD_STR]; /* Module name */
   long   InKey;                  /* Key to ring where waveforms live */
   int    HeartbeatInt;           /* Heartbeat interval in seconds */
   double MinGapToLog;            /* Gaps longer than this value (sec) are logged */
   int    TablePrintInterval;     /* Interval in sec at which to print table on screen */
   double ReportDeadChan;         /* Report to statmgr if a chan is dead this many min */
   short     nGetLogo;            /* Number of logos in GetLogo   */
   MSG_LOGO *GetLogo;             /* Logos of requested waveforms */
} GPARM;

