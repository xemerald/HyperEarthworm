/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: socket_ew.h 7967 2019-03-20 17:54:37Z stefan $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.9  2018-11-26 20:47:34  alexander
 *     Adding new functions to handle IPv6 networking. In particular,
 *     the wrapper functions setuptcpclient_ew() and setuptcpserver_ew()
 *     allow applications to be agnostic to whether an address is IPv4 or 
 *     IPv6, while at the same time tidying up boilerplate code needed for
 *     TCP sockets
 *
 *     Revision 1.8  2007/03/28 14:13:39  paulf
 *     minor MACOSX #ifdefs added
 *
 *     Revision 1.7  2006/03/10 13:50:56  paulf
 *     minor linux related fixes to removing _SOLARIS from the include line
 *
 *     Revision 1.6  2005/07/27 15:10:56  friberg
 *     added in _LINUX ifdef for an include
 *
 *     Revision 1.5  2004/05/18 21:34:48  mark
 *     Removed SocketSysInit prototype (already included via ew_complex_funcs.h)
 *
 *     Revision 1.4  2003/02/04 17:55:24  davidk
 *     Defined a new constant CONNREFUSED_EW  indicating that a server refused a client connection.
 *     It is defined as the appropriate CONNFREFUSED constant for each platform.
 *     Added a new function socketSetError_ew() that allows that caller to specify the last socket error.
 *     This is neccessary in a Solaris client when a connection is refused by server.
 *
 *     Revision 1.3  2002/09/10 19:21:43  dhanych
 *     REmoved include of winsock2.h ; it caused all sorts of compile-time
 *     problems
 *
 *     Revision 1.2  2000/03/05 21:54:17  lombard
 *     Added definition of INADDR_NONE for Solaris
 *     Added `include <errno.h>' for Solaris so EINPROGRESS would be defined
 *     Placed `ifndef min' around min to prevent redefinition errors
 *     Added missing prototype for recv_all().
 *
 *     Revision 1.1  2000/02/14 20:05:54  lucky
 *     Initial revision
 *
 *
 */

#ifndef SOCKET_EW_H
#define SOCKET_EW_H

/**************************************************************/
/* #defines                                                   */
/**************************************************************/
#define SOCKET_CLOSE_IMMEDIATELY_EW 0
#define SOCKET_CLOSE_GRACEFULLY_EW -1
#define SOCKET_CLOSE_SIMPLY_EW     -2

#ifdef _WINNT
#include <winsock2.h>
#ifdef WIN32_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#endif
#define _WINSOCKAPI_
#include <windows.h>
#include <ws2tcpip.h> 			/* typedef fd_set */
# define WOULDBLOCK_EW WSAEWOULDBLOCK
# define CONNECT_WOULDBLOCK_EW WSAEWOULDBLOCK
# define CONNREFUSED_EW WSAECONNREFUSED
  typedef unsigned __int64 Time_ew;
# define TIMEOUT_ADJUSTMENT_ew 10000	/* Convert from millisec. to */
					/*    100 nanosec            */
#else /* _WINNT */
# include <sys/select.h> 		/* typedef fd_set */
#endif /* _WINNT */

/* #ifdef __sun             __sun isn't defined on x86Solaris 2.6 */

#ifdef _SOLARIS
  /* the following Solaris'ism is not found on Linux platforms */
# include <sys/filio.h>
#endif

#ifdef __DARWIN_UNIX03
  /* the following Solaris'ism is not found on Linux platforms */
# include <sys/filio.h>
#endif

#ifdef _LINUX
  /* the following IS FOUND on Linux platforms */
# include <asm/ioctls.h>
#endif

/* these are common to Solaris and Linux */
#if defined  (_SOLARIS) || defined(_LINUX) || defined(__DARWIN_UNIX03)
# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/types.h>
# include <sys/times.h>
# include <sys/param.h>
# include <sys/ioctl.h>
# include <errno.h>     /* Needed so EINPROGRESS is defined */
# define ioctlsocket ioctl
# define closesocket close 
# define FAR
# define INVALID_SOCKET -1
# define SOCKET_ERROR -1
# define WOULDBLOCK_EW EWOULDBLOCK
# define CONNECT_WOULDBLOCK_EW EINPROGRESS
# define CONNREFUSED_EW ECONNREFUSED
 typedef unsigned int Time_ew;
# define TIMEOUT_ADJUSTMENT_ew 1/1000  /* Convert from millisec. to sec. */
#ifndef INADDR_NONE
#define INADDR_NONE     0xffffffff      /* should be in <netinet/in.h> */
#endif
#endif /* _SOLARIS or _LINUX or __DARWIN_UNIX03 */

#ifndef min
#define min(a, b)  (((a) < (b)) ? (a) : (b)) 
#endif

