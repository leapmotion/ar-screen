#pragma once

#include <array>
#include <iostream>
#include "Leap/GL/Internal/Typle.h"

namespace Leap {
namespace GL {
// The contents of the Internal namespace are not intended to be used publicly, and provide
// no guarantee as to the stability of their API.  The classes and functions are used
// internally in the implementation of the publicly-presented classes.
namespace Internal {

template <typename Typle_> struct Tuple_t;

template <>
struct Tuple_t<EmptyTyple> {
	Tuple_t () { }
	Tuple_t (Tuple_t const &) { }
};

template <typename Head_, typename... BodyTypes_>
struct Tuple_t<Typle_t<Head_,BodyTypes_...>> {
	typedef Typle_t<Head_,BodyTypes_...> Typle;
	static size_t const LENGTH = Length_f<Typle>::V;
	typedef Head_ Head;
	typedef Tuple_t<Typle_t<BodyTypes_...>> BodyTuple;
	Tuple_t () { }
	Tuple_t (Tuple_t const &t) : m_head(t.m_head), m_body_tuple(t.m_body_tuple) { }
	Tuple_t (Head_ const &head, BodyTuple const &body_tuple) : m_head(head), m_body_tuple(body_tuple) { }
	Tuple_t (Head_ const &head, BodyTypes_... body_args) : m_head(head), m_body_tuple(body_args...) { }
	Head_ const &head () const { return m_head; }
	Head_ &head () { return m_head; }
	BodyTuple const &body_tuple () const { return m_body_tuple; }
	BodyTuple &body_tuple () { return m_body_tuple; }
	template <size_t INDEX_> typename std::enable_if<INDEX_==0,Head_ const &>::type el () const { return m_head; }
	template <size_t INDEX_> typename std::enable_if<INDEX_==0,Head_ &>::type el () { return m_head; }
	template <size_t INDEX_> typename std::enable_if<(INDEX_>0),typename Element_f<Typle_t<Head_,BodyTypes_...>,INDEX_>::T const &>::type el () const { return m_body_tuple.template el<INDEX_-1>(); }
	template <size_t INDEX_> typename std::enable_if<(INDEX_>0),typename Element_f<Typle_t<Head_,BodyTypes_...>,INDEX_>::T &>::type el () { return m_body_tuple.template el<INDEX_-1>(); }
	std::array<Head_,LENGTH> const &as_array () const {
		static_assert(TypleIsUniform_f<Typle>::V, "This method is only well-defined if the Tuple_t's element types are uniform.");
		assert(reinterpret_cast<uint8_t const *>(&m_head) + sizeof(Head_) == reinterpret_cast<uint8_t const *>(&m_body_tuple) && "This method is only well-defined if the elements of this Tuple_t are densely packed.");
		return *reinterpret_cast<std::array<Head_,LENGTH> const *>(this);
	}
	std::array<Head_,LENGTH> &as_array () {
		static_assert(TypleIsUniform_f<Typle>::V, "This method is only well-defined if the Tuple_t's element types are uniform.");
		assert(reinterpret_cast<uint8_t *>(&m_head) + sizeof(Head_) == reinterpret_cast<uint8_t *>(&m_body_tuple) && "This method is only well-defined if the elements of this Tuple_t are densely packed.");
		return *reinterpret_cast<std::array<Head_,LENGTH> *>(this);
	}
private:
	Head_ m_head;
	BodyTuple m_body_tuple;
};

typedef Tuple_t<EmptyTyple> EmptyTuple;

template <typename... Types_>
Tuple_t<Typle_t<Types_...>> tuple (Types_... args) {
	return Tuple_t<Typle_t<Types_...>>(args...);
}

inline void PrintWithoutParens (std::ostream &out, EmptyTuple) { }

template <typename Head_, typename... BodyTypes_>
void PrintWithoutParens (std::ostream &out, Tuple_t<Typle_t<Head_,BodyTypes_...>> const &t) {
	out << t.head();
	if (Length_f<Typle_t<BodyTypes_...>>::V > 0) {
		out << ',';
		PrintWithoutParens(out, t.body_tuple());
	}
}

template <typename Typle_>
std::ostream &operator << (std::ostream &out, Tuple_t<Typle_> const &t) {
	out << '(';
	PrintWithoutParens(out, t);
	return out << ')';
}



} // end of namespace Internal
} // end of namespace GL
} // end of namespace Leap
