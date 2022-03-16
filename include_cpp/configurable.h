/*
** Configurable.h
**
** TConfigurable is an abstract base class for classes that must receive
**               and process configuration file lines.  That includes
**               virtually all module internal objects, however, it is
**               not absolute that all such objects will handle the lines.
*/
//---------------------------------------------------------------------------
#ifndef _WORM_CONFIGURABLE_H
#define _WORM_CONFIGURABLE_H
//---------------------------------------------------------------------------
#include <worm_defs.h>
#include <worm_statuscode.h>
#include <configsource.h>


class TConfigurable
{
protected:
   /* Rather than requiring the implementing class to maintain its own state
   ** flag element, store it in this base class.
   ** Rather than a bool, using a WORM_STATUS_CODE to enable the state to be
   **    WORM_STAT_NOTINIT   = IsReady() not yet called
   **    WORM_STAT_BADSTATE  = one of the derivative classes found an error
   ** or WORM_STAT_SUCCESS   = none of the derivative classes found an error
   **
   ** Implemented as a global variable to simplify state checking across
   ** multiple objects.
   ** Essentially, configuration state should only be checked one time per
   ** program execution.
   */
   WORM_STATUS_CODE ConfigState;

   /* CheckConfig() -- allows derivative classes to report the status of their
   **                  the lookup values.
   **
   ** From within any deriving class, or further derivation, ALWAYS contain a call to
   ** <super_class>::CheckConfig() in their own CheckConfig() method...
   ** this ensures that all classes in the heirarchy get their chance to report status.
   **
   ** All implementations should set ConfigStatus value to WORM_STAT_BADSTATE if there
   ** is a configuration problem, otherwise leave it alone.
   */
   virtual void CheckConfig() { };

public:
   TConfigurable();

   /*
   **  HandleConfigLine()
   **
   **  PARMS:
   **          p_parser -- the parser being used, command string already
   **                      in the current token for comparison with Its()
   **
   ** RETURN:
   **          HANDLE_INVALID --  line invalid
   **          HANDLE_UNUSED  --  line not used
   **          HANDLE_USED    --  line used okay
   **
   **  Override for child classes to handle command lines
   **
   */
   virtual HANDLE_STATUS HandleConfigLine( ConfigSource * p_parser );

   /*  IsReady() -- is the internal element configured and ready to use?
   **
   ** RETURNS:  true | false
   **
   ** Override CheckConfig() rather than the IsReady() method.
   */
   bool IsReady();
};


#endif

