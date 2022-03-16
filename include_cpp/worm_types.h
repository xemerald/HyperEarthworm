/*
**
*/
//---------------------------------------------------------------------------
#ifndef _WORM_TYPES_H
#define _WORM_TYPES_H
//---------------------------------------------------------------------------

// microsoft pragma to avoid 157 messages in some cases
#pragma warning(disable:4786)

#include <string>
#include <map>
#include <vector>
//using namespace std;

/*
**  YYYYMMDDhhmmss.sss
**  123456789.12345678
*/
#define WORM_TIMESTR_LEN 26
typedef char WORM_TIME[WORM_TIMESTR_LEN+1];
typedef char * WORM_TIME_PTR;
#define WORM_TIME_INVALID "19000101000000.000"

#define WORM_PROGRAMNAME_LEN 38
typedef char PROGRAM_NAME[WORM_PROGRAMNAME_LEN+1];

#define WORM_LOGDIR_LEN 120
typedef char LOG_DIRECTORY[WORM_LOGDIR_LEN+1];

#define WORM_FILENAME_LEN 240
typedef char GEN_FILENAME[WORM_FILENAME_LEN+1];

// This type is sacred -- to support Earthworm MSG_LOGO
typedef unsigned char WORM_INSTALLATION_ID;
#define WORM_INSTALLATION_INVALID (WORM_INSTALLATION_ID)0
#define WORM_INSTALLATION_WILD (WORM_INSTALLATION_ID)0
typedef std::map<std::string, WORM_INSTALLATION_ID> INSTALLATION_MAP;
typedef INSTALLATION_MAP::iterator INSTALLATION_MAP_ITERATOR;

// This type is sacred -- to support Earthworm MSG_LOGO
typedef unsigned char WORM_MODULE_ID;
#define WORM_MODULE_INVALID (WORM_MODULE_ID)0
#define WORM_MODULE_WILD (WORM_MODULE_ID)0
typedef std::map<std::string, WORM_MODULE_ID> MODULE_MAP;
typedef MODULE_MAP::iterator MODULE_MAP_ITERATOR;

// This type is sacred -- to support Earthworm MSG_LOGO
typedef unsigned char WORM_MSGTYPE_ID;
#define WORM_MSGTYPE_INVALID (WORM_MSGTYPE_ID)0
#define WORM_MSGTYPE_WILD (WORM_MSGTYPE_ID)0
typedef std::map<std::string, WORM_MSGTYPE_ID> MESSAGETYPE_MAP;
typedef MESSAGETYPE_MAP::iterator MESSAGETYPE_MAP_ITERATOR;

// This type is sacred -- to support Earthworm MSG_LOGO
typedef unsigned long WORM_RING_ID;
#define WORM_RING_INVALID 0L
typedef std::map<std::string, WORM_RING_ID> RING_MAP;
typedef RING_MAP::iterator RING_MAP_ITERATOR;



#define MAX_INSTALLNAME_LEN 32
typedef char WORM_INSTALL_NAME[MAX_INSTALLNAME_LEN+1];

#define MAX_RINGNAME_LEN 32
typedef char WORM_RING_NAME[MAX_RINGNAME_LEN+1];

#define MAX_MODNAME_LEN 32
typedef char WORM_MODULE_NAME[MAX_MODNAME_LEN+1];

#define MAX_MSGTYPENAME_LEN 32
typedef char WORM_MSGTYPE_NAME[MAX_MSGTYPENAME_LEN+1];


// Whenever EARTHWORM'S transport.h is included in a project,
// _USING_EW_XPORT must be defined
#ifndef _USING_EW_XPORT
typedef struct {                   /***** description of message source *************/
    WORM_MSGTYPE_ID       type;    /* message is of this type                       */
    WORM_MODULE_ID        mod;     /* from this module                              */
    WORM_INSTALLATION_ID  instid;  /* installation id (e.g. 1 = USGS) [installation] */
} MSG_LOGO;                        /*************************************************/

#endif



#endif

