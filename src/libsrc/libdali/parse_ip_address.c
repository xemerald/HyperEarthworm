/******************************************************************************
 *                                                                            *
 *            Functions for Parsing IP Addresses and Port Numbers             *
 *            ---------------------------------------------------             *
 *                                                                            *
 *                             parse_ip_address.c                             *
 *                                                                            *
 * parse_ip_address()   - Parse an IP address.  The IP address can be either  *
 *                        an IPv4 address, an IPv6 address, or an IPv6        *
 *                        address with an embedded IPv4 address.              *
 * parse_ip_address_with_port() - Parse an IP address with an optional port   *
 *                        number.  The IP address can be either an IPv4       *
 *                        address, an IPv6 address, or an IPv6 address with   *
 *                        an embedded IPv4 address.  Optionally follow the IP *
 *                        address with a colon and a port number.             *
 *                                                                            *
 *                            parse_ipv4_address.c                            *
 *                                                                            *
 * parse_ipv4_address() - Parse an IPv4 address.                              *
 *                                                                            *
 *                            parse_ipv6_address.c                            *
 *                                                                            *
 * parse_ipv6_address() - Parse an IPv6 address.                              *
 *                                                                            *
 *                              parse_ip_port.c                               *
 *                                                                            *
 * parse_ip_port()      - Parse an IP port number.  IP port numbers are       *
 *                        decimal digits only, between 0 and 65535.  Leading  *
 *                        zeros are ignored.                                  *
 *                                                                            *
 *                           get_ip_format_string.c                           *
 *                                                                            *
 * get_ip_format_string() - Return a text string containing a description of  *
 *                        the ip_format returned by the IP parsing functions. *
 *                                                                            *
 *                                                                            *
 * References: Internet Protocol Version 6 (IPv6) Addressing Architecture     *
 *             (RFC3513), https://tools.ietf.org/html/rfc3513                 *
 *             IP Version 6 Addressing Architecture (RFC4291),                *
 *             https://tools.ietf.org/html/rfc4291                            *
 *             Uniform Resource Identifier (URI): Generic Syntax (RFC3986),   *
 *             https://tools.ietf.org/html/rfc3986                            *
 *                                                                            *
 *                                                                            *
 * 3.2.2.  Host, Uniform Resource Identifier (URI): Generic Syntax (RFC3986)  *
 *                                                                            *
 *    IPv4address = dec-octet "." dec-octet "." dec-octet "." dec-octet       *
 *                                                                            *
 *      dec-octet   = DIGIT                 ; 0-9                             *
 *                  / %x31-39 DIGIT         ; 10-99                           *
 *                  / "1" 2DIGIT            ; 100-199                         *
 *                  / "2" %x30-34 DIGIT     ; 200-249                         *
 *                  / "25" %x30-35          ; 250-255                         *
 *                                                                            *
 *    IPv6address =                              6( h16 ":" ) ls32            *
 *                  /                       "::" 5( h16 ":" ) ls32            *
 *                  / [               h16 ] "::" 4( h16 ":" ) ls32            *
 *                  / [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32            *
 *                  / [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32            *
 *                  / [ *3( h16 ":" ) h16 ] "::"    h16 ":"   ls32            *
 *                  / [ *4( h16 ":" ) h16 ] "::"              ls32            *
 *                  / [ *5( h16 ":" ) h16 ] "::"              h16             *
 *                  / [ *6( h16 ":" ) h16 ] "::"                              *
 *                                                                            *
 *      ls32        = ( h16 ":" h16 ) / IPv4address                           *
 *                  ; least-significant 32 bits of address                    *
 *                                                                            *
 *      h16         = 1*4HEXDIG                                               *
 *                  ; 16 bits of address represented in hexadecimal           *
 *                                                                            *
 * IP port numbers are 16 bits in both IPv4 and IPv6.  The syntax is:         *
 *                                                                            *
 *    IPport      = DIGIT                   ; 0-9                             *
 *                  / %x31-39 DIGIT         ; 10-99                           *
 *                  / %x31-39 2DIGIT        ; 100-999                         *
 *                  / %x31-39 3DIGIT        ; 1000-9999                       *
 *                  / %x31-35 4DIGIT        ; 10000-59999                     *
 *                  / "6" %x30-34 3DIGIT    ; 60000-64999                     *
 *                  / "65" %x30-34 2DIGIT   ; 65000-65499                     *
 *                  / "655" %x30-32 DIGIT   ; 65500-65529                     *
 *                  / "6553" %x30-35        ; 65530-65535                     *
 *                                                                            *
 *                                                                            *
 * An IPv6address may optionally be enclosed in square brackets ([...]).      *
 * Brackets are required only to disambiguate the separation of the IPv6      *
 * address from the port number.  For example, the two input strings below    *
 * are equivalent:                                                            *
 *                                                                            *
 *    0123:4567:89AB:CDEF:FEDC:BA98:7654:3210:80                              *
 *    [0123:4567:89AB:CDEF:FEDC:BA98:7654:3210]:80                            *
 *                                                                            *
 * Enclosing brackets are not required since the IPv6 address is completely   *
 * defined prior to the colon separating the port number from the IP address. *
 *                                                                            *
 * The parser recognizes any portion of the input string that is a valid IPv4 *
 * or IPv6 address.  Thus, the two input strings below are equivalent:        *
 *                                                                            *
 *    :::80                                                                   *
 *    [::]:80                                                                 *
 *                                                                            *
 * The last two fields of an IPv6 address may be replaced with an embedded    *
 * IPv4 addess, such as:                                                      *
 *                                                                            *
 *    0123:4567:89AB:CDEF:FEDC:BA98:192.168.0.1:80                            *
 *    ::130.118.44.7:80                                                       *
 *    ::0000:130.118.44.7:80                                                  *
 *    ::FFFF:130.118.44.7:80                                                  *
 *                                                                            *
 * An IPv6 address with a ::0000 (::) prefix is termed an "IPv4-compatible    *
 * IPv6 address" (RFC3513).  Those are deprecated (RFC4291).  An IPv6 address *
 * with a ::FFFF prefix is termed an "IPv4-mapped IPv6 address".  The parser  *
 * validates the syntax of an IPv6 address with an embedded IPv4 address, but *
 * does not restrict the value of the IPv6 prefix.                            *
 *                                                                            *
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

#include <ctype.h>			/* isalnum() */
#include <stdint.h>			/* uint8_t */
#include <stdio.h>			/* sscanf() */
#include <stdlib.h>			/* NULL */
#include <string.h>			/* strchr() */

