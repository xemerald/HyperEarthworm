
/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: getmenu.c 7675 2019-01-16 06:15:41Z alexander $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.8  2019/01/14 19:18:21 aschnackenberg
 *     Fixed parsing of addresses for IPv6 compatibility
 *
 *     Revision 1.7  2005/05/05 16:47:14  mark
 *     Added header files for exit() and isdigit()
 *
 *     Revision 1.6  2004/07/09 18:29:59  lombard
 *     Changed PSCN to PSCNL structure to match recent change in ws_clinetII.
 *
 *     Revision 1.5  2004/06/04 00:14:24  lombard
 *     Don't print the dot after network if location string is empty.
 *     That way you can tell if you are talking to an SCN wave_server vs an SCNL
 *     wave_server.
 *
 *     Revision 1.4  2004/05/18 22:35:26  lombard
 *     Modified for location code
 *
 *     Revision 1.3  2000/08/17 15:55:14  lombard
 *     Fixed bug where index of outer loop was also being used in
 *     inner loops. This bug had no real effect, since the outer
 *     loop could only be traversed once anyway.
 *
 *     Revision 1.2  2000/07/24 19:18:47  lucky
 *     Implemented global limits to module, installation, ring, and message type strings.
 *
 *     Revision 1.1  2000/02/14 17:41:21  lucky
 *     Initial revision
 *
 *
 */

/*
  getmenu.c

  Get the menu from wave_serverV
*/
/* Modified for Y2K compliance: PNL, 11/5/1998 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <platform.h> /* includes system-dependent socket header files */
#include <chron3.h>
#include <earthworm.h>
#include <ws_clientII.h>
#include <socket_ew.h>
#include <parse_ip_address.h>
#include <stdlib.h>
#include <ctype.h>

#ifndef INADDR_NONE
#define INADDR_NONE  0xffffffff  /* should be in <netinet/in.h> */
#endif

#define MAX_WAVESERVERS   10
#define MAX_ADRLEN        64
#define MAX_ADRCMDLEN     255
#define MAXTXT           150

/* Function prototypes
   *******************/
void LogWsErr( char [], int );
void Add2ServerList( char * );

/* Globals to set from configuration file
   **************************************/
char     MyModName[MAX_MOD_STR];                        /* Speak as this module */
int      numserv;                              /* Number of wave servers */
char     wsAddrCmdLine[MAX_WAVESERVERS][MAX_ADRCMDLEN];
char     wsIp[MAX_WAVESERVERS][MAX_ADRLEN];
int      ws_rc[MAX_WAVESERVERS];
char     wsPort[MAX_WAVESERVERS][MAX_ADRLEN];  /* Available wave servers */
int      CalendarDate = 1;                     /* 1: yymmdd etc */
                                               /* 0: for internal tank format*/

/* Constants
   *********/
const long   wsTimeout = 30000;                /* milliSeconds to wait for reply */


/* Time conversion factor for moving between
   Carl Johnson's seconds-since-1600 routines (chron3.c)
   and Solaris' and OS/2's seconds-since-1970 routines
   *****************************************************/
static char   Str1970[18] = "19700101000000.00";
static double Sec1970;


