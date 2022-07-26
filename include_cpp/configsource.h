// configsource.h: interface for the ConfigSource class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CONFIGSOURCE_H__INCLUDED_)
#define _CONFIGSOURCE_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//---------------------------------------------------------------------------
#include <stdlib.h>  // for atol, atoi
#include <stdio.h>
#include <string.h>
#include <limits.h> // for invalid values
#include <float.h>  // for invalid values


#define MAX_LINE_LENGTH   1024

#define MAX_TOKEN_LENGTH  100

#define CS_MODE_COMMAND  0


// These must all be negative numbers, as they are mingled with
// the number of bytes read in the ReadLine() return code.
enum COMFILE_STATE
{
     COMFILE_ERROR   = -2
   , COMFILE_EOF     = -1
   , COMFILE_GOOD    =  0
};


//---------------------------------------------------------------------------
class ConfigSource  
{
protected:

	int          ReadMode;

   char curr_token[MAX_TOKEN_LENGTH+1];

	int			 LastError; // err

	int			 LineParseIndex;  // i

	char		    CurrentLine[MAX_LINE_LENGTH+1]; // card

	char		    Token[100];
	bool		    TokenIsNull;     // nul

   char         LastMessage[120];

public:
	ConfigSource();
   ~ConfigSource();

   virtual void Close() { };

   virtual int  ReadLine() = 0;

   const char* GetCurrentLine() { return (const char *)CurrentLine; }
   char* NextToken();
   const char* GetCurrentToken() { return (const char *)Token; }
   char* GetToken(int n, int off);
   char* GetToken(int n);
   int Load(const char * p_cmd);
   int Error( char** p_textual = NULL );
   bool IsTokenNull() { return TokenIsNull; }
   char* String();
   int Int();
   int Int(int n);
   int Int(int n, int off);
   long Long();
   long Long(int n);
   long Long(int n, int off);
   // Double(?) returns MINDOUBLE for invalid
   double Double();
   double Double(int n);
   double Double(int n, int off);
   bool Its(const char* p_str);

   static int INVALID_INT;
   static long INVALID_LONG;
   static double INVALID_DOUBLE;

};

#endif // !defined(_CONFIGSOURCE_H__INCLUDED_)
