#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "earthworm.h"
#include "ws_clientIIx.h"
#include "socket_ew.h"
#include "time_ew.h"

#define WS_MAX_RECV_BUF_LEN 4096

static int menu_reqid = 0;
static int WS_CL_DEBUG = WS_DEBUG_SERVER_ERRORS | WS_DEBUG_DATA_ERRORS;

/* constants */

/* new in III */
#define wsNUMBER_OF_MENUS 20

/* not used yet */
#define wsMAX_SCNLs_PER_MENU 256
#define wsMAX_REPLY_LEN \
   ((wsMAX_SCNLs_PER_MENU * 80/*chars per line*/) + 80 /*initial response*/)

/* globals */

static double dAttempting;
static double dNow;

void * pDEBUG;

/* new in III */
int wsFetchMenu( WS_MENU menu, WS_PSCNL *pSCNLBuffer, 
                 int *pSCNLBufferLen, int timeout);
int wsAddMenuToList(wsEnvironmentStruct * pEnv, WS_MENU menu);
int wsMergeMenuIntoPSCNLList(wsEnvironmentStruct *pEnv,
                            WS_PSCNL SCNLReplyList, int SCNLsFound,
                            WS_MENU_REC * pMenu);
static int Compare_WS_PSCNL_RECs(const void *elem1, const void *elem2);
int wsCombinePSCNLs(WS_PSCNL pResult, WS_PSCNL pIn1, WS_PSCNL pIn2);
static int wsConvertLinkedListToArray(WS_MENU menu, WS_PSCNL *pSCNLBuffer, 
                                      int *pSCNLBufferLen);

static int  wsParseMenuReply( WS_MENU, char* );
static void wsSkipN( char*, int, int* );
static int wsRemoveMenuFrompPSCNLBuffer(wsHANDLE pEnv, WS_MENU pMenu);
static int wsWaitForServerResponse(WS_MENU menu, char* buf, int buflen, 
                                   int timeout_msec, int iTraceType);

static int wsParseBinHeaderReply( TRACE_REQ* getThis, char* reply );
static int wsParseAsciiHeaderReply( TRACE_REQ* getThis, char* reply );

static int wsCompareMenuNumbers(const void * pElem1, const void * pElem2);
static int wsCompareMenuNumbersForTraceReqs(const void * pElem1, const void * pElem2);

/* socket timing routines */
Time_ew adjustTimeoutLength(int timeout_msec);
struct timeval FAR * resetTimeout(struct timeval FAR *);

/* Unused Protoypes for internal functions 
static int  wsWaitBinHeader( WS_MENU, char*, int, int );
static int  wsParseBinHeaderReply( TRACE_REQ*, char* );
static int  wsParseAsciiHeaderReply( TRACE_REQ*, char* );
*/

/***************************************************************************
 *      wsInitialize: Allocates a structure that the ws_client routines    *
 *      use for storing data relative to this threads connections and      *
 *      and wave_server information.                                       *
 ***************************************************************************/
int wsInitialize(wsHANDLE* ppEnv, int iMaxSnippetSize)
{
                 /*
                 Arguments:
                  ppEnv:  A pointer to a wsEnvironmentStruct*.  The function
                          allocates a structure and assigns it to the 
                          wsEnvironmentStruct*.

                 Return Codes:  
                  WS_ERR_NONE:  if all went well.
                  WS_ERR_MEMORY: if could not allocate memory.
                 */

  wsEnvironmentStruct * pEnv = NULL;
  int iRetVal = WS_ERR_NONE;

  pEnv = (wsEnvironmentStruct *) malloc(sizeof(wsEnvironmentStruct));
  if(pEnv == NULL)
  {
    iRetVal = WS_ERR_MEMORY;
    goto wsInitialize_Abort;
  }
  /* else */
  memset(pEnv,0,sizeof(wsEnvironmentStruct));

  /* Allocate the trace snippet buffer
  ***********************************/
  if ( (pEnv->bSnippetBuffer = malloc( (size_t)iMaxSnippetSize)) == NULL )
  {
    logit("t","wsInitialize(): Cant allocate snippet buffer of %d bytes. "
             "Quitting\n",iMaxSnippetSize);
    goto wsInitialize_Abort;
  }

  pEnv->iSnippetBufferSize = iMaxSnippetSize;

  *ppEnv=pEnv;

  return(iRetVal);

wsInitialize_Abort:
  if(pEnv)
  {
    if(pEnv->bSnippetBuffer)
      free(pEnv->bSnippetBuffer);
    free(pEnv);
  }
  return(iRetVal);

}/* End wsInitialize() */

/**************************************************************************
 *      wsAddServer: builds a combined menu from many waveservers.        *
 *      Called with the address and port of one waveserver.               *
 *      On the first call it creates a linked list to store the 'menu'    *
 *      reply from the indicated waveserver.                              *
 *      On subsequent calls, it appends the menu replies to the list.     *
 **************************************************************************/

int wsAddServer(wsEnvironmentStruct * pEnv, 
              char* ipAdr, char* port, int timeout, int refreshtime )
                 /*
                 Arguments:
                  ipAdr:  is the dot form of the IP address as a char string.
                  port:  TCP port number as a char string.
                  timeout:  timeout interval in milliseconds.
                  refreshtime:  time in secs between refreshes for this menu
                                0 = default (8 hours = 28800)
                               -1 = never
                 return:  
                 WS_ERR_NONE:  if all went well.
                 WS_ERR_NO_CONNECTION: if we could not get a connection.
                 WS_ERR_SOCKET: if we could not create a socket.
                 WS_ERR_BROKEN_CONNECTION:  if the connection broke.
                 WS_ERR_TIMEOUT: if a timeout occured.
                  WS_ERR_MEMORY: if out of memory.
                  WS_ERR_INPUT: if bad input parameters.
                 WS_ERR_PARSE: if parser failed.
                 */
{
  int ret,  rc;
  WS_MENU menu = NULL;
  WS_PSCNL SCNLReplyList = NULL;
  int SCNLsFound=0;
  time_t tnow;
  
  if ( !ipAdr || !port ||
    (strlen(ipAdr) >= wsADRLEN) || (strlen(port) >= wsADRLEN) )
  {
    ret = WS_ERR_INPUT;
    if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS) 
      logit("","wsAddServer: bad address: %s port: %s\n",  ipAdr, port);
    goto Abort;
  }

  menu = ( WS_MENU_REC* )calloc(sizeof(WS_MENU_REC),1);
  if(menu == NULL)
  {
    ret = WS_ERR_MEMORY;
    logit("t", "wsAddServer: memory allocation error\n");
    goto Abort;
  }
  
  strcpy( menu->addr, ipAdr );
  strcpy( menu->port, port );
  menu->tRefreshInterval = refreshtime;


  if (WS_CL_DEBUG & WS_DEBUG_OVERKILL) 
    logit("", "wsAddServer: calling wsFetchMenu()\n");

  rc=wsFetchMenu( menu, &SCNLReplyList, &SCNLsFound,timeout);
  if(rc!= WS_ERR_NONE  && rc!= WS_ERR_BUFFER_OVERFLOW)
  {
    ret = rc;
    if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS)
      logit("t", "wsAddServer: wsFetchMenu() failed with code [%d]\n", rc);
    goto Abort;
  }

  menu->tNextRefresh = time(&tnow) + menu->tRefreshInterval;

  menu->stats.tServerAdded = time(NULL);

  if(WS_CL_DEBUG & WS_DEBUG_OVERKILL) 
    logit("e", "wsAddServer: calling wsAddMenuToList()\n");


  /* Add the menu to the list */
  wsAddMenuToList(pEnv,menu);


  if(WS_CL_DEBUG & WS_DEBUG_OVERKILL) 
    logit("e", "wsAddServer: calling wsMergeMenuIntoPSCNLList()\n");

  /* Merge the SCNL's into the new list */
  rc=wsMergeMenuIntoPSCNLList(pEnv,SCNLReplyList,SCNLsFound,menu);
  if(rc!= WS_ERR_NONE)
  {
    ret = rc;
    if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS) 
      logit("t", "wsAddServer: wsMergeMenuIntoPSCNLList() failed with code [%d]\n", rc);
    goto Abort;
  }

  ret=WS_ERR_NONE;


Abort:
  if(SCNLReplyList)
  {
    free(SCNLReplyList);
  }

  if (WS_CL_DEBUG & WS_DEBUG_OVERKILL) logit("e", "wsAddServer: done\n");


  return ret;
}  /* end of wsAddServer() */


