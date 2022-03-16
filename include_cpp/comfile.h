//---------------------------------------------------------------------------
#ifndef Comfile_H
#define Comfile_H
//---------------------------------------------------------------------------


#include "configsource.h"


#define MAX_COM_FILE 10

//#define CS_MODE_COMMAND  0
#define CS_MODE_FILE     1
#define CS_MODE_ARCHIVE  2



//---------------------------------------------------------------------------
class TComFileParser : public ConfigSource
{
private:

protected:

	FILE *       files[MAX_COM_FILE];
	int	       OpenFileCount;
	FILE *       Archive;

	char *       name;
	bool		    eof;

public:
   TComFileParser();
   ~TComFileParser();

   bool Open(const char* p_filename);

   bool DesignateArchive(FILE* p_file);

   void Close();

   int  ReadLine();

};

#endif
