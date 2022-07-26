
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: imp_exp_gen.h 1840 2005-04-28 23:14:19Z dietz $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.3  2005/04/28 23:14:19  dietz
 *     Added HEARTSEQ definition (for export*ack/import_ack) and cleaned
 *     up formatting.
 *
 *     Revision 1.2  2000/07/24 18:54:14  lucky
 *     Added MAX_ALIVE_STR definition so that import and export can
 *     agree on the max length.
 *
 *     Revision 1.1  2000/02/14 20:05:54  lucky
 *     Initial revision
 *
 *
 */

#ifndef IMP_EXP_GEN_H
#define IMP_EXP_GEN_H

/* This is the include file for import_generic and export_generic, created
   to deal with  binary messages . Alex 10/10/96 */
/*
Modified to read binary messages, alex 10/10/96: 
The scheme (I got it from Carl) is define some sacred characters. 
Sacred characters are the start-of-message and end-of-message framing 
characters, and an escape character. The sender's job is to cloak 
unfortunate bit patterns in the data which look like sacred characters 
by inserting before them an 'escape' character.  Our problem here is to 
recognize, and use, the 'real' start- and end-of-messge characters, 
and to 'decloak' any unfortunate look-alikes within the message body.
*/

/* The sacred characters for framing messages 
 ********************************************/
#define STX  2    /* Start Transmission: used to frame beginning of message  */
#define ETX  3    /* End Transmission:   used to frame end of message        */
#define ESC 27    /* Escape: used to 'cloak' unfortunate binary bit patterns */
                  /*         which look like sacred characters               */

/* Define States for Socket Message Interpretation 
 *************************************************/
#define SEARCHING_FOR_MESSAGE_START   0
#define EXPECTING_MESSAGE_START       1
#define ASSEMBLING_MESSAGE            2
  
/* Other useful definitions
 **************************/
#define	MAX_ALIVE_STR  256   /* maximum size of the socket alive string        */
#define INBUFFERSIZE   100   /* buffer Size for Socket Receiving Buffer        */
#define HEARTSEQ       255   /* sequence# always assigned to socket alive msgs */

#endif
