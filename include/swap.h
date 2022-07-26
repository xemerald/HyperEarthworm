/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: swap.h 7382 2018-05-02 00:18:51Z baker $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.4  2005/06/13 18:11:06  dietz
 *     add
 *
 *     Revision 1.3  2004/04/13 22:21:43  dietz
 *     added prototype for WaveMsg2MakeLocal()
 *
 *     Revision 1.2  2000/03/09 21:59:17  davidk
 *     added a prototype for SwapFloat(), it had not been inluded in the
 *     list of swap function prototypes.
 *
 *     Revision 1.1  2000/02/14 20:05:54  lucky
 *     Initial revision
 *
 *
 */

#ifndef SWAP_H
#define SWAP_H

#include "earthworm_simple_funcs.h"
#include "trace_buf.h"
#include "platform.h"

#ifdef WIN64  /* use keyword to address alignment issues with 64-bit Windows */
#define EW_UNALIGNED __unaligned
#else
#define EW_UNALIGNED
#endif

/* include file for swap.c: handy routines for swapping earthwormy things */

void SwapUint16( void * );
#define SwapInt16( data ) SwapUint16( data )
#define SwapShort( data ) SwapUint16( data )
void SwapUint32( void * );
#define SwapInt32( data ) SwapUint32( data )
#define SwapInt( data ) SwapUint32( data )
#define SwapFloat( data ) SwapUint32( data )
void SwapUint64( EW_UNALIGNED void * );
#define SwapInt64( data ) SwapUint64( data )
#define SwapDouble( data ) SwapUint64( data )

/* fixes wave message into local byte order, based on globals _SPARC and _INTEL */
int WaveMsgMakeLocal( TRACE_HEADER* );
int WaveMsg2MakeLocal( TRACE2_HEADER* );
int WaveMsg2XMakeLocal( TRACE2X_HEADER* );

#endif
