/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: getstation.c 7675 2019-01-16 06:15:41Z alexander $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.4  2019/01/15 23:00:00  aschnackenberg
 *     modified Add2ServerList to be compatible IPv6
 *
 *     Revision 1.3  2004/08/09 23:29:19  dietz
 *     modified to work with location code
 *
 */
 
/*
  getstation.c
*/

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse_ip_address.h"
#include "earthworm.h"
#include "chron3.h"
#include "ws_clientII.h"
#include "kom.h"

#ifndef INADDR_NONE
#define INADDR_NONE  0xffffffff  /* should be in <netinet/in.h> */
#endif

#define MAX_WAVESERVERS   100
#define MAX_ADRLEN        20
#define MAXTXT           150

typedef struct 
{
	char cWsIp[MAX_WAVESERVERS][MAX_ADRLEN];   /* IP number of a waveserver(s) */
	char cWsPort[MAX_WAVESERVERS][MAX_ADRLEN]; /* Port Number of a waveserver(s) */
	char cWsLabel[MAX_WAVESERVERS][MAXTXT];    /* Label for a waveserver */
	int  iNumWs;                               /* Number of waveservers read */
} GPARM;


/* Function prototypes
 *********************/
void GetConfig( char *, GPARM * );
void LogWsErr( char [], int );
void Add2ServerList( char *, GPARM * );

/* Constants
 ***********/
const long wsTimeout = 30000;                /* milliSeconds to wait for reply */

/****************************************************************************/

int main( int argc, char *argv[] )
{
	int	rc;                  /* Function return code */
	int	i;
	int	iSCNLFound;

	char	*pStaToSearchFor;
	char	*pCompToSearchFor;
	char	*pNetToSearchFor;
	char	*pLocToSearchFor;

	GPARM config_parameters;
	WS_MENU_QUEUE_REC queue;

	queue.head = (WS_MENU) NULL;
	queue.tail = (WS_MENU) NULL;

	/* Check command line arguments
	 ****************************/
	if ( argc != 6 )
	{
		printf( "Usage: getstation <wsv_file> <station> <component> <network> <location>\n" );
		printf( "Where:\n");
		printf( "       <wsv_file>  = file of waveserver addresses in the form:\n");
		printf( "                     Waveserver aaa.bbb.ccc.ddd:ppppp\n");
		printf( "       <station>   = station code,   or wild as a wildcard\n");
		printf( "       <component> = component code, or wild as a wildcard\n");
		printf( "       <network>   = network code,   or wild as a wildcard\n");
		printf( "       <location>  = location code,  or wild as a wildcard\n");
		return -1;
	}

	/* Since our routines call logit, we must initialize it, although we don't
	* want to!
	*/
	logit_init( "getstation", (short) 0, 256, 0 );

	pStaToSearchFor  = argv[2];
	pCompToSearchFor = argv[3];
	pNetToSearchFor  = argv[4];
	pLocToSearchFor  = argv[5];

	/* Zero the wave server arrays
	***************************/
	for ( i = 0; i < MAX_WAVESERVERS; i++ )
	{
		memset( config_parameters.cWsIp[i], 0, MAX_ADRLEN );
		memset( config_parameters.cWsPort[i], 0, MAX_ADRLEN );
	}

	/* read waveservers from config file 
	************************************/
	GetConfig( argv[1], &config_parameters );

	/* Print out startup Info
	 ************************/
	printf ("\ngetstation: Searching for the following SCNL:\n");
	printf ("%s.%s.%s.%s\n\n", pStaToSearchFor, pCompToSearchFor, 
                                   pNetToSearchFor, pLocToSearchFor );

	printf ("getstation: Read in the following %d waveserver(s):\n", 
                 config_parameters.iNumWs);
	for ( i = 0; i < config_parameters.iNumWs; i++)
	{
		printf("\t%s:%s\n", config_parameters.cWsIp[i], config_parameters.cWsPort[i]);
	}
	printf("\n");

	/* Initialize the socket system
	 ****************************/
	SocketSysInit();

	/* Build the current wave server menus
	 ***********************************/
	for ( i = 0; i < config_parameters.iNumWs; i++ )
	{
		rc = wsAppendMenu( config_parameters.cWsIp[i], config_parameters.cWsPort[i], 
                                   &queue, wsTimeout );
		if ( rc != WS_ERR_NONE )
		{
			LogWsErr( "wsAppendMenu", rc );
			return -1;
		}
	}
	if (queue.head == NULL )
	{
		logit("e", "getstation: nothing in server\n");
		exit( 0 );
	}

	/* Print contents of all tanks
 	 ***************************/
	for ( i = 0; i < config_parameters.iNumWs; i++ )
	{
		WS_PSCNL scnlp;

		rc = wsGetServerPSCNL( config_parameters.cWsIp[i], config_parameters.cWsPort[i], 
                                       &scnlp, &queue );
		if ( rc == WS_ERR_EMPTY_MENU ) 
			continue;

		while ( 1 )
		{
			if( strcmp( pStaToSearchFor, scnlp->sta) == 0  ||
			    strcmp( pStaToSearchFor, "wild") == 0         )
			{
				if( strcmp( pCompToSearchFor, scnlp->chan) == 0  ||
				    strcmp( pCompToSearchFor, "wild") == 0          )
				{
					if( strcmp( pNetToSearchFor, scnlp->net) == 0  || 
					    strcmp( pNetToSearchFor, "wild") == 0          )
					{
					   if( strcmp( pLocToSearchFor, scnlp->loc) == 0  || 
					       strcmp( pLocToSearchFor, "wild") == 0          )
					   {

						printf("Found SCNL: ");
						printf("%s.%s.%s.%s ", scnlp->sta, scnlp->chan, 
						        scnlp->net, scnlp->loc);
						printf("on Waveserver %s:%s\n", config_parameters.cWsIp[i], 
						       config_parameters.cWsPort[i]);
						iSCNLFound = 1;
					   }
					}
				}
			}
			if ( scnlp->next == NULL )
				break;
			else
				scnlp = scnlp->next;
		}
	}
	if (iSCNLFound == 0)
	printf("\nSCNL %s.%s.%s.%s not found.\n", pStaToSearchFor, pCompToSearchFor, 
               pNetToSearchFor,pLocToSearchFor);

	/* Release the linked list created by wsAppendMenu
	 ***********************************************/
	wsKillMenu( &queue );

	return 0;
}

