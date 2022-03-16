/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: kwhdrs_ew.h 7540 2018-07-06 16:49:38Z baker $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.3  2005/10/11 20:15:53  luetgert
 *     Added network and location codes.
 *
 *     Revision 1.2  2002/11/03 00:11:53  lombard
 *     Protected from multiple includes.
 *
 *     Revision 1.1  2002/03/22 19:53:47  lucky
 *     Initial revision
 *
 *
 */

/* kwhdrs_ew.h   Combined K2 & Mt. Whitney header definition
                 also Tag definition & Frame header definition  */

/*   2/18/99 -- [ET]  Modified version with 'FIX_8BYTES_K2RW' fix */

/*   11/3/00 -- [JL]  Changed the variable type of cellStartTime[5]
                      in MODEM_RW_PARMS from int to short.

                      Changed the variable type of blockTime
                      in FRAME_HEADER from
                      unsigned long blockTime;
                         to
                      unsigned short blockTime;
                      unsigned short blockTime2;
                      This circumvents a mis-alignment problem
                      that otherwise occurs due to the fact
                      that long variables on the SUN are aligned
                      on longword boundaries.

                      Reversed [ET]'s modification of 2/18/99
                      and fixed the problem properly.

                      Added SENSOR TYPE CODES 32-40

                      */
