/*
 * request_template.h -- Template and example of a request message class
 *                       for use in the mutable server model.
 *
 *                       In addition to passport handling (which is provided
 *                       as a result of deriving from the MutableServerRequest
 *                       class,
 *                       this example class is expected to also handle
 *                       an integer and a char string of undetermined length
 *                       (to demonstrate memory allocation and cleanup).
 */
 
// STEP 1: Do a global replacement of the string "RequestTemplate"
//         with the name of your new class
//
#if ! defined(_MSM_RequestTemplate_H)
#define _MSM_RequestTemplate_H


// STEP 2: If the new class should be derived from a class other than
//         MutableServerRequest, replace this with the appropriate
//         include.
//         This specific include requires the project make file to have
//         /earthworm/..../src/libsrc_cpp/servers/messaging
//         in the include path.
#include <mutableserverrequest.h>



// STEP 3: If the new class should be derived from a class other than
//         MutableServerRequest, replace it with the appropriate
//         base class name.
class RequestTemplate : public MutableServerRequest
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
   // RequestTemplate TO LOCATE THE FIRST ITEMS TO CHANGE.
   // ----------------------------------------------------------

   // BufferInitAlloc -- when preparing to format a message
   //                    using FormatBuffer()], this is
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
   //             for MutableServerRequest class
   // ----------------------------------------------------------

   // Only Passport-related items were implemented in the
   // MutableServerRequest class, so they may be ignored here.
   
   // IF THE NEW CLASS IS NOT DERIVED FROM MutableServerRequest
   // REMOVE THESE COMMENT LINES.
   

   // ----------------------------------------------------------
   //             for RequestTemplate class
   // ----------------------------------------------------------
   
// STEP 4: ADD CLASS-SPECIFIC VARIABLES AND FUNCTIONS/METHODS

   // THIS IS THE FIRST LOCATION WHERE ITEMS FOR THE
   // RequestTemplate CLASS ARE TO BE ENTERED.
   
   // THIS IS THE APPROPRIATE LOCATIONS TO DECLARE CONTAINER
   // VARIABLES.
   //
   // SINCE THIS REQUEST EXAMPLE IS TO SHOW AN int AND A char pointer,
   // DECLARE RELEVANT VARIABLES HERE:
   
   int  MyInt;
   
   char MyCharPointer;
   
   int  MyStringLength; // used to avoid unneeded reallocations.
   
   /*
   void MyExampleMethod( int some_parameter );
   */
   
public:  // How others use this class

   // ----------------------------------------------------------
   //             for RequestTemplate class
   // ----------------------------------------------------------

// STEP 5: ADD CLASS-SPECIFIC METHOD DECLARATIONS.

   // TO PERFORM INITIALIZATION, DECLARE A CONSTRUCTOR
   RequestTemplate();
   
   // IF YOU NEED TO PERFORM [MEMORY] CLEANUP AFTER FINISHED WITH THE
   // CLASS, DO SO IN THE DESTRUCTOR.
   // IF NO CLEANUP IS NEEDED, THIS DECLARATION MAY BE
   // REMOVED (ALONG WITH THE DEFINITION IN THE cpp FILE).
   ~RequestTemplate();


   // ADD DECLARATIONS FOR ACCESSOR METHODS (FUNCTIONS) TO
   // GET TO THE VARIABLES SPECIFIC TO THIS CLASS:
   
   void SetMyInteger( int p_int );
   
   // SetMyString()
   //
   // PARAMETERS:
   //       p_str may be NULL
   //
   // RETURNS:
   //      true = set okay
   //     false = failed to allocate memory for the buffer
   bool SetMyString( const char * p_str );
   
   
   // Adding 'const' at the end prevents the caller from
   // changing the value in a way that would alter the contents
   // of the class instance.
   
   int GetMyInteger() const;
   
   // GetMyString() may return NULL
   char * GetMyString() const;


   
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
   //       Methods from MutableServerRequest class
   // ----------------------------------------------------------

   //void ClearPassport();

   //void AddPassportLine( const char * p_line );

   //void AddPassportLine( std::string p_line );

   //int GetPassportLineCount();

   // GetPassportLine() -- return one of the passport lines.
   //
   //  p_index must be in the range 0 - GetPassportLineCount()
   //
   // RETURNS:
   //        the passport line
   //     or NULL if index out of range, etc.
   //
   //const char * GetPassportLine( int p_index );

   // EVERYTHING ABOVE (BETWEEN THE DOUBLE-DASHED LINES)
   // MAY BE DELETED IN ACTUAL IMPLEMENTATIONS.
   // ===========================================================


// DON'T DELETE THIS CLASS-TERMINATING BRACKET
};

#endif // _MSM_RequestTemplate_H