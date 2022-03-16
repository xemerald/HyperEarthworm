#ifndef pipeconsole_H
#define pipeconsole_H

  /******************************************************************
   *                         Prototypes                             *
   *                                                                *
   ******************************************************************/

// -------- First set of functions are for startstop / server side --------

//Creates in/out pipes and process for 'cmd.exe' console (with 'stdin' and
// 'stdout' redirected to pipes).
// pTagStr:  tag string for in/out pipes.
// Returns NULL if successful; returns an error message if failure.
const PTSTR startPipeConsole(char *pTagStr);

//Terminates the pipe-console and/or its child process(es).
// termAllFlag:  TRUE to terminate console and child process(es); FALSE to
//  terminate child process(es) and only terminate console if there were
//  no child process(es) running.
// Returns TRUE if successful; FALSE if not.
BOOL terminatePipeConsole(BOOL termAllFlag);

//Return TRUE if pipe-console is running.
BOOL isPipeConsoleRunning();

//Blocks until pipe-console is not running, up to the given timeout time.
// timeoutMs:  time-out interval, in millisecond, or INFINITE for no timeout.
void waitForPipeConsoleStopped(DWORD timeoutMs);

//Closes handles to pipe-console pipes.
void closePipeConsoleHandles();

// -------- Functions below here are for user / client side --------

//Creates and returns a file handle for the console 'stdin' pipe.  Data
// for the console is written into this handle.  The pipe needs to have
// been created before this function is called.
// pTagStr:  tag string for pipe.
// Returns the handle if successful; returns INVALID_HANDLE_VALUE if failure.
HANDLE getPipeInFileHandle(char *pTagStr);

//Creates and returns a file handle for the console 'stdout'/'stderr' pipe.
// Data from the console is read from this handle.  The pipe needs to have
// been created before this function is called.
// pTagStr:  tag string for pipe.
// Returns the handle if successful; returns INVALID_HANDLE_VALUE if failure.
HANDLE getPipeOutFileHandle(char *pTagStr);

//Runs loop that receives keyboard input and sends it to the 'stdin' pipe.
// (A control handler is installed to redirect the Ctrl-C input to the
// pipe-console.)
// pTagStr:  tag string for pipe.
// ctrlPipePathname:  path name for pipe-console "control" pipe.
// If successful then blocks indefinitely; if failure then returns FALSE.
BOOL doPipeInLoop(char *pTagStr, char *ctrlPipePathname);

//Runs loop that receives data from the pipe-console 'stdout'/'stderr'
// pipe and sends it to the local-console 'stdout' stream.
// pTagStr:  tag string for pipe.
// If successful then blocks until pipe-console is closed and then
// returns TRUE; if failure then returns FALSE.
BOOL doPipeOutLoop(char *pTagStr);

#endif /* not defined pipeconsole_H */