/**************************************************************

REVISION HISTORY:

Version 1.00 (July 5, 1994)
-------------------------
Initial release with support for 12 channels.

Version 1.10 (August 25, 1994)
----------------------------
NOTE: all structure sizes and previous parameter offsets remain the same.

Used expansion space in CHANNEL_RW_PARMS to add the following:
   unsigned short sensorSerialNumberExt;  (high word of sensor serial number)
   float resp1,resp2,resp3,resp4,resp5,resp6; (sensor response)

Used expansion space in MISC_RW_PARMS to add the following:
   unsigned char minBatteryVoltage; (minimum battery voltage x 10)

Added new sensor code definitions for FBA 4g, 2g, 1g, 0.5g, 0.25g and 0.1g
for parameter CHANNEL_RW_PARMS.sensorType.  The code definitions are
   #define SENSOR_FBA_4G 11
   #define SENSOR_FBA_2G 12
                etc.

Changed MISC_RO_PARMS.batteryVoltage type from unsigned short to signed
short.  MISC_RO_PARMS.batteryVoltage has negative value if charging.
(For example, MISC_RO_PARMS.batteryVoltage = -130 indicates 13.0 V
and charging).

Redefined the meaning of STREAM_RW_PARMS.filterFlag.
Redefined the meaning of MODEM_RW_PARMS.flags.
Changed TIMING_RW_PARMS.gpsTurnOnInterval from hours to minutes.

New parameters:
   MISC_RO_PARMS.temperature
   MISC_RO_PARMS.restartSource
   MISC_RW_PARMS.networkFlag
   FRAME_HEADER.msec
   STREAM_RO_PARMS.nscans
   STREAM_RO_PARMS.startTimeMsec
   STREAM_RO_PARMS.triggerTimeMsec
   CHANNEL_RO_PARMS.aqOffset

   Added on 8/26/94:
   TIMING_RO_PARMS.gpsLongitude
   TIMING_RO_PARMS.gpsLatitude
   TIMING_RO_PARMS.gpsAltitude

   Added 9/9/94:
   MODEM_RW_PARMS.flags: added more definitions for bit fields

   Added 10/11/94:
   TIMING_RO_PARMS.dacCount


  10/20/94 make note to CORRECT name:
   STREAM_RW_PARMS.triggerThreshold & .detriggerThreshold ACTUALLY 'VOTES TO TRIGGER' !!!!!

   11/18/94   make header 24 channels -- to check arrays --  OK!

   Version 1.20 (28 November 1994)
 ---------------------------------------
   11/29/94  make header 18 channels - Mt.Whitney -- for now
                MAX_CHANNELS 18
                FRAME_HEADER_TYPE:  FHT16 3   one 16-bit channel bit map
                                    FHT24 4     "        "                + byte bit map
                PARAMETER_VERSION 120
 ---------------------------------------------------------



  Version 1.20 Mt Whit / 1.30 K2 (January 30, 1995) (March 3, 1995)
  ----------------------------
      Moved MTWHITNEY define here from mt.h to allow preprocessing
      for different headers for K2 and Mt Whit
              Mt Whit:        MAX_CHANNELS                    12 -> 18
                                      PARAMETER_VERSION               110 -> 120
                                      FRAME_HEADER_TYPE               3 (16 ch) -> 4 (24 ch)

      FRAME_HEADER:  Renamed 'unsigned char pad4' to 'uc channelBitMap1' to
      support chan 17-24 (chan 1-16 are in channelBitMap)

      MISC_RW_PARMS: added ul channel_bitmap, ul cutler_bitmap,
      uc cutler_decimation, uc cutler_irig_type, uc cutler_protocol;
      uc cutlerCode now just sets the baudrate;
      reduced 'char bpad[14]' & 'int wpad[7]
      to [10] & [3] to keep structure the same size

      MISC_RO_PARMS: add us installedChan so QT can see how many
      channels are installed; decreased char bytepad[5] -> [3] to
      keep struct the same size

      K2_ALARM: changed ui channelBitMap to ulong

  *********************************************

     8/25/94
     3/03/95   Ver 1.20/1.30
              remove K2 File System(K2-Directory) definitions. not used
              remove EVT_SUMMARY, not used
              remove STATUS_INFO, not used
              add from another .H: K2_TAG

  -----------------03-05-95 06:23pm-----------------
   3/05/95  update with 1.20/1.30 header
            make final header, frame & tag structures = typedefs
  --------------------------------------------------

  -----------------03-16-95 01:05pm-----------------
   3/16/95  combined version K2 & MW headers - 12/18 channels
  --------------------------------------------------

   ----------------------------------------------
   5/21/96  add Parameters to Mt. Whitney (& K2)
            NO CHANGE IN VERSION NUMBER - REDEFINITION OF EXISTING PADS

            STREAM_RO_PARAMS: pad[2] -> triggerBitMap, pad[1]
            MISC_RW_PARMS: bpad[11],wpad[3] -> char siteID[17]
            MODEM_RW_PARMS:

              char pauseBetweenCalls   '' define '0'= random delay 5-30sec

              from:  char bytepad[17]
              to:    char cellShare;  '' K2 only, =0 for 1 Hz output, =1 for cell phone ''
                     char cellOnTime;       '' in minutes ''
                     unsigned char cellWarmupTime;  '' in seconds ''
                     int cellStartTime[2];  '' minutes since midnight ''
                     char bytepad[10];

              from:  unsigned short wordpad[7];
                     unsigned long dwordpad[8];
              to:    char calloutMsg[46];

  -------------------------------------------------


  -------------------------------------------------
    5/31/96  add SRT & TRIG DATA bits to  ' roParms.stream.flags'
  -------------------------------------------------

   ----------------------------------------------
    6/10/96  change  modem-rw:
                         int  cellStartTime[] from 2 to 5
                         char bytepad[] from 10 to 4

  -------------------------------------------------

   7/2/96  noted for CHANNEL_RO_PARAMETERS:
              'recorded channels entered consecutively'

   9/18/96  new k2defs.h from MERGED MCU - no changes that effect
               kwhd1.c
              QLCA's vhf1/2.c merged into kwhd1/2.c
              cellShare -- NOT USED
              REMOVE:  from formatted files: Sensor Response -- not used
              kwhdrs.h NOT redistributed over all files
  *************************************************************

   ---------------------

   3/28/97

  File Version 1.50: 1.40(K2) & 1.50(MW) 12/18-channel header (February 1997)
  ------------------------------------------------------

    Change the following:

    CHANNEL_RW_PARAMETERS:

    after 'gain':

    OUT -- 8 bytes :

     float resp1; * sensor response; poles and zeroes *
     float resp2;

    IN  -- 8 bytes :

     unsigned char triggertype;       * Type of trigger calculation  *
                                      default = 0 = Threshold

                                   valid: code 0h  threshold
                                               1   sta/lta

     unsigned char iirtrigfilter;     * coefficients for iir trigger filter *
                                      default = 1 = B  0.1Hz to 12.5 Hz @ 200sps

                    valid: code 0h  A
                                1   B
                                2   C
                                3   D

     unsigned char stasecondsTten;    * STA seconds * ten *
                                       default = code 7 = 1.0 seconds

                      valid: code 0h  0.1  store 1     Ah  1.6 store 16
                                  1   0.2   as   2     B   1.8  as   18
                                  2   0.3        3     C   2.0       20
                                  3   0.4        4     D   2.5       25
                                  4   0.5        5     E   3.0       30
                                  5   0.6        6     F   5.0       50
                                  6   0.8        8    10  10.0      100
                                  7   1.0       10
                                  8   1.2       12
                                  9   1.4       14

     unsigned char ltaseconds;         * LTA seconds *
                                        default = code 4 = 60 seconds

                                   valid: code 0h  20
                                               1   30
                                               2   40
                                               3   50
                                               4   60
                                               5   80
                                               6  100
                                               7  120

     unsigned short sta/ltaratio;    * STA/LTA trigger ratio * ten *
                                     default = code 3 = 4

                      valid: code 0h  1.5  store 15   7h  15 store 150
                                  1   2     as   20   8h  20  as   200
                                  2   3          30   9   30       300
                                  3   4          40   A   40       400
                                  4   6          60   B   60       600
                                  5   8          80   C  100      1000
                                  6  10         100

     unsigned char sta/ltapercent;  * STA/LTA detrigger percent of trigger ratio *
                                     default = code 3 = 40%

                      valid: code 0h  10%  store 10
                                  1   15    as   15
                                  2   20         20
                                  3   40         40
                                  4   60         60
                                  5  100        100

     char bytepad1[1];        * reserved *



    after 'alarmTriggerThreshold':

    OUT - 16 bytes :

     float resp3;
     float resp4;
     float resp5;
     float resp6;

    IN  - 16 bytes:

     char bytepad[16];     * reserved *

     ------------

   STREAM_RW_PARMS:

   OUT -- 24 bytes:

     short wordpad[4]; * for expansion *

     long dwordpad[4]; * for expansion *

   IN -- 24 bytes:

     char bytepada;
     unsigned char FilterType;        * FirB: 0 regular, 1 causal *
     unsigned char DataFmt;           * Serial Data Stream: 0=uncompressed
                                                       1=compressed  *
     char Reserved;
     short Timeout;                   * Serial Data Stream Mode:        *
                                      *    mode 1 - 0   default    *
                                      *         2 - 6..32767       *
                                      *             send 'continue cmd' *
                                      *         3 -  -1  cmd for each block *

     unsigned short TxBlkSize;        * Serial Data Stream :           *
                                      *  fixed @ 100 samples per block *
     unsigned short BufferSize;       * Serial Data Stream:            *
                                      *  size of #SSTRBUF.BIN file in disk *
                                      *  number of 16,384 byte blocks *
                                      *  default= 64  - 1Meg *
     unsigned short SampleRate;       * Serial Data Stream:            *
                                      *  fixed at 100 sps *
     unsigned long TxChanMap;         * Serial Data Stream:            *
                                      *  select up to 6 channels *

     long dwordpad[2]; * for expansion *

   ----------------

   Bottom Of Header Comments
****************************************************************************/

