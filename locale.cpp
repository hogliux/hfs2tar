#include <stdexcept>
#include <iconv.h>
#include <wchar.h>
#include <errno.h>

#include "locale.hpp"

namespace internal {
  class iconv {
  private:
    iconv_t m_cd;
  public:
    iconv( const char * dst_charset, const char * src_charset ) :
      m_cd( iconv_open( dst_charset, src_charset ) ) {
      if (m_cd==NULL) {
	throw std::runtime_error("Unable to open iconv");
      }
    }
    
    ~iconv() {
      if (m_cd!=NULL) {
	iconv_close(m_cd);
	m_cd = NULL;
      }
    }
    
    size_t operator()(char **inbuf, size_t *inbytesleft, char **outbuf, size_t *outbytesleft) {
      return ::iconv(m_cd, inbuf, inbytesleft, outbuf, outbytesleft);
    }
  };
}

std::string utf16_to_utf8( const std::basic_string<uint16_t> & wide_str ) {
  internal::iconv cd( "UTF-8", "UTF-16" );
  size_t len = wide_str.size();
  size_t len_guess = len;
  while ( true ) {
    /* reset state */
    size_t in_bytes_left=0;
    size_t out_bytes_left=0;
    cd( NULL, &in_bytes_left, NULL, &out_bytes_left );

    std::basic_string<uint16_t> copy(wide_str);
    int err;
    char * inbuf = (char*)copy.c_str();
    char buffer[len_guess+1];
    char * outbuf = buffer;
    in_bytes_left = len * sizeof(uint16_t);
    out_bytes_left = len_guess;
    err = cd( &inbuf, &in_bytes_left, &outbuf, &out_bytes_left );
    if ( err < 0 ) {
      if ( errno != E2BIG ) {
	throw std::runtime_error( "Error during locale conversion" );
      }
    } else {
      return std::string(buffer,len_guess-out_bytes_left);
    }
    len_guess <<= 1;
  }
}
