/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: rw_strongmotionII.h 8023 2019-06-19 12:20:58Z kevin $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.4  2003/06/10 15:55:57  lombard
 *     Modified rd_strongmotionII() prototype.
 *
 *     Revision 1.3  2001/08/02 23:22:50  davidk
 *     corrected a couple of comments.
 *
 *     Revision 1.2  2001/04/06 17:22:52  davidk
 *     Added formatted comments to the file.  Added additional doucmentation.
 *
 *     Revision 1.1  2001/04/06 17:16:06  davidk
 *     Initial revision
 *
 *
 *
 */

/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW API FORMATTED COMMENT
TYPE LIBRARY

LIBRARY EW_LIBSRC

SUB_LIBRARY RW_STRONGMOTION_II

LOCATION THIS_FILE

DESCRIPTION  Library for converting converting from an 
Earthworm TYPE_STRONGMOTION2 message to a structure and visa versa.
<br><br>
Each TYPE_STRONGMOTION2 message is intended to contain information for
one channel only, with optional event association information.
<br><br>
Field units with multiple channels will produce multiple 
TYPE_STRONGMOTION2 messages.
<br><br>
Timestamps are in seconds since 1970/01/01 00:00:00.00 UTC.
<br><br>
Data are in units of cgs. (centimeters, grams, seconds)
<br><br>
<b>WARNING!</b> All measurements in the Earthworm TYPE_STRONGMOTION2
message are assumed to be positive.  SM_NULL indicates that 
a measurement is missing, and any other negative value 
will result in unpredictable behavior from strong motion 
applications.  If you have sources that may produce signed
values for PGA, PGV, PGD, or RSA values, you must be sure to 
change these to absolute values before they are passed to this
library!
<br>
Written by Lynn Dietz,   January 2001
<br><br>

*************************************************
************************************************/


/* rw_strongmotionII.h
 *
 * Header file for the functions in rw_strongmotionII.c that
 * convert from a TYPE_STRONGMOTION2 message to a structure 
 * and visa versa.
 * 
 * Each TYPE_STRONGMOTION2 message is intended to contain information
 * one channel only, with optional event association information.
 * Field units with multiple channels will produce multiple 
 * TYPE_STRONGMOTION2 messages.
 *
 * Timestamps are in seconds since 1970/01/01 00:00:00.00 UTC.
 *
 * Data are in units of cgs.
 *
 * written by Lynn Dietz   January 2001
 */

#ifndef RW_STRONGMOTION_II_H
#define RW_STRONGMOTION_II_H

#include <earthworm.h>
/*
#include <earthworm_defs.h>
#include <earthworm_simple_funcs.h>
*/

#include <trace_buf.h>

/* Useful constants
 *******************/
/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE DEFINE 

LIBRARY EW_LIBSRC

SUB_LIBRARY RW_STRONGMOTION_II

LANGUAGE C

LOCATION THIS_FILE

CONSTANT_GROUP Scientific Constants

CONSTANT GRAVITY_CGS
VALUE 978.03
DESCRIPTION Gravity(g) in cm/sec/sec

*************************************************
************************************************/
#define GRAVITY_CGS 978.03    /* Gravity in cm/sec/sec */

/* Define maximum lengths for strings/arrays 
   in and Earthworm TYPE_STRONGMOTION2 message
 *********************************************/

/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE DEFINE 

LIBRARY EW_LIBSRC

SUB_LIBRARY RW_STRONGMOTION_II

LANGUAGE C

LOCATION THIS_FILE

CONSTANT_GROUP Strong Motion Message Constants

CONSTANT SM_MAX_RSA
VALUE 20
DESCRIPTION Maximum number of spectral values for
a given message.

CONSTANT SM_NULL
VALUE -1.0
DESCRIPTION NULL value for PGA, PGV, and PGD measurments.
If any of the peak measurments for a message are not
available, use SM_NULL in there place to indicate so.
All measurements in the Earthworm TYPE_STRONGMOTION2
message are assumed to be positive.  SM_NULL indicates that 
a measurement is missing, and any other negative value 
will result in unpredictable behavior from strong motion 
applications.

*************************************************
************************************************/
#define SM_MAX_RSA    25      /* max # spectral values for a given channel */
#define SM_NULL       -1.0      /* null value for period & RSA               */


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE DEFINE 

LIBRARY EW_LIBSRC

SUB_LIBRARY RW_STRONGMOTION_II

LANGUAGE C

LOCATION THIS_FILE

