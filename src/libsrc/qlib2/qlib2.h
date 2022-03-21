/************************************************************************/
/*  Global definitions for qlib.					*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2013 The Regents of the University of California.
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research and non-profit purposes,
 * without fee, and without a written agreement is hereby granted,
 * provided that the above copyright notice, this paragraph and the
 * following three paragraphs appear in all copies.
 * 
 * Permission to incorporate this software into commercial products may
 * be obtained from the Office of Technology Licensing, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA  94704.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND
 * ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
 * CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
 * UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/*	$Id: qdefines.h 7457 2018-06-03 00:35:15Z baker $ 	*/

#ifndef	__qdefines_h
#define	__qdefines_h

#include <stdio.h>
#include <sys/param.h>

extern char *qlib2_version;

#define	QUOTE(x)	#x
#define	STRING(x)	QUOTE(x)

#define	IS_LEAP(yr)	( yr%400==0 || (yr%4==0 && yr%100!=0) )
#define	SEC_PER_MINUTE	60
#define	SEC_PER_HOUR	3600
#define	SEC_PER_DAY	86400
#define SEC_PER_YEAR(yr) sec_per_year(yr)
#define	TICKS_PER_SEC	10000
#define	TICKS_PER_MSEC	(TICKS_PER_SEC/1000)
#define USECS_PER_SEC	1000000
#define	USECS_PER_MSEC	(USECS_PER_SEC/1000)
#define	USECS_PER_TICK	(USECS_PER_SEC/TICKS_PER_SEC)

#define	    DAYS_PER_YEAR(yr)	    \
			(365 + ((yr%4==0)?1:0) + \
			 ((yr%100==0)?-1:0) + \
			 ((yr%400==0)?1:0))
#define	BIT(a,i)	((a >> i) & 1)
#define	IHUGE		(65536*32767)
#define	DIHUGE		(140737488355328.)

#ifndef	MAX
#define MAX(a,b)	((a >= b) ? a : b)
#endif
#ifndef	MIN
#define MIN(a,b)	((a <= b) ? a : b)
#endif

#define	UNKNOWN_STREAM	"UNK"
#define	UNKNOWN_COMP	"U"

#define	DATA_HDR_IND_D	'D'
#define	DATA_HDR_IND_R	'R'
#define	DATA_HDR_IND_Q	'Q'
#define	DATA_HDR_IND_M	'M'
#define	VOL_HDR_IND	'V'
#define	DATA_HDR_IND	DATA_HDR_IND_D

#define	UNK_HDR_TYPE	0
#define	QDA_HDR_TYPE	1
#define	SDR_HDR_TYPE	2
#define SDR_VOL_HDR_TYPE 3
#define	DRM_HDR_TYPE	4

#define	JULIAN_FMT	0
#define	JULIAN_FMT_1	1
#define	MONTH_FMT	2
#define	MONTH_FMT_1	3
#define	JULIANC_FMT	4
#define	JULIANC_FMT_1	5
#define	MONTHS_FMT	6
#define	MONTHS_FMT_1	7
#define	ISO_CALENDAR	8
#define	ISO_ORDINAL	9
#define	ISO_CALENDAR_Z	10
#define	ISO_ORDINAL_Z	11

#define	TRUE		1
#define	FALSE		0

#if defined(__linux__) || defined(__LINUX__) || defined(__MACH__) || defined( __ARM_ARCH_4T__)
#include <math.h>
#else
/* Library definitions that are not always in math.h	*/
#ifndef M_LN2
#define M_LN2	    0.69314718055994530942
#endif
#define	log2(x)	    ((double)(log(x)/M_LN2))
#define	exp2(x)	    pow(2.,x)
#endif

#define	QLIB2_CLASSIC	(qlib2_op_mode == 0)
#define	QLIB2_NOEXIT	(qlib2_op_mode == 1)

#define	MS_ERROR		-2
#define	QLIB2_MALLOC_ERROR	-3
#define	QLIB2_TIME_ERROR	-4
#define	MS_COMPRESS_ERROR	-5

#endif

/************************************************************************/
/*  Data types for SEED data records.					*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2000 The Regents of the University of California.
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research and non-profit purposes,
 * without fee, and without a written agreement is hereby granted,
 * provided that the above copyright notice, this paragraph and the
 * following three paragraphs appear in all copies.
 * 
 * Permission to incorporate this software into commercial products may
 * be obtained from the Office of Technology Licensing, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA  94704.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND
 * ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
 * CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
 * UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/*	$Id: msdatatypes.h 4229 2011-05-25 22:33:01Z lombard $ 	*/

#ifndef	SEED_BIG_ENDIAN

/*  Define UNKNOWN datatype.		*/
#define	UNKNOWN_DATATYPE		0

/*  General datatype codes.		*/
#define	INT_16				1
#define	INT_24				2
#define	INT_32				3
#define	IEEE_FP_SP			4
#define IEEE_FP_DP			5

/*  FDSN Network codes.			*/
#define	STEIM1				10
#define	STEIM2				11
#define	GEOSCOPE_MULTIPLEX_24		12
#define	GEOSCOPE_MULTIPLEX_16_GR_3	13
#define	GEOSCOPE_MULTIPLEX_16_GR_4	14
#define	USNN				15
#define	CDSN				16
#define	GRAEFENBERG_16			17
#define	IPG_STRASBOURG_16		18

/*  Older Network codes.		*/
#define	SRO				30
#define	HGLP				31
#define	DWWSSN_GR			32
#define	RSTN_16_GR			33

/*  Data format.   	 		*/
#define DATA_FMT_INT			1
#define DATA_FMT_FLOAT			2
#define DATA_FMT_DOUBLE			3

/*  Definitions for blockette 1000	*/
#define SEED_LITTLE_ENDIAN		0
#define	SEED_BIG_ENDIAN			1

#define	IS_STEIM_COMP(n)    ((n==STEIM1 || n==STEIM2) ? 1 : 0)

#endif

/************************************************************************/
/*	Header file for qlib time structures.				*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2000 The Regents of the University of California.
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research and non-profit purposes,
 * without fee, and without a written agreement is hereby granted,
 * provided that the above copyright notice, this paragraph and the
 * following three paragraphs appear in all copies.
 * 
 * Permission to incorporate this software into commercial products may
 * be obtained from the Office of Technology Licensing, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA  94704.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND
 * ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
 * CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
 * UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/*	$Id: timedef.h 2498 2006-11-16 15:23:49Z ilya $ 	*/

#ifndef	__timedef_h
#define	__timedef_h

#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

/*	Time structures.					*/

typedef struct _ext_time {
    int		year;		/* Year.			*/
    int		doy;		/* Day of year (1-366)		*/
    int		month;		/* Month (1-12)			*/
    int		day;		/* Day of month (1-31)		*/
    int		hour;		/* Hour (0-23)			*/
    int		minute;		/* Minute (0-59)		*/
    int		second;		/* Second (0-60 (leap))		*/
    int		usec;		/* Microseconds (0-999999)	*/
} EXT_TIME;

typedef struct	_int_time {
    int		year;		/* Year.			*/
    int		second;		/* Seconds in year (0-...)	*/
    int		usec;		/* Microseconds (0-999999)	*/
} INT_TIME;

#endif

/************************************************************************/
/*  Steim compression information and datatypes.			*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2000 The Regents of the University of California.
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research and non-profit purposes,
 * without fee, and without a written agreement is hereby granted,
 * provided that the above copyright notice, this paragraph and the
 * following three paragraphs appear in all copies.
 * 
 * Permission to incorporate this software into commercial products may
 * be obtained from the Office of Technology Licensing, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA  94704.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND
 * ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
 * CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
 * UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/*	$Id: qsteim.h 4229 2011-05-25 22:33:01Z lombard $ 	*/

#ifndef	__qsteim_h
#define	__qsteim_h

#define	STEIM1_SPECIAL_MASK	0
#define	STEIM1_BYTE_MASK	1
#define	STEIM1_HALFWORD_MASK	2
#define	STEIM1_FULLWORD_MASK	3

#define	STEIM2_SPECIAL_MASK	0
#define	STEIM2_BYTE_MASK	1
#define	STEIM2_123_MASK		2
#define	STEIM2_567_MASK		3

typedef union u_diff {			/* union for steim 1 objects.	*/
    signed char	    byte[4];		/* 4 1-byte differences.	*/
    short	    hw[2];		/* 2 halfword differences.	*/
    int		    fw;			/* 1 fullword difference.	*/
} U_DIFF;

typedef struct frame {			/* frame in a seed data record.	*/
    unsigned int    ctrl;		/* control word for frame.	*/
    U_DIFF	    w[15];		/* compressed data.		*/
} FRAME;

typedef struct seed_data_frames {	/* seed data frames.		*/
    FRAME f[1];				/* data record header frames.	*/
} SDF;

#endif

/************************************************************************/
/*  Field definitions used in SEED data record headers.			*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2000 The Regents of the University of California.
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research and non-profit purposes,
 * without fee, and without a written agreement is hereby granted,
 * provided that the above copyright notice, this paragraph and the
 * following three paragraphs appear in all copies.
 * 
 * Permission to incorporate this software into commercial products may
 * be obtained from the Office of Technology Licensing, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA  94704.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND
 * ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
 * CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
 * UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/*	$Id: sdr.h 6803 2016-09-09 06:06:39Z et $ 	*/

#ifndef	__sdr_h
#define __sdr_h

#define	SDR_HDR_SIZE	64		/* SDR hdr size in file.	*/

#define	ACTIVITY_CALIB_PRESENT	0x1
#define	ACTIVITY_TIME_GAP	0x2	/* for backwards compatibility.	*/
#define	ACTIVITY_TIME_CORR_APPLIED	02
#define	ACTIVITY_BEGINNING_OF_EVENT	0x4
#define	ACTIVITY_END_OF_EVENT	0x8
#define	ACTIVITY_POS_LEAP_SECOND    0x10
#define	ACTIVITY_NEG_LEAP_SECOND    0x20
#define	ACTIVITY_EVENT_IN_PROGRESS  0x40

#define	IO_PARITY_ERROR		0x2
#define	IO_LONG_RECORD		0x2
#define	IO_SHORT_RECORD		0x4

#define	QUALITY_SATURATION	0x1
#define	QUALITY_CLIPPING	0x2
#define	QUALITY_SPIKES		0x4
#define	QUALITY_GLITHES		0x8
#define	QUALITY_MISSING		0x10
#define	QUALITY_TELEMETRY_ERROR	0x20
#define	QUALITY_FILTER_CHARGE	0x40
#define	QUALITY_QUESTIONABLE_TIMETAG	0x80

