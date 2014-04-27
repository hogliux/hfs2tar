#pragma once
#include <ostream>
#include <cstddef>
#include <stdexcept>

#include <boost/config.hpp>
#if defined(BOOST_NO_STDC_NAMESPACE)
namespace std{ 
  using ::size_t; 
} // namespace std
#endif

#include <boost/detail/endian.hpp>
#include <boost/type_traits/is_enum.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/equal_to.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/access.hpp>

namespace internal {
  
  /* endian conversion helper */
  template <typename T>
  T swap_endian(T u) {
#ifdef BOOST_LITTLE_ENDIAN
    union
    {
      T u;
      unsigned char u8[sizeof(T)];
    } source, dest;
    
    source.u = u;
    
    for (size_t k = 0; k < sizeof(T); k++)
      dest.u8[k] = source.u8[sizeof(T) - k - 1];
    
    return dest.u;
#else
    return u;
#endif
  }

}

class bigendian_binary_iarchive {
  std::istream & m_is;

  template<class Archive>
  struct load_enum_type {
    template<class T>
    static void invoke(Archive &ar, T &t){
      uint32_t copy;
      ar.m_is.read( reinterpret_cast<char*>(&copy), sizeof(uint32_t) );
      t = static_cast<T>(internal::swap_endian(copy));
    }
  };

  template <class Archive>
  struct load_integer_type {
    template<class T>
    static void invoke(Archive & ar, T & t){
      T copy;
      ar.m_is.read( reinterpret_cast<char*>(&copy), sizeof(T) );
      t = internal::swap_endian(copy);
    }
  };

  template <class Archive>
  struct load_noninteger_type {
    template<class T>
    static void invoke(Archive & ar, T & t){
      ar.m_is.read( reinterpret_cast<char*>(&t), sizeof(T) );
    }
  };

  template<class Archive>
  struct load_primitive {
    template<class T>
    static void invoke(Archive & ar, T & t){
      typedef typename boost::mpl::eval_if< boost::is_integral<T>,
					    boost::mpl::identity<load_integer_type<Archive> >,
					    boost::mpl::identity<load_noninteger_type<Archive> > >::type tSaver;
      tSaver::invoke( ar, t );
    }
  };

  template<class Archive>
  struct load_only {
    template<class T>
    static void invoke(Archive & ar, T & t){
      // make sure call is routed through the highest interface that might
      // be specialized by the user.
      boost::serialization::serialize_adl(
					  ar, 
					  t, 
					  0
					  );
    }
  };
  template<class T>
  void load(T &t){
        typedef 
	  BOOST_DEDUCED_TYPENAME boost::mpl::eval_if<boost::is_enum< T >,
						     boost::mpl::identity<load_enum_type<bigendian_binary_iarchive> >,
						     //else
						     BOOST_DEDUCED_TYPENAME boost::mpl::eval_if<
						       // if its primitive
						       boost::mpl::equal_to<
							 boost::serialization::implementation_level< T >,
							 boost::mpl::int_<boost::serialization::primitive_type>
							 >,
						       boost::mpl::identity<load_primitive<bigendian_binary_iarchive> >,
						       // else
						       boost::mpl::identity<load_only<bigendian_binary_iarchive> >
						       > >::type typex;
	typex::invoke(*this, t);
  }    
public:
  ///////////////////////////////////////////////////
  // Implement requirements for archive concept

  typedef boost::mpl::bool_<false> is_loading;
  typedef boost::mpl::bool_<true> is_saving;

  // this can be a no-op since we ignore pointer polymorphism
  template<class T>
  void register_type(const T * = NULL){}

  unsigned int get_library_version(){
    return 0;
  }

  void 
  load_binary(void *address, std::size_t count){
    throw std::runtime_error("load_binary not implemented");
  }

  // the << operators 
  template<class T>
  bigendian_binary_iarchive & operator>>(T & t){
    load(t);
    return * this;
  }

  template<class T, int N>
  bigendian_binary_iarchive & operator>>(T (&t)[N]){
    for ( unsigned int i=0; i<N; ++i ) {
      *this >> t[i];
    }
    return *this;
  }

  template<class T>
  bigendian_binary_iarchive & operator>>(boost::serialization::nvp< T > & t){
    * this >> t.value();
    return * this;
  }

  // the & operator 
  template<class T>
  bigendian_binary_iarchive & operator&(T & t){
    return this->operator>>(t);
  }
  ///////////////////////////////////////////////

  bigendian_binary_iarchive(std::istream & is) :
    m_is(is) {}
};

