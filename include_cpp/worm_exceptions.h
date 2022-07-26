//---------------------------------------------------------------------------
#ifndef worm_exceptionsH
#define worm_exceptionsH
//---------------------------------------------------------------------------
#include <stdio.h>
// microsoft pragma to avoid warnings from STL
#pragma warning(disable:4786)
#include <string>
#include <string.h>
#include <exception>

//---------------------------------------------------------------------------
class worm_exception : public std::exception
{
protected:
   std::string What;
public:
   worm_exception( const char * p_message ) throw()
   {
      if ( p_message == NULL )
      {
         What = "";
      }
      else
      {
         What.reserve( strlen(p_message) );
         What = p_message;
      }
   }
   worm_exception( const std::string & p_message ) throw()
   {
      What = p_message;
   }

   ~worm_exception() throw() {}

   worm_exception operator= ( worm_exception & p_other )
   {
      What = p_other.what();
      return *this;
   }
   worm_exception operator= ( worm_exception * p_other )
   {
      What = p_other->what();
      return *this;
   }
   worm_exception operator+= ( const int p_value )
   {
      char _i[15];
      sprintf( _i , "%d" , p_value );
      What.reserve( What.size() + strlen(_i) );
      What += _i;
      return *this;
   }
   worm_exception operator+= ( const long p_value )
   {
      char _i[18];
      sprintf( _i , "%d" , p_value );
      What.reserve( What.size() + strlen(_i) );
      What += _i;
      return *this;
   }
   worm_exception operator+= ( const double p_value )
   {
      char _f[15];
      sprintf( _f , "%f" , p_value );
      What.reserve( What.size() + strlen(_f) );
      What += _f;
      return *this;
   }
   worm_exception operator+= ( const char * p_text )
   {
      if ( p_text != NULL )
      {
         What.reserve( What.size() + strlen(p_text) );
         What += p_text;
      }
      return *this;
   }
   worm_exception operator+= ( const std::string p_text )
   {
      What += p_text;
      return *this;
   }
   const char * what() const throw() { return What.c_str(); }
   std::string what_string() const { return What; }
};
//---------------------------------------------------------------------------
//
// the main purpose for the following classes is to allow applications
// to distinguish among and handle different conditions based on
// exception class.
//
//---------------------------------------------------------------------------
class timeout_exception : public worm_exception
{
public:
   timeout_exception() : worm_exception( "timed out" ) { }
   timeout_exception( char * p_what ) : worm_exception( p_what ) { }
   timeout_exception( std::string & p_what ) : worm_exception( p_what ) { }
};

//---------------------------------------------------------------------------
class length_exception : public worm_exception
{
public:
   length_exception() : worm_exception( "length error" ) { }
   length_exception( char * p_what ) : worm_exception( p_what ) { }
   length_exception( std::string & p_what ) : worm_exception( p_what ) { }
};

//---------------------------------------------------------------------------
class cancel_exception : public worm_exception
{
public:
   cancel_exception() : worm_exception( "cancelled" ) { }
   cancel_exception( char * p_what ) : worm_exception( p_what ) { }
   cancel_exception( std::string & p_what ) : worm_exception( p_what ) { }
};

//---------------------------------------------------------------------------

#endif