#define	MSHEAR_TIMETAG_FLAG	0x01

typedef	char		SEED_BYTE;	/* signed byte			*/
typedef	unsigned char	SEED_UBYTE;	/* unsigned byte		*/
typedef	short		SEED_WORD;	/* 16 bit signed		*/
typedef	unsigned short	SEED_UWORD;	/* 16 bit unsigned		*/
typedef	int		SEED_LONG;	/* 32 bit signed		*/
typedef	unsigned int	SEED_ULONG;	/* 32 bit unsigned		*/
typedef	char		SEED_CHAR;	/* 7 bit character, high bit 0	*/
typedef unsigned char	SEED_UCHAR;	/* 8 bit character, unsigned.	*/
typedef signed char	SEED_SCHAR;	/* 8 bit character, signed.	*/
typedef	float		SEED_FLOAT;	/* IEEE floating point		*/

typedef struct _sdr_time {
    SEED_UWORD	year;		/* Year					*/
    SEED_UWORD	day;		/* Day of year (1-366)			*/
    SEED_UBYTE	hour;		/* Hour (0-23)				*/
    SEED_UBYTE	minute;		/* Minute (0-59)			*/
    SEED_UBYTE	second;		/* Second (0-60 (leap))			*/
    SEED_UBYTE	pad;		/* Padding				*/
    SEED_UWORD	ticks;		/* 1/10 millisecond (0-9999)		*/
} SDR_TIME;

#define	SDR_SEQ_LEN		6
#define	SDR_STATION_LEN		5
#define	SDR_LOCATION_LEN	2
#define	SDR_CHANNEL_LEN		3
#define	SDR_NETWORK_LEN		2

/************************************************************************/
/*  SEED Fixed Data Record Header (SDR)					*/
/************************************************************************/
typedef struct _sdr_hdr {			/* byte offset  */
    SEED_CHAR	seq_no[SDR_SEQ_LEN];		/*	0   */
    SEED_CHAR	data_hdr_ind;			/*	6   */
    SEED_CHAR	space_1;			/*	7   */
    SEED_CHAR	station_id[SDR_STATION_LEN];	/*	8   */
    SEED_CHAR	location_id[SDR_LOCATION_LEN];	/*	13  */
    SEED_CHAR	channel_id[SDR_CHANNEL_LEN];	/*	15  */
    SEED_CHAR	network_id[SDR_NETWORK_LEN];	/*	18  */
    SDR_TIME	time;				/*	20  */
    SEED_UWORD	num_samples;			/*	30  */
    SEED_WORD	sample_rate_factor;		/*	32  */
    SEED_WORD	sample_rate_mult;		/*	34  */
    SEED_UBYTE	activity_flags;			/*	36  */
    SEED_UBYTE	io_flags;			/*	37  */
    SEED_UBYTE	data_quality_flags;		/*	38  */
    SEED_UBYTE	num_blockettes;			/*	39  */
    SEED_LONG	num_ticks_correction;		/*	40  */
    SEED_UWORD	first_data;			/*	44  */
    SEED_UWORD	first_blockette;		/*	46  */
} SDR_HDR;

/************************************************************************/
/*  Blockette Definitions.						*/
/************************************************************************/
typedef struct _blockette_hdr {		/*  Common binary blockette hdr.*/
    SEED_UWORD	type;			/*  binary blockette number.	*/
    SEED_UWORD	next;			/*  byte offset from sdr to next*/
} BLOCKETTE_HDR;			/*  blockette for this record.	*/

typedef struct _blockette_100 {		/*  Sample Rate Blockette.	*/
    BLOCKETTE_HDR   hdr;		/*  binary blockette header.	*/
    SEED_FLOAT	actual_rate;
    SEED_CHAR	flags;
    SEED_CHAR	reserved[3];
} BLOCKETTE_100;

/*  Quanterra Threshold Detector. Comments refer to Quanterra usage.	*/
typedef struct _blockette_200 {		/*  Generic Event Detection blockette.*/
    BLOCKETTE_HDR   hdr;		/*  binary blockette header.	*/
    SEED_FLOAT	signal_amplitude;	/*  Amp that caused detection.	*/
    SEED_FLOAT	signal_period;		/*  Not used by Quanterra.	*/
    SEED_FLOAT	background_estimate;	/*  Limit that was exceeded.	*/
    SEED_UBYTE	detection_flags;	/*  Not used by Quanterra.	*/
    SEED_UBYTE	reserved;		/*  Not used.			*/
    SDR_TIME	time;			/*  Onset time of detector.	*/
    /* Quanterra additions to SEED version 2.3 blockette.		*/
    SEED_CHAR	detector_name[24];	/*  Quanterra detector name.	*/
} BLOCKETTE_200;
#define	BLOCKETTE_200_STD_SIZE	(sizeof(BLOCKETTE_200)-24)

typedef struct _blockette_201 {		/*  Murdock Event Detection blockette.*/
    BLOCKETTE_HDR   hdr;		/*  binary blockette header.	*/
    SEED_FLOAT	signal_amplitude;	/*  Amplitude of signal (counts)*/
    SEED_FLOAT	signal_period;		/*  Period of signal in seconds.*/
    SEED_FLOAT	background_estimate;	/*  Background estimates (counts)*/
    SEED_UBYTE	detection_flags;	/*  bit 0: 1=dilitational,0=compression */
    SEED_UBYTE	reserved;		/*  Unused - set to 0.		*/
    SDR_TIME	time;			/*  Signal onset time.		*/
    SEED_UBYTE	signal_to_noise[6];	/*  sn ratios - only use first 5*/
    SEED_UBYTE	loopback_value;		/*  Loopback value (0, 1, or 2).*/
    SEED_UBYTE	pick_algorithm;		/*  Pick algorithm - (0 or 1).	*/
    /* Quanterra additions to SEED version 2.3 blockette.		*/
    SEED_CHAR	detector_name[24];	/*  Quanterra detector name.	*/
} BLOCKETTE_201;
#define	BLOCKETTE_201_STD_SIZE	(sizeof(BLOCKETTE_201)-24)

typedef struct _blockette_300 {		/*  Step Calibration blockette.	*/
    BLOCKETTE_HDR   hdr;		/*  binary blockette header.	*/
    SDR_TIME	time;
    SEED_UBYTE	num_step_calibrations;
    SEED_UBYTE	calibration_flags;
    SEED_ULONG	step_duration;
    SEED_ULONG	interval_duration;
    SEED_FLOAT	calibration_amplitude;
    SEED_CHAR	calibration_channel[3];
    SEED_UBYTE	reserved;
    /* Quanterra additions to SEED version 2.3 blockette.		*/
    SEED_FLOAT	reference_amplitude;
    SEED_CHAR	coupling[12];
    SEED_CHAR	rolloff[12];
} BLOCKETTE_300;
#define	BLOCKETTE_300_STD_SIZE	(sizeof(BLOCKETTE_300)-8)

typedef struct _blockette_310 {		/*  Sine Calibration Blockette.	*/
    BLOCKETTE_HDR   hdr;		/*  binary blockette header.	*/
    SDR_TIME	time;    
    SEED_UBYTE	reserved_1;
    SEED_UBYTE	calibration_flags;
    SEED_ULONG	calibration_duration;
    SEED_FLOAT	calibration_period;
    SEED_FLOAT	calibration_amplitude;
    SEED_CHAR	calibration_channel[3];
    SEED_UBYTE	reserved;
    /* Quanterra additions to SEED version 2.3 blockette.		*/
    SEED_FLOAT	reference_amplitude;
    SEED_CHAR	coupling[12];
    SEED_CHAR	rolloff[12];
} BLOCKETTE_310;
#define	BLOCKETTE_310_STD_SIZE	(sizeof(BLOCKETTE_310)-8)

typedef struct _blockette_320 {		/*  Pseudo-random Calibration blockette.*/
    BLOCKETTE_HDR   hdr;		/*  binary blockette header.	*/
    SDR_TIME	time;    
    SEED_UBYTE	reserved_1;
    SEED_UBYTE	calibration_flags;
    SEED_FLOAT	calibration_duration;
    SEED_FLOAT	calibration_amplitude;
    SEED_CHAR	calibration_channel[3];
    SEED_UBYTE	reserved;
    /* Quanterra additions to SEED version 2.3 blockette.		*/
    SEED_FLOAT	reference_amplitude;
    SEED_CHAR	coupling[12];
    SEED_CHAR	rolloff[12];
    SEED_CHAR	noise_type[8];
} BLOCKETTE_320;
#define	BLOCKETTE_320_STD_SIZE	(sizeof(BLOCKETTE_320)-8)

typedef struct _blockette_390 {		/*  Generic Calibration blockette*/
    BLOCKETTE_HDR   hdr;		/*  binary blockette header.	*/
    SDR_TIME	time;    
    SEED_UBYTE	reserved_1;
    SEED_UBYTE	calibration_flags;
    SEED_FLOAT	calibration_duration;
    SEED_FLOAT	calibration_amplitude;
    SEED_CHAR	calibration_channel[3];
    SEED_UBYTE	reserved;
} BLOCKETTE_390;

typedef struct _blockette_395 {		/*  Calibration Abort blockette.*/
    BLOCKETTE_HDR   hdr;		/*  binary blockette header.	*/
    SDR_TIME	time;    
    SEED_UWORD	reserved;
} BLOCKETTE_395;

typedef struct _blockette_400 {		/*  Beam blockette.		*/
    BLOCKETTE_HDR   hdr;		/*  binary blockette header.	*/
    SEED_FLOAT	azimuth;
    SEED_FLOAT	slowness;
    SEED_UWORD	config;
    SEED_UWORD	reserved;
} BLOCKETTE_400;

typedef struct _blockette_405 {		/*  Beam Delay blockette.	*/
    BLOCKETTE_HDR   hdr;		/*  binary blockette header.	*/
    SEED_UWORD	delay;
} BLOCKETTE_405;