#ifndef KWHDRS_EW_H
#define KWHDRS_EW_H

#include <stdint.h>		/* int32_t, uint32_t */

/* Type Definitions:
 *        char: 1 byte integer
 *        short: 2 byte integer
 *        float: 4 byte floating point (IEEE format)
 *        int32_t: 4 byte integer (used to be 'long')
 *   All structures are 16-bit aligned
*/

#define MAX_ETNA_CHANNELS 4

#define MAX_EVEREST_CHANNELS 6

#define MAX_MW_CHANNELS 18
#define VERSION_MW 150 /* =1.20 & 1.50; header version number x 100 */

#define MAX_K2_CHANNELS 12
#define VERSION_K2 140 /* = 1.00, 1.10, 1.30 & 1.40; header version number x 100 */

#define DFS_MAX_CHANNELS 3  /* =3 orig, eventually = 6 */

#define MAX_STREAMS 1
#define INSTRUMENT_CODE 9 /* for K2 */

/* SENSOR TYPE CODES */
#define SENSOR_UNKNOWN      0
#define SENSOR_ACCELERATION 1
#define SENSOR_VELOCITY     2
#define SENSOR_FBA11       10
#define SENSOR_FBA_4G      11     /* 4g FBA */
#define SENSOR_FBA_2G      12     /* 2g FBA */
#define SENSOR_FBA_1G      13     /* 1g FBA */
#define SENSOR_FBA_0_5G    14     /* 0.5g FBA */
#define SENSOR_FBA_0_25G   15     /* 0.25g FBA */
#define SENSOR_FBA_0_1G    16     /* 0.1g FBA */
#define SENSOR_FBA23       20
#define SENSOR_WR1         30     /* velocity OR acceleration */
#define SENSOR_EPI         32     /* Episensor */
#define SENSOR_S6000       33
#define SENSOR_MARKL22     34
#define SENSOR_MARKL4C     35
#define SENSOR_CMG3        36
#define SENSOR_CMG3T       37
#define SENSOR_CMG40T      38
#define SENSOR_CMG5        39
#define SENSOR_KS_2000     40

/* Voter type codes */
#define VOTER_UNDEFINED         0
#define VOTER_CHANNEL           'C'
#define VOTER_STREAM            'S'
#define VOTER_NETWORK           'N'
#define VOTER_KEYBOARD          'K'
#define VOTER_EXTERNAL          'E'
#define STREAM_K2_MAX_VOTERS (MAX_K2_CHANNELS+3) /* channels+network+keyboard+ext */
#define STREAM_MW_MAX_VOTERS (MAX_MW_CHANNELS+3) /* channels+network+keyboard+ext */

/* Restart sources for use with MISC_RO_PARMS.restartSource */
#define RESTART_UNKNOWN 0
#define RESTART_FROM_POWERSWITCH 1 /* someone flipped the power switch */
#define RESTART_FROM_COMMAND 2 /* user command to reset */
#define RESTART_FROM_WATCHDOG 3 /* watchdog caught a system crash */
#define RESTART_FROM_DSPFAILURE 4 /* DSP failure caused reset */
#define RESTART_FROM_BATTERYFAILURE 5 /* battery failure caused reset */
#define RESTART_FROM_MEMORYERROR 6 /* memory error caused reset */

