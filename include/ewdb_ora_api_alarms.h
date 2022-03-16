/*
 *   THIS FILE IS UNDER RCS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: ewdb_ora_api_alarms.h 7322 2018-04-14 03:27:27Z baker $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.1  2008/12/02 21:37:42  paulf
 *     oracle surgery additions
 *
 *     Revision 1.10  2005/03/07 18:48:17  mark
 *     Added bSecondary to rules
 *
 *     Revision 1.9  2005/03/03 17:53:36  mark
 *     Added bActive flag to recipient struct
 *
 *     Revision 1.8  2005/02/17 23:01:56  mark
 *     Added WipeAllAlarmsInfo fn
 *
 *     Revision 1.7  2005/02/03 21:28:33  mark
 *     Added alarm groups; numerous changes
 *
 *     Revision 1.6  2005/01/19 21:32:58  mark
 *     Added iPhases and bUseMag to alarms rule struct
 *
 *     Revision 1.5  2004/12/15 23:05:17  mark
 *     Added new function prototypes
 *
 *     Revision 1.4  2004/11/23 17:21:29  mark
 *     Added idPolygon to rule struct
 *
 *     Revision 1.3  2004/04/23 19:35:49  michelle
 *     added OkToExecute field to alarms audit struck
 *     need this to prevent an alarm being sent twice, once for mag and second for phase threshold, when
 *     this should be just one alarm actually sent/executed with both phase and mag info
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
 * definitions and function prototypes for the alarms
 * subschema.   
 * 
 * This file should never be included in an application
 * program.  Instead, include the master include file
 * ewdb_ora_api.h
 * 
 *    Lucky Vidmar, May 20, 2003
 *
 *******************************************************/

#ifndef _EWDB_ORA_API_ALARMS_H
#define _EWDB_ORA_API_ALARMS_H



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

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_AlarmsRecipientStruct
TYPE_DEFINITION struct _EWDB_AlarmsRecipientStruct
DESCRIPTION Information about a recipient of alarms.

MEMBER idRecipient
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of this recipient.

MEMBER dPriority
MEMBER_TYPE int
MEMBER_DESCRIPTION Priority number of this recipient.

MEMBER sDescription
MEMBER_TYPE char[EWDB_MAXIMUM_AMPS_PER_CODA]
MEMBER_DESCRIPTION Name label for this recipient

MEMBER bActive
MEMBER_TYPE int
MEMBER_DESCRIPTION Non-zero if alarms should be sent to this recipient (active),
		or 0 if no alarms should be sent (inactive).

*************************************************
************************************************/
typedef struct _EWDB_AlarmsRecipientStruct
{
	EWDBid 	idRecipient;
	int 	dPriority;
	char	sDescription[1024];
	int		bActive;
} EWDB_AlarmsRecipientStruct;


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF

LIBRARY  EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_EmailDeliveryStruct
TYPE_DEFINITION struct _EWDB_EmailDeliveryStruct
DESCRIPTION Information about an email delivery. 

MEMBER idDelivery
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of this delivery..

MEMBER sAddress
MEMBER_TYPE char[256]
MEMBER_DESCRIPTION Email address to deliver to.

MEMBER sMailServer
MEMBER_TYPE char[256]
MEMBER_DESCRIPTION Mail server to use for delivery.

*************************************************
************************************************/
typedef struct _EWDB_EmailDeliveryStruct
{
	EWDBid	idDelivery;
	char	sAddress[256];	
	char	sMailServer[256];	
} EWDB_EmailDeliveryStruct;


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF

LIBRARY  EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_PagerDeliveryStruct
TYPE_DEFINITION struct _EWDB_PagerDeliveryStruct
DESCRIPTION Information about a pager delivery. 

MEMBER idDelivery
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of this delivery..

MEMBER sNumber
MEMBER_TYPE char[256]
MEMBER_DESCRIPTION Pager number to send to.

MEMBER sPagerCompany
MEMBER_TYPE char[256]
MEMBER_DESCRIPTION Pager company to use.

*************************************************
************************************************/
typedef struct _EWDB_PagerDeliveryStruct
{
	EWDBid	idDelivery;
	char	sNumber[256];	
	char	sPagerCompany[256];	
} EWDB_PagerDeliveryStruct;


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF

LIBRARY  EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_PhoneDeliveryStruct
TYPE_DEFINITION struct _EWDB_PhoneDeliveryStruct
DESCRIPTION Information about a phone delivery. 

MEMBER idDelivery
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of this delivery..

MEMBER sPhoneNumber
MEMBER_TYPE char[256]
MEMBER_DESCRIPTION Telephone number to dial.

*************************************************
************************************************/
typedef struct _EWDB_PhoneDeliveryStruct
{
	EWDBid	idDelivery;
	char	sPhoneNumber[256];	
} EWDB_PhoneDeliveryStruct;


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF

LIBRARY  EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_QddsDeliveryStruct
TYPE_DEFINITION struct _EWDB_QddsDeliveryStruct
DESCRIPTION Information about a qdds delivery. 

MEMBER idDelivery
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of this delivery..

MEMBER sQddsDirectory
MEMBER_TYPE char[256]
MEMBER_DESCRIPTION Full path to the directory
where qdds files should be written.

*************************************************
************************************************/
typedef struct _EWDB_QddsDeliveryStruct
{
	EWDBid	idDelivery;
	char	sQddsDirectory[256];	
} EWDB_QddsDeliveryStruct;


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF

LIBRARY  EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_CustomDeliveryStruct
TYPE_DEFINITION struct _EWDB_CustomDeliveryStruct
DESCRIPTION Information about a custom delivery. 

MEMBER idDelivery
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of this delivery..

MEMBER sDescription
MEMBER_TYPE char[256]
MEMBER_DESCRIPTION Name label for this custom delivery.

*************************************************
************************************************/
typedef struct _EWDB_CustomDeliveryStruct
{
	EWDBid	idDelivery;
	char	sDescription[256];
} EWDB_CustomDeliveryStruct;


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF

LIBRARY  EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_AlarmDeliveryUnionStruct
TYPE_DEFINITION struct _EWDB_AlarmDeliveryUnionStruct
DESCRIPTION Structure simulating a union of different
delivery mechanism types, switched based on the value
of DelMethodInd.

MEMBER DelMethodInd
MEMBER_TYPE int
MEMBER_DESCRIPTION Index of the delivery method
corresponding to the constants defined above.

MEMBER idRecipientDelivery
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database id of the recipient.

MEMBER email
MEMBER_TYPE EWDB_EmailDeliveryStruct
MEMBER_DESCRIPTION Information about an email delivery.

MEMBER pager
MEMBER_TYPE EWDB_PagerDeliveryStruct
MEMBER_DESCRIPTION Information about a pager delivery.

MEMBER phone
MEMBER_TYPE EWDB_PhoneDeliveryStruct
MEMBER_DESCRIPTION Information about a phone delivery.

MEMBER qdds
MEMBER_TYPE EWDB_QddsDeliveryStruct
MEMBER_DESCRIPTION Information about a qdds delivery.

MEMBER custom
MEMBER_TYPE EWDB_CustomDeliveryStruct
MEMBER_DESCRIPTION Information about a custom delivery.