typedef struct _blockette_500 {		/*  Timing blockette (Quanterra). */
    BLOCKETTE_HDR   hdr;		/*  binary blockette header.	*/
    SEED_FLOAT	vco_correction;		/*  VCO correction.		*/
    SDR_TIME	time;			/*  Time of timing exception.	*/
    SEED_SCHAR	usec99;			/*  time extension to microsec.	*/
    SEED_SCHAR	reception_quality;	/*  Clock Reception quality.	*/
    SEED_LONG	count;			/*  Count (for exception type).	*/
    SEED_CHAR	exception_type[16];	/*  Type of timing exception.	*/
    SEED_CHAR	clock_model[32];	/*  Type of clock in use.	*/
    SEED_CHAR	clock_status[128];	/*  Clock status string.	*/
} BLOCKETTE_500;

typedef struct _blockette_1000 {	/*  Data format blockette.	*/
    BLOCKETTE_HDR   hdr;		/*  binary blockette header.	*/
    SEED_CHAR	format;			/*  data format.		*/
    SEED_CHAR	word_order;		/*  word order.			*/
    SEED_CHAR	data_rec_len;		/*  record length in 2**n.	*/
    SEED_CHAR	reserved;		/*  unused.			*/
} BLOCKETTE_1000;

typedef struct _blockette_1001 {	/*  Data extention blockette.	*/
    BLOCKETTE_HDR   hdr;		/*  binary blockette header.	*/
    SEED_SCHAR	clock_quality;		/*  clock quality.		*/
    SEED_SCHAR	usec99;			/*  time extension to microsec.	*/
    SEED_UCHAR	flags;			/*  flags.			*/
    SEED_SCHAR	frame_count;		/*  # of 64-byte steim frames.	*/
} BLOCKETTE_1001;

/* Variable length blockettes.						*/
typedef struct _blockette_2000 {	/*  Opaque data blockette.	*/
    BLOCKETTE_HDR   hdr;		/*  binary blockette header.	*/
    SEED_WORD	    blockette_len;	/*  total blockete length.	*/
    SEED_WORD	    data_offset;	/*  offset to opaque data.	*/
    SEED_LONG	    record_num;		/*  record number.		*/
    SEED_UCHAR	    word_order;		/*  opaque data word order.	*/
    SEED_UCHAR	    data_flags;		/*  opaque data flags.		*/
    SEED_UCHAR	    num_hdr_strings;	/*  number of opaque hdr strings*/
} BLOCKETTE_2000;

typedef struct _blockette_unknown {	/*  Unknown blockette.		*/
    BLOCKETTE_HDR   hdr;		/*  binary blockette header.	*/
    SEED_CHAR	body[128];		/*  body, suitably large.	*/
} BLOCKETTE_UNKNOWN;

#endif

/************************************************************************/
/*  Field definitions used in QDA data record headers.			*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2000 The Regents of the University of California.
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research and non-profit purposes,
 * without fee, and without a written agreement is hereby granted,
 * provided that the above copyright notice, this paragraph and the
 * following three paragraphs appear in all copies.
 * 
 * Permission to incorporate this software into commercial products may
 * be obtained from the Office of Technology Licensing, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA  94704.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND
 * ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
 * CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
 * UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/*	$Id: qda.h 2498 2006-11-16 15:23:49Z ilya $ 	*/

#ifndef	__qda_h
#define	__qda_h

#define	QDA_HDR_SIZE	64		    /* QDA file header size.	*/

#define	SOH_INACCURATE		    0x80    /* inaccurate time tagging, in SOH  */
#define	SOH_GAP			    0x40    /* time gap detected, in SOH	*/
#define	SOH_EVENT_IN_PROGRESS	    0x20    /* record contains event data	*/
#define	SOH_BEGINNING_OF_EVENT	    0x10    /* record is first of event sequence	*/
#define	SOH_CAL_IN_PROGRESS	    0x08    /* record contains calibration data	*/
#define	SOH_EXTRANEOUS_TIMEMARKS    0x04    /* too many time marks received during this record*/
#define	SOH_EXTERNAL_TIMEMARK_TAG   0x02    /* record time-tagged at a mark    */
#define	SOH_RECEPTION_GOOD	    0x01    /* time reception is adequate   */

typedef struct _qda_time {
    char	year;
    char	month;
    char	day;
    char	hour;
    char	minute;
    char	second;
} QDA_TIME;

/*  Fixed QDA header	*/
typedef struct qda_hdr {	    /*	byte offset  */
    int		header_flag;		    /*  0   */
    char	frame_type;		    /*  4   */
    char	component;		    /*  5   */
    char	stream;			    /*	6   */
    char	soh;			    /*	7   */
    char	station_id[4];		    /*	8   */
    short	millisecond;		    /*	12  */
    short	time_mark;		    /*	14  */
    int		samp_1;			    /*	16  */
    short	clock_corr;		    /*	20  */
    short	num_samples;		    /*	22  */
    char	sample_rate;		    /*	24  */
    char	reserved;		    /*	25  */
    QDA_TIME	time;			    /*	36  */
    int		seq_no;			    /*	32  */
} QDA_HDR;

#endif

/************************************************************************/
/* Adebahr DRM seismo.h file.						*/
/************************************************************************/

#include    "drm_seismo.h"

/************************************************************************/
/*  data_hdr structure used to store parsed Mini-SEED header and	*/
/*  blockettes in an accessible manner.					*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2002 The Regents of the University of California.
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research and non-profit purposes,
 * without fee, and without a written agreement is hereby granted,
 * provided that the above copyright notice, this paragraph and the
 * following three paragraphs appear in all copies.
 * 
 * Permission to incorporate this software into commercial products may
 * be obtained from the Office of Technology Licensing, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA  94704.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND
 * ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
 * CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
 * UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/*	$Id: data_hdr.h 2498 2006-11-16 15:23:49Z ilya $ 	*/

#ifndef	__data_hdr_h
#define	__data_hdr_h

#define	DH_STATION_LEN	5
#define	DH_CHANNEL_LEN	3
#define	DH_LOCATION_LEN	2
#define DH_NETWORK_LEN	2


/* Linked list structure for storing blockettes.			*/
typedef struct _bs {			/* blockette structure.		*/
    char	*pb;			/* ptr to actual blockette.	*/
    unsigned short int type;		/* blockette number.		*/
    unsigned short int len;		/* length of blockette in bytes.*/
    unsigned short int wordorder;	/* wordorder of blockette.	*/
    struct _bs	*next;			/* ptr to next blockette struct.*/
} BS;

/* Data header structure, containing SEED Fixed Data Header info	*/
/* as well as other useful info.					*/
typedef struct	data_hdr {
    int		seq_no;			/* sequence number		*/
    char	station_id[DH_STATION_LEN+1];	/* station name		*/
    char	location_id[DH_LOCATION_LEN+1];	/* location id		*/
    char	channel_id[DH_CHANNEL_LEN+1];	/* channel name		*/
    char	network_id[DH_NETWORK_LEN+1];	/* network id		*/
    INT_TIME	begtime;		/* begin time with corrections	*/
    INT_TIME	endtime;		/* end time of packet		*/
    INT_TIME	hdrtime;		/* begin time in hdr		*/
    int		num_samples;		/* number of samples		*/
    int		num_data_frames;	/* number of data frames	*/
    int		sample_rate;		/* sample rate			*/
    int		sample_rate_mult;	/* sample rate multiplier.	*/
    int		num_blockettes;		/* # of blockettes (0)		*/
    int		num_ticks_correction;	/* time correction in ticks	*/
    int		first_data;		/* offset to first data		*/
    int		first_blockette;	/* offset of first blockette	*/
    BS		*pblockettes;		/* ptr to blockette structures	*/
    int		data_type;		/* data_type (for logs or data)	*/
    int		blksize;		/* blocksize of record (bytes).	*/
    unsigned char activity_flags;	/* activity flags		*/
    unsigned char io_flags;		/* i/o flags			*/
    unsigned char data_quality_flags;	/* data quality flags		*/
    signed char hdr_wordorder;		/* wordorder of header.		*/
    signed char data_wordorder;		/* wordorder of data.		*/
    char	record_type;		/* record type (D,R,Q or V)	*/
    char	cextra[2];		/* future expansion.		*/
    int		x0;			/* first value (STEIM compress)	*/
    int		xn;			/* last value (STEIM compress)	*/
    int		xm1;			/* last value in prev record	*/
    int		xm2;			/* next to last val in prev rec	*/
    float	rate_spsec;		/* blockette 100 sample rate	*/
} DATA_HDR;

/* Attribute structure for a specific data_hdr and blksize.		*/
typedef struct _ms_attr{
    int sample_size;			/* # bytes for sample (0=NA)	*/
    int	alignment;			/* alignment requirement (1 min)*/
    int nframes;			/* # of frame in use (0=NA)	*/
    int framelimit;			/* max # of frames  (0=NA)	*/
    int nbytes;				/* # of bytes in use		*/
    int bytelimit;			/* max # of bytes		*/
} MS_ATTR;

#endif

/************************************************************************/
/*  Time routines for Quanterra data processing.			*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2018 The Regents of the University of California.
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research and non-profit purposes,
 * without fee, and without a written agreement is hereby granted,
 * provided that the above copyright notice, this paragraph and the
 * following three paragraphs appear in all copies.
 * 
 * Permission to incorporate this software into commercial products may
 * be obtained from the Office of Technology Licensing, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA  94704.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND
 * ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
 * CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
 * UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/*	$Id: qtime.h 7457 2018-06-03 00:35:15Z baker $ 	*/

#ifndef	__qtime_h
#define	__qtime_h


