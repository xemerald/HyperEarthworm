#include <limits.h>		/* CHAR_BIT */
#include <stdio.h>		/* printf() */

#define STR1(x) #x
#define STR(x) STR1(x)

int main() {
   printf( "\n"
           "buildinfo: "
#ifdef _LINUX
                       "Linux"
#else
#ifdef _MACOSX
                       "MacOS"
#else
#ifdef _SOLARIS
                       "Solaris"
#else
                       "Unsupported Unix"

#endif
#endif
#endif
                                        " EARTHWORM-%u build of "
                                        __DATE__ " " __TIME__ "\n\n",
                                        CHAR_BIT * (int) sizeof( void * ) );
#if 0
		   printf( "MSVC++ [predefined _MSC_VER]         = %s\n", STR( _MSC_VER ) );
   printf( "                                       1800 is VS2013\n" );
   printf( "                                       1900 is VS2015 (minimum required)\n" );
   printf( "                                       191x is VS2017\n" );
   printf( "SDK    [ntverp.h   VER_PRODUCTBUILD] = %s\n", STR( VER_PRODUCTBUILD ) );
   printf( "                                       7600  is SDK 7.1A (minimum required)\n" );
   printf( "                                       9200  is SDK 8.0\n" );
   printf( "                                       9600  is SDK 8.1\n" );
   printf( "                                       10011 is SDK 10\n" );
   printf( "Target [windows.h  _WIN32_WINNT]     = %s\n", STR( _WIN32_WINNT ) );
   printf( "                                       0x0400 is Windows NT 4.0\n" );
   printf( "                                       0x0500 is Windows 2000\n" );
   printf( "                                       0x0501 is Windows XP\n" );
   printf( "                                       0x0502 is Windows Server 2003\n" );
   printf( "                                       0x0600 is Windows Vista\n" );
   printf( "                                              or Windows Server 2008\n" );
   printf( "                                       0x0601 is Windows 7 (minimum required)\n" );
   printf( "                                       0x0602 is Windows 8\n" );
   printf( "                                       0x0603 is Windows 8.1\n" );
   printf( "                                       0x0A00 is Windows 10\n" );
#endif
   return 0;
}