*************************************************
************************************************/
typedef struct _EWDB_AlarmDeliveryUnionStruct
{
	int							DelMethodInd;
	EWDBid						idRecipientDelivery;
	EWDB_EmailDeliveryStruct	email;
	EWDB_PagerDeliveryStruct	pager;
	EWDB_PhoneDeliveryStruct	phone;
	EWDB_QddsDeliveryStruct		qdds;
	EWDB_CustomDeliveryStruct	custom;
} EWDB_AlarmDeliveryUnionStruct;


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF

LIBRARY  EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_AlarmsCritProgramStruct
TYPE_DEFINITION struct _EWDB_AlarmsCritProgramStruct
DESCRIPTION Information about a criteria program 

MEMBER idCritProgram
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of this program.

MEMBER sProgName
MEMBER_TYPE char[256]
MEMBER_DESCRIPTION Full path to the criteria program executable

MEMBER sProgDir
MEMBER_TYPE char[256]
MEMBER_DESCRIPTION Full path to the working directory.

MEMBER sProgDescription
MEMBER_TYPE char[256]
MEMBER_DESCRIPTION Label for this program.

*************************************************
************************************************/
typedef struct _EWDB_AlarmsCritProgramStruct
{
	EWDBid		idCritProgram;
	char		sProgName[256];
	char		sProgDir[256];
	char		sProgDescription[256];
} EWDB_AlarmsCritProgramStruct;


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF

LIBRARY  EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_AlarmsFormatStruct
TYPE_DEFINITION struct _EWDB_AlarmsFormatStruct
DESCRIPTION Information about a format.

MEMBER idFormat
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of this format.

MEMBER sDescription
MEMBER_TYPE char[256]
MEMBER_DESCRIPTION Label for this format.

MEMBER sFmtInsert
MEMBER_TYPE char[EWDB_ALARMS_MAX_FORMAT_LEN]
MEMBER_DESCRIPTION Insertion format.

MEMBER sFmtDelete
MEMBER_TYPE char[EWDB_ALARMS_MAX_FORMAT_LEN]
MEMBER_DESCRIPTION Deletion format.

*************************************************
************************************************/
typedef struct _EWDB_AlarmsFormatStruct
{
	EWDBid		idFormat;
	char		sDescription[256];
	char		sFmtInsert[EWDB_ALARMS_MAX_FORMAT_LEN];
	char		sFmtDelete[EWDB_ALARMS_MAX_FORMAT_LEN];
} EWDB_AlarmsFormatStruct;


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF

LIBRARY  EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_AlarmsRuleStruct
TYPE_DEFINITION struct _EWDB_AlarmsRuleStruct
DESCRIPTION Information about a single rule.

MEMBER idRule
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of this rule.

MEMBER dMag
MEMBER_TYPE double
MEMBER_DESCRIPTION Threshhold magnitude for this rule.

MEMBER Auto
MEMBER_TYPE int
MEMBER_DESCRIPTION Automatic or reviewed criterion.

MEMBER idRecipientDelivery
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of the delivery to be used
		if this rule is satisfied.  If idGroup is non-zero, this must be zero.

MEMBER DeliveryIndex
MEMBER_TYPE ind
MEMBER_DESCRIPTION Index into the Delivery array of the AlarmList
structure defined in the APPS section. 

MEMBER Format
MEMBER_TYPE EWDB_AlarmsFormatStruct
MEMBER_DESCRIPTION Information about the format to use.

MEMBER bCritInUse
MEMBER_TYPE int
MEMBER_DESCRIPTION Should the criteria program be evaluated?

MEMBER CritProg
MEMBER_TYPE EWDB_AlarmsCritProgramStruct
MEMBER_DESCRIPTION Information about the criteria program to use.

MEMBER idPolygon
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of the polygon to match with this rule, or 0 if there's
      no polygon (i.e. anywhere in the world)

MEMBER bUseMag
MEMBER_TYPE int
MEMBER_DESCRIPTION Should a magnitude be used with this rule?  If not, only number of
      phases will be used.

MEMBER iPhases
MEMBER_TYPE int
MEMBER_DESCRIPTION Minimum number of phases used to trigger an alarm.

MEMBER iGroup
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of the group associated with this rule, or 0 for no group.
		If idRecipientDelivery is non-zero, this must be 0.

MEMBER bSecondary
MEMBER_TYPE int
MEMBER_DESCRIPTION 1 if an alarm will be sent if the system is "secondary", 0 if alarms
		only sent for "primary" systems.  Only applies to groups; non-group alarms will
		only be sent to "primary" systems.

*************************************************
************************************************/
typedef struct _EWDB_AlarmsRuleStruct
{
	EWDBid							idRule;
	double							dMag;
	int								Auto;	
	EWDBid							idRecipientDelivery;
	int								DeliveryIndex; 
	EWDB_AlarmsFormatStruct			Format;
	int								bCritInUse;	
	EWDB_AlarmsCritProgramStruct	CritProg;
	int								idPolygon;
	int								bUseMag;
	int								iPhases;
	EWDBid							idGroup;
	int								bSecondary;
} EWDB_AlarmsRuleStruct;


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF

LIBRARY  EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_AlarmsRecipientStructSummary
TYPE_DEFINITION struct _EWDB_AlarmsRecipientStructSummary
DESCRIPTION Summary information about how an alarm should be delivered to 
a recipient.

MEMBER idRule
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of the rule which raised the alarm.

MEMBER sTableName
MEMBER_TYPE char[64]
MEMBER_DESCRIPTION Name of the delivery table (email, pager, etc.) to use.

MEMBER idDelivery
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of the delivery to use.

MEMBER idRecipientDelivery
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of the recipient's delivery to use.

MEMBER idRecipient
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of the recipient.

*************************************************
************************************************/
typedef struct _EWDB_AlarmsRecipientStructSummary
{
    EWDBid			idRule;
	char			sTableName[64];
	EWDBid			idDelivery;
	EWDBid			idRecipientDelivery;
	EWDBid			idRecipient;
} EWDB_AlarmsRecipientStructSummary;


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF

LIBRARY  EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_AlarmAuditStruct
TYPE_DEFINITION struct _EWDB_AlarmAuditStruct
DESCRIPTION Information about an alarms audit entry.

MEMBER idAudit
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of this audit.

MEMBER idEvent
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of the event which triggered this alarm.

MEMBER bAuto
MEMBER_TYPE int
MEMBER_DESCRIPTION Automatic or reviewed criterion.

MEMBER Recipient
MEMBER_TYPE EWDB_AlarmsRecipientStruct
MEMBER_DESCRIPTION Information about the recipient who receieved this alarm.

MEMBER Format
MEMBER_TYPE EWDB_AlarmsFormatStruct
MEMBER_DESCRIPTION Information about the format used to send.

MEMBER idDelivery
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of the delivery mechanism used to send.

MEMBER tAlarmDeclared
MEMBER_TYPE double
MEMBER_DESCRIPTION Time when the alarm was declared.

MEMBER tAlarmExecuted
MEMBER_TYPE double
MEMBER_DESCRIPTION Time when the alarm was executed.

MEMBER DelMethodInd
MEMBER_TYPE int
MEMBER_DESCRIPTION  Index of the delivery method
corresponding to the constants defined above.