/*---------------------------------------------------*/

/* Voting information */

struct VOTER_INFO {
   unsigned char type; /* voter type code */
   unsigned char number; /* channel number, stream number, etc. */
   short weight; /* voting weight: range is -100 to 100 */
};

/*---------------------------------------------------*/

/* Channel read/write parameters */

#define CHANNEL_ID_LENGTH    5
#define NETWORK_CODE_LENGTH  3
#define LOCATION_CODE_LENGTH 3

struct CHANNEL_RW_PARMS {
   char id[CHANNEL_ID_LENGTH]; /* NULL terminated */
   char bytepad; /* for expansion */
   unsigned short sensorSerialNumberExt; /* high word of serial number */
   short north; /* displacement */
   short east; /* displacement */
   short up; /* displacement */
   short altitude;
   short azimuth;
   unsigned short sensorType;
   unsigned short sensorSerialNumber; /* low word of serial number */
  unsigned short gain;  /*  only '1' defined as gain of 1 5/31/95 */

/* begin  added 3/29/97 1.40 K2 & 1.50 MTW */
   unsigned char triggerType;  /* Type of trigger calculation  added headver 1.50
                                  default = 0 = Threshold

                                  valid: code 0h  threshold
                                              1   sta/lta    */

   unsigned char iirTriggerFilter;  /* coefficients for iir trigger filter  ver 1.50
                                    default = 1 = B  0.1Hz to 12.5 Hz @ 200sps

                                    valid: code 0h  A
                                                1   B
                                                2   C  */



   unsigned char StaSeconds;  /* STA seconds * ten   ver 1.50
                                     default = code 7 = 1.0 seconds

                    valid: code 0h  0.1  store 1     Ah  1.6 store 16
                                1   0.2   as   2     B   1.8  as   18
                                2   0.3        3     C   2.0       20
                                3   0.4        4     D   2.5       25
                                4   0.5        5     E   3.0       30
                                5   0.6        6     F   5.0       50
                                6   0.8        8    10  10.0      100
                                7   1.0       10
                                8   1.2       12
                                9   1.4       14   */

   unsigned char LtaSeconds;    /* LTA seconds     ver 1.50
                                   default = code 4 = 60 seconds

                                 valid: code 0h  20
                                             1   30
                                             2   40
                                             3   50
                                             4   60
                                             5   80
                                             6  100
                                             7  120  */

   unsigned short StaLtaRatio;  /* STA/LTA trigger ratio * ten  ver 1.50
                                    default = code 3 = 4

                    valid: code 0h  1.5  store 15   7h  15 store 150
                                1   2     as   20   8h  20  as   200
                                2   3          30   9   30       300
                                3   4          40   A   40       400
                                4   6          60   B   60       600
                                5   8          80   C  100      1000
                                6  10         100      */

   unsigned char StaLtaPercent;  /* STA/LTA detrigger percent of trigger ratio
                                     default = code 3 = 40%         ver 1.50

                    valid: code 0h  10%  store 10
                                1   15    as   15
                                2   20         20
                                3   40         40
                                4   60         60
                                5  100        100   */

   char bytepada;
/* end 1.40 & 1./50 additions */

   float fullscale; /* volts */
   float sensitivity; /* in volts per unit (e.g., g's) */
   float damping; /* fraction of critical */
   float naturalFrequency; /* hz */
   float triggerThreshold; /* fraction of fullscale */
   float detriggerThreshold; /* fraction of fullscale */
   float alarmTriggerThreshold; /* fraction of fullscale */

/* EpiSensor addition 3/24/05 */
   float calCoil;               /* g/Volt for EpiSensor */
   unsigned char range;         /* Range code for sensor: from EEPROM
                                      1=4g
                                      2=2g
                                      3=1g
                                      4=1/2g
                                      5=1/4g, etc */
   unsigned char sensorgain;    /* Same, but as determined by MCU */
/* End of EpiSensor addition 3/24/05 */

/* Network and location code additions 3/24/05 */
   char networkcode[NETWORK_CODE_LENGTH];     /* Network code  */
   char locationcode[LOCATION_CODE_LENGTH];   /* Location code */
   char bytepad2[4];
/* End of network and location code additions 3/24/05 */
};


/*---------------------------------------------------*/

/* Read only parameters */
/* recorded channels entered consecutively */

struct CHANNEL_RO_PARMS {
   int32_t maxPeak; /* raw sample counts */
   uint32_t maxPeakOffset; /* offset from start of file */
   int32_t minPeak;
   uint32_t minPeakOffset; /* offset from start of file */
   int32_t mean; /* raw sample counts */
   int32_t aqOffset;
   int32_t dwordpad[3]; /* for expansion */
};

