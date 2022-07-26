
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: sudshead.h 6803 2016-09-09 06:06:39Z et $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.3  2003/10/03 20:13:36  cjbryan
 *     updated to be more compatible with Banfill's recent suds.h
 *
 *     Revision 1.2  2001/04/12 03:03:08  lombard
 *     reorgainized to put the format-specific header items in *head.h
 *     and the putaway-specific items in *putaway.h, of which there
 *     currently is only sacputaway.h
 *     Function protoypes used by putaway.c moved to pa_subs.h
 *
 *
 *
 */

#ifndef SUDSPUTAWAY_H
#define SUDSPUTAWAY_H
/*
// suds.h -- General include for SUDS library
*/
#define _SUDS_VERSION "1.44"
/*
// Revision History:

// Version 1.31 - Thu 30-Jan-1992 11:19, RB
//    Added SUDS_INSTRUMENT structure and expanded datatype in SUDS_DESCRIPTRACE
//    and MUXDATA to include 12 bit unsigned, 12 bit signed, 16 unsigned and
//    16 bit signed data stored as short integers.

// Version 1.32 - 20-Aug-1992 RLB
//    Cleaned up considerably.

// Version 1.40 - 01-Oct-1992 RLB
//    Complete rebuild of I/O functions, overhaul of everything else.

// Version 1.41 - 21-Dec-1992 RLB
//    Fixed file access problems, added datatype '2', minor cleanup.

// Version 1.42 - 14-Feb-1993 RLB
//    Fixed problem when first struct in file has data following it.
//    Added suds_abs_pos( ) function and other minor cleanup.

// Version 1.43 02-Jun-1993, RLB
//    Fixed seek problems when working with multiple files.
//    Added suds_update( ) function.

// Version 1.44 23-Aug-1993, RLB
//    Added SUDS_CHANSET struct.

// Dec 1999, Tom Murray
//    Added #pragma pack's to correct padding of structures

//---------------------------------------------------------------------

//---------------------------------------------------------------------
// DEFINES

// Define code for type of machine suds is being compiled on.
// '6' is 80x86 machine
*/
#define MACHINE         '6'

/* // Define symbols for missing data. */
#define NODATA          "-32767."
#define NOTIME          "-2147472000" /* // Dec 14, 1901 00:00 00 GMT, near largest long */
#define NOCHAR          '_'
#define NOSTRG          ""
#define NOLIST          "0"

/* // Integer defines for standard variable types */
#define CHR             1       /* // char 1 byte */
#define MIN             2       /* // char number 1 byte */
#define STR             3       /* // string */
#define BTS             4       /* // char 1 byte */
#define SHT             5       /* // short 2 bytes */
#define LNG             6       /* // long 4 bytes */
#define FLT             7       /* // float 4 bytes */
#define DBL             8       /* // double 8 bytes */
#define STI             9       /* // struct stat_ident 8 bytes */
#define STT             10      /* // st_time 4 bytes */
#define MST             11      /* // ms_time 8 bytes */
#define LLT             12      /* // lon_lat 8 bytes */
#define CAL             13      /* // struct calib 500 bytes */
#define CPX             14      /* // struct complex 16 bytes */
#define BTW             15      /* // unsigned short 2 bytes */

/* // Structure types or identifiers */
#define NO_STRUCT       0
#define STAT_IDENT      1
#define STRUCTTAG       2
#define TERMINATOR      3
#define EQUIPMENT       4
#define STATIONCOMP     5
#define MUXDATA         6
#define DESCRIPTRACE    7
#define LOCTRACE        8
#define CALIBRATION     9
#define FEATURE         10
#define RESIDUAL        11
#define EVENT           12
#define EV_DESCRIPT     13
#define ORIGIN          14
#ifdef ERROR
#undef ERROR   /* stupid NT */
#endif
#define ERROR           15
#define FOCALMECH       16
#define MOMENT          17
#define VELMODEL        18
#define LAYERS          19
#define COMMENT         20
#define PROFILE         21
#define SHOTGATHER      22
#define CALIB           23
#define COMPLEX         24
#define TRIGGERS        25
#define TRIGSETTING     26
#define EVENTSETTING    27
#define DETECTOR        28
#define ATODINFO        29
#define TIMECORRECTION  30
#define INSTRUMENT      31
#define CHANSET         32

#define TOTAL_STRUCTS   32

