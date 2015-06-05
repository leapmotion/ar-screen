#pragma once

// The contents of this file are functions and structures used in C++ template metaprogramming.

#include <tuple>

namespace Leap {
namespace GL {
// The contents of the Internal namespace are not intended to be used publicly, and provide
// no guarantee as to the stability of their API.  The classes and functions are used
// internally in the implementation of the publicly-presented classes.
namespace Internal {

// This strange function is used in a metaprogramming technique that I call "lifted C++",
// where ordinary C++ functions are used within a decltype expression to extract the
// return type.  The functions are never actually run -- all the computation is manifest
// in the return types of the functions and the operations defined upon them.  Thus,
// with some care, you get to use the full richness of ordinary C++ functions with
// operator and function overloading at compile time.  The decltype "lifts" the ordinary
// C++ code into the metaprogram.  Victor Dods
template <typename T_>
T_ TypeTheoreticConstruct () {
  throw "Don't ever call this function -- only use it within a decltype expression.";
}

template <size_t COUNT_, typename Type_>
struct UniformTuple {
  // This is an example of "lifted C++".
  typedef decltype(std::tuple_cat(TypeTheoreticConstruct<std::tuple<Type_>>(),
                                  TypeTheoreticConstruct<typename UniformTuple<COUNT_-1,Type_>::T>())) T;
};

template <typename Type_>
struct UniformTuple<0,Type_> {
  typedef std::tuple<> T;
};

} // end of namespace Internal
} // end of namespace GL
} // end of namespace Leap