MEMBER sInvocationString
MEMBER_TYPE char[256]
MEMBER_DESCRIPTION  Text inserted into the alarm message by the calling program.

MEMBER idGroup
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of the group associated with this audit.  Can be 0
		if no groups are associated.

MEMBER idRecipientDelivery
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of the delivery mechanism used with this audit.
		Cannot be 0.

*************************************************
************************************************/
typedef struct _EWDB_AlarmAuditStruct
{
	EWDBid								idAudit;
	EWDBid								idEvent;
	EWDBid								idCore;
	int									bAuto;
	int									AlarmType;
	int									OkToExecute;
    EWDB_AlarmsRecipientStruct  		Recipient;
    EWDB_AlarmsFormatStruct	 			Format;
    EWDBid	    						idDelivery;
	double								tAlarmDeclared;
	double								tAlarmExecuted;
	int									DelMethodInd;
	char								sInvocationString[256];
	EWDBid								idGroup;
	EWDBid								idRecipientDelivery;
} EWDB_AlarmAuditStruct;


/* Definition of Alarm Types */
#define		ALARM_TYPE_LOCATION		10
#define		ALARM_TYPE_ML			11
#define		ALARM_TYPE_MB			12
#define		ALARM_TYPE_MBLG			13
#define		ALARM_TYPE_MS			14
#define		ALARM_TYPE_MWP			15


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF

LIBRARY  EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_AlarmsGroupStruct
TYPE_DEFINITION struct _EWDB_AlarmsGroupStruct
DESCRIPTION Information about an alarm group.

MEMBER idGroup
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of this group.

MEMBER sName
MEMBER_TYPE char[256]
MEMBER_DESCRIPTION Human-readable name associated with this group.

MEMBER bActive
MEMBER_TYPE int
MEMBER_DESCRIPTION Non-zero if alarms should be sent to this group (active),
		or 0 if no alarms should be sent (inactive).

*************************************************
************************************************/

typedef struct _EWDB_AlarmsGroupStruct
{
	EWDBid	idGroup;
	char	sName[256];
	int		bActive;
} EWDB_AlarmsGroupStruct;


/************************************************
************ SPECIAL FORMATTED COMMENT **********
EW5 API FORMATTED COMMENT
TYPE TYPEDEF

LIBRARY  EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

TYPEDEF EWDB_AlarmsGroupRecipientStruct
TYPE_DEFINITION struct _EWDB_AlarmsGroupRecipientStruct
DESCRIPTION Information about an alarm group.

MEMBER idGroupRecipient
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of this group recipient.

MEMBER idGroup
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of the group this recipient is associated with.

MEMBER idRecipientDelivery
MEMBER_TYPE EWDBid
MEMBER_DESCRIPTION Database ID of the recipient's delivery.

MEMBER bActive
MEMBER_TYPE int
MEMBER_DESCRIPTION Non-zero if alarms should be sent to this group member (active),
		or 0 if no alarms should be sent (inactive).

*************************************************
************************************************/

typedef struct _EWDB_AlarmsGroupRecipientStruct
{
	EWDBid	idGroupRecipient;
	EWDBid	idGroup;
	EWDBid	idRecipientDelivery;
	int		bActive;
} EWDB_AlarmsGroupRecipientStruct;


/**********************************************************
 #########################################################
    Function Prototype Section
 #########################################################
**********************************************************/

/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_CreateAlarmAudit

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_CreateAlarmAudit.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

PARAMETER 1
PARAM_NAME pAudit
PARAM_TYPE EWDB_AlarmAuditStruct * 
PARAM_DESCRIPTION Pointed to the structure containing information
about the audit entry to be created.

DESCRIPTION Creates or updates an alarm audit entry. If idAudit
value in the structure is set, it updates the values for the
audit. Otherwise, it creates a new audit entry.

*************************************************
************************************************/
int ewdb_api_CreateAlarmAudit(EWDB_AlarmAuditStruct *pAudit);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_CreateAlarmsCriteria

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_CreateAlarmsCriteria.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

PARAMETER 1
PARAM_NAME pCrit
PARAM_TYPE EWDB_AlarmsCritProgramStruct * 
PARAM_DESCRIPTION Pointer to the structure containing the information
about a criteria program to insert into the database.

DESCRIPTION Creates or updates a criteria program entry in the database. 
If idCritProgram value in the structure is set, it updates the values 
for the program. Otherwise, it creates a new program entry.

*************************************************
************************************************/
int ewdb_api_CreateAlarmsCriteria(EWDB_AlarmsCritProgramStruct *pCrit);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_CreateAlarmsFormat

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_CreateAlarmsFormat.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

PARAMETER 1
PARAM_NAME pForm
PARAM_TYPE EWDB_AlarmsFormatStruct * 
PARAM_DESCRIPTION Pointer to the structure containing the information
about a format to insert into the database.

DESCRIPTION Creates or updates a format entry in the database. 
If idFormat value in the structure is set, it updates the values 
for the format. Otherwise, it creates a new format entry.

*************************************************
************************************************/
int ewdb_api_CreateAlarmsFormat(EWDB_AlarmsFormatStruct *pForm);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_CreateAlarmsRule

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_CreateAlarmsRule.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

PARAMETER 1
PARAM_NAME pRule
PARAM_TYPE EWDB_AlarmsRuleStruct * 
PARAM_DESCRIPTION Pointer to the structure containing the information
about the rule to insert into the database.

PARAMETER 2
PARAM_NAME pDel
PARAM_TYPE EWDB_AlarmDeliveryUnionStruct * 
PARAM_DESCRIPTION Pointer to the structure containing the information
about the delivery mechanism to associate with this rule.

DESCRIPTION Creates or updates a rule entry in the database. 
If idRule value in the pRule structure is set, it updates the values 
for the current rule. Otherwise, it creates a new rule entry.

*************************************************
************************************************/
int ewdb_api_CreateAlarmsRule(EWDB_AlarmsRuleStruct *pRule, 
                              EWDB_AlarmDeliveryUnionStruct *pDel);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_CreateAlarmsRecipient

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_CreateAlarmsRecipient.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

PARAMETER 1
PARAM_NAME pRecipient
PARAM_TYPE EWDB_AlarmsRecipientStruct * 
PARAM_DESCRIPTION Pointer to the structure containing the information
about a recipient to insert into the database.

DESCRIPTION Creates or updates a recipient entry in the database. 
If idRecipient value in the structure is set, it updates the values 
for the current recipient. Otherwise, it creates a new recipient entry.

*************************************************
************************************************/
int ewdb_api_CreateAlarmsRecipient(EWDB_AlarmsRecipientStruct *pRecipient);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_CreateCustomDelivery

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_CreateCustomDelivery.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

PARAMETER 1
PARAM_NAME *pidRecipientDelivery
PARAM_TYPE int 
PARAM_DESCRIPTION Pointer to the database ID of the recipient delivery for 
this delivery mechanism returned by this function.

PARAMETER 2
PARAM_NAME idRecipient
PARAM_TYPE int 
PARAM_DESCRIPTION Database ID of the recipient for whom this delivery is created.

PARAMETER 3
PARAM_NAME *pCustom
PARAM_TYPE EWDB_CustomDeliveryStruct 
PARAM_DESCRIPTION Information about the delivery to insert.