int wsMergeMenuIntoPSCNLList(wsEnvironmentStruct *pEnv,
                            WS_PSCNL SCNLReplyList, int SCNLsFound,
                            WS_MENU_REC * pMenu)
{
  WS_PSCNL pTemp  = NULL;
  WS_PSCNL pTemp2 = NULL;
  int ret, rc;
  int i,j;
  int rcComp;
  int iNumCurrentSCNLs = 0;
  int ctr;

  pTemp = (WS_PSCNL) malloc((pEnv->iNumPSCNLs + SCNLsFound)  
                            * sizeof(WS_PSCNL_REC));
  if(WS_CL_DEBUG & WS_DEBUG_OVERKILL) 
    logit("", "wsMergeMenuIntoPSCNLList[%p]: after malloc\n", pEnv);

  if(pTemp == NULL)
  {
    ret = WS_ERR_MEMORY;
    logit("t", "wsMergeMenuIntoPSCNLList: memory allocation error for %zu bytes\n",
          (pEnv->iNumPSCNLs + SCNLsFound) * sizeof(WS_PSCNL_REC));
    goto Abort;
  }
  
  /* start at the beginning of the SCNLReplyList */
  i=0;

  if (WS_CL_DEBUG & WS_DEBUG_OVERKILL) 
    logit("", "wsMergeMenuIntoPSCNLList: looping through existing PSCNLs\n");

  /* loop through the PSCNLs in the existing pPSCNLBuffer */
  for(j=0;i<SCNLsFound && j<pEnv->iNumPSCNLs;j++)
  {
    while((rcComp = 
           Compare_WS_PSCNL_RECs(&(SCNLReplyList[i]), &(pEnv->pPSCNLBuffer[j])))
          < 0)
    {
      
      /* we have a new SCNL */
      memcpy(&(pTemp[iNumCurrentSCNLs]), &(SCNLReplyList[i]),  sizeof(WS_PSCNL_REC));
      pTemp[iNumCurrentSCNLs].MenuList[0] = pMenu;
      pTemp[iNumCurrentSCNLs].iNumMenus = 1;
      iNumCurrentSCNLs++;
      
      if(WS_CL_DEBUG & WS_DEBUG_OVERKILL) 
        logit("", "wsMergeMenuIntoPSCNLList: Adding (%s:%s:%s:%s)- %s:%s to PSCNLList\n",
              SCNLReplyList[i].sta, SCNLReplyList[i].chan, SCNLReplyList[i].net,
              SCNLReplyList[i].loc, pMenu->addr, pMenu->port);
      
      i++;  /* move to the next SCNLReplyList SCNL */
      
      if( i >= SCNLsFound)
        break;
    }       /* end while (SCNLReplyList.SCNL < pPSCNLBuffer.SCNL) */


    if(rcComp < 0)
    {
      /* we must have dropped out because i >= SCNLsFound, meaning we're done with
         the "new" menu, but not with the existing combined menu.
         Drop out of the loop, and the logic below will grab the remaining PSCNLs.
         from the existing menu base.
      ****************************************************/
      break;
    }
    else if(rcComp == 0)
    {
      /* we found a matching SCNL in the new menu.  Merge it
         with the existing one.
      ****************************************************/
      SCNLReplyList[i].iNumMenus=1;
      SCNLReplyList[i].MenuList[0]=pMenu;
      rc = wsCombinePSCNLs(&(pTemp[iNumCurrentSCNLs]), &(pEnv->pPSCNLBuffer[j]),
                           &(SCNLReplyList[i]));
      if(rc == WS_ERR_BUFFER_OVERFLOW)
      {
        if(WS_CL_DEBUG & WS_DEBUG_DATA_WARNINGS)
          logit("t","Overflow of server menus supporting [%s,%s,%s,%s]."
                "  Some discarded.\n",
                SCNLReplyList[i].sta, SCNLReplyList[i].chan, 
                SCNLReplyList[i].net, SCNLReplyList[i].loc
               );
      }
      else if(rc == WS_WRN_FLAGGED)
      {
        /* ah crud, we have an existing reference to the new server.
           this shouldn't happen.
        ***********************************************************/
        logit("t","wsMergeMenuIntoPSCNLList():  WARNING WARNING!! "
          "Bug in SCNL processing, existing matching server reference "
          "found.  Deleting current reference.\n"
          );
      }
      else if(rc != WS_ERR_NONE)
      {
        if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS)
          logit("t","%s: unexpected retcode[%d] from %s\n",
                "wsMergeMenuIntoPSCNLList()", rc,
                "wsCombinePSCNLs()");
      }
      
      iNumCurrentSCNLs++;
      /* we should probably do something about storing
      tStart and tEnd for each server's data for each PSCNL
      *******************************************************/

      /* Since we've used this entry from the new list, we need to increment the counter */
      i++;  /* move to the next SCNLReplyList SCNL */
      
    }  /* if SCNLReplyList.SCNL = pPSCNLBuffer.SCNL */
    else
    {
      /* business as usual.  The current record from the new menu comes after the
         current record from the combined menu source.  Add the record from the
         combined menu, and continue to the top of the loop.
      ****************************************************/

      /* we have an existing PSCNL to be copied over to the new list. */
      memcpy(&(pTemp[iNumCurrentSCNLs]), &(pEnv->pPSCNLBuffer[j]), sizeof(WS_PSCNL_REC));
      iNumCurrentSCNLs++;
    }

  }  /* end for i and j in PSCNL lists */

  if(WS_CL_DEBUG & WS_DEBUG_OVERKILL) 
    logit("", "wsMergeMenuIntoPSCNLList: after looping through existing PSCNLs\n");
  
  if(j < pEnv->iNumPSCNLs  &&  i < SCNLsFound)
  {
    logit("et","wsMergeMenuIntoPSCNLList():  ERROR ERROR Internal error!  Bug!\n");
    ret = WS_ERR_UNKNOWN;
    goto Abort;
  }

  if(WS_CL_DEBUG & WS_DEBUG_OVERKILL) 
    logit("", "wsMergeMenuIntoPSCNLList: checking(pEnv->iNumPSCNLs)\n");

  if(j < pEnv->iNumPSCNLs)
  {
    /* the easy case.  Just copy the rest of the old ones over */
    memcpy(&(pTemp[iNumCurrentSCNLs]), &(pEnv->pPSCNLBuffer[j]), 
      (pEnv->iNumPSCNLs - j) * sizeof(WS_PSCNL_REC));
    iNumCurrentSCNLs += pEnv->iNumPSCNLs - j;
  }
  else if (i < SCNLsFound)
  {
    for(ctr=i; ctr < SCNLsFound; ctr++)
    {
      if(WS_CL_DEBUG & WS_DEBUG_OVERKILL) 
        logit("", "wsMergeMenuIntoPSCNLList: Add-ing (%s:%s:%s:%s)- %s:%s to PSCNLList\n",
              SCNLReplyList[ctr].sta, SCNLReplyList[ctr].chan, SCNLReplyList[ctr].net,
              SCNLReplyList[ctr].loc,
              pMenu->addr, pMenu->port);
      
      memcpy(&(pTemp[iNumCurrentSCNLs]), &(SCNLReplyList[ctr]), sizeof(WS_PSCNL_REC));
      pTemp[iNumCurrentSCNLs].MenuList[0] = pMenu;
      pTemp[iNumCurrentSCNLs].iNumMenus = 1;
      iNumCurrentSCNLs++;
    }
  }

  if(WS_CL_DEBUG & WS_DEBUG_OVERKILL) 
    logit("e", "wsMergeMenuIntoPSCNLList: after checking(pEnv->iNumPSCNLs)\n");
  /* now everything is merged into the pTemp list. */
  /* swap lists, so that pPSCNLBuffer now points at the new list,
     and pTemp points at the old. */
  pTemp2 = pTemp;
  pTemp = pEnv->pPSCNLBuffer;
  pEnv->pPSCNLBuffer = pTemp2;

  /* update the number of PSCNLs in the environment struct */
  pEnv->iNumPSCNLs = iNumCurrentSCNLs;

  /* set the return code to no error */
  ret = WS_ERR_NONE;

  if(WS_CL_DEBUG & WS_DEBUG_OVERKILL) 
    logit("", "wsMergeMenuIntoPSCNLList: freeing(pTemp)\n");

Abort:
  if(pTemp)
    free(pTemp);

  if(WS_CL_DEBUG & WS_DEBUG_OVERKILL) 
    logit("", "wsMergeMenuIntoPSCNLList: end!\n");

  return(ret);

}  /* end wsMergeMenuIntoPSCNLList() */


int wsCombinePSCNLs(WS_PSCNL pResult, WS_PSCNL pIn1, WS_PSCNL pIn2)
{

  int i,j,k;
  int bMatches  =FALSE;
  int bOverFlow =FALSE;
  int iRemainingMenusToCopy;
  int ctr;


  /* initialize the counters for the input WS_PSCNLs */
  i=j=0;

  /* copy the the bulk of pIn1 over to pResult */   
  memcpy(pResult, pIn1,  sizeof(WS_PSCNL_REC));

  /* now merge the two menulists */
  for(k=0; i< pIn1->iNumMenus   &&  
           j < pIn2->iNumMenus  &&
           k < (sizeof(pResult->MenuList)/sizeof(WS_MENU)); k++)
  {
    if(pIn1->MenuList[i]->menunum < pIn2->MenuList[j]->menunum)
    {
      /* process 1 */
      pResult->MenuList[k]=pIn1->MenuList[i];
      i++;
    }
    else if(pIn1->MenuList[i]->menunum > pIn2->MenuList[j]->menunum)
    {
      /* process 2 */
      pResult->MenuList[k]=pIn2->MenuList[j];

      if(WS_CL_DEBUG & WS_DEBUG_OVERKILL) 
        logit("", "wsCombinePSCNLs: Adding %s:%s as menu[%d] to chan <%s.%s.%s.%s>\n",
              pIn2->MenuList[j]->addr, pIn2->MenuList[j]->port, k,
              pIn2->sta, pIn2->chan, pIn2->net, pIn2->loc
             );

      j++;
    }
    else
    {
      /* ruh roh!  We have a double match.  Return, success, but with matches */
      bMatches = TRUE;

      /* process both 1&2 */
      pResult->MenuList[k]=pIn1->MenuList[i];
      i++; 
      j++;
    }
  }  /* end for merge of pin1 menus and pin2 menus */

  /* we hit the end of either pIn1, pIn2, or pResult */

  /* check for pResult overflow */
  if(k == (sizeof(pResult->MenuList)/sizeof(WS_MENU)))
  {
    /* we overflowed pResult */
    bOverFlow = TRUE;
  }
  else if(i< pIn1->iNumMenus)  
  {
    iRemainingMenusToCopy = pIn1->iNumMenus - i;
    if((iRemainingMenusToCopy + k) > (sizeof(pResult->MenuList)/sizeof(WS_MENU)))
    {
      iRemainingMenusToCopy = (sizeof(pResult->MenuList)/sizeof(WS_MENU)) - k;
      bOverFlow = TRUE;
    }
    for(ctr=i; ctr < i+ iRemainingMenusToCopy; ctr++)
    {
      pResult->MenuList[k] = pIn1->MenuList[ctr];
      k++;
    }
  }
  else if(j< pIn2->iNumMenus)  
  {
    iRemainingMenusToCopy = pIn2->iNumMenus - j;
    if((iRemainingMenusToCopy + k) > (sizeof(pResult->MenuList)/sizeof(WS_MENU)))
    {
      iRemainingMenusToCopy = (sizeof(pResult->MenuList)/sizeof(WS_MENU)) - k;
      bOverFlow = TRUE;
    }
    for(ctr=j; ctr < j+ iRemainingMenusToCopy; ctr++)
    {
      pResult->MenuList[k] = pIn2->MenuList[ctr];
      if(WS_CL_DEBUG & WS_DEBUG_OVERKILL) 
        logit("", "wsCombinePSCNLs(rem): Adding %s:%s as menu[%d] to chan <%s.%s.%s.%s>\n",
              pIn2->MenuList[ctr]->addr, pIn2->MenuList[ctr]->port, k,
              pIn2->sta, pIn2->chan, pIn2->net, pIn2->loc
             );
      k++;
    }
  } 

  /* DK 021402  Oops we forgot to do this earlier.  Doh!!! */
  pResult->iNumMenus = k;

  if(bOverFlow)
    return(WS_ERR_BUFFER_OVERFLOW);
  else if(bMatches)
    return(WS_WRN_FLAGGED);
  else
    return(WS_ERR_NONE);
}  /* end wsCombinePSCNLs() */



int wsAddMenuToList(wsEnvironmentStruct * pEnv, WS_MENU menu)
{

  WS_MENU * pTemp;
  int ret;

  if ( !pEnv || !menu)
  {
    ret = WS_ERR_INPUT;
    logit("t","wsAddMenuToList: bad params pEnv[%p], menu[%p]\n", pEnv, menu);
    goto Abort;
  }

  /* allocate a larger list if we don't have one or it is already full */
  if(pEnv->iNumMenusInList == pEnv->iMenuListSize)
  {
    pTemp = pEnv->MenuList;
    /* create a new menu list (allocate an array of menu pointers) */
    pEnv->MenuList = (WS_MENU *) 
        malloc((wsNUMBER_OF_MENUS+pEnv->iMenuListSize)  * sizeof(WS_MENU));
    if(pEnv->MenuList == NULL)
    {
      ret = WS_ERR_MEMORY;
      logit("t", "wsAddMenuToList: memory allocation error for %zu bytes\n",
            (wsNUMBER_OF_MENUS+pEnv->iMenuListSize)  * sizeof(WS_MENU));
      goto Abort;
    }
    /* DK Change 02/14/02 */
    pEnv->iMenuListSize += wsNUMBER_OF_MENUS;

    if(pTemp)
    {
      /* DK fixed memory error resulting in segfault 02/14/02 */
      memcpy(pEnv->MenuList,pTemp,pEnv->iNumMenusInList * sizeof(WS_MENU));
      /* end DK 02/14/02 */
      free(pTemp);
    }
  }

  /* append the menu ptr to the end of the list */
  pEnv->MenuList[pEnv->iNumMenusInList] = menu;

  /* get a new menu# */
  (pEnv->MenuList[pEnv->iNumMenusInList])->menunum = pEnv->iNextMenuNum;
  pEnv->iNextMenuNum++;
  pEnv->iNumMenusInList++;
  ret = WS_ERR_NONE;

Abort:

  return(ret);
}  /* end wsAddMenuToList() */

/***************************************************************************
 *      wsFetchMenu: fetches a menu from a single wave_server.  Starts and *
 *      finishes with closed sockets.                                      *
 ***************************************************************************/

