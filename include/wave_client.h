
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: wave_client.h 1111 2002-11-03 00:22:11Z lombard $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.2  2002/11/03 00:22:11  lombard
 *     Protected from multiple includes.
 *
 *     Revision 1.1  2000/02/14 20:05:54  lucky
 *     Initial revision
 *
 *
 */

#ifndef WAVE_CLIENT_H
#define WAVE_CLIENT_H

/*  
 *  wave_client.h
 */

/* Parameters read in from a configuration file 
***********************************************/
static char ServerIPAdr[20];   /* IP address of waveform server           */
static int  ServerPort;	       /* wave_server's well-known port number	  */

/* Prototypes for functions in wave_client.c 
********************************************/
int  wave_request( double,   double,   char * );
int  wave_inquire( double *, double *, 
                   unsigned char *, unsigned char *, unsigned char * );
void wave_client_config( char * );
int  wave_client_setup ( char *, int );

/* Error codes returned by functions in wave_client.c:
   wave_request() returns # buffers received (>0) or an error code (<0)
   wave_inquire() returns 0 on success or an error code (<0) on failure
***********************************************************************/
#define ERR_ALLOC     -1       /* error allocating memory buffer        */
#define ERR_NOHOST    -2       /* trouble getting server's host info    */
#define ERR_SOCKET    -3       /* error with socket connect, send, recv */
#define ERR_FILEIO    -4       /* error opening or writing to file      */
#define ERR_NODATA    -5       /* data requested is not in tank         */
#define ERR_OVRFLW    -6       /* data buffer length > BufferSize       */ 
#define ERR_NOCONFIG  -7       /* configuration file has not been read  */
#define ERR_STRIO     -8       /* error doing string i/o                */

/* Error strings to accompany error codes;
   indexed by absolute value of error number
********************************************/
static char *Wave_Client_Error[] = 
{ "",                                       /* no error     */
  "error allocating memory",                /* ERR_ALLOC    */
  "error getting wave_server's host info",  /* ERR_NOHOST   */
  "trouble with socket",                    /* ERR_SOCKET   */
  "error on file i/o",                      /* ERR_FILEIO   */
  "time period not in tank",                /* ERR_NODATA   */
  "wave_server reply overflows target",     /* ERR_OVERFLW  */
  "wave_client configfile not read",        /* ERR_NOCONFIG */
  "error decoding wave_server response",    /* ERR_STRIO    */
};

#endif
