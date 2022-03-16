
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: wait_timer.h 1178 2003-01-30 23:09:19Z lombard $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.2  2003/01/30 23:09:19  lombard
 *     Added multiple-include protection.
 *
 *     Revision 1.1  2000/02/14 20:05:54  lucky
 *     Initial revision
 *
 *
 */

#ifndef WAIT_TIMER_H
#define WAIT_TIMER_H

              /**********************************************
               *                wait_timer.h                *
               *                                            *
               *  Include file for wait_timer functions.    *
               **********************************************/

#include <earthworm.h>

int init_wait_timer( timer_t *, DWORD * );
int start_wait_timer( timer_t, LONG, DWORD * );
int wait_timer( timer_t, DWORD * );

#endif
