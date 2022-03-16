/******************************************************************************
 *                                                                            *
 *            Functions for Parsing IP Addresses and Port Numbers             *
 *            ---------------------------------------------------             *
 *                                                                            *
 * See parse_ip_address.c for a description of the IP parsing functions.      *
 *                                                                            *
 * Author: Larry Baker                                                        *
 *         US Geological Survey                                               *
 *         baker@usgs.gov                                                     *
 *                                                                            *
 * History:                                                                   *
 *                                                                            *
 *     3/16/2018  L. Baker      Original version.                             *
 *                                                                            *
 ******************************************************************************/

#include <stdint.h>			/* uint8_t */

#ifdef _WINNT
#include <winsock2.h>			/* struct sockaddr_storage */
#else
#include <sys/socket.h>			/* struct sockaddr_storage */
#endif

enum ip_format {
   /* Format is undefined: ==0 */
   UNKNOWN_IP_FORMAT                        =  0, /* Used internally only */
   /* Format is valid: >0, bits 0-7 encode result */
   VALID_IP_PORT                            =  1, /* Bit mask format modifier */
   IPv4_FORMAT                              =  2, /* Bit mask and format */
   IPv6_FORMAT                              =  4, /* Bit mask and format */
   IPv6_WITH_EMBEDDED_IPv4_FORMAT           =  IPv6_FORMAT | IPv4_FORMAT,
   IPv4_FORMAT_WITH_PORT                    =  IPv4_FORMAT | VALID_IP_PORT,
   IPv6_FORMAT_WITH_PORT                    =  IPv6_FORMAT | VALID_IP_PORT,
   IPv6_WITH_EMBEDDED_IPv4_FORMAT_WITH_PORT =  IPv6_WITH_EMBEDDED_IPv4_FORMAT
                                                           | VALID_IP_PORT,
   /* Format is invalid: <0, bits 0-7 == 0 */
   INVALID_IP_FORMAT                        = ( -1 << 8 ),
   INCOMPLETE_IPv4_FORMAT                   = ( -2 << 8 ),
   INCOMPLETE_IPv6_FORMAT                   = ( -3 << 8 ),
   INVALID_IPv4_FORMAT                      = ( -4 << 8 ),
   INVALID_IPv4_SEPARATOR                   = ( -5 << 8 ),
   INVALID_IPv6_FORMAT                      = ( -6 << 8 ),
   INVALID_IPv6_SEPARATOR                   = ( -7 << 8 ),
   INVALID_IP_PORT                          = ( -8 << 8 )
};

/* parse_ip_address.c */
int formatsocketaddress( const struct sockaddr *address, int addrLen,
                         char hostString[64], char portString[10] );
int numeric_address_to_string( int isIPv6, uint8_t inAddr[16], uint8_t inPort[2],
                               char hostString[64], char portString[10] );

enum ip_format parse_ip_address_string(const char *, char[64], char[10], int*);
enum ip_format parse_ip_address( const char *, const char **, uint8_t[16] );
enum ip_format parse_ip_address_with_port( const char *, const char **,
                                           uint8_t[16], uint8_t[2] );

enum ip_format parse_ipv4_address( const char *, const char **, uint8_t[4] );

enum ip_format parse_ipv6_address( const char *, const char **, uint8_t[16] );

enum ip_format parse_ip_port( const char *, const char **, uint8_t[2] );

const char *get_ip_format_string( enum ip_format );