CONSTANT_GROUP Strong Motion Alternate Time Codes

CONSTANT SM_ALTCODE_NONE
VALUE 0
DESCRIPTION Code for no alternate time.

CONSTANT SM_ALTCODE_RECEIVING_MODULE
VALUE 1
DESCRIPTION Code for an alternate time derived by
a program module that receives a message from a
box in the field.

CONSTANT SM_ALTCODE_DATABASE
VALUE 2
DESCRIPTION Code for an alternate time derived by
the database or strong motion database insertion module.

CONSTANT SM_ALTCODE_AUTOMATED_REVIEWER
VALUE 3
DESCRIPTION Code for an alternate time derived by
a Human (who has hopefully reviewed the data).

*************************************************
************************************************/
/* Sources for the alternate time in the SM_DATA structure
 *********************************************************/
#define SM_ALTCODE_NONE                0
#define SM_ALTCODE_RECEIVING_MODULE    1
#define SM_ALTCODE_DATABASE            2
#define SM_ALTCODE_AUTOMATED_REVIEWER  3
#define SM_ALTCODE_HUMAN_REVIEWER      4


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF 

LIBRARY EW_LIBSRC

SUB_LIBRARY RW_STRONGMOTION_II

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF SM_INFO
TYPE_DEFINITION struct _SM_INFO
DESCRIPTION Structure that contains data for a 
single strong motion message.  This structure
is equivalent to the binary form of a strong
motion message(without a MSG_LOGO).

MEMBER sta
MEMBER_TYPE char[TRACE_STA_LEN]
MEMBER_DESCRIPTION NTS, Site code as per IRIS SEED.

MEMBER comp
MEMBER_TYPE char[TRACE_CHAN_LEN]
MEMBER_DESCRIPTION NTS, Component code as per IRIS SEED.

MEMBER net
MEMBER_TYPE char[TRACE_NET_LEN]
MEMBER_DESCRIPTION NTS, Network code as per IRIS SEED.

MEMBER loc
MEMBER_TYPE char[TRACE_LOC_LEN]
MEMBER_DESCRIPTION NTS, Location code as per IRIS SEED.

MEMBER qid
MEMBER_TYPE char[EVENTID_SIZE]
MEMBER_DESCRIPTION EventId that the message is associated
with.  This EventID is the one specified by qAuthor.

MEMBER qauthor
MEMBER_TYPE char[AUTHOR_FIELD_SIZE]
MEMBER_DESCRIPTION Author of an event with which this 
message is associated.  The author assigns the qid.

MEMBER t
MEMBER_TYPE double
MEMBER_DESCRIPTION The main timestamp for the message.  If the
message came from an SM box in the field, then this is the trigger
time reported by the box.  If the message came from a module processing
continuous telemetry, then this is the pick time or earlies of PGA, PGV,
or PGD time.   (seconds since 1970/01/01 00:00:00.00)

MEMBER talt
MEMBER_TYPE double
MEMBER_DESCRIPTION (Optional) Alternate timestamp for the message.  
This param is provided as a sanity check or correction of the 
main timestamp(t).  This timestamp could be assigned by an acquisition 
module, database loader, human, or other.  The author type of this 
timestamp is determined by altcode(see below).  
(seconds since 1970/01/01 00:00:00.00)

MEMBER altcode
MEMBER_TYPE int
MEMBER_DESCRIPTION (Required if talt is specified.) 
Code specifying the source of the alternate time field(talt).
See SM_ALTCODE_NONE for a list of recognized source codes.

MEMBER pga
MEMBER_TYPE double
MEMBER_DESCRIPTION The Peak Ground Acceleration(PGA) for this 
message.  (cm/s/s)

MEMBER tpga
MEMBER_TYPE double
MEMBER_DESCRIPTION Time of the pga.  (seconds since 1970)

MEMBER pgv
MEMBER_TYPE double
MEMBER_DESCRIPTION The Peak Ground Velocity(PGV) for this 
message. (cm/s)

MEMBER tpgv
MEMBER_TYPE double
MEMBER_DESCRIPTION Time of the pgv.  (seconds since 1970)

MEMBER pgd
MEMBER_TYPE double
MEMBER_DESCRIPTION The Peak Ground Displacement(PGD) for this 
message.  (cm)

MEMBER tpgd
MEMBER_TYPE double
MEMBER_DESCRIPTION Time of the pgd.  (seconds since 1970)

MEMBER nrsa
MEMBER_TYPE int
MEMBER_DESCRIPTION Number of response spectrum accel(RSA) pairs.
Valid value range is (0 - SM_MAX_RSA).

