/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: earthworm_simple_funcs.h 7119 2018-02-14 23:05:31Z baker $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.10  2009/12/14 19:21:39  scott
 *     Added threading & recovery from bad reconfigure to startstop
 *
 *     Revision 1.9  2007/11/12 21:27:36  paulf
 *     fixed logit so that addslash is now ew_addslash
 *
 *     Revision 1.8  2007/07/20 13:30:29  paulf
 *     added OpenDir and GetNextFileName
 *
 *     Revision 1.7  2006/05/19 20:01:07  dietz
 *     Added one more argument to SendMail() prototype (for "From" field)
 *
 *     Revision 1.6  2006/04/05 19:30:19  stefan
 *     logit fix for log file slash
 *
 *     Revision 1.5  2004/07/29 17:33:53  dietz
 *     Added prototypes for Get*Name() functions in getutil.c
 *
 *     Revision 1.4  2003/07/31 21:12:31  davidk
 *     added get_prog_name2() prototype.
 *
 *     Revision 1.3  2002/06/06 19:34:50  lucky
 *     Added geo_to_km_deg
 *
 *     Revision 1.2  2001/07/01 21:59:42  davidk
 *     Added prototype for geo_to_km() from libsrc/util/geo_to_km.c
 *
 *     Revision 1.1  2001/04/06 21:03:30  davidk
 *     Initial revision
 *
 *
 ************************************************************/

#ifndef EARTHWORM_SIMPLE_FUNCS_H
# define EARTHWORM_SIMPLE_FUNCS_H

/* DO NOT PUT ANY #includes in this file!!!! */

/* This file contains prototypes for earthworm libsrc
   functions that are simple and require no special
   type definitions.  If you have more complex functions
   (semaphores, threads, mutexes, sockets, etc.) you should
   put them in earthworm_complex_funcs.h

   Note, please try to keep functions from the same object
   together in one section of one file.  So all of the logit.c
   stuff should go together.
   Davidk 2001/04/06
*************************************************************/

/* Prototypes for Earthworm utility functions
 ********************************************/
long  GetKey  ( char * );                   /* getutil.c    sys-independent  */
long  GetKeyWithDefault  ( char *, long );  /* getutil.c    sys-independent  */
int   GetInst ( char *, unsigned char * );  /* getutil.c    sys-independent  */
int   GetModId( char *, unsigned char * );  /* getutil.c    sys-independent  */
int   GetType ( char *, unsigned char * );  /* getutil.c    sys-independent  */
int   GetLocalInst( unsigned char * );      /* getutil.c    sys-independent  */
char *GetKeyName  ( long );                 /* getutil.c    sys-independent  */
char *GetInstName ( unsigned char );        /* getutil.c    sys-independent  */
char *GetModIdName( unsigned char );        /* getutil.c    sys-independent  */
char *GetTypeName ( unsigned char );        /* getutil.c    sys-independent  */
char *GetLocalInstName( void );             /* getutil.c    sys-independent  */
void  GetUtil_LoadTable( void );            /* getutil.c    sys-independent  */
int   GetUtil_LoadTableCore( int );         /* getutil.c    sys-independent  */

int  is_logit_initialized();		    /* logit.c      sys-independent  */
void logit_init( const char *, short, int, int );/* logit.c sys-independent  */
void ew_addslash( char * );		    /* logit.c      appends platform- */
					    /*              specific directory */
					    /*              slash to string  */
void html_logit( const char *, const char *, ... );/* logit.c sys-independent */

void logit( const char *, const char *, ... )/* logit.c     sys-independent */
#ifdef __GNUC__
__attribute__ ((format (printf, 2, 3)))     /* gcc will check printf() args */
#endif
;
int  get_prog_name( const char *, char * ); /* logit.c DEPRECATED!!!!!  */
int  get_prog_name2( const char *, char *, int );/* logit.c sys-independent */


int SendMail( char [][60], int, char *, char *, 
              char *, char *, char *, char *, char * );   

/* System-dependent stuff goes here
   ********************************/

int  copyfile( char *, char *, char *, char *, char *, char *, char * );
                                            /* copyfile.c   system-dependent */

int  chdir_ew( char * );                    /* dirops_ew.c  system-dependent */
int  CreateDir( char * );                   /* dirops_ew.c  system-dependent */
int  RecursiveCreateDir( char * );          /* dirops_ew.c  system-dependent */
int  GetFileName( char * );                 /* dirops_ew.c  system-dependent */
int  OpenDir( char * );                     /* dirops_ew.c  system-dependent */
int  GetNextFileName( char * );             /* dirops_ew.c  system-dependent */
int  rename_ew( char *, char * );           /* dirops_ew.c  system-dependent */

int  GetDiskAvail( unsigned * );            /* getavail.c   system-dependent */

int  getsysname_ew( char *, int );          /* getsysname_ew.c sys-dependent */

int SendPage( char * );                     /* sendpage.c   system-dependent */

void sleep_ew( unsigned );                  /* sleep_ew.c   system-dependent */


int  pipe_init ( char *, unsigned long );   /* pipe.c       system-dependent */
int  pipe_put  ( char *, int );             /* pipe.c       system-dependent */
int  pipe_get  ( char *, int, int * );      /* pipe.c       system-dependent */
int  pipe_error( void );                    /* pipe.c       system-dependent */
void pipe_close( void );                    /* pipe.c       system-dependent */

/* from geo_to_km.c */
int geo_to_km (double lat1, double lon1, double lat2, double lon2,
              double* dist, double* azm);
int geo_to_km_deg (double lat1, double lon1, double lat2, double lon2,
              double* dist, double *xdeg, double* azm);

#endif /* EARTHWORM_SIMPLE_FUNCS_H */