int wsFetchMenu( WS_MENU menu, WS_PSCNL *pSCNLBuffer, 
                 int *pSCNLBufferLen, int timeout)
                 /*
                 Arguments:
                  menu:       WS_MENU containing info for the server from 
                              which we are retrieving a menu.
                  pSCNLBuffer: pointer to a WS_PSCNL.  The function will set
                              the WS_PSCNL to a buffer allocated and filled
                              with SCNLs by the function.
                  pSCNLBufferLen:
                              pointer to a caller allocated integer where 
                              the function will write the number of SCNLs
                              retrieved.
                  timeout:    timeout interval in milliseconds.

                 return:  
                  WS_ERR_NONE:  if all went well.
                  WS_ERR_NO_CONNECTION: if we could not get a connection.
                  WS_ERR_SOCKET: if we could not create a socket.
                  WS_ERR_BROKEN_CONNECTION:  if the connection broke.
                  WS_ERR_TIMEOUT: if a timeout occured.
                  WS_ERR_MEMORY: if out of memory.
                  WS_ERR_INPUT: if bad input parameters.
                  WS_ERR_PARSE: if parser failed.
                 */
{


  char request[wsREQLEN];
  int len, ret;
  char reply[wsREPLEN];
  int rc;



  /* Initialize space for reply from wave_server */
  memset(reply,0,wsREPLEN);
  
  /* Connect to the server */
  if ( wsAttachServer( menu, timeout) != WS_ERR_NONE )
  {
    ret = WS_ERR_NO_CONNECTION;
    goto Abort;
  }

  /* Generate a menu request */
  sprintf( request, "MENU: %d SCNL \n", menu_reqid++ );
  len = (int)strlen(request);

  /* Send the menu request */
  if(( ret =  send_ew( menu->sock, request, len, 0, timeout ) ) != len ) 
  {
    if(ret < 0 )
    {
      if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS)
        logit("t", "wsFetchMenu: connection broke to server %s:%s\n",
              menu->addr, menu->port);
      ret = WS_ERR_BROKEN_CONNECTION;
    } 
    else 
    {
     if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS)
       logit("t", "wsFetchMenu: server %s:%s timed out\n",
             menu->addr, menu->port);
      ret = WS_ERR_TIMEOUT;
    }
    goto Abort;
  }
  
  /* Get the menu reply */
  ret = wsWaitForServerResponse( menu, reply, wsREPLEN, timeout, WS_TRACE_ASCII );
  if ( ret != WS_ERR_NONE && ret != WS_ERR_BUFFER_OVERFLOW )
    /* we might have received some useful data in spite of the overflow */
  {
    goto Abort;
  }

  /* We have received the server's reply, so detach! */
  if(WS_CL_DEBUG & WS_DEBUG_SERVER_INFO)
    logit("t","Menu fetched, detaching server\n");
  wsDetachServer(menu, 0);

  if(WS_CL_DEBUG & WS_DEBUG_OVERKILL) 
    logit("e", "wsFetchMenu: calling wsParseMenuReply()\n");

  /* Parse the menu reply */
  if((rc = wsParseMenuReply( menu, reply)) 
     != WS_ERR_NONE )
  {
    if ( ret == WS_ERR_BUFFER_OVERFLOW && rc == WS_ERR_PARSE )
    {
      logit("", "wsFetchMenu: buffer overflow; parse failure\n");
    } 
    else
    {
      ret = rc;
    }
    goto Abort;
  }

  /* make sure that we got something back in the menu reply */
  if ( menu->pscnl == NULL )
  {
    if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS)
      logit("t", "wsFetchMenu: no SCNL at server %s:%s\n", 
            menu->addr, menu->port);
    ret = WS_ERR_EMPTY_MENU;
    goto Abort;
  }

  if(WS_CL_DEBUG & WS_DEBUG_OVERKILL) 
    logit("", "wsFetchMenu: calling wsConvertLinkedListToArray()\n");

  /* convert from the linked list of SCNLs for the menu to an array */
  if((rc = wsConvertLinkedListToArray(menu, pSCNLBuffer, pSCNLBufferLen))
     != WS_ERR_NONE)
  {
    ret=rc;
    logit("","wsConvertLinkedListToArray() failed, returning [%d]\n", rc);
    goto Abort;
  }

  if(WS_CL_DEBUG & WS_DEBUG_OVERKILL) 
    logit("", "wsFetchMenu: calling qsort(pSCNLBuffer)\n");
  /* sort the array of SCNLs */
  qsort(*pSCNLBuffer, *pSCNLBufferLen, sizeof(WS_PSCNL_REC), Compare_WS_PSCNL_RECs);

  /*we're done, pack it up and go home. */
  ret = WS_ERR_NONE;

Abort:
  if(WS_CL_DEBUG & WS_DEBUG_OVERKILL) 
    logit("", "wsFetchMenu: done.\n");
  return(ret);
}  /* end wsFetchMenu() */


static int wsConvertLinkedListToArray(WS_MENU menu, WS_PSCNL *pSCNLBuffer, 
                                      int *pSCNLBufferLen)
{
  /* 
      params:
        menu   a menu ptr for a single server
        pSCNLBuffer
  
      return codes:
        WS_ERR_NONE
        WS_ERR_PARSE
        WS_ERR_MEMORY
  *****************/

  int iNumElements=0;
  WS_PSCNL pSCNL;
  WS_PSCNL SCNLList=NULL;
  int ret;

  /* traverse the linked list of PSCNLs */
  pSCNL=menu->pscnl;
  for(iNumElements = 0; pSCNL!=NULL; pSCNL = pSCNL->next)
  {
    iNumElements++;
  }

  /* if we didn't find any PSCNLs then something went wrong.  Blame
     wsParseMenuReply(), it should not have handed out an empty list.
     Maybe it is wsFetchMenu()'s fault, but it may slap us around if
     we blame it.
  ******************************************************************/
  if(!iNumElements)
  {
    ret = WS_ERR_PARSE;
    logit("t","ERROR: wsConvertLinkedListToArray() could not make sense of "
          "SCNL list generated by wsParseMenuReply() for [%s,%s]\n",
          menu->addr,menu->port);
    goto Abort;
  }
  /*else*/

  /* allocate an array of PSCNLs based on the number of elements we found
     in the linked list above. 
  ******************************************************************/
  SCNLList = (WS_PSCNL) malloc(iNumElements * sizeof(WS_PSCNL_REC));

  if(SCNLList == NULL)
  {
    ret = WS_ERR_MEMORY;
    logit("t", "wsConvertLinkedListToArray(): memory allocation "
          " error for %d WS_PSCNL_RECs\n",
          iNumElements);
    goto Abort;
  }


  /* traverse the linked list of PSCNLs, and copy each one. */
  pSCNL=menu->pscnl;
  for(iNumElements = 0; pSCNL!=NULL; pSCNL = pSCNL->next)
  {
    memcpy(&(SCNLList[iNumElements]),pSCNL,sizeof(WS_PSCNL_REC));
    iNumElements++;
  }

  /* set the length of the output buffer to the number of elements we 
     found and copied while traversing the list.
  ******************************************************************/
  *pSCNLBufferLen = iNumElements;

  /* set the caller's SCNLBuffer to point at the new SCNLList */
  *pSCNLBuffer = SCNLList;

  /* delete the linked list */
  wsKillPSCNL(menu->pscnl);

  ret = WS_ERR_NONE;

Abort:
  return(ret);
}  /* end wsConvertLinkedListToArray() */

/* wsParseMenuReply is untouched from ws_clientII */
/************************************************************************
 * wsParseMenuReply: parse the reply we got from the waveserver into    *
 * a menu list. Handles replies to MENU, MENUPIN and MENUSCNL requests. *
 ************************************************************************/
static int wsParseMenuReply( WS_MENU menu, char* reply )
{
/* Arguments:
*       menu: pointer to menu structure to be allocated and filled in.
*      reply: pointer to reply to be parsed.
*   Returns: WS_ERR_NONE:  if all went well
*            WS_ERR_INPUT: if bad input parameters
*            WS_ERR_PARSE: if we couldn't parse the reply
*            WS_ERR_MEMORY: if out of memory
  */
  int reqid = 0;
  int pinno = 0;
  char    sta[TRACE2_STA_LEN];         /* Site name */
  char    chan[TRACE2_CHAN_LEN];       /* Component/channel code */
  char    net[TRACE2_NET_LEN];         /* Network name */
  char    loc[TRACE2_LOC_LEN];         /* Location code */
  double tankStarttime = 0.0, tankEndtime = 0.0;
  char datatype[3];
  int scnl_pos = 0;
  
  if ( !reply || !menu )
  {
    if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS) 
      logit("t", "wsParseMenuReply: WS_ERR_INPUT\n");
    return WS_ERR_INPUT;
  }
  
  if ( sscanf( &reply[scnl_pos], "%d", &reqid ) < 1 )
  {
    if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS) 
      logit( "t","wsParseMenuReply(): error parsing reqid\n" );
    return WS_ERR_PARSE;
  }
  wsSkipN( reply, 1, &scnl_pos );
  while ( reply[scnl_pos] && reply[scnl_pos] != '\n' )
  {
    WS_PSCNL pscnl = NULL;
    if ( sscanf( &reply[scnl_pos], "%d %s %s %s %s %lf %lf %s",
      &pinno, sta, chan, net, loc,
      &tankStarttime, &tankEndtime, datatype ) < 7 )
    {
      if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS) 
        logit( "t","wsParseMenuReply(): error decoding reply<%s>\n",
              &reply[scnl_pos] );
      return WS_ERR_PARSE;
    }
    pscnl = ( WS_PSCNL_REC* )calloc(sizeof(WS_PSCNL_REC),1);
    if ( pscnl == NULL )
    {
        logit("t", "wsParseMenuReply(): error allocating memory(%zu)\n",
              sizeof(WS_PSCNL_REC));
      return WS_ERR_MEMORY;
    }
    
    pscnl->next = menu->pscnl;
    pscnl->pinno = (short)pinno;
    strcpy( pscnl->sta, sta );
    strcpy( pscnl->chan, chan );
    strcpy( pscnl->net, net );
    strcpy( pscnl->loc, loc );
    pscnl->tankStarttime = tankStarttime;
    pscnl->tankEndtime = tankEndtime;
    menu->pscnl = pscnl;
    wsSkipN( reply, 8, &scnl_pos );
  }
  
  return WS_ERR_NONE;
}  /* wsParseMenuReply() */



static int Compare_WS_PSCNL_RECs(const void *elem1, const void *elem2) 
{

	/* DK 2005/04/27  Changed the sort order from SCN to SNCL */
	WS_PSCNL pscnl1,pscnl2;
  int ret;

  pscnl1 = (WS_PSCNL) elem1;
  pscnl2 = (WS_PSCNL) elem2;

  ret= strcmp(pscnl1->sta, pscnl2->sta);
  if(ret)
    return(ret);

  /*else*/
  ret= strcmp(pscnl1->net, pscnl2->net);
  if(ret)
    return(ret);

  /*else*/
  ret= strcmp(pscnl1->chan, pscnl2->chan);
  if(ret)
    return(ret);

  /*else*/
  ret= strcmp(pscnl1->loc, pscnl2->loc);
  return(ret);
}  /* end Compare_WS_PSC_RECs() */

/* wsAttachServer is untouched from ws_clientII */
/***********************************************************************
 *  wsAttachServer: Open a connection to a server. The timeout starts  *
 *    when connect() is called by connect_ew() in socket_ew_common.c   *
 ***********************************************************************/
int wsAttachServer( WS_MENU menu, int timeout )
/*
Arguemnts:
menu: pointer to the menu of the server
timeout: time interval in milliseconds; use -1 for no timeout.
returns: WS_ERR_NONE: if all went well.
WS_ERR_INPUT: if menu is missing.
WS_ERR_SOCKET: if a socket error occurred.
WS_ERR_NO_CONNECTION: if a connection could not be established
*/
{
  int                ret = WS_ERR_NONE;
  SOCKET             sock = 0;   /* Socket descriptor                  */
  
  if ( !menu )
  {
    ret = WS_ERR_INPUT;
      logit( "", "wsAttachServer(): WS_ERR_INPUT\n");
    goto Abort;
  }
  if ( menu->sock > 0 )  /* maybe already connected, so disconnect first */
  {
    if(WS_CL_DEBUG & WS_DEBUG_SERVER_INFO) 
      logit("","Existing socket during wsAttachServer(), detaching server\n");
    wsDetachServer( menu, 0);
  }
  
  /* open a non_blocking socket
  *****************************/
  if ( ( sock = setuptcpclient_ew( menu->addr, menu->port, timeout ) ) < 0 )
  {
    if(sock == -2) {
      ret = WS_ERR_SOCKET;
      if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS) logit( "", "wsAttachServer(): socket call failed\n" );
    } else {
      ret = WS_ERR_NO_CONNECTION;
      if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS) logit( "", "wsAttachServer(): connect call failed\n");
    }
    goto Abort;
  }
  
    menu->sock = sock;
    if(WS_CL_DEBUG & WS_DEBUG_SERVER_INFO) 
      logit("e","SUCCESS\n");

    hrtime_ew(&dNow);
    menu->stats.dTotalTimeOverhead += dNow - dAttempting;
    menu->stats.bConnected = TRUE;
    menu->stats.dConnectedSince = dNow;
    menu->stats.iNumTotalConnections++;


    ret = WS_ERR_NONE;
    return ret;
    
    /* An error occured;
    * don't blab about here since we already did earlier. */
Abort:
    if (menu) menu->sock = (SOCKET)-1; /* mark the socket as dead */
    return ret;
}

/* wsDetachServer is untouched from ws_clientII */
/*********************************************************************
 * wsDetachServer: Immediately disconnect from a socket if it's open *
 *********************************************************************/
void wsDetachServer( WS_MENU menu, int iError )
/*  
Arguments:
menu: menu of server to be detached
*/
{

  if ( !menu || menu->sock == -1 )
    return;

  if(WS_CL_DEBUG & WS_DEBUG_SERVER_INFO) 
    logit("t","Disconnecting Server (%s:%s)\n", menu->addr,menu->port);

  if(!menu->stats.bConnected)
  {
    logit("t","wsDetachServer(): Warning: error in stats package, "
      "disconnecting an inactive connection(%s:%s).\n",
      menu->addr,menu->port);
  }
  hrtime_ew(&dAttempting);
 
  
  closesocket_ew( menu->sock, SOCKET_CLOSE_IMMEDIATELY_EW );
  menu->sock = (SOCKET)-1;


  hrtime_ew(&dNow);
  menu->stats.dTotalTimeOverhead += dNow - dAttempting;
  if(menu->stats.bConnected)
    menu->stats.dTotalTimeConnected += dAttempting - menu->stats.dConnectedSince;
  menu->stats.dConnectedSince = 0;
  menu->stats.bConnected = FALSE;
  if(iError)
    menu->stats.iNumErrors++;


}  /* end wsDetachServer() */

/* untouched from ws_clientII */
/**************************************************************************
 *      wsSkipN: moves forward the pointer *posp in buf by moving forward *
 *      cnt words.  Words are delimited by either space or horizontal     *
 *      tabs; newline marks the end of the buffer.                        *
 **************************************************************************/
static void wsSkipN( char* buf, int cnt, int* posp )
{
  int pos = *posp;
  
  while ( cnt )
  {
    while ( buf[pos] != ' ' && buf[pos] != '\t' )
    {
      if ( !buf[pos] )
      {
        goto done;
      }
      if ( buf[pos] == '\n' )
      {
        ++pos;
        goto done;
      }
      ++pos;
    }
    --cnt;
    while ( buf[pos] == ' ' || buf[pos] == '\t' )
    {
      ++pos;
    }
  }
done:
  *posp = pos;
}

/* untouched from ws_clientII */
int setWsClient_ewDebug(int debug)
{
/* setWsClient_ewDebug() turns debugging on or off for 
the ws_clientII routines
  */

  if(debug)
    logit("t", "Setting WS_CL_DEBUG to (0x%x).\n",debug);

  WS_CL_DEBUG=debug;
  return(0);
}


int wsRemoveServer(wsHANDLE pEnv, char * szIPAddr, char * szPort)
{
  int i, rc;
  int iMenuPos;
  WS_MENU pMenu=NULL;

  for(i=0; i< pEnv->iNumMenusInList; i++)
  {
    if(!strcmp(pEnv->MenuList[i]->addr,szIPAddr))
    {
      if(!strcmp(pEnv->MenuList[i]->port,szPort))
      {
        pMenu = pEnv->MenuList[i];
        break;
      }
    }
  }

  iMenuPos = i;

  if(!pMenu)
  {
    return(WS_ERR_MENU_NOT_FOUND);
  }
  /* else */
  /* now we must delete the menu */

  rc = wsRemoveMenuFrompPSCNLBuffer(pEnv, pMenu);
  if(rc == WS_WRN_FLAGGED)
  {
    logit("t","%s: Warning menu [%s:%s] not found in pPSCNLBuffer.  Bug!\n",
          "wsRemoveServer()",pMenu->addr, pMenu->port);
  }

  /* first remove it from the environment's menulist */
  for(i=iMenuPos; i < pEnv->iNumMenusInList - 1; i++)
  {
    pEnv->MenuList[i] = pEnv->MenuList[i+1];
  }
  
  /* free the menu */
  free(pMenu);

  return(WS_ERR_NONE);
}  /* end wsRemoveServer() */


static int wsRemoveMenuFrompPSCNLBuffer(wsHANDLE pEnv, WS_MENU pMenu)
{

  int i,j,ctr;
  int bMenuFound = FALSE;

  /* we now have a WS_MENU that we must delete from the pPSCNLBuffer */
  for(i=0; i < pEnv->iNumPSCNLs; i++)
  {
    for(j=0; j < pEnv->pPSCNLBuffer[i].iNumMenus; j++)
    {
      if(pEnv->pPSCNLBuffer[i].MenuList[j] == pMenu)
      {
        bMenuFound = TRUE;
        for(ctr=j; ctr < pEnv->pPSCNLBuffer[i].iNumMenus - 1; ctr++)
        {
          pEnv->pPSCNLBuffer[i].MenuList[ctr] = 
            pEnv->pPSCNLBuffer[i].MenuList[ctr+1];
        }
        break;
      }
    }  /* end for j in iNumMenus for current PSCNL */
  }    /* end for i in iNumPSCNLs for current environment */

  if(bMenuFound)
    return(WS_ERR_NONE);
  else
    return(WS_WRN_FLAGGED);

}  /* end wsRemoveMenuFrompPSCNLBuffer() */


void wsDestroy(wsHANDLE * ppEnv )
{
  /* we could use wsRemoveServer() to drop each server,
     but much more efficient to just wipe it all out. */

  int i;
  wsHANDLE pEnv = *ppEnv;

  /* go through each menu */
  for(i=0; i < pEnv->iNumMenusInList; i++)
  {
    /* close socket if neccessary */
    if(pEnv->MenuList[i]->sock != -1)
    {
      if(WS_CL_DEBUG & WS_DEBUG_SERVER_INFO) 
        logit("t","wsDestroy(), detaching server\n");
      wsDetachServer(pEnv->MenuList[i],0);
    }

    /* free menu */
    free(pEnv->MenuList[i]);
  }

  /* free PSCNL buffer */
  free(pEnv->pPSCNLBuffer);

  /* free anything else? */

  /* free the environment (kinda sounds like greenpeace) */
  free(pEnv);

  /* null out the handle */
  *ppEnv = NULL;
}  /* end wsDestroy() */

/*********************************************************************
 * wsGetTrace: retrieves the piece of raw trace data specified in    *
 * the structure 'getThis': The current menu list, as built by the   *
 * routines above will be searched for a matching SCNL. If a match   *
 * is found, the associated wave server will be contacted, and a     *
 * request for the trace snippet will be made.                       *
 *********************************************************************/
/* THIS ROUTINE IS UNTESTED!! ora_trace_fetch uses wsGetNextTraceFromRequestList() instead */
int wsGetTrace(TRACE_REQ* getThis, wsHANDLE pEnv, int iTraceType, int timeout_msec)
                  /*
                  Arguments:
                  getThis:   a TRACE_REQ structure (see ws_client.h), with the
                  request portion filled in.
                  wsHANDLE:  a handle for the current environment
                  timeout:   Time in milliseconds to wait for reply
                  return:   WS_ERR_NONE: all went well.
                  WS_WRN_FLAGGED: wave server returned error flag instead
                  of trace data.
                  WS_ERR_EMPTY_MENU: No menu list found.
                  WS_ERR_SCNL_NOT_IN_MENU: SCNL not found in menu list.
                  WS_ERR_NO_CONNECTION: if socket was already closed
                  The socket will be closed for the following:
                  WS_ERR_BUFFER_OVERFLOW: trace buffer supplied too small.
                  WS_ERR_TIMEOUT: if a timeout occured
                  WS_ERR_BROKEN_CONNECTION: if a connection broke.
                  WS_ERR_SOCKET: problem changing socket options.
                  */
{
  int rc;
  int ret = WS_ERR_TIMEOUT;
  int first_error = WS_ERR_NONE;

  WS_MENU menu = NULL;
  double dnow, dTimeoutStart=0.0, dTimeoutSeconds=0.0;
  int  bTimeout;

  if(timeout_msec != -1)
  {
    bTimeout = TRUE;
    dTimeoutStart = hrtime_ew(&dnow);
    dTimeoutSeconds = (double)(timeout_msec)/1000;
    if(hrtime_ew(&dnow) > dTimeoutStart + dTimeoutSeconds)
      return(WS_ERR_TIMEOUT);
  }
  else
  {
    bTimeout = FALSE;
  }

  /* check pointer input params */
  if (!(getThis && pEnv))
  {
    logit("t", "wsGetTrace: NULL input ptr: getThis[%p], pEnv[%p]\n",
          getThis, pEnv);
    return WS_ERR_INPUT;
  }

  /* check iTraceType input param */
  if(!(iTraceType == WS_TRACE_BINARY || iTraceType == WS_TRACE_ASCII))
  {
    logit("t", "wsGetTrace: iTraceType is not valid!  "
          "Must be either WS_TRACE_BINARY or WS_TRACE_ASCII\n");
    return WS_ERR_INPUT;
  }

  while((rc = wsSearchSCNL( getThis, &menu, pEnv)) == WS_ERR_NONE)
  {
    if(bTimeout)
    {
      /* reset the timeout, since we are potentially processing
         multiple servers here (iteratively) and we don't want
         to preclude one server's chances to answer the request
         just because another timed out.
         DK 021702
      **********************************************************/
       dTimeoutStart = hrtime_ew(&dnow);
       if(hrtime_ew(&dnow) > dTimeoutStart + dTimeoutSeconds)
         break;
    }

    if(bTimeout)
    {
      rc = wsGetTraceFromServer(getThis, menu, iTraceType, 
                                (int)((dTimeoutStart + dTimeoutSeconds - hrtime_ew(&dnow)) * 1000));
    }
    else
    {
      rc = wsGetTraceFromServer(getThis, menu, iTraceType, -1/*timeout*/);
    }

    if(rc == WS_ERR_NONE || rc == WS_ERR_BUFFER_OVERFLOW)
    {
      ret = rc;
      break;
    }
    else if(first_error == WS_ERR_NONE)
    {
      first_error = rc;
    }
  }  /* end while wsSearchSCNL */

  /* now the tricky part of getting the error code right */
  /* we have only set ret if we got the best data possible,
     meaning success, or buffer overflow due to the 
     client's buffer wasn't big enough.  So unless we 
     accomplished our mission, ret is still WS_ERR_TIMEOUT.
     Now we have to figure out what ret should be.  First check
     first_error, as this should be the first and most meaningful
     error we received.  If first_error is error free, then next
     check err, in case we ran into problems locating the PSCNL.
     If err has no error information, then we must have timed out
     at the top of the loop.
  ****************************************************************/
  if(ret == WS_ERR_TIMEOUT)
  {
    if(first_error != WS_ERR_NONE)
    {
      ret = first_error;
    }
    else if(rc != WS_ERR_NONE)
    {
      ret = rc;
    }
    /* else it was a genuine timeout, so change nothing */
  }

  return(ret);
} /* end wsGetTrace() */