/* // Standard variable types redefined for portability and clarity */
/* //typedef void            VOID; */
typedef char            CHAR;   /* // A single ascii character */
typedef unsigned char   MINI;   /* // A 1 byte integer (0 to 255) */
typedef char            STRING; /* // A character string, null byte terminated */
typedef unsigned char   BITS8;  /* // An 8 bit field */
typedef unsigned short  BITS16; /* // A 16 bit field */
typedef short           SH_INT; /* // A 16 bit signed integer */
typedef int32_t         LG_INT; /* // A 32 bit signed integer */
typedef float           FLOAT;  /* // A 32 bit floating point number, IEEE */
typedef double          DOUBLE; /* // A 64 bit double precision number, IEEE */
typedef struct {
   FLOAT   fx;
   FLOAT   fy;
} VECTOR;
typedef struct {
   FLOAT   cr;
   FLOAT   ci;
} COMPLEXX;
typedef struct {
   DOUBLE  dr;
   DOUBLE  di;
} D_COMPLEX;
typedef struct {
   FLOAT   xx;
   FLOAT   yy;
   FLOAT   xy;
} TENSOR;
typedef LG_INT  ST_TIME;     /* // Stamp GMT time in seconds before or after Jan 1, */
/* //      1970, resolution is one second */
typedef DOUBLE  MS_TIME;     /* // GMT time in seconds before or after Jan 1, */
/* //      1970, resolution finer than microseconds */
typedef DOUBLE  LONLAT;      /* // Latitude or longitude in degrees, N and E positive */

/*
//---------------------------------------------------------------------
// STRUCTURE DEFINITIONS

// SUDS_FORM:  Information on each suds structure used for input from ascii,
//             output to ascii, loading or unloading the database, etc.

// antipadding
*/
#pragma pack(1)

typedef struct {
   LG_INT num;
   STRING *meaning;
} SUDS_CODES;

typedef struct {
  LG_INT  fstype;           /* // Structure types or identifiers */
  STRING  *fname;           /* // Name used to identify structure item */
  LG_INT  ftype;            /* // Type of variable in field */
  LG_INT  flength;          /* // Length of variable in the field */
  LG_INT  offset;           /* // Offset of variable pointer from beginning of */
  /* // structure measured in bytes, where first byte */
  /* // of structure=0. */
  STRING  *initval;         /* // Value to initialize structure */
  STRING  *fformat;         /* // Printf type format to read + write field */
  LG_INT  nextfstype;       /* // If structure, this is fstype */
} SUDS_FORM;


/* // SUDS_STRUCTTAG:  Structure to identify structures when archived together */

#define ST_MAGIC   'S'       /* // magic character for sync in structtag */

typedef struct {
  CHAR sync;                /* // The letter S. If not present, error exists. */
  /* // Use to unscramble damaged files or tapes. */
  CHAR machine;             /* // code for machine writing binary file for use */
  /* // in identifying byte order and encoding. */
  SH_INT id_struct;         /* // structure identifier: numbers defined above */
  LG_INT len_struct;        /* // structure length in bytes for fast reading */
  /* // and to identify new versions of the structure */
  LG_INT len_data;          /* // length of data following structure in bytes */
} SUDS_STRUCTTAG;


/* // SUDS_STATIDENT:  Station identification. */

typedef struct {             /* // station component identifier */
  STRING  network[4];       /* // network name */
  STRING  st_name[5];       /* // name of station where equipment is located */
  CHAR    component;        /* // component v,n,e */
  SH_INT  inst_type;        /* // instrument type */
} SUDS_STATIDENT;


/* // SUDS_ATODINFO: Information on the A to D converter */

typedef struct {
  SH_INT  base_address;     /* // base I/O address of this device */
  SH_INT  device_id;        /* // device identifier */
  BITS16  device_flags;     /* // device flags */
  SH_INT  extended_bufs;    /* // number of extended buffers used */
  SH_INT  external_mux;     /* // AtoD external mux control word */
  CHAR    timing_source;    /* // AtoD timing source: i=internal, e=external */
  CHAR    trigger_source;   /* // AtoD trigger source: i=internal, e=external */
} SUDS_ATODINFO;


/* // SUDS_CALIBRATION: Calibration information for a station component */

#define NOCALPTS 30

typedef struct {
  COMPLEXX   pole;          /* // pole */
  COMPLEXX   zero;          /* // zero */
} SUDS_CALIBR;

