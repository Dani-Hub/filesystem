//  Boost wide_test.cpp  -----------------------------------------------------//

//  Copyright Beman Dawes 2005

//  Use, modification, and distribution is subject to the Boost Software
//  License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

//  See library home page at http://www.boost.org/libs/filesystem

#include <boost/filesystem/config.hpp>
# ifdef BOOST_FILESYSTEM_NARROW_ONLY
#   error Narrow character paths only for this compiler and library combo
# endif

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/scoped_array.hpp>
#include <boost/test/minimal.hpp>

#include "../src/utf8_codecvt_facet.hpp"

namespace fs = boost::filesystem;

#include <iostream>
#include <iomanip>
#include <string>
#include <cerrno>

#include "lpath.hpp"

namespace
{
  template< class Path >
  void create_file( const Path & ph, const std::string & contents )
  {
    // TODO: why missing symbol error on Darwin
# ifndef __APPLE__
    fs::ofstream f( ph );
# else    
    std::ofstream f( ph.external_file_string().c_str() );
# endif    
    if ( !f )
      throw fs::basic_filesystem_error<Path>( "wide_test create_file",
        ph, errno );
    if ( !contents.empty() ) f << contents;
  }

  template< class Path >
  void test( const Path & dir, const Path & file, const Path & dot )
  {
    fs::initial_path<Path>();
    fs::current_path<Path>();
    fs::remove( dir / file );
    fs::remove( dir );
    BOOST_CHECK( !fs::exists( dir / file ) );
    BOOST_CHECK( !fs::exists( dir ) );
    BOOST_CHECK( fs::create_directory( dir ) );
    BOOST_CHECK( fs::exists( dir ) );
    BOOST_CHECK( fs::is_directory( dir ) );
    BOOST_CHECK( fs::is_empty( dir ) );
    create_file( dir / file, "wide_test file contests" );
    BOOST_CHECK( fs::exists( dir / file ) );
    BOOST_CHECK( !fs::is_directory( dir / file ) );
    BOOST_CHECK( !fs::is_empty( dir / file ) );
    BOOST_CHECK( fs::file_size( dir / file ) == 23 || fs::file_size( dir / file ) == 24 );
    BOOST_CHECK( fs::equivalent( dir / file, dot / dir / file ) );
    BOOST_CHECK( fs::last_write_time( dir / file ) );
    typedef fs::basic_directory_iterator<Path> it_t;
    int count(0);
    for ( it_t it( dir ); it != it_t(); ++it )
    {
      BOOST_CHECK( *it == dir / file );
      BOOST_CHECK( !fs::is_empty( *it ) );
      ++count;
    }
    BOOST_CHECK( count == 1 );
  }

  // test boost::detail::utf8_codecvt_facet - even though it is not used by
  // Boost.Filesystem on Windows, early detection of problems is worthwhile.
  std::string to_external( const std::wstring & src )
  {
    fs::detail::utf8_codecvt_facet convertor;
    std::size_t work_size( convertor.max_length() * (src.size()+1) );
    boost::scoped_array<char> work( new char[ work_size ] );
    std::mbstate_t state;
    const wchar_t * from_next;
    char * to_next;
    if ( convertor.out( 
      state, src.c_str(), src.c_str()+src.size(), from_next, work.get(),
      work.get()+work_size, to_next ) != std::codecvt_base::ok )
      boost::throw_exception( "to_external conversion error" );
    *to_next = '\0';
    return std::string( work.get() );
  }

} // unnamed namespace

//  test_main  ---------------------------------------------------------------//

int test_main( int argc, char * argv[] )
{

  // So that tests are run with known encoding, use Boost UTF8 codecvt
  std::locale global_loc = std::locale();
  std::locale loc( global_loc, new fs::detail::utf8_codecvt_facet );
  fs::wpath_traits::imbue( loc );

  std::string s( to_external( L"\x2780" ) );
  for (std::size_t i = 0; i < s.size(); ++i )
    std::cout << std::hex << int( static_cast<unsigned char>(s[i]) ) << " ";
  std::cout << std::dec << std::endl;
  BOOST_CHECK( to_external( L"\x2780" ).size() == 3 );
  BOOST_CHECK( to_external( L"\x2780" ) == "\xE2\x9E\x80" );

  std::cout << "begin path test..." << std::endl;
  test( fs::path( "foo" ), fs::path( "bar" ), fs::path( "." ) );
  std::cout << "complete\n\n";

  //  x2780 is circled 1 against white background == e2 9e 80 in UTF-8
  //  x2781 is circled 2 against white background == e2 9e 81 in UTF-8
  std::cout << "begin wpath test..." << std::endl;
  test( fs::wpath( L"\x2780" ), fs::wpath( L"\x2781" ), fs::wpath( L"." ) );
  std::cout << "complete\n\n";

  const long dir[] = { 'b', 'o', 'o', 0 };
  const long file[] = { 'f', 'a', 'r', 0 };
  const long dot[] = { '.', 0 };
  std::cout << "begin lpath test..." << std::endl;
  test( ::user::lpath( dir ), ::user::lpath( file ), ::user::lpath( dot ) );
  std::cout << "complete\n\n";

  return 0;
}