MEMBER pdrsa
MEMBER_TYPE double[SM_MAX_RSA]
MEMBER_DESCRIPTION Period(s) at which RSA values are given.

MEMBER rsa
MEMBER_TYPE double[SM_MAX_RSA]
MEMBER_DESCRIPTION RSA value for each given period. (cm/s/s)

MEMBER ai
MEMBER_TYPE double
MEMBER_DESCRIPTION The AI (Arias Intensity) value. (cm/s/s)

MEMBER tload
MEMBER_TYPE double
MEMBER_DESCRIPTION Time the message was first loaded into an
Earthworm DB.  (seconds since 1970)  NOTE:  This field is 
not part of ascii TYPE_STRONGMOTIONII msg, but may be filled 
when pulling data from DBMS.

MEMBER length
MEMBER_TYPE double
MEMBER_DESCRIPTION Length in seconds of data analyszed to
acquire the peak ground motions.  NOTE:  This field is 
not part of ascii TYPE_STRONGMOTIONII msg.

NOTE NTS when used above refers to (NULL Terminated String).

*************************************************
************************************************/
/* Structure to contain strongmotion data from one channel
   In the comments below, NTS = Null Terminated String
 *********************************************************/
typedef struct _SM_INFO {

/* fields supplied in TYPE_STRONGMOTION2 msg */
   char    sta[TRACE_STA_LEN];   /* REQUIRED: NTS, Site code as per IRIS SEED      */  
   char    comp[TRACE_CHAN_LEN]; /* REQUIRED: NTS, Component code as per IRIS SEED */  
   char    net[TRACE_NET_LEN];   /* REQUIRED: NTS, Network code as per IRIS SEED   */
   char    loc[TRACE_LOC_LEN];   /* OPTIONAL: NTS, Location code as per IRIS SEED  */
   char    qid[EVENTID_SIZE];    /* OPTIONAL: NTS, eventid data associates with    */
   char    qauthor[AUTHOR_FIELD_SIZE];  /* OPTIONAL: NTS, author of the eventid    */
                                 /*   (required if qid is given)                   */
   double  t;                    /* REQUIRED: time: trigger reported by SM box,    */
                                 /*   or pick time from continuous telemetry,      */
                                 /*   or earliest of tpga, tpgv, tpgd              */
                                 /*   (seconds since 1970/01/01 00:00:00.00)       */
   double  talt;                 /* OPTIONAL: alternate time, reported by          */
                                 /*   nobody, acq. software, analyst, etc.         */
   int     altcode;              /* OPTIONAL: code specifying the source of        */
                                 /*   the alternate time field                     */
   double  pga;                  /* REQUIRED: peak ground acceleration (cm/s/s)    */
   double  tpga;                 /* OPTIONAL: time of pga                          */
   double  pgv;                  /* REQUIRED: peak ground velocity (cm/s)          */
   double  tpgv;                 /* OPTIONAL: time of pgv                          */
   double  pgd;                  /* REQUIRED: peak ground displacement (cm)        */
   double  tpgd;                 /* OPTIONAL: time of pgd                          */
   int     nrsa;                 /* REQUIRED: # response spectrum accel (RSA) pairs*/
                                 /*   Valid range for nrsa is 0 to SM_MAX_RSA      */
   double  pdrsa[SM_MAX_RSA];    /* period (s) at which RSA values are given       */
   double  rsa[SM_MAX_RSA];      /* RSA value (cm/s/s) at given period             */ 
   double  trsa[SM_MAX_RSA];     /* time of rsa at given period                    */ 
   double  ai;                   /* OPTIONAL: Arias Intensity                      */

/* fields supplied by DBMS: */
   double  tload;                /* time data was loaded into DBMS - this field is */
                                 /*   not part of ascii TYPE_STRONGMOTIONII msg,   */
                                 /*   but may be filled when pulling data from DBMS*/
/* fields appended */
   double  length;               /* OPTIONAL: length of trace snippet analyzed     */
} SM_INFO;