typedef struct {
  SUDS_STATIDENT ca_name;   /* // station component identification */
  FLOAT maxgain;            /* // maximum gain of calibration curve */
  FLOAT normaliz;           /* // factor to multiply standard calib by to make */
  /* // peak at given frequency=1 */
  SUDS_CALIBR cal[NOCALPTS]; /* // calibration info */
  ST_TIME   begint;         /* // time this calibration becomes effective */
  ST_TIME   endt;           /* // time this calibration is no longer effective */
} SUDS_CALIBRATION;


/* // SUDS_COMMENT:  Comment tag to be followed by the bytes of comment */

typedef struct {
  SH_INT   refer;           /* // structure identifier comment refers to */
  SH_INT   item;            /* // item in structure comment refers to */
  SH_INT   length;          /* // number of bytes in comment */
   SH_INT   unused;
} SUDS_COMMENT;


/* // SUDS_CHANSET:  Associate station/components into sets. */

typedef struct {
  SH_INT  type;             /* // Set type; 0=single channel(s), 1=orthogonal vector */
  SH_INT  entries;          /* // Number of entries in set (these follow as data) */
  STRING  network[4];       /* // Network name */
  STRING  name[5];          /* // Set name */
  ST_TIME active;           /* // Set is defined after this time */
  ST_TIME inactive;         /* // Set is not defined after this time */
} SUDS_CHANSET;

/* // Entries of this form follow SUDS_CHANSET struct. */
typedef struct _CHANSETENTRY {
  LG_INT  inst_num;         /* // Instrument serial number */
  SH_INT  stream_num;       /* // Stream of instrument */
  SH_INT  chan_num;         /* // Channel of stream */
  SUDS_STATIDENT st;        /* // Station/component identifier */
} CHANSETENTRY;


/* // SUDS_DESCRIPTRACE:  Descriptive information about a seismic trace. */
/* //                     Normally followed by waveform. */

typedef struct {
  SUDS_STATIDENT dt_name;   /* // station component identification */
  MS_TIME begintime;        /* // time of first data sample */
  SH_INT  localtime;        /* // minutes to add to GMT to get local time */
  CHAR    datatype;         /* // s = 12 bit unsigned stored as short int, 0 to 4096, */
  /* // q = 12 bit signed stored as short int, -2048 to 2048, */
  /* // u = 16 bit unsigned stored as short int, 0 to 65536 */
  /* // i = 16 bit signed stored as short int, -32767 to 32767, */
  /* // 2 = 24 bit signed integer stored as long, */
  /* // l = 32 bit signed integer stored as long, */
  /* // r = 12 bit data, 4 lsb time stored as short int, */
  /* // f = float (32 bit IEEE real), */
  /* // d = double (64 bit IEEE real), */
  /* // c = complex, */
  /* // v = vector, */
  /* // t = tensor */
  CHAR    descriptor;       /* // g=good, t=telemetry noise, c=calibration, etc */
  SH_INT  digi_by;          /* // agency code who digitized record; 0=original */
  SH_INT  processed;        /* // processing done on this waveform */
  LG_INT  length;           /* // number of samples in trace */
  FLOAT   rate;             /* // samples per second */
  FLOAT   mindata;          /* // minimum value of data (type s,l,f only) */
  FLOAT   maxdata;          /* // maximum value of data (type s,l,f only) */
  FLOAT   avenoise;         /* // average value of first 200 samples (type s,l,f only) */
  LG_INT  numclip;          /* // number of clipped datapoints */
  MS_TIME time_correct;     /* // time correction to be added to begintime */
  FLOAT   rate_correct;     /* // rate correction to be added to rate */
} SUDS_DESCRIPTRACE;


/* // SUDS_DETECTOR:  Information on detector program being used */

typedef struct {
  CHAR    dalgorithm;       /* // triggering algorithm: x=xdetect, m=mdetect */
  /* // e=eqdetect */
  CHAR    event_type;       /* // c=calibration, e=earthquake, E=explosion, */
  /* // f=free run, n=noise, etc. */
  CHAR    net_node_id[10];  /* // network node identification */
  FLOAT   versionnum;       /* // software version number */
  LG_INT  event_number;     /* // unique event number assigned locally. */
  LG_INT  spareL;           /* // spare */
} SUDS_DETECTOR;


/* // SUDS_EQUIPMENT:  Equipment making up a station/component. Primarily used for */
/* //                  maintenance but may be referenced by researcher. One or more */
/* //                  structures exist for each piece of equipment making up a */
/* //                  station/component. */

typedef struct {
  SUDS_STATIDENT this;      /* // identifier of this piece of equipment */
  SUDS_STATIDENT previous;  /* // next piece of equipment toward sensor */
  SUDS_STATIDENT next;      /* // next piece of equipment toward recorder */
  STRING  serial[8];        /* // serial number */
  SH_INT  model;            /* // model such as L4, HS10, etc. */
  SH_INT  knob1;            /* // knob setting or series resistor value of Lpad */
  SH_INT  knob2;            /* // knob setting or shunt  resistor value of Lpad */
  SH_INT  reason;           /* // reason change was made */
  FLOAT   frequency;        /* // sensor corner frequency, vco freq, transmitter */
  /* // frequency, etc. */
  ST_TIME effective;        /* // date/time these values became effective */
} SUDS_EQUIPMENT;


/* // SUDS_ERROR:  Error matrix */

typedef struct {
  FLOAT   covarr[10];       /* // covariance matrix */
} SUDS_ERROR;


/* // SUDS_EVENT:  General information about an event. */

typedef struct {
  SH_INT  authority;        /* // organization processing the data */
  LG_INT  number;           /* // unique event number assigned by organization */
  SH_INT  felt;             /* // number of felt reports */
  CHAR    mintensity;       /* // maximum Modified Mercali Intensity */
  CHAR    ev_type;          /* // e=earthquake, E=explosion, n=nuclear, */
  /* // i=icequake, b=b_type, n=net, r=regional, */
  /* // t=teleseism, c=calibration, n=noise */
  CHAR    tectonism;        /* // observed u=uplift, s=subsidence, S=strikeslip */
  /* // faulting, N=normal faulting, T=thrust */
  CHAR    waterwave;        /* // seiche, tsunami, etc. */
  CHAR    mechanism;        /* // t=thrust, s=strike-slip, n=normal, e=explosive */
  CHAR    medium;           /* // medium containing explosion or event */
  FLOAT   size;             /* // magnitude or pounds TNT for explosions */
} SUDS_EVENT;


/* // SUDS_EVENTSETTING:  Settings for earthquake trigger system */

typedef struct {
  STRING  netwname[4];      /* // network name */
  MS_TIME beginttime;       /* // time these values in effect */
  SH_INT  const1;           /* // trigger constant 1 */
  SH_INT  const2;           /* // trigger constant 2 */
  SH_INT  threshold;        /* // trigger threshold */
  SH_INT  const3;           /* // trigger constant 3 */
  FLOAT   minduration;      /* // minimum duration for event */
  FLOAT   maxduration;      /* // maximum duration for event */
  CHAR    algorithm;        /* // triggering algorithm: x=xdetect, m=mdetect */
  /* // e=eqdetect */
  CHAR    spareK;           /* // spare */
  SH_INT  spareI;           /* // spare */
} SUDS_EVENTSETTING;


/* // SUDS_EVDESCR:  Descriptive information about an event typically used for */
/* //                major, destructive earthquakes. This structure is typically */
/* //                associated with EVENT structure. */

typedef struct {
  STRING  eqname[20];  /* // Popular name used to refer to this earthquake */
  STRING  country[16]; /* // country of earthquake */
  STRING  state[16];   /* // state, province or other political subdivision */
  SH_INT  localtime;   /* // hours to add to GMT to get local time */
   SH_INT  spareB;
} SUDS_EVDESCR;


/* // SUDS_FEATURE:  Observed phase arrival time, amplitude, and period. */

typedef struct {
  SUDS_STATIDENT fe_name;   /* // station component identification */
  SH_INT  obs_phase;        /* // observed phase code */
  CHAR    onset;            /* // wave onset descriptor, i or e */
  CHAR    direction;        /* // first motion: U,D,+,- */
  SH_INT  sig_noise;        /* // ratio ampl. of first peak or trough to noise */
  CHAR    data_source;      /* // i=interactive,a=automatic,r=rtp, or user code */
  CHAR    tim_qual;         /* // timing quality given by analyst: 0-4, etc. */
  /* //      n=ignore timing */
  CHAR    amp_qual;         /* // amplitude quality given by analyst: 0-4, etc. */
  /* //      n=ignor amplitude information */
  CHAR    ampunits;         /* // units amplitude measured in: d=digital counts */
  /* //      m=mm on develocorder, etc. */
  SH_INT  gain_range;       /* // 1 or gain multiplier if gain range in effect */
  MS_TIME time;             /* // phase time, x value where pick was made */
  FLOAT   amplitude;        /* // peak-to-peak amplitude of phase */
  FLOAT   period;           /* // period of waveform measured */
  ST_TIME time_of_pick;     /* // time this pick was made */
  SH_INT  pick_authority;   /* // organization processing the data */
  SH_INT  pick_reader;      /* // person making this pick */
} SUDS_FEATURE;


