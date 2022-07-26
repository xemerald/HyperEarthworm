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

/******************************************************************************
 *                                                                            *
 *                           get_ip_format_string()                           *
 *                                                                            *
 * Return a text string containing a description of the ip_format returned by *
 * the IP parsing functions.                                                  *
 *                                                                            *
 * Input:   format - enum ip_format returned by the IP parsing functions      *
 * Returns: const char pointer to a description of the ip_format              *
 *                                                                            *
 ******************************************************************************/

#include <parse_ip_address.h>		/* enum ip_format */

const char *get_ip_format_string( enum ip_format format ) {

   switch ( format ) {

      case UNKNOWN_IP_FORMAT:
         return "Unrecognized";
      case VALID_IP_PORT:
         return "IP port no.";
      case IPv4_FORMAT:
         return "IPv4";
      case IPv6_FORMAT:
         return "IPv6";
      case IPv6_WITH_EMBEDDED_IPv4_FORMAT:
         return "IPv6 with embedded IPv4";
      case IPv4_FORMAT_WITH_PORT:
         return "IPv4 with port no.";
      case IPv6_FORMAT_WITH_PORT:
         return "IPv6 with port no.";
      case IPv6_WITH_EMBEDDED_IPv4_FORMAT_WITH_PORT:
         return "IPv6 with embedded IPv4 with port no.";
      case INVALID_IP_FORMAT:
         return "Invalid";
      case INCOMPLETE_IPv4_FORMAT:
         return "Incomplete IPv4";
      case INCOMPLETE_IPv6_FORMAT:
         return "Incomplete IPv6";
      case INVALID_IPv4_FORMAT:
         return "Invalid IPv4";
      case INVALID_IPv4_SEPARATOR:
         return "Invalid IPv4 field separator";
      case INVALID_IPv6_FORMAT:
         return "Invalid IPv6";
      case INVALID_IPv6_SEPARATOR:
         return "Invalid IPv6 field separator";
      case INVALID_IP_PORT:
         return "Invalid IP port no.";
      default:
         return "get_ip_format_string(): Invalid ip_format";

   }

}