int wsGetTraceFromServer(TRACE_REQ* getThis, WS_MENU menu, int iTraceType, int timeout_msec)
                  /*
                  Arguments:
                  getThis:   a TRACE_REQ structure (see ws_client.h), with the
                  request portion filled in.
                  wsHANDLE:  a handle for the current environment
                  timeout:   Time in milliseconds to wait for reply
                  return:   WS_ERR_NONE: all went well.
                  WS_WRN_FLAGGED: wave server returned error flag instead
                  of trace data.
                  WS_ERR_EMPTY_MENU: No menu list found.
                  WS_ERR_SCNL_NOT_IN_MENU: SCNL not found in menu list.
                  WS_ERR_NO_CONNECTION: if socket was already closed
                  The socket will be closed for the following:
                  WS_ERR_BUFFER_OVERFLOW: trace buffer supplied too small.
                  WS_ERR_TIMEOUT: if a timeout occured
                  WS_ERR_BROKEN_CONNECTION: if a connection broke.
                  WS_ERR_SOCKET: problem changing socket options.
                  */

  {

  int len,rc;
  int err = WS_ERR_NONE;

  double dnow, dTimeoutStart=0.0, dTimeoutSeconds=0.0;
  char request[wsREQLEN];
  
  int  bTimeout;

  if(timeout_msec != -1)
  {
    bTimeout = TRUE;
    dTimeoutStart = hrtime_ew(&dnow);
    dTimeoutSeconds = (double)(timeout_msec)/1000;
    if(hrtime_ew(&dnow) > dTimeoutStart + dTimeoutSeconds)
      return(WS_ERR_TIMEOUT);
  }
  else
  {
    bTimeout = FALSE;
  }

  /* check pointer input params */
  if (!(getThis && menu))
  {
    logit("t", "wsGetTrace: NULL input ptr: getThis[%p], menu[%p]\n",
          getThis, menu);
    return WS_ERR_INPUT;
  }

  /* check iTraceType input param */
  if(!(iTraceType == WS_TRACE_BINARY || iTraceType == WS_TRACE_ASCII))
  {
    logit("t", "wsGetTrace: iTraceType is not valid!  "
          "Must be either WS_TRACE_BINARY or WS_TRACE_ASCII\n");
    return WS_ERR_INPUT;
  }

  /* check for live socket.  Open if not already connected. */
  if (menu->sock < 0) 
  {
    if(bTimeout)
    {
      rc=wsAttachServer(menu,(int)((dTimeoutStart + dTimeoutSeconds - hrtime_ew(&dnow)) * 1000));
    }
    else
    {
      rc=wsAttachServer(menu, -1/*timeout*/);
    }
    if(rc!= WS_ERR_NONE)
    {
      err=rc;
      goto HandleErrors;
    }
  }
  
  /* create the request string based on the iTraceType */
  if(iTraceType == WS_TRACE_BINARY)
  {
    sprintf( request, "GETSCNLRAW: %d %s %s %s %s %lf %lf\n",
            menu_reqid++, getThis->sta, getThis->chan, getThis->net, getThis->loc,
            getThis->reqStarttime, getThis->reqEndtime );
  }
  else if(iTraceType == WS_TRACE_ASCII)
  {
    sprintf( request, "GETSCNL: %d %s %s %s %s %lf %lf %ld\n",
            menu_reqid++, getThis->sta, getThis->chan, getThis->net, getThis->loc,
            getThis->reqStarttime, getThis->reqEndtime, getThis->fill );
  }
  else
  {
    logit("t", "wsGetTrace: iTraceType is not valid! (check2) "
          "Must be either WS_TRACE_BINARY or WS_TRACE_ASCII\n");
    return WS_ERR_INPUT;
  }
  
  /* Calculate the length of our request */
  len = (int)strlen(request);

  /* Send the trace request */
  if(bTimeout)
  {
    rc = send_ew(menu->sock, request, len, 0, 
                 (int)((dTimeoutStart + dTimeoutSeconds - hrtime_ew(&dnow)) * 1000));
  }
  else
  {
    rc = send_ew(menu->sock, request, len, 0, -1 /* tiemout */);
  }

  if(rc != len)
  {
    if (rc < 0 )
      err = WS_ERR_BROKEN_CONNECTION;
    else 
      err = WS_ERR_TIMEOUT;

    if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS) 
      logit("t","Error(%d) received while sending data to wave_server!"
                "%d bytes sent.\n",err,rc);
    
    goto HandleErrors;
  }
  
  /* Retrieve the reply to the request.  This takes different code 
  depending on the iTraceType */
  if(iTraceType == WS_TRACE_BINARY)
  {

    /* get the header for a Binary trace reply */
    if(bTimeout)
    {
      rc = wsWaitForServerResponse(menu, getThis->pBuf, getThis->bufLen, 
                                   (int)((dTimeoutStart + dTimeoutSeconds - hrtime_ew(&dnow)) * 1000),
                                   WS_TRACE_BINARY);
    }
    else
    {
      rc = wsWaitForServerResponse(menu, getThis->pBuf, getThis->bufLen, 
                                   -1 /*timeout*/, WS_TRACE_BINARY);
    }

    switch (rc) 
    {
    case WS_ERR_INPUT:
      logit("t","wsGetTrace() Error!  wsWaitBinHeader() returned InputError\n");
      err = rc;
      goto HandleErrors;
      break;
    case WS_ERR_NONE:
      break;
    default:
      /* buffer overflow, timeout, socket error, or broken connection */
      if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS) 
        logit("t","wsGetTrace() Error!  wsWaitBinHeader() returned %d\n",rc);
      err = rc;
      goto HandleErrors;
    }  /* end switch(rc=wsWaitForServerResponse) */
    
    /* parse the header for a Binary trace reply */
    rc = wsParseBinHeaderReply( getThis, getThis->pBuf );
    switch (rc) 
    {
    case WS_ERR_NONE:
      break;
    case WS_WRN_FLAGGED:
      getThis->actLen = 0;
      err = rc;
      if(WS_CL_DEBUG & WS_DEBUG_DATA_WARNINGS) 
        logit("","Server(%s:%s) sent flag(F%c) for req (%s %s %s %s) (%.3f-%.3f)\n",
              menu->addr, menu->port, getThis->retFlag, 
              getThis->sta, getThis->chan, getThis->net, getThis->loc,
              getThis->reqStarttime, getThis->reqEndtime);
      goto HandleErrors;
      break;
    default:  /* unexpected error */
      if(WS_CL_DEBUG & WS_DEBUG_DATA_ERRORS) 
        logit("t","wsGetTrace() Error!  wsParseBinHeaderReply() returned %d\n"
                  " for req (%s %s %s %s) (%.3f-%.3f) - server (%s:%s)\n",
              rc, getThis->sta, getThis->chan, getThis->net, getThis->loc,
              getThis->reqStarttime, getThis->reqEndtime, 
              menu->addr, menu->port);
      err = rc;
      goto HandleErrors;
      break;
    }  /* end switch(rc=wsParseBinHeaderReply) */
    
    /* get the actual trace data */
    if ( getThis->actLen )  /* why shouldn't it be ???? DK 01/12/01 */
    {
      long int binLen = getThis->actLen;
      
      if ( (int)getThis->bufLen < binLen )
        binLen = (int)getThis->bufLen;

      if(bTimeout)
      {
        rc = recv_all(menu->sock, getThis->pBuf, binLen, 0, 
          (int)((dTimeoutStart + dTimeoutSeconds - hrtime_ew(&dnow)) * 1000));
      }
      else
      {
        rc = recv_all(menu->sock, getThis->pBuf, binLen, 0, -1/*timeout*/);
      }
      
      if ( rc != binLen ) 
      {
        if ( rc < 0 )
        {
          err = WS_ERR_BROKEN_CONNECTION;
        } 
        else 
        {
          err = WS_ERR_TIMEOUT;
        }
        goto HandleErrors;
      }
      
      if ( binLen < getThis->actLen ) 
      {
        if(WS_CL_DEBUG & WS_DEBUG_DATA_ERRORS) 
          logit("t","wsGetTraceFromServer() WARNING!  snippet size(%ld) "
                    "exceeded buffer len(%ld), trace truncated\n"
                    " for req (%s %s %s %s) (%.3f-%.3f) - server (%s:%s)\n",
                getThis->actLen, binLen,
                getThis->sta, getThis->chan, getThis->net, getThis->loc,
                getThis->reqStarttime, getThis->reqEndtime, 
                menu->addr, menu->port);

        getThis->actLen = binLen;
        err = WS_ERR_BUFFER_OVERFLOW;
      }
      else
      {
        err = WS_ERR_NONE;
      }
    }  /* end if actLen > 0 */
  }  /* end if WS_TRACE_BINARY */
  else if(iTraceType == WS_TRACE_ASCII)
  {
    /* get the Ascii reply */
    if(bTimeout)
    {
      rc = wsWaitForServerResponse(menu, getThis->pBuf, getThis->bufLen, 
                                   (int)((dTimeoutStart + dTimeoutSeconds - hrtime_ew(&dnow)) * 1000),
                                   WS_TRACE_ASCII );
    }
    else
    {
      rc = wsWaitForServerResponse(menu, getThis->pBuf, getThis->bufLen, 
        -1, WS_TRACE_ASCII );
    }
    if ( rc != WS_ERR_NONE && rc != WS_ERR_BUFFER_OVERFLOW )
    {
      err = rc;
      goto HandleErrors;
    }
    else
    {
      /* copy the return code into our return code */
      err = rc;
      
      /* parse the header and the trace itself */
      rc = wsParseAsciiHeaderReply( getThis, getThis->pBuf );

      switch (rc) 
      {
      case WS_ERR_NONE:
        break;
      case WS_WRN_FLAGGED:
        getThis->actLen = 0;
        err = rc;
        /* copy the return code from wsParseAsciiHeaderReply() if
           it contains an error, and wsWaitForServerResponse()
           completed successfully.
         *********************************************************/
        if((err == WS_ERR_NONE) && (rc != WS_ERR_NONE))
          err = rc;

        if(WS_CL_DEBUG & WS_DEBUG_DATA_WARNINGS) 
          logit("","Server(%s:%s) sent flag(F%c) for req (%s %s %s %s) (%.3f-%.3f)\n",
          menu->addr, menu->port, getThis->retFlag, 
          getThis->sta, getThis->chan, getThis->net, getThis->loc,
          getThis->reqStarttime, getThis->reqEndtime);
        goto HandleErrors;
        break;
      default:  /* unexpected error */
        if(WS_CL_DEBUG & WS_DEBUG_DATA_ERRORS) 
          logit("t","wsGetTrace() Error!  wsParseAsciiHeaderReply() returned %d\n"
          " for req (%s %s %s %s) (%.3f-%.3f) - server (%s:%s)\n",
          rc, getThis->sta, getThis->chan, getThis->net, getThis->loc,
          getThis->reqStarttime, getThis->reqEndtime, 
          menu->addr, menu->port);
        err = rc;
        goto HandleErrors;
        break;
      }  /* end switch(rc=wsParseBinHeaderReply) */

      
      /* wsParseAsciiReply puts the trace data into getThis, 
      so now we're done */
    }
  }  /* end if WS_TRACE_ASCII */
  else
  {
    logit("t", "wsGetTrace: iTraceType is not valid! (check3) "
          "Must be either WS_TRACE_BINARY or WS_TRACE_ASCII\n");
    return WS_ERR_INPUT;
  }

  
HandleErrors:

  if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS) 
  {
    if  ( err == WS_ERR_TIMEOUT ) {
      logit( "e","wsGetTraceBin(): server %s:%s timed out\n", menu->addr,
            menu->port );
    } else if ( err == WS_ERR_BROKEN_CONNECTION ) {
      logit( "e","wsGetTraceBin(): broken connection to server %s:%s\n",
            menu->addr, menu->port);
    }
  }

  if(err != WS_ERR_NONE && err != WS_WRN_FLAGGED)
  {
    if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS) 
      logit("t","\nAbortServer due to error(%d)in wsGetTraceFromServer()\n "
            "\t\t for SCNL (%s,%s,%s %s) (%.3f-%.3f).  \n\t\t Detaching server.\n",err,
            getThis->sta, getThis->chan, getThis->net, getThis->loc, 
            getThis->reqStarttime, getThis->reqEndtime);

    if(err == WS_ERR_BUFFER_OVERFLOW)
      wsDetachServer( menu, 0); 
    else
      wsDetachServer( menu, err); 
  }

  return(err);
}  /* end wsGetTraceFromServer() */

/*******************************************************************
 * wsWaitForServerResponse(): Retrieve a message.  The message     *
 * will be terminated by a newline.  If the message is of type     *
 * ASCII, then  nothing else is expected after the newline, so we  *
 * can read several characters at a time without fear of reading   *
 * past the newline.  If it is of type BINARY then there is binary *
 * data after the newline, so we need to read 1 character at a     *
 * time, so that we don't accidentally grab some binary data.      *
 *                                                                 *
 * This message may have internal nulls which will be converted to *
 * spaces.                                                         *
 * Returns after newline is read, when timeout expires if set,     *
 * or on error.                                                    *
 *******************************************************************/
