/*
 * server_template.h -- Template and server class deriving from
 *                      the MutableServerBase class.
 *
 *                      The base class handles communications, so this class
 *                      primarily must only concern itself with the specific
 *                      handling for this type of server (to include relevant
 *                      configuration, passports and calculations.
 */
 
// STEP 1: Do a global replacement of the string "ServerTemplate"
//         with the name of your new class
//
#if ! defined(_MSM_ServerTemplate_H)
#define _MSM_ServerTemplate_H


// IF THE NEW SERVER WILL NOT NEED DATABASE CONNECTIVITY, THIS MAY BE
// CHANGED TO <mutableserverbase.h>
#include <dbmutableserver.h>


// STEP 2: Declare a structure type to contain configuration parameters
//         that may change due to arriving passport data.
//
typedef _ServerTemplate_PARAMS
{
   // These are just examples used in the source
   long      ExampleLongParam;
   char      ExampleStringParam[31];
   bool      ExampleFlagParam;
} ServerTemplate_PARAMS;


// IF THE NEW SERVER WILL NOT NEED DATABASE CONNECTIVITY, THE PARENT CLASS
// MAY BE CHANGE TO MutableServerBase
class ServerTemplate : public DBMutableServer
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
   //                  from MutableServerBase
   // =======================================================================

// STEP 3: Replace the TYPE_XXXXXXX with the appropriate Earthworm message type
//         that will be returned if this server runs in module mode.
//
   // OutputMessageTypeKey() -- Returns the string used to identify 
   //                           result messages sent in Module mode
   //                           via the ring (e.g.:  TYPE_XXXXXXX).
   //                           This is intended to allow different
   //                           derivative servers to emit different
   //                           message types.
   //
   const char * OutputMessageTypeKey() { return "TYPE_XXXXXXX"; }

// STEP 4: Set an appropriate size for the socket buffer.
//
   /*
   ** GetMaxSocketBufferSize() -- Server mode uses this to allocate
   **                             a buffer of sufficient size to
   **                             parse the arriving and format
   **                             the outgoing message
   */
   long GetMaxSocketBufferSize() { return 1024L; }
   

   /*
    * GetRequestContainer(), 
    *  GetResultContainer() -- gets a container to hold the request
    *                          (e.g. passport & event info) or result info.
    *                          Since we don't know what all
    *                          possible content that can be in a
    *                          request or returned for the various derivative
    *                          classes of servers, the container
    *                          is returned as a base class pointer,
    *                          This base class code does not need
    *                          to know about the container, only
    *                          the following virtual methods do.
    *
    * RETURNS:
    *      a pointer to the container
    *   or NULL on failure
    */
   MutableServerRequest * GetRequestContainer();

   MutableServerResult * GetResultContainer();


   // - - - - - - - - - - - - - - - - - - - - - - - - -
   // For Standalone or Client modes
   //
   // Gets request parameters/instructions from command line args
   // or stdin
   //
   // r_container = a pointer to the appropriate type of
   //               MutableServerRequest derivative class
   //               (cast to the specific type in the method).
   //
   // RETURN:
   //     WORM_STAT_SUCCESS
   //     WORM_STAT_FAILURE
   //
   WORM_STATUS_CODE GetRequestFromInput( int    p_argc
                                       , char * p_argv[]
                                       , void * r_container
                                       );
   
   // - - - - - - - - - - - - - - - - - - - - - - - - -
   // For Server, Module or Standalone modes
   //
   // The actual processing
   //
   // PARAMETERS:
   //
   //      p_requestcontainer = pointer to a request object
   //                           appropriate for this server class
   //
   //      r_resultcontainer = pointer to an object of a type
   //                          derived from the MutableServerResult class
   //                          which is appropriate for this type of server.
   //                          (cast to the specific type in the method).
   //
   // RETURN:
   //      WORM_STAT_SUCCESS  = good results
   //      WORM_STAT_BADSTATE = algorithmic, not system, failure (no results obtained)
   //      WORM_STAT_FAILURE  = system failure
   //
   WORM_STATUS_CODE ProcessRequest( void * p_requestcontainer
                                  , void * r_resultcontainer
                                  );


   // - - - - - - - - - - - - - - - - - - - - - - - - -
   // For Client, Module or Standalone modes
   //
   // What to do with the results;
   //
   // PARAMETERS:
   //
   //      p_resultcontainer = pointer to an object of a type
   //                          derived from the MutableServerResult class
   //                          which is appropriate for this type of server.
   //                          (cast to the specific type in the method).
   //
   // MUST RETURN:
   //      WORM_STAT_SUCCESS  = good results
   //      WORM_STAT_BADSTATE = algorithmic, not system, failure (no results obtained)
   //      WORM_STAT_FAILURE  = system failure
   //
   WORM_STATUS_CODE HandleResult( void * p_resultcontainer );


   // =======================================================================
   //                  from DBMutableServerBase
   // =======================================================================
   
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
   HANDLE_STATUS HandleParameterLine( ConfigSource * p_parser
                                    , void         * p_params
                                    );



   // =======================================================================
   //                  for ServerTemplate
   // =======================================================================

   // Structure to hold the default parameters
   //
   ServerTemplate_PARAMS   DefaultParameters;


public:


   // =======================================================================
   //                  from TConfigurable
   // =======================================================================

   /*
    *  HandleConfigLine() -- allows programs to handle configuration in a
    *                        consistent manner.
    *
    *  PARMS:
    *          p_parser -- Pointer to some type of configuration source object,
    *                      such as ComFile or OracleConfigSource.  The object
    *                      will have a command string already in the current
    *                      token for comparison using the Its() method.
    *
    * RETURN:
    *          HANDLE_INVALID --  line invalid
    *          HANDLE_UNUSED  --  line not used
    *          HANDLE_USED    --  line used okay
    *
    *  Override for child classes to handle parameter lines
    *  Call <super_class::HandleConfigLine() within derivative classes
    *  to allow them a whack at the parameter as well.
    */
   HANDLE_STATUS HandleConfigLine( ConfigSource * p_parser );


   // =======================================================================
   //                  for ServerTemplate
   // =======================================================================

   ServerTemplate();
   
   ~ServerTemplate();
   

};

#endif // _MSM_ServerTemplate_H