
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: pipe.c 6298 2015-04-10 02:49:19Z et $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2005/07/15 18:20:20  friberg
 *     Unix version of libsrc for POSIX systems
 *
 *     Revision 1.1  2000/02/14 18:46:17  lucky
 *     Initial revision
 *
 *
 */

/***************************************************************
 *                            pipe.c                           *
 *                                                             *
 *      Routines for writing to and reading from a pipe        *
 *      under Unix.                                            *
 ***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static FILE  *Pipe;  /* "write-handle" to pipe to "child" process */
static int pipeErrorFlag = 0;


/*****************************************************************/
/* pipe_init() starts a process and opens a pipe to it.  The  	 */
/*             pipe replaces stdin of the new process.		 */
/*    Returns:   0 on success					 */ 
/*		-1 on failure 					 */
/*****************************************************************/
int pipe_init( char  *nextproc,  /* command to start new process */
      unsigned long   pipesize ) /* how big (bytes) to make pipe */ 
				 /* NOT USED                     */
{
   Pipe = popen( nextproc, "w" );
   if ( Pipe == (FILE *) NULL )
   {
     pipeErrorFlag = 1;
     return (-1);
   }
   pipeErrorFlag = 0;
   return ( 0 );
} 


/*****************************************************************/
/* pipe_put() writes a msg to a pipe, terminating with null byte */
/* Returns 0 if there were no errors, 				 */
/*	  some number if there were errors			 */
/*****************************************************************/
#define MAXWRITE 500

int pipe_put( char *msg, 	/* null-terminated char string 	*/
	      int   msgtype )	/* type of message (0-255)	*/      
{
   char     str[4];
   char     *m;
   unsigned n;
   unsigned nwrite;
   unsigned nwritten;    /* Number of chars written to pipe    */

/* Write message type to pipe
   **************************/
   if ( (msgtype > 255) || (msgtype < 0) )
   {
      fprintf( stderr, "msgtype out of range.  msgtype: %d\n", msgtype );
      return( -1 );
   }
   if (ferror(Pipe))
   {
      fprintf( stderr, "Error status (%d) reported by pipe\n", errno );
      pipeErrorFlag = 1;
      return( -1 );
   }
   sprintf( str, "%3d", msgtype );

   nwritten = fwrite( str, sizeof(char), (size_t)3, Pipe );
/* printf("pipe_put:  type:%3d.   msg:\n%s\n", msgtype, msg ); */
   if ( nwritten < 3 )
   {
      fprintf( stderr, "Write error. nwritten: %d  Should be 3.\n", nwritten );
      pipeErrorFlag = 1;
      return( -1 );
   }
   if (ferror(Pipe))
   {
      fprintf( stderr, "Error status (%d) writing msgtype to pipe\n", errno );
      pipeErrorFlag = 1;
      return( -1 );
   }

/* Write message string to pipe, including the null character
   **********************************************************/
   m        = msg;
   nwrite   = strlen( msg ) + 1;
   nwritten = 0;

   while ( nwrite > 0 )
   {
      n = fwrite( m, sizeof(char), ( nwrite > MAXWRITE ) ?
                  (size_t)MAXWRITE : (size_t)nwrite, Pipe );
      if ( n == 0 )
      {
         fprintf( stderr, "pipe_put(): Write error!\n" );
         pipeErrorFlag = 1;
         return( -1 );
      }
      if (ferror(Pipe))
      {
         fprintf( stderr, "Error status (%d) writing message to pipe\n", errno );
         pipeErrorFlag = 1;
         return( -1 );
      }
      nwritten += n;
      nwrite -= n;
      m += n;
      fflush( Pipe );
   }

   if ( nwritten != strlen( msg ) + 1 )
   {
      fprintf( stderr, "Pipe write error. nwritten != strlen(msg)+1\n" );
      pipeErrorFlag = 1;
      return( -1 );
   }

   if (ferror(Pipe))
   {
      fprintf( stderr, "Error status (%d) after writing to pipe\n", errno );
      pipeErrorFlag = 1;
      return( -1 );
   }

   return( 0 );
}

/*****************************************************************
 * pipe_get() reads a msg from a pipe (stdin) and writes it as a *
 *            null-terminated char string to the given address	 *
 * Returns  # of chars written to msg (not including null-byte)	 * 
 *         -1 if the message was longer than maxlen              *
 *         -2 if <null> encountered reading message type         *
 *            or stream error detected                           *
 *         -3 if EOF encountered reading message type            *
 *****************************************************************/

int pipe_get( char *msg,	/* address to copy msg to 	*/
	      int   maxlen, 	/* size of msg buffer		*/
	      int  *type )	/* type of message returned 	*/
{
   char  typestr[4];
   char *m;
   char  ch;
   int   i;

   if ( feof( stdin ) )      /* check if stream already at EOF */
      return ( -3 );
   if ( ferror( stdin ) )    /* check for stream error */
      return ( -2 );

/* Start by reading message type from pipe
 *****************************************/
   m = typestr;     /* Use a working copy of the target address */

   for ( i = 0; i < 3; i++ )
   {
      *m = fgetc( stdin );
      if ( *m == '\0' )  
      {
         *m = '\0';
     	 return ( -2 );
      }
      if ( *m == (char)EOF )  
      {
         *m = '\0';
     	 return ( -3 );
      }
      m++;
   }
   typestr[i] = '\0';
  *type = atoi(typestr);

/* Now read the message (terminated by null byte)
 ************************************************/
   m = msg;     /* Use a working copy of the target address */

   for ( i = 0; i < maxlen; i++ )
   {
      *m = fgetc( stdin );
      if ( *m == '\0' || *m == (char)EOF )  
      {
         *m = '\0';
     	 return ( i );
      }
      m++;
   }
	
/* If you got here, the message was too long!  Skip to the end of it
 *******************************************************************/
   while( 1 )
   {
      ch = fgetc( stdin );
      if ( ch == '\0' || ch == (char)EOF )
         break;
   }
   return( -1 );
}


/*****************************************************************
 * pipe_error() returns a non-zero value if an error has been    *
 * flagged on the output pipe.                                   *
 *****************************************************************/
int pipe_error( void )
{
  return pipeErrorFlag | ferror(Pipe);
}


/*****************************************************************/
/* pipe_close()  Closes the pipe			  	 */
/*****************************************************************/
void pipe_close( void )   
{
   pclose( Pipe );
} 
