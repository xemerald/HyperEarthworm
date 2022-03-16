/*
 * dbmutableserverbase.h -- Extends the MutableServerBase class to add
 *                          database access.
 * 
 * CONFIGURATION (.d) FILE:
 * 
 *        DBConnection  User Password Service
 */
 
#if ! defined(_MSM_DBServer_H)
#define _MSM_DBServer_H


#include "mutableserverbase.h"
#include <string>  // std::string

extern "C" {

#include <ewdb_ora_api.h>
#include <ewdb_cli_base.h>

}

class DBMutableServer : public MutableServerBase
{
protected:

   // =======================================================================
   //                  from TConfigurable
   // =======================================================================

   /* CheckConfig() -- allows derivative classes to report the status of their
   **                  the configuration state.
   **
   ** Set ConfigStatus = WORM_STAT_BADSTATE if there
   ** is a configuration problem, otherwise take no action.
   */
   void CheckConfig();


   // =======================================================================
   //                  from WormServerBase
   // =======================================================================

   // PrepareToRun() -- actions to take prior to entering main loop
   // 
   //   ALWAYS call base class's PrepareToRun() at the top
   //
   // RETURN:  true if ready
   //          false if some condition prevents proper execution
   //
   bool PrepareToRun();


   // =======================================================================
   //                  for DBMutableServer
   // =======================================================================


   // Database connection parameters
   //
   std::string DB_User
             , DB_Password
             , DB_Service
             ;

   // InitializeDB() -- Initializes the database connection, using
   //                   DB_User, DB_Password, DB_Service 
   //                   Called within this class's PrepareToRun() method.
   //
   // RETURNS:
   //       true  = success
   //      false  = error
   //
   bool InitializeDB();

   // GetDefaultsFromDB() -- Gets default parameter lines from the database,
   //                        passes each line, along with a pointer (to the
   //                        default parameters struct) to HandleParameterLine()
   //
   // RETURNS:
   //       true  = success
   //      false  = error
   //
   bool GetDefaultsFromDB( void * p_parmstruct );

   
   // HandleParameterLine() -- handle configuration lines which may be either
   //                          lines from a .d file, or passport lines from
   //                          the database.
   // 
   // PARAMETERS:
   //       p_parser = pointer to a ComFile or OracleConfigSource object
   //       p_params = pointer to either a struct holding default parameters,
   //                  or to a struct that is being used to hold parameters
   //                  for an arriving processing request.
   // 
   // RETURNS:
   //          HANDLE_INVALID --  line invalid
   //          HANDLE_UNUSED  --  line not used
   //          HANDLE_USED    --  line used okay
   //
   virtual HANDLE_STATUS HandleParameterLine( ConfigSource * p_parser
                                            , void         * p_params
                                            ) = 0;



public:


   // =======================================================================
   //                  from TConfigurable
   // =======================================================================

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
   **  Call <super_class::HandleConfigLine() within derivative classes
   **  to allow them a whack at the parameter as well.
   */
   HANDLE_STATUS HandleConfigLine( ConfigSource * p_parser );


};

#endif // _MSM_DBServer_H