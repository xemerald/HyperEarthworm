/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: hydra_defs.h 2068 2006-01-30 19:41:11Z friberg $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2006/01/30 19:41:10  friberg
 *     added in hydra includes for raypicker module
 *
 *     Revision 1.2  2005/12/16 19:14:55  davidk
 *     moved MAX_PASSPORT_LEN from ioc_defs.h
 *
 *     Revision 1.1  2005/11/16 18:44:43  davidk
 *     Split former ioc_defs.h up into two parts:
 *     common/hydra_defs.h which includes hydra common definitions
 *     and
 *     resp/ioc_defs.h which includes only neic resp definitions.
 *
 *     Revision 1.4  2005/10/19 20:18:54  patton
 *     Moved the loc passport entries out of the ioc_defs (which is a common
 *     include header) to their own response specific header file.  JMP
 *
 *     Revision 1.3  2005/10/05 21:55:57  mark
 *     Added state for forcing RayPellet generation
 *
 *     Revision 1.2  2005/07/06 22:58:53  michelle
 *     added Md state and status msg
 *
 *     Revision 1.1.1.1  2005/06/22 19:30:36  michelle
 *     new directory tree built from files in HYDRA_NEWDIR_2005-06-20 tagged hydra and earthworm projects
 *
 *
 *
 */


#ifndef HYDRA_DEFS_H
#define HYDRA_DEFS_H

/* General definitions used in the IOC effort
 *********************************************/

// Windows compilers, either through nmake or Visual C++, have these as built-in types.
// Redefining them causes problems, and #ifndefs don't work on built-in types.
//#ifndef WIN32
#ifndef true
 #define true 1
 #define false 0
#endif

#ifndef __cplusplus
typedef int bool;
#endif
//#endif


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE DEFINE 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY IOC_API

LANGUAGE C

LOCATION THIS_FILE

CONSTANT_GROUP MODULE_RET constants

CONSTANT MODULE_RET_UNDEFINED
VALUE -2
DESCRIPTION The return code is undefined.  This is the
initial return code for each state.  When the module 
processing the state completes, it should set the return
code as one of the other three values (error, warning, ok).

CONSTANT MODULE_RET_OK
VALUE 0
DESCRIPTION The module successfully completed processing.

CONSTANT MODULE_RET_ERROR
VALUE -1
DESCRIPTION The module experienced an unrecoverable error
while processing.

CONSTANT MODULE_RET_WARNING
VALUE 1
DESCRIPTION The module experienced a hickup while processing.
There might not have been sufficient data for the Origin,
or some other problem may have occured that hampered but
did not prevent processing of the Origin data for the given
state.

WARNING!!!!!!!!!!
DO NOT CHANGE THESE VALUES.  THEY ARE COPIED FROM
SQL Trigger Code!!!!!!!!!!!!!!

*************************************************
************************************************/
/* Return values of processing modules */
#define		MODULE_RET_UNDEFINED  -2
#define		MODULE_RET_OK		0
#define		MODULE_RET_ERROR	-1
#define		MODULE_RET_WARNING	1

/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE DEFINE 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY IOC_API

LANGUAGE C

LOCATION THIS_FILE

CONSTANT_GROUP STATE_STATUS constants

CONSTANT STATE_STATUS_UNDEFINED
VALUE 0
DESCRIPTION Default status identifier, indicating that
the status of a state processing step is undefined/unknown.

CONSTANT STATE_STATUS_SCHEDULED
VALUE 1
DESCRIPTION A state has been scheduled for processing,
but processing has not yet started.

CONSTANT STATE_STATUS_STARTED
VALUE 2
DESCRIPTION A module has begun processing for a state.
Processing is not yet complete.

CONSTANT STATE_STATUS_COMPLETED
VALUE 3
DESCRIPTION Processing of a state has completed.
iModuleRetVal should be set, and there should be
no more processing/manipulation remaining for the state.

WARNING!!!!!!!!!!
DO NOT CHANGE THESE VALUES.  THEY ARE COPIED FROM
SQL Trigger Code!!!!!!!!!!!!!!

*************************************************
************************************************/
/* State status definitions */
#define STATE_STATUS_UNDEFINED 0
#define STATE_STATUS_SCHEDULED 1
#define STATE_STATUS_STARTED   2
#define STATE_STATUS_COMPLETED 3

/* State label definitions */
#define	MAX_LABEL_LEN		40
#define	MAX_STATE_LABEL_LEN		MAX_LABEL_LEN


/* Passport Definitions */
#define MAX_PASSPORT_LEN 1024



#define		MODULE_INTERNAL	"INTERNAL_NO_MODULE"

#endif /* HYDRA_DEFS_H */