#ifdef	__cplusplus
extern "C" {
#endif

extern int init_leap_second_table ();

extern int is_leap_second
   (INT_TIME it);		/* INT_TIME structure			*/

extern int is_leap_second_te
   (double tepoch);		/* True epoch time			*/

extern int is_leap_second_ne
   (double nepoch);		/* Nominal epoch time			*/

extern void dy_to_mdy 
   (int		doy,		/* day of year (input).			*/
    int		year,		/* year (input).			*/
    int		*month,		/* month of year (returned).		*/
    int		*mday);		/* day of month (returned).		*/

extern int mdy_to_doy
   (int		month,		/* month of year (input).		*/
    int		day,		/* day of month (input).		*/
    int		year);		/* year (input).			*/

extern EXT_TIME normalize_ext 
   (EXT_TIME	et);		/* EXT_TIME to normalize.		*/

extern INT_TIME normalize_time
   (INT_TIME	it);		/* INT_TIME to normalize.   		*/

extern EXT_TIME int_to_ext
   (INT_TIME	it);		/* INT_TIME to convert to EXT_TIME.	*/

extern INT_TIME ext_to_int
   (EXT_TIME	et);		/* EXT_TIME to convert to INT_TIME.	*/

extern double int_to_tepoch
   (INT_TIME	it);		/* INT_TIME to convert to True epoch.	*/

extern INT_TIME tepoch_to_int
   (double	tepoch);	/* True epoch to convert to INT_TIME.	*/

extern double int_to_nepoch
   (INT_TIME	it);		/* INT_TIME to convert to True epoch.	*/

extern INT_TIME nepoch_to_int
   (double	nepoch);	/* Nominal epoch to convert to INT_TIME.*/

extern double nepoch_to_tepoch
   (double	nepoch);	/* Nominal epoch to convert to true.	*/

extern double tepoch_to_nepoch
   (double	tepoch);	/* True epoch to convert to nominal.	*/

extern int sec_per_year
   (int		year);		/* year (input).			*/

extern int nsec_per_year
   (int		year);		/* year (input).			*/

extern int missing_time
   (INT_TIME	time);		/* INT_TIME structure.			*/

extern INT_TIME add_time
   (INT_TIME	it,		/* INT_TIME initial structure.		*/
    int		seconds,	/* number of seconds to add.		*/
    int		usecs);		/* number of usecs to add.		*/

extern INT_TIME add_dtime
   (INT_TIME	it,		/* INT_TIME initial structure.		*/
    double	dusec);		/* number of usecs to add.		*/

extern double sps_rate 
   (int rate,			/* sample rate in qlib convention.	*/
    int rate_mult);		/* sampe_rate_mult in qlib convention.	*/

extern void time_interval2
   (int		n,		/* number of samples.			*/
    int		rate,		/* input rate in qlib convention.	*/
    int		rate_mult,	/* sample rate_mult in qlib convention.	*/
    int		*seconds,	/* number of seconds in time interval.	*/
    int		*usecs);	/* number of usecs in time interval.	*/

extern double dsamples_in_time2
   (int		rate,		/* sample rate in qlib convention.	*/
    int		rate_mult,	/* sample rate_mult in qlib convention.	*/
    double	dusecs);	/* number of usecs in time interval.	*/

extern double tdiff
   (INT_TIME	it1,		/* INT_TIME t1.				*/
    INT_TIME	it2);		/* INT_TIME t2.				*/

extern char *time_to_str 
   (INT_TIME	it,		/* INT_TIME to convert to string.	*/
    int		fmt);		/* format specifier.			*/

extern char *utime_to_str 
   (INT_TIME	it,		/* INT_TIME to convert to string.	*/
    int		fmt);		/* format specifier.			*/

extern char *interval_to_str
   (EXT_TIME	et,		/* Interval to convert to string.	*/
    int		fmt);		/* format specifier.			*/

extern INT_TIME *parse_date
   (char	*str);		/* string containing date to parse.	*/

extern INT_TIME *parse_date_month
   (char	*str);		/* string containing date to parse.	*/

extern EXT_TIME *parse_interval
   (char	*str);		/* string containing interval to parse.	*/

extern int valid_span
   (char	*span);		/* string containing timespan.		*/

extern INT_TIME end_of_span 
   (INT_TIME	it,		/* INT_TIME with initial time.		*/
    char    	*span);		/* string containing timespan.		*/

extern INT_TIME add_interval
   (INT_TIME	it,		/* INT_TIME containing initial time.	*/
    EXT_TIME	interval);	/* EXT_TIME containing time interval.	*/

extern INT_TIME int_time_from_time_tm
   (struct tm	*tm);		/* ptr to time to convert.		*/

extern time_t unix_time_from_ext_time 
   (EXT_TIME	et);		/* EXT_TIME to convert to unix timestamp*/

extern time_t unix_time_from_int_time
   (INT_TIME	it);		/* INT_TIME to convert to Unix timestamp*/

extern EXT_TIME det_time_to_int_time
   (long	evtsec,		/* quanterra seconds for detection time.*/
    int		msec);		/* quanterra msecs for detection time.	*/

extern INT_TIME int_time_from_timeval
   (struct timeval *tv);	/* ptr to struct timeval with input time*/

#ifdef	qlib2_fortran

/************************************************************************/
/* Fortran interludes to qtime routines.				*/
/************************************************************************/

#ifdef	fortran_suffix
extern void f_add_time_ 
#else
extern void f_add_time
#endif
   (INT_TIME	*it,		/* Initial time.			*/
    int		*seconds,	/* Number of seconds to add.		*/
    int		*usecs,		/* Number of usecs to add.		*/
    INT_TIME	*ot);		/* Resultant time.			*/

#ifdef	fortran_suffix
void f_add_dtime_ 
#else
void f_add_dtime
#endif
   (INT_TIME	*it,		/* Initial time.			*/
    double	*usecs,		/* Number of usecs to add.		*/
    INT_TIME	*ot);		/* Resultant time.			*/

#ifdef	fortran_suffix
extern void f_time_interval2_ 
#else
extern void f_time_interval2
#endif
   (int		*n,		/* number of samples.			*/
    int		*rate,		/* sample rate.				*/
    int		*rate_mult,	/* sample rate_mult in qlib convention.	*/
    int		*seconds,	/* result interval for n samples (sec)	*/
    int		*usecs);	/* result interval for n samples (usec)	*/

#ifdef	fortran_suffix
extern double f_dsamples_in_time_ 
#else
extern double f_dsamples_in_time
#endif
   (int		*rate,		/* sample rate.				*/
    double	*dusecs);	/* number of usecs.			*/

#ifdef	fortran_suffix
extern double f_tdiff_ 
#else
extern double f_tdiff
#endif
   (INT_TIME	*it1,		/* time1.				*/
    INT_TIME	*it2);		/* time2.  Return (time1-time2)		*/

#ifdef	fortran_suffix
void f_dy_to_mdy_
#else
void f_dy_to_mdy
#endif
   (int		*doy,		/* day of year (input).			*/
    int		*year,		/* year (input).			*/
    int		*month,		/* month of year (returned).		*/
    int		*mday);		/* day of month (returned).		*/

#ifdef	fortran_suffix
int f_mdy_to_doy_
#else
int f_mdy_to_doy
#endif
   (int		*month,		/* month of year (input).		*/
    int		*day,		/* day of month (input).		*/
    int		*year);		/* year (input).			*/

#ifdef	fortran_suffix
extern void f_int_to_ext_ 
#else
extern void f_int_to_ext
#endif
   (INT_TIME	*it,		/* input INT_TIME to be convert.	*/
    EXT_TIME	*et);		/* returned equivalent EXT_TIME.	*/

#ifdef	fortran_suffix
extern void f_ext_to_int_ 
#else
extern void f_ext_to_int
#endif
   (EXT_TIME	*et,		/* input EXT_TIME to be converted.	*/
    INT_TIME	*it);		/* returned equivalent INT_TIME.	*/

#ifdef	fortran_suffix
extern void f_time_to_str_ 
#else
extern void f_time_to_str
#endif
   (INT_TIME	*it,		/* INT_TIME to be converted.		*/
    int		*fmt,		/* format number for string.		*/
    char	*str,		/* output characters string.		*/
    int		slen);		/* (fortran supplied) length of string.	*/

#ifdef	fortran_suffix
extern int f_parse_date_ 
#else
extern int f_parse_date
#endif
   (INT_TIME	*it,		/* INT_TIME to be converted.		*/
    char	*str,		/* output characters string.		*/
    int		slen);		/* (fortran supplied) length of string.	*/

#ifdef	fortran_suffix
extern int f_parse_date_month_ 
#else
extern int f_parse_date_month
#endif
   (INT_TIME	*it,		/* INT_TIME to be converted.		*/
    char	*str,		/* output characters string.		*/
    int		slen);		/* (fortran supplied) length of string.	*/

#ifdef	fortran_suffix
extern double f_nepoch_to_tepoch_
#else
extern double f_nepoch_to_tepoch
#endif
   (double	*nepoch);	/* Nominal epoch to convert to true.	*/

#ifdef	fortran_suffix
extern double f_tepoch_to_nepoch_
#else
extern double f_tepoch_to_nepoch
#endif
   (double	*tepoch);	/* True epoch to convert to nominal.	*/

#endif

#ifdef	__cplusplus
}
#endif

#endif

/************************************************************************/
/*  Utility routines for Quanterra data processing.			*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2003 The Regents of the University of California.
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research and non-profit purposes,
 * without fee, and without a written agreement is hereby granted,
 * provided that the above copyright notice, this paragraph and the
 * following three paragraphs appear in all copies.
 * 
 * Permission to incorporate this software into commercial products may
 * be obtained from the Office of Technology Licensing, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA  94704.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND
 * ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
 * CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
 * UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/*	$Id: qutils.h 4229 2011-05-25 22:33:01Z lombard $ 	*/

#ifndef	__qutils_h
#define	__qutils_h


