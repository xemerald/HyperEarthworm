/*
 *    Revision history:
 *     $Log$
 *     Revision 1.10  2007/05/18 17:22:23  luetgert
 *     Added an error check to send_ew() to make sure there are no errors on
 *     the socket prior to issueing the send() request.
 *     .
 *
 *     Revision 1.9  2005/03/28 20:42:52  mark
 *     Set error code on blocking call to recv_all
 *
 *     Revision 1.8  2005/03/22 17:24:27  mark
 *     Set error code on blocking call to connect_ew
 *
 *     Revision 1.7  2005/02/24 20:40:22  mark
 *     Fix to accept_ew for nonblocking sockets
 *
 *     Revision 1.6  2004/04/12 22:15:37  dietz
 *     :included string.h
 *
 *     Revision 1.5  2003/02/04 17:57:56  davidk
 *     Added a call to socketSetError_ew() to set the socket error when
 *     connect() fails during the select() loop in connect_ew().
 *     Normally, ON A NON-BLOCKING SOCKET, connect() is called,
 *     it returns a WOULDBLOCK/INPROGRESS condition, then we go
 *     into a select() loop to check for socket-writability within a timeout
 *     period.  There was an error in the code, that did not notice the
 *     CORRECT error (atleast on Solaris) and so the code would return
 *     a TIMEOUT error instead of the actual error.
 *
 *     The change only applies to the connect_ew function() when run
 *     in non-blocking mode (clients connecting to a server - using a timeout value).
 *     The change only applies when a socket-error occurs while the function is
 *     waiting for the connect to happen.  The instance where you will most-likely
 *     see a difference, is when you try connecting to a non-existent socket.
 *     Previously the function would return TIMEOUT, now it will return
 *     Revision 1.4  2000/12/01 23:48:54  lombard
 *     Fixed a few more logit format errors.
 *
 *     Revision 1.3  2000/07/10 21:14:51  lombard
 *     Fixed bug in recvfrom_ew where improper arguments were used in recvfrom calls
 *
 *     Revision 1.2  2000/06/28 17:17:54  lombard
 *     Fixed bug in format for logit calls after select errors, several places
 *
 *     Revision 1.1  2000/02/14 18:51:48  lucky
 *     Initial revision
 *
 *
 */

/****************** socket_ew_common *************************/

/*
 * Changes:
 * 12/7/1999, PNL
 * Accept_ew now sets the new socket to non-blocking mode.
 *  Previously it was assumed that the new socket inherited non-blocking
 *  from the original socket; this is not true in WinNT or Solaris 2.6.
 * Errors from select() calls are now handled.
 */
/********************* #INCLUDES *****************************/
/*************************************************************/
#include <errno.h>
#include <string.h>

#include "socket_ew.h"

#pragma comment(lib, "Ws2_32.lib")

/********************** GLOBAL *******************************/
/********************* VARIABLES *****************************/
/*************************************************************/
/* Timeout used for select() calls: 0.2 seconds */
int SELECT_TIMEOUT_SECONDS=0;
int SELECT_TIMEOUT_uSECONDS=200000;

int EW_SOCKET_DEBUG=0;       /* Set by setSocket_ewDebug() */

extern int SOCKET_SYS_INIT;  /* Global initialization flag.
                                Declared in sys-dependent socket_ew.c,
                                set in SocketSysInit(),
                                checked in socket_ew()  */

/********************* SOCKET_ew *****************************/
/*********** Internal Utility Function Prototypes ************/
/*************************************************************/
struct timeval FAR * resetTimeout(struct timeval FAR *);
Time_ew adjustTimeoutLength(int timeout_msec);


