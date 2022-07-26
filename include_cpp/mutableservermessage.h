// mutableserverresult.h: interface for the MutableServerResult class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(MUTABLESERVERMESSAGE_H)
#define MUTABLESERVERMESSAGE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// microsoft pragma to avoid warnings relating to the container
// template names being too long
#pragma warning(disable:4786)

#include <string>  // for std::string
#include <vector>  // for std::vector
#include <worm_exceptions.h>

/*
enum MSR_PARSE_STATE
{
  MSR_PARSE_COMPLETE   = 0
, MSR_PARSE_INCOMPLETE = 1
, MSR_PARSE_ERROR      = 2
};
*/


class MutableServerMessage 
{
protected:
   
   std::string MessageBuffer;

   // ----------------------------------------------------------
   //     virtual methods from MutableServerMessage
   //     to be implemented for a complete derivative classes
   // ----------------------------------------------------------

   // BufferInitAlloc -- when preparing to format a message
   //                    using FormatBuffer()], this is
   //                    first called.  The result is,
   //                    for the first call, MessageBuffer
   //                    will be assured to be at least this
   //                    long.
   //                    Overriding this method allows
   //                    derivative classes to minimize
   //                    reallocations as message bits are appended
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
   virtual long BufferInitAlloc() { return 10L; }


   // FormatDerivativeData
   //
   //     Method by which derivative classes append
   //     their content to MessageBuffer.
   //
   //
   //  NOTE: ALWAYS CALL <super_class>::FormatDerivativeData()
   //        at the start of each implementation to allow
   //        base classes to append their stuff (if any)
   //        to the buffer.
   //        Thus the buffer is built up from the base-most class.
   //
   // THROW worm_exception for errors
   //
   virtual void FormatDerivativeData() { return; }


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
   virtual void ParseDerivativeData() { return; }

public:

	MutableServerMessage();

   // FormatBuffer()
   //
   //   -- calls BufferInitAlloc()
   //   -- calls FormatDerivativeData()
   //   -- terminates the buffer with an additional '\n'
   //      (that is, a line of zero length)
   //
   // THROWs worm_exception for errors
   //
   void FormatBuffer();

   // GetBufferLength()
   // GetBuffer()
   //
   //       Used to obtain an ascii buffer suitable for
   //       transmission across a socket or ring.
   // 
   long GetBufferLength() const;
   const char * GetBuffer() const;

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
   bool ParseMessageLine( const char * p_buffer
                        ,       bool   p_append_nl   = true
                        ,       bool   p_clearbuffer = false
                        );

   // ParseFromBuffer -- used to parse a buffer which is known to
   //                    be complete.  Specifically, messages
   //                    which arrive from a ring.
   //
   // THROWs worm_exception for errors
   //
   void ParseFromBuffer( const char * p_buffer );
};

#endif // !defined(MUTABLESERVERMESSAGE_H)
