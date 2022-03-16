
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: int_types.h 6803 2016-09-09 06:06:39Z et $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2000/02/14 20:06:34  lucky
 *     Initial revision
 *
 *
 */


/*	$OpenBSD: types.h,v 1.2 1997/09/21 10:46:17 niklas Exp $	*/
/*	$NetBSD: types.h,v 1.8 1995/04/29 05:28:05 cgd Exp $	*/

/*
 * Sun RPC is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify Sun RPC without charge, but are not authorized
 * to license or distribute it to anyone else except as part of a product or
 * program developed by the user.
 * 
 * SUN RPC IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 * 
 * Sun RPC is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 * 
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY SUN RPC
 * OR ANY PART THEREOF.
 * 
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 * 
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 *
 *	from: @(#)types.h 1.18 87/07/24 SMI
 *	@(#)types.h	2.3 88/08/15 4.0 RPCSRC
 */

/*
 * Rpc additions to <sys/types.h>
 */
#ifndef _RPC_TYPES_H
#define _RPC_TYPES_H

#define	bool_t	int32_t
#define	enum_t	int32_t
#define __dontcare__	-1

#ifndef FALSE
#	define FALSE	(0)
#endif
#ifndef TRUE
#	define TRUE	(1)
#endif
#ifndef NULL
#	define NULL	0
#endif

#define mem_alloc(bsize)	malloc(bsize)
#define mem_free(ptr, bsize)	free(ptr)

#ifndef makedev /* ie, we haven't already included it */
#include <sys/types.h>
#endif
/* withers forced local
#include <sys/time.h> */
/*
#include "time.h"
*/

/* don't want to include 'platform.h' here because it includes 'winsock.h'
    which may declare 'ntohl()' and 'htonl()' causing build errors */

/* For pre-MSVC 2010 define standard int types, otherwise use inttypes.h */
#if defined(_MSC_VER) && _MSC_VER < 1600
  typedef signed char int8_t;
  typedef unsigned char uint8_t;
  typedef signed short int int16_t;
  typedef unsigned short int uint16_t;
  typedef signed int int32_t;
  typedef unsigned int uint32_t;
  typedef signed __int64 int64_t;
  typedef unsigned __int64 uint64_t;
#else
  #include <inttypes.h>
#endif

/* define unsigned integers with slightly different names */
typedef uint32_t u_int32_t;
typedef uint16_t u_int16_t;
typedef uint8_t u_int8_t;
typedef uint64_t u_int64_t;

typedef unsigned int u_int;

#endif /* !_RPC_TYPES_H */
