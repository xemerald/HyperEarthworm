#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <socket_ew.h>

/*
 * if successfully establishes a TCP connection, returns valid socket descriptor
 *    if socket() fails, returns -2
 *    if socket() succeeds but connection fails, return -1 (so we can try again later)
 * */
SOCKET setuptcpclient_ew(const char *host, const char *service, int timeout_msec) {
   // Tell the system what kind(s) of address info we want
   struct addrinfo addrCriteria;                   // Criteria for address match
   memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
   addrCriteria.ai_family = AF_UNSPEC;             // v4 or v6 is OK
   addrCriteria.ai_socktype = SOCK_STREAM;         // Only streaming sockets
   addrCriteria.ai_protocol = IPPROTO_TCP;         // Only TCP protocol
   int socketEstablished = 0;                      //Flag to deal with case where socket() fails completely
   struct addrinfo *addr = NULL;

   // Get address(es)
   struct addrinfo *servAddr; // Holder for returned list of server addrs
   int rtnVal = getaddrinfo(host, service, &addrCriteria, &servAddr);
   //XXX XXX XXX
   //TODO: REPLACE WITH APPROPRIATE ERROR MECHANISM!!!
   if (rtnVal != 0) return -2;

   int sock = -1;
   for (addr = servAddr; addr != NULL; addr = addr->ai_next) {
      // Create a reliable, stream socket using TCP
      sock = socket_ew(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
      if (sock < 0)
         continue;  // Socket creation failed; try next address

      //here, we have established a socket, save this info:
      socketEstablished = 1;

      // Establish the connection to the echo server
      if ( connect_ew(sock, addr->ai_addr, addr->ai_addrlen, timeout_msec) == 0)
         break;     // Socket connection succeeded; break and return socket

      closesocket_ew(sock, SOCKET_CLOSE_IMMEDIATELY_EW); // Socket connection failed; try next address
      sock = -1;
   }

   if(!socketEstablished)
   {
      sock = -2;  //this way we can differentiate between socket() failure and connect() failure
   }

   freeaddrinfo(servAddr); // Free addrinfo allocated in getaddrinfo()
   return sock;
}