PARAMETER 4
PARAM_NAME isAudit
PARAM_TYPE int 
PARAM_DESCRIPTION If 1, the values should be inserted into the alarms audit
tables, otherwise they should go to the actual alarm tables.

DESCRIPTION If the current delivery is already in the database, it updates
its values. Otherwise, it creates a new delivery and returns the database
ID of the recipient delivery value (pidRecipientDelivery).

*************************************************
************************************************/
int ewdb_api_CreateCustomDelivery(int *pidRecipientDelivery,
                                  int idRecipient, 
                                  EWDB_CustomDeliveryStruct *pCustom, 
                                  int isAudit);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_CreateEmailDelivery

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_CreateEmailDelivery.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

PARAMETER 1
PARAM_NAME *pidRecipientDelivery
PARAM_TYPE int 
PARAM_DESCRIPTION Pointer to the database ID of the recipient delivery for 
this delivery mechanism returned by this function.

PARAMETER 2
PARAM_NAME idRecipient
PARAM_TYPE int 
PARAM_DESCRIPTION Database ID of the recipient for whom this delivery is created.

PARAMETER 3
PARAM_NAME *pEmail
PARAM_TYPE EWDB_EmailDeliveryStruct 
PARAM_DESCRIPTION Information about the delivery to insert.

PARAMETER 4
PARAM_NAME isAudit
PARAM_TYPE int 
PARAM_DESCRIPTION If 1, the values should be inserted into the alarms audit
tables, otherwise they should go to the actual alarm tables.

DESCRIPTION If the current delivery is already in the database, it updates
its values. Otherwise, it creates a new delivery and returns the database
ID of the recipient delivery value (pidRecipientDelivery).

*************************************************
************************************************/
int ewdb_api_CreateEmailDelivery(int *pidRecipientDelivery,
                                 int idRecipient, 
                                 EWDB_EmailDeliveryStruct *pEmail, 
                                 int isAudit);

/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_CreatePagerDelivery

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_CreatePagerDelivery.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

PARAMETER 1
PARAM_NAME *pidRecipientDelivery
PARAM_TYPE int 
PARAM_DESCRIPTION Pointer to the database ID of the recipient delivery for 
this delivery mechanism returned by this function.

PARAMETER 2
PARAM_NAME idRecipient
PARAM_TYPE int 
PARAM_DESCRIPTION Database ID of the recipient for whom this delivery is created.

PARAMETER 3
PARAM_NAME *pPager
PARAM_TYPE EWDB_PagerDeliveryStruct 
PARAM_DESCRIPTION Information about the delivery to insert.

PARAMETER 4
PARAM_NAME isAudit
PARAM_TYPE int 
PARAM_DESCRIPTION If 1, the values should be inserted into the alarms audit
tables, otherwise they should go to the actual alarm tables.

DESCRIPTION If the current delivery is already in the database, it updates
its values. Otherwise, it creates a new delivery and returns the database
ID of the recipient delivery value (pidRecipientDelivery).

*************************************************
************************************************/
int ewdb_api_CreatePagerDelivery(int *pidRecipientDelivery, int idRecipient, 
      EWDB_PagerDeliveryStruct *pPager, int isAudit);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_CreatePhoneDelivery

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_CreatePhoneDelivery.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

PARAMETER 1
PARAM_NAME *pidRecipientDelivery
PARAM_TYPE int 
PARAM_DESCRIPTION Pointer to the database ID of the recipient delivery for 
this delivery mechanism returned by this function.

PARAMETER 2
PARAM_NAME idRecipient
PARAM_TYPE int 
PARAM_DESCRIPTION Database ID of the recipient for whom this delivery is created.

PARAMETER 3
PARAM_NAME *pPhone
PARAM_TYPE EWDB_PhoneDeliveryStruct 
PARAM_DESCRIPTION Information about the delivery to insert.

PARAMETER 4
PARAM_NAME isAudit
PARAM_TYPE int 
PARAM_DESCRIPTION If 1, the values should be inserted into the alarms audit
tables, otherwise they should go to the actual alarm tables.

DESCRIPTION If the current delivery is already in the database, it updates
its values. Otherwise, it creates a new delivery and returns the database
ID of the recipient delivery value (pidRecipientDelivery).

*************************************************
************************************************/
int ewdb_api_CreatePhoneDelivery(int *pidRecipientDelivery, int idRecipient, 
      EWDB_PhoneDeliveryStruct *pPhone, int isAudit);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_CreateQddsDelivery

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_CreateQddsDelivery.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

PARAMETER 1
PARAM_NAME *pidRecipientDelivery
PARAM_TYPE int 
PARAM_DESCRIPTION Pointer to the database ID of the recipient delivery for 
this delivery mechanism returned by this function.

PARAMETER 2
PARAM_NAME idRecipient
PARAM_TYPE int 
PARAM_DESCRIPTION Database ID of the recipient for whom this delivery is created.

PARAMETER 3
PARAM_NAME *pQdds
PARAM_TYPE EWDB_QddsDeliveryStruct 
PARAM_DESCRIPTION Information about the delivery to insert.

PARAMETER 4
PARAM_NAME isAudit
PARAM_TYPE int 
PARAM_DESCRIPTION If 1, the values should be inserted into the alarms audit
tables, otherwise they should go to the actual alarm tables.

DESCRIPTION If the current delivery is already in the database, it updates
its values. Otherwise, it creates a new delivery and returns the database
ID of the recipient delivery value (pidRecipientDelivery).

*************************************************
************************************************/
int ewdb_api_CreateQddsDelivery(int *pidRecipientDelivery, int idRecipient, 
     EWDB_QddsDeliveryStruct *pQdds, int isAudit);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_DeleteAlarmsFormat

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_DeleteAlarmsFormat.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

PARAMETER 1
PARAM_NAME idFormat
PARAM_TYPE int 
PARAM_DESCRIPTION Database ID of the format entry to delete.

DESCRIPTION Permanently deletes the format entry from the database.

*************************************************
************************************************/
int ewdb_api_DeleteAlarmsFormat(int idFormat);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_DeleteAlarmsRule

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_DeleteAlarmsRule.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

PARAMETER 1
PARAM_NAME idRule
PARAM_TYPE int 
PARAM_DESCRIPTION Database ID of the rule entry to delete.

DESCRIPTION Permanently deletes the rule entry from the database.

*************************************************
************************************************/
int ewdb_api_DeleteAlarmsRule(int idRule);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_DeleteAlarmsRecipient

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_DeleteAlarmsRecipient.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

PARAMETER 1
PARAM_NAME idRule
PARAM_TYPE int 
PARAM_DESCRIPTION Database ID of the recipient entry to delete.

DESCRIPTION Permanently deletes the recipient entry from the database.

*************************************************
************************************************/
int ewdb_api_DeleteAlarmsRecipient(int idRecipient);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_DeleteCritProgram

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_DeleteCritProgram.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

PARAMETER 1
PARAM_NAME idRule
PARAM_TYPE int 
PARAM_DESCRIPTION Database ID of the criteria program entry to delete.

DESCRIPTION Permanently deletes the criteria program entry from the database.

*************************************************
************************************************/
int ewdb_api_DeleteCritProgram(int idCritProgram);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_DeleteCustomDelivery

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_DeleteCustomDelivery.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

