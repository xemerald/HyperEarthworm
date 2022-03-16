//---------------------------------------------------------------------------
#ifndef LoggerH
#define LoggerH
//---------------------------------------------------------------------------

#include <worm_statuscode.h>
#include <timefuncs.h>       // WORM_TIMESTR_LENGTH
#include <mutex.h>

#include <fstream>

typedef short WORM_LOG_FLAGS;
#define WORM_LOG_TOFILE    0x01
#define WORM_LOG_TOSTDOUT  0x02
#define WORM_LOG_TOSTDERR  0x04
#define WORM_LOG_TIMESTAMP 0x08
#define WORM_LOG_PIDSTAMP  0x10
#define WORM_LOG_NAMESTAMP 0x20




//---------------------------------------------------------------------------
class TLogger
{
private:
   // The vast majority of the time, this space will not be used, but
   // at least the startstop status message's max size is apx 15K.
   // (actually, there are rumors of one message that is apx 65K).
   static char _buff[16384];
   static char _args[16384];

   static void HandleLog(  WORM_LOG_FLAGS p_flags, char * p_buffer );

protected:

   static TMutex * AccessLock;
   static std::fstream OutStream;
   static char PreviousDate[WORM_TIMESTR_LENGTH+1];
   static bool TruncOnOpen;
   static WORM_STATUS_CODE OpenFile();
   static int MaxTooLongLength; // track the length of the longest message that was too long

public:
   static void TruncateOnOpen( bool p_newstate = true ) { TruncOnOpen = p_newstate; }
   /*  Logit()
   **
   ** RETURNS:  length (count) of variable parameter list
   **           -1 = error
   */
   static void Logit( WORM_LOG_FLAGS p_flags, const char* p_format, ... );
   static void Close();

   /*
   ** Three functions to replace the C logit calls:
   */
   void DUMMY_INIT( const char *, short, int, int ) { return; }
   // Not implemented
   void HTML_Logit( const char *, const char *, ... );
   int Logit( const char *, const char *, ... );
};

#endif
