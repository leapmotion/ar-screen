#pragma once

#include "Leap/GL/Common.h"
#include "Leap/GL/GLHeaders.h"
#include "Leap/GL/Internal/UniformTraits.h"

namespace Leap {
namespace GL {
// The contents of the Internal namespace are not intended to be used publicly, and provide
// no guarantee as to the stability of their API.  The classes and functions are used
// internally in the implementation of the publicly-presented classes.
namespace Internal {

// This structure is for encoding a value (e.g. int32_t(10034)) in the type system for metaprogramming.
template <typename T_, T_ V_> struct Value_t { static T_ const V = V_; };

template <typename Uniform_> struct UniformNameOf_f;
template <typename Name_, Name_ NAME_, GLenum GL_TYPE_, size_t ARRAY_LENGTH_, typename CppType_, MatrixStorageConvention MATRIX_STORAGE_CONVENTION_>
struct UniformNameOf_f<UniformSpecification<Name_,NAME_,GL_TYPE_,ARRAY_LENGTH_,CppType_,MATRIX_STORAGE_CONVENTION_>> {
  typedef Value_t<Name_,NAME_> T;
};

template <typename Uniform_> struct GlTypeMappingOf_f;
template <typename Name_, Name_ NAME_, GLenum GL_TYPE_, size_t ARRAY_LENGTH_, typename CppType_, MatrixStorageConvention MATRIX_STORAGE_CONVENTION_>
struct GlTypeMappingOf_f<UniformSpecification<Name_,NAME_,GL_TYPE_,ARRAY_LENGTH_,CppType_,MATRIX_STORAGE_CONVENTION_>> {
  typedef Typle_t<Value_t<Name_,NAME_>,Value_t<GLenum,GL_TYPE_>> T;
};

template <typename Uniform_> struct ArrayLengthMappingOf_f;
template <typename Name_, Name_ NAME_, GLenum GL_TYPE_, size_t ARRAY_LENGTH_, typename CppType_, MatrixStorageConvention MATRIX_STORAGE_CONVENTION_>
struct ArrayLengthMappingOf_f<UniformSpecification<Name_,NAME_,GL_TYPE_,ARRAY_LENGTH_,CppType_,MATRIX_STORAGE_CONVENTION_>> {
  typedef Typle_t<Value_t<Name_,NAME_>,Value_t<size_t,ARRAY_LENGTH_>> T;
};

template <typename Uniform_> struct CppTypeMappingOf_f;
template <typename Name_, Name_ NAME_, GLenum GL_TYPE_, size_t ARRAY_LENGTH_, typename CppType_, MatrixStorageConvention MATRIX_STORAGE_CONVENTION_>
struct CppTypeMappingOf_f<UniformSpecification<Name_,NAME_,GL_TYPE_,ARRAY_LENGTH_,CppType_,MATRIX_STORAGE_CONVENTION_>> {
  typedef Typle_t<Value_t<Name_,NAME_>,CppType_> T;
};

template <typename Uniform_> struct CppTypeOf_f;
template <typename Name_, Name_ NAME_, GLenum GL_TYPE_, size_t ARRAY_LENGTH_, typename CppType_, MatrixStorageConvention MATRIX_STORAGE_CONVENTION_>
struct CppTypeOf_f<UniformSpecification<Name_,NAME_,GL_TYPE_,ARRAY_LENGTH_,CppType_,MATRIX_STORAGE_CONVENTION_>> {
  typedef CppType_ T;
};

template <typename Uniform_> struct MatrixStorageConventionMappingOf_f;
template <typename Name_, Name_ NAME_, GLenum GL_TYPE_, size_t ARRAY_LENGTH_, typename CppType_, MatrixStorageConvention MATRIX_STORAGE_CONVENTION_>
struct MatrixStorageConventionMappingOf_f<UniformSpecification<Name_,NAME_,GL_TYPE_,ARRAY_LENGTH_,CppType_,MATRIX_STORAGE_CONVENTION_>> {
  typedef Typle_t<Value_t<Name_,NAME_>,Value_t<MatrixStorageConvention,MATRIX_STORAGE_CONVENTION_>> T;
};

template <typename UniformMappingsTyple_> struct CheckUniformTypes;
template <> struct CheckUniformTypes<Typle_t<>> { static void Check () { } };
template <typename Name_, Name_ NAME_, GLenum GL_TYPE_, size_t ARRAY_LENGTH_, typename CppType_, MatrixStorageConvention MATRIX_STORAGE_CONVENTION_, typename... BodyUniformMappings_>
struct CheckUniformTypes<Typle_t<UniformSpecification<Name_,NAME_,GL_TYPE_,ARRAY_LENGTH_,CppType_,MATRIX_STORAGE_CONVENTION_>,BodyUniformMappings_...>> {
  static void Check () {
    Internal::UniformTraits<GL_TYPE_>::template CheckCompatibilityOf<CppType_,ARRAY_LENGTH_>();
    CheckUniformTypes<Typle_t<BodyUniformMappings_...>>::Check();
  }
};

template <GLenum GL_TYPE_, size_t ARRAY_LENGTH_, MatrixStorageConvention MATRIX_STORAGE_CONVENTION_, typename T_>
typename std::enable_if<!UniformTraits<GL_TYPE_>::IS_MATRIX_TYPE>::type UniformizedInterface_UploadArray (GLint location, const T_ &value) {
  typedef typename UniformTraits<GL_TYPE_>::UniformArgumentType UniformArgumentType;
  UniformTraits<GL_TYPE_>::template CheckCompatibilityOf<T_,ARRAY_LENGTH_>();
  UniformTraits<GL_TYPE_>::UploadUsingPointer(location, ARRAY_LENGTH_, reinterpret_cast<const UniformArgumentType *>(&value));
}

template <GLenum GL_TYPE_, size_t ARRAY_LENGTH_, MatrixStorageConvention MATRIX_STORAGE_CONVENTION_, typename T_>
typename std::enable_if<UniformTraits<GL_TYPE_>::IS_MATRIX_TYPE>::type UniformizedInterface_UploadArray (GLint location, const T_ &value) {
  typedef typename UniformTraits<GL_TYPE_>::UniformArgumentType UniformArgumentType;
  UniformTraits<GL_TYPE_>::template CheckCompatibilityOf<T_,ARRAY_LENGTH_>();
  UniformTraits<GL_TYPE_>::UploadUsingPointer(location, ARRAY_LENGTH_, MATRIX_STORAGE_CONVENTION_, reinterpret_cast<const UniformArgumentType *>(&value));
}

} // end of namespace Internal
} // end of namespace GL
} // end of namespace Leap