static int wsWaitForServerResponse(WS_MENU menu, char* buf, int buflen, 
                                   int timeout_msec, int iTraceType)
/*
Arguments:
menu: menu of server from which message is received
buf: buffer in which to place the message, terminated by null.
buflen: number of bytes in the buffer.
timeout_msec: timout interval in milliseconds. 
return: WS_ERR_NONE: all went well.
WS_ERR_BUFFER_OVERFLOW: ran out of space before the message
end; calling program will have to decide how serious this is.
WS_ERR_INPUT: missing input parameters.
WS_ERR_SOCKET: error setting socket options.
WS_ERR_TIMEOUT: time expired before we read everything.
WS_ERR_BROKEN_CONNECTION: if the connection failed.
*/
{
  int ii, ir = 0;  /* character counters */
  int nr = 0;
  int done = FALSE;
  int len = 0;
  int ret;
  double dnow, dTimeoutStart=0.0, dTimeoutSeconds=0.0;
  int  bTimeout;

  if(timeout_msec != -1)
  {
    bTimeout = TRUE;
    dTimeoutStart = hrtime_ew(&dnow);
    dTimeoutSeconds = ((double)timeout_msec)/1000;
    if(hrtime_ew(&dnow) > dTimeoutStart + dTimeoutSeconds)
      return(WS_ERR_TIMEOUT);
  }
  else
  {
    bTimeout = FALSE;
  }

  if ( !buf || buflen <= 0 )
  {
    logit( "e", "wsWaitForServerResponse(): no input buffer\n");
    return WS_ERR_INPUT;
  }

  /* check iTraceType input param */
  if(!(iTraceType == WS_TRACE_BINARY || iTraceType == WS_TRACE_ASCII))
  {
    logit("t", "wsWaitForServerResponse: iTraceType is not valid!  "
          "Must be either WS_TRACE_BINARY or WS_TRACE_ASCII\n");
    return WS_ERR_INPUT;
  }
  
  while (!done)
  {
    if ((bTimeout) && hrtime_ew(&dnow) > dTimeoutStart + dTimeoutSeconds ) 
    {
      ret = WS_ERR_TIMEOUT;

      if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS) 
        logit("t", "wsWaitForServerResponse timed out\n");
      goto Done;
    }
    if ( ir >= buflen - 2 )
    {
      if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS) 
        logit( "e", "wsWaitForServerResponse(): reply buffer overflows\n" );
      ret = WS_ERR_BUFFER_OVERFLOW;
      goto Done;
    }
#if 0
    /* old code 02/05/2002 */
    len = WS_MAX_RECV_BUF_LEN;
    if ( ir + len >= buflen - 1 )
    {
      if(iTraceType == WS_TRACE_BINARY)
        len = 1;
      else if (iTraceType == WS_TRACE_ASCII)
        len = buflen - ir - 2; /* leave room for the terminating null */
      else
      {
        ret = WS_ERR_UNKNOWN;
        goto Done;
      }
    }
#endif /* 0 */
    /* new code 02/05/2002 DK */
    if(iTraceType == WS_TRACE_BINARY)
      len = 1;
    else if (iTraceType == WS_TRACE_ASCII)
      len = buflen - ir - 2; /* leave room for the terminating null */
    else
    {
      ret = WS_ERR_UNKNOWN;
      goto Done;
    }
    /* end new code 02/05/2002 */

    if(bTimeout)
      nr = recv_ew( menu->sock, &buf[ir], len, 0,
                    (int)((dTimeoutStart + dTimeoutSeconds - hrtime_ew(&dnow)) * 1000));
    else
      nr = recv_ew( menu->sock, &buf[ir], len, 0, -1);

    if(nr == SOCKET_ERROR)  
    {
      if(socketGetError_ew() == WOULDBLOCK_EW)
      {
        ret = WS_ERR_TIMEOUT;
      }
      else
      {
        if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS) 
          logit("e", "wsWaitForServerResponse(): Error[%d] on socket recv()\n",
                socketGetError_ew());
        ret = WS_ERR_BROKEN_CONNECTION;
      }
      goto Done;
    }
    if ( nr > 0 )
    {
      ii = 0;
      /* got something, adjust ir and c  */
      ir += nr;
      
      /* replace NULL char in ascii string with SPACE char */
      for (ii = ir-nr; ii < ir; ii++)
      {
        if ( !buf[ii] ) buf[ii] = ' ';
        else if(buf[ii] == '\n')
        {
          done = TRUE;
           /* we are having problems with data not catching the \n because we
              are retrieving blocks of data at a time, and are only checking
              at the top of the loop.  So we have changed to check here, where
              we are examining every character.
           */
          continue;
        }
      }
    }  /* end if nr > 0 */
  }    /* end while (!done) */
  
  ret = WS_ERR_NONE;

Done:

  buf[ir] = '\0';                 /* null-terminate the reply      */

  return ret;
}

/* untouched from ws_clientII.c */
/***********************************************************************
 * wsParseBinHeaderReply: parse the reply we got from the waveserver   *
 * into a TRACE_REQ structure. Handles the header for replies reply to *
 * GETSCNLRAW requests.                                                *
 ***********************************************************************/
static int wsParseBinHeaderReply( TRACE_REQ* getThis, char* reply )
{
/* Arguments:
*    getThis: pointer to TRACE_REQ structure to be filled with reply info
*      reply: pointer to reply to be parsed.
*   Returns: WS_ERR_NONE:  if all went well
*            WS_ERR_INPUT: if bad input parameters
*            WS_ERR_PARSE: if we couldn't parse part of the reply
*            WS_WRN_FLAGGED: server sent us a no-data flag
  */
  int reqid = 0;
  int pinno = 0;
  char    sta[TRACE2_STA_LEN];         /* Site name */
  char    chan[TRACE2_CHAN_LEN];       /* Component/channel code */
  char    net[TRACE2_NET_LEN];         /* Network name */
  char    loc[TRACE2_LOC_LEN];         /* Location code */
  char flag[9];
  char datatype[3];
  double tankStarttime = 0.0, tankEndtime = 0.0;
  int bin_len = 0;
  int scnl_pos = 0;
  
  if ( !reply || !getThis )
  {
    logit( "", "wsParseBinHeaderReply(): bad input parameters\n");
    return WS_ERR_INPUT;
  }
  
  if ( sscanf( &reply[scnl_pos], "%d %d", &reqid, &pinno ) < 2 )
  {
    if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS) 
      logit( "","wsParseBinHeaderReply(): error parsing reqid/pinno\n" );
    return WS_ERR_PARSE;
  }
  wsSkipN( reply, 2, &scnl_pos );
  
  if ( sscanf( &reply[scnl_pos], "%s %s %s %s", sta, chan, net, loc ) < 3 )
  {
    if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS) 
      logit( "","wsParseBinHeaderReply(): error parsing SCNL\n" );
    return WS_ERR_PARSE;
  }
  wsSkipN( reply, 4, &scnl_pos );
  
  if ( sscanf( &reply[scnl_pos], "%s %s", flag, datatype ) < 2 )
  {
    if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS) 
      logit( "","wsParseBinHeaderReply(): error parsing flag/datatype\n" );
    return WS_ERR_PARSE;
  }
  wsSkipN( reply, 2, &scnl_pos );
  
  if ( strlen(flag) == 1 )
  {
    if ( sscanf( &reply[scnl_pos], "%lf %lf", &tankStarttime, 
      &tankEndtime ) < 2 )
    {
      if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS) 
        logit( "","wsParseBinHeaderReply(): error parsing starttime/endtime\n" );
      return WS_ERR_PARSE;
    }
    wsSkipN( reply, 2, &scnl_pos );
    
    if ( sscanf( &reply[scnl_pos], "%d", &bin_len ) < 1 )
    {
      if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS) 
        logit( "","wsParseBinHeaderReply(): error parsing bin_len\n" );
      return WS_ERR_PARSE;
    }
    wsSkipN( reply, 1, &scnl_pos );
    
  }
  else if ( strlen(flag) == 2 )
  {
    tankStarttime = 0.0;
    tankEndtime = 0.0;
    bin_len = 0;
    if ( strcmp(flag,"FL") == 0 )
    {
      if ( sscanf( &reply[scnl_pos], "%lf", &tankStarttime) < 1 )
      {
        if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS) 
          logit( "e","wsParseBinHeaderReply(): error parsing tank starttime\n" );
        return WS_ERR_PARSE;
      }
      wsSkipN( reply, 1, &scnl_pos );
    }
    else if ( strcmp(flag,"FR") == 0 )
    {
      if ( sscanf( &reply[scnl_pos], "%lf", &tankEndtime) < 1 )
      {
        if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS) 
          logit( "e","wsParseBinHeaderReply(): error parsing tank endtime\n" );
        return WS_ERR_PARSE;
      }
      wsSkipN( reply, 1, &scnl_pos );
    }
  }
  else
  {
    if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS) 
      logit( "","wsParseBinHeaderReply(): bad flag[%s]\n", flag );
    return WS_ERR_PARSE;
  }
  
  getThis->pinno = (short)pinno;
  getThis->actStarttime = tankStarttime;
  getThis->actEndtime = tankEndtime;
  getThis->samprate = (double) 0.0; /* server doesn't send this */
  getThis->actLen = bin_len;
  if ( strlen( flag ) >= 2 ) {
    getThis->retFlag = flag[1];
    return WS_WRN_FLAGGED;
  } else {
    getThis->retFlag = '\0';
    return WS_ERR_NONE;
  }
}  /* end wsParseBinHeaderReply() */

/* untouched from ws_clientII.c */
/***********************************************************************
 * wsParseAsciiHeaderReply: parse the reply we got from the waveserver *
 * into a TRACE_REQ structure. Handles the header for replies reply to *
 * GETSCNL and GETPIN requests.                                        *
 ***********************************************************************/
static int wsParseAsciiHeaderReply( TRACE_REQ* getThis, char* reply )
{
/* Arguments:
*    getThis: pointer to TRACE_REQ structure to be filled with reply info
*      reply: pointer to reply to be parsed.
*   Returns: WS_ERR_NONE:  if all went well
*            WS_ERR_INPUT: if bad input parameters
*            WS_ERR_PARSE: if we couldn't parse part of the reply
*            WS_WRN_FLAGGED: server sent us a no-data flag
  */
  int reqid = 0;
  int pinno = 0;
  char    sta[TRACE2_STA_LEN];         /* Site name */
  char    chan[TRACE2_CHAN_LEN];       /* Component/channel code */
  char    net[TRACE2_NET_LEN];         /* Network name */
  char    loc[TRACE2_LOC_LEN];         /* Location code */
  char flag[9];
  char datatype[3];
  double tankStarttime = 0.0, samprate = 0.0;
  double tankEndtime =0.0;
  int scnl_pos = 0;
  
  if ( !reply )
  {
    logit( "", "wsParseAsciiHeaderReply(): bad input parameters\n");
    return WS_ERR_INPUT;
  }
  
  if ( sscanf( &reply[scnl_pos], "%d %d", &reqid, &pinno ) < 2 )
  {
    if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS) 
      logit( "e","wsParseAsciiHeaderReply(): error parsing reqid/pinno\n" );
    return WS_ERR_PARSE;
  }
  wsSkipN( reply, 2, &scnl_pos );
  
  if ( sscanf( &reply[scnl_pos], "%s %s %s %s", sta, chan, net, loc ) < 3 )
  {
    if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS) 
      logit( "e","wsParseAsciiHeaderReply(): error parsing SCNL\n" );
    return WS_ERR_PARSE;
  }
  wsSkipN( reply, 4, &scnl_pos );
  
  if ( sscanf( &reply[scnl_pos], "%s %s", flag, datatype ) < 2 )
  {
    if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS) 
      logit( "e","wsParseAsciiHeaderReply(): error parsing flag/datatype\n" );
    return WS_ERR_PARSE;
  }
  wsSkipN( reply, 2, &scnl_pos );
  
  if ( strlen(flag) == 1 || strcmp(flag,"FG") == 0 )
  {
    if ( sscanf( &reply[scnl_pos], "%lf %lf", &tankStarttime, &samprate ) < 2 )
    {
      if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS) 
        logit( "e","wsParseAsciiHeaderReply(): error parsing startT/samprate\n" );
      return WS_ERR_PARSE;
    }
    wsSkipN( reply, 2, &scnl_pos );
  }
  else if ( strlen(flag) == 2 )
  {
    if ( strcmp(flag,"FL") == 0 )
    {
      if ( sscanf( &reply[scnl_pos], "%lf", &tankStarttime) < 1 )
      {
        if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS) 
          logit( "e","wsParseAsciiHeaderReply(): error parsing startTime\n" );
        return WS_ERR_PARSE;
      }
      wsSkipN( reply, 1, &scnl_pos );
    }
    else if ( strcmp(flag,"FR") == 0 )
    {
      /* DK 2001/10/01  if wave_serverV sends an "FR" flag, then it
         sends the tankEndtime as the only double after the flags,
         not the sample rate.
      if ( sscanf( &reply[scnl_pos], "%lf", &samprate) < 1 )
      **************************************************************/
      if ( sscanf( &reply[scnl_pos], "%lf", &tankEndtime) < 1 )
      {
        if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS) 
          logit( "e","wsParseAsciiHeaderReply(): error parsing samprate\n" );
        return WS_ERR_PARSE;
      }
      wsSkipN( reply, 1, &scnl_pos );
    }
  }
  
  getThis->pinno = (short)pinno;
  getThis->actStarttime = tankStarttime;
  getThis->actEndtime = tankEndtime;
  getThis->samprate = samprate;
  getThis->actLen = (long)(strlen( reply ) - scnl_pos);
  memmove(reply, &reply[scnl_pos], getThis->actLen);
  reply[getThis->actLen] = 0;
  
  if ( strlen( flag ) >= 2 ) {
    getThis->retFlag = flag[1];
    return WS_WRN_FLAGGED;
  } else {
    getThis->retFlag = '\0';
    return WS_ERR_NONE;
  }
}  /* end wsParseAsciiHeaderReply() */