PARAMETER 1
PARAM_NAME idDelivery
PARAM_TYPE int 
PARAM_DESCRIPTION Database ID of the delivery to deleted.

DESCRIPTION Permanently deletes the delivery entry from the database.

*************************************************
************************************************/
int ewdb_api_DeleteCustomDelivery(int idDelivery);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_DeleteEmailDelivery

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_DeleteEmailDelivery.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

PARAMETER 1
PARAM_NAME idDelivery
PARAM_TYPE int 
PARAM_DESCRIPTION Database ID of the delivery to deleted.

DESCRIPTION Permanently deletes the delivery entry from the database.

*************************************************
************************************************/
int ewdb_api_DeleteEmailDelivery(int idDelivery);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_DeletePagerDelivery

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_DeletePagerDelivery.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

PARAMETER 1
PARAM_NAME idDelivery
PARAM_TYPE int 
PARAM_DESCRIPTION Database ID of the delivery to deleted.

DESCRIPTION Permanently deletes the delivery entry from the database.

*************************************************
************************************************/
int ewdb_api_DeletePagerDelivery(int idDelivery);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_DeletePhoneDelivery

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_DeletePhoneDelivery.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

PARAMETER 1
PARAM_NAME idDelivery
PARAM_TYPE int 
PARAM_DESCRIPTION Database ID of the delivery to deleted.

DESCRIPTION Permanently deletes the delivery entry from the database.

*************************************************
************************************************/
int ewdb_api_DeletePhoneDelivery(int idDelivery);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_DeleteQddsDelivery

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_DeleteQddsDelivery.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

PARAMETER 1
PARAM_NAME idDelivery
PARAM_TYPE int 
PARAM_DESCRIPTION Database ID of the delivery to deleted.

DESCRIPTION Permanently deletes the delivery entry from the database.

*************************************************
************************************************/
int ewdb_api_DeleteQddsDelivery(int idDelivery);



/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetAlarmsAudit

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_GetAlarmsAudit.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Successful return, but there were more entries found
in the database than could be written into the supplied buffer.

PARAMETER 1
PARAM_NAME idEvent
PARAM_TYPE int 
PARAM_DESCRIPTION Database ID of the event for which alarm audits
are being retrieved.

PARAMETER 2
PARAM_NAME *pAudit
PARAM_TYPE EWDB_AlarmAuditStruct 
PARAM_DESCRIPTION Preallocated array of audit structures to be filled.

PARAMETER 3
PARAM_NAME *NumFound
PARAM_TYPE int 
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of items found in the database..

PARAMETER 4
PARAM_NAME *NumRetrieved
PARAM_TYPE int 
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of items retrieved from the database..

PARAMETER 5
PARAM_NAME BufferLen
PARAM_TYPE int 
PARAM_DESCRIPTION Length of the input buffer - number of preallocated
structures to be filled.

DESCRIPTION For a given idEvent, retrieves and writes to the pAudit
buffer the information about the alarms that were sent for this event.
It will write up to the BufferLen records, then return the number of 
found and retrieved items so that the calling routine can re-allocate the 
buffer and call again.

*************************************************
************************************************/
int ewdb_api_GetAlarmsAudit(int idEvent, EWDB_AlarmAuditStruct *pAudit,
                            int *NumFound, int *NumRetrieved, int BufferLen);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetAlarmsCriteriaList

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_GetAlarmsCriteriaList.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Successful return, but there were more entries found
in the database than could be written into the supplied buffer.

PARAMETER 1
PARAM_NAME idCritProgram
PARAM_TYPE int 
PARAM_DESCRIPTION Database ID of the criteria program to retrieve. If present,
the function will retrieve the information for that criteria program only. 
Otherwise, all programs are retrieved.

PARAMETER 2
PARAM_NAME *pCrit
PARAM_TYPE EWDB_AlarmAuditStruct 
PARAM_DESCRIPTION Preallocated array of criteria program structures to be filled.

PARAMETER 3
PARAM_NAME *NumFound
PARAM_TYPE int 
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of items found in the database..

PARAMETER 4
PARAM_NAME *NumRetrieved
PARAM_TYPE int 
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of items retrieved from the database..

PARAMETER 5
PARAM_NAME BufferLen
PARAM_TYPE int 
PARAM_DESCRIPTION Length of the input buffer - number of preallocated
structures to be filled.

DESCRIPTION If idCritProgram is greater than 0, the function returns information
about that criteria program only. Otherwise it retrieves and writes to the 
pCrit buffer the information about all criteria programs. 
It will write up to the BufferLen records, then return the number of 
found and retrieved items so that the calling routine can re-allocate the 
buffer and call again.

*************************************************
************************************************/
int ewdb_api_GetAlarmsCriteriaList(int idCritProgram,
      EWDB_AlarmsCritProgramStruct *pCrit, 
      int *NumFound, int *NumRetrieved, int BufferLen);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetAlarmsFormats

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_GetAlarmsFormats.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Successful return, but there were more entries found
in the database than could be written into the supplied buffer.

PARAMETER 1
PARAM_NAME idFormat
PARAM_TYPE int 
PARAM_DESCRIPTION Database ID of the format to retrieve. If present,
the function will retrieve the information for that format only. 
Otherwise, all formats are retrieved.

PARAMETER 2
PARAM_NAME *pFormat
PARAM_TYPE EWDB_AlarmsFormatStruct 
PARAM_DESCRIPTION Preallocated array of format structures to be filled.

PARAMETER 3
PARAM_NAME *NumFound
PARAM_TYPE int 
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of items found in the database..

PARAMETER 4
PARAM_NAME *NumRetrieved
PARAM_TYPE int 
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of items retrieved from the database..

PARAMETER 5
PARAM_NAME BufferLen
PARAM_TYPE int 
PARAM_DESCRIPTION Length of the input buffer - number of preallocated
structures to be filled.

DESCRIPTION If idFormat is greater than 0, the function returns information
about that format only. Otherwise it retrieves and writes to the 
pFormat buffer the information about all formats. 
It will write up to the BufferLen records, then return the number of 
found and retrieved items so that the calling routine can re-allocate the 
buffer and call again.

*************************************************
************************************************/
int ewdb_api_GetAlarmsFormats(int idFormat, EWDB_AlarmsFormatStruct *pFormat, 
                              int *NumFound, int *NumRetrieved, int BufferLen);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetAlarmsRules

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_GetAlarmsRules.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Successful return, but there were more entries found
in the database than could be written into the supplied buffer.

PARAMETER 1
PARAM_NAME idRule
PARAM_TYPE int 
PARAM_DESCRIPTION Database ID of the rule to retrieve. If present,
the function will retrieve the information for that rule only. 
Otherwise, all rules are retrieved.

PARAMETER 2
PARAM_NAME *pRule
PARAM_TYPE EWDB_AlarmsRuleStruct 
PARAM_DESCRIPTION Preallocated array of rule structures to be filled.

PARAMETER 3
PARAM_NAME *NumFound
PARAM_TYPE int 
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of items found in the database..

PARAMETER 4
PARAM_NAME *NumRetrieved
PARAM_TYPE int 
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of items retrieved from the database..

PARAMETER 5
PARAM_NAME BufferLen
PARAM_TYPE int 
PARAM_DESCRIPTION Length of the input buffer - number of preallocated
structures to be filled.

