/*
 * result_template.h -- Template for a result message class for use
 *                      in the mutable server model.
 *
 *                      This template class derives from the MutableServerResult
 *                      class, and as such already knows how to handle an
 *                      status value of type int.
 * 
 *                      Variables or structs needed to contain other return
 *                      values must be declared herein.
 */
 
// STEP 1: Do a global replacement of the string "ResultTemplate"
//         with the name of your new class
//
#if ! defined(_MSM_ResultTemplate_H)
#define _MSM_ResultTemplate_H


// STEP 2: If the new class should be derived from a class other than
//         MutableServerResult, replace this with the appropriate include.
//         This specific include requires the project make file to have
//         /earthworm/..../src/libsrc_cpp/servers/messaging
//         in the include path.
#include <mutableserverresult.h>



// STEP 3: If the new class should be derived from a class other than
//         MutableServerResult, replace it with the appropriate
//         base class name.
class ResultTemplate : public MutableServerResult
{
protected:


   // ----------------------------------------------------------
   //     virtual methods from MutableServerMessage
   //     to be implemented for a complete derivative class.
   //
   // THAT IS: These methods were declared in a base class (as
   //          an interface), but they must be redeclared here
   //          and implemented in request_template.cpp
   //          to be able to use the final class.
   //
   // NONE OF THIS NEEDS TO BE CHANGED, SEARCH FOR THE STRING
   // ResultTemplate TO LOCATE THE FIRST ITEMS TO CHANGE.
   // ----------------------------------------------------------

   // BufferInitAlloc -- when preparing to format a message
   //                    using FormatBuffer(), this is
   //                    first called.  The result is,
   //                    for the first call, MessageBuffer
   //                    will be assured to be at least this
   //                    long.
   //                    Overriding this method allows
   //                    derivative classes to minimize
   //                    reallocations as bits are appended
   //                    to the message buffer;
   //
   // NOTE: In each derivative implementation,
   //       call <super_class>::BufferInitAlloc() to get the
   //       space needed by that super class (and all baser
   //       classes), then add the size needed by the
   //       derivative to arrive at the total size needed.
   //       The 10 specified here is for the message-
   //       terminating '\n' and a slight overrun space.
   //
   long BufferInitAlloc();


   // FormatDerivativeData
   //
   //     Method by which derivative classes append
   //     their content to MessageBuffer.
   //
   //  NOTE: ALWAYS CALL <super_class>::FormatDerivativeData()
   //        at the start of each implementation to allow
   //        base classes to append their stuff (if any)
   //        to the buffer.
   //        Thus the buffer is built upwards from the base-most
   //        class to the most-derived class.
   //        Generally, a newline is used to separate lines.
   //
   // THROW worm_exception for errors
   //
   //       (The worm_exception class is included through
   //        some base class.  Example of use is in the cpp file)
   //
   void FormatDerivativeData();


   // ParseDerivativeData
   //
   //     Method by which derivative classes extract
   //     their content to MessageBuffer.
   //
   //  NOTE: Always call <super_class>::ParseDerivativeData()
   //        at the top of each implementation to allow
   //        base classes to get their data out of the
   //        buffer first.
   //
   //  USAGE:
   //
   //     If parsing a multi-line message ('\n'-terminated lines),
   //     use a loop consisting of:
   //
   //       i = MessageBuffer.find("\n")  to find the first line end,
   //       MessageBuffer.substr(0, i)    to extract the string
   //                                        (excluding the '\n')
   //       MessageBuffer.erase(0, i+1)     to remove that portion
   //
   //     Since the message should be terminated by an additional
   //     '\n', when the string returned in the second step
   //     is of zero length, that is the end of the message.
   //
   //     (If find() does not, it returns MessageBuffer.npos)
   //
   // THROW worm_exception for errors
   //
   void ParseDerivativeData();

   // ----------------------------------------------------------
   //             for MutableServerResult class
   // ----------------------------------------------------------

   // Only Status code-related items were implemented in the
   // MutableServerResult class, so they may be ignored here.
   
   // IF THE NEW CLASS IS NOT DERIVED FROM MutableServerResult
   // REMOVE THESE COMMENT LINES.
   

   // ----------------------------------------------------------
   //             for ResultTemplate class
   // ----------------------------------------------------------
   
// STEP 4: ADD CLASS-SPECIFIC VARIABLES AND FUNCTIONS/METHODS

   // THIS IS THE FIRST LOCATION WHERE ITEMS FOR THE
   // RequestTemplate CLASS ARE TO BE ENTERED.
   
   // THIS IS THE APPROPRIATE LOCATIONS TO DECLARE VARIABLES TO CONTAIN
   // RESULT VALUES
   
   // SOME EXAMPLES ARE SHOWN:
   
