#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <socket_ew.h>

void formatsocketaddress_ew(const struct sockaddr *address, char addrString[64]) {
   // Buffer to contain result (IPv6 sufficient to hold IPv4)
   char addrBuffer[INET6_ADDRSTRLEN];
   void *numericAddress; // Pointer to binary address
   in_port_t port; // Port to print
   
   // Test for address
   if (address == NULL)
      return;

   memset(addrString, 0, INET6_ADDRSTRLEN);
   // Set pointer to address based on address family
   switch (address->sa_family) {
   case AF_INET:
      numericAddress = &((struct sockaddr_in *) address)->sin_addr;
      port = ntohs(((struct sockaddr_in *) address)->sin_port);
      break;
   case AF_INET6:
      numericAddress = &((struct sockaddr_in6 *) address)->sin6_addr;
      port = ntohs(((struct sockaddr_in6 *) address)->sin6_port);
      break;
   default:
      strncpy(addrBuffer, "[unknown type]", sizeof(addrBuffer));    // Unhandled type
      return;
   }
   // Convert binary to printable address
   if (inet_ntop(address->sa_family, numericAddress, addrBuffer, sizeof(addrBuffer)) == NULL) {
      strncpy(addrString, (char *)("[invalid address]"), strlen((char *)"[invalid address]") + 1); // Unable to convert
   }
  else {
    strncpy(addrString, addrBuffer, strlen(addrBuffer));  
  }
}