#ifndef _WINNT
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include "parse_ip_address.h"		/* enum ip_format */

#ifdef _WINNT
#include <winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#endif

#define STRICT_IP_FIELD_WIDTHS

#ifndef TRUE
#define TRUE  ( 0 == 0 )
#endif
#ifndef FALSE
#define FALSE ( 0 != 0 )
#endif

enum ip_format
parse_ip_address_string(const char *str, char hostStr[64], char portStr[10], int *isIPv6)
{
   enum ip_format parseResult = 0;
   const char *endPtr;
   uint8_t inAddr[16] = {0};
   uint8_t inPort[2] = {0};
   int errCode = 0;

   parseResult = parse_ip_address_with_port(str, &endPtr, inAddr, inPort);
   if((parseResult & IPv6_FORMAT) == IPv6_FORMAT) {
      *isIPv6 = 1;
   } else if ((parseResult & IPv4_FORMAT) == IPv4_FORMAT) {
      *isIPv6 = 0;
   } else {
      //this must be an error, return here:
      return parseResult;
   }
   if((errCode = numeric_address_to_string(*isIPv6, inAddr, inPort, hostStr, portStr)) != 0) {
      return errCode;
   }

   return parseResult;
}
 
/******************************************************************************
 *                                                                            *
 *                             parse_ip_address()                             *
 *                                                                            *
 * Parse an IP address.  The IP address can be either an IPv4 address, an     *
 * IPv6 address, or an IPv6 address with an embedded IPv4 address.            *
 *                                                                            *
 * Input:   str    - const char pointer to an IP address string               *
 * Outputs: endptr - Address of a const char pointer, or NULL                 *
 *                   If non-NULL, endptr is updated to point past the valid   *
 *                      IP address in str, or to the location in str where    *
 *                      the IP address is invalid                             *
 *          inAddr - 16-byte array to contain the IP address, in network      *
 *                      canonical order (high byte first)                     *
 *                   Only the first 4 bytes are valid for an IPv4 address     *
 *                   inAddr is valid only if a valid IP address is found      *
 * Returns: enum ip_format encoding the result of the parsing                 *
 *          Valid results return a value greater than UNKNOWN_IP_FORMAT       *
 *          The IPv4_FORMAT bit is set for a valid IPv4 address               *
 *          The IPv6_FORMAT bit is set for a valid IPv6 address               *
 *          The IPv6_FORMAT and IPv4_FORMAT bits are set for a valid IPv6     *
 *             address with an embedded IPv4 address                          *
 *                                                                            *
 * get_ip_format_string() returns a pointer to a description of the ip_format *
 *                                                                            *
 ******************************************************************************/