#define MAXSENDSIZE_EW 8192/*4096*/

#include <earthworm.h>
/**************************************************************/
/* EXTERNS                                                    */
/**************************************************************/
extern int SELECT_TIMEOUT_SECONDS;
extern int SELECT_TIMEOUT_uSECONDS;
extern int EW_SOCKET_DEBUG;

/**************************************************************/
/* Socket based function prototypes                           */
/**************************************************************/
SOCKET setuptcpclient_ew(const char *host, const char *service, int timeout_msec);
  /* setuptcpclient_ew() checks the supplied parameters and 
     determines whether it is appropriate to resolve host and
     service as IPv4 or IPv6
     The function takes care of the initial socket() call, and proceeds to call
     connect_ew() with the appropriate timeout parameter for nonblocking sockets
     calls, returning a valid SOCKET if succesful, and -1 if not
  */

SOCKET setuptcpserver_ew(const char *host, const char *service, int backlog);
  /* setuptcpserver_ew() checks the supplied parameters and determines
     whether it is appropriate to resolve host and service as IPv4 or IPv6.
     The function takes care of the initial socket() call, and proceeds to call
     bind__ew() and listen_ew() (passing the 'backlog' parameter to listen_ew())
     The function returns a valid SOCKET if successful, and 
     -3 if address resolution fails
     -2 setsockopt() fails
     and -1 if otherwise unable to establish a server socket
   */

int numeric_address_to_string_ew(int isIPv6, uint8_t inAddr[16], uint8_t inPort[2],
                                 char hostString[64], char portString[10]);
  /* numeric_address_to_string_ew() converts numeric arrays inAddr and inPort
     to human-readable strings
   */

int formatsocketaddress_ew(const struct sockaddr *address, int addrLen, 
                             char hostString[64], char portString[10]);
  /* formatsocketaddress_ew() accepts a (typically client) connection specified as 
     'address'
   
      and returns hostString, portString, providing a human-readable representation
      of the connection 'address'
   */

SOCKET accept_ew (SOCKET s, struct sockaddr FAR* addr, int FAR* addrlen,
			   int timeout); 
  /* accept_ew() attempts to accept a connection on a socket.
     timeout is the length of time in millisec. that accept_ew() 
     will wait before returning.  Timeout is measure from the
     point after the initial accept() call.  Pass -1 for infinite
     accept_ew to block.  If a successful
     connection is not accepted before the timeout expires, or
     if an error occurs, the function returns INVALID_SOCKET.  
     If the latest socket error was WOULDBLOCK_EW, then 
     no connections were requested during the timeout period.
     Caller can call socketGetError_ew() for details about 
     any failures. 
  */


int bind_ew (SOCKET s, struct sockaddr FAR* name, int namelen );
  /* bind_ew() attempts to bind the socket s to a name/port number.
     Caller can call socketGetError_ew() for details about any failures.  
  */


int closesocket_ew(SOCKET s,int HowToClose);
  /* closesocket_ew() closes the socket s.  HowToClose indicates
     whether the socket should be closed gracefully or immediately.
     Use SOCKET_CLOSE_IMMEDIATELY_EW or SOCKET_CLOSE_GRACEFULLY_EW
     to indicate closure method.  Caller can call socketGetError_ew()
     for details about any failures.
  */


int connect_ew(SOCKET s, struct sockaddr FAR* name, 
				int namelen, int timeout);
  /* connect_ew() attempts to create a socket connection during a
     period specified by timeout.  If it succeeds it returns a
     successful condition.  If it fails either due to a network
     error, or a timeout, it closes the socket and returns an error.  
     *Note:  The timeout clock starts after connect_ew() calls
     connect(), not when connect_ew() starts.
     Caller can call socketGetError_ew() for details about any 
     failures.
  */


int listen_ew (SOCKET s, int backlog );
  /* listen_ew() signals the mysterious protocol stack god, that the
     socket is ready to accept connections.
     Caller can call socketGetError_ew() for details about any failures.  
  */


int recv_ew (SOCKET s,char FAR* buf,int len,int flags, int timeout);
  /* recv_ew attempts to receive data on a connection oriented scoket.
     timeout is the length of time in millisec. that the recv_ew() 
     will wait before returning(if no data is received), after making
     the initial recv() call.  If data (or a shutdown request) is not
     received before the timeout expires, or if an error occurs, the 
     function returns SOCKET_ERROR.  If the latest socket error is
     WOULDBLOCK_EW, then no data was received during the timeout 
     period. As soon as data is received, the function returns.  
     The function does not attempt to completely fill the buffer 
     before returning.  
     If (-1) is passed for timeout_msec, then recv_ew() reverts to a blocking
     recv() call.
     Caller can call socketGetError_ew() for details about any failures.
  */