/********************* SOCKET_ew *****************************/
/********************* Functions *****************************/
/*************************************************************/
SOCKET setuptcpclient_ew(const char *host, const char *service, int timeout_msec) {
/*
 * if successfully establishes a TCP connection, returns valid socket descriptor
 *    if socket() fails, returns -2
 *    if socket() succeeds but connection fails, return -1 (so we can try again later)
 * */
   char * funcName = "setuptcpclient_ew()";
   // Tell the system what kind(s) of address info we want
   struct addrinfo addrCriteria;                   // Criteria for address match
   memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
   addrCriteria.ai_family = AF_UNSPEC;             // v4 or v6 is OK
   addrCriteria.ai_socktype = SOCK_STREAM;         // Only streaming sockets
   addrCriteria.ai_protocol = IPPROTO_TCP;         // Only TCP protocol
   int socketEstablished = 0;                      //Flag to deal with case where socket() fails completely
   struct addrinfo *addr = NULL;
   char dupAddress[64];                            //for testing/checking IPv6 addresses

   /* SWL 2019-06-19 Seems we need to initialize the Windows system before we can get  addrinfo       *
    * Shouldn't hurt anything to do it here rather than in socket_ew below, and it fixes new failures *
    * in heli_ewII, ewhtmlemail, q3302ew, etc. Hopefully things'll run OK on Linux.                   */
   if (!SOCKET_SYS_INIT)
	   SocketSysInit();

   // Get address(es)
   struct addrinfo *servAddr; // Holder for returned list of server addrs
   int rtnVal = getaddrinfo(host, service, &addrCriteria, &servAddr);
   //XXX XXX XXX
   if (rtnVal != 0) {
      logit("et", "%s- getaddrinfo error: %s\n", funcName, gai_strerror(rtnVal));
      return -3;
   }

   int sock = -1;
   for (addr = servAddr; addr != NULL; addr = addr->ai_next) {
      // check for 'link-local' addresses (IPv6).
      if(addr->ai_family == AF_INET6) {
         if(getnameinfo(addr->ai_addr, addr->ai_addrlen, dupAddress, sizeof(dupAddress), NULL, 0, NI_NUMERICHOST) == 0) {
            if(strncmp(dupAddress, "fe80", strlen("fe80")) == 0) {
               //here we can be sure that the address is a link-local address
               //set the scope to '2'
               ((struct sockaddr_in6 *)(addr->ai_addr))->sin6_scope_id = 2;
            }
         }
      }
      // Create a reliable, stream socket using TCP
      sock = socket_ew(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
      if (sock < 0)
         continue;  // Socket creation failed; try next address

      //here, we have established a socket, save this info:
      socketEstablished = 1;

      // Establish the connection to the echo server
      if ( connect_ew(sock, addr->ai_addr, addr->ai_addrlen, timeout_msec) == 0)
         break;     // Socket connection succeeded; break and return socket


      sock = -1;
   }

   if(!socketEstablished)
   {
      sock = -2;  //this way we can differentiate between socket() failure and connect() failure
   }

   freeaddrinfo(servAddr); // Free addrinfo allocated in getaddrinfo()
   return sock;
}

SOCKET setuptcpserver_ew(const char *host, const char *service, int backlog)
{
  /* setuptcpserver_ew() checks the supplied parameters and determines
     whether it is appropriate to resolve host and service as IPv4 or IPv6.
     The function takes care of the initial socket() call, and proceeds to call
     bind__ew() and listen_ew() (passing the 'backlog' parameter to listen_ew())
     The function returns a valid SOCKET if successful, and
     -3 if address resolution fails
     -2 setsockopt() fails
     and -1 if otherwise unable to establish a server socket
   */
   // Construct the server address structure
   struct addrinfo addrCriteria;                   // Criteria for address match
   memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
   addrCriteria.ai_family = AF_UNSPEC;             // Any address family
   addrCriteria.ai_flags = AI_PASSIVE;             // Accept on any address/port
   addrCriteria.ai_socktype = SOCK_STREAM;         // Only stream sockets
   addrCriteria.ai_protocol = IPPROTO_TCP;         // Only TCP protocol
   int on = 1;                                     // boolean flag to pass for setsockopt() setting 'REUSEADDR' to 'on'
   struct addrinfo *addr = NULL;

   struct addrinfo *servAddr; // List of server addresses
   int rtnVal = getaddrinfo(host, service, &addrCriteria, &servAddr);
   if (rtnVal != 0) {
      //failure, use specific return code
      return -3;
   }

   SOCKET servSock = -1;
   for (addr = servAddr; addr != NULL; addr = addr->ai_next) {
      // Create a TCP socket
      servSock = socket_ew(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
      if (servSock < 0)
         continue;       // Socket creation failed; try next address

      //call setsockopt():
      if(setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) != 0) {
         //log messages will be printed by client
         if(EW_SOCKET_DEBUG) logit("et", "setsockopt() failed, errno: %s. Socket: %d\n", strerror(errno), servSock);
         //close socket
         closesocket_ew(servSock, SOCKET_CLOSE_IMMEDIATELY_EW);
         //go to "Heavy Restart"... probably make a specific return code
         return -2;
      }

      // XXX should bind() and listen() be broken up?
      // Bind to the local address and set socket to listen
      if ((bind_ew(servSock, addr->ai_addr, addr->ai_addrlen) == 0) &&
         (listen_ew(servSock, backlog) == 0)) {
         break;       // Bind and listen successful
      }

      closesocket_ew(servSock, SOCKET_CLOSE_IMMEDIATELY_EW);  // Close and try again
      servSock = -1;
   }

   // Free address list allocated by getaddrinfo()
   freeaddrinfo(servAddr);

   return servSock;
}

int numeric_address_to_string_ew(int isIPv6, uint8_t inAddr[16], uint8_t inPort[2],
                              char hostString[64], char portString[10])
{
   /* numeric_address_to_string_ew() turns {inAddr, inPort} into strings
      hostString, portString.

      returns 0 if successful, a value < 0 if not successful
    */
   struct sockaddr *address;
   int addrLen;
   struct sockaddr_in ipv4Addr;
   struct sockaddr_in6 ipv6Addr;
   if(isIPv6) {
      memset(&ipv6Addr, 0, sizeof(struct sockaddr_in6));
      ipv6Addr.sin6_family = AF_INET6;
      memcpy(&(ipv6Addr.sin6_addr), inAddr, sizeof(ipv6Addr.sin6_addr));
      memcpy(&(ipv6Addr.sin6_port), inPort, sizeof(ipv6Addr.sin6_port));
      addrLen = sizeof(struct sockaddr_in6);
      address = (struct sockaddr *) &ipv6Addr;
   } else {
      memset(&ipv4Addr, 0, sizeof(struct sockaddr_in));
      ipv4Addr.sin_family = AF_INET;
      memcpy(&(ipv4Addr.sin_addr), inAddr, sizeof(ipv4Addr.sin_addr));
      memcpy(&(ipv4Addr.sin_port), inPort, sizeof(ipv4Addr.sin_port));
      addrLen = sizeof(struct sockaddr_in);
      address = (struct sockaddr *) &ipv4Addr;
   }

   return formatsocketaddress_ew(address, addrLen, hostString, portString);
}

int formatsocketaddress_ew(const struct sockaddr *address, int addrLen,
                char hostString[64], char portString[10]) {
  /* formatsocketaddress_ew() accepts a (typically client) connection specified as
     'address'

      and returns a format string in addrString, provided a human-readable representation
      of the connection 'address'
   */

   int err = 0;
   int hostSize = 64;
   int portSize = 10;

   // Test for address
   if (address == NULL)
      return -1;

   memset(hostString, 0, hostSize);
   memset(portString, 0, portSize);

   //look up info:
   if((err = getnameinfo(address, addrLen, hostString, hostSize,
                         portString, portSize, NI_NUMERICHOST | NI_NUMERICSERV)) != 0) {
      strncpy(hostString, (char *)("[invalid address]"), strlen((char *)"[invalid address]") + 1); // Unable to convert
      //for now:
      logit("et", "gai_strerror(): %s\n", gai_strerror(err));
      return -2;
   }
   return 0;
}

SOCKET socket_ew (int af, int type, int protocol)
{
  /* socket_ew() allocates a socket descriptor and associated
     resources. It first makes sure that the Socket system has
     been initialized, then it calls socket(), and finally sets
     the socket descriptor to non-blocking mode.
     Arguments af, type and protocol are passed directly to socket().
     No network I/O occurs.
     Caller can call socketGetError_ew() for details about any
     failures.
  */

  SOCKET newSocket;
  static char * MyFuncName = "socket_ew()";
  int retVal;
  unsigned long lOnOff=1;

  if (EW_SOCKET_DEBUG)
    logit("et","SOCKET_DEBUG: Entering %s\n",MyFuncName);

  if (!SOCKET_SYS_INIT)
    SocketSysInit();

  newSocket = socket(af,type,protocol);
  if (newSocket == INVALID_SOCKET  && EW_SOCKET_DEBUG)
    logit("et","SOCKET_DEBUG: Error: %d, occurred in %s\n",
	  socketGetError_ew(),MyFuncName);

  if (newSocket != INVALID_SOCKET)
  {
    retVal=ioctlsocket(newSocket,FIONBIO,&lOnOff);
    if (retVal==SOCKET_ERROR)
    {
      if(EW_SOCKET_DEBUG)
        logit("et","SOCKET_DEBUG: Error: %d, occurred in %s during change to non-blocking\n",
              socketGetError_ew(),MyFuncName);
      closesocket(newSocket);
      return((SOCKET)SOCKET_ERROR);
    }
  }

  if(EW_SOCKET_DEBUG)
    logit("et","SOCKET_DEBUG: Exiting %s\n",MyFuncName);
  return(newSocket);
}

/*************************************************************/

int connect_ew(SOCKET s, struct sockaddr FAR* name,
	       int namelen, int timeout_msec)
{
  /* connect_ew() attempts to create a socket connection during a
   * period specified by timeout.
   * Arguments s, name, and namelenare passed directly to connect().
   * On success conect_ew() returns zero.
   * On failure, either due to a network error, or a timeout, conect_ew
   * closes the socket and returns SOCKET_ERROR.
   * *Note:  The timeout clock starts after connect_ew() calls
   * connect(), not when connect_ew() starts.
   * A timeout value of -1 causes connect_ew() to revert to a blocking
   * connect call.
   * Caller can call socketGetError_ew() for details about any
   * failures.
   */

   static char * MyFuncName = "connect_ew()";

   int     retVal, ioctlRetVal, connectRetVal, selectRetVal;
   fd_set  ConnectedSockets;
   long    lOnOff;
   int     lastError;
   struct  timeval SelectTimeout;

   if ( EW_SOCKET_DEBUG )
    logit( "et" , "SOCKET_DEBUG: Entering %s. Socket: %d namelen: %d timeout: %d\n", MyFuncName, s, namelen, timeout_msec );

/* If there is no timeout, make the socket blocking
   ************************************************/
   if ( timeout_msec == -1 )
   {
     lOnOff = 0;
     ioctlRetVal = ioctlsocket( s, FIONBIO, (u_long *)&lOnOff );
     if ( ioctlRetVal < 0 )
     {
       if ( EW_SOCKET_DEBUG )
         logit( "et", "SOCKET_DEBUG: Error: %d, occurred in %s during change to blocking\n",
                socketGetError_ew(), MyFuncName );
       retVal = -1;
       goto Done;
     }

/* Try to get a connection (blocking)
 **********************************/
     if ( connect( s, name, namelen ) == 0 )     /* Got a connection */
       retVal = 0;
     else                                        /* Didn't get a connection */
       retVal = -1;

/* Change the socket back to non-blocking so
   we don't screw up any further operations
   *****************************************/
      lOnOff = 1;
      ioctlRetVal=ioctlsocket(s,FIONBIO,(u_long *)&lOnOff);
      if ( ioctlRetVal < 0 )
      {
        logit( "et", "Error: %d, occurred in %s during change to non-blocking\n",
               socketGetError_ew(), MyFuncName );
        retVal = -1;
      }
      goto Done;
   }

/* Initiate a non-blocking connection request
   ******************************************/
   connectRetVal = connect( s, name, namelen );

   if ( connectRetVal == 0 )                         /* Got a connection */
   {
      retVal = 0;
      goto Done;
   }

   lastError = socketGetError_ew();

   if ( lastError != CONNECT_WOULDBLOCK_EW )         /* Connect() error */
   {
     logit( "et", "Connect request failed in connect_ew(): %s.\n",
            strerror(lastError) );
     retVal = -1;
     goto Done;
   }

/* Hang around in select() until connect is successful
                        or until timeout period expires
   ****************************************************/
   FD_ZERO( &ConnectedSockets );
   FD_SET( s, &ConnectedSockets );

   SelectTimeout.tv_sec  = timeout_msec/1000;
   SelectTimeout.tv_usec = (timeout_msec%1000)*1000;

   selectRetVal = select( s+1, 0, &ConnectedSockets, 0, &SelectTimeout );

/* select() failed
   ***************/
   if ( selectRetVal == -1 )
   {
     logit( "et", "select() failed in connect_ew(). Error: %d\n",
            socketGetError_ew() );
     retVal = -1;
   }

/* select() succeeded; connection may have been completed
   ******************************************************/
   else if ( selectRetVal > 0  &&  FD_ISSET( s, &ConnectedSockets ) )
   {
   /* NOTE: For Solaris, we must do one more connection test.
    *       Other possible tests besides getsockopt(SO_ERROR) could
    *       be a zero-length read() or a call to getpeername()
    */
     int rc;
     int error = 0;
     socklen_t len = sizeof( error );

     rc  = getsockopt(s, SOL_SOCKET, SO_ERROR, (char *)&error, &len);

     if ( rc < 0 )          /* Pending error on some systems  */
     {
       error = socketGetError_ew();
       retVal = -1;
     }
     else if ( error )      /* Pending error on others systems */
     {
       socketSetError_ew(error);
       retVal = -1;
     }
     else                   /* OK, got a connection! */
     {
       if ( EW_SOCKET_DEBUG )
         logit( "et", "SOCKET_DEBUG: Got a connection\n" );
       retVal = 0;
     }

     if ( retVal == -1  &&  EW_SOCKET_DEBUG )
       logit("et", "SOCKET_DEBUG: connect_ew() connection failed; "
             "getsockopt detected error: %s.\n",
             strerror(error) );
   }

/* Only other possibility: select timed out!
   *****************************************/
   else
   {
     /* Set the socket error to lastError that we got from connect(); do this because
	  * select() sets the error code to 0 when it's successful.
	  */
     socketSetError_ew(lastError);
     if ( EW_SOCKET_DEBUG ) /* this line added by Alex 2/9/99 */
       logit( "et", "SOCKET_DEBUG: connect timed out in connect_ew().\n" );
     retVal = -1;
   }

Done:

   if ( retVal == -1 )
   {
     closesocket_ew( s, SOCKET_CLOSE_SIMPLY_EW ); /*skip setsockopt()*/
     retVal = SOCKET_ERROR;
   }

   if ( EW_SOCKET_DEBUG )
     logit("et","SOCKET_DEBUG: Exiting %s\n",MyFuncName);

   return(retVal);
}

/*************************************************************/

int bind_ew (SOCKET s, struct sockaddr FAR* name, int namelen )
{
  /* bind_ew() attempts to bind the socket s to a name/port number.
     This is basicly same as normal bind() call, with some logging added.
     Caller can call socketGetError_ew() for details about any failures.
  */

  int retVal;
  static char * MyFuncName = "bind_ew()";

  if(EW_SOCKET_DEBUG)
    logit("et","SOCKET_DEBUG: Entering %s\n",MyFuncName);

  retVal=bind(s,name,namelen);
  if (retVal < 0  && EW_SOCKET_DEBUG)
    logit("et","SOCKET_DEBUG: Error: %d, occurred in %s\n",
	  socketGetError_ew(),MyFuncName);

  if(EW_SOCKET_DEBUG)
    logit("et","Exiting %s\n",MyFuncName);

  return(retVal);
}

/*************************************************************/

int listen_ew (SOCKET s, int backlog )
{
  /* listen_ew() signals the mysterious protocol stack god, that the
     socket is ready to accept connections.
     Arguments are passed directly to listen().
     Caller can call socketGetError_ew() for details about any failures.
  */

  int retVal;
  static char * MyFuncName = "listen_ew()";

  if(EW_SOCKET_DEBUG)
    logit("et","SOCKET_DEBUG: Entering %s\n",MyFuncName);

  retVal=listen(s, backlog);
  if (retVal < 0  && EW_SOCKET_DEBUG)
    logit("et","SOCKET_DEBUG: Error: %d, occurred in %s\n",
	  socketGetError_ew(),MyFuncName);

  if(EW_SOCKET_DEBUG)
    logit("et","SOCKET_DEBUG: Exiting %s\n",MyFuncName);

  return(retVal);
}

/*************************************************************/

SOCKET accept_ew(SOCKET s, struct sockaddr FAR* addr, int FAR* addrlen,
		 int timeout_msec)
{
  /* accept_ew() attempts to accept a connection on a socket.
     Arguments s, addr, addrlen are passed directly to accept(),
     timeout_msec: length of time in milliseconds that accept_ew()
      will wait before returning. Timeout is measure from the
      point after the initial accept() call.
     Pass timeout of -1 for accept_ew to revert to blocking
      accept() call.
     If no connection is accepted before the timeout expires,
      or if an error occurs, the function returns INVALID_SOCKET.
     Caller can call socketGetError_ew() for details about
     any failures.
     If the latest socket error was WOULDBLOCK_EW, then
      no connections were made during the timeout period.
  */

  SOCKET newSocket = (SOCKET)0;
  static char * MyFuncName = "accept_ew()";
  Time_ew StartTime;
  fd_set AcceptedSockets;
  struct timeval SelectTimeout;
  Time_ew timeout=adjustTimeoutLength(timeout_msec);
  int retVal;
  long lOnOff;
  int sel;

  if(EW_SOCKET_DEBUG)
    logit("et","SOCKET_DEBUG: Entering %s\n",MyFuncName);

  /* If there is no timeout, make the socket blocking */
  if(timeout_msec == -1)
  {
    lOnOff = 0;
    retVal=ioctlsocket(s,FIONBIO,(u_long *)&lOnOff);

    if (retVal < 0)
    {
      if(EW_SOCKET_DEBUG)
        logit("et","SOCKET_DEBUG: Error: %d, occurred in %s during change to blocking\n",
              socketGetError_ew(),MyFuncName);
      goto Abort;
    }
  }

  newSocket = accept( s, addr, (socklen_t *) addrlen );

  /* If there is no timeout, then the call was made blocking,
     change it back so that we don't screw up any further operations
  */
  if(timeout_msec == -1)
  {
    lOnOff = 1;
    retVal=ioctlsocket(s,FIONBIO,(u_long *)&lOnOff);
    if (retVal==SOCKET_ERROR)
    {
      if(EW_SOCKET_DEBUG)
        logit("et","SOCKET_DEBUG: Error: %d, occurred in %s during change to non-blocking\n",
              socketGetError_ew(),MyFuncName);
      goto Abort;
    }
  }

  if (newSocket == INVALID_SOCKET)
  {
    if (socketGetError_ew() == WOULDBLOCK_EW)
    {
      FD_ZERO(&AcceptedSockets);
      FD_SET(s,&AcceptedSockets);
      StartTime=GetTime_ew();
      while( (sel = select(s+1, &AcceptedSockets, 0, 0,
                           resetTimeout(&SelectTimeout))) == 0)
      { /* select timed out; if timeout hasn't expired, reset and try again */
        if ( GetTime_ew() - timeout > StartTime )
        {
          /* We need to set the error code to WOULDBLOCK_EW, because select() above
		   * will reset the code to 0.
		   */
          socketSetError_ew(WOULDBLOCK_EW);
          return INVALID_SOCKET;
        }

        FD_ZERO(&AcceptedSockets);
        FD_SET(s,&AcceptedSockets);
        sleep_ew(1000);  /* Sleep for a second, and then try again.*/
      }
      if (sel < 0 && EW_SOCKET_DEBUG)
      {
        logit("et", "SOCKET_DEBUG: Error %d occured during select() in %s\n",
              socketGetError_ew(), MyFuncName);
        goto Abort;
      }
      newSocket = accept( s, addr, (socklen_t *) addrlen );
    }
    if(newSocket == INVALID_SOCKET && EW_SOCKET_DEBUG)
    {
      logit("et","SOCKET_DEBUG: Error: %d, occurred in %s\n",
            socketGetError_ew(),MyFuncName);
    }
  }

  /* Set the new socket to non-blocking mode */
  lOnOff = 1;
  retVal = ioctlsocket(newSocket,FIONBIO,(u_long *)&lOnOff);
  if (retVal == SOCKET_ERROR)
  {
    if (EW_SOCKET_DEBUG)
      logit("et","SOCKET_DEBUG: Error: %d, occurred in %s setting new socket to non-blocking\n",
            socketGetError_ew(),MyFuncName);
    goto Abort;
  }
  return(newSocket);

Abort:
  if (newSocket > 0) closesocket_ew(newSocket, 0);
  newSocket = INVALID_SOCKET;
  closesocket_ew(s, 0);
  s = INVALID_SOCKET;
  return(newSocket);
}

/*************************************************************/

int recv_all (SOCKET s,char FAR* buf,int len,int flags, int timeout_msec)
{
  /* recv_all attempts to receive data on a connection oriented scoket.
     buf:     buffer for incoming data, which must be provided by the caller
     len:     number of bytes to read; buffer must be at least len + 1 bytes.
     flags:   flags that are passed directly to recv().
     timeout: length of time in milliseconds that the recv_ew() will wait
     before returning(if no data is received), after making the initial recv()
     call.

     If timeout_msec > 0, recv_all() returns when the sooner of two things
     happens:
     1.  The timeout from the time of the first recv() call, expires;
     2.  "len" bytes of data are received.

     recv_all() returns the number of bytes of data received, or SOCKET_ERROR
     on error.  The caller is responsible for noting any discrepencies in the
     difference between the number of bytes requested to be sent, and the
     number of reported bytes sent.  If there is a discrepency, then a timeout
     or error occured. Caller can call socketGetError_ew() for details about
     any failures.

     If timeout_msec == -1, recv_all() sets the socket to blocking and returns
     when:
     1. "len" bytes of data are received.
     2. EOF is detected by recv returning 0 bytes.
     */

  int retVal,ioctlRetVal;
  static char * MyFuncName = "recv_all()";
  fd_set ReadableSockets;
  Time_ew StartTime;
  struct timeval SelectTimeout;
  Time_ew timeout=adjustTimeoutLength(timeout_msec);
  int BytesToRecv = len;
  int BytesRcvd = 0;
  int BytesJustRcvd;
  long lOnOff;
  int sel;

  if(EW_SOCKET_DEBUG)
    logit("et","SOCKET_DEBUG: Entering %s\n",MyFuncName);

  /* If there is no timeout, make the socket blocking */
  if(timeout_msec == -1)
  {
    lOnOff = 0;
    ioctlRetVal=ioctlsocket(s,FIONBIO,(u_long *)&lOnOff);
    if (ioctlRetVal==SOCKET_ERROR)
    {
      if(EW_SOCKET_DEBUG)
        logit("et","SOCKET_DEBUG: Error: %d, occurred in %s during change to blocking\n",
              socketGetError_ew(),MyFuncName);
      return ioctlRetVal;
    }
  }

  StartTime = GetTime_ew();
  while ( BytesRcvd < BytesToRecv )
  {
    if ( (timeout_msec > 0) && ((GetTime_ew() - timeout) > StartTime ))
    {  /* Time is up; return what we got */
      retVal = BytesRcvd;
      goto Done;
    }
    BytesJustRcvd = recv(s, buf + BytesRcvd, BytesToRecv - BytesRcvd, flags);
    if ( BytesJustRcvd == 0 )        /* apparently EOF */
    {
      retVal = BytesRcvd;
      goto Done;
    }
    if ( BytesJustRcvd < 0 ) /* Error happened */
    {
      if ( socketGetError_ew() == WOULDBLOCK_EW )
      {
        FD_ZERO(&ReadableSockets);
        FD_SET(s,&ReadableSockets);
        while( (sel = select(s+1, &ReadableSockets, 0, 0,
                       resetTimeout(&SelectTimeout))) == 0)
        { /* select timed out; if timeout hasn't expired, reset and try again */
          if ( GetTime_ew() - timeout > StartTime )
          {
            retVal = BytesRcvd;
            goto Done;
          }
          FD_ZERO(&ReadableSockets);
          FD_SET(s,&ReadableSockets);
          sleep_ew(100);  /* Wait a while, and then try
                             again */
        }
        if (sel < 0)
        {
          logit("et", "Error %d occured during select() in %s\n",
                socketGetError_ew(), MyFuncName);
          retVal = BytesRcvd;
          goto Done;
        }

        /* Set BytesJustRcvd, so that we are not kicked out of the
           while loop because of a hard error on a recv.  Note: we
           will still be kicked out if we have exceeded the timeout.
        */
        BytesJustRcvd = 0;

		/* Reset the socket error to "blocking"; select() above clears the error. */
        socketSetError_ew(WOULDBLOCK_EW);
      }
      else  /* some other error occured */
      {
        if(EW_SOCKET_DEBUG)
          logit("et","SOCKET_DEBUG: Error: %d, occurred in %s\n",
                socketGetError_ew(),MyFuncName);
        retVal = BytesJustRcvd; /* the error condition */
        goto Done;
      }
    }  /* End of If there was an error on recv() */
    else
    {
      BytesRcvd += BytesJustRcvd;
    }
  }  /* End: while not all data sent */
  retVal = BytesRcvd;

 Done:
  /* If there is no timeout, then the call was made blocking,
     change it back so that we don't screw up any further operations
  */
  if(timeout_msec == -1)
  {
    lOnOff = 1;
    ioctlRetVal=ioctlsocket(s,FIONBIO,(u_long *)&lOnOff);
    if (ioctlRetVal==SOCKET_ERROR)
    {
      if(EW_SOCKET_DEBUG)
        logit("et","SOCKET_DEBUG: Error: %d, occurred in %s during change to non-blocking\n",
              socketGetError_ew(),MyFuncName);
    }
  }

  if(EW_SOCKET_DEBUG)
    logit("et","SOCKET_DEBUG: Exiting %s\n",MyFuncName);

  return(retVal);
}

/*************************************************************/

int recv_ew (SOCKET s, char FAR* buf, int len, int flags, int timeout_msec)
{
  /* recv_ew attempts to receive data on a connection oriented scoket.
     buf:     buffer for incoming data, which must be provided by the caller
     len:     length of the buffer.
     flags:   flags that are passed directly to recv().
     timeout: length of time in milliseconds. that the recv_ew() will wait
     before returning(if no data is received), after making
     the initial recv() call. If data (or a shutdown request) is not
     received before the timeout expires, or if an error occurs, the
     function returns SOCKET_ERROR. As soon as any data is received,
     the function returns; the function does not attempt to completely
     fill the buffer before returning.
     Caller can call socketGetError_ew() for details about any failures.
     If the latest socket error is WOULDBLOCK_EW, then recv_ew timed out
     before receiving any data,

     If (-1) is passed for timeout_msec, then recv_ew() reverts to a blocking
     recv() call.
  */

  int retVal,ioctlRetVal;
  static char * MyFuncName = "recv_ew()";
  fd_set ReadableSockets;
  Time_ew StartTime;
  struct timeval SelectTimeout;
  Time_ew timeout=adjustTimeoutLength(timeout_msec);
  long lOnOff;
  int sel = 0;

  if(EW_SOCKET_DEBUG)
    logit("et","SOCKET_DEBUG: Entering %s\n",MyFuncName);

  /* If there is no timeout, make the socket blocking */
  if(timeout_msec == -1)
  {
    lOnOff = 0;
    ioctlRetVal=ioctlsocket(s,FIONBIO,(u_long *)&lOnOff);
    if (ioctlRetVal==SOCKET_ERROR)
    {
      logit("et","Error: %d, occurred in %s during change to blocking\n",
            socketGetError_ew(),MyFuncName);
    }
  }
  retVal=recv(s,buf,len,flags);

  /* If there is no timeout, then the call was made blocking,
     change it back so that we don't screw up any further operations
  */
  if(timeout_msec == -1)
  {
    lOnOff = 1;
    ioctlRetVal=ioctlsocket(s,FIONBIO,(u_long *)&lOnOff);
      if (ioctlRetVal==SOCKET_ERROR)
      {
        logit("et","Error: %d, occurred in %s during change to non-blocking\n",
              socketGetError_ew(),MyFuncName);
      }
  }

  /* Use select() to wait for something to read. We use a small time interval
   * (0.2 seconds) in select, and check the clock against timeout_msec (here
   * converted to seconds) in a while() loop.
   */
  if (retVal < 0 && socketGetError_ew() == WOULDBLOCK_EW)
  {
    FD_ZERO(&ReadableSockets);
    FD_SET(s,&ReadableSockets);
    StartTime=GetTime_ew();
    while( (sel = select(s+1, &ReadableSockets, 0, 0,
                         resetTimeout(&SelectTimeout))) == 0 )
    {  /* select timed out; if timeout hasn't expired, reset and try again */

      if ( GetTime_ew() - timeout > StartTime )
        break;
      FD_ZERO(&ReadableSockets);
      FD_SET(s,&ReadableSockets);
      sleep_ew(100);  /* Wait a while, and then try again */
    }
    if (sel < 0)
    {
      logit("et", "Error %d occured during select() in %s\n",
            socketGetError_ew(), MyFuncName);
      return(SOCKET_ERROR);
    }
    /* Try to read, even if select() timed out */
    retVal=recv(s,buf,len,flags);
  }

  if(retVal <0  && EW_SOCKET_DEBUG)
  {
    if (sel == 0)
      logit("et", "SOCKET_DEBUG: Timeout occured in %s\n", MyFuncName);
    else
      logit("et","SOCKET_DEBUG: Error: %d, occurred in %s\n",
            socketGetError_ew(),MyFuncName);
  }

  if(EW_SOCKET_DEBUG)
		logit("et","SOCKET_DEBUG: Exiting %s\n",MyFuncName);

  return(retVal);
}

/*************************************************************/

int recvfrom_ew (SOCKET s, char FAR* buf, int len, int flags,
			  struct sockaddr FAR* from, int FAR* fromlen,
			  int timeout_msec)
{

  /* recvfrom_ew() is similar to recv_ew(), except used for datagram
     sockets.  timeout is specified in milliseconds.  Caller can call
     socketGetError_ew() for details about any failures.
  */

  int retVal, ioctlRetVal;
  static char * MyFuncName = "recvfrom_ew()";
  fd_set ReadableSockets;
  Time_ew StartTime;
  struct timeval SelectTimeout;
  Time_ew timeout=adjustTimeoutLength(timeout_msec);
  long lOnOff;
  int sel;
  socklen_t flen;

  if(EW_SOCKET_DEBUG)
    logit("et","SOCKET_DEBUG: Entering %s\n",MyFuncName);


  /* If there is no timeout, make the socket blocking */
  if(timeout_msec == -1)
  {
    lOnOff = 0;
    ioctlRetVal=ioctlsocket(s,FIONBIO,(u_long *)&lOnOff);
    if (ioctlRetVal==SOCKET_ERROR)
    {
      logit("et","Error: %d, occurred in %s during change to blocking\n",
            socketGetError_ew(),MyFuncName);
      /* Should we return this error, or continue? */
    }
  }
  /* Use a local copy of fromlen (because recvfrom sets fromlen=0 if socket is
     non-blocking and there's no data => fromlen=0 input to second recvfrom) */
  flen = *fromlen;
  retVal = recvfrom(s,buf,len,flags,from,&flen);

  /* If there is no timeout, then the call was made blocking,
     change it back so that we don't screw up any further operations */
  if(timeout_msec == -1)
  {
    lOnOff = 1;
    ioctlRetVal=ioctlsocket(s,FIONBIO,(u_long *)&lOnOff);
    if (ioctlRetVal==SOCKET_ERROR)
    {
      logit("et","SOCKET_DEBUG: Error: %d, occurred in %s during change to non-blocking\n",
            socketGetError_ew(),MyFuncName);
    }
  }

  if (retVal < 0 && socketGetError_ew() == WOULDBLOCK_EW)
  {
    FD_ZERO(&ReadableSockets);
    FD_SET(s,&ReadableSockets);
    StartTime=GetTime_ew();
    while( (sel = select(s+1, &ReadableSockets, 0, 0,
                         resetTimeout(&SelectTimeout))) == 0 )
    {  /* select timed out; if timeout hasn't expired, reset and try again */

      if ( GetTime_ew() - timeout > StartTime )
        break;

      FD_ZERO(&ReadableSockets);
      FD_SET(s,&ReadableSockets);
      sleep_ew(100);  /* Wait a while, and then try
                    again */
    }
    if (sel < 0)
    {
      logit("et", "SOCKET_DEBUG: Error %d occured during select() in %s\n",
            socketGetError_ew(), MyFuncName);
      return(SOCKET_ERROR);
    }
    /* Try to read, even if select() timed out */
    flen = *fromlen;
    retVal = recvfrom(s,buf,len,flags,from,&flen);
  }

  if(retVal <0  && EW_SOCKET_DEBUG)
  {
    logit("et","SOCKET_DEBUG: Error: %d, occurred in %s\n",
              socketGetError_ew(),MyFuncName);
  }

  if(EW_SOCKET_DEBUG)
		logit("et","SOCKET_DEBUG: Exiting %s\n",MyFuncName);

  *fromlen = flen;
  return(retVal);
}

/*************************************************************/

int send_ew ( SOCKET s, const char FAR * buf, int len, int flags,
	      int timeout_msec)
{
  /* Send `len' bytes from `buf' out a socket `s'.
     Argument `flags' is passed directly to send().
     If timeout_msec > 0, send_ew() returns when the sooner of two things
     happens:
     1.  The timeout measured in milliseconds expires;
     2.  All of the data provided by the caller is sent.
     If timeout_msec == -1, the socket is set to blocking and send_ew()
     returns when all the data is sent or an error occured.
     send_ew() always returns when an unexpected error occurs.
     send_ew() returns the number of bytes of data sent, or
     SOCKET_ERROR on error.  The caller is responsible for noting
     any discrepencies in the difference between the number of bytes
     requested to be sent, and the number of reported bytes sent.  If
     there is a discrepency, then a timeout may have occured.
     Caller can call socketGetError_ew() for details about any failures.
     If the latest socket error was WOULDBLOCK_EW, then
      the timeout occured before all the data was sent.
     */

  int retVal, ioctlRetVal;
  static char * MyFuncName = "send_ew()";
  int BytesToSend=len;
  int BytesSent=0;
  int BytesJustSent=0;
  Time_ew StartTime;
  fd_set WriteableSockets;
  struct timeval SelectTimeout;
  Time_ew timeout=adjustTimeoutLength(timeout_msec);
  long lOnOff;
  int sel, ret;
  int errval[10];
  socklen_t errlen;

  if(EW_SOCKET_DEBUG)
    logit("et","SOCKET_DEBUG: Entering %s\n",MyFuncName);

  /* If there is no timeout, make the socket blocking */
  if(timeout_msec == -1)
  {
    lOnOff = 0;
    ioctlRetVal=ioctlsocket(s,FIONBIO,(u_long *)&lOnOff);
    if (ioctlRetVal==SOCKET_ERROR)
    {
      if(EW_SOCKET_DEBUG)
        logit("et","SOCKET_DEBUG: Error: %d, occurred in %s during change to blocking\n",
              socketGetError_ew(),MyFuncName);
      return (ioctlRetVal);
    }
  }

  StartTime = GetTime_ew();
  while( BytesSent < BytesToSend )
  {
    if ( (timeout_msec >= 0) && ((GetTime_ew() - timeout) > StartTime ))
    {
      retVal = BytesSent;
      goto Done;
    }

    /* Do a last-minute check to make sure there are  */
    /* no errors on the socket. JHL 20070518          */
    errval[0] = 1;
    errlen = 8;
    ret = getsockopt(s, SOL_SOCKET, SO_ERROR, (char *)errval, &errlen);
    if(EW_SOCKET_DEBUG) logit("et","SOCKET_DEBUG: %s getsockopt() s: %d ret: %d %d %d\n",MyFuncName, s, ret, errval[0], errlen );
    if(errval[0] > 1) {
      if(EW_SOCKET_DEBUG)
        logit("et","SOCKET_DEBUG: Error: %d, occurred in %s \n", socketGetError_ew(),MyFuncName);
	  retVal = SOCKET_ERROR;
      return(retVal);
    }
    /* End of check  */

    BytesJustSent = send(s, buf+BytesSent, min(len-BytesSent, MAXSENDSIZE_EW),
                         flags);
    if (BytesJustSent <= 0)
    {
      if (BytesJustSent == 0 || socketGetError_ew() == WOULDBLOCK_EW)
      {
        FD_ZERO(&WriteableSockets);
        FD_SET(s,&WriteableSockets);
        while( (sel = select(s+1, 0, &WriteableSockets, 0,
                             resetTimeout(&SelectTimeout))) == 0)
        {  /* select timed out; if timeout hasn't expired, reset and try again */
          if ( GetTime_ew() - timeout > StartTime )
          {
            retVal = BytesSent;
            goto Done;
          }

          FD_ZERO(&WriteableSockets);
          FD_SET(s,&WriteableSockets);
          sleep_ew(100);  /* Wait a while, and then try again */
        }
        if (sel < 0)
        {
          logit("et", "SOCKET_DEBUG: Error %d occured during select() in %s\n",
                socketGetError_ew(), MyFuncName);
          retVal = BytesSent;
          goto Done;
        }

        /* Set BytesJustSent, so that we are not kicked out of the
        while loop because of a hard error on a send.  Note:  we
        will still be kicked out if we have exceeded the timeout.
        */
        BytesJustSent = 0;
      }
      else  /* some other error occured */
      {
        if(EW_SOCKET_DEBUG)
          logit("et","SOCKET_DEBUG: Error: %d, occurred in %s\n",
                socketGetError_ew(),MyFuncName);
        retVal = BytesSent;
        goto Done;
      }
    }  /* End of If there was an error on send() */
    else
    {
      BytesSent += BytesJustSent;
    }
  }  /* End: while not all data sent */
  retVal = BytesSent;

Done:
  /* If there is no timeout, then the call was made blocking,
  change it back so that we don't screw up any further operations
  */
  if(timeout_msec == -1)
  {
    lOnOff = 1;
    ioctlRetVal=ioctlsocket(s,FIONBIO,(u_long *)&lOnOff);
    if (ioctlRetVal < 0)
    {
      if(EW_SOCKET_DEBUG)
        logit("et","SOCKET_DEBUG: Error: %d, occurred in %s during change to non-blocking\n",
        socketGetError_ew(),MyFuncName);
      retVal = SOCKET_ERROR;
    }
  }

  if(EW_SOCKET_DEBUG)
    logit("et","SOCKET_DEBUG: Exiting %s\n",MyFuncName);

  return(retVal);
}

/*************************************************************/

int sendto_ew (SOCKET s, const char FAR * buf, int len,
	       int flags, const struct sockaddr FAR * to,
	       int tolen, int timeout_msec)
{
  /* sendto_ew() is similar to send_ew(), except used for datagram
     sockets. Once the socket is ready for sending, sendto_ew calls
     sendto() only once. No checks are made to ensure all data is sent.
     Arguments s, flags, to,  and tolen are passed directly to sendto().
     Timeout is specified in milliseconds; value of -1 sets socket to
     blocking mode and turns off timing.
     Caller can call socketGetError_ew() for details about any failures.
  */

  int retVal, ioctlRetVal;
  static char * MyFuncName = "sendto_ew()";
  Time_ew StartTime;
  fd_set WriteableSockets;
  struct timeval SelectTimeout;
  Time_ew timeout=adjustTimeoutLength(timeout_msec);
  long lOnOff;
  int sel;

  if(EW_SOCKET_DEBUG)
    logit("et","SOCKET_DEBUG: Entering %s\n",MyFuncName);

  /* If there is no timeout, make the socket blocking */
  if(timeout_msec == -1)
  {
    lOnOff = 0;
    ioctlRetVal=ioctlsocket(s,FIONBIO,(u_long *)&lOnOff);
    if (ioctlRetVal==SOCKET_ERROR)
    {
      if(EW_SOCKET_DEBUG)
      {
        logit("et","SOCKET_DEBUG: Error: %d, occurred in %s during change to blocking\n",
              socketGetError_ew(),MyFuncName);
      }
      return ioctlRetVal;
    }
  }

  StartTime=GetTime_ew();
  retVal = sendto(s,buf,len,flags,to,tolen);

  /* If there is no timeout, then the call was made blocking,
     change it back so that we don't screw up any further operations
  */
  if(timeout_msec == -1)
  {
    lOnOff = 1;
    ioctlRetVal = ioctlsocket(s,FIONBIO,(u_long *)&lOnOff);
    if (ioctlRetVal < 0)
    {
      if(EW_SOCKET_DEBUG)
        logit("et","SOCKET_DEBUG: Error: %d, occurred in %s during change to non-blocking\n",
        socketGetError_ew(),MyFuncName);
      return SOCKET_ERROR;
    }
  }

  if (retVal < 0 && socketGetError_ew() == WOULDBLOCK_EW)
  {
    FD_ZERO(&WriteableSockets);
    FD_SET(s,&WriteableSockets);
    while( (sel = select(s+1, 0, &WriteableSockets, 0,
                         resetTimeout(&SelectTimeout))) == 0)
    {  /* select timed out; if timeout hasn't expired, reset and try again */
      if ( GetTime_ew() - timeout > StartTime )
        return SOCKET_ERROR;

      FD_ZERO(&WriteableSockets);
      FD_SET(s,&WriteableSockets);
      sleep_ew(100);
      /* Wait a while, and then try again */
    }
    if (sel < 0)
    {
      logit("et", "SOCKET_DEBUG: Error %d occured during select() in %s\n",
            socketGetError_ew(), MyFuncName);
      return SOCKET_ERROR;
    }
    retVal=sendto(s,buf,len,flags,to,tolen);
  }

  if(retVal <0  && EW_SOCKET_DEBUG)
  {
    logit("et","SOCKET_DEBUG: Error: %d, occurred in %s\n",
          socketGetError_ew(),MyFuncName);
  }

  if(EW_SOCKET_DEBUG)
    logit("et","SOCKET_DEBUG: Exiting %s\n",MyFuncName);

  return(retVal);
}

/*************************************************************/

struct timeval FAR * resetTimeout(struct timeval FAR * pSelectTimeout)
{

  /* resetTimeout() reinitializes the TIMEVAL structure used in
     select() calls.  Depending on the OS, the timeout value
     maybe altered during the select() call, and therefore needs
     to be reinitialized before every select() call.
     */
  static char * MyFuncName = "resetTimeout()";
  static int EW_SOCKET_DEBUG_R=0;
  if(EW_SOCKET_DEBUG_R)
    logit("et","SOCKET_DEBUG: Entering %s\n",MyFuncName);

  pSelectTimeout->tv_sec=SELECT_TIMEOUT_SECONDS;
  pSelectTimeout->tv_usec=SELECT_TIMEOUT_uSECONDS;

  if(EW_SOCKET_DEBUG_R)
    logit("et","SOCKET_DEBUG: Exiting %s\n",MyFuncName);

  return(pSelectTimeout);
}

/*************************************************************/

int closesocket_ew(SOCKET s,int HowToClose)
{
  /* closesocket_ew() closes the socket s.  HowToClose indicates
     whether the socket should be closed gracefully or immediately.
     Use SOCKET_CLOSE_IMMEDIATELY_EW or SOCKET_CLOSE_GRACEFULLY_EW
     to indicate closure method.  Caller can call socketGetError_ew()
     for details about any failures.
  */

  /*
    #define SOCKET_CLOSE_IMMEDIATELY_EW 0
    #define SOCKET_CLOSE_GRACEFULLY_EW -1
    #define SOCKET_CLOSE_SIMPLY_EW     -2
  */

  static char * MyFuncName = "closesocket_ew()";
  struct linger Linger_Value;
  int retVal;

  if(EW_SOCKET_DEBUG)
    logit("et","SOCKET_DEBUG: Entering %s\n",MyFuncName);

/* Note1: setsockopt(SO_LINGER) doesn't seem to work on x86 Solaris 2.5,
          at least when the socket isn't connected. WMK 981019
   Note2: Added the case SOCKET_CLOSE_SIMPLY_EW to skip the call to
          setsockopt. This case is used in connect_ew when the connection
          has failed.  LDD 981022
 *************************************************************************/
  if ( HowToClose != SOCKET_CLOSE_SIMPLY_EW )
  {
    if ( HowToClose == SOCKET_CLOSE_IMMEDIATELY_EW )
    {
      Linger_Value.l_onoff=1;     /* Reset or hard close */
      Linger_Value.l_linger=0;    /* Set timeout to 0 seconds */
    }
    else
    {
      Linger_Value.l_onoff=0;     /* Non-blocking graceful close (NBGC) */
      Linger_Value.l_linger=0;
    }

    if ( setsockopt(s,SOL_SOCKET,SO_LINGER,(char *) &Linger_Value,
                    sizeof(struct linger)) == -1 )
    {
      if(EW_SOCKET_DEBUG)
        logit( "et", "SOCKET_DEBUG: closesocket_ew:setsockopt error: %s\n",
               strerror(socketGetError_ew()) );
    }
  }

  retVal=closesocket(s);

  if(EW_SOCKET_DEBUG)
    logit("et","SOCKET_DEBUG: Exiting %s\n",MyFuncName);

  return (retVal);
}

/*************************************************************/

int select_ew (int nfds, fd_set FAR * readfds, fd_set FAR * writefds,
	       fd_set FAR * exceptfds,
	       int timeout_msec)

     /* select_ew() determines the state of sets of sockets, by
     calling select().  Timeout is in milliseconds, and is
     converted by select_ew to the select() timeout structure, and
     passed on (to select()). No "-1" feature here; if you are willing to
     block indefinitely in select(), you might as well wait in the actual
     I/O call instead.
     Caller can call socketGetError_ew() for details about any failures.
     */
{
  int retVal;
  static char * MyFuncName = "select_ew()";
  struct timeval SelectTimeout={0,0};

  if(EW_SOCKET_DEBUG)
    logit("et","SOCKET_DEBUG: Entering %s\n",MyFuncName);

  SelectTimeout.tv_usec=1000 * timeout_msec;

  retVal = select(nfds,readfds,writefds,exceptfds,&SelectTimeout);
  if (retVal < 0  && EW_SOCKET_DEBUG)
    logit("et","SOCKET_DEBUG: Error: %d, occurred in %s\n",
	  socketGetError_ew(),MyFuncName);

  if(EW_SOCKET_DEBUG)
    logit("et","SOCKET_DEBUG: Exiting %s\n",MyFuncName);

  return(retVal);
}

/*************************************************************/

int setSocket_ewDebug(int debug)
{
  /* setSocket_ewDebug() turns debugging on or off for
     the SOCKET_ew routines.
  */
  EW_SOCKET_DEBUG=debug;
  return(0);
}

/*************************************************************/

int setSocket_ewSelectTimeout(unsigned int Timeout)
{
  /* setSocket_ewSelectTimeout() sets the timeout period
     passed to select() calls made internally within the
     SOCKET_ew routines.  The timeout period is in
     milliseconds.
  */
  SELECT_TIMEOUT_uSECONDS=1000*Timeout;
  return(0);
}
