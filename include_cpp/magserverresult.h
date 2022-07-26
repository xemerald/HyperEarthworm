// magserverresult.h: interface for the MagServerResult class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_MAGSERVERRESULT_H__INCLUDED_)
#define _MAGSERVERRESULT_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "mutableserverresult.h"
#include <earthworm_defs.h>  // MAG_NULL
#include <string>
#include <vector>


//-------------------------------------------------------------------


typedef struct _MAG_SRVR_CHANNEL
{
   long   ampid;  // id if amp came from database
   long   channelid;
   long   componentid;
   char   sta[10];
   char   comp[10];
   char   net[10];
   char   loc[10];
   float  lat;
   float  lon;
   float  elev;
   float  azm;
   float  dip;

   float  magnitude;

   float  amp1;
   double amp1time;
   float  amp1period;

   float  amp2;
   double amp2time;
   float  amp2period;

} MAG_SRVR_CHANNEL;

typedef std::vector<MAG_SRVR_CHANNEL> MAG_SRVR_CHANNEL_VECTOR;

//-------------------------------------------------------------------
class MagServerResult : public MutableServerResult  
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
   //
   long BufferInitAlloc();


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
   void FormatDerivativeData();


   // ParseDerivativeData
   //
   //     Method by which derivative classes extract
   //     their content from MessageBuffer.
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
   //                   for MagServerResult
   // ----------------------------------------------------------

   long  EventId
      ,  OriginId
      ;
   int   MagType;
   float MagAverage;
   float MagError;

   std::string Author;
   
   MAG_SRVR_CHANNEL_VECTOR Channels;
   
public:

	MagServerResult();

   void SetMagnitudeInfo(       long    p_eventid
                        ,       long    p_originid
                        ,       int     p_magtype
                        ,       float   p_average
                        ,       float   p_error
                        , const char  * p_author
                        );

   void ClearChannels();

   // This will not overwrite/update existing
   // information for a channel, calling it again
   // for the same channel will duplicate the channel
   void AddChannel( MAG_SRVR_CHANNEL p_channelinfo );


   const long GetEventId() const;
   
   const long GetOriginId() const;

   const int GetMagType() const;

   const float GetAverage() const;

   const float GetError() const;

   const char * GetAuthor() const;

   const int GetChannelCount();

   bool GetChannel( unsigned int p_index
                  , MAG_SRVR_CHANNEL * r_channel
                  ) const;

};

#endif // !defined(_MAGSERVERRESULT_H__INCLUDED_)
