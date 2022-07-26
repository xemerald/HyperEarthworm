
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: sendmail.c 6187 2014-11-14 20:36:32Z paulf $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.3  2007/03/28 14:45:30  paulf
 *     MACOSX additions
 *
 *     Revision 1.2  2006/05/19 23:56:20  dietz
 *     Added "from" argument which was required in Windows version. It's a
 *     dummy argument in this Unix version
 *
 *     Revision 1.1  2005/07/15 18:20:20  friberg
 *     Unix version of libsrc for POSIX systems
 *
 *     Revision 1.5  2004/04/12 22:27:51  dietz
 *     included string.h
 *
 *     Revision 1.4  2001/03/15 23:53:31  alex
 *     removed "free( cmnd )" commands, since cmnd is not malloced! Alex
 *
 *     Revision 1.3  2001/03/14 21:11:37  lucky
 *     Changed the way that the message is built and sent -- no more
 *     standard input method -- just echo the message into the mail program.
 *
 *     Revision 1.2  2000/06/30 17:35:12  lombard
 *     Added call to stat() to check for existence of mail program
 *
 *     Revision 1.1  2000/02/14 18:46:17  lucky
 *     Initial revision
 *
 *
 */

                     /*******************************
                      *          sendmail           *
                      *                             *
                      *   Function to send email.   *
                      *******************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <earthworm.h>


#ifdef _SOLARIS
#define DEFAULT_MAIL_PROG	"/usr/ucb/Mail"
#else
#define DEFAULT_MAIL_PROG	"mail"
#endif /* _MACOSX - doesn't need this */

/*
 * SendMail ()
 *   list of receipients
 *   number of emails to send
 *   mail program to use
 *   subject of the message
 *   message to send
 *   message prefix
 *   message suffix
 *   mail server to use
 *
 *
 * Modified by Lucky Vidmar Tue Jan 19 16:05:00 MST 1999
 *  to include the message and the configurable mail program
 *
 */


int SendMail( char person[][60], int nmail, char *mailProg, 
                      char *subject, char *msg, char *msgPrefix, 
                      char *msgSuffix, char *mailServer, char *from )
{
   FILE *f;
   char MsgText[MAX_EMAIL_MSG_SIZE]; 
   char cmnd[2*MAX_EMAIL_MSG_SIZE]; 
   int  i;
   struct stat mailstat;


   if (mailProg == NULL)
   {
      mailProg = (char *) calloc ((strlen (DEFAULT_MAIL_PROG) + 1), 
                          sizeof (char));
      strcpy (mailProg, DEFAULT_MAIL_PROG);
   }

   /* Make sure we can find the mail program */
   if ( stat(mailProg, &mailstat) < 0)
   {
     fprintf(stderr, "SendMail: error accessing mail program <%s>: %s\n",
             mailProg, strerror(errno));
     return( -1 );
   }
   /* Maybe we should check for execute permissions but that takes more work */


   /* Build the message */
   if (msgPrefix != NULL)
       sprintf (MsgText, "%s%s", msgPrefix, msg);
   else 
		sprintf (MsgText, "%s", msg);
   
   if (msgSuffix != NULL)
        strcat (MsgText, msgSuffix);

   for ( i = 0; i < nmail; i++ )
   {
      /* Mail message without a subject */
      if (subject == NULL)
      {
         (void) sprintf( cmnd, "echo \"%s\" | %s %s", 
								MsgText, mailProg, person[i] );
      }
      /* Mail message with the given subject */
      else
      {
         (void) sprintf( cmnd, "echo \"%s\" | %s -s \"%s\" %s", 
									MsgText, mailProg, subject, person[i] );
      }

      if ( ( f = popen( cmnd, "w" ) ) == NULL )
      {
         fprintf( stderr, "sendmail: Can't run command \"%s\"\n", cmnd );
         return( -1 );
      }
      pclose( f );
   }
   return( 0 );
}