enum ip_format
parse_ip_address( const char *str, const char **endptr, uint8_t inAddr[16] ) {

   enum states {
      IPv4_FIELD,
      IPv6_FIELD,
      FINAL
   } state;

   enum ip_format  format;
   char            c;
   const char     *tok;
   unsigned int    tokLen;


   /* Parse the first field to determine the IP address format */
   tok    = str;
   tokLen = 0;
   for ( c = *str; isalnum( c ); c = *++str )     /* Find the field separator */
      ++tokLen;

   switch ( c ) {

      case '.':                                          /* IPv4 uses periods */
         state = IPv4_FIELD;
         break;

      case ':':                                          /* IPv6 uses colons  */
         state = IPv6_FIELD;
         break;

      case '[':                                      /* IPv6 can be bracketed */
         if ( tokLen == 0 ) {
            state = IPv6_FIELD;
            break;
         }
         /* else state = FINAL */

      default:
         state = FINAL;
         break;

   }

   /* Call the correct IP address parser (tok = start of input string) */
   switch ( state ) {

      case IPv4_FIELD:
         format = parse_ipv4_address( tok, endptr, inAddr );
         break;

      case IPv6_FIELD:
         format = parse_ipv6_address( tok, endptr, inAddr );
         break;

      default:
         format = INVALID_IP_FORMAT;
         if ( endptr != NULL )
            *endptr = tok;
        break;

   }
 
   return format;

}

/******************************************************************************
 *                                                                            *
 *                        parse_ip_address_with_port()                        *
 *                                                                            *
 * Parse an IP address with an optional port number.  The IP address can be   *
 * either an IPv4 address, an IPv6 address, or an IPv6 address with an        *
 * embedded IPv4 address.  Optionally follow the IP address with a colon and  *
 * a port number.                                                             *
 *                                                                            *
 * Input:   str    - const char pointer to an IP address string               *
 * Outputs: endptr - Address of a const char pointer, or NULL                 *
 *                   If non-NULL, endptr is updated to point past the valid   *
 *                      IP address and optional port number in str, or to the *
 *                      location in str where the IP address or port number   *
 *                      is invalid                                            *
 *          inAddr - 16-byte array to contain the IP address, in network      *
 *                      canonical order (high byte first)                     *
 *                   Only the first 4 bytes are valid for an IPv4 address     *
 *                   inAddr is valid only if a valid IP address is found      *
 *          inPort - 2-byte array to contain the IP port number, in network   *
 *                      canonical order (high byte first)                     *
 *                   inPort is valid only if a valid IP address and port      *
 *                      number are found                                      *
 * Returns: enum ip_format encoding the result of the parsing                 *
 *          Valid results return a value greater than UNKNOWN_IP_FORMAT       *
 *          The IPv4_FORMAT bit is set for a valid IPv4 address               *
 *          The IPv6_FORMAT bit is set for a valid IPv6 address               *
 *          The IPv6_FORMAT and IPv4_FORMAT bits are set for a valid IPv6     *
 *             address with an embedded IPv4 address                          *
 *          The VALID_IP_PORT bit is set if a valid port number is found      *
 *                                                                            *
 * get_ip_format_string() returns a pointer to a description of the ip_format *
 *                                                                            *
 ******************************************************************************/

enum ip_format
parse_ip_address_with_port( const char *str, const char **endptr,
                            uint8_t inAddr[16], uint8_t inPort[2] ) {

   const char     *theEndptr;
   enum ip_format  format;


   /* Parse the IP address portion of the input string */
   format = parse_ip_address( str, &theEndptr, inAddr );
   str = theEndptr;                            /* Advance past the IP address */

   /* If the IP address portion is valid, parse the IP port number if the */
   /* next input character is a colon                                     */
   if ( format > UNKNOWN_IP_FORMAT ) {
      if ( *str == ':' ) {
         ++str;                              /* Advance to the IP port number */
         /* If the IP port number is invalid, set the return format to     */
         /* INVALID_IP_PORT; otherwise, preserve the IP address format and */
         /* set the VALID_IP_PORT format bit                               */
         if ( parse_ip_port( str, &theEndptr, inPort ) == INVALID_IP_PORT )
            format = INVALID_IP_PORT;
         else
            format |= VALID_IP_PORT;
      }
   }

   if ( endptr != NULL )
      *endptr = theEndptr;

   return format;

}

