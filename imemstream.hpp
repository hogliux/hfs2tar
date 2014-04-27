#include <streambuf>
#include <istream>

struct membuf: std::streambuf {
  inline membuf(char const* base, size_t size) {
    char* p(const_cast<char*>(base));
    this->setg(p, p, p + size);
  }
};
struct imemstream: virtual membuf, std::istream {
  inline imemstream(char const* base, size_t size)
    : membuf(base, size)
    , std::istream(static_cast<std::streambuf*>(this)) {
  }
};

