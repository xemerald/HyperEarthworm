
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: wave_client.c 6821 2016-09-19 15:36:06Z paulf $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.2  2001/04/17 17:30:42  davidk
 *     added an explicit typecast to get rid of a compiler warning on NT.
 *
 *     Revision 1.1  2000/02/14 18:51:48  lucky
 *     Initial revision
 *
 *
 */

/*
   wave_client.c

   Contains functions for connecting to a wave_server's socket, making
   a request for a time-chunk of waveforms, and writing the requested
   data to a disk file.  
   Library routines for talking to the original "wave_server".

   Returns the number of waveform messages received.
*/

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <earthworm.h>
#include <kom.h>
#include "wave_client.h"

#define MAX_MSG_LEN  126

void   SocketClose( SOCKET );    /* socket_ew.c  system-dependent */

/* Global variables
 ******************/
static int      Config_Init = 0;      /* 1=config file has been read	    */
static struct hostent    *Host;       /* Contains host ip address           */
static struct sockaddr_in Sin;        /* Server's socket address stucture   */
static unsigned long      Addr;       /* binary form of server's ip address */

/*********************************************************
 *                       wave_request                    *
 *     Connects to socket, requests data, writes file    *
 *********************************************************/
int wave_request( double  ton,       /* start-time of requested waveforms */
	          double  toff,      /* end-time of requested waveforms   */
                  char   *pathfile ) /* where to write file of waveforms  */
{
   char          request[MAX_MSG_LEN];
   char         *buf, *bufptr;
   FILE         *fp;
   long          maxbuflen;
   long          databuflen;
   long          ntoget;
   long          getatonce;
   SOCKET        sd;                   /* Socket descriptor */
   int           ir, nr;
   int           ndatabuf, nbufrec;
   int           length;
   int           ret;
   unsigned char inst, module, type;   /* logo of incoming waveform messages */
   char          c;
   int           i, m, t;

/* Make sure configuration file has been read
 ********************************************/
   if ( !Config_Init )
   {
      /*printf("wave_request: Config file has not been read.\n" );*/ /*DEBUG*/
      return( ERR_NOCONFIG );
   }

/* Open the file that will contain the trace data
 ************************************************/
   fp = fopen( pathfile, "wb" );
   if ( fp == NULL )
   {
      /*printf("wave_request: Error opening output file <%s>\n",
              pathfile );*/ /*DEBUG*/
      return( ERR_FILEIO );
   }

/* Set up socket connection here
 *******************************/
   if ( (sd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 )
   {
      logit( "t", "wave_request: Socket call failed.\n" );
      SocketPerror( "wave_request: Socket call failed" );
      fclose( fp );
      return( ERR_SOCKET );
   }

   if ( connect( sd, (struct sockaddr *)&Sin, sizeof(Sin) ) == -1 )
   {
      logit( "t", "wave_request: Connect to wave_server host failed.\n" );
      SocketPerror( "wave_request: Connect to wave_server host failed" );
      ret = ERR_SOCKET;
      goto done;
   }

/* Build and send a request to the wave_server
 *********************************************/
   sprintf( request, "REQ: %15.2f %15.2f\n", ton, toff );
   length = (int) strlen(request);
   if ( send( sd, request, length, 0 ) == -1 )
   {
      logit( "t", "wave_request: Error sending waveform request.\n" );
      SocketPerror( "wave_request: Error sending waveform request" );
      ret = ERR_SOCKET;
      goto done;
   }

/* Start reading the socket, one character at a time
 ***************************************************/
   ir = 0;
   do
   {
      if ( ir == MAX_MSG_LEN ) {   	/* stop if there's no more room */
	 /*printf("wave_request: wave_server reply too long.\n ");*/ /*DEBUG*/
         ret = ERR_OVRFLW;
         goto done;
      }
      nr = recv( sd, &c, 1, 0 );  	/* try to get a char from socket*/
      if ( nr == 1 ) {         		/* got a character; save it   	*/
         request[ir++] = c;      	
      }
      else if ( nr == -1 ) {          	/* trouble reading socket 	*/
         logit( "e", "wave_request: Error on socket recv\n" );
         ret = ERR_SOCKET;
         goto done;
      }
   } while( c != '\n' );		/* last char of reply	  	*/
   request[ir] = '\0';                  /* null-terminate the reply     */

/* Process inquiry reply from wave_server
 ****************************************/
   if( sscanf( request, "%d %d %d %ld %d",
               &i, &m, &t, &maxbuflen, &ndatabuf ) < 5 )
   {
	logit("e","wave_request: error decoding wave_server reply:\n%s\n",
               request );
        inst     = 0;
        module   = 0;
        type     = 0;
        ndatabuf = 0;
        ret      = ERR_STRIO;
        goto done;
   }
   if ( i<256 && i>0 ) {
        inst = (unsigned char) i;
   }
   else {
	logit( "e", "wave_request: invalid installation id <%d>\n", i );
        inst = 0;
   }
   if ( m<256 && m>0 ) {
        module = (unsigned char) m;
   }
   else {
	logit( "e", "wave_request: invalid module id <%d>\n", m );
        module = 0;
   }
   if ( t<256 && t>0 ) {
        type = (unsigned char) t;
   }
   else {
	logit( "e", "wave_request: invalid msg type <%d>\n", t );
        type = 0;
   }
   /*printf("wave_request: accepting %d msg(s) of logo Inst:%d Mod:%d Type:%d\n",
           ndatabuf, (int) inst, (int) module, (int) type );*/ /*DEBUG*/

/* Bail out if there's no data coming
 ************************************/
   if ( ndatabuf == 0 )
   {
      /*printf("wave_request: Requested time period not in tank\n");*/ /*DEBUG*/
      ret = ERR_NODATA;
      goto done;
   }

/* Allocate memory for incoming messages
 ***************************************/
   buf = (char *) malloc( (size_t) maxbuflen );
   if ( buf == (char *)NULL )
   {
      /*printf("wave_request: Error allocating memory buffer.\n");*/ /*DEBUG*/
      ret = ERR_ALLOC;
      goto done;
   }

/* Loop reading messages from socket
 ***********************************/
   nbufrec = 0;
   do
   {

   /* Read the size of the next message from the socket
    ***************************************************/
      ir = 0;
      do
      {
         if ( ir == maxbuflen ) {   	/* stop if there's no more room */
             ret = ERR_OVRFLW;
             goto almostdone;
         }
         nr = recv( sd, &c, 1, 0 );  	/* try to get a char from socket*/
         if ( nr == 1 ) {         	/* got a character; save it   	*/
             buf[ir++] = c;      	
         }
         else if ( nr == -1 ) {         /* trouble reading socket 	*/
             logit( "e", "wave_request: Error on socket recv\n" );
             ret = ERR_SOCKET;
             goto almostdone;
         }
      } while( c != '\n' );		/* last char of reply	  	*/
      buf[ir] = '\0';                   /* null-terminate the reply     */

      sscanf( buf, "%ld", &databuflen );
      /*printf( "wave_request: incoming msg[%ld]\n", databuflen );*/ /*DEBUG*/

      if( databuflen > maxbuflen )
      {
          logit( "e",
                 "wave_request: Data_buffer:%ld bytes > max_size:%ld\n",
                  databuflen, maxbuflen );
          ret = ERR_OVRFLW;
          goto almostdone;
      }

   /* Read one waveform message from the server
    *******************************************/
      ntoget = databuflen;
      bufptr = buf;
      while ( ntoget > 0 )
      {
      /* warning: trouble trying to recv 32 kbytes or more at once */
      /*          (recv dies with an "invalid argument" error)     */
         getatonce = ( (ntoget<32000) ? ntoget : 32000 );
         nr = recv( sd, bufptr, getatonce, 0 );
         if ( nr == -1 )
         {
            logit( "t", "wave_request: Error reading socket.\n" );
            SocketPerror( "wave_request: Error reading socket" );
            ret = ERR_SOCKET;
            goto almostdone;
         }
         bufptr += nr;
         ntoget -= nr;
      }

   /* Get end-of-message flag; make sure it's a newline!
    ****************************************************/
      nr = recv( sd, &c, 1, 0 );
      if ( nr != 1  ||  c != '\n' )
      {
         logit( "e", "wave_request: Error on recv end-of-message flag\n" );
         ret = ERR_SOCKET;
         goto almostdone;
      }

   /* Write this data buffer to the disk file
    *****************************************/
      if ( fwrite( buf, sizeof(char), databuflen, fp ) != (size_t)databuflen )
      {
         /*printf("wave_request: Error writing to file.\n");*/ /*DEBUG*/
         ret = ERR_FILEIO;
         goto almostdone;
      }

   } while ( ++nbufrec < ndatabuf );

/* Received all the data we requested
 ************************************/
   ret = nbufrec;

almostdone:
   free( buf );

done:
   SocketClose( sd );
   fclose( fp );
   return( ret );
}


/********************************************************
 *                       wave_inquire                   *
 *     Asks the wave_server for the time range and      *
 *     and the logo (installation, module and message   *
 *     type) of the data that's stored in the tank      *
 ********************************************************/
int wave_inquire( double        *tstart, /* start-time of first buffer in tank */
		  double        *tend,	 /* start-time of last buffer in tank  */
                  unsigned char *inst,   /* source installation of served data */
		  unsigned char *module, /* source module of served waveforms  */
                  unsigned char *type )  /* type of waveform msg being served  */
{
   SOCKET  sd;                   /* Socket descriptor */
   int     i, m, t;
   char    msg[MAX_MSG_LEN];
   char    c;
   int     length, nrec;
   int     ir;

/* Make sure configuration file has been read
 ********************************************/
   if ( !Config_Init )
   {
      /*printf("wave_inquire: Configuration file has not been read.\n");*/ /*DEBUG*/
      return( ERR_NOCONFIG );
   }

/* Set up socket connection here
 *******************************/
   if ( (sd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 )
   {
      logit( "t", "wave_inquire: Socket call failed.\n" );
      SocketPerror( "wave_inquire: Socket call failed" );
      return( ERR_SOCKET );
   }

   if ( connect( sd, (struct sockaddr *)&Sin, sizeof(Sin) ) == -1 )
   {
      logit( "t", "wave_inquire: Connect to wave_server host failed.\n" );
      SocketPerror( "wave_inquire: Connect to wave_server host failed" );
      SocketClose( sd );
      return( ERR_SOCKET );
   }

/* Send an inquiry to the wave_server
 ************************************/
   sprintf( msg, "INQ:\n" );
   length = (int) strlen( msg );
   if ( send( sd, msg, length, 0 ) == -1 )
   {
      logit( "t", "wave_inquire: Error sending inquiry.\n" );
      SocketPerror( "wave_inquire: Error sending inquiry" );
      SocketClose( sd );
      return( ERR_SOCKET );
   }

/* Start reading the socket.
 ***************************/
   nrec = 0;
   do
   {
      if ( nrec == MAX_MSG_LEN ) {   	/* stop if there's no more room */
	 logit( "e", "wave_inquire: wave_server reply too long.\n " );
         SocketClose( sd );
         return( ERR_OVRFLW );
      }
      ir = recv( sd, &c, 1, 0 );  	/* try to get a char from socket*/
      if ( ir == 1 ) {         		/* got a character; save it   	*/
         msg[nrec++] = c;      	
      }
      else if ( ir == -1 ) {          	/* trouble reading socket 	*/
         logit( "t", "wave_inquire: Error on socket recv\n" );
         SocketPerror( "wave_inquire: Error on socket recv" );
         SocketClose( sd );
         return( ERR_SOCKET );
      }
   } while( c != '\n' );		/* last char of reply	  	*/
   msg[nrec] = '\0';                    /* null-terminate reply         */

/* Process inquiry reply from wave_server
 ****************************************/
   if( sscanf( msg, "%lf %lf %d %d %d", tstart, tend, &i, &m, &t ) < 5 )
   {
	logit( "e", "wave_inquire: error decoding wave_server reply:\n%s\n",
               msg );
       *tstart = 0.;
       *tend   = 0.;
       *inst   = 0;
       *module = 0;
       *type   = 0;
        SocketClose( sd );
        return( ERR_STRIO );
   }
   if ( i<256 && i>0 ) {
       *inst = (unsigned char) i;
   }
   else {
	logit( "e", "wave_inquire: invalid installation id <%d>\n", i );
       *inst = 0;
   }
   if ( m<256 && m>0 ) {
       *module = (unsigned char) m;
   }
   else {
	logit( "e", "wave_inquire: invalid module id <%d>\n", m );
       *module = 0;
   }
   if ( t<256 && t>0 ) {
       *type = (unsigned char) t;
   }
   else {
	logit( "e", "wave_inquire: invalid msg type <%d>\n", t );
       *type = 0;
   }
   SocketClose( sd );
   return( 0 );
}


/***********************************************************************
 * wave_client_config()  processes command file using kom.c functions  *
 *                    exits if any errors are encountered              *
 ***********************************************************************/
void wave_client_config( char *configfile )
{
   int      ncommand;   /* # of required commands you expect to process   */
   char     init[10];   /* init flags, one byte for each required command */
   int      nmiss;      /* number of required commands that were missed   */
   char    *com;
   char    *str;
   int      nfiles;
   int      success;
   int      i;

/* Set to zero one init flag for each required command
 *****************************************************/
   ncommand = 2;
   for( i=0; i<ncommand; i++ )  init[i] = 0;

/* Open the main configuration file
 **********************************/
   nfiles = k_open( configfile );
   if ( nfiles == 0 ) {
	fprintf( stderr,
                "wave_client: Error opening command file <%s>; exitting!\n",
                 configfile );
	exit( -1 );
   }

/* Process all command files
 ***************************/
   while(nfiles > 0)   /* While there are command files open */
   {
        while(k_rd())        /* Read next line from active file  */
        {
	    com = k_str();         /* Get the first token from line */

        /* Ignore blank lines & comments
         *******************************/
            if( !com )           continue;
            if( com[0] == '#' )  continue;

        /* Open a nested configuration file
         **********************************/
            if( com[0] == '@' ) {
               success = nfiles+1;
               nfiles  = k_open(&com[1]);
               if ( nfiles != success ) {
                  fprintf( stderr,
                          "wave_client: Error opening command file <%s>; exitting!\n",
                           &com[1] );
                  exit( -1 );
               }
               continue;
            }

        /* Process anything else as a command
         ************************************/
            /* Read the wave_server's host name
             **********************************/
   /*0*/    if( k_its( "ServerIPAdr" ) )
            {
                str = k_str();
                if(str) strcpy( ServerIPAdr, str );
                init[0] = 1;
            }
            /* Read the port number to find waveform data on
             ***********************************************/
   /*1*/    else if( k_its( "ServerPort" ) )
            {
                ServerPort = k_int();
                init[1] = 1;
            }
            /* Command not recognized
             ************************/
            else
            {
                fprintf(stderr,
                       "wave_client: <%s> unknown command in <%s>.\n",
                        com, configfile );
                continue;
            }

        /* See if there were any errors processing the command
         *****************************************************/
            if( k_err() ) {
               fprintf( stderr,
                       "wave_client: Bad <%s> command in <%s>; exitting!\n",
                        com, configfile );
               exit( -1 );
            }
        }
        nfiles = k_close();
   }

/* After all files are closed, check init flags for missed commands
 ******************************************************************/
   nmiss = 0;
   for ( i=0; i<ncommand; i++ )  if( !init[i] ) nmiss++;
   if ( nmiss ) {
       fprintf( stderr, "wave_client: ERROR, no " );
       if ( !init[0] )  fprintf( stderr, "<ServerIPAdr> " );
       if ( !init[1] )  fprintf( stderr, "<ServerPort> "   );
       fprintf( stderr, "command(s) in <%s>; exitting!\n", configfile );
       exit( -1 );
   }

/* Set up socket address structure for wave_server's machine
 ***********************************************************/
   if((int)(Addr = inet_addr(ServerIPAdr)) == -1)
   {
      fprintf( stderr,
              "wave_client: inet_addr failed for ServerIPAdr <%s>\n",
              ServerIPAdr );
      exit( -1 );
   }

   Host = gethostbyaddr((char *)&Addr, sizeof(Addr), AF_INET);
   if(Host == NULL)
   {
      fprintf( stderr,
              "wave_client: gethostbyaddr failed for ServerIPAdr <%s>\n",
               ServerIPAdr );
      exit( -1 );
   }
   memset( (char *) &Sin, '\0', sizeof(Sin) );
   memcpy( (char *) &Sin.sin_addr, Host->h_addr, Host->h_length );
   Sin.sin_family = AF_INET;
   Sin.sin_port   = htons( (short)ServerPort );

/* Set configuration flag
 ************************/
   Config_Init = 1;

   return;
}

/***********************************************************************
 * wave_client_setup()  Initializes or resets the global variables     *
 *                      ServerIPAdr & ServerPort used by wave_client   *
 *                      routines to connect to a wave_server           *
 ***********************************************************************/
int wave_client_setup( char *ipadr,    /* server's IP address    */
                       int   port )    /* server's port number   */
{
/* Transfer arguments to wave_client globals
 *******************************************/
     strcpy( ServerIPAdr, ipadr );
     ServerPort = port;

/* Set up socket address structure for wave_server's machine
 ***********************************************************/
   if((int)(Addr = inet_addr(ServerIPAdr)) == -1)
   {
      logit( "et",
             "wave_client: inet_addr failed for ServerIPAdr <%s>\n",
              ServerIPAdr );
      return( -1 );
   }

   Host = gethostbyaddr((char *)&Addr, sizeof(Addr), AF_INET);
   if(Host == NULL)
   {
      logit( "et",
             "wave_client: gethostbyaddr failed for ServerIPAdr <%s>\n",
              ServerIPAdr );
      return( -1 );
   }
   memset( (char *) &Sin, '\0', sizeof(Sin) );
   memcpy( (char *) &Sin.sin_addr, Host->h_addr, Host->h_length );
   Sin.sin_family = AF_INET;
   Sin.sin_port   = htons( (short)ServerPort );

/* Set configuration flag
 ************************/
   Config_Init = 1;

   return( 0 );
}
