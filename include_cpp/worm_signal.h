//---------------------------------------------------------------------------
#ifndef worm_signalH
#define worm_signalH
//---------------------------------------------------------------------------
#include <signal.h>

// These typedefs are for signal() handling
#ifdef __cplusplus
   typedef void (*SIG_HANDLR_PTR)(int);
#else
   typedef void (*SIG_HANDLR_PTR)();
#endif

void SignalHandler( int p_signum );

#endif