   /*
   int  MyInt;
   
   char MyCharPointer;
   
   int  MyStringLength; // used to avoid unneeded reallocations.
   
   struct SomeStructType MyStruct
   */
   
   // THIS IS ALSO THE PLACE TO ADD DECLARATIONS FOR ANY METHODS
   // THIS CLASS WILL USE FOR INTERNAL ACTIVITIES
   
   /*
   void MyActionMethod( int some_parameter );
   */

public:

// STEP 5: ADD CLASS-SPECIFIC METHOD DECLARATIONS FOR PUBLIC ACCESS.

   // DECLARE A CONSTRUCTOR FOR INITIALIZATION ACTIVITIES:
   
   ResultTemplate();
   
   // IF CLEANUP IS NEEDED (MOST LIKELY IN CASE OF INTERNAL MEMORY ALLOCATION
   // BY THIS CLASS), UNCOMMENT THIS DESTRUCTOR DECLARATION:
   /*
    ~ResultTemplate(); 
    */
   
   // DECLARE ACCESSOR METHODS FOR CLASS-SPECIFIC RESULT VARIABLES:
   //
   // IMPLEMENTATION FOR EXAMPLES SHOWN ARE ALSO IN result_template.cpp:
   
   /*
    
   void SetMyInt( int parameterA );
    
   int GetMyInt();
    
   */
   
   
   
   
   // ===========================================================
   // EVERYTHING FOLLOWING (BETWEEN THE DOUBLE-DASHED LINES)
   // MAY BE DELETED IN ACTUAL IMPLEMENTATIONS.
   
   
   // THE FOLLOWING METHODS ARE ALSO AVAILABLE BY USERS OF
   // THIS CLASS AS A RESULT OF ITS DERIVING FROM THESE OTHER
   // BASE CLASSES.
   
   
   // ----------------------------------------------------------
   //     Methods from MutableServerMessage class
   // ----------------------------------------------------------
   
   // FormatBuffer()
   //
   //   MUST CALL THIS BEFORE CALLING
   //      GetBufferLength() and GetBuffer()
   //
   //   -- calls BufferInitAlloc()
   //   -- calls FormatDerivativeData()
   //   -- terminates the buffer with an additional '\n'
   //      (that is, a line of zero length)
   //
   // THROWs worm_exception for errors
   //
   //void FormatBuffer();

   // GetBufferLength()
   // GetBuffer()
   //
   //       Used to obtain an ascii buffer suitable for
   //       transmission across a socket or ring.
   // 
   //long GetBufferLength() const;
   //const char * GetBuffer() const;

   // ParseMessageLine -- used to handle a buffer which contains
   //                     only one '\n' terminated line, and which,
   //                     therefore may or may not be a complete
   //                     message.
   //
   //   This method:
   //   -- calls BufferInitAlloc()
   //   -- if strlen(p_buffer) > 0:
   //      -- appends buffer to MessageBuffer
   //      -- if p_append_nl == true
   //         -- appends '\n' to MessageBuffer
   //   -- else  [[ strlen(p_buffer) == 0 ]]
   //      -- if p_append_nl == true
   //         -- appends '\n' to MessageBuffer
   //      -- calls ParseDerivativeData()
   //
   // p_buffer: single line of a message
   //           (terminating '\n' stripped off)
   //
   // p_append_nl: should '\n' be appended to p_buffer
   //           
   // p_clearbuffer: used to clear MessageBuffer for first
   //                line of message.
   //
   // RETURNS:
   //      true  = this line completed the message and parsing completed
   //      false = this line did not finish the message
   //
   // THROWs worm_exception for errors
   //
   //bool ParseMessageLine( const char * p_buffer
   //                     ,       bool   p_append_nl   = true
   //                     ,       bool   p_clearbuffer = false
   //                     );

   // ParseFromBuffer -- used to parse a buffer which is known to
   //                    be complete.  Specifically, messages
   //                    which arrive from a ring.
   //
   // THROWs worm_exception for errors
   //
   //void ParseFromBuffer( const char * p_buffer );
   

   // ----------------------------------------------------------
   //       Methods from MutableServerResult class
   // ----------------------------------------------------------

   // THROWs worm_exception if p_status is not one of following:
   //
   //          MSB_RESULT_GOOD
   //          MSB_RESULT_BUSY
   //          MSB_RESULT_FAIL
   //          MSB_RESULT_ERROR
   //
   //void SetStatus( short p_status );

   //short GetStatus();


   // EVERYTHING ABOVE (BETWEEN THE DOUBLE-DASHED LINES)
   // MAY BE DELETED IN ACTUAL IMPLEMENTATIONS.
   // ===========================================================


// DON'T DELETE THIS CLASS-TERMINATING BRACKET


};


#endif // _MSM_ResultTemplate_H