/* // SUDS_FOCALMECH:  General information about a focal mechanism. */

typedef struct {
  FLOAT   astrike;          /* // strike of plane a */
  FLOAT   adip;             /* // dip of plane a */
  FLOAT   arake;            /* // rake of plane a */
  FLOAT   bstrike;          /* // strike of plane b */
  FLOAT   bdip;             /* // dip of plane b */
  FLOAT   brake;            /* // rake of plane b */
  CHAR    prefplane;        /* // preferred plane a or b or blank */
   CHAR    spareC[3];
} SUDS_FOCALMECH;

/* // SUDS_INSTRUMENT: Instrument hardware settings, mainly PADS related */
/* //                  added by R. Banfill, Jan 1991 */

typedef struct {
  SUDS_STATIDENT in_name;   /* // Station component identification */
  SH_INT  in_serial;        /* // Instrument serial number */
  SH_INT  comps;            /* // Number of components recorded by instrument */
  SH_INT  channel;          /* // Actual channel number on recorder */
  CHAR    sens_type;        /* // Sensor type; a=accel, v=vel, d=disp... */
  CHAR    datatype;         /* // see SUDS_DESCRIPTRACE.datatype */
  LG_INT  void_samp;        /* // Invalid or void sample value */
  FLOAT   dig_con;          /* // Digitizing constant (counts / volt) */
  FLOAT   aa_corner;        /* // Anti-alias filter corner frequency (Hz) */
  FLOAT   aa_poles;         /* // Anti-alias filter poles */
  FLOAT   nat_freq;         /* // Transducer natural frequency (Hz) */
  FLOAT   damping;          /* // Transducer damping coeff. */
  FLOAT   mot_con;          /* // Transducer motion constant (volts / GMU) */
  FLOAT   gain;             /* // Amplifier gain (dB) */
  FLOAT   local_x;          /* // Local coordinate X (meters) */
  FLOAT   local_y;          /* // Local coordinate Y (meters) */
  FLOAT   local_z;          /* // Local coordinate Z (meters) */
  ST_TIME effective;        /* // Time these setting took effect */
  FLOAT   pre_event;        /* // Pre-event length (IST+pre_event=trigger time) */
  SH_INT  trig_num;         /* // Trigger number on instrument */
  STRING  study[6];         /* // Study name, used to insure unique station names */
  SH_INT  sn_serial;        /* // Sensor serial number */
} SUDS_INSTRUMENT;

/* // SUDS_LAYERS:  Velocity layers. */

typedef struct {
  FLOAT   thickness;        /* // thickness in kilometers */
  FLOAT   pveltop;          /* // p velocity at top of layer */
  FLOAT   pvelbase;         /* // p velocity at base of layer */
  FLOAT   sveltop;          /* // s velocity at top of layer */
  FLOAT   svelbase;         /* // s velocity at base of layer */
  SH_INT  function;         /* // velocity function in layer: 0=constant, */
  /* //    1=linear, 2=exponential, etc. */
   SH_INT  spareF;
} SUDS_LAYERS;


/* // SUDS_LOCTRACE:  Location of trace. */

typedef struct {
  SUDS_STATIDENT lt_name;   /* // station component identification */
  STRING  *fileloc;         /* // pointer to pathname in file system */
  STRING  *tapeloc;         /* // pointer to name of tape or offline storage */
  LG_INT  beginloc;         /* // bytes from begining of file to trace */
} SUDS_LOCTRACE;


/* // SUDS_MOMENT:  Moment tensor information. */

typedef struct {
  BITS8   datatypes;        /* // sum of: 1=polarities, 2=amplitudes, */
  /* // 4=waveforms, etc. */
  CHAR    constraints;      /* // solution constrained: d=deviatoric, */
  /* // c=double couple */
   CHAR    spareD[2];
  FLOAT   sc_moment;        /* // scalar moment */
  FLOAT   norm_ten[6];      /* // normalized moment tensor */
} SUDS_MOMENT;


/* // SUDS_MUXDATA:  Header for multiplexed data */