/******************************************************************************
 *                                                                            *
 *                              parse_ip_port()                               *
 *                                                                            *
 * Parse an IP port number.  IP port numbers are decimal digits only, between *
 * 0 and 65535.  Leading zeros are ignored.                                   *
 *                                                                            *
 * Input:   str    - const char pointer to an IP port string                  *
 * Outputs: endptr - Address of a const char pointer, or NULL                 *
 *                   If non-NULL, endptr is updated to point past the valid   *
 *                      IP port number in str, or to the location in str      *
 *                      where the IP port number is invalid                   *
 *          inPort - 2-byte array to contain the IP port number, in network   *
 *                      canonical order (high byte first)                     *
 *                   inPort is valid only if a valid IP port number is found  *
 * Returns: enum ip_format VALID_IP_PORT or INVALID_IP_PORT                   *
 *                                                                            *
 ******************************************************************************/

enum ip_format
parse_ip_port( const char *str, const char **endptr, uint8_t inPort[2] ) {

   char            c;
   const char     *tok;
   unsigned int    tokLen;
   enum ip_format  format;
   uint32_t        port;
   const char     *s;


   tok    = str;
   tokLen = 0;
   for ( c = *str; isdigit( c ); c = *++str )
      ++tokLen;
#if ( DEBUG > 1 )
   fprintf( stderr, "IP_PORT: \"%.*s\"\n", tokLen, tok );
#endif
   format = VALID_IP_PORT;
   if ( tokLen == 0 )
      format = INVALID_IP_PORT;
#ifdef STRICT_IP_FIELD_WIDTHS
   else if ( tokLen > 5 ) {
      format = INVALID_IP_PORT;
      str = tok + 5;                                   /* Error is at tok + 5 */
   }
#endif
   else {
      port = 0;
      for ( s = tok; s < str; ++s ) {
         port = ( 10 * port ) + ( *s - '0' );
         if ( port > UINT16_MAX ) {
            format = INVALID_IP_PORT;
            str = tok;
            break;
         }
      }
   }
   if ( format == VALID_IP_PORT ) {
      inPort[0] = port >> 8;
      inPort[1] = port & 0xFF;
#if ( DEBUG > 0 )
      fprintf( stderr, "FINAL: \"%.*s\" [%0.2X%0.2X]\n",
                       tokLen, tok, inPort[0], inPort[1] );
#endif
   }

   if ( endptr != NULL )
      *endptr = str;

   return format;

}

/******************************************************************************
 *                                                                            *
 *                            parse_ipv4_address()                            *
 *                                                                            *
 * Parse an IPv4 address.                                                     *
 *                                                                            *
 * Input:   str    - const char pointer to an IPv4 address string             *
 * Outputs: endptr - Address of a const char pointer, or NULL                 *
 *                   If non-NULL, endptr is updated to point past the valid   *
 *                      IPv4 address in str, or to the location in str where  *
 *                      the IPv4 address is invalid                           *
 *          inAddr - 4-byte array to contain the IPv4 address, in network     *
 *                      canonical order (high byte first)                     *
 *                   inAddr is valid only if a valid IPv4 address is found    *
 * Returns: enum ip_format encoding the result of the parsing                 *
 *          Valid results return a value greater than UNKNOWN_IP_FORMAT       *
 *          The IPv4_FORMAT bit is set for a valid IPv4 address               *
 *                                                                            *
 ******************************************************************************/

