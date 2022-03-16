#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <socket_ew.h>

SOCKET setuptcpserver_ew(const char *host, const char *service, int backlog)
{
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
      if(setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(char *)) != 0) {
         //log messages will be printed by client
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
