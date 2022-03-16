/* 
 *  socket_ew.c for Solaris
 *
 *  Contains system-dependent functions for dealing with
 *  sockets.
 */

#include <errno.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include "socket_ew.h"

ulong MS_PER_TICK;
struct tms mytimestruct;

int SOCKET_SYS_INIT = 0;   /* Global initialization flag,
                              set in SocketSysInit(), 
                              checked in socket_ew()  */

/********************** SocketSysInit *********************
   Initialize a socket system (dummy function in Solaris)
**********************************************************/

void SocketSysInit( void )
{
   SOCKET_SYS_INIT++;
   return;
}

/********************** SocketClose **********************
                      Close a Socket
**********************************************************/

void SocketClose( SOCKET soko )
{
   close( soko );
   return;
}

/********************** SocketPerror *********************
                    Print an error message
**********************************************************/

void SocketPerror( char *note )
{
   perror( note );
   return;
}

/************************ sendall() ***********************
*       looks like the standard send(), but does not      *
*       return until either error, or all has been sent   *
*	Also, we break sends into lumps as some           *
*	implementations can't send too much at once.      *
*	Will found this out.
***********************************************************/

#define SENDALL_MAX_LUMP 1024	/* no send() larger  than this */

int sendall(SOCKET socket, const char *msg, long msgLen, int flags)
{
	int   ret;  /* number of bytes actually sent, or error */
	long  nextByte;
	int   nsend;

	nsend = SENDALL_MAX_LUMP; /* try sending in lumps of this size */
	nextByte = 0;

	while ( nextByte<msgLen )
		{
		if ( msgLen-nextByte < nsend ) nsend = msgLen-nextByte; /* last small send? */
		ret = send(socket, (const char*)&msg[nextByte], nsend, flags);
		if (ret < 0)
			{
			logit("t","send error %d\n",ret);
			return( ret );
			}
		nextByte += ret;  /* we actually sent only this many */
		}
	return ( msgLen );
}

/********************** socketGetError_ew *****************
     Returns the error code for the most recent socket error.
**********************************************************/
int socketGetError_ew()
{
  return((int)errno);
}

/********************** socketSetError_ew *****************
     Sets the error code for the most recent socket error.
**********************************************************/
void socketSetError_ew(int error)
{
  errno=error;
}

Time_ew GetTime_ew()
{
  struct timeval CurrentTime;

  gettimeofday(&CurrentTime,0);
  return(CurrentTime.tv_sec);
}

Time_ew adjustTimeoutLength(int timeout_msec)
{
  return((timeout_msec+999)/1000); /* Convert miliseconds to
                                      seconds.  Don't truncate */
}