enum ip_format
parse_ipv4_address( const char *str, const char **endptr, uint8_t inAddr[4] ) {

   enum ip_format  format;
   char            c;
   int             isValid;
   const char     *tok;
   unsigned int    tokLen;
   unsigned int    nFields;
#if ( DEBUG > 0 )
   const char     *field[4];
   unsigned int    i;
#endif
   unsigned int    fieldLen[4];
   const char     *s;
   uint32_t        octet;


   c       = *str;                                   /* Start of input string */
   tok     = str;
   tokLen  = 0;
   nFields = 0;

   format = UNKNOWN_IP_FORMAT;
   while ( format == UNKNOWN_IP_FORMAT ) {

      while ( isdigit( c ) ) {                    /* Find the field separator */
            ++tokLen;
            c = *++str;
      }

      /* We have a candidate IPv4 dotted-decimal octet */

#if ( DEBUG > 1 )
      fprintf( stderr, "IPv4_FIELD: \"%.*s\"\n", tokLen, tok );
#endif
      isValid = ( tokLen > 0 );                                /* 1-3 digits? */
#ifdef STRICT_IP_FIELD_WIDTHS
      if ( tokLen > 3 ) {
         isValid = FALSE;
         str = tok + 3;                                /* Error is at tok + 3 */
      }
#endif
      if ( isValid ) {
         octet = 0;                                      /* Convert to binary */
         for ( s = tok; s < str; ++s ) {
            octet = ( 10 * octet ) + ( *s - '0' );
            if ( octet > UINT8_MAX ) {                              /* 0-255? */
               isValid  = FALSE;
               str = tok;                                  /* Error is at tok */
               break;
            }
         }
      }
      if ( !isValid )
         format = INVALID_IPv4_FORMAT;
      else {                                               /* So far, so good */
#if ( DEBUG > 0 )
         field[nFields]     = tok;                          /* Save the token */
#endif
         fieldLen[nFields]  = tokLen;
         inAddr[nFields]    = octet;
         ++nFields;
         if ( nFields == 4 )       /* We have all 32 bits of the IPv4 address */
            format = IPv4_FORMAT;                                  /* d.d.d.d */
         else if ( c == '.' ) {                   /* . is the field separator */
            c       = *++str;                    /* Advance to the next field */
            tok     = str;                               /* Start a new token */
            tokLen  = 0;
         } else if ( c == '\0' )                       /* End of input string */
            format = INCOMPLETE_IPv4_FORMAT;          /* Incomplete IPv4 addr */
         else                    /* Some other punctuation; invalid separator */
            format = INVALID_IPv4_SEPARATOR;
      }

   }

#if ( DEBUG > 0 )
   if ( format == IPv4_FORMAT )                         /* Valid IPv4 address */
      for ( i = 0; i < nFields; ++i )
         fprintf( stderr, "FINAL: \"%.*s\" [%0.2X]\n",
                          fieldLen[i], field[i], inAddr[i] );
#endif

   if ( endptr != NULL )
      *endptr = str;

   return format;

}

/******************************************************************************
 *                                                                            *
 *                            parse_ipv6_address()                            *
 *                                                                            *
 * Parse an IPv6 address.                                                     *
 *                                                                            *
 * Input:   str    - const char pointer to an IPv6 address string             *
 * Outputs: endptr - Address of a const char pointer, or NULL                 *
 *                   If non-NULL, endptr is updated to point past the valid   *
 *                      IPv6 address in str, or to the location in str where  *
 *                      the IPv6 address is invalid                           *
 *          inAddr - 16-byte array to contain the IPv6 address, in network    *
 *                      canonical order (high byte first)                     *
 *                   inAddr is valid only if a valid IPv6 address is found    *
 * Returns: enum ip_format encoding the result of the parsing                 *
 *          Valid results return a value greater than UNKNOWN_IP_FORMAT       *
 *          The IPv6_FORMAT bit is set for a valid IPv6 address               *
 *          The IPv6_FORMAT and IPv4_FORMAT bits are set for a valid IPv6     *
 *             address with an embedded IPv4 address                          *
 *                                                                            *
 ******************************************************************************/