#ifdef	__cplusplus
extern "C" {
#endif

extern int my_wordorder;	/* Unknown endian.			*/
extern int hdr_wordorder;	/* desired hdr wordorder. (W)		*/
extern int data_wordorder;	/* desired data wordorder. (W)		*/
extern int default_data_hdr_ind;/* dflt data_hdr_ind (R/W).		*/
extern int qlib2_errno;		/* qlib2 extented error code.		*/
extern int qlib2_op_mode;	/* qlib2 operation mode.		*/

extern void seed_to_comp
   (char	*seed,		/* SEED channel name (input).		*/
    char	**stream,	/* SEED stream name (output).		*/
    char	**component);	/* SEED component name (output).	*/

extern void comp_to_seed 
   (char	*stream,	/* SEED stream name (input).		*/
    char	*component,	/* SEED component name (input).		*/
    char	**seed);	/* SEED channel name (output).		*/

extern int allnull
   (char	*p,		/* ptr to block of memory to check.	*/
    int		n);		/* number of bytes to check.		*/

extern int roundoff
   (double	d);		/* double precision value to round.	*/

extern int xread 
   (int		fd,		/* file descriptor of input file.	*/
    char	*buf,		/* ptr to input buffer.			*/
    int		n);		/* desired number of bytes to read.	*/

extern int xwrite 
   (int		fd,		/* file descriptor of output file.	*/
    char	*buf,		/* ptr to ouput buffer.			*/
    int		n);		/* desired number of bytes to write.	*/

extern void cstr_to_fstr
   (char	*str,		/* input null-terminated string.	*/
    int		flen);		/* output fortran blank-padded string.	*/

extern int date_fmt_num 
   (char	*str);		/* string containing date fmt string.	*/

extern int print_syntax
   (char	*cmd,		/* program name.			*/
    char	*syntax[],	/* syntax array.			*/
    FILE	*fp);		/* FILE ptr for output.			*/

extern char *uppercase
   (char	*string);	/* string to convert to upper case.	*/

extern char *lowercase
   (char	*string);	/* string to convert to lower case.	*/

extern char *charncpy
   (char	*out,		/* ptr to output string.		*/
    char	*in,		/* ptr to input string.			*/
    int		n);		/* number of characters to copy.	*/

extern char *charvncpy
   (char	*out,		/* ptr to output string.		*/
    char	*in,		/* ptr to input string.			*/
    int		n,		/* number of characters to copy.	*/
    int		i);		/* index of SEED var string to copy.	*/

extern char *trim
   (char *str);			/* string to trim trailing blanks.	*/

extern char *tail
   (char	*path);		/* pathname.				*/

extern char *capnstr
   (char	*dst,		/* destination char array to fill.	*/
    char	*src,		/* source string to copy.		*/
    int		n);		/* char length for destination array.	*/

extern char *capnint
   (char	*dst,		/* destination char array to fill.	*/
    int		ival,		/* integer to be encoded.		*/
    int		n);		/* char length for destination array.	*/

extern char *capnlong
   (char	*dst,		/* destination char array to fill.	*/
    long int	ival,		/* long integer to be encoded.		*/
    int		n);		/* char length for destination array.	*/

extern char *capntstr
   (char	*dst,		/* destination char array to fill.	*/
    char	*src,		/* source string to copy.		*/
    int		n);		/* char length for destination array.	*/

extern char *capntint
   (char	*dst,		/* destination char array to fill.	*/
    int		ival,		/* integer to be encoded.		*/
    int		n);		/* char length for destination array.	*/

extern char *capntlong
   (char	*dst,		/* destination char array to fill.	*/
    long int	ival,		/* long integer to be encoded.		*/
    int		n);		/* char length for destination array.	*/

extern int get_my_wordorder(void);	

extern int set_hdr_wordorder
   (int		wordorder);	/* desired wordorder for headers.	*/

extern int set_data_wordorder
   (int		wordorder);	/* desired wordorder for data.		*/

extern char set_record_type
   (char	record_type);	/* desired default record_type.		*/

extern int init_qlib2
   (int		mode);		/* desired qlib2 operation mode.	*/

extern int wordorder_from_time
   (unsigned char *p);		/* ptr to fixed data time field.	*/

extern int is_data_hdr_ind 
    (char c);			/* data_hdr_ind char.			*/

extern int is_vol_hdr_ind 
    (char c);			/* vol_hdr_ind char.			*/

extern void swab2
   (short int	*in);		/* ptr to short integer to byteswap.	*/

extern void swab3
   (unsigned char *in);		/* ptr to byte array to byteswap.	*/

extern void swab4
   (int		*in);		/* ptr to integer to byteswap.		*/

extern void swab8
   (double	*in);		/* ptr to double to byteswap.		*/

extern void swabt 
   (SDR_TIME	*st);		/* ptr to SDR_TIME to byteswap.		*/

extern void swabf
   (float	*in);		/* ptr to float to byteswap.		*/

#ifdef	qlib2_fortran

/************************************************************************/
/* Fortran interludes to qutils routines.				*/
/************************************************************************/

#ifdef	fortran_suffix
int f_get_my_wordorder_ (void);
#else
int f_get_my_wordorder (void);
#endif

#ifdef	fortran_suffix
int f_set_hdr_wordorder_
#else
int f_set_hdr_wordorder
#endif
   (int		*wordorder);	/* desired wordorder for headers.	*/

#ifdef	fortran_suffix
int f_set_data_wordorder_
#else
int f_set_data_wordorder
#endif
   (int		*wordorder);	/* desired wordorder for data.		*/

#ifdef	fortran_suffix
void f_set_record_type_
#else
void f_set_record_type
#endif
   (char 	*i_record_type,	/* desired default record_type.		*/
    char	*o_record_type,	/* output default record_type.		*/
    int		ilen,		/* (fortran supplied) length of string.	*/
    int		olen);		/* (fortran supplied) length of string.	*/

#endif

#ifdef	__cplusplus
}
#endif

#endif

/************************************************************************/
/*  Routines for processing native binary dialup (QDA) Quanterra data.	*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2000 The Regents of the University of California.
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research and non-profit purposes,
 * without fee, and without a written agreement is hereby granted,
 * provided that the above copyright notice, this paragraph and the
 * following three paragraphs appear in all copies.
 * 
 * Permission to incorporate this software into commercial products may
 * be obtained from the Office of Technology Licensing, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA  94704.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND
 * ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
 * CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
 * UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/*	$Id: qda_utils.h 2498 2006-11-16 15:23:49Z ilya $ 	*/

#ifndef	__qda_utils_h
#define	__qda_utils_h


#ifdef	__cplusplus
extern	"C" {
#endif

extern char *get_component_name
   (char	*station,	/* station name.			*/
    int		comp);		/* component number.			*/

extern INT_TIME decode_time_qda
   (QDA_TIME	qt,		/* input time in QDA format.		*/
    short int	millisecond,	/* millisecond offset.			*/
    int		wordorder);	/* wordorder of time contents.		*/

extern void decode_flags_qda
   (int		*pclock,	/* ptr to clock correction (output).	*/
    int		soh,		/* state_of_health flag.		*/
    unsigned char *pa,		/* ptr to activity flag (output);	*/
    unsigned char *pi,		/* ptr to i/o flag (output).		*/
    unsigned char *pq,		/* ptr to quality flag (output).	*/
    int		wordorder);	/* wordorder of time contents.		*/

extern void encode_flags_qda 
   (int		old_soh,	/* old state_of_health flag (input).	*/
    unsigned char *soh,		/* state_of_health flag (output).	*/
    unsigned char pa,		/* activity flag (input).		*/
    unsigned char pi,		/* i/o flag (input).			*/
    unsigned char pq,		/* quality flag (input).		*/
    int		wordorder);	/* wordorder of time contents.		*/

extern DATA_HDR *decode_hdr_qda
   (QDA_HDR	*ihdr,		/* ptr to input header in QDA format.	*/
    int		maxbytes);	/* max # bytes in buffer.		*/

extern int is_qda_header 
   (QDA_HDR	*p,		/* ptr to buffer containing header.	*/
    int		nr);		/* max number of bytes for header.	*/

#ifdef	__cplusplus
}
#endif

#endif

/************************************************************************/
/*  Routines for processing DRM Quanterra data.				*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2000 The Regents of the University of California.
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research and non-profit purposes,
 * without fee, and without a written agreement is hereby granted,
 * provided that the above copyright notice, this paragraph and the
 * following three paragraphs appear in all copies.
 * 
 * Permission to incorporate this software into commercial products may
 * be obtained from the Office of Technology Licensing, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA  94704.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND
 * ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
 * CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
 * UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/*	$Id: drm_utils.h 2498 2006-11-16 15:23:49Z ilya $ 	*/

#ifndef	__drm_utils_h
#define	__drm_utils_h

#ifdef	__cplusplus
extern "C" {
#endif

extern INT_TIME decode_time_drm 
   (DA_TIME	dt,		/* DRM time structure containing time.	*/
    int		wordorder);	/* wordorder of time contents.		*/

extern DA_TIME encode_time_drm 
   (INT_TIME	it,		/* IN_TIME structure to decode.		*/
    int		wordorder);	/* wordorder for encoded time contents.	*/

extern void decode_flags_drm
   (int		*pclock,	/* ptr to clock correction (output).	*/
    int		soh,		/* state_of_health flag.		*/
    unsigned char *pa,		/* ptr to activity flag (output);	*/
    unsigned char *pi,		/* ptr to i/o flag (output).		*/
    unsigned char *pq,		/* ptr to quality flag (output).	*/
    int		wordorder);	/* wordorder for encoded time contents.	*/

extern void encode_flags_drm
   (int		old_soh,	/* old state_of_health flag (input).	*/
    unsigned char *soh,		/* state_of_health flag (output).	*/
    unsigned char pa,		/* activity flag (input).		*/
    unsigned char pi,		/* i/o flag (input).			*/
    unsigned char pq,		/* quality flag (input).		*/
    int		wordorder);	/* wordorder for encoded time contents.	*/

extern DATA_HDR *decode_hdr_drm 
   (STORE_DATA	*ihdr,		/* ptr to raw DRM header.		*/
    int		maxbytes);	/* max # bytes in buffer.		*/

extern STORE_DATA *encode_hdr_drm
   (DATA_HDR	*ihdr);		/* ptr to input DATA_HDR.		*/

extern int is_drm_header 
   (STORE_FILE_HEAD *p,		/* ptr to buffer containing header.	*/
    int		nr);		/* max number of bytes for header.	*/

#ifdef	__cplusplus
}
#endif

#endif

/************************************************************************/
/*  Routines for processing SEED Data Record (SDR) Quanterra data.	*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2002 The Regents of the University of California.
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research and non-profit purposes,
 * without fee, and without a written agreement is hereby granted,
 * provided that the above copyright notice, this paragraph and the
 * following three paragraphs appear in all copies.
 * 
 * Permission to incorporate this software into commercial products may
 * be obtained from the Office of Technology Licensing, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA  94704.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND
 * ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
 * CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
 * UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/*	$Id: sdr_utils.h 4229 2011-05-25 22:33:01Z lombard $ 	*/

#ifndef	__sdr_utils_h
#define	__sdr_utils_h


