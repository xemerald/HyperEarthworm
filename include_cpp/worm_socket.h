//---------------------------------------------------------------------------
#ifndef _WORM_SOCKET_H
#define _WORM_SOCKET_H
//---------------------------------------------------------------------------

#if  defined(_WNNT) || defined(_Windows)

// This include is not used herein, but will prevent
// difficult-to-track-down multiple-declaration conflicts

#define _WINSOCKAPI_  // prevent wacko compilation errors
#include <windows.h>
#include <winsock2.h>

#define WORM_WOULDBLOCK WSAEWOULDBLOCK

#elif  defined(_SOLARIS)

#include <sys/types.h>
#include <socket.h>

#define WORM_WOULDBLOCK EWOULDBLOCK

#else

#error worm_socket.h not completed for this O/S

#endif


enum SOCKET_CLOSE_TYPE
{
   SOCKET_DONT_CLOSE
 , SOCKET_CLOSE_IMMEDIATELY
 , SOCKET_CLOSE_GRACEFULLY
 , SOCKET_CLOSE_SIMPLY
};


// Worm Socket Function Ids
//
// Intended to track the source of socket errors for
// exception handling
//
enum WS_FUNCTION_ID
{
    WSF_NONE
  , WSF_IOCTLSOCK      // ioctlsocket()
  , WSF_SETSOCKOPT     // setsockopt()
  , WSF_GETSOCKOPT     // getsockopt()
  , WSF_GETSOCKOPT_ERR // error obtained by good getsockopt() call
  , WSF_SOCKET         // socket()
  , WSF_INET_ADDR      // inet_addr()
  , WSF_GETHOSTBYADDR  // gethostbyaddr()
  , WSF_CONNECT        // connect()
  , WSF_SELECT         // select()
  , WSF_BIND           // bind()
  , WSF_LISTEN         // listen()
  , WSF_ACCEPT         // accept()
  , WSF_SEND           // send()
  , WSF_RECV           // recv()
  , WSF_RECVFROM       // recvfrom()
};



enum SOCKET_RECV_STATUS
{
    RECV_STATUS_ERROR        = -3 // socket error
  , RECV_STATUS_BADPARAM     = -2 // read request timed out
  , RECV_STATUS_CLIENTCLOSED = -1 // client closed socket gracefully
  , RECV_STATUS_COMPLETE     =  0 // data or complete message obtained
  , RECV_STATUS_NOTHING      =  1 // nothing waiting on socket
  , RECV_STATUS_PARTIAL      =  2 // bytes, but no complete message obtained
  , RECV_STATUS_TIMEDOUT     =  3 // read request timed out
  , RECV_STATUS_CANCELLED    =  4 // cancel flag encountered
};

enum SOCKET_SEND_STATUS
{
    SEND_STATUS_ERROR        = -3 // socket error
  , SEND_STATUS_CLIENTCLOSED = -2 // client closed socket gracefully
  , SEND_STATUS_BADPARAM     = -1 // read request timed out
  , SEND_STATUS_GOOD         =  0
  , SEND_STATUS_TIMEDOUT     =  1 // read request timed out
  , SEND_STATUS_CANCELLED    =  2 // cancel flag encountered
};


#endif

 