enum ip_format
parse_ipv6_address( const char *str, const char **endptr, uint8_t inAddr[16] ) {

   enum ip_format  format;
   char            c;
   int             isBracketed;
   int             foundZeros;
   int             isValid;
   const char     *tok;
   unsigned int    tokLen;
   unsigned int    nFields;
#if ( DEBUG > 0 )
   const char     *field[8];
#endif
   unsigned int    fieldLen[8];
   const char     *s;
   uint32_t        hextet;
   unsigned int    i, j;


   c = *str;                                         /* Start of input string */
   isBracketed = ( c == '[' );
   if ( isBracketed )
      c = *++str;
   tok     = str;
   tokLen  = 0;
   nFields = 0;

   foundZeros = FALSE;               /* Flag when :: is found in an IPv6 addr */

   format = UNKNOWN_IP_FORMAT;
   while ( format == UNKNOWN_IP_FORMAT ) {

      while ( isxdigit( c ) ) {       /* Find the field separator punctuation */
            ++tokLen;
            c = *++str;
      }

      /* We have a candidate IPv6 hexadecimal field */

#if ( DEBUG > 1 )
      fprintf( stderr, "IPv6_FIELD: \"%.*s\"\n", tokLen, tok );
#endif
      isValid = TRUE;                               /* Assume hextet is valid */
#ifdef STRICT_IP_FIELD_WIDTHS
      if ( tokLen > 4 ) {                          /* 0-4 hexadecimal digits? */
         isValid = FALSE;
         str = tok + 4;                                /* Error is at tok + 4 */
      }
#endif
      if ( isValid ) {
         hextet = 0;                                     /* Convert to binary */
         for ( s = tok; s < str; ++s ) {
            hextet = ( 16 * hextet ) +
                     ( isdigit( *s ) ? ( *s - '0' )
                                     : ( toupper( *s ) - 'A' ) + 10 );
            if ( hextet > UINT16_MAX ) {                          /* 0-65535? */
               isValid  = FALSE;
               str = tok;                                  /* Error is at tok */
               break;
            }
         }
      }
      if ( !isValid ) {
         if ( foundZeros ) {       /* IPv6 address is valid if it contains :: */
            if ( fieldLen[nFields-1] == 0 )         /* Previous field was ::? */
               str = tok;                                 /* endptr is at tok */
            else                      /* Previous field was terminated by a : */
               str = tok - 1;                         /* endptr is at tok - 1 */
            format = IPv6_FORMAT;
         } else
            format = INVALID_IPv6_FORMAT;
      }
      if ( isValid ) {                                     /* So far, so good */
#if ( DEBUG > 0 )
         field[nFields]      = tok;                         /* Save the token */
#endif
         fieldLen[nFields]   = tokLen;
         inAddr[2*nFields]   = hextet >> 8;        /* Canonical network order */
         inAddr[2*nFields+1] = hextet & 0xFF;
         ++nFields;
         if ( nFields == 8 ) {    /* We have all 128 bits of the IPv6 address */
            format = IPv6_FORMAT;                          /* x:x:x:x:x:x:x:x */
            if ( tokLen == 0 ) {                 /* Is the final field empty? */
               if ( c == ':' ) {                                    /* ...::? */
                  if ( foundZeros ) {         /* Cannot have more than one :: */
                     str = tok - 1;                    /* Error is at tok - 1 */
                     format = INVALID_IPv6_FORMAT;
                  } else {
                     ++str;                  /* Advance past the IPv6 address */
                  }
               } else
                  format = INCOMPLETE_IPv6_FORMAT;          /* x:x:x:x:x:x:x: */
            }
         } else if ( c == ':' ) {                 /* : is the field separator */
            c = *++str;                          /* Advance to the next field */
            if ( tokLen == 0 ) {                                    /* ...::? */
               if ( foundZeros ) {            /* Cannot have more than one :: */
                  if ( fieldLen[nFields-1] == 0 ) {                /* ...:::? */
                     --str;     /* This : is past the end of the IPv6 address */
                     format = IPv6_FORMAT;   /* ...:: terminates IPv6 address */
                  } else {                                     /* ...::x::... */
                     str = tok - 1;                    /* Error is at tok - 1 */
                     format = INVALID_IPv6_FORMAT;
                  }
               } else {
                  foundZeros = TRUE;
                  if ( nFields == 1 ) {                              /* ...:? */
                     if ( c == ':' )                                   /* ::? */
                        c = *++str;              /* Advance to the next field */
                     else {                                          /* :x... */
                        str = tok;                         /* Error is at tok */
                        --nFields;
                        format = INVALID_IPv6_FORMAT;
                     }
                  }
               }
            }
            tok     = str;                               /* Start a new token */
            tokLen  = 0;
         } else if ( ( c == '\0' ) ||                  /* End of input string */
                   ( ( c == ']'  ) && isBracketed ) ) {/* or at the closing ] */
            format = IPv6_FORMAT;
            if ( tokLen == 0 ) {
               --nFields;
               if ( ( nFields > 0 ) && ( fieldLen[nFields-1] > 0 ) )
                  format = INCOMPLETE_IPv6_FORMAT;                   /* ...x: */
            }
         } else {                /* Some other punctuation; invalid separator */

            /* Might be an embedded IPv4 address */

            if ( ( nFields == 7 ) || foundZeros ) { /* If so, it must be last */
               int         theFormat;
               const char *theEndptr;
               uint8_t     IPv4inAddr[4];
               theFormat = parse_ipv4_address( tok, &theEndptr, IPv4inAddr );
               if ( theFormat == IPv4_FORMAT ) {
                  s                   = strchr( str + 1, '.' ) + 1;
                  fieldLen[nFields-1] = s - tok - 1;
#if ( DEBUG > 0 )
                  field[nFields]      = s;
#endif
                  fieldLen[nFields]   = theEndptr - s;
                  ++nFields;
                  i = nFields - 2;
                  inAddr[2*i]   = IPv4inAddr[0];   /* Canonical network order */
                  inAddr[2*i+1] = IPv4inAddr[1];
                  ++i;
                  inAddr[2*i]   = IPv4inAddr[2];
                  inAddr[2*i+1] = IPv4inAddr[3];
                  str = theEndptr;     /* Found a valid IPv4 address; advance */
                  format = IPv6_WITH_EMBEDDED_IPv4_FORMAT;
               }
            }
            if ( format == UNKNOWN_IP_FORMAT ) {
               if ( foundZeros ) { /* IPv6 address is valid if it contains :: */
                  if ( fieldLen[nFields-1] == 0 )   /* Previous field was ::? */
                     str = tok;                           /* endptr is at tok */
                  else                /* Previous field was terminated by a : */
                     str = tok - 1;                   /* endptr is at tok - 1 */
                  format = IPv6_FORMAT;
               } else
                  format = INVALID_IPv6_SEPARATOR;
            }
         }
      }

   }

#if ( DEBUG > 0 )
   for ( i = 0; i < nFields; ++i )
      if ( fieldLen[i] > 0 )
         fprintf( stderr, "FINAL: \"%.*s\" [%0.2X%0.2X]\n",
                          fieldLen[i], field[i], inAddr[2*i], inAddr[2*i+1] );
      else
         fprintf( stderr, "FINAL: \"%.*s\" [0..0]\n", fieldLen[i], field[i] );
#endif

   /* Finish validating the IPv6 address */
   if ( ( format & IPv6_FORMAT ) != 0 ) {         /* IPv6 address looks valid */
      /* Fewer than 8 fields must contain :: */
      if ( ( nFields != 8 ) && !foundZeros )
         format = INCOMPLETE_IPv6_FORMAT;                /* Not enough fields */
      /* Bracketed IPv6 address must be terminated by ] */
      else if ( isBracketed ) {               /* If bracketed, ] must be next */
         if ( c == ']' )
            ++str;                                      /* Advance past the ] */
         else                           /* ] is missing; incomplete IPv6 addr */
            format = INCOMPLETE_IPv6_FORMAT;
      }
   }
   /* Insert zeros for :: */
   if ( ( format & IPv6_FORMAT ) != 0 ) {               /* Valid IPv6 address */
      if ( foundZeros ) {                         /* IPv6 address contains :: */
         j = 7;                   /* Slide the fields after the :: to the end */
         for ( i = nFields-1; fieldLen[i] > 0; --i ) {
            inAddr[2*j+1] = inAddr[2*i+1];
            inAddr[2*j]   = inAddr[2*i];
            --j;
         }
         while ( j > i ) {                      /* Fill in the gap with zeros */
            inAddr[2*j+1] = 0;
            inAddr[2*j]   = 0;
            --j;
         }
      }
   }

#if ( DEBUG > 0 )
   if ( ( format & IPv6_FORMAT ) != 0 )                 /* Valid IPv6 address */
      for ( i = 0; i < 8; ++i )
         fprintf( stderr, "FINAL: [%0.2X%0.2X]\n", inAddr[2*i], inAddr[2*i+1] );
#endif

   if ( endptr != NULL )
      *endptr = str;

   return format;

}