/*---------------------------------------------------*/

/* Stream read/write information */

struct STREAM_K2_RW_PARMS {
   unsigned char filterFlag; /* BIT0 = 1 if filtered data;  NOT USED
                                BIT1 = 1 if auto FT after Event
                                BIT2 = 1 if compressed - NOT USED */
   unsigned char primaryStorage; /* = 0 for drive A: ,etc. */
   unsigned char secondaryStorage; /* = 1 for drive B:, etc. */
   unsigned char bytepad[5]; /* for expansion */
   unsigned short eventNumber; /* *NOT USED* */
   unsigned short sps; /* sampling rate */
   unsigned short apw; /* array propagation window, in seconds */
   unsigned short preEvent; /* in seconds */
   unsigned short postEvent; /* in seconds */
   unsigned short minRunTime; /* in seconds */
   short VotesToTrigger; /* signed number */
   short VotesToDetrigger;

/* begin change 3/29/97 1.40 & 1.50 */
   char bytepada;
   unsigned char FilterType;        /* 0 regular, 1 causal */
   unsigned char DataFmt;
   char Reserved;
   short Timeout;
   unsigned short TxBlkSize;
   unsigned short BufferSize;
   unsigned short SampleRate;
   uint32_t TxChanMap;
   int32_t dwordpad[2]; /* for expansion */
/* end change 3/29/97 1.40 & 1.50 */

   /* Voter info: one entry for each channel, network, user command */
   /* Use functions IsStreamVoter() and GetStreamVoterWeight() in
          STRMPAR.C to access the members.
   */
   struct VOTER_INFO voterInfo[STREAM_K2_MAX_VOTERS];
};

/* Stream read/write information */

struct STREAM_MW_RW_PARMS {
   unsigned char filterFlag; /* BIT0 = 1 if filtered data;
                                BIT1 = 1 if auto FT after Event
                                BIT2 = 1 if compressed */
   unsigned char primaryStorage; /* = 0 for drive A: ,etc. */
   unsigned char secondaryStorage; /* = 1 for drive B:, etc. */
   unsigned char bytepad[5]; /* for expansion */
   unsigned short eventNumber; /* *NOT USED* */
   unsigned short sps; /* sampling rate */
   unsigned short apw; /* array propagation window, in seconds */
   unsigned short preEvent; /* in seconds */
   unsigned short postEvent; /* in seconds */
   unsigned short minRunTime; /* in seconds */
   short VotesToTrigger; /* signed number */
   short VotesToDetrigger;

/* begin change 3/29/97 1.40 & 1.50 */
   char bytepada;
   unsigned char FilterType;        /* 0 regular, 1 causal */
   unsigned char DataFmt;
   char Reserved;
   short Timeout;
   unsigned short TxBlkSize;
   unsigned short BufferSize;
   unsigned short SampleRate;
   uint32_t TxChanMap;
   int32_t dwordpad[2]; /* for expansion */
/* end change 3/29/97 1.40 & 1.50 */

   /* Voter info: one entry for each channel, network, user command */
   /* Use functions IsStreamVoter() and GetStreamVoterWeight() in
          STRMPAR.C to access the members.
   */
   struct VOTER_INFO voterInfo[STREAM_MW_MAX_VOTERS];
};

/*---------------------------------------------------*/

/* Stream read only parameters */

struct STREAM_RO_PARMS {
   uint32_t startTime; /* first sample time, includes PEM */
   uint32_t triggerTime;
   uint32_t duration; /* in # of frames; note: frames may have different sizes */
   unsigned short errors;
   unsigned short flags; /* Bit 0 = 1 if functional test */
                         /* bit 1 = 1 if Sensor Response Test, SRT (& 0x03 = 0x03) */
                         /* bit 2 = 1 if recorded data = trigger data (& 0x04 = 0x04) */
   unsigned short startTimeMsec;
   unsigned short triggerTimeMsec;
   uint32_t nscans; /* # of scans in the event */

  /* start of RW_K2_PARMS is 8 bytes too far along--don't know where */
  /*  the problem is, so taking 8 bytes away from the end of the */
  /*  RO_K2_PARMS area -- 2/18/99 -- [ET] */

  /* The problem was the ambiguous definition of four variables as int */
  /* rather than short.  With these fixed, ET's kludge is no longer required. */
  /* -- 11/3/00 -- [JL] */

   uint32_t triggerBitMap; /* indicates first channel to trigger 5/96 */
   uint32_t pad; /* for expansion */
};

/*---------------------------------------------------*/

/* Miscellaneous read/write parameters: 92 bytes */

