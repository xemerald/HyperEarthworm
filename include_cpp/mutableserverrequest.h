// mutableserverrequest.h: interface for the MutableServerRequest class.
//
// This is a virtual base class for handling within MutableServerBase.
// Mostly unimplemented, it serves as a template to provide a
// consistent interface.
//
// It adds passport handling on top of a MutableServerMessage.
//////////////////////////////////////////////////////////////////////

#if !defined(MUTABLESERVERREQUEST_H)
#define MUTABLESERVERREQUEST_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "mutableservermessage.h"


// Each passport line is started thusly:
//
//  |->PP <passport_line>
//
//  as if:  sprintf( buffer, "%s....", MSR_PASSPORT_TAG,... )
//
#define MSR_PASSPORT_TAG  "PP "

// After the passport, the start of the data is
// indicated with this tag alone on a line:
//
//  |->DATA
//
//  as if:  sprintf( buffer, "%s\n", MSR_DATASTART_LINE );
//
#define MSR_DATASTART_LINE "DATA"


// --------------------------------------------------------------------
class MutableServerRequest : public MutableServerMessage
{
protected:

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
   //        Thus the buffer is built up from the base-most class.
   //
   // THROW worm_exception for errors
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

   std::vector<std::string> Passport;

public:

   // ----------------------------------------------------------
   //             for MutableServerRequest class
   // ----------------------------------------------------------

	MutableServerRequest();

   void ClearPassport();

   void AddPassportLine( const char * p_line );

   void AddPassportLine( std::string p_line );

   int GetPassportLineCount();

   // GetPassportLine() -- return one of the passport lines.
   //
   //  p_index must be in the range 0 - GetPassportLineCount()
   //
   // RETURNS:
   //        the passport line
   //     or NULL if index out of range, etc.
   //
   const char * GetPassportLine( int p_index );

};

#endif // !defined(MUTABLESERVERREQUEST_H)
