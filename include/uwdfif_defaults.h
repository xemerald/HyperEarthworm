#ifndef _UWDFIF_DEFAULTS_H_DEFINED
#define _UWDFIF_DEFAULTS_H_DEFINED

/* This file specifies default file names, etc. for uwdfif */
/* Include file to specify station mapping file for utilities that
   use "uwdfif" data file interface;  this only affects the
   reading of UW-1 style data, and the conversion of UW-1 to UW-2 data  */
#ifdef USBR
# ifdef _WIN32
#  define DEFAULT_STA_MAP "sta.mapping"
#  define DEFAULT_STA_TABLE "all.sta"
#  define DEFAULT_STA_REV_TABLE "rev.sta"
# else
#  define DEFAULT_STA_MAP "/dat/pvsn/tables/sta.mapping"
#  define DEFAULT_STA_TABLE "/dat/pvsn/tables/all.sta"
#  define DEFAULT_STA_REV_TABLE "/dat/pvsn/tables/rev.sta"
# endif
#  define ISTIMECODE(x) (! strcmp((x), "IRGH") || ! strcmp((x), "TCG") || \
			! strcmp((x), "WWVB") || ! strcmp((x), "TRIG") || \
			! strcmp((x), "TIMA") || ! strcmp((x), "TIMB") )
#else
#  define DEFAULT_STA_MAP "sta.mapping"
#  define DEFAULT_STA_TABLE "all.sta"
#  define DEFAULT_STA_REV_TABLE "rev.sta"
#  define ISTIMECODE(x) (! strcmp((x), "IRGH") || ! strcmp((x), "TCG") || \
			 ! strcmp((x), "WWVB") || ! strcmp((x), "TRIG"))
#endif

#endif /* _UWDFIF_DEFAULTS_H_DEFINED */