struct MISC_RW_PARMS {
   unsigned short serialNumber;
   unsigned short nchannels;            /* number of channels used */
#define STN_ID_LENGTH 5
   char stnID[STN_ID_LENGTH]; /* null terminated */
#define COMMENT_LENGTH 33
   char comment[COMMENT_LENGTH]; /* NULL terminated */
   short elevation; /* meters above sea level */
   float latitude; /*  degrees North */
   float longitude; /* degrees East */
   short userCodes[4];                             /* 60 bytes to here */

   /* Cutler grabber */
   unsigned char cutlerCode; /* 0: Cutler Off
                              * 1: 4800 baud
                              * 2: 9600 baud
                              * 3: 19200 baud
                              * 4: 38400 baud    added 1.20-1.30
                              * 5: 57600 baud    added 1.20-1.30
                              */

   unsigned char minBatteryVoltage; /* minimum alarm battery voltage x 10 */

  /*  added 5 following parameters 3/3/95 for 1.20 MW & 1.30 K2 headers */
   unsigned char cutler_decimation;     /* Cutler grabber decimation factor */
                                                                        /* valid 0=1:1(raw), 1= 1:2, 2= 1:4, 3= 1:5, 4= 1:10, 5= 1:20 */
                                                                        /* restrictions on 1:4 @ 250 sps? */
   unsigned char cutler_irig_type;  /* 0:B   1:E (default)   2:H */
   uint32_t cutler_bitmap;         /* Digital Field Station bit map - channels to output */
   uint32_t channel_bitmap;        /* channels selected for acq storage */
   unsigned char cutler_protocol; /* 0:CRLF - USGS DFS, 1:KMI/Agbabian */

   char siteID[17];    /* added 5/21/96 */

   /* Network */
   unsigned char externalTrigger; /* either 1 (on) or 0 (off) */
   unsigned char networkFlag; /* BIT0 = 0 (Master) or = 1 (Slave) */

};

/*---------------------------------------------------*/

/* Miscellaneous read only parameters */

struct MISC_RO_PARMS {
   unsigned char a2dBits; /* A/D bits per sample; = A2DBITS */
   unsigned char sampleBytes; /* bytes per sample; = 3 */
   unsigned char restartSource; /* code of restart source:
                                   0 = unknown
                                   1 = power switch;
                                   2 = user command,
                                   3 = software watchdog,
                                   4 = DSP failure,
                                   5 = battery failure
                                   6 = memory error */
   char bytepad[3]; /* for expansion */

   unsigned short installedChan; /* number of channels in system
                                    = (# dsp brds) * (chan per brd) */
   unsigned short maxChannels; /* physical number of channels */
   unsigned short sysBlkVersion; /* sys block version * 100 */
   unsigned short bootBlkVersion; /* boot block version * 100 */
   unsigned short appBlkVersion;  /* application block version * 100 */
   unsigned short dspBlkVersion; /* DSP version * 100 */

   /* System Status */
   short batteryVoltage; /* voltage * 10; negative value indicates charging */

   unsigned short crc; /* 16-bit CRC of entire file, with this word set to 0xffff */
                                           /* NOTE: this parameter is not used at the moment.
              For integrity checking, the header and each frame are preceeded
              by a structure tag which contains a checksum */
   unsigned short flags; /* bit 0 = 1 if DSP system error */
   short temperature; /* degrees C x 10 */
   short wordpad[3]; /* for expansion */

   int32_t dwordpad[4]; /* for expansion */
};

/*---------------------------------------------------*/

struct TIMING_RW_PARMS
{
   unsigned char gpsTurnOnInterval; /* minutes between GPS update checking */
   unsigned char gpsMaxTurnOnTime; /* max time in minutes GPS tries to lock before giving up */
   unsigned char bytepad[6];
   short localOffset; /* hours ahead of UTC; if this parameter is non-zero
                                                 then all times are in UTC time. If this parameter
                                                 is zero, then all times are based on local time.
                                          */
   short wordpad[3];
   int32_t dwordpad[4];
};

/*---------------------------------------------------*/

struct TIMING_RO_PARMS
{
   unsigned char clockSource; /* 0 = RTC from cold start
                                 1 = keyboard
                                 2 = Sync w/ ext. ref. pulse
                                 3 = Internal GPS
                                 */

   unsigned char gpsStatus; /* Bit 0=1 if currently checking for presence
                                          of GPS board
                               Bit 1=1 if GPS board present
                               Bit 2=1 if error communicating with GPS
                               Bit 3=1 if failed to lock within an
                                          allotted time (gpsMaxTurnOnTime)
                               Bit 4=1 if not locked
                               Bit 5=1 when GPS power is ON
                               Bits 6,7=undefined
                             */

   unsigned char gpsSOH; /* Current state of health; same as Acutime SOH code */
   unsigned char bytepad[5]; /* for expansion */