#ifdef __cplusplus
extern "C" {
#endif
/* Function Prototypes
 *********************/

/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY EW_LIBSRC

SUB_LIBRARY RW_STRONGMOTION_II

LANGUAGE C

LOCATION THIS_FILE

FUNCTION rd_strongmotionII

SOURCE_LOCATION src/libsrc/util/rw_strongmotionII.c

RETURN_TYPE int 

RETURN_VALUE 1
RETURN_DESCRIPTION Success.

RETURN_VALUE 0
RETURN_DESCRIPTION no SM messages in buffer.

RETURN_VALUE -1
RETURN_DESCRIPTION Failure.  See logfile for details.


PARAMETER 1
PARAM_NAME msgP
PARAM_TYPE char **
PARAM_DESCRIPTION  address of the pointer to the buffer holding 
one or more strong motion messages.  The pointer may be advanced
to point to the next strong motion message if there may be another
one.

PARAMETER 2
PARAM_NAME sm
PARAM_TYPE SM_INFO * 
PARAM_DESCRIPTION  A pointer to an SM_INFO struct allocated
by the caller, where the function will result the SM_INFO
struct derived from the message.

PARAMETER 3
PARAM_NAME logErr
PARAM_TYPE int
PARAM_DESCRIPTION  Flag to log error messages.
Value 1: write error messages with logit calls
Value 0: do not log error messages


DESCRIPTION This function reads an ascii TYPE_STRONGMOTION2 
message and fills in a SM_INFO structure.  It returns 0 on success,
-1 on failure.

NOTE  Example of a TYPE_STRONGMOTION2 message.
<br><br><font size=-1 face=monospace>
SCNL: CMB.BHZ.BK.                                   <br>
TIME: 2001/02/25 02:37:00.000                       <br>
ALT:  2001/02/25 02:40:40.000 CODE: 1               <br>
PGA: 6.846210 TPGA: 2001/02/25 02:37:00.000         <br>
PGV: 0.140000 TPGV: 2001/02/25 02:37:00.000         <br>
PGD: 0.000000 TPGD: 2001/02/25 02:37:00.000         <br>
RSA: 3/0.30 4.415404/1.00 0.925639/3.00 0.297907    <br> 
QID: 41059467 014024003:UCB                         <br>   
</b></font>

*************************************************
************************************************/
int  rd_strongmotionII( char **msgP, SM_INFO *sm, int logErr );

/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY EW_LIBSRC

SUB_LIBRARY RW_STRONGMOTION_II

LANGUAGE C

LOCATION THIS_FILE

FUNCTION wr_strongmotionII

SOURCE_LOCATION src/libsrc/util/rw_strongmotionII.c

RETURN_TYPE int 

RETURN_VALUE 0
RETURN_DESCRIPTION Success.

RETURN_VALUE -1
RETURN_DESCRIPTION Failure.  This is caused by a 
buffer overflow, where the caller's buffer was too
small to hold the entire message generated by the 
function.  If the caller believes their buffer should
be adequately large, then one or more of the strings
in the SM_INFO struct may not be properly terminated.


PARAMETER 1
PARAM_NAME sm
PARAM_TYPE SM_INFO *
PARAM_DESCRIPTION  Pointer to an SM_INFO struct that the
function will read to get the information for the 
TYPE_STRONGMOTION2 message.

PARAMETER 2
PARAM_NAME buf
PARAM_TYPE char *
PARAM_DESCRIPTION  Pointer to a buffer allocated by the caller
where the function will write the TYPE_STRONGMOTION2 message.

PARAMETER 3
PARAM_NAME buflen
PARAM_TYPE int 
PARAM_DESCRIPTION  The length of the caller's buffer(buf) in bytes.


DESCRIPTION Reads a SM_INFO structure and writes an ascii 
TYPE_STRONGMOTION2 message (null terminated). It returns 
0 on success, and -1 on failure.  See rd_strongmotion for
an example of a TYPE_STRONGMOTION2 message.

*************************************************
************************************************/
int  wr_strongmotionII( SM_INFO *sm, char *buf, int buflen );

/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE FUNCTION_PROTOTYPE

LIBRARY EW_LIBSRC

SUB_LIBRARY RW_STRONGMOTION_II

LANGUAGE C

LOCATION THIS_FILE

FUNCTION log_strongmotionII

SOURCE_LOCATION src/libsrc/util/rw_strongmotionII.c

RETURN_TYPE void 

PARAMETER 1
PARAM_NAME sm
PARAM_TYPE SM_INFO *
PARAM_DESCRIPTION  Pointer to an SM_INFO struct whose 
contents the function will log to a file.


DESCRIPTION Writes the contents of a SM_INFO structure 
to an Earthworm log file.

*************************************************
************************************************/
void log_strongmotionII( SM_INFO *sm );

#ifdef __cplusplus
}
#endif
#endif /* #ifndef RW_STRONGMOTION_II_H */
