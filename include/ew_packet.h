
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: ew_packet.h 15 2000-02-14 20:06:34Z lucky $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2000/02/14 20:05:54  lucky
 *     Initial revision
 *
 *
 */


/* The definition of an Earthworm UDP packet
   *****************************************/

#ifndef PACKET_H
#define PACKET_H

#define UDP_SIZ   1472        /* Total length of UDP packet in bytes */
#define UDP_HDR      6        /* Bytes in our header */
#define UDP_DAT   1466        /* Available bytes for data */

typedef struct
{
   unsigned char  msgInst;    /* Message Installation (0=>Earthworm) */
   unsigned char  msgType;    /* Message Type */
   unsigned char  modId;      /* Id of module originating message */
   unsigned char  fragNum;    /* Packet number of message; 0=>first */
   unsigned char  msgSeqNum;  /* Message Sequence number  */
   unsigned char  lastOfMsg;  /* 1=> last packet of message, else 0 */
   char     text[UDP_DAT];    /* The cargo bay, as characters */
} PACKET;

#endif
