#pragma once

#include "Leap/GL/Internal/Tuple.h"
#include "Leap/GL/Internal/TypeMap.h"

namespace Leap {
namespace GL {
// The contents of the Internal namespace are not intended to be used publicly, and provide
// no guarantee as to the stability of their API.  The classes and functions are used
// internally in the implementation of the publicly-presented classes.
namespace Internal {

template <typename TypeMap_> struct Map_t;

template <>
struct Map_t<TypeMap_t<EmptyTyple>> {
    Map_t () { }
};

template <typename TypeMap_>
struct Map_t {
    typedef Tuple_t<typename TypeMap_::Codomain> Values;
    Map_t (Map_t const &m) : m_values(m.m_values) { }
    template <typename... Types_>
    Map_t (Types_... args) : m_values(args...) { }
    template <typename DomainElement_> struct val_const_ReturnType_f { typedef typename Eval_f<TypeMap_,DomainElement_>::T const &T; };
    template <typename DomainElement_> struct val_ReturnType_f { typedef typename Eval_f<TypeMap_,DomainElement_>::T &T; };
    template <typename DomainElement_> typename val_const_ReturnType_f<DomainElement_>::T val () const { return m_values.template el<IndexIn_f<typename TypeMap_::Domain,DomainElement_>::V>(); }
    template <typename DomainElement_> typename val_ReturnType_f<DomainElement_>::T &val () { return m_values.template el<IndexIn_f<typename TypeMap_::Domain,DomainElement_>::V>(); }
    Values const &values () const { return m_values; }
    Values &values () { return m_values; }
private:
    Values m_values;
};

// template <typename... DomainElements_, typename... CodomainElements_>
// Map_t<typename Zip_f<Typle_t<Typle_t<DomainElements_...>,Typle_t<CodomainElements_...>>>::T> map (CodomainElements_... args) {
//  return Map_t<typename Zip_f<Typle_t<Typle_t<DomainElements_...>,Typle_t<CodomainElements_...>>>::T>(args...);
// }

} // end of namespace Internal
} // end of namespace GL
} // end of namespace Leap