#ifdef	__cplusplus
extern "C" {
#endif

extern INT_TIME decode_time_sdr
   (SDR_TIME	st,		/* SDR_TIME structure to decode.	*/
    int		wordorder);	/* wordorder of time contents.		*/

extern SDR_TIME encode_time_sdr
   (INT_TIME	it,		/* IN_TIME structure to decode.		*/
    int		wordorder);	/* wordorder for encoded time contents.	*/

extern DATA_HDR *decode_hdr_sdr
   (SDR_HDR	*ihdr,		/* input SDR header.			*/
    int		maxbytes);	/* max # bytes in buffer.		*/

extern int eval_rate 
   (int	sample_rate_factor,	/* Fixed data hdr sample rate factor.	*/
    int	sample_rate_mult);	/* Fixed data hdr sample rate multiplier*/

extern char *asc_sdr_time
   (char	*str,		/* string to encode time into.		*/
    SDR_TIME	st,		/* SDR_TIME structure to decode.	*/
    int		wordorder);	/* wordorder for encoded time contents.	*/

extern time_t unix_time_from_sdr_time
   (SDR_TIME	st,		/* SDR_TIME structure to convert.	*/
    int		wordorder);	/* wordorder for encoded time contents.	*/

extern int read_blockettes
   (DATA_HDR	*hdr,		/* data_header structure.		*/
    char	*str);		/* ptr to fixed data header.		*/

extern BS *find_blockette 
   (DATA_HDR	*hdr,		/* pointer to DATA_HDR structure.	*/
    int		n);		/* blockette type to find.		*/

extern BS *find_pblockette 
   (DATA_HDR	*hdr,		/* pointer to DATA_HDR structure.	*/
    BS		*bs,		/* BS* to start with.			*/
    int		n);		/* blockette type to find.		*/

extern int blockettecmp
   (BS		*bs1,		/* BS* of first blockette to compare.	*/
    BS		*bs2);		/* BS* of first blockette to compare.	*/

extern int write_blockettes
   (DATA_HDR	*hdr,		/* ptr to data_hdr			*/
    char	*str);		/* ptr to output SDR.			*/

extern int add_blockette
   (DATA_HDR	*hdr,		/* ptr to data_hdr.			*/
    char	*str,		/* pre-constructed blockette.		*/
    int		type,		/* blockette type.			*/
    int		l,		/* length of blockette.			*/
    int		wordorder,	/* wordorder of blockette contents.	*/
    int		where);		/* i -> i-th blockette from start,	*/
				/* -1 -> append as last blockette.	*/

extern int delete_blockette 
   (DATA_HDR	*hdr,		/* ptr to DATA_HDR.			*/
    int		n);		/* blockette # to delete.  -1 -> ALL.	*/

extern int delete_pblockette 
   (DATA_HDR	*hdr,		/* ptr to DATA_HDR.			*/
    BS		*dbs);		/* BS* to delete.			*/

extern int add_required_miniseed_blockettes 
   (DATA_HDR	*hdr);		/* ptr to DATA_HDR.			*/

extern void init_data_hdr 
   (DATA_HDR	    *hdr);	/* ptr to DATA_HDR to initialize.	*/

extern DATA_HDR *new_data_hdr (void);	

extern DATA_HDR *copy_data_hdr
   (DATA_HDR	    *hdr_dst,	/* ptr to destination DATA_HDR.		*/
    DATA_HDR	    *hdr_src);	/* ptr to source DATA_HDR to copy.	*/

extern DATA_HDR *dup_data_hdr
   (DATA_HDR	*hdr);		/* ptr to DATA_HDR to duplicate.	*/

extern void free_data_hdr
   (DATA_HDR	*hdr);		/* ptr to DATA_HDR to free.	*/

extern void dump_hdr 
   (DATA_HDR	*h,		/* ptr to Data_Hdr structure.		*/
    char	*str,		/* write debugging info into string.	*/
    int		date_fmt);	/* format specified for date/time.	*/

extern void dump_sdr_flags
   (DATA_HDR	*ch,		/* DATA_HDR for record.			*/
    int		detail);	/* dump only non-zero field if false.	*/

extern int is_sdr_header 
   (SDR_HDR	*p,		/* ptr to buffer containing header.	*/
    int		nr);		/* max number of bytes for header.	*/

extern int is_sdr_vol_header
   (SDR_HDR	*p,		/* ptr to buffer containing header.	*/
    int		nr);		/* max	number of bytes for header.	*/

extern int decode_seqno 
   (char	*str,		/* string containing ascii seqno.	*/
    char	**pp);		/* ptr to next char after seqno.	*/

extern char *q_clock_model
   (char	clock_model);	/* numeric clock model from blockette.	*/

extern char *q_clock_status
   (char	*status,	/* status string from blockette.	*/
    char	clock_model);	/* numeric clock model from blockette.	*/

extern int swab_blockette
   (int		type,		/* blockette number.			*/
    char	*contents,	/* string containing blockette.		*/
    int		len);		/* length of blockette (incl header).	*/

extern int init_sdr_hdr
   (SDR_HDR	*sh,		/* ptr to space for sdr data hdr.	*/
    DATA_HDR	*hdr,		/* initial DATA_HDR for sdr record.	*/
    BS		*extra_bs);	/* ptr to block-specific blockettes.	*/

extern int update_sdr_hdr
   (SDR_HDR	*sh,		/* ptr to space for SDR data hdr.	*/
    DATA_HDR	*hdr);		/* initial DATA_HDR for SDR record.	*/

#ifdef	qlib2_fortran

/************************************************************************/
/* Fortran interludes to sdr_utils routines.				*/
/************************************************************************/

#ifdef	fortran_suffix
extern void f_init_data_hdr_
#else
extern void f_init_data_hdr
#endif
   (DATA_HDR	    *hdr);	/* ptr to DATA_HDR to initialize.	*/

#ifdef	fortran_suffix
extern int f_delete_blockette_
#else
extern int f_delete_blockette
#endif
   (DATA_HDR	*hdr,		/* ptr to DATA_HDR.			*/
    int		*n);		/* blockette # to delete.  -1 -> ALL.	*/

#endif

#ifdef	fortran_suffix
extern void f_copy_data_hdr_
#else
extern void f_copy_data_hdr
#endif
   (DATA_HDR	    *hdr_dst,	/* ptr to destination DATA_HDR.		*/
    DATA_HDR	    *hdr_src);	/* ptr to source DATA_HDR to copy.	*/

#ifdef	__cplusplus
}
#endif

#endif

/************************************************************************/
/*  Routines for processing MiniSEED records and files.			*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2000 The Regents of the University of California.
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research and non-profit purposes,
 * without fee, and without a written agreement is hereby granted,
 * provided that the above copyright notice, this paragraph and the
 * following three paragraphs appear in all copies.
 * 
 * Permission to incorporate this software into commercial products may
 * be obtained from the Office of Technology Licensing, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA  94704.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND
 * ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
 * CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
 * UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/*	$Id: ms_utils.h 4229 2011-05-25 22:33:01Z lombard $ 	*/

#ifndef	__ms_utils_h
#define	__ms_utils_h


#ifdef	__cplusplus
extern "C" {
#endif

extern int read_ms 
   (DATA_HDR	**phdr,		/* pointer to pointer to DATA_HDR.	*/
    void	*data_buffer,	/* pointer to output data buffer.	*/
    int		max_num_points,	/* max # data points to return.		*/
    FILE	*fp);		/* FILE pointer for input file.		*/

extern int read_ms_record 
   (DATA_HDR	**phdr,		/* pointer to pointer to DATA_HDR.	*/
    char	**pbuf,		/* ptr to buf for MiniSEED record.	*/
    FILE	*fp);		/* FILE pointer for input file.		*/

extern int read_ms_hdr 
   (DATA_HDR	**phdr,		/* pointer to pointer to DATA_HDR.	*/
    char	**pbuf,		/* ptr to buf for MiniSEED record.	*/
    FILE	*fp);		/* FILE pointer for input file.		*/

extern int read_ms_bkt
   (DATA_HDR	*hdr,		/* data_header structure.		*/
    char	*buf,		/* ptr to fixed data header.		*/
    FILE	*fp);		/* FILE pointer for input file.		*/

extern int read_ms_data 
   (DATA_HDR	*hdr,		/* pointer to pointer to DATA_HDR.	*/
    char	*buf,		/* pointer to output data buffer.	*/
    int		offset,		/* offset in buffer to write data.	*/
    FILE	*fp);		/* FILE pointer for input file.		*/

extern DATA_HDR *decode_fixed_data_hdr
    (SDR_HDR	*ihdr);		/* MiniSEED header.			*/

extern MS_ATTR get_ms_attr
   (DATA_HDR	*hdr);		/* ptr to DATA_HDR structure.		*/

extern int decode_data_format 
   (char	*str);		/* string containing data format.	*/

extern char *encode_data_format 
   (int		format);	/* data format number.			*/

extern int mseed_to_data_format 
   (int		format);	/* data format number.			*/

#ifdef	qlib2_fortran

/************************************************************************/
/* Fortran interludes to ms_utils routines.				*/
/************************************************************************/

#ifdef	fortran_suffix
extern int f_read_ms_
#else
extern int f_read_ms
#endif
   (DATA_HDR	*fhdr,		/* pointer to FORTRAN DATA_HDR.		*/
    void	*data_buffer,	/* pointer to output data buffer.	*/
    int		*maxpts,	/* max # data points to return.		*/
    FILE	**pfp);		/* FILE pointer for input file.		*/

#endif

#ifdef	__cplusplus
}
#endif

#endif

/************************************************************************/
/*  Routines for processing MiniSEED records and files.			*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2000 The Regents of the University of California.
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research and non-profit purposes,
 * without fee, and without a written agreement is hereby granted,
 * provided that the above copyright notice, this paragraph and the
 * following three paragraphs appear in all copies.
 * 
 * Permission to incorporate this software into commercial products may
 * be obtained from the Office of Technology Licensing, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA  94704.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND
 * ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
 * CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
 * UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/*	$Id: ms_pack.h 4229 2011-05-25 22:33:01Z lombard $ 	*/

#ifndef	__ms_pack_h
#define	__ms_pack_h