DESCRIPTION If idRule is greater than 0, the function returns information
about that rule only. Otherwise it retrieves and writes to the 
pRule buffer the information about all rules. 
It will write up to the BufferLen records, then return the number of 
found and retrieved items so that the calling routine can re-allocate the 
buffer and call again.

*************************************************
************************************************/
int ewdb_api_GetAlarmsRules(int idRule, EWDB_AlarmsRuleStruct *pRule,
                            int *NumFound, int *NumRetrieved, int BufferLen);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetAlarmsRecipientList

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_GetAlarmsRecipientList.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Successful return, but there were more entries found
in the database than could be written into the supplied buffer.

PARAMETER 1
PARAM_NAME idRecipient
PARAM_TYPE int 
PARAM_DESCRIPTION Database ID of the recipient to retrieve. If present,
the function will retrieve the information for that recipient only. 
Otherwise, all recipients are retrieved.

PARAMETER 2
PARAM_NAME *pRecipients
PARAM_TYPE EWDB_AlarmsRecipientStruct 
PARAM_DESCRIPTION Preallocated array of recipient structures to be filled.

PARAMETER 3
PARAM_NAME *NumFound
PARAM_TYPE int 
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of items found in the database..

PARAMETER 4
PARAM_NAME *NumRetrieved
PARAM_TYPE int 
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of items retrieved from the database..

PARAMETER 5
PARAM_NAME BufferLen
PARAM_TYPE int 
PARAM_DESCRIPTION Length of the input buffer - number of preallocated
structures to be filled.

DESCRIPTION If idRecipient is greater than 0, the function returns information
about that recipient only. Otherwise it retrieves and writes to the 
pRecipient buffer the information about all recipients. 
It will write up to the BufferLen records, then return the number of 
found and retrieved items so that the calling routine can re-allocate the 
buffer and call again.

*************************************************
************************************************/
int ewdb_api_GetAlarmsRecipientList(int idRecipient, 
      EWDB_AlarmsRecipientStruct *pRecipients, 
      int *NumFound, int *NumRetrieved, int BufferLen);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetAlarmsRecipientSummary

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_GetAlarmsRecipientSummary.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Successful return, but there were more entries found
in the database than could be written into the supplied buffer.

PARAMETER 1
PARAM_NAME idRecipient
PARAM_TYPE int 
PARAM_DESCRIPTION Database ID of the recipient to retrieve. If present,
the function will retrieve the summary information for that recipient only. 
Otherwise, all recipients are retrieved.

PARAMETER 2
PARAM_NAME *pSummary
PARAM_TYPE EWDB_AlarmsRecipientStructSummary 
PARAM_DESCRIPTION Preallocated array of summary structures to be filled.

PARAMETER 3
PARAM_NAME *NumFound
PARAM_TYPE int 
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of items found in the database..

PARAMETER 4
PARAM_NAME *NumRetrieved
PARAM_TYPE int 
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of items retrieved from the database..

PARAMETER 5
PARAM_NAME BufferLen
PARAM_TYPE int 
PARAM_DESCRIPTION Length of the input buffer - number of preallocated
structures to be filled.

DESCRIPTION If idRecipient is greater than 0, the function returns summary
information about that recipient only. Otherwise it retrieves and writes to the 
pSummary buffer the summary information about all recipients. 
It will write up to the BufferLen records, then return the number of 
found and retrieved items so that the calling routine can re-allocate the 
buffer and call again.

*************************************************
************************************************/
int ewdb_api_GetAlarmsRecipientSummary(int idRecipient, 
      EWDB_AlarmsRecipientStructSummary *pSummary, 
      int *NumFound, int *NumRetrieved, int BufferLen);

int	ewdb_api_GetRecipientSummary (int idRecipient,
                                  EWDB_AlarmsRecipientStructSummary *pSummary, 
                                  int *NumFound, int *NumRetrieved, int BufferLen);

int		ewdb_api_GetRecipientDeliverySummary (int idRecipientDelivery,
                                          EWDB_AlarmsRecipientStructSummary *pSummary, 
                                          int *NumFound, 
                                          int *NumRetrieved, int BufferLen);

/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetCustomDeliveries

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_GetCustomDeliveries.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Successful return, but there were more entries found
in the database than could be written into the supplied buffer.

PARAMETER 1
PARAM_NAME isAudit
PARAM_TYPE int 
PARAM_DESCRIPTION If 1, audit delivery entries will be retrieved. Otherwise,
function returns actual alarm delivery entries.

PARAMETER 2
PARAM_NAME idDelivery
PARAM_TYPE int 
PARAM_DESCRIPTION Database ID of the delivery to retrieve. If present,
the function will retrieve the information for that delivery only. Otherwise,
all deliveries of this type are returned.

PARAMETER 3
PARAM_NAME *pDelivery
PARAM_TYPE EWDB_CustomDeliveryStruct 
PARAM_DESCRIPTION Preallocated array of delivery structures to be filled.

PARAMETER 4
PARAM_NAME *NumFound
PARAM_TYPE int 
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of items found in the database..

PARAMETER 5
PARAM_NAME *NumRetrieved
PARAM_TYPE int 
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of items retrieved from the database..

PARAMETER 6
PARAM_NAME BufferLen
PARAM_TYPE int 
PARAM_DESCRIPTION Length of the input buffer - number of preallocated
structures to be filled.

DESCRIPTION If idDelivery is greater than 0, the function returns delivery
information about that delivery only. Otherwise it retrieves and writes to the 
pDelivery buffer the delivery information about all deliveries. 
It will write up to the BufferLen records, then return the number of 
found and retrieved items so that the calling routine can re-allocate the 
buffer and call again.
If isAudit is set, the retrieval is done from the audit tables, instead of 
from the actual alarm tables.

*************************************************
************************************************/
int ewdb_api_GetCustomDeliveries(int isAudit, int idDelivery, 
      EWDB_CustomDeliveryStruct *pDelivery, 
      int *NumFound, int *NumRetrieved, int BufferLen);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetEmailDeliveries

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_GetEmailDeliveries.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Successful return, but there were more entries found
in the database than could be written into the supplied buffer.

PARAMETER 1
PARAM_NAME isAudit
PARAM_TYPE int 
PARAM_DESCRIPTION If 1, audit delivery entries will be retrieved. Otherwise,
function returns actual alarm delivery entries.

PARAMETER 2
PARAM_NAME idDelivery
PARAM_TYPE int 
PARAM_DESCRIPTION Database ID of the delivery to retrieve. If present,
the function will retrieve the information for that delivery only. Otherwise,
all deliveries of this type are returned.

PARAMETER 3
PARAM_NAME *pDelivery
PARAM_TYPE EWDB_EmailDeliveryStruct 
PARAM_DESCRIPTION Preallocated array of delivery structures to be filled.

PARAMETER 4
PARAM_NAME *NumFound
PARAM_TYPE int 
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of items found in the database..

PARAMETER 5
PARAM_NAME *NumRetrieved
PARAM_TYPE int 
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of items retrieved from the database..

PARAMETER 6
PARAM_NAME BufferLen
PARAM_TYPE int 
PARAM_DESCRIPTION Length of the input buffer - number of preallocated
structures to be filled.