char ip_in_same_subnet(const char * ip1, const char * ip2, char *netmask)
{
    unsigned int ipA, ipB, ipC, ipD;
    unsigned int testA, testB, testC, testD;
    unsigned int maskA, maskB, maskC, maskD;
    char hostStr[64] = {0};
    char portStr[10] = {0};
    int isIPv6 = 0;
    struct in6_addr addr1;
    struct in6_addr addr2;
    struct sockaddr_in6 lookupAddr;
#ifdef _WINNT
    int lookupLength = sizeof(struct sockaddr_in6);
#endif


    lookupAddr.sin6_family = AF_INET6;

    //First, determine whether we are dealing with IPv6 or IPv4:
    if(parse_ip_address_string(ip1, hostStr, portStr, &isIPv6) <= 0) {
      //this is an error! probably should warn but for now just return 0:
      return 0;
    }
    if(!isIPv6) {
       sscanf(ip1, "%u.%u.%u.%u", &ipA, &ipB, &ipC, &ipD);
       sscanf(ip2, "%u.%u.%u.%u", &testA, &testB, &testC, &testD);
       sscanf(netmask, "%u.%u.%u.%u", &maskA, &maskB, &maskC, &maskD);
       if (((ipA & maskA) == (testA & maskA)) && ((ipB & maskB) == (testB & maskB)) && ((ipC & maskC) == (testC & maskC)) && ((ipD & maskD) == (testD & maskD)))
       {
           return 1; /*true*/
       }
      return 0; /*false*/
   } else {
      //In IPv6, the first 48 bits are used for the routing prefix and the next
      //16 bits are used for the subnet ID. Because of this, the first 64 bits
      //(or four 16-bit blocks) must be the same for two addresses to be on the same subnet
      //call inet_pton() to get string addresses into structs:
#ifdef _WINNT
      if(WSAStringToAddress(TEXT(ip1), AF_INET6, NULL, (LPSOCKADDR)&lookupAddr, (LPINT)lookupLength) != 0) {
      	return 0;
      }
      memcpy(&addr1, &(lookupAddr.sin6_addr), sizeof(addr1));
      memset(&(lookupAddr.sin6_addr), 0, sizeof(struct in6_addr));
      if(WSAStringToAddress(TEXT(ip2), AF_INET6, NULL, (LPSOCKADDR)&lookupAddr, (LPINT)lookupLength) != 0) {
        return 0;
      }
      memcpy(&addr2, &(lookupAddr.sin6_addr), sizeof(addr2));
#else
      if(inet_pton(AF_INET6, ip1, &addr1) != 1) {
         //again, should warn, just return 0:
         return 0;
      }
      if(inet_pton(AF_INET6, ip2, &addr2) != 1) {
         return 0;
      }
#endif
      if((addr1.s6_addr[0] == addr2.s6_addr[0]) && (addr1.s6_addr[1] == addr2.s6_addr[1]) && 
         (addr1.s6_addr[2] == addr2.s6_addr[2]) && (addr1.s6_addr[3] == addr2.s6_addr[3]) &&
         (addr1.s6_addr[4] == addr2.s6_addr[4]) && (addr1.s6_addr[5] == addr2.s6_addr[5]) && 
         (addr1.s6_addr[6] == addr2.s6_addr[6]) && (addr1.s6_addr[7] == addr2.s6_addr[7]))
      {
         return 1;
      } 
   }
   return 0;
}

