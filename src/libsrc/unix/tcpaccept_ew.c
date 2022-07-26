#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <socket_ew.h>

//returns value less than 0 if an error
SOCKET accepttcp_ew(SOCKET servSock) {
   struct sockaddr_storage clntAddr; // Client address
   // Set length of client address structure (in-out parameter)
   socklen_t clntAddrLen = sizeof(clntAddr);
   SOCKET clntSock = -1;

   // Wait for a client to connect
   clntSock = accept_ew(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
   
   return clntSock;
}