/****************************************************************************/

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
		printf( "%s: SCNL not in menu.\n", fun );
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

/****************************************************************************/

void Add2ServerList( char * addr_port, GPARM * Gparm )
{
    char *sep = " \t:"; /* space, tab, and colon */
    char *spaceSep = " \t";
    char *ptr = NULL;
    char *tail = NULL;
    char *spacePtr = NULL;
    int isIPv6 = 0;
    enum ip_format parseResult = 0;

    //first, make sure the separator is present at all:
    ptr = strpbrk(addr_port, sep);
    if(ptr == NULL)
    {
        logit("et", "Add2ServerList: unable to parse address string, check separator\n");
        return;
    }
    //for ease of parsing, if the separator is a space, just replace it with a colon:
    if((spacePtr = strpbrk(addr_port, spaceSep)) != NULL)
    {
        spacePtr[0] = ':';
    }
    //if we have an address separator AND [another] colon, we can assume it is an IPv6 address:
    if((tail = strchr(ptr + 1, ':')) != NULL)
    {
        if((parseResult = parse_ip_address_string(addr_port, Gparm->cWsIp[Gparm->iNumWs], Gparm->cWsPort[Gparm->iNumWs], &isIPv6)) <= 0)
        {
            logit("et", "Add2ServerList: Error parsing address and port\n");
            return;
        }
    } else {
        strncpy(Gparm->cWsIp[Gparm->iNumWs], addr_port, (ptr - addr_port));
        strcpy(Gparm->cWsPort[Gparm->iNumWs], ptr + 1);
    }
	Gparm->iNumWs++;

    return;
}

/****************************************************************************/

#define ncommand 1           /* Number of commands in the config file */

void GetConfig( char *config_file, GPARM *Gparm )
{
	char	init[ncommand];     /* Flags, one for each command */
	int		nmiss;              /* Number of commands that were missed */
	int		nfiles;
	int		i;

	/* Set to zero one init flag for each required command
	 ***************************************************/
	for ( i = 0; i < ncommand; i++ ) 
		init[i] = 0;

	/* Initialize Configuration parameters
	 ***********************************/
	Gparm->iNumWs = 0;

	/* Open the main configuration file
	 ********************************/
	nfiles = k_open( config_file );
	if ( nfiles == 0 )
	{
		printf( "getstation: Error opening configuration file <%s> Exiting.\n",
		config_file );
		exit( -1 );
	}

	/* Process all nested configuration files
	**************************************/
	while ( nfiles > 0 )          /* While there are config files open */
	{
		while ( k_rd() )           /* Read next line from active file  */
		{
			int  success;
			char *com;
			char *str;

			com = k_str();       /* Get the first token from line */

			if ( com == NULL ) 
				continue;    /* Ignore blank lines */
			if ( com[0] == '#' ) 
				continue;    /* Ignore comments */

			/* Open another configuration file
			*******************************/
			if ( com[0] == '@' )
			{
				success = nfiles + 1;
				nfiles  = k_open( &com[1] );
				if ( nfiles != success )
				{
					printf( "getstation: Error opening command file <%s>. Exiting.\n",
					&com[1] );
					exit( -1 );
				}
				continue;
			}

			/* Read configuration parameters
			 *****************************/
			else if ( k_its( "Waveserver" ) )
			{
				if ((Gparm->iNumWs + 1) < MAX_WAVESERVERS)
				{
					str = k_str();
					if ( str != NULL )
					{
						Add2ServerList( str, Gparm );
					}
					init[0] = 1;
				}
				else
				{
					printf ("getstation: Max number of waveservers reached, ignoring this server.\n");
				}
			}

			/* An unknown parameter was encountered
			 ************************************/
			else
			{
				printf( "getstation: <%s> unknown parameter in <%s>\n",
				com, config_file );
				continue;
			}

			/* See if there were any errors processing the command
			 ***************************************************/
			if ( k_err() )
			{
				printf( "getstation: Bad <%s> command in <%s>. Exiting.\n", com,
				config_file );
				exit( -1 );
			}
		}
		nfiles = k_close();
	}

	/* After all files are closed, check flags for missed commands
	 ***********************************************************/
	nmiss = 0;
	for ( i = 0; i < ncommand; i++ )
		if ( !init[i] )
			nmiss++;

	if ( nmiss > 0 )
	{
		printf( "getstation: ERROR, no " );
		if ( !init[0] ) 
			printf( "<Waveserver> " );
			exit( -1 );
	}
	return;
}
