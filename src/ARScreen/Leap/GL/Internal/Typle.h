#pragma once

namespace Leap {
namespace GL {
// The contents of the Internal namespace are not intended to be used publicly, and provide
// no guarantee as to the stability of their API.  The classes and functions are used
// internally in the implementation of the publicly-presented classes.
namespace Internal {

template <typename... Types_> struct Typle_t;

typedef Typle_t<> EmptyTyple;

template <typename T_> struct IsTyple_f { static bool const V = false; };
template <typename... Types_> struct IsTyple_f<Typle_t<Types_...>> { static bool const V = true; };

template <typename T_> struct Length_f;
template <> struct Length_f<EmptyTyple> { static size_t const V = 0; };
template <typename Head_, typename... BodyTypes_> struct Length_f<Typle_t<Head_,BodyTypes_...>> { static size_t const V = 1+Length_f<Typle_t<BodyTypes_...>>::V; };

template <typename T_> struct Head_f;
template <typename Head_, typename... BodyTypes_> struct Head_f<Typle_t<Head_,BodyTypes_...>> { typedef Head_ T; };

template <typename T_> struct BodyTyple_f;
template <typename Head_, typename... BodyTypes_> struct BodyTyple_f<Typle_t<Head_,BodyTypes_...>> { typedef Typle_t<BodyTypes_...> T; };

template <typename Head_, typename BodyTyple_> struct HeadBodyTyple_f;
template <typename Head_, typename... BodyTypes_> struct HeadBodyTyple_f<Head_,Typle_t<BodyTypes_...>> { typedef Typle_t<Head_,BodyTypes_...> T; };

template <typename Typle_, template <typename T_> class Metafunction_f_> struct OnEach_f;
template <typename Head_, typename... BodyTypes_, template <typename T_> class Metafunction_f_> struct OnEach_f<Typle_t<Head_,BodyTypes_...>,Metafunction_f_> {
	typedef typename HeadBodyTyple_f<typename Metafunction_f_<Head_>::T,typename OnEach_f<Typle_t<BodyTypes_...>,Metafunction_f_>::T>::T T;
};
template <typename Head_, template <typename T_> class Metafunction_f_> struct OnEach_f<Typle_t<Head_>,Metafunction_f_> {
	typedef Typle_t<typename Metafunction_f_<Head_>::T> T;
};

template <typename T_, size_t LENGTH_> struct UniformTyple_f { typedef typename HeadBodyTyple_f<T_,typename UniformTyple_f<T_,LENGTH_-1>::T>::T T; };
template <typename T_> struct UniformTyple_f<T_,0> { typedef EmptyTyple T; };

template <typename T_> struct TypleIsUniform_f;
template <> struct TypleIsUniform_f<EmptyTyple> { static bool const V = true; };
template <typename Head_> struct TypleIsUniform_f<Typle_t<Head_>> { static bool const V = true; };
template <typename Element0_, typename Element1_, typename... Rest_> struct TypleIsUniform_f<Typle_t<Element0_,Element1_,Rest_...>> {
	static bool const V = std::is_same<Element0_,Element1_>::value && TypleIsUniform_f<Typle_t<Element1_,Rest_...>>::V;
};

template <typename T_, size_t INDEX_> struct Element_f;// { static_assert(INDEX_ < Length_f<T_>::V, "INDEX_ must be less than length of typle."); };
template <typename Head_, typename... BodyTypes_> struct Element_f<Typle_t<Head_,BodyTypes_...>,0> { typedef Head_ T; };
template <size_t INDEX_> struct Element_f<EmptyTyple,INDEX_> { typedef std::nullptr_t T; };
template <typename Head_, typename... BodyTypes_, size_t INDEX_> struct Element_f<Typle_t<Head_,BodyTypes_...>,INDEX_> {
	static_assert(INDEX_ < Length_f<Typle_t<Head_,BodyTypes_...>>::V, "INDEX_ must be less than length of typle.");
	typedef typename Element_f<Typle_t<BodyTypes_...>,INDEX_-1>::T T;
};

template <typename Typle_, typename T_> struct OccurrenceCount_f;
template <typename T_> struct OccurrenceCount_f<EmptyTyple,T_> { static size_t const V = 0; };
template <typename Head_, typename... BodyTypes_, typename T_> struct OccurrenceCount_f<Typle_t<Head_,BodyTypes_...>,T_> {
	static size_t const V = (std::is_same<Head_,T_>::value ? 1 : 0) + OccurrenceCount_f<Typle_t<BodyTypes_...>,T_>::V;
};

template <typename Typle_> struct ContainsDuplicates_f;
template <> struct ContainsDuplicates_f<EmptyTyple> { static bool const V = false; };
template <typename Head_, typename... BodyTypes_> struct ContainsDuplicates_f<Typle_t<Head_,BodyTypes_...>> {
	static bool const V = (OccurrenceCount_f<Typle_t<BodyTypes_...>,Head_>::V > 0) || ContainsDuplicates_f<Typle_t<BodyTypes_...>>::V;
};

template <typename Typle_, typename Element_> struct IndexIn_f;
template <typename Element_> struct IndexIn_f<EmptyTyple,Element_> { static size_t const V = 0; };
template <typename Head_, typename... BodyTypes_, typename Element_> struct IndexIn_f<Typle_t<Head_,BodyTypes_...>,Element_> {
	static size_t const V = std::is_same<Head_,Element_>::value ? 0 : 1+IndexIn_f<Typle_t<BodyTypes_...>,Element_>::V;
};

template <typename Typle_> struct EachTypleHasEqualLength_f;
template <> struct EachTypleHasEqualLength_f<EmptyTyple> { static bool const V = true; };
template <typename Head_> struct EachTypleHasEqualLength_f<Typle_t<Head_>> { static bool const V = true; };
template <typename Head_, typename... BodyTypes_> struct EachTypleHasEqualLength_f<Typle_t<Head_,BodyTypes_...>> {
	static bool const V = Length_f<Head_>::V == Length_f<typename Head_f<Typle_t<BodyTypes_...>>::T>::V &&
	                      EachTypleHasEqualLength_f<Typle_t<BodyTypes_...>>::V;
};

template <typename Typles_> struct Zip_f {
    static_assert(EachTypleHasEqualLength_f<Typles_>::V, "Each Typle_t element must have the same length.");
private:
	typedef typename OnEach_f<Typles_,Head_f>::T HeadTyple;
	typedef typename OnEach_f<Typles_,BodyTyple_f>::T BodyTypleTyple;
public:
	typedef typename HeadBodyTyple_f<HeadTyple,typename Zip_f<BodyTypleTyple>::T>::T T;
};
template <typename... RestOfTyples_> struct Zip_f<Typle_t<EmptyTyple,RestOfTyples_...>> { 
private:
    typedef Typle_t<EmptyTyple,RestOfTyples_...> Typles;
    static_assert(EachTypleHasEqualLength_f<Typles>::V, "each Typle_t element must have the same length");
public:
    typedef EmptyTyple T;
};

template <typename ZippedTyples_> struct Unzip_f {
    typedef typename Zip_f<ZippedTyples_>::T T; // Zip_f is its own inverse
};

} // end of namespace Internal
} // end of namespace GL
} // end of namespace Leap