DESCRIPTION If idDelivery is greater than 0, the function returns delivery
information about that delivery only. Otherwise it retrieves and writes to the 
pDelivery buffer the delivery information about all deliveries. 
It will write up to the BufferLen records, then return the number of 
found and retrieved items so that the calling routine can re-allocate the 
buffer and call again.
If isAudit is set, the retrieval is done from the audit tables, instead of 
from the actual alarm tables.

*************************************************
************************************************/
int ewdb_api_GetEmailDeliveries(int isAudit, int idDelivery, 
       EWDB_EmailDeliveryStruct *pDelivery, 
       int *NumFound, int *NumRetrieved, int BufferLen);


int	ewdb_api_GetEmailRecipient(int idDelivery, 
                                EWDB_EmailDeliveryStruct *pDelivery, 
                                int *NumFound, int *NumRetrieved, 
                                int BufferLen);

/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetPagerDeliveries

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_GetPagerDeliveries.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Successful return, but there were more entries found
in the database than could be written into the supplied buffer.

PARAMETER 1
PARAM_NAME isAudit
PARAM_TYPE int 
PARAM_DESCRIPTION If 1, audit delivery entries will be retrieved. Otherwise,
function returns actual alarm delivery entries.

PARAMETER 2
PARAM_NAME idDelivery
PARAM_TYPE int 
PARAM_DESCRIPTION Database ID of the delivery to retrieve. If present,
the function will retrieve the information for that delivery only. Otherwise,
all deliveries of this type are returned.

PARAMETER 3
PARAM_NAME *pDelivery
PARAM_TYPE EWDB_PagerDeliveryStruct 
PARAM_DESCRIPTION Preallocated array of delivery structures to be filled.

PARAMETER 4
PARAM_NAME *NumFound
PARAM_TYPE int 
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of items found in the database..

PARAMETER 5
PARAM_NAME *NumRetrieved
PARAM_TYPE int 
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of items retrieved from the database..

PARAMETER 6
PARAM_NAME BufferLen
PARAM_TYPE int 
PARAM_DESCRIPTION Length of the input buffer - number of preallocated
structures to be filled.

DESCRIPTION If idDelivery is greater than 0, the function returns delivery
information about that delivery only. Otherwise it retrieves and writes to the 
pDelivery buffer the delivery information about all deliveries. 
It will write up to the BufferLen records, then return the number of 
found and retrieved items so that the calling routine can re-allocate the 
buffer and call again.
If isAudit is set, the retrieval is done from the audit tables, instead of 
from the actual alarm tables.

*************************************************
************************************************/
int ewdb_api_GetPagerDeliveries(int isAudit, int idDelivery, 
                                EWDB_PagerDeliveryStruct *pDelivery, 
                                int *NumFound, int *NumRetrieved, int BufferLen);

int	ewdb_api_GetPagerRecipient(int idDelivery, 
                                EWDB_PagerDeliveryStruct *pDelivery, 
                                int *NumFound, int *NumRetrieved, 
                                int BufferLen);

/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetPhoneDeliveries

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_GetPhoneDeliveries.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Successful return, but there were more entries found
in the database than could be written into the supplied buffer.

PARAMETER 1
PARAM_NAME isAudit
PARAM_TYPE int 
PARAM_DESCRIPTION If 1, audit delivery entries will be retrieved. Otherwise,
function returns actual alarm delivery entries.

PARAMETER 2
PARAM_NAME idDelivery
PARAM_TYPE int 
PARAM_DESCRIPTION Database ID of the delivery to retrieve. If present,
the function will retrieve the information for that delivery only. Otherwise,
all deliveries of this type are returned.

PARAMETER 3
PARAM_NAME *pDelivery
PARAM_TYPE EWDB_PhoneDeliveryStruct 
PARAM_DESCRIPTION Preallocated array of delivery structures to be filled.

PARAMETER 4
PARAM_NAME *NumFound
PARAM_TYPE int 
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of items found in the database..

PARAMETER 5
PARAM_NAME *NumRetrieved
PARAM_TYPE int 
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of items retrieved from the database..

PARAMETER 6
PARAM_NAME BufferLen
PARAM_TYPE int 
PARAM_DESCRIPTION Length of the input buffer - number of preallocated
structures to be filled.

DESCRIPTION If idDelivery is greater than 0, the function returns delivery
information about that delivery only. Otherwise it retrieves and writes to the 
pDelivery buffer the delivery information about all deliveries. 
It will write up to the BufferLen records, then return the number of 
found and retrieved items so that the calling routine can re-allocate the 
buffer and call again.
If isAudit is set, the retrieval is done from the audit tables, instead of 
from the actual alarm tables.

*************************************************
************************************************/
int ewdb_api_GetPhoneDeliveries(int isAudit, int idDelivery, 
                                EWDB_PhoneDeliveryStruct *pDelivery, 
                                int *NumFound, int *NumRetrieved, int BufferLen);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetQddsDeliveries

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_GetQddsDeliveries.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Successful return, but there were more entries found
in the database than could be written into the supplied buffer.

PARAMETER 1
PARAM_NAME isAudit
PARAM_TYPE int 
PARAM_DESCRIPTION If 1, audit delivery entries will be retrieved. Otherwise,
function returns actual alarm delivery entries.

PARAMETER 2
PARAM_NAME idDelivery
PARAM_TYPE int 
PARAM_DESCRIPTION Database ID of the delivery to retrieve. If present,
the function will retrieve the information for that delivery only. Otherwise,
all deliveries of this type are returned.

PARAMETER 3
PARAM_NAME *pDelivery
PARAM_TYPE EWDB_QddsDeliveryStruct 
PARAM_DESCRIPTION Preallocated array of delivery structures to be filled.

PARAMETER 4
PARAM_NAME *NumFound
PARAM_TYPE int 
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of items found in the database..

PARAMETER 5
PARAM_NAME *NumRetrieved
PARAM_TYPE int 
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of items retrieved from the database..

PARAMETER 6
PARAM_NAME BufferLen
PARAM_TYPE int 
PARAM_DESCRIPTION Length of the input buffer - number of preallocated
structures to be filled.

DESCRIPTION If idDelivery is greater than 0, the function returns delivery
information about that delivery only. Otherwise it retrieves and writes to the 
pDelivery buffer the delivery information about all deliveries. 
It will write up to the BufferLen records, then return the number of 
found and retrieved items so that the calling routine can re-allocate the 
buffer and call again.
If isAudit is set, the retrieval is done from the audit tables, instead of 
from the actual alarm tables.

*************************************************
************************************************/
int ewdb_api_GetQddsDeliveries(int isAudit, int idDelivery, 
                               EWDB_QddsDeliveryStruct *pDelivery, 
                               int *NumFound, int *NumRetrieved, int BufferLen);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetAndIncrementCubeVersion

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_GetAndIncrementCubeVersion.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Successful return, but there were more entries found
in the database than could be written into the supplied buffer.

PARAMETER 1
PARAM_NAME idEvent
PARAM_TYPE int 
PARAM_DESCRIPTION Database ID of the event for which CUBE
version number is retrieved.

PARAMETER 2
PARAM_NAME VersionNumber
PARAM_TYPE int *
PARAM_DESCRIPTION Pointer to the integer value representing the
CUBE version number being returned.

DESCRIPTION Returns the current CUBE version number for the 
event and increments the number by one.

*************************************************
************************************************/
int ewdb_api_GetAndIncrementCubeVersion (int idEvent, int *VersionNumber);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_UpdateRulePolygons

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_UpdateRulePolygons.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

PARAMETER 1
PARAM_NAME idOldPolygon
PARAM_TYPE EWDBid 
PARAM_DESCRIPTION Database ID of the polygon that is being replaced

PARAMETER 2
PARAM_NAME idNewPolygon
PARAM_TYPE EWDBid
PARAM_DESCRIPTION Database ID of the new polygon.

DESCRIPTION Searches the DB for any rules that include idOldPolygon, and replaces them
		with idNewPolygon.  Call this function when a polygon is redrawn in the DB, prior
		to deleting the old polygon.

*************************************************
************************************************/
int ewdb_api_UpdateRulePolygons(EWDBid idOldPolygon, EWDBid idNewPolygon);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_CreateAlarmsGroup

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_CreateAlarmsGroup.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

PARAMETER 1
PARAM_NAME pGroup
PARAM_TYPE EWDB_AlarmsGroupStruct * 
PARAM_DESCRIPTION Pointer to a valid group struct

DESCRIPTION Creates a new alarm group in the DB.

*************************************************
************************************************/
int ewdb_api_CreateAlarmsGroup (EWDB_AlarmsGroupStruct *pGroup);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_CreateGroupRecipient

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_CreateGroupRecipient.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

PARAMETER 1
PARAM_NAME pGroupRecipient
PARAM_TYPE EWDB_AlarmsGroupRecipientStruct * 
PARAM_DESCRIPTION Pointer to a valid group recipient struct

DESCRIPTION Creates a new group recipient in the DB, associated with an existing group.

*************************************************
************************************************/
int ewdb_api_CreateGroupRecipient(EWDB_AlarmsGroupRecipientStruct *pGroupRecipient);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_DeleteGroup

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_DeleteGroup.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

PARAMETER 1
PARAM_NAME idGroup
PARAM_TYPE EWDBid 
PARAM_DESCRIPTION DB id of the group to delete.

DESCRIPTION Deletes a group from the DB.

*************************************************
************************************************/
int ewdb_api_DeleteGroup(EWDBid idGroup);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_DeleteGroupRecipient

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_DeleteGroupRecipient.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

PARAMETER 1
PARAM_NAME idGroup
PARAM_TYPE EWDBid 
PARAM_DESCRIPTION DB id of the group whose recipient is to be deleted.

PARAMETER 2
PARAM_NAME idRecipientDelivery
PARAM_TYPE EWDBid 
PARAM_DESCRIPTION DB id of the recipient's delivery that will be deleted.

DESCRIPTION Disassociates a group recipient from the DB group.

*************************************************
************************************************/
int ewdb_api_DeleteGroupRecipient(EWDBid idGroup, EWDBid idRecipientDelivery);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetAlarmsGroupList

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_GetAlarmsGroupList.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Successful return, but there were more entries found
in the database than could be written into the supplied buffer.

PARAMETER 1
PARAM_NAME idGroup
PARAM_TYPE int 
PARAM_DESCRIPTION Database ID of the group to retrieve. If present,
the function will retrieve the information for that group only. Otherwise,
all groups are returned.

PARAMETER 2
PARAM_NAME *pGroups
PARAM_TYPE EWDB_AlarmsGroupStruct *
PARAM_DESCRIPTION Preallocated array of group structures to be filled.

PARAMETER 3
PARAM_NAME *NumFound
PARAM_TYPE int 
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of items found in the database..

PARAMETER 4
PARAM_NAME *NumRetrieved
PARAM_TYPE int 
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of items retrieved from the database..

PARAMETER 5
PARAM_NAME BufferLen
PARAM_TYPE int 
PARAM_DESCRIPTION Length of the input buffer - number of preallocated
structures to be filled.

DESCRIPTION If idGroup is greater than 0, the function returns group
information about that group only. Otherwise it retrieves and writes to the 
pGroups buffer the group information about all groups. 
It will write up to the BufferLen records, then return the number of 
found and retrieved items so that the calling routine can re-allocate the 
buffer and call again.

*************************************************
************************************************/
int	ewdb_api_GetAlarmsGroupList(int idGroup, 
                                    EWDB_AlarmsGroupStruct *pGroups, 
                                    int *NumFound, int *NumRetrieved, 
                                    int BufferLen);


/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_GetAlarmsGroupRecipients

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_GetAlarmsGroupRecipients.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

RETURN_VALUE EWDB_RETURN_WARNING
RETURN_DESCRIPTION Successful return, but there were more entries found
in the database than could be written into the supplied buffer.

PARAMETER 1
PARAM_NAME idGroup
PARAM_TYPE int 
PARAM_DESCRIPTION Database ID of the group whose recipients are to be retrieved.

PARAMETER 2
PARAM_NAME *pGroupRecipients
PARAM_TYPE EWDB_AlarmsGroupRecipientStruct *
PARAM_DESCRIPTION Preallocated array of group structures to be filled.

PARAMETER 3
PARAM_NAME *NumFound
PARAM_TYPE int 
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of items found in the database..

PARAMETER 4
PARAM_NAME *NumRetrieved
PARAM_TYPE int 
PARAM_DESCRIPTION Pointer to an integer where the function will write
the number of items retrieved from the database..

PARAMETER 5
PARAM_NAME BufferLen
PARAM_TYPE int 
PARAM_DESCRIPTION Length of the input buffer - number of preallocated
structures to be filled.

DESCRIPTION Retrieves all the recipients associated with an alarm group. 
It will write up to the BufferLen records, then return the number of 
found and retrieved items so that the calling routine can re-allocate the 
buffer and call again.

*************************************************
************************************************/
int	ewdb_api_GetAlarmsGroupRecipients(int idGroup, 
                                    EWDB_AlarmsGroupRecipientStruct *pGroupRecipients, 
                                    int *NumFound, int *NumRetrieved, 
                                    int BufferLen);

/************************************************
************ SPECIAL FORMATTED COMMENT **********
TYPE FUNCTION_PROTOTYPE

LIBRARY EWDB_API_LIB

SUB_LIBRARY ALARMS_API

LANGUAGE C

LOCATION THIS_FILE

FUNCTION ewdb_api_WipeAllAlarmsInfo

STABILITY NEW

SOURCE_LOCATION alarms/ewdb_api_WipeAllAlarmsInfo.c

RETURN_TYPE int 

RETURN_VALUE EWDB_RETURN_SUCCESS
RETURN_DESCRIPTION Success.

RETURN_VALUE EWDB_RETURN_FAILURE
RETURN_DESCRIPTION Failure.  See logfile for details.

DESCRIPTION Deletes all alarms-related information from the database:  recipients,
	deliveries, rules, polygons, groups, audits, you name it.

WARNING!!!  It is assumed that you know exactly what you're doing before you call this!
	What you should be doing is removing all old alarms info so that new info can be
	imported.  Otherwise, Ray will hold you down while Harley gives you a wedgie.

*************************************************
************************************************/
int ewdb_api_WipeAllAlarmsInfo();

#endif /* _EWDB_ORA_API_ALARMS_H */
