#pragma once

#ifdef _WIN32
# include <windows.h>
# include <io.h>
# include <direct.h>
# define strcasecmp _stricmp
# define strncasecmp _strnicmp
# define popen _popen
# define pclose _pclose
# define fstat _fstat
# define stat _stat
# define isatty _isatty	/* deprecated POSIX usage */
# define creat _creat
# define unlink _unlink
# define mkdir _mkdir
# define write _write
# define close _close
# define fileno _fileno
# define swab _swab
# define cabs _cabs
# define fileno _fileno
# define _USE_MATH_DEFINES
# ifndef DllExport
#  define DllExport	__declspec( dllexport )
# endif
# ifndef DllImport
#  define DllImport	__declspec( dllimport )
# endif
# ifndef WIN32
#  define WIN32
# endif
#else
# include <unistd.h>
# include <sys/file.h>
# include <sys/time.h>
# include <rpc/rpc.h>
# include <dirent.h>
# ifndef DllExport
#  define DllExport
# endif
# ifndef DllImport
#  define DllImport	extern
# endif
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef int BOOL;

#ifdef _WIN32
# define PATH_SEP ('\\')
#else
# define PATH_SEP ('/')
#endif
#ifndef BASENAME
# define BASENAME(s)	(strrchr((s),PATH_SEP) == NULL ? (s) : strrchr((s),PATH_SEP)+1)
#endif

#ifndef LAST_CHAR
# define LAST_CHAR(s)	((s)[strlen(s)-1])
#endif
#ifndef MIN
# define MIN(a,b)	((a) <= (b) ? (a) : (b))
#endif
#ifndef MAX
# define MAX(a,b)	((a) >= (b) ? (a) : (b))
#endif
#ifndef ABS
# define ABS(x)	((x) < 0 ? -(x) : (x))
#endif
#ifndef SIGN
# define SIGN(x)	(((x) < 0) ? -1 : 1)
#endif
#ifndef CHS
# define CHS(x)	(x) = (-x)
#endif
#ifndef ROUND
# define ROUND(x)	((int)((x) + ((x) < 0 ? -.5 : .5)))
#endif
#ifndef TRUE
# define TRUE (1)
#endif
#ifndef FALSE
# define FALSE (0)
#endif

/*  Master header block is defined below */

struct masthead {
    short nchan; /* Number of channels; not used for UW-2 */
    int lrate;   /* Sample rate, samples per 1000 seconds; not used in UW-2 */
    int lmin;    /* Reference time for Carl's date routine (grgmin) */
    int lsec;    /* Reference second from above min in microseconds */
    int length;  /* Number of samples per channel in total record
                    Note that in UW-2, this is defunct */
    short tapenum;  /* Original 11/34 tape number; now run number */
    short eventnum; /* Event number on 11/34 tape; now sequence number */
    short flg[10];  /* Extra user defined flags for expansion.
            Some current usages follow:
            flg[0] is -3 when lmin is not set but the digitization
                   rate and seconds modulo 10 have been set.
            flg[0] is -2 when lmin and lsec have not been set.
            flg[0] is -1 when lmin is set to within plus or minus 3.
            flg[0] is what you get from the online demultiplexer.
                   it usually means  that the time is set
                   to within 3 seconds and the digrate is good
                   to about 3 places, but this may be in error:
                a) if the online clock was not set at reboot
                       time then the year will not be 198?.
                b) if the digrate is exactly 100 in old 64 chan
                   stuff then this is not even close.
                   The digrate was constant throughout the
                   64-chan configuration.
            flg[0] is  1 when ping is through with it.  This
                   generally means the time is set to within
                   .2 second and the digrate has not been reset.
            flg[0] is  2  when the digrate has been set to 4
                significant figures and the starting time
                has been set to within one sampling interval.
            flg[1] is the logical OR of: low bit: 0 -> Squash Lock On
                                         low bit: 1 -> Squash Lock Off
                                         2nd bit: 0 -> Not Squashed
                                         2nd bit: 1 -> Squashed
            flg[2] is number of times the file has been merged.
            flg[3] is 0 usually and 1 if the station names have been modified.
            flg[4] is decimation factor if slashed
            flg[5] is the channel number (1 - hm.nchan) of the
                time code from which the time was set.
            flg[6] is used by the 5-day merge package.  It is
                set to 1 after the station names have been corrected.
            flg[7] is used by 'stack' to tell how many files have
                been stacked onto this one.
*/
    char extra[10];   /* extra codes for expansion
            extra[0]  is currently used as an event type flag
            extra[1] is set to ' ' (blank) or 'I' if integer data are IEEE
		conformant; 'D' if data are DEC style byte reversed;
		'I' or 'D' are preferred forms for new format
            extra[2] is set to ' ' (blank) or '1' if UW-1 format is used;
               '2' for UW-2 format;  '1' and '2' are preferred forms for
	       new format
*/
    char comment[80]; /* 80 optional comment characters */
};

/* Channel headers for UW-1 and UW-2 are defined below */

struct chhead1 {  /* UW-1 station headers; one per channel */
    char name[6]; /* Station name (4 characters and a null) */
    short lta;    /* long term average */
    short trig;   /* Trigger  (positive for trigger on) */
    short bias;   /* DC offset */
}; /* length of chhead1 = 12 bytes */

struct chhead2 { /* UW-2 channel headers; one per channel */
    int chlen;    /* channel length in samples */
    int offset;   /* start offset of channel; bytes rel. to start of file */
    int start_lmin;    /* start time in min; same def. as lmin in masthead */
    int start_lsec;    /* start time offset relative to lmin; u-sec */
    int lrate;    /* sample rate in samples per 1000 secs */
    int expan1;	/* expansion field for long integers */
    short lta;    /* long term average; same as chhead1 */
    short trig;   /* Trigger (positive for trigger on); same as chhead1 */
    short bias;   /* DC offset; same as chhead1 */
    short fill;   /* Fill short int so short int block 8 bytes long */
    char name[8]; /* station name (4 characters and null) */
    char fmt[4];  /* first char designates data fmt (S, L, or F) */
    char compflg[4]; /* component id as per Seed Appen I */
    char chid[4]; /* unique channel id; user defined */
    char src[4]; /* expansion for characters */
}; /* length of chhead2 = 56 bytes */

/* Specification of structure for indexing expansion structures */

struct expanindex {
    char structag[4]; /* tag to indicate specific structure;  currently
	"CH2" is the tag for the UW-2 channel structures */
    int numstructs; /* number of structures to read; for the "CH2"
	   structures, this would be the number of channels */
    int offset; /* file offset for beginning of structure; for "CH2"
	   this would be the beginning of the sequence of channel header
	   structures */
}; /* length of expanstruct = 12 bytes */

/* Alternate time structure for UWDFdhref_stime(), UWDFchref_stime(), and
   UWDFset_ref_stime() */

struct stime {
    int yr, mon, day, hr, min;
    float sec;
}; /* lenth of stime = 24 bytes */

struct sta_loc {
    float lat, lon, elev;
    char plflg, stcom[80];
}; /* elemental structure for station location info */

#ifdef __LIBUWDFIF
# define extern
#endif
#include "uwdfif_proto.h"
#ifdef __LIBUWDFIF
# undef extern
#endif
#include "uwdfif_defaults.h"
