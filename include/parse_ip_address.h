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

enum ip_format parse_ip_address_string(const char *str, char hostStr[64], char portStr[10], int *isIPv6);
  /*
    parse_ip_address_string() takes the input 'str', and attempts to separate the input into a valid 
    IPv4 host and port, or an IPv6 host and port.
    The flag 'isIPv6' is set when the address is a properly formatted IPv6 address

    The return value is the enum 'ip_format', whose values are listed above
    This function is the main or preferred way of parsing an IP address, it calls the helper functions
    'parse_ip_address_with_port()' which handles the parsing
    and 'numeric_address_to_string_ew()' to verify that the result is in fact valid
  */
enum ip_format parse_ip_address( const char *, const char **, uint8_t[16] );
enum ip_format parse_ip_address_with_port( const char *, const char **,
                                           uint8_t[16], uint8_t[2] );
/* parse_ipv4_address.c */

enum ip_format parse_ipv4_address( const char *, const char **, uint8_t[4] );

/* parse_ipv6_address.c */

enum ip_format parse_ipv6_address( const char *, const char **, uint8_t[16] );

/* parse_ip_port.c */

enum ip_format parse_ip_port( const char *, const char **, uint8_t[2] );

/* get_ip_format_string.c */

const char *get_ip_format_string( enum ip_format );
