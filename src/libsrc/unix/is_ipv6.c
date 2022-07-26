#include <stdio.h>
#include <string.h>
#include <socket_ew.h>

//returns 1 if address is IPv6
//returns 0 if IPv4
//returns -1 if error
//
//XXX XXX XXX: this function doesn't take into account that addresses
//may in fact be available over multiple protocols. It relies on the 
//suggestion that the first host returned by getaddrinfo is
//recommended for use 
int isIPv6(const char *host)
{
   struct addrinfo addrCriteria;
   memset(&addrCriteria, 0, sizeof(addrCriteria));
   addrCriteria.ai_family = AF_UNSPEC;
   addrCriteria.ai_flags = AI_PASSIVE;
   addrCriteria.ai_socktype = SOCK_STREAM;
   addrCriteria.ai_protocol = IPPROTO_TCP;
   int retVal = -1;

   struct addrinfo *servAddr; //linked list for getaddrinfo
   int rtnVal = 0;
   if((rtnVal = getaddrinfo(host, NULL, &addrCriteria, &servAddr)) != 0) {
      //error
      return -1;
   }
   if(servAddr->ai_family = AF_INET6) {
      retVal = 1;
   } else if (servAddr->ai_family = AF_INET) {
      retVal = 0;
   }

   //Free address list allocated by getaddrinfo()
   freeaddrinfo(servAddr);

   return retVal;
}