#ifdef	__cplusplus
extern "C" {
#endif

extern int ms_pack_data
   (DATA_HDR	*hdr,		/* ptr to initial data hdr.		*/
    BS		*init_bs,	/* ptr to onetime blockettes.		*/
    int		num_samples,	/* number of data samples.		*/
    void	*data,		/* ptr to data buffer.			*/
    int		*n_blocks,	/* # miniSEED blocks (returned).	*/
    char	**pp_ms,	/* ptr **miniSEED (returned).		*/
    int		ms_len,		/* miniSEED buffer len (if supplied).	*/
    char	*p_errmsg);	/* ptr to error msg buffer.		*/

extern int ms_pack_update_hdr
   (DATA_HDR	*hdr,		/* ptr to data hdr to update.		*/
    int		num_records,	/* number of mseed records just packed.	*/
    int		num_samples,	/* number of samples just packed.	*/
    int 	*data);		/* data buffer used for last ms_pack.	*/

extern int init_miniseed_hdr
   (SDR_HDR	*sh,		/* ptr to space for miniSEED data hdr.	*/
    DATA_HDR	*hdr,		/* initial DATA_HDR for miniSEED record.*/
    BS		*extra_bs);	/* ptr to block-specific blockettes.	*/

extern int update_miniseed_hdr
   (SDR_HDR	*sh,		/* ptr to space for miniSEED data hdr.	*/
    DATA_HDR	*hdr);		/* initial DATA_HDR for miniSEED record.*/

extern int ms_pack_steim 
   (DATA_HDR	*hdr0,		/* ptr to initial data hdr.		*/
    BS		*init_bs,	/* ptr to onetime blockettes.		*/
    int		*data,		/* ptr to data buffer.			*/
    int		*diff,		/* ptr to diff buffer (optional)	*/
    int		num_samples,	/* number of data samples.		*/
    int		*n_blocks,	/* # miniSEED blocks (returned).	*/
    char	**pp_ms,	/* ptr **miniSEED (returned).		*/
    int		ms_len,		/* miniSEED buffer len (if supplied).	*/
    char	*p_errmsg);	/* ptr to error msg buffer.		*/

extern int ms_pack_int 
   (DATA_HDR	*hdr0,		/* ptr to initial data hdr.		*/
    BS		*init_bs,	/* ptr to onetime blockettes.		*/
    int		*data,		/* ptr to data buffer.			*/
    int		num_samples,	/* number of data samples.		*/
    int		*n_blocks,	/* # miniSEED blocks (returned).	*/
    char	**pp_ms,	/* ptr **miniSEED (returned).		*/
    int		ms_len,		/* miniSEED buffer len (if supplied).	*/
    char	*p_errmsg);	/* ptr to error msg buffer.		*/

extern int ms_pack_fp 
   (DATA_HDR	*hdr0,		/* ptr to initial data hdr.		*/
    BS		*init_bs,	/* ptr to onetime blockettes.		*/
    void	*data,		/* ptr to data buffer.			*/
    int		num_samples,	/* number of data samples.		*/
    int		*n_blocks,	/* # miniSEED blocks (returned).	*/
    char	**pp_ms,	/* ptr **miniSEED (returned).		*/
    int		ms_len,		/* miniSEED buffer len (if supplied).	*/
    char	*p_errmsg);	/* ptr to error msg buffer.		*/

extern int ms_pack_text
   (DATA_HDR	*hdr0,		/* ptr to initial data hdr.		*/
    BS		*init_bs,	/* ptr to onetime blockettes.		*/
    char	*data,		/* ptr to data buffer.			*/
    int		num_samples,	/* number of data samples.		*/
    int		*n_blocks,	/* # miniSEED blocks (returned).	*/
    char	**pp_ms,	/* ptr **miniSEED (returned).		*/
    int		ms_len,		/* miniSEED buffer len (if supplied).	*/
    char	*p_errmsg);	/* ptr to error msg buffer.		*/

#ifdef	qlib2_fortran

/************************************************************************/
/* Fortran interludes to ms_pack routines.				*/
/************************************************************************/

#ifdef	fortran_suffix
extern int f_ms_pack_data_
#else
extern int f_ms_pack_data
#endif
   (DATA_HDR	*hdr,		/* ptr to initial data hdr.		*/
    int		*num_samples,	/* number of data samples.		*/
    int		*data,		/* ptr to data buffer.			*/
    int		*n_blocks,	/* # miniSEED blocks (returned).	*/
    char	*p_ms,		/* ptr *miniSEED (required).		*/
    int		*ms_len);	/* miniSEED buffer len (required).	*/

#ifdef	fortran_suffix
extern int ms_pack_update_hdr_
#else
extern int ms_pack_update_hdr
#endif
   (DATA_HDR	*hdr,		/* ptr to data hdr to update.		*/
    int		*num_records,	/* number of mseed records just packed.	*/
    int		*num_samples,	/* number of samples just packed.	*/
    int 	*data);		/* data buffer used for last ms_pack.	*/

#endif

#ifdef	__cplusplus
}
#endif

#endif

/************************************************************************/
/*  Routines for processing MiniSEED records and files.			*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2000 The Regents of the University of California.
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research and non-profit purposes,
 * without fee, and without a written agreement is hereby granted,
 * provided that the above copyright notice, this paragraph and the
 * following three paragraphs appear in all copies.
 * 
 * Permission to incorporate this software into commercial products may
 * be obtained from the Office of Technology Licensing, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA  94704.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND
 * ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
 * CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
 * UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/*	$Id: ms_pack2.h,v 1.2 2008/10/24 19:53:55 doug Exp $ 	*/

#ifndef	__ms_pack2_h
#define	__ms_pack2_h


#ifdef	__cplusplus
extern "C" {
#endif

extern int ms_pack2_data
   (DATA_HDR	*hdr,		/* ptr to initial data hdr.		*/
    BS		*init_bs,	/* ptr to onetime blockettes.		*/
    int		num_samples,	/* number of data samples.		*/
    void	*data,		/* ptr to data buffer.			*/
    int		*n_blocks,	/* # miniSEED blocks (returned).	*/
    int		*n_samples,	/* # data samples packed (returned).	*/
    char	**pp_ms,	/* ptr **miniSEED (returned).		*/
    int		ms_len,		/* miniSEED buffer len (if supplied).	*/
    char	*p_errmsg);	/* ptr to error msg buffer.		*/

extern int ms_pack2_update_hdr
(DATA_HDR	*hdr,		/* ptr to data hdr to update.		*/
    int		num_records,	/* number of mseed records just packed.	*/
    int		num_samples,	/* number of samples just packed.	*/
    int 	*data);		/* data buffer used for last ms_pack2.	*/

extern int init_miniseed_hdr
   (SDR_HDR	*sh,		/* ptr to space for miniSEED data hdr.	*/
    DATA_HDR	*hdr,		/* initial DATA_HDR for miniSEED record.*/
    BS		*extra_bs);	/* ptr to block-specific blockettes.	*/

extern int update_miniseed_hdr
   (SDR_HDR	*sh,		/* ptr to space for miniSEED data hdr.	*/
    DATA_HDR	*hdr);		/* initial DATA_HDR for miniSEED record.*/

extern int ms_pack2_steim 
   (DATA_HDR	*hdr0,		/* ptr to initial data hdr.		*/
    BS		*init_bs,	/* ptr to onetime blockettes.		*/
    int		*data,		/* ptr to data buffer.			*/
    int		*diff,		/* ptr to diff buffer (optional)	*/
    int		num_samples,	/* number of data samples.		*/
    int		*n_blocks,	/* # miniSEED blocks (returned).	*/
    int		*n_samples,	/* # data samples packed (returned).	*/
    char	**pp_ms,	/* ptr **miniSEED (returned).		*/
    int		ms_len,		/* miniSEED buffer len (if supplied).	*/
    char	*p_errmsg);	/* ptr to error msg buffer.		*/

extern int ms_pack2_int 
   (DATA_HDR	*hdr0,		/* ptr to initial data hdr.		*/
    BS		*init_bs,	/* ptr to onetime blockettes.		*/
    int		*data,		/* ptr to data buffer.			*/
    int		num_samples,	/* number of data samples.		*/
    int		*n_blocks,	/* # miniSEED blocks (returned).	*/
    int		*n_samples,	/* # data samples packed (returned).	*/
    char	**pp_ms,	/* ptr **miniSEED (returned).		*/
    int		ms_len,		/* miniSEED buffer len (if supplied).	*/
    char	*p_errmsg);	/* ptr to error msg buffer.		*/

extern int ms_pack2_float 
   (DATA_HDR	*hdr0,		/* ptr to initial data hdr.		*/
    BS		*init_bs,	/* ptr to onetime blockettes.		*/
    float	*data,		/* ptr to data buffer.			*/
    int		num_samples,	/* number of data samples.		*/
    int		*n_blocks,	/* # miniSEED blocks (returned).	*/
    int		*n_samples,	/* # data samples packed (returned).	*/
    char	**pp_ms,	/* ptr **miniSEED (returned).		*/
    int		ms_len,		/* miniSEED buffer len (if supplied).	*/
    char	*p_errmsg);	/* ptr to error msg buffer.		*/

extern int ms_pack2_double 
   (DATA_HDR	*hdr0,		/* ptr to initial data hdr.		*/
    BS		*init_bs,	/* ptr to onetime blockettes.		*/
    double	*data,		/* ptr to data buffer.			*/
    int		num_samples,	/* number of data samples.		*/
    int		*n_blocks,	/* # miniSEED blocks (returned).	*/
    int		*n_samples,	/* # data samples packed (returned).	*/
    char	**pp_ms,	/* ptr **miniSEED (returned).		*/
    int		ms_len,		/* miniSEED buffer len (if supplied).	*/
    char	*p_errmsg);	/* ptr to error msg buffer.		*/

extern int ms_pack2_text
   (DATA_HDR	*hdr0,		/* ptr to initial data hdr.		*/
    BS		*init_bs,	/* ptr to onetime blockettes.		*/
    char	*data,		/* ptr to data buffer.			*/
    int		num_samples,	/* number of data samples.		*/
    int		*n_blocks,	/* # miniSEED blocks (returned).	*/
    int		*n_samples,	/* # data samples packed (returned).	*/
    char	**pp_ms,	/* ptr **miniSEED (returned).		*/
    int		ms_len,		/* miniSEED buffer len (if supplied).	*/
    char	*p_errmsg);	/* ptr to error msg buffer.		*/

#ifdef	qlib2_fortran

/************************************************************************/
/* Fortran interludes to ms_pack2 routines.				*/
/************************************************************************/

#ifdef	fortran_suffix
extern int f_ms_pack2_data_
#else
extern int f_ms_pack2_data
#endif
   (DATA_HDR	*hdr,		/* ptr to initial data hdr.		*/
    int		*num_samples,	/* number of data samples.		*/
    int		*data,		/* ptr to data buffer.			*/
    int		*n_blocks,	/* # miniSEED blocks (returned).	*/
    int		*n_samples,	/* # data samples packed (returned).	*/
    char	*p_ms,		/* ptr *miniSEED (required).		*/
    int		*ms_len);	/* miniSEED buffer len (required).	*/

#ifdef	fortran_suffix
extern int ms_pack2_update_hdr_
#else
extern int ms_pack2_update_hdr
#endif
   (DATA_HDR	*hdr,		/* ptr to data hdr to update.		*/
    int		*num_records,	/* number of mseed records just packed.	*/
    int		*num_samples,	/* number of samples just packed.	*/
    int 	*data);		/* data buffer used for last ms_pack2.	*/

#endif

#ifdef	__cplusplus
}
#endif