/*********************************************
 *  wsKillPSCNL: Deallocates the PSCNL list  *
 *********************************************/
void wsKillPSCNL( WS_PSCNL pscnl )
/*
Arguments:
pscnl: pointer to a list of scnl structures
*/
{
  WS_PSCNL next;

  while ( pscnl )
  {
    next = pscnl->next;
    
    free( pscnl );
    pscnl = next;
  }
  return;
}

/* finds a PSCNL.  returns the menu that contained the PSCNL.
   if *menup is NULL then it starts at the beginning of the
   list of menus.  If *menup is NOT NULL, then it starts searching
   at the menu immediately after *menup in the list.
*******************************************************************/
int wsSearchSCNL( TRACE_REQ* getThis, WS_MENU* menup, wsHANDLE pEnv)
{
  int i;
  WS_PSCNL_REC PSCNL, *pPSCNL;
  WS_MENU menu_start;

  if(!(getThis && pEnv))
  {
    logit("t","wsSearchSCNL()  Error NULL pointer input params!\n");
    return(WS_ERR_INPUT);
  }
          
  menu_start = *menup;

  
  memset(&PSCNL, 0, sizeof(WS_PSCNL_REC));
  strncpy(PSCNL.sta,  getThis->sta,  sizeof(PSCNL.sta) - 1);
  strncpy(PSCNL.chan, getThis->chan, sizeof(PSCNL.chan) - 1);
  strncpy(PSCNL.net,  getThis->net,  sizeof(PSCNL.net) - 1);
  strncpy(PSCNL.loc,  getThis->loc,  sizeof(PSCNL.loc) - 1);


  /* find the PSCNL */
  pPSCNL = bsearch(&PSCNL, pEnv->pPSCNLBuffer, pEnv->iNumPSCNLs, 
                  sizeof(WS_PSCNL_REC), Compare_WS_PSCNL_RECs);

  if(!pPSCNL)
  {
    if(WS_CL_DEBUG & WS_DEBUG_OVERKILL) 
      logit("t","wsSearchSCNL():  Error [%s,%s,%s,%s] not found.\n",
                          PSCNL.sta,PSCNL.chan,PSCNL.net,PSCNL.loc);
    return(WS_ERR_SCNL_NOT_IN_MENU);
  }
  else
  {
    if(WS_CL_DEBUG & WS_DEBUG_DATA_INFO) 
      logit("","SCNL (%s,%s,%s,%s) found in menulist!\n", 
            PSCNL.sta, PSCNL.chan, PSCNL.net, PSCNL.loc);
  }

  if(menu_start)
  {
    /* find the NEXT available menu, not the current one!!!! DK 2003/01/27 */
    for(i=0;i < pPSCNL->iNumMenus && pPSCNL->MenuList[i]->menunum <= menu_start->menunum; i++);

    if(i == pPSCNL->iNumMenus)
    {
      if(WS_CL_DEBUG & WS_DEBUG_DATA_WARNINGS) 
        logit("t","wsSearchSCNL(): WARNING additional menu for (%s,%s,%s,%s) "
                  "not found, all %d menus already processed.\n", 
            pPSCNL->sta, pPSCNL->chan, pPSCNL->net, pPSCNL->loc, pPSCNL->iNumMenus);
      return(WS_ERR_MENU_NOT_FOUND);
    }
    /* move to the next menu */
    /*  we are already at the desired menu, no need to increment   
        i++;
     ************************************************************/
  }
  else
  {
    i=0;  /* start at beginning of list */
  }

  if(i >= pPSCNL->iNumMenus)
  {
    *menup = NULL;
    return(WS_WRN_FLAGGED);
  }
  else
  {
    *menup = pPSCNL->MenuList[i];
    if(WS_CL_DEBUG & WS_DEBUG_DATA_INFO) 
      logit("","SCNL (%s,%s,%s,%s) found in menulist.  Using server %d-%d (%s:%s)!\n", 
            PSCNL.sta, PSCNL.chan, PSCNL.net,PSCNL.loc,  
            i, (*menup)->menunum, (*menup)->addr, (*menup)->port);
    return(WS_ERR_NONE);
  }
}  /* end wsSearchSCNL() */

      
WS_MENU wsSearchMenu(wsHANDLE pEnv, char * addr, char * port)
{
  int i;

  for(i=0; i < pEnv->iNumMenusInList; i++)
  {
    if(strcmp(pEnv->MenuList[i]->addr,addr))
      if(strcmp(pEnv->MenuList[i]->port,port))
        return(pEnv->MenuList[i]);

  }
  return(NULL);
}  /* end wsSearchMenu() */

/* Return the pscnl list for this server *
 *****************************************************************************/
int wsGetServerPSCNL( char* addr, char* port, WS_PSCNL* pscnlp, wsHANDLE pEnv )
{
  int i, j;
  WS_MENU menu;
  WS_PSCNL pFirstPSCNL, pLastPSCNL, pNewPSCNL;

  if((pFirstPSCNL = malloc(sizeof(WS_PSCNL_REC))) == NULL)
  {
    logit("t","wsGetServerPSCNL(): Error! Failed to allocate %zu bytes.\n",
      sizeof(WS_PSCNL_REC));
    return(WS_ERR_MEMORY);
  }

  memset(pFirstPSCNL, 0, sizeof(WS_PSCNL_REC));
  pLastPSCNL = pFirstPSCNL;

  menu = wsSearchMenu(pEnv, addr, port);

  if(!menu)
  {
    if(WS_CL_DEBUG & WS_DEBUG_DATA_ERRORS) 
      logit("t","wsGetServerPSCNL(): error [%s:%s] not in menulist.\n", addr,port);
    return(WS_ERR_MENU_NOT_FOUND);
  }
  
  for(i=0; i < pEnv->iNumPSCNLs; i++)
  {
    for(j=0; j < pEnv->pPSCNLBuffer[i].iNumMenus; j++)
    {
      if(pEnv->pPSCNLBuffer[i].MenuList[j] == menu)
      {
        if((pNewPSCNL = malloc(sizeof(WS_PSCNL_REC))) == NULL)
        {
          logit("t","wsGetServerPSCNL(): Error! Failed to allocate %zu bytes.\n",
                sizeof(WS_PSCNL_REC));
          return(WS_ERR_MEMORY);
        }
        memcpy(pNewPSCNL, &(pEnv->pPSCNLBuffer[i]), sizeof(WS_PSCNL_REC));
        pLastPSCNL->next = pNewPSCNL;
        pLastPSCNL = pNewPSCNL;
        break;
      }
    }
  }

  /* set the caller's pointer, remember the first PSCNL is blank
     so we want to bypass it. */
  *pscnlp = pFirstPSCNL->next;
  free(pFirstPSCNL);

  if(*pscnlp == NULL)
  {
    if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS) 
      logit("t","wsGetServerPSCNL(): [%s:%s]  Empty Menu!\n", addr,port);
    return(WS_ERR_EMPTY_MENU);
  }
  else
  {
    return(WS_ERR_NONE);
  }
}  /* end wsGetServerPSCNL() */


static int wsCompareMenuNumbersForTraceReqs(const void * pElem1, const void * pElem2)
{

  WS_MENU menu1, menu2;

  menu1 = ((TRACE_REQ *)pElem1)->menu;
  menu2 = ((TRACE_REQ *)pElem2)->menu;

  /* put the NULLs at the front of the list.  less extra baggage
     to carry around.  */
  if(!menu1)
  {
    if(!menu2)
      return(0);
    else
      return(-1);
  }
  if(!menu2)
    return(1);
  else
    return(wsCompareMenuNumbers(menu1,menu2));
}  /* end wsCompareMenuNumbersforTraceReqs() */


static int wsCompareMenuNumbers(const void * pElem1, const void * pElem2)
{

  if(((WS_MENU) pElem1)->menunum == ((WS_MENU) pElem2)->menunum)
    return(0);
  else if(((WS_MENU) pElem1)->menunum < ((WS_MENU) pElem2)->menunum) 
    return(-1);
  else
    return(1);
}  /* end wsCompareMenuNumbers()*/

/***************************************/
int wsPrepRequestList(TRACE_REQ* RequestList, int iNumReqsInList, 
                      int iTraceType, int timeout_sec, wsHANDLE pEnv)
{
  int i;
  int rc;


  /* check input params */
  if(!(RequestList && iNumReqsInList && pEnv))
  {
    logit("t", "wsPrepRequestList: NULL input params!\n");
    return WS_ERR_INPUT;
  }
  /* check iTraceType input param */
  if(!(iTraceType == WS_TRACE_BINARY || iTraceType == WS_TRACE_ASCII))
  {
    logit("t", "wsPrepRequestList: iTraceType is not valid!  "
          "Must be either WS_TRACE_BINARY or WS_TRACE_ASCII\n");
    return WS_ERR_INPUT;
  }

  /* set the iCurrentTraceReq to the beginning of the list */
  pEnv->iCurrentTraceReq = 0;
  /* set iNumTraceReqsInList to the number of reqs in the new list. */
  pEnv->iNumTraceReqsInList = iNumReqsInList;

  /* set the iTraceType */
  pEnv->iTraceTypeForList = iTraceType;

  /* set the CurrentMenu */
  pEnv->CurrentMenu = NULL;

  /* set the timeout period for each call */
  if(timeout_sec <= 0)
    pEnv->iTimeoutMsec = -1;
  else
    pEnv->iTimeoutMsec = 1000 * timeout_sec;

  /* for each server in the menu:
     make sure status is initialized to healthy,
     make sure socket is closed, so we can operate
     with one connection at a time.  */

  for(i=0; i < pEnv->iNumMenusInList; i++)
  {
    /* initialize server status */
    pEnv->MenuList[i]->serverstatus = WS_SERVER_STATUS_HEALTHY;

    /* close the socket if open */
    wsDetachServer(pEnv->MenuList[i],0);
  }

  /* run wsSearchSCNL for each trace req */
  for(i=0; i < iNumReqsInList; i++)
  {
    rc = wsSearchSCNL(&(RequestList[i]), &(RequestList[i].menu), pEnv);
    if(rc != WS_ERR_NONE)
    {
      RequestList[i].menu = NULL;
    }
    RequestList[i].wsError = WS_ERR_NONE;
  }

    /* sort the RequestList by menu */
  qsort(RequestList,iNumReqsInList,sizeof(TRACE_REQ),
        wsCompareMenuNumbersForTraceReqs);

  /* return success */
  return(WS_ERR_NONE);

}  /* end wsPrepRequestList() */