   unsigned short gpsLockFailCount; /* # of times GPS failed to locked within gpsMaxTurnOnTime */
   unsigned short gpsUpdateRTCCount; /* # of times GPS actually updated the RTC */
   short acqDelay; /* time in msec between actual A/D conversion and DSP output */
   short gpsLatitude; /* latitude x 100 , degrees North */
   short gpsLongitude; /* longitude x 100, degrees East */
   short gpsAltitude; /* altitude in meters */
   unsigned short dacCount; /* dac counts */
   short wordpad; /* for expansion */
   short gpsLastDrift[2]; /* in msec.; e.g. 5 = RTC was 5 msec faster than GPS */
  /*
   * From KMI support: here's what these two components mean: (PNL 7/23/00)
   * short gpsLastDrift[0] = the last measured GPS drift in mS.
   * short gpsLastDrift[1] = the previously measured GPS drift in mS.
   */

   uint32_t gpsLastTurnOnTime[2]; /* time when GPS was last turned on */
   uint32_t gpsLastUpdateTime[2]; /* time of last RTC update */
   uint32_t gpsLastLockTime[2]; /* time of last GPS lock */
   int32_t dwordpad[4]; /* for expansion */
};

/*---------------------------------------------------*/

#define MODEM_INITCMD_LENGTH 64
#define MODEM_DIALPREFIX_LENGTH 16
#define MODEM_DIALSUFFIX_LENGTH 16
#define MODEM_HANGUPCMD_LENGTH 16
#define MODEM_AUTOANSWERON_LENGTH 16
#define MODEM_AUTOANSWEROFF_LENGTH 16
#define MODEM_PHONES 4
#define MODEM_PHONENUMBER_LENGTH 24

struct MODEM_RW_PARMS {
   /* reserved space for modems */
   char initCmd[MODEM_INITCMD_LENGTH]; /* initialization string */
   char dialingPrefix[MODEM_DIALPREFIX_LENGTH]; /* dialing prefix */
   char dialingSuffix[MODEM_DIALSUFFIX_LENGTH];
   char hangupCmd[MODEM_HANGUPCMD_LENGTH];
   char autoAnswerOnCmd[MODEM_AUTOANSWERON_LENGTH];
   char autoAnswerOffCmd[MODEM_AUTOANSWEROFF_LENGTH];
   char phoneNumber[MODEM_PHONES][MODEM_PHONENUMBER_LENGTH]; /* 4 phone numbers */

   unsigned char waitForConnection; /* secs */
   unsigned char pauseBetweenCalls; /* secs, 5/21/96 define '0' */
   unsigned char maxDialAttempts;

  /*  added 5/21/96 */
   char cellShare;  /* K2 only, =0 for 1 Hz output, =1 for cell phone */
                    /* @ MERGE 9/18/96 -- NOT USED */
   char cellOnTime;       /* in minutes */
   unsigned char cellWarmupTime;  /* in seconds */
   short cellStartTime[5];  /* minutes since midnight, 2->5, 6/10/96 */
   char bytepad[4];       /* 10->4, 6/10/96 */

   unsigned short flags; /* BIT0 = 1 to enable auto call out
                            BIT1 = 1 to call out on battery < 12 V
                            BIT2 = 1 to call out on battery charge failed
                            BIT3 = 1 to call out on extreme temperature
                            BIT4 = 1 to call out on event
                            BIT5 = 1 to call out on GPS lock failure
                         */
  char calloutMsg[46];   /*  added 5/21/96 */
};

/*---------------------------------------------------*/

/* Group read/write parms into one structure */

struct RW_K2_PARMS {
   struct MISC_RW_PARMS misc;
   struct TIMING_RW_PARMS timing;
   struct CHANNEL_RW_PARMS channel[MAX_K2_CHANNELS];
   struct STREAM_K2_RW_PARMS stream;
   struct MODEM_RW_PARMS modem;
};

/*---------------------------------------------------*/

/* Group read/write parms into one structure */
struct RW_MW_PARMS {
   struct MISC_RW_PARMS misc;
   struct TIMING_RW_PARMS timing;
   struct CHANNEL_RW_PARMS channel[MAX_MW_CHANNELS];
   struct STREAM_MW_RW_PARMS stream;
   struct MODEM_RW_PARMS modem;
};

/*---------------------------------------------------*/

#define FILE_DESCRIPTOR_SIZE 8 /* size of file descriptor */

/*   K2 */
/* Group read only parms into one structure */
struct RO_K2_PARMS {
   /* first 8 bytes is a common file descriptor */
   char id[3]; /* = 'KMI' to denote a Kinemetrics file */
   unsigned char instrumentCode; /* = '9' for K2 */
   unsigned short headerVersion; /* header version * 100 */
   unsigned short headerBytes; /* size of header following (includes RW_PARMS) */

   /* actual header follows */
   struct MISC_RO_PARMS misc;
   struct TIMING_RO_PARMS timing;
   struct CHANNEL_RO_PARMS channel[MAX_K2_CHANNELS];
   struct STREAM_RO_PARMS stream;
};

/*---------------------------------------------------*/