#endif

/************************************************************************/
/*  Routines for packing STEIM1, STEIM2, INT_32, INT_16, and INT_24	*/
/*  data records.							*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2000 The Regents of the University of California.
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research and non-profit purposes,
 * without fee, and without a written agreement is hereby granted,
 * provided that the above copyright notice, this paragraph and the
 * following three paragraphs appear in all copies.
 * 
 * Permission to incorporate this software into commercial products may
 * be obtained from the Office of Technology Licensing, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA  94704.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND
 * ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
 * CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
 * UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/*	$Id: pack.h 4229 2011-05-25 22:33:01Z lombard $ 	*/

#ifndef	__pack_h
#define	__pack_h


#ifdef	__cplusplus
extern "C" {
#endif

extern int pack_steim1
   (SDF		*p_sdf,		/* ptr to SDR structure.		*/
    int		data[],		/* unpacked data array.			*/
    int		diff[],		/* unpacked diff array.			*/
    int		ns,		/* num_samples.				*/
    int		nf,		/* total number of data frames.		*/
    int		pad,		/* flag to specify padding to nf.	*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    int		*pnframes,	/* number of frames actually packed.	*/
    int		*pnsamples);	/* number of samples actually packed.	*/

extern int pack_steim2
   (SDF		*p_sdf,		/* ptr to SDR structure.		*/
    int		data[],		/* unpacked data array.			*/
    int		diff[],		/* unpacked diff array.			*/
    int		ns,		/* num_samples.				*/
    int		nf,		/* total number of data frames.		*/
    int		pad,		/* flag to specify padding to nf.	*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    int		*pnframes,	/* number of frames actually packed.	*/
    int		*pnsamples);	/* number of samples actually packed.	*/

extern int pad_steim_frame
   (SDF	    	*p_sdf,
    int		fn,	    	/* current frame number.		*/
    int	    	wn,		/* current work number.			*/
    int	    	nf,		/* total number of data frames.		*/
    int		swapflag,	/* flag to swap byte order of data.	*/
    int	    	pad);		/* flag to pad # frames to nf.		*/

extern int pack_int_32 
   (int		p_packed[],	/* output data array - packed.		*/
    int		data[],		/* input data array - unpacked.		*/
    int		ns,		/* desired number of samples to pack.	*/
    int		max_bytes,	/* max # of bytes for output buffer.	*/
    int		pad,		/* flag to specify padding to max_bytes.*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    int		*pnbytes,	/* number of bytes actually packed.	*/
    int		*pnsamples);	/* number of samples actually packed.	*/

extern int pack_int_16
   (short int p_packed[],	/* output data array - packed.		*/
    int		data[],		/* input data array - unpacked.		*/
    int		ns,		/* desired number of samples to pack.	*/
    int		max_bytes,	/* max # of bytes for output buffer.	*/
    int		pad,		/* flag to specify padding to max_bytes.*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    int		*pnbytes,	/* number of bytes actually packed.	*/
    int		*pnsamples);	/* number of samples actually packed.	*/

extern int pack_int_24 
   (unsigned char p_packed[],	/* output data array - packed.		*/
    int		data[],		/* input data array - unpacked.		*/
    int		ns,		/* desired number of samples to pack.	*/
    int		max_bytes,	/* max # of bytes for output buffer.	*/
    int		pad,		/* flag to specify padding to max_bytes.*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    int		*pnbytes,	/* number of bytes actually packed.	*/
    int		*pnsamples);	/* number of samples actually packed.	*/

extern int pack_fp_sp 
   (float p_packed[],	/* output data array - packed.		*/
    float	*data,		/* input data array - unpacked.		*/
    int		ns,		/* desired number of samples to pack.	*/
    int		max_bytes,	/* max # of bytes for output buffer.	*/
    int		pad,		/* flag to specify padding to max_bytes.*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    int		*pnbytes,	/* number of bytes actually packed.	*/
    int		*pnsamples);	/* number of samples actually packed.	*/

extern int pack_fp_dp 
   (double p_packed[],	/* output data array - packed.		*/
    double 	*data,		/* input data array - unpacked.		*/
    int		ns,		/* desired number of samples to pack.	*/
    int		max_bytes,	/* max # of bytes for output buffer.	*/
    int		pad,		/* flag to specify padding to max_bytes.*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    int		*pnbytes,	/* number of bytes actually packed.	*/
    int		*pnsamples);	/* number of samples actually packed.	*/

extern int pack_text 
   (char 	p_packed[],	/* output data array - packed.		*/
    char	data[],		/* input data array - unpacked.		*/
    int		ns,		/* desired number of samples to pack.	*/
    int		max_bytes,	/* max # of bytes for output buffer.	*/
    int		pad,		/* flag to specify padding to max_bytes.*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    int		*pnbytes,	/* number of bytes actually packed.	*/
    int		*pnsamples);	/* number of samples actually packed.	*/

#ifdef	__cplusplus
}
#endif

#endif

/************************************************************************/
/*  Routines for processing MiniSEED records and files.			*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2000 The Regents of the University of California.
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research and non-profit purposes,
 * without fee, and without a written agreement is hereby granted,
 * provided that the above copyright notice, this paragraph and the
 * following three paragraphs appear in all copies.
 * 
 * Permission to incorporate this software into commercial products may
 * be obtained from the Office of Technology Licensing, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA  94704.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND
 * ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
 * CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
 * UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/*	$Id: ms_unpack.h 2498 2006-11-16 15:23:49Z ilya $ 	*/

#ifndef	__ms_unpack_h
#define	__ms_unpack_h


#ifdef	__cplusplus
extern "C" {
#endif

extern int ms_unpack 
   (DATA_HDR	*hdr,		/* ptr to DATA_HDR for Mini-SEED record.*/
    int		max_num_points,	/* max # of points to return.		*/
    char	*ms,		/* ptr to Mini-SEED record.		*/
    void	*data_buffer);	/* ptr to output data buffer.		*/

#ifdef	__cplusplus
}
#endif

#endif

/************************************************************************/
/*  Routines for unpacking STEIM1, STEIM2, INT_32, INT_16, and INT_24	*/
/*  data records.							*/
/*									*/
/*	Douglas Neuhauser						*/
/*	Seismological Laboratory					*/
/*	University of California, Berkeley				*/
/*	doug@seismo.berkeley.edu					*/
/*									*/
/************************************************************************/

/*
 * Copyright (c) 1996-2000 The Regents of the University of California.
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for educational, research and non-profit purposes,
 * without fee, and without a written agreement is hereby granted,
 * provided that the above copyright notice, this paragraph and the
 * following three paragraphs appear in all copies.
 * 
 * Permission to incorporate this software into commercial products may
 * be obtained from the Office of Technology Licensing, 2150 Shattuck
 * Avenue, Suite 510, Berkeley, CA  94704.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND
 * ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
 * CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
 * UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

/*	$Id: unpack.h 4229 2011-05-25 22:33:01Z lombard $ 	*/

#ifndef	__unpack_h
#define	__unpack_h


#ifdef	__cplusplus
extern "C" {
#endif

extern int unpack_steim1
   (FRAME	*pf,		/* ptr to Steim1 data frames.		*/
    int		nbytes,		/* number of bytes in all data frames.	*/
    int		num_samples,	/* number of data samples in all frames.*/
    int		req_samples,	/* number of data desired by caller.	*/
    int		*databuff,	/* ptr to unpacked data array.		*/
    int		*diffbuff,	/* ptr to unpacked diff array.		*/
    int		*px0,		/* return X0, first sample in frame.	*/
    int		*pxn,		/* return XN, last sample in frame.	*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    char	**p_errmsg);	/* ptr to ptr to error message.		*/

extern int unpack_steim2 
   (FRAME	*pf,		/* ptr to Steim2 data frames.		*/
    int		nbytes,		/* number of bytes in all data frames.	*/
    int		num_samples,	/* number of data samples in all frames.*/
    int		req_samples,	/* number of data desired by caller.	*/
    int		*databuff,	/* ptr to unpacked data array.		*/
    int		*diffbuff,	/* ptr to unpacked diff array.		*/
    int		*px0,		/* return X0, first sample in frame.	*/
    int		*pxn,		/* return XN, last sample in frame.	*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    char	**p_errmsg);	/* ptr to ptr to error message.		*/

extern int unpack_int_16 
   (short int	*ibuf,		/* ptr to input data.			*/
    int		nbytes,		/* number of bytes in all data frames.	*/
    int		num_samples,	/* number of data samples in all frames.*/
    int		req_samples,	/* number of data desired by caller.	*/
    int		*databuff,	/* ptr to unpacked data array.		*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    char	**p_errmsg);	/* ptr to ptr to error message.		*/

extern int unpack_int_32
   (int		*ibuf,		/* ptr to input data.			*/
    int		nbytes,		/* number of bytes in all data frames.	*/
    int		num_samples,	/* number of data samples in all frames.*/
    int		req_samples,	/* number of data desired by caller.	*/
    int		*databuff,	/* ptr to unpacked data array.		*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    char	**p_errmsg);	/* ptr to ptr to error message.		*/

extern int unpack_int_24
   (unsigned char *ibuf,	/* ptr to input data.			*/
    int		nbytes,		/* number of bytes in all data frames.	*/
    int		num_samples,	/* number of data samples in all frames.*/
    int		req_samples,	/* number of data desired by caller.	*/
    int		*databuff,	/* ptr to unpacked data array.		*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    char	**p_errmsg);	/* ptr to ptr to error message.		*/

extern int unpack_fp_sp
   (float	*ibuf,		/* ptr to input data.			*/
    int		nbytes,		/* number of bytes in all data frames.	*/
    int		num_samples,	/* number of data samples in all frames.*/
    int		req_samples,	/* number of data desired by caller.	*/
    float	*databuff,	/* ptr to unpacked data array.		*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    char	**p_errmsg);	/* ptr to ptr to error message.		*/

extern int unpack_fp_dp
   (double	*ibuf,		/* ptr to input data.			*/
    int		nbytes,		/* number of bytes in all data frames.	*/
    int		num_samples,	/* number of data samples in all frames.*/
    int		req_samples,	/* number of data desired by caller.	*/
    double	*databuff,	/* ptr to unpacked data array.		*/
    int		data_wordorder,	/* wordorder of data (NOT USED).	*/
    char	**p_errmsg);	/* ptr to ptr to error message.		*/




#ifdef	__cplusplus
}
#endif

#endif