/***************************************/
/***********************************************************
   WARNING WARNING WARNING WARNING WARNING WARNING WARNING
   The wsClientIII routines use a single snippet buffer that
   is malloced durin wsInitialize().  The caller should
   NEVER NEVER NEVER Free ptrCurrent->pBuf, and should copy
   the contents of ptrCurrent->pBuf before the next call
   to wsGetNextTraceFromRequestList(), because the old data
   will be overwritten!!!!!!
   DK 0130 2003
   WARNING WARNING WARNING WARNING WARNING WARNING WARNING
***********************************************************/
int wsGetNextTraceFromRequestList(TRACE_REQ* RequestList, wsHANDLE pEnv, 
                           TRACE_REQ** ppResult)
{

  TRACE_REQ * pReq;
  int ctr;
  int rc, ret;
  WS_MENU error_menu;
  int server_error;
  
  double dnow, dTimeoutStart=0.0, dTimeoutSeconds=0.0, dTemp;
  int  bTimeout;

  /* DK Change 02/06/02 */
  *ppResult = NULL;
  /* End DK Change 02/06/02 */

  if(pEnv->iTimeoutMsec > 0)
  {
    bTimeout = TRUE;
    dTimeoutStart = hrtime_ew(&dnow);
    dTimeoutSeconds = (double)(pEnv->iTimeoutMsec) / 1000;
  }
  else
  {
    bTimeout = FALSE;
  }

  if(pEnv->iNumTraceReqsInList <= pEnv->iCurrentTraceReq)
  {
    /* all items in list have already been processed */
    pEnv->iNumTraceReqsInList = pEnv->iCurrentTraceReq = 0;
    wsCloseAllSockets(pEnv);
    return(WS_ERR_LIST_FINISHED);
  }

  pReq = &(RequestList[pEnv->iCurrentTraceReq]);

  /* first check for NULL menu */
  if(!pReq->menu)
  {
    /* we may have already had a failed request
       for a wave_server that had this channel in
       the menu 
    **********************************************/
    if(pReq->wsError != WS_ERR_NONE)
      ret = pReq->wsError;
    else
      ret = WS_ERR_SCNL_NOT_IN_MENU;

    goto Finished;
  }

  /* copy the snippet buffer to the current req */
  pReq->pBuf = pEnv->bSnippetBuffer;
  pReq->bufLen = pEnv->iSnippetBufferSize;

  /* set the CurrentMenu */
  if(pEnv->CurrentMenu)
  {
    if(pEnv->CurrentMenu != pReq->menu)
    {
      if(WS_CL_DEBUG & WS_DEBUG_SERVER_INFO) 
        logit("","Current menu differs from new menu, detaching server.\n"
              "\t\t\tCurrent Menu (%s:%s), New Menu(%s:%s)\n",
              pEnv->CurrentMenu->addr, pEnv->CurrentMenu->port,
              pReq->menu->addr, pReq->menu->port);

      wsDetachServer(pEnv->CurrentMenu,0);
      pEnv->CurrentMenu = pReq->menu;
    }
  }
  else
  {
    pEnv->CurrentMenu = pReq->menu;
  }

  /* try to process the current TRACE_REQ */

  if(WS_CL_DEBUG & WS_DEBUG_DATA_INFO) 
    logit("","Data reqd for (%s,%s,%s,%s): %.0f+%.0f from(%s:%s) - ",
          pReq->sta, pReq->chan, pReq->net, pReq->loc,
          pReq->reqStarttime, (pReq->reqEndtime - pReq->reqStarttime),          
          pReq->menu->addr, pReq->menu->port
         );
  
  
  pReq->menu->stats.iNumSnippetsAttempted++;
  hrtime_ew(&dnow);
  
  if(bTimeout)
  {
    rc = wsGetTraceFromServer(pReq, pReq->menu, pEnv->iTraceTypeForList, 
                              (int)((dTimeoutStart + dTimeoutSeconds - dnow) * 1000));
  }
  else
    rc = wsGetTraceFromServer(pReq, pReq->menu, pEnv->iTraceTypeForList, -1/*timeout*/);
  
  /* check for non server-wide errors */
  if(rc == WS_ERR_NONE || rc == WS_ERR_BUFFER_OVERFLOW)
  {
    pReq->menu->stats.iNumSnippetsRetrieved++;

    if(WS_CL_DEBUG & WS_DEBUG_DATA_INFO) 
      logit("","SUCCESS in %6.2f seconds\n",hrtime_ew(&dTemp) - dnow);

    ret = rc;
    goto Finished;
  }
  else 
  {
    if(rc == WS_WRN_FLAGGED  || rc == WS_ERR_TIMEOUT)
    {
      if(rc == WS_WRN_FLAGGED)
        pReq->menu->stats.iNumSnippetsFlagged++;
      else
        pReq->menu->stats.iNumSnippetsTimedOut++;

      /* don't log FLAG warnings, they are logged in wsGetTraceFromServer(). */
      if((!WS_WRN_FLAGGED) & WS_CL_DEBUG & WS_DEBUG_DATA_WARNINGS) 
        logit("","ERROR(%d) for (%s,%s,%s,%s): %.0f+%.0f from(%s:%s) in %6.2f seconds\n",
              rc, pReq->sta, pReq->chan, pReq->net, pReq->loc,
              pReq->reqStarttime, (pReq->reqEndtime - pReq->reqStarttime),
              pReq->menu->addr, pReq->menu->port,
              hrtime_ew(&dTemp) - dnow);

      /* set the error code */
      if(pReq->wsError == WS_ERR_NONE)
        pReq->wsError = rc;

      /* wsSearchSCNL using current menu */
      rc = wsSearchSCNL(pReq, &(pReq->menu), pEnv);
      if(rc != WS_ERR_NONE)
      {
        pReq->menu = NULL;
        ret = pReq->wsError;
        goto Finished;
      }
    }
    else
    {
      pReq->menu->stats.iNumErrors++;


      if(WS_CL_DEBUG & WS_DEBUG_SERVER_ERRORS) 
        logit("","SERVER ERROR(%d) from(%s:%s) for (%s,%s,%s,%s): %.0f+%.0f "
                 "in %6.2f seconds\n",
              rc, pReq->menu->addr, pReq->menu->port,
              pReq->sta, pReq->chan, pReq->net, pReq->loc,
              pReq->reqStarttime, (pReq->reqEndtime - pReq->reqStarttime),
              hrtime_ew(&dTemp) - dnow);

      /* there is a server problem! */
      pReq->menu->serverstatus = WS_SERVER_STATUS_ERROR;
      error_menu = pReq->menu;
      server_error = rc;  /* from wsGetTraceFromServer() */

      for(ctr=pEnv->iCurrentTraceReq; ctr < pEnv->iNumTraceReqsInList; ctr++)
      {
        pReq = &(RequestList[ctr]);
        if(pReq->menu == error_menu)
        {
          if(pReq->wsError == WS_ERR_NONE)
            pReq->wsError = server_error;

          rc = wsSearchSCNL(pReq, &(pReq->menu), pEnv);

          if(rc != WS_ERR_NONE && rc != WS_WRN_FLAGGED)
          {
            logit("","wsSearchSCNL(): error while searching for SCNL (%s,%s,%s,%s)\n",
                   pReq->sta, pReq->chan, pReq->net, pReq->loc);
            pReq->menu = NULL;
            /* NO NO NO    goto Finished; */
          }
        }
        else
        {
          /* we only want to update the requests from a matching server ,
             and they are ordered by server */
          break;
        }
      }

    }  /* end else  (not successfullish, and not a flagged warning */

    /* re-sort the remaining RequestList by menu */
    qsort(&(RequestList[pEnv->iCurrentTraceReq]),
          pEnv->iNumTraceReqsInList - pEnv->iCurrentTraceReq,
          sizeof(TRACE_REQ),
          wsCompareMenuNumbersForTraceReqs);

  }    /* end else from if wsGetTraceFromServer() successfullish*/

  /* if we got here, we ran into some sort of processing error. */
  /* to be fair to the next trace received (by giving it a full
     timeout time) we should return to the caller, and tell it
     that we had some temporary error.  It shouldn't expect that
     we lost any data, just that we ran into some error, and we
     should be called again.
  ***************************************************************/
  ret = WS_ERR_TEMPORARY;
  *ppResult = NULL;
  return(ret);

Finished:
  pEnv->iCurrentTraceReq++;

  *ppResult = pReq;

  return(ret);

}  /* end wsGetNextTraceFromRequestList() */

/***************************************/
int wsEndRequestList(TRACE_REQ* RequestList, wsHANDLE pEnv)
{
    /* shutdown list params */
    pEnv->iNumTraceReqsInList = pEnv->iCurrentTraceReq = 0;
    wsCloseAllSockets(pEnv);
    return(WS_ERR_NONE);
}  /* end wsEndRequestList() */

/*******************************************************
* Other                                                *
*******************************************************/
void wsCloseAllSockets(wsHANDLE pEnv)
{
  int i;

  /*logit("et","wsCloseAllSockets(): Calling wsDetachServer for each server in menulist. \n");*/
  for(i=0; i< pEnv->iNumMenusInList; i++)
  {
    wsDetachServer(pEnv->MenuList[i],0);
  }

}  /* end wsCloseAllSockets() */


int wsRefreshMenu(WS_MENU menu)
{
  /* DK CLEANUP
   int wsRemoveServer(wsHANDLE pEnv, char * szIPAddr, char * szPort); */

  logit("et","ERROR! wsRefreshMenu() called, but not implemented! Continuing!\n");
  return(WS_ERR_NONE);
}

int wsRefreshAllMenus(wsHANDLE pEnv)
{
  /* for each server, remove from list */
  /* for each server, add to list */
  logit("et","ERROR! wsRefreshAllMenus() called, but not implemented! Continuing!\n");
  return(WS_ERR_NONE);
}

int wsPrintServerStats(wsHANDLE pEnv)
{
  int i;

  logit("e","\n************************************\n");
  logit("e","************************************\n");
  logit("et","Server Statistics\n");
  logit("e","************************************\n");
  logit("e","    Server               Connected    Overhead    Connxs/    Errors  Connected    Snippets Rtrvd/\n"
            "                           (sec)        (sec)    Attempts            Time (%%)    Flgd/Tmout/Atmptd\n");
  for(i=0; i < pEnv->iNumMenusInList; i++)
  {
    logit("e","(%s:%s)%s\t"  /* server:port <fill>*/
             "  %8.2f "     /* conected time */
             "    %7.2f "     /* overhead time */
             "  %3d/%3d    "    /* connections/attempted */
             "  %3d    "       /* errors */
             " %5.2f%% "  /* connection time % */
             "  %6d/%d/%d/%d\n",
          pEnv->MenuList[i]->addr, pEnv->MenuList[i]->port, 
          (strlen(pEnv->MenuList[i]->addr)<14)?"  ":"",
          pEnv->MenuList[i]->stats.dTotalTimeConnected,
          pEnv->MenuList[i]->stats.dTotalTimeOverhead,
          pEnv->MenuList[i]->stats.iNumTotalConnections,
          pEnv->MenuList[i]->stats.iNumTotalConnectionsAttempted,
          pEnv->MenuList[i]->stats.iNumErrors,
          pEnv->MenuList[i]->stats.dTotalTimeConnected/
           (time(NULL) - pEnv->MenuList[i]->stats.tServerAdded)*100,
          pEnv->MenuList[i]->stats.iNumSnippetsRetrieved,
          pEnv->MenuList[i]->stats.iNumSnippetsFlagged,
          pEnv->MenuList[i]->stats.iNumSnippetsTimedOut,
          pEnv->MenuList[i]->stats.iNumSnippetsAttempted
          );

  }
  logit("e","************************************\n\n");

  return(WS_ERR_NONE);
}
