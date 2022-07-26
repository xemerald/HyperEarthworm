/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: ewdb_ora_api_raw_infra.h 7322 2018-04-14 03:27:27Z baker $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2008/12/02 21:37:42  paulf
 *     oracle surgery additions
 *
 *     Revision 1.1  2003/05/20 20:57:10  lucky
 *     Initial revision
 *
 *
 *
 */


/*******************************************************
 *
 * This include file contains only the structure
 * definitions and function prototypes for the raw
 * infrastructure subschema.   
 * 
 * This file should never be included in an application
 * program.  Instead, include the master include file
 * ewdb_ora_api.h
 * 
 *    Lucky Vidmar, May 20, 2003
 *
 *******************************************************/

#ifndef _EWDB_ORA_API_RAW_INFRA_H
#define _EWDB_ORA_API_RAW_INFRA_H



/**********************************************************
 #########################################################
    Struct Definition Section
 #########################################################
**********************************************************/

/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF 

LIBRARY  EWDB_API_LIB

SUB_LIBRARY RAW_INFRASTRUCTURE_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_FunctionBindStruct
TYPE_DEFINITION struct _EWDB_FunctionBindStruct
DESCRIPTION Structure for storing parameters for a function bind.  This
structure is used to store (in the database raw infrastructure schema)
a binding between a device and a response function.  This structure's
contents dictate that a certain device has a certain piece of
functionality (presumably related to signal response) for a time period
between tOn and tOff.

MEMBER idFunctionBind
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION DB ID of the function bind mechanism.

MEMBER idDevice
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION DB ID of the device to which a given functionality is bound.

MEMBER tOn
MEMBER_TYPE double
MEMBER_DESCRIPTION Start of the time interval for which the
functionality exists for the given device.

MEMBER tOff
MEMBER_TYPE double
MEMBER_DESCRIPTION End of the time interval for which the functionality
exists for the given device.

MEMBER idFunction
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION DB ID of the Function bound to the device.

MEMBER szFunctionName
MEMBER_TYPE char[]
MEMBER_DESCRIPTION Name of the function type of the function identified
by idFunction.

MEMBER bOverridable
MEMBER_TYPE int
MEMBER_DESCRIPTION Flag indicating whether the function is an
overridable default for the device.  Devices can be derived from other
devices, and the derived devices inherit the functional properties of
the inheritees.  This flag indicates if this is a property of the
inheritee that should be overriden by the derived device.

MEMBER pFunction
MEMBER_TYPE void *
MEMBER_DESCRIPTION Pointer to the actual Function that is being bound
by this FunctionBind.

*************************************************
************************************************/
typedef struct _EWDB_FunctionBindStruct
{
  EWDBid idFunctionBind;
  EWDBid idDevice;
  EWDBid idFunction;
  char   szFunctionName[EWDB_RAW_INFRA_NAME_LEN];
  double tOff;
  double tOn;
  int    bOverridable;
  void * pFunction;
} EWDB_FunctionBindStruct;

/**********************************************************
 #########################################################
    Function Prototype Section
 #########################################################
**********************************************************/

#endif /* _EWDB_ORA_API_RAW_INFRA_H */