/* Group read only parms into one structure */
struct RO_MW_PARMS {
   /* first 8 bytes is a common file descriptor */
   char id[3]; /* = 'KMI' to denote a Kinemetrics file */
   unsigned char instrumentCode; /* = '9' for K2 */
   unsigned short headerVersion; /* header version * 100 */
   unsigned short headerBytes; /* size of header following (includes RW_PARMS) */

   /* actual header follows */
   struct MISC_RO_PARMS misc;
   struct TIMING_RO_PARMS timing;
   struct CHANNEL_RO_PARMS channel[MAX_MW_CHANNELS];
   struct STREAM_RO_PARMS stream;
};


/* This is what the K2 header looks like */
typedef struct {
   struct RO_K2_PARMS roParms;
   struct RW_K2_PARMS rwParms;
} K2_HEADER;

/* This is what the MtWhitney header looks like */
typedef struct {
   struct RO_MW_PARMS roParms;
   struct RW_MW_PARMS rwParms;
} MW_HEADER;

/*---------------------------------------------------*/
/*             K2 Event Frame Structure              */
/*---------------------------------------------------*/

#define FRAME_HEADER_SIZE 32 /* bytes */
#define MW_FRAME_HEADER_TYPE 4  /* 4 for 24 ch */
#define K2_FRAME_HEADER_TYPE 3  /* 3 for 16 ch */
#define TIMECODE_BYTES 13

typedef struct { /* 32 bytes */
   unsigned char frameType; /* like a version #, = FRAME_HEADER_TYPE */
   unsigned char instrumentCode; /* instrument code */
   unsigned short recorderID;
   unsigned short frameSize; /* # of frame bytes, includes 32 byte header */
/*   note: the uint32_t variable blockTime (defined below) is represented
           here by two unsigned short variables to circumvent a mis-alignment
           problem that otherwise occurs due to the fact that long variables
           on the SUN are aligned on longword boundaries.
   uint32_t blockTime;  block time */
   unsigned short blockTime1; /* block time */
   unsigned short blockTime2; /* block time */
   unsigned short channelBitMap; /* 1 bit for each ch. in use. Ch. 1 is bit 0 */
   unsigned short streamPar; /* see comments below */
   unsigned char frameStatus; /* see comments below */
   unsigned char frameStatus2; /* see comments below */
   unsigned short msec; /* 0..999 */
   unsigned char channelBitMap1; /* extended chan bit map, ch17-24 */
   unsigned char timeCode[TIMECODE_BYTES]; /* time code, see comment below */
} FRAME_HEADER;

/* FRAME_HEADER Description:
   frameStatus:
          Bits 0-3 = frame sequence number 0..9
          Bit 4    = stream triggered flag. Set if frame exceeds stream trigger level.
          Bit 5    = Compressed flag. Set if compressed frame.
          Bits 6-7 = Sample size. 1=16 bits (2 bytes), 2=24 bits (3 bytes)
                                                          3=32 bits (4 bytes)
                                 Samples are expressed as a signed integer of digital counts.
   frameStatus2:
          Bit 0 = set if issued the ADD SCAN command
          Bits 1-7: currently unused.

   streamPar:
          Bits 0-11 = Stream sampling rate, 1..4095
          Bits 12-15 = Stream number 0..15

   timeCode[]:
          Time code sampled every millisecond.
          Bits 4-7 of timeCode[0] = time code type (TBD)
          Bit 3 of timeCode[0] = time code bit sampled at first msec of frame (0=low, 1=high)
          Bit 2 of timeCode[0] = time code bit at 2nd msec.
          Bit 0 of timeCode[12] = last (100th msec) bit of time code
*/

/*----------------------------------------------------*/
/*                K2 TAG FRAME HEADER                 */
/*----------------------------------------------------*/
/* STRUCTURE TYPE CODES */
#define KFF_K2_HDR 1 /* K2 header */
#define KFF_K2_FRAME 2 /* K2 FRAME; frame data follows */
#define KFF_BYTE_ORDER 1/* 0=INTEL (LSB first) , 1 = MOTOROLA (MSB first) */
#define KFF_VERSION 1
#define KFF_SYNC_CHAR 'K'


/* TAG. Separates and identifies structures */
typedef struct {
   unsigned char sync; /* sync character 'K' */
   unsigned char byteOrder; /* = 0 for LSB first (INTEL), 1 for MSB first (MOTOROLA) */
   unsigned char version; /* File format version; KFF_VERSION */
   unsigned char instrumentType; /* instrument type code */
   uint32_t type; /* structure type code */
   unsigned short length; /* structure size in bytes */
   unsigned short dataLength; /* # of data bytes following the structure */
   unsigned short id; /* unique instrument id (e.g. serial number); used for multi-instrument files */
   unsigned short checksum; /* 16 bit checksum of structure + data */
} KFF_TAG;

#endif /* KWHDRS_EW_H */