int main( int argc, char *argv[] )
{
  int          rc;                  /* Function return code */
  int          i, j;
  char         line[80];
  WS_MENU_QUEUE_REC queue;

  queue.head = (WS_MENU) NULL;
  queue.tail = (WS_MENU) NULL;

  /* Calculate timebase conversion constant
  **************************************/
  Sec1970 = julsec17( Str1970 );

  /* Check command line arguments
  ****************************/
  if ( argc == 1 || argc-1 > MAX_WAVESERVERS)
    {
      printf( "Usage: getmenu server:port [ server:port [ server:port ] ... ]\n               Max number of servers: %d.\n", MAX_WAVESERVERS );
      return -1;
    }

  /* Since our routines call logit, we must initialize it, although we don't
   * want to!
   */
  logit_init( "getmenu", (short) 0, 256, 0 );


  /* Zero the wave server arrays
  ***************************/
  for ( i = 0; i < MAX_WAVESERVERS; i++ )
    {
      memset( wsAddrCmdLine[i], 0, MAX_ADRCMDLEN );
      memset( wsIp[i], 0, MAX_ADRLEN );
      memset( wsPort[i], 0, MAX_ADRLEN );
      ws_rc[i] = WS_ERR_NONE;
    }

  /* Initialize the socket system
  ****************************/
  SocketSysInit();

  /* Put the server argument into its place */
  for( i = 1; i < argc; i++ ) {
      Add2ServerList( argv[i] );
  }

  /* Build the current wave server menus
  ***********************************/
  for ( i = 0; i < numserv; i++ )
    {
      rc = wsAppendMenu( wsIp[i], wsPort[i], &queue, wsTimeout );
      ws_rc[i] = rc;
      sleep_ew(100);
    }

  if (queue.head == NULL )
    {
      logit("e", "GETMENU: nothing in server");
      if(numserv > 1) {
	logit("e", "s");
      }
      logit("e", "\n");
      exit( 0 );
    }

  /* Print contents of all tanks
  ***************************/
  for ( i = 0; i < numserv; i++ )
    {
      WS_PSCNL scnlp;
      printf( "\nTank contents for %s:%s\n", wsAddrCmdLine[i], wsPort[i]);

      if ( ws_rc[i] != WS_ERR_NONE ) {
	LogWsErr( "wsAppendMenu", ws_rc[i] );
	continue;
      }

      rc = wsGetServerPSCNL( wsIp[i], wsPort[i], &scnlp, &queue );
      if ( rc == WS_ERR_EMPTY_MENU ) continue;

      while ( 1 )
        {
          printf( "ws: %s:%s  ", wsAddrCmdLine[i], wsPort[i] );
          printf( "pin: %4hd", scnlp->pinno );
	  if (strlen(scnlp->loc)) 
	      printf( "  %5s.%-3s.%-2s.%s", scnlp->sta, scnlp->chan, scnlp->net,
		      scnlp->loc);
	  else
	      printf( "  %5s.%-3s.%-2s", scnlp->sta, scnlp->chan, scnlp->net);

          date17( scnlp->tankStarttime+Sec1970, line );
          printf( "  Start: " );
          for ( j = 0; j < 8; j++ ) putchar( line[j] );
          putchar( '_' );
          for ( j = 8; j < 12; j++ ) putchar( line[j] );
          putchar( '_' );
          for ( j = 12; j < 17; j++ ) putchar( line[j] );

          date17( scnlp->tankEndtime+Sec1970, line );
          printf( "  End: " );
          for ( j = 0; j < 8; j++ ) putchar( line[j] );
          putchar( '_' );
          for ( j = 8; j < 12; j++ ) putchar( line[j] );
          putchar( '_' );
          for ( j = 12; j < 17; j++ ) putchar( line[j] );
          putchar( '\n' );

          if ( !CalendarDate )
            {
              printf( "                             " );
              printf( "%17.2f       %17.2f\n",
                      scnlp->tankStarttime, scnlp->tankEndtime);
            }

          if ( scnlp->next == NULL )
            break;
          else
            scnlp = scnlp->next;
        }
    }

  /* Release the linked list created by wsAppendMenu
  ***********************************************/
  wsKillMenu( &queue );

  return 0;
}


void LogWsErr( char fun[], int rc )
{
  switch ( rc )
    {
    case WS_ERR_INPUT:
      printf( "%s: Bad input parameters.\n", fun );
      break;

    case WS_ERR_EMPTY_MENU:
      printf( "%s: Empty menu.\n", fun );
      break;

    case WS_ERR_SERVER_NOT_IN_MENU:
      printf( "%s: Empty menu.\n", fun );
      break;

    case WS_ERR_SCNL_NOT_IN_MENU:
      printf( "%s: SCN not in menu.\n", fun );
      break;

    case WS_ERR_BUFFER_OVERFLOW:
      printf( "%s: Buffer overflow.\n", fun );
      break;

    case WS_ERR_MEMORY:
      printf( "%s: Out of memory.\n", fun );
      break;

    case WS_ERR_BROKEN_CONNECTION:
      printf( "%s: The connection broke.\n", fun );
      break;

    case WS_ERR_SOCKET:
      printf( "%s: Could not get a connection.\n", fun );
      break;

    case WS_ERR_NO_CONNECTION:
      printf( "%s: Could not get a connection.\n", fun );
      break;

    default:
      printf( "%s: unknown ws_client error: %d.\n", fun, rc );
      break;
    }

  return;
}

/******************************************************************
 * Parse server address and port number in format "address:port"; *
 * add them to the server lists.                                  *
 ******************************************************************/
void Add2ServerList( char * addr_port )
{
  int isIPv6 = 0;
  enum ip_format parseResult = 0;
  char *ptr = NULL;
  char *tail = NULL;
  //address strings may be formatted accordingly:
  // addressString => [host_name|ipv4_address|ipv6_address]:port_number
  //
  // when the host is specified by name or ipv4 address, we can just split 
  // on the ':'
  //
  // for IPv6 addresses, it's best to use the function parse_ip_address_string()
  //
  ptr = strchr(addr_port, ':');
  if((tail = strchr(ptr+1, ':')) != NULL)
  {
    if((parseResult = parse_ip_address_string(addr_port, wsIp[numserv], wsPort[numserv], &isIPv6)) <= 0) {
        logit("e", "getmenu: failed to parse IP address %s. Parse result: %s\n", 
                        addr_port, get_ip_format_string(parseResult));
        return;
    }
  } else {
    //we presumably have an IPv4_address:port or host_name:port combo-
    strncpy(wsIp[numserv], addr_port, (ptr - addr_port));
    strcpy(wsPort[numserv], ptr + 1);
  }
  strncpy(wsAddrCmdLine[numserv], wsIp[numserv], sizeof(wsAddrCmdLine[numserv]));
  numserv++;

  return;
}

