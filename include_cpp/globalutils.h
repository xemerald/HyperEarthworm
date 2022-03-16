/*
** TGlobalUtils is a class that handles program-global variables and such.
**
**
**   Specifically, it provides program-global access to:
**
**     1. System-wide defines from earthworm_global.d
**     2. Site-specific defines from earthworm.d
**     3. Program-wide globals from the <program>.d file
**     4. Program name and module id (from the Initialize method).
**
** CONFIGURATION FILES
**
**   The two configuration files are parsed the first time any of the
**   content items are queried through the static methods.
**
**    earthworm_global.d -- world-wide [sacred] system variables
**                     Ring       - ring name to key id mapping{s)
**                     Module     - module name to id mapping(s)
**                     Message    - message type name to id mapping(s)
**    earthworm.d -- system variables
**                     LogDir     - Log directory
**                     LogLevel   - site-global logging level (1 - 9)
**                                  (over-ridden by <module>.d)
**
**    <module>.d -- module-specific parameters
**                     WriteLogFile - flag entry, if present write log for module
**                     LogLevel     - module-specific logging level (1 - 9)
**                                    (overrides magworm_site.d)
**
*/
//---------------------------------------------------------------------------
#ifndef GlobalUtilsH
#define GlobalUtilsH
//---------------------------------------------------------------------------
#include <configurable.h>
//#include <worm_environ.h> -- in globalutils.cpp
//#include <logger.h> -- in globalutils.cpp
#include <process.h> // getpid

// microsoft pragma to avoid warnings from STL
#pragma warning(disable:4786)

#include <string>
#include <vector>
#include <worm_types.h>
#include <worm_defs.h>
#include <worm_statuscode.h>
#include <configurable.h>


//---------------------------------------------------------------------------
class TGlobalUtils : public TConfigurable
{
private:

   /*
   **  See worm_environ.h for environment variables pertaining to
   **                     global and site configuration files.
   */
   static std::vector<std::string> ConfigFiles;
   static int                      ConfigFileCount;

   /*
   ** GetUtil_LoadTable
   **
   ** Reads configuration files using TComFileParser, passes each line to ParseCommandLine().
   ** Calls CheckConfig() at end.
   ** Exits on some errors, many merely reported (to enable complete file parsing).
   */
   void LoadFiles();

protected:

   // program identity
   static PROGRAM_NAME ProgramName;
   static WORM_MODULE_ID ThisModuleId;
   static WORM_INSTALLATION_ID ThisInstallation;

   static LOG_DIRECTORY HomeDirectory;
   static char Version[12];

   // program settings
   static bool DoLogFile;
   static WORM_LOGGING_LEVEL LogLevel;

   static long HeartBeatInt;

   // flag for all module threads to check for termination
   static volatile bool TerminateFlag;

   // lookup containers
   static INSTALLATION_MAP InstallIds;
   static RING_MAP RingIds;
   static MODULE_MAP ModuleIds;
   static MESSAGETYPE_MAP MessageTypeIds;

   // ParseCommand() -- allows derivative classes to extend the lookup values.
   //                   Called called from LoadFiles().
   //
   // From within any deriving class, or further derivation, ALWAYS make a call to
   // <super_class>::ParseCommand() the first statement in the ParseCommand() method...
   // this ensures that all classes in the heirarchy get their shot at the parameter,
   // starting from the base-most class.
   //
   virtual bool ParseLookupLine( const char * p_filename, ConfigSource & p_parser );

   // CheckConfig() -- allows derivative classes to report the status of their the lookup values.
   //
   // From within any deriving class, or further derivation, ALWAYS contain a call to
   // <super_class>::CheckConfig() in their own CheckConfig() method...
   // this ensures that all classes in the heirarchy get their chance to report status.
   //
   // All implementations should set ConfigStatus value to WORM_STAT_BADSTATE if there
   // is a configuration problem, otherwise leave it alone.
   //
   virtual void CheckConfig();


public:

   TGlobalUtils( char* p_programname );

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
   */
   HANDLE_STATUS HandleConfigLine( ConfigSource * p_parser );


   static void SetTerminateFlag() { TerminateFlag = true; }
   static bool GetTerminateFlag() { return TerminateFlag; }

   /*
   ** GetEnvironmentValue() -- calls getenv(), can return NULL
   */
   static char* GetEnvironmentValue( const char* );

   static char* GetHomeDirectory() { return HomeDirectory; }

   static char* GetVersion() { return Version; }

   static const char* GetProgramName() { return ProgramName; }

   static unsigned int GetPID() { return getpid(); }

   static WORM_MODULE_ID GetThisModuleId() { return ThisModuleId; }

   static WORM_INSTALLATION_ID GetThisInstallationId() { return ThisInstallation; }

   static void SetFileLoggingState( bool p_newstate )
   {
      DoLogFile = p_newstate;
   }

   static bool WriteLogFile() { return DoLogFile; }

   static WORM_LOGGING_LEVEL GetLoggingLevel() { return LogLevel; }

   static long GetHeartbeatInt() { return HeartBeatInt; }

   /*
   ** LookupInstallationId()
   **
   **  Convert installation key to id number using table defined in ConfigFile
   **  RETURNS: 0 to n = installation id
   **                0 = specified installation name is unknown
   */
   static const WORM_INSTALLATION_ID LookupInstallationId( const char* p_name );

   /*
   ** LookupModuleId()
   **
   **  Convert module name to modid number using table defined in ConfigFile
   **  RETURNS: 0 to n = module id
   **                0 = specified module name is unknown
   */
   static const WORM_MODULE_ID LookupModuleId( const char* p_name );

   /*
   ** LookupMessageTypeId()
   **
   ** Convert message-type name to number using table defined in ConfigFile
   **  RETURNS: 0 to n = message type id
   **                0 = specified message type name is unknown
   */
   static const WORM_MSGTYPE_ID LookupMessageTypeId( const char* p_name );

   /*
   ** LookupRingKey()
   **
   **  Convert ring name to key number using table defined in ConfigFile
   **  RETURNS: 0 to n = key number
   **                0 = specified ring name is unknown
   */
   static const WORM_RING_ID LookupRingKey( const char* p_name );

};

#endif
 