int recv_all (SOCKET s,char FAR* buf,int len,int flags, int timeout_msec);
  /* recv_all attempts to receive data on a connection oriented scoket.
     timeout is the length of time in millisec. that the recv_ew() will wait
     before returning(if no data is received), after making the initial recv()
     call.  

     if timeout_msec > 0, recv_all() returns when the sooner of two things
     happens: 
     1.  The timeout measured in millisec. from the time of the first
     send() call, expires; 
     2.  "len" bytes of data are received.
     recv_all() returns the number of bytes of data received, or SOCKET_ERROR
     on error.  The caller is responsible for noting any discrepencies in the
     difference between the number of bytes requested to be sent, and the
     number of reported bytes sent.  If there is a discrepency, then a timeout
     occured.  Caller can call socketGetError_ew() for details about any
     failures.
     if timeout_msec == -1, recv_all() sets the socket to blocking and returns
     when:
     1. "len" bytes of data are received.
     2. EOF is detected by recv returning 0 bytes.
     */

int recvfrom_ew (SOCKET s, char FAR* buf, int len, int flags, 
			  struct sockaddr FAR* from, int FAR* fromlen,
			  int timeout);
  /* recvfrom_ew() is similar to recv_ew(), except used for datagram
     sockets.  timeout is specified in milliseconds.  Caller can call 
     socketGetError_ew() for details about any failures. 
  */


int select_ew (int nfds, fd_set FAR * readfds, fd_set FAR * writefds, 
			fd_set FAR * exceptfds, 
			int timeout);
  /* select_ew() determines the state of sets of sockets, by 
     calling select().  Timeout is in milliseconds, and is
     converted by select_ew to the select() timeout structure, and
     passed on (to select()).
     Caller can call socketGetError_ew() for details about any failures.
  */


int send_ew ( SOCKET s, const char FAR * buf, int len, int flags, 
			 int timeout);
  /* send_ew() returns when the sooner of two things happens:  
      1.  The timeout measured in millisec. from the time of 
           the first send() call, expires;  
      2.  All of the data provided by the caller is sent.
     send_ew() returns the number of bytes of data sent, or
     SOCKET_ERROR on error.  The caller is responsible for noting
     any discrepencies in the difference between the number of bytes
     requested to be sent, and the number of reported bytes sent.  If
     there is a discrepency, then a timeout occured.
     Caller can call socketGetError_ew() for details about any failures.
  */


int sendto_ew (SOCKET s, const char FAR * buf, int len, 
			int flags, const struct sockaddr FAR * to,
			int tolen, int timeout); 
  /* sendto_ew() is similar to send_ew(), except used for datagram
     sockets.  timeout is specified in milliseconds.  Caller can call
     socketGetError_ew() for details about any failures. 
  */


SOCKET socket_ew (int af, int type, int protocol);
  /* socket_ew() allocates a socket
     descriptor and associated resources.  It first calls socket(),
     and then sets the socket descriptor to non-blocking mode.
     No network I/O occurs.
     Caller can call socketGetError_ew() for details about any failures.
  */



/*void SocketSysInit( void );*/
/********************** SocketSysInit ********************
 *              Initialize the socket system             *
 *         We are using Windows socket version 2.2.      *
 *********************************************************/


/**************************************************************/
/* SOCKET_ew utility function prototypes                      */
/**************************************************************/


int setSocket_ewSelectTimeout(unsigned int Timeout);
  /* setSocket_ewSelectTimeout() sets the timeout period
     passed to select() calls made internally within the 
     SOCKET_ew routines.  The timeout period is in 
     milliseconds.
  */


int setSocket_ewDebug(int debug);
  /* setSocket_ewDebug() turns debugging on or off for 
     the SOCKET_ew routines.
  */


int socketGetError_ew();
  /* socketGetError_ew() returns the error code for the most
     recent socket error.
  */

void socketSetError_ew(int error);
  /* socketSetError_ew() sets the system error associated with
     the last socket call.
  */

Time_ew GetTime_ew();
/*************************************************************/
/* Network utility prototypes                                */
/*************************************************************/
int numeric_address_to_string_ew(int isIPv6, uint8_t inAddr[16], 
                uint8_t inPort[2], char hostStr[64], char portStr[10]);
/* numeric_address_to_string_ew() converts numeric arrays inAddr, inPort 
   into strings hostStr, portStr
 */

char ip_in_same_subnet(const char *ip1, const char *ip2, char *netmask);
/* Determines if ip1 and ip2 belong to the same subnet
   If IPs are IPv4, uses netmask to determine which subnet
   If IPs are IPv6, compares first 64 bits of each address
   
   returns 1 if ips are in same subnet, or 0 if not OR
   if processing failed (eg, due to mixed use of address families)
 */

#endif 
/* SOCKET_EW_H : don't include file if already included*/