typedef struct {
  STRING  netname[4];       /* // network name */
  MS_TIME begintime;        /* // time of first data sample */
  SH_INT  loctime;          /* // minutes to add to GMT to get local time */
  SH_INT  numchans;         /* // number of channels: if !=1 then multiplexed */
  FLOAT   dig_rate;         /* // samples per second */
  CHAR    typedata;         /* // see SUDS_DESCRIPTRACE.datatype */
  CHAR    descript;         /* // g=good, t=telemetry noise, c=calibration, etc */
  SH_INT  spareG;           /* // spare */
  LG_INT  numsamps;         /* // number of sample sweeps. Typically not known */
  /* // when header is written, but can be added later */
  LG_INT  blocksize;        /* // number of demultiplexed samples per channel if */
  /* // data is partially demultiplexed, otherwise=0 */
} SUDS_MUXDATA;


/* // SUDS_ORIGIN: Information about a specific solution for a given event */

typedef struct {
  LG_INT  number;           /* // unique event number assigned by organization */
  SH_INT  authority;        /* // organization processing the data */
  CHAR    version;          /* // version of solution within organization */
  CHAR    or_status;        /* // processing status: f=final, a=automatic, etc */
  CHAR    preferred;        /* // p=preferred location */
  CHAR    program;          /* // name of processing program  h=hypo71, */
  /* // l=hypolayer, i=isc, c=centroid, etc. */
  CHAR    depcontrl;        /* // depth control: f=fixed, etc. */
  CHAR    convergence;      /* // hypocentral convergence character */
  LG_INT  region;           /* // geographic region code assigned locally */
  MS_TIME orgtime;          /* // origin time */
  LONLAT  or_lat;           /* // latitude, north is plus */
  LONLAT  or_long;          /* // longitude, east is plus */
  FLOAT   depth;            /* // depth in kilometers, + down */
  FLOAT   err_horiz;        /* // horizontal error in km */
  FLOAT   err_depth;        /* // vertical error in km */
  FLOAT   res_rms;          /* // rms of residuals */
  STRING  crustmodel[6];    /* // code for model used in this location */
  SH_INT  gap;              /* // azimuthal gap in degrees */
  FLOAT   nearstat;         /* // distance in km to nearest station */
  SH_INT  num_stats;        /* // number of stations reporting phases */
  SH_INT  rep_p;            /* // number of p phases reported */
  SH_INT  used_p;           /* // number of p times used in the solution */
  SH_INT  rep_s;            /* // number of s phases reported */
  SH_INT  used_s;           /* // number of s times used in the solution */
  SH_INT  mag_type;         /* // magnitude type: coda,tau,xmag ml,mb,ms,mw */
  SH_INT  rep_m;            /* // number of magnitude readings reported */
  SH_INT  used_m;           /* // number of magnitude readings used */
  FLOAT   magnitude;        /* // magnitude value */
  FLOAT   weight;           /* // average magnitude weight */
  FLOAT   mag_rms;          /* // rms of magnitude */
  ST_TIME effective;        /* // time this solution was calculated */
} SUDS_ORIGIN;


/* // SUDS_PROFILE:  Grouping of shotgathers by profile. */

typedef struct {
   int junk1;
  /* // What is your suggestion? */
} SUDS_PROFILE;


/* // SUDS_RESIDUAL:  Calculated residuals for arrival times, magnitudes, etc. */

typedef struct {
  LG_INT  event_num;        /* // unique event number */
  SUDS_STATIDENT re_name;   /* // station component identification */
  SH_INT  set_phase;        /* // phase code set for this solution */
  CHAR    set_tim_qual;     /* // timing quality assigned for this soln: 0-4 */
  CHAR    set_amp_qual;     /* // amplitude quality assigned for this soln: 0-4 */
  FLOAT   residual;         /* // traveltime residual or phase magnitude */
  FLOAT   weight_used;      /* // weight used in this solution */
  FLOAT   delay;            /* // delay time or station correction used */
  FLOAT   azimuth;          /* // azimuth event to station, 0 north */
  FLOAT   distance;         /* // distance in km event to station */
  FLOAT   emergence;        /* // angle of emergence from source, 0=down,180=up */
} SUDS_RESIDUAL;


/* // SUDS_SHOTGATHER:  Grouping of waveforms by source event */

typedef struct {
   int junk2;
  /* // What is your suggestion? */
} SUDS_SHOTGATHER;


/* // SUDS_STATIONCOMP:  Generic station component information */

