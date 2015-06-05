#pragma once

#include "Leap/GL/Internal/Typle.h"

namespace Leap {
namespace GL {
// The contents of the Internal namespace are not intended to be used publicly, and provide
// no guarantee as to the stability of their API.  The classes and functions are used
// internally in the implementation of the publicly-presented classes.
namespace Internal {

template <typename MappingTyple_> struct TypeMap_t {
private:
	static_assert(IsTyple_f<MappingTyple_>::V, "MappingTyple_ must be a Typle_t.");
	typedef typename Element_f<MappingTyple_,0>::T HeadMapping;
	static_assert(IsTyple_f<HeadMapping>::V, "HeadMapping must be a typle.");
	static_assert(Length_f<HeadMapping>::V == 2, "Each mapping must be a (domain,codomain) pair.");
	typedef TypeMap_t<typename BodyTyple_f<MappingTyple_>::T> BodyMap;
public:
	typedef typename Element_f<typename Unzip_f<MappingTyple_>::T,0>::T Domain;
	typedef typename Element_f<typename Unzip_f<MappingTyple_>::T,1>::T Codomain;
	static_assert(!ContainsDuplicates_f<Domain>::V, "Domain must contain no duplicate elements.");
	static_assert(Length_f<Domain>::V == Length_f<Codomain>::V, "Domain and Codomain must have the same length.");
};

template <typename TypeMap_, typename DomainElement_> struct Eval_f {
private:
	static size_t const INDEX_IN_DOMAIN = IndexIn_f<typename TypeMap_::Domain,DomainElement_>::V;
	static_assert(INDEX_IN_DOMAIN < Length_f<typename TypeMap_::Domain>::V, "DomainElement_ not found in TypeMap_'s Domain.");
public:
	typedef typename Element_f<typename TypeMap_::Codomain,INDEX_IN_DOMAIN>::T T;
};

} // end of namespace Internal
} // end of namespace GL
} // end of namespace Leap