int numeric_address_to_string(int isIPv6, uint8_t inAddr[16], uint8_t inPort[2],
                              char hostString[64], char portString[10])
{
   /* numeric_address_to_string() turns {inAddr, inPort} into strings 
      hostString, portString. 

      returns 0 if successful, a value < 0 if not successful
    */
   int addrLen;
   struct sockaddr_in  ipv4Addr;
   struct sockaddr_in6 ipv6Addr;
   if(isIPv6) {
      memset(&ipv6Addr, 0, sizeof(ipv6Addr));
      ipv6Addr.sin6_family = AF_INET6;
      memcpy(&(ipv6Addr.sin6_addr), inAddr, sizeof(ipv6Addr.sin6_addr));
      memcpy(&(ipv6Addr.sin6_port), inPort, sizeof(ipv6Addr.sin6_port));
      addrLen = sizeof(ipv6Addr);
      return formatsocketaddress( (struct sockaddr *) &ipv6Addr, addrLen, hostString, portString );
   } else {
      memset(&ipv4Addr, 0, sizeof(ipv4Addr));
      ipv4Addr.sin_family = AF_INET;
      memcpy(&(ipv4Addr.sin_addr), inAddr, sizeof(ipv4Addr.sin_addr));
      memcpy(&(ipv4Addr.sin_port), inPort, sizeof(ipv4Addr.sin_port));
      addrLen = sizeof(ipv4Addr);
      return formatsocketaddress( (struct sockaddr *) &ipv4Addr, addrLen, hostString, portString );
   }
}

int formatsocketaddress( const struct sockaddr *address, int addrLen,
                         char hostString[64], char portString[10] ) {
  /* formatsocketaddress() accepts a (typically client) connection specified as 
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
                         portString, portSize, NI_NUMERICSERV)) != 0) {
      strncpy(hostString, (char *)("[invalid address]"), strlen((char *)"[invalid address]") + 1); // Unable to convert
      //for now:
      return err;
   }
   return 0;
}

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
