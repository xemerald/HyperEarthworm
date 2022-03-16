
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: remote_copy.c 7966 2019-03-20 16:08:14Z stefan $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.2  2007/03/28 14:45:30  paulf
 *     MACOSX additions
 *
 *     Revision 1.1  2005/07/15 18:20:20  friberg
 *     Unix version of libsrc for POSIX systems
 *
 *     Revision 1.2  2004/04/12 22:33:59  dietz
 *     included stdlib.h
 *
 *     Revision 1.1  2000/02/14 18:46:17  lucky
 *     Initial revision
 *
 *
 */

/* 
 * remote_copy.c for Solaris                              981212:JHL
 *
 * Copies a file from local machine to a remote machine using rcp
 *
 */

/* added in platform.h for the Linux build */
#include <platform.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#ifndef _MACOSX
#include <sys/wait.h>
#endif /* _MACOSX - doesn't need this */


/*****************************************************************************
 *  remote_copy( )  copies a local file to a remote Solaris or SunOS machine *
 *****************************************************************************/   
int remote_copy( char  *fullname, /* Name of /dir/file to copy               */
                 char  *tname,    /* Temporary remote file name              */
                 char  *fname,    /* Final remote file name                  */
                 char  *host,     /* Remote machine to copy file to          */
                 char  *dir,      /* Directory on remote machine             */
                 char  *userid,   /* Use this user name on remote machine    */
                 char  *passwd,   /* Userid's password on remote machine     */
                 char  *errtxt,   /* String to return error message in       */
                 pid_t *mypid,    /* ext. process id to preserve re-entrancy */
                 int   *mystat )  /* ext. status variable for re-entrancy    */

/* For this function to work, make sure that the following files are set     *
 * up properly on the remote machine:                                        *
 *   /etc/hosts         must contain address and localhostname               *
 *   /etc/hosts.equiv   must contain local_hostname                          *
 *   .rhosts            in <userid>'s home directory must contain a line:    *
 *                      local_hostname local_username                        *
 *                      describing who is running this program.              *
 *                                                                           *
 *  Also make sure that entries for the remote host are in the               *
 *  local machine's /etc/hosts and /etc/hosts.equiv files.                   */
 
{
    char   subname[] = "remote_copy";
    char   rcppath[175];          /* path to initially copy file to */
    char   tmpname[100];          /* temporary file name (remote)   */
    char   finalname[100];        /* final name for copied file     */
    int    exitstat;

    errtxt[0] = '\0';
/* Build temporary path & final path to write file to
 ****************************************************/
    sprintf( rcppath,   "%s@%s:%s%s",  userid, host, dir, tname );
    sprintf( tmpname,   "%s%s",        dir, tname );
    sprintf( finalname, "%s%s",        dir, fname );

/* Start new process to copy file
 ********************************/
    *mypid = fork1();
    switch( *mypid ) {
        case -1: /* fork failed */
            sprintf( errtxt, "%s <%s>: fork failed", subname, rcppath );
            perror( errtxt );
            return(1);

        case  0: /* in new child process */
            execl( "/usr/bin/rcp", "rcp", "-p", fullname, rcppath, (char *) 0  );
            perror( "remote_copy: execl" );
            exit(1);

        default: /* in parent */
            break;
    }

    if ( waitpid( *mypid, mystat, 0 ) == -1 ) {
        sprintf( errtxt, "%s: <%s>: waitpid error <%d>\n", 
                         subname, rcppath, *mypid);
        return( 2 );
    }

/* See if the child (in this case, rcp) terminated abnormally
 ************************************************************/
    if ( WIFSIGNALED(*mystat) ) {
        sprintf( errtxt, "%s: <%s>: rcp terminated by signal %d <%d>\n", 
                 subname, rcppath, WTERMSIG(*mystat), *mypid ); 
        return( 3 );
    }
    else if ( WIFSTOPPED(*mystat) ) {
        sprintf( errtxt, "%s: <%s>: rcp stopped by signal %d <%d>\n", 
                 subname, rcppath, WSTOPSIG(*mystat), *mypid );      
        return( 3 );
    }
    else if ( WIFEXITED(*mystat) ) {
        exitstat = WEXITSTATUS(*mystat);
        if( exitstat != 0 ) {
            sprintf( errtxt, "%s: <%s>: rcp exited with status %d <%d>\n", 
                     subname, rcppath, exitstat, *mypid );      
            return( 3 );
        }
    }

/* Start new process to rename the remote file
 *********************************************/
    *mypid = fork1();
    switch( *mypid ) {
        case -1: /* fork failed */
            sprintf( errtxt, "%s <%s>: fork failed", subname, fullname );
            perror( errtxt );
            return(1);

        case  0: /* in new child process */
            execl( "/bin/rsh", "rsh", "-l", userid, host,
                   "/usr/bin/mv", tmpname, finalname, (char *) 0  ); 
            perror( "remote_copy: execl" ); 
            exit(1);

        default: /* in parent */
            break;
    }

    if ( waitpid( *mypid, mystat, 0 ) == -1 ) {
        sprintf( errtxt, "%s <%s>: waitpid error", subname, fname );
        return( 2 );
    } 

/* See if the child (in this case, rsh mv) terminated abnormally
 ***************************************************************/
    if ( WIFSIGNALED(*mystat) ) {
        sprintf( errtxt, "%s <%s>: rsh mv terminated by signal %d", 
                subname, fname, WTERMSIG(*mystat) );      
        return( 3 );
    }
    else if ( WIFSTOPPED(*mystat) ) {
        sprintf( errtxt, "%s <%s>: rsh mv stopped by signal %d", 
                subname, fname, WSTOPSIG(*mystat) );      
        return( 3 );
    }
    else if ( WIFEXITED(*mystat) ) {
        exitstat = WEXITSTATUS(*mystat);
        if( exitstat != 0 ) {
            sprintf( errtxt, "%s <%s>: rsh mv exitted with status %d", 
                    subname, fname, exitstat );      
            return( 3 );
        }
    }

/* Everything went smoothly
 **************************/
    return( 0 );
}
