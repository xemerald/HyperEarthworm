/* psn4head.h - Header file for the Psn4Putaway functions.
   Created Dec 14 2005 by Larry Cochrane, Redwood City, PSN
 */

#ifndef PSN4HEAD_H
#define PSN4HEAD_H

#ifndef BYTE
#define BYTE unsigned char
#endif

#ifndef WORD
#define WORD unsigned short
#endif

#ifndef UINT
#define UINT unsigned int
#endif

/* PSN Type 4 Header Include File */

/* All structures must be byte packed by the compiler */
#pragma pack(1)

/* This structure is used to hold the channel information variable header info */
typedef struct  {
	double sensorOutVolt;
	double ampGain;
	double adcInputVolt;
} ChannelInfo;

/* Time structure used in the PSNType4 header */
typedef struct  {
	WORD 	year;
	char	month;
	char	day;
	char	hour;
	char	minute;
	char	second;
	char	notUsed;
	UINT	nanoseconds;
} DateTime;

/* Variable information header */
typedef struct  {
	BYTE 	checkNumber;
	BYTE 	id;
	UINT 	length;				// length of the data that follows this header
} VarHeader;

// flags for flags field below
#define NO_CRC		0x00000001
#define NO_MINMAX	0x00000002

/* See http://www.seismicnet.com/psnformat4.html for more information.
   This structure must me 154 bytes long.
*/
typedef struct  {
	char 	headerID[8];
	UINT 	variableHdrLength;
	DateTime startTime;
	double 	startTimeOffset;
	double 	spsRate;
	UINT 	sampleCount;
	UINT	flags;
	char	timeRefType[3];
	char	timeRefStatus;
	BYTE	sampleType;
	BYTE	sampleCompression;
	double	compIncident;
	double	compAzimuth;
	char	compOrientation;
	BYTE	sensorType;
	double	latitude;
	double	longitude;
	double	elevation;
	char	name[6];
	char	compName[4];
	char 	network[6];
	double	sensitivity;
	double	magCorr;
	short	atodBits;
	double	minimum;
	double	maximum;
	double 	mean;
} PSNType4;

/* Go back to default packing */
#pragma pack()

/* eof psn4head.h */
#endif