typedef struct {
  SUDS_STATIDENT sc_name;   /* // station component identification */
  SH_INT  azim;             /* // component azimuth clockwise from north */
  SH_INT  incid;            /* // component angle of incidence from vertical */
  /* //      0 is vertical, 90 is horizontal */
  LONLAT  st_lat;           /* // latitude, north is plus */
  LONLAT  st_long;          /* // longitude, east is plus */
  FLOAT   elev;             /* // elevation in meters */
  CHAR    enclosure;        /* // d=dam, n=nuclear power plant, v=underground */
  /* //      vault, b=buried, s=on surface, etc. */
  CHAR    annotation;       /* // annotated comment code */
  CHAR    recorder;         /* // type device data recorded on */
  CHAR    rockclass;        /* // i=igneous, m=metamorphic, s=sedimentary */
  SH_INT  rocktype;         /* // code for type of rock */
  CHAR    sitecondition;    /* // p=permafrost, etc. */
  CHAR    sensor_type;      /* // sensor type: d=displacement, v=velocity, */
  /* // a=acceleration, t=time code */
  CHAR    data_type;        /* // see SUDS_DESCRIPTRACE.datatype */
  CHAR    data_units;       /* // data units: d=digital counts, v=millivolts, */
  /* // n=nanometers (/sec or /sec/sec) */
  CHAR    polarity;         /* // n=normal, r=reversed */
  CHAR    st_status;        /* // d=dead, g=good */
  FLOAT   max_gain;         /* // maximum gain of the amplifier */
  FLOAT   clip_value;       /* // +-value of data where clipping begins */
  FLOAT   con_mvolts;       /* // conversion factor to millivolts: mv per counts */
  /* //      0 means not defined or not appropriate */
  /* // max_ground_motion=digital_sample*con_mvolts* */
  /* // max_gain */
  SH_INT  channel;          /* // a2d channel number */
  SH_INT  atod_gain;        /* // gain of analog to digital converter */
  ST_TIME effective;        /* // date/time these values became effective */
  FLOAT   clock_correct;    /* // clock correction in seconds. */
  FLOAT   station_delay;    /* // seismological station delay. */
} SUDS_STATIONCOMP;


/* // SUDS_TERMINATOR:  Structure to end a sequence of related structures when */
/* //                   loaded in a serial file or on a serial device. */

typedef struct {
  SH_INT  structid;         /* // id for structure at beginning of this sequence */
   SH_INT  spareA;
} SUDS_TERMINATOR;


/* // SUDS_TIMECORRECTION:  Time correction information. */

typedef struct {
  SUDS_STATIDENT tm_name;   /* // time trace station id used to determine */
  /* // correction. */
  MS_TIME time_correct;     /* // time correction to be added to begintime */
  FLOAT   rate_correct;     /* // rate correction to be added to rate */
  CHAR    sync_code;        /* // synchronization code as follows: */
  /* //   0 = total failure,   1 = 1 second synch, */
  /* //   2 = 10 second synch, 3 = minute synch, */
  /* //   4, 5 = successful decode. */
  CHAR    program;          /* // program used to decode time: */
  /* //   e = irige, c = irigc */
  ST_TIME effective_time;   /* // time this correction was calculated */
   SH_INT  spareM;
} SUDS_TIMECORRECTION;


/* // SUDS_TRIGGERS:  Earthquake detector trigger statistics */

typedef struct {
  SUDS_STATIDENT tr_name;   /* // station component identification */
  SH_INT  sta;              /* // short term average */
  SH_INT  lta;              /* // long term average; pre_lta for xdetect */
  SH_INT  abs_sta;          /* // short term absolute average */
  SH_INT  abs_lta;          /* // long term absolute average */
  SH_INT  trig_value;       /* // value of trigger level (eta) */
  SH_INT  num_triggers;     /* // number of times triggered during this event */
  MS_TIME trig_time;        /* // time of first trigger */
} SUDS_TRIGGERS;


/* // SUDS_TRIGSETTING:  Settings for earthquake trigger system */

typedef struct {
  STRING  netwname[4];      /* // network name */
  MS_TIME beginttime;       /* // time these values in effect */
  SH_INT  const1;           /* // trigger constant 1 */
  SH_INT  const2;           /* // trigger constant 2 */
  SH_INT  threshold;        /* // trigger threshold */
  SH_INT  const3;           /* // trigger constant 3 */
  SH_INT  const4;           /* // trigger constant 4 */
  SH_INT  wav_inc;          /* // weighted average increment */
  FLOAT   sweep;            /* // trigger sweep time in seconds */
  FLOAT   aperture;         /* // seconds for coincident station triggers */
  CHAR    algorithm;        /* // triggering algorithm: x=xdetect, m=mdetect */
  /* // e=eqdetect */
  CHAR    spareJ;           /* // spare */
  SH_INT  spareI;           /* // spare */
} SUDS_TRIGSETTING;


