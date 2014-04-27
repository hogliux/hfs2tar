#pragma once
#include <boost/fusion/include/map.hpp>
#include <boost/fusion/algorithm.hpp>

namespace internal { 
  template <typename Ar>
  struct saver {
  private:
    Ar & ar;
  public:
    saver( Ar & a ) : ar(a) {}
    template <typename T>
    void operator()(T& data) const {
      ar & data;
    }
  };
}


#define BOOST_FUSION_ADD_SERIALIZER( x ) \
namespace boost { \
  namespace serialization { \
    template <typename Ar> \
    void serialize(Ar& ar, struct x& fusion_type, unsigned version) { \
      internal::saver<Ar> s(ar); \
      boost::fusion::for_each(fusion_type, s); \
    } \
  } \
}
