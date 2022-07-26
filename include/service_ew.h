/*
 *   THIS FILE IS UNDER CVS - DO NOT MODIFY UNLESS YOU HAVE
 *   CHECKED IT OUT USING THE COMMAND CHECKOUT.
 *
 *    $Id: service_ew.h 1861 2005-06-08 17:39:31Z mark $
 *
 *    Revision history:
 *     $Log$
 *     Revision 1.6  2005/06/08 17:39:31  mark
 *     Added wait_for_dependencies
 *
 *     Revision 1.5  2004/12/06 22:13:55  mark
 *     Added get_service_status
 *
 *     Revision 1.4  2004/08/03 18:47:20  mark
 *     Added dependencies param to install_service
 *
 *     Revision 1.3  2004/07/27 20:05:15  mark
 *     Fixed compile errors/warnings
 *
 *     Revision 1.2  2004/07/20 16:48:34  mark
 *     Added #include <winsvc.h> for Win32 projects
 *
 *     Revision 1.1  2004/07/13 16:21:27  mark
 *     Initial checkin
 *
 */

#ifndef _SERVICE_EW_H_
#define _SERVICE_EW_H_

#ifndef WIN32
/*******
 * Windows constants used for services.  These were taken from winsvc.h
 ******/

#define SERVICE_STOPPED                0x00000001
#define SERVICE_START_PENDING          0x00000002
#define SERVICE_STOP_PENDING           0x00000003
#define SERVICE_RUNNING                0x00000004
#define SERVICE_CONTINUE_PENDING       0x00000005
#define SERVICE_PAUSE_PENDING          0x00000006
#define SERVICE_PAUSED                 0x00000007

#define SERVICE_CONTROL_STOP           0x00000001
#define SERVICE_CONTROL_PAUSE          0x00000002
#define SERVICE_CONTROL_CONTINUE       0x00000003
#define SERVICE_CONTROL_INTERROGATE    0x00000004
#define SERVICE_CONTROL_SHUTDOWN       0x00000005
#define SERVICE_CONTROL_PARAMCHANGE    0x00000006
#define SERVICE_CONTROL_NETBINDADD     0x00000007
#define SERVICE_CONTROL_NETBINDREMOVE  0x00000008
#define SERVICE_CONTROL_NETBINDENABLE  0x00000009
#define SERVICE_CONTROL_NETBINDDISABLE 0x0000000A

typedef void (*LPSERVICE_MAIN_FUNCTION)(
    int   dwNumServicesArgs,
    char   *lpServiceArgVectors[]
    );

typedef void (*LPHANDLER_FUNCTION)(
    int    dwControl
    );

#else
#include <winsvc.h>
#endif

/*******************
 * install_service
 *
 * Installs this app as a service.  This only needs to be called once, period;
 * not once per time you run.  After this is called, the service will be set
 * to start automatically at every reboot.
 *
 * Params:
 *  szServiceName - internal name given to this service
 *  szDisplayName - name displayed in the Service Manager
 *  szDependencies - double-NULL terminated array of NULL-terminated names of other services
 *                   that this service depends on, or NULL for no dependencies.
 ******************/
void install_service(const char *szServiceName, const char *szDisplayName, const char *szDependencies);

/*******************
 * uninstall_service
 *
 * Uninstalls this app as a service.
 *
 * Params:
 *	szServiceName - internal name given to this service, used during install_service
 ******************/
void uninstall_service(const char *szServiceName);

/******************
 * start_service
 *
 * Starts running the service.  This function should be called within the main() function.
 * Its purpose it to register this app as running with the Windows Service Manager, then
 * call another function which will act as a second main() function.
 *
 * Params;
 *	szServiceName - internal name given to this service, used during install_service
 *	mainfunc - new "main" function.
 * Returns:
 *	0 if all is well, otherwise an error value.  This function will not return until the
 *	"mainfunc" function has completed and returned.
 *****************/
int start_service(char *szServiceName, LPSERVICE_MAIN_FUNCTION mainfunc);

/******************
 * init_service
 *
 * Initializes the service.  This function should be called towards the beginning of
 * the mainfunc() function passed to start_service.
 *
 * Params:
 *	szServiceName - internal name given to this service, used during install_service
 *					and start_service
 *	allow_pause - non-zero if this service can pause/continue; zero if it can't
 *	handler - function that is called when the Windows Service Manager needs to issue
 *				STOP, PAUSE, or INTERROGATE commands
 * Returns:
 *	0 if all is well, otherwise an error value.
 ******************/
int init_service(char *szServiceName, int allow_pause, LPHANDLER_FUNCTION handler);

/******************
 * set_service_status
 *
 * Informs the Windows Service Manager (and anybody else who's listening via the WSM)
 * the current state of the service.  See the Win32 SetServiceStatus() function for
 * more details.
 *
 * Params:
 *	state - current state of the service (e.g. SERVICE_RUNNING), or 0 to re-send the
 *			last known state.
 *	error - Win32 error to report, or 0 if everything is fine
 *	checkpoint - current checkpoint for _PENDING states.  Start with a 0 value when you
 *				enter one of these states, then increment by 1 with subsequent set_service_status
 *				calls until you're no longer pending.
 *	hint - how long the Service Manager can expect to wait until you're no longer pending,
 *			or until the next "checkpoint" status is sent (in ms).
 * Returns:
 *	0 if all is well, otherwise an error value.
 ******************/
int set_service_status(int state, int error, int checkpoint, int hint);

/******************
 * get_service_status
 *
 * Determines the status of a given service.  This can be used to determine if all service
 * dependencies are running.
 *
 * Params:
 *	szServiceName - internal name given to this service, used during install_service
 *					and start_service (does not have to have registered as an earthworm/hydra
 *					service, but can refer to any service)
 *  pState - return value referring to current service state (e.g. SERVICE_RUNNING)
 *  pError - return value referring to current service error value, or 0 if all is well.
 *
 * Returns:
 *	0 if all is well, otherwise an error value.
 ******************/
int get_service_status(char *szServiceName, int *pState, int *pError);

/******************
 * wait_for_dependencies
 *
 * Waits for all of the given dependencies to start running.  This is used when a service has
 * more than one dependency, since Windows only waits for one service to start running rather
 * than all of them.
 *
 * Params:
 *	szDependencyList - List of dependant services given to install_service; each service name
 *			is NULL terminated, and the entire array is double-NULL terminated.
 *  pCheckpoint -	On entry, the next checkpoint to pass to set_service_status.
 *					On exit, the next checkpoint for the calling function to pass to
 *						set_service_status.
 *
 * Returns:
 *	0 if all is well, otherwise an error value.
 ******************/
int wait_for_dependencies(char *szDependencyList, int *pCheckpoint);

#endif /* _SERVICE_EW_H_ */
