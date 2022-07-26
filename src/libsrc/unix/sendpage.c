/************************************************************************
                                sendpage.c
        	This is the Unix version. Alex 1/19/96
 ************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>

#define PAGEIT "/dev/pagerport"            /* This is port ttya on suns */
#define PAGEIT_BAUD B9600
#ifdef _USE_TERMIOS
# include <sys/stat.h>
# include <fcntl.h>
# include <termios.h>
# ifdef __sgi
#  define PAGEIT "/dev/ttyf2"              /* IRIX Octane onboard serial board */
# endif
# ifdef __hpux
#  include <sys/termiox.h>
#  include <sys/modem.h>
#  define PAGEIT "/dev/cul0p0"
# endif
#else
# include <sgtty.h>

#ifndef _MACOSX
# include <sys/ttold.h>
#endif /* _MACOSX - doesn't have this */

#endif


/***********************************************************************
                             Error Routine 
 
      This routine gets called everytime an error occurs in the code.
          It simply gets the error string and prints it out.
************************************************************************/

int error_routine (char *error_string)
{
   fprintf( stderr, "%s: %s\n", error_string, strerror(errno) );
   return( -1 );
}

/***********************************************************************
                            Time-Out Handler
************************************************************************/

int SendPageHung = 0;               /* Meaning not hung */
jmp_buf ToHere;                     /* Buffer for saving processor state */

#ifdef _USE_POSIX_SIGNALS
void TimeOutHandler(int sig, siginfo_t *sip, void *up)
#else
void TimeOutHandler( int sig )
#endif
{
   SendPageHung = 1;                /* Meaning: yup, we're hung */
   longjmp( ToHere, 0 );            /* Restore state to right after */
                                    /*    the setjmp call */
}


/*************************************************************************
              Send a Pager Request to PAGEIT via Serial Port
                            SendPage( buff ) 

       Will time out after four seconds if anything caused a hang.
       buff = String to output to serial port

   Returns:
        0 => All went well
       -1 => Time out
       -2 => Error while writing to port
*************************************************************************/

int SendPage( char *buff )
{
   int	           fd;          /* File descriptor */
   char            errString[128], ttynam[128];
#ifdef _USE_TERMIOS
   struct termios tty;
#else
   struct sgttyb   tty;
#endif

/* Arrange for timeout alarm
   *************************/
#ifdef _USE_POSIX_SIGNALS
   struct sigaction act;
   act.sa_flags = SA_SIGINFO; sigemptyset(&act.sa_mask);
   act.sa_sigaction = TimeOutHandler;
   sigaction(SIGALRM, &act, (struct sigaction *)NULL);
#else
   signal( SIGALRM, TimeOutHandler );
#endif
   alarm( 4u );

/* Declare not hung
   ****************/
   SendPageHung = 0;

/* Take a snapshot of PC, registers, and stack as they are now
   ***********************************************************/
   setjmp( ToHere );

/* We may be here either by regular program flow, or because the
   timer has run out, the TimeOut Handler has been executed, and
   has returned us to here. And you thought goto's were bad!!!
   *************************************************************/
   if ( SendPageHung == 1 )
   {
      printf( "SendPage has timed out\n" );
      return( -1 );                     /* Meaning time-out; good luck */
   }

/* append path if necessary */
    if ( strncmp(PAGEIT,"/dev/",5) != 0 )
	sprintf(ttynam,"/dev/%s",PAGEIT);
    else
    	strcpy(ttynam,PAGEIT);

/* Open the PAGEIT port and set baud rate
   **************************************/
#ifdef _USE_TERMIOS
   fd = open(ttynam, O_RDWR | O_NDELAY | O_NOCTTY , 0);
#else
   fd = open( ttynam, 1 );               /* 2'nd arg = 1 for write */
#endif
   if ( fd == -1 )
   {
      sprintf(errString,"SendPage: Can't open %s",ttynam);
      error_routine( errString );
      goto fail;
   }

/* Get attributes
   ******************/
#ifdef _USE_TERMIOS
   if ( tcgetattr(fd,&tty) == -1 )
#else
   if ( gtty( fd, &tty ) != 0 )
#endif
   {
      error_routine( "SendPage: Can't get tty modes" );
      goto fail;
   }

#ifdef _USE_TERMIOS
   tty.c_iflag =	IGNBRK	/* ignore break from device */
   		|	IXON	/* enable start/stop output control */
   		|	IXOFF;	/* enable start/stop input control */
   tty.c_oflag =	0;	/* turn off output processing */
   tty.c_cflag = 	CREAD	/* enable receiver */
		|	HUPCL	/* hang-up on last close */
		|	CLOCAL	/* don't monitor modem status */
    		|	PARENB	/* enable parity */
		|	CS7;	/* 7-bit interface */
   tty.c_cflag	&=	~PARODD;/* even parity */
   tty.c_cflag &=	~CSTOPB;/* one stop bit */
   cfsetospeed(&tty, PAGEIT_BAUD);
   cfsetispeed(&tty, PAGEIT_BAUD);
   tty.c_cc[VMIN] =	'\001';	/* one char reads */
   if (  tcsetattr(fd, TCSANOW, &tty) != 0 )
#else
   tty.sg_ispeed = tty.sg_ospeed = PAGEIT_BAUD;
   if ( stty(fd, &tty) != 0 )
#endif

   {
      error_routine( "SendPage: Can't set tty modes" );
      goto fail;
   }

/* Write to PAGEIT
   ***************/
   if ( write(fd, buff, strlen(buff)) == EOF )
   {
      error_routine( "SendPage: Can't write to serial port" );
      goto fail;
   }

   close( fd );
   alarm( (unsigned)0 );
   return( 0 );

/* Something went wrong
   ********************/
fail:
   alarm( (unsigned)0 );
   close( fd );
   return( -2 );
}