/* // SUDS_VELMODEL:  Velocity model */

typedef struct {
  STRING  netname[4];       /* // network name */
  STRING  modelname[6];     /* // model name */
   CHAR    spareE;
  CHAR    modeltype;        /* // p=profile A to B, a=area within corners A B */
  LONLAT  latA;             /* // latitude of point A, north is plus */
  LONLAT  longA;            /* // longitude of point A, east is plus */
  LONLAT  latB;             /* // latitude of point B, north is plus */
  LONLAT  longB;            /* // longitude of point B, east is plus */
  ST_TIME time_effective;   /* // time this model was created */
} SUDS_VELMODEL;

/* //--------------------------------------------------------------------- */
/* // SUDS structure */

typedef struct _SUDS {
   int type;
   int32_t data_len;
   union {
      SUDS_STATIDENT st;
      SUDS_ATODINFO ad;
      SUDS_CALIBRATION ca;
      SUDS_COMMENT co;
      SUDS_CHANSET cs;
      SUDS_DESCRIPTRACE dt;
      SUDS_DETECTOR de;
      SUDS_EQUIPMENT eq;
      SUDS_ERROR er;
      SUDS_EVENT ev;
      SUDS_EVENTSETTING es;
      SUDS_EVDESCR ed;
      SUDS_FEATURE fe;
      SUDS_FOCALMECH fo;
      SUDS_INSTRUMENT in;
      SUDS_LAYERS la;
      SUDS_LOCTRACE lo;
      SUDS_MOMENT mo;
      SUDS_MUXDATA mu;
      SUDS_ORIGIN or;
      SUDS_PROFILE pr;
      SUDS_RESIDUAL re;
      SUDS_SHOTGATHER sh;
      SUDS_STATIONCOMP sc;
      SUDS_TERMINATOR te;
      SUDS_TIMECORRECTION tc;
      SUDS_TRIGGERS tr;
      SUDS_TRIGSETTING ts;
      SUDS_VELMODEL vm;
   } _SUDSUNION;
} SUDS;


/* // remove anti-padding */
/* // syntax may be wrong - might be #pragma pack instead */

#pragma pack()

/* //--------------------------------------------------------------------- */
/* // Function prototypes for SUDS library functions */

/* // Manifest constants ------------------------------------------------- */
#define SUDS_EOF      -1
#define SUDS_READONLY  1
#define SUDS_READWRITE 2
#define SUDS_APPEND    3
#define SUDS_CREATE    4

#ifndef TRUE
#define TRUE           1
#endif
#ifndef FALSE
#define FALSE          0
#endif

/* // Input / output functions ------------------------------------------- */
#ifndef _SUDS_IO_

/* // Open and close functions */
extern int suds_open( char *filespec, int mode );
extern int suds_close( int fd );

/* // Read functions */
extern int suds_read( int fd, SUDS *suds );
extern int suds_read_data( int fd, void *ptr, int32_t len );

/* // Write functions */
extern int suds_write( int fd, SUDS *suds );
extern int suds_write_data( int fd, void  *ptr, int32_t len );
extern int suds_update( int fd, SUDS *suds );
extern int suds_flush( int fd );

/* // Navigation functions */
extern int32_t suds_pos( int fd );
extern int32_t suds_abs_pos( int fd );
extern int suds_seek( int fd, int32_t pos );
extern int suds_rewind( int fd );

/* // Retrieve error messages */
extern char *suds_get_err( void );

#endif

/* // Structure initialization ------------------------------------------- */
#ifndef _SUDS_INIT_

extern void suds_init( SUDS *suds );

#endif

/* // Time functions ----------------------------------------------------- */
#ifndef _SUDS_TIME_

extern double get_mstime( void );
extern double make_mstime( int year, int month, int day, int hour,
                           int minute, double second );
extern int decode_mstime( double mstime, int *year, int *month, int *day,
                          int *hour, int *minute, double *second );
extern char *list_mstime( double mstime, int format );
extern int yrday( int month, int day, int leap );
extern void mnday( int doy, int leap, int *month, int *day );
extern int isleap( int year, int cal );

#endif

/* //--------------------------------------------------------------------- */

#endif
