#pragma once

#include "Leap/GL/Common.h"
#include "Leap/GL/GLHeaders.h" // convenience header for cross-platform GL includes
#include "Leap/GL/Internal/UniformTraits.h"
#include "Leap/GL/ShaderException.h"

namespace Leap {
namespace GL {
// The contents of the Internal namespace are not intended to be used publicly, and provide
// no guarantee as to the stability of their API.  The classes and functions are used
// internally in the implementation of the publicly-presented classes.
namespace Internal {

template <GLenum GL_TYPE_> struct UniformUploader;

// std::is_fundamental is used so that the Upload function which accepts `const T_ &` is disabled
// for argument types such as GLfloat, GLboolean, etc.  Otherwise the wrong overload is called
// for UniformUploader<GL_BOOL>::Upload(loc, true).

#define DEFINE_UNIFORM_SETTER(GL_TYPE_) \
  template <> struct UniformUploader<GL_TYPE_> { \
    static_assert(UniformTraits<GL_TYPE_>::IS_DEFINED, "UniformTraits<GL_TYPE_> not defined."); \
    template <typename... Types_> \
    static void Upload (GLint location, Types_... args) { \
      UniformTraits<GL_TYPE_>::UploadUsingValues(location, args...); \
    } \
    template <typename T_> \
    static typename std::enable_if<!std::is_fundamental<T_>::value>::type Upload (GLint location, const T_ &value) { \
      typedef typename UniformTraits<GL_TYPE_>::UniformArgumentType UniformArgumentType; \
      UniformTraits<GL_TYPE_>::CheckCompatibilityOf<T_,1>(); \
      UniformTraits<GL_TYPE_>::UploadUsingPointer(location, 1, reinterpret_cast<const UniformArgumentType *>(&value)); \
    } \
    template <size_t ARRAY_LENGTH_, typename T_> \
    static void UploadArray (GLint location, const T_ &value) { \
      typedef typename UniformTraits<GL_TYPE_>::UniformArgumentType UniformArgumentType; \
      UniformTraits<GL_TYPE_>::CheckCompatibilityOf<T_,ARRAY_LENGTH_>(); \
      UniformTraits<GL_TYPE_>::UploadUsingPointer(location, ARRAY_LENGTH_, reinterpret_cast<const UniformArgumentType *>(&value)); \
    } \
  }

#define DEFINE_MATRIX_UNIFORM_SETTER(GL_TYPE_) \
  template <> struct UniformUploader<GL_TYPE_> { \
    static_assert(UniformTraits<GL_TYPE_>::IS_DEFINED, "UniformTraits<GL_TYPE_> not defined."); \
    template <typename T_> \
    static void Upload (GLint location, const T_ &value, MatrixStorageConvention matrix_storage_convention) { \
      typedef typename UniformTraits<GL_TYPE_>::UniformArgumentType UniformArgumentType; \
      UniformTraits<GL_TYPE_>::CheckCompatibilityOf<T_,1>(); \
      UniformTraits<GL_TYPE_>::UploadUsingPointer(location, 1, matrix_storage_convention, reinterpret_cast<const UniformArgumentType *>(&value)); \
    } \
    template <size_t ARRAY_LENGTH_, typename T_> \
    static void UploadArray (GLint location, const T_ &value, MatrixStorageConvention matrix_storage_convention) { \
      typedef typename UniformTraits<GL_TYPE_>::UniformArgumentType UniformArgumentType; \
      UniformTraits<GL_TYPE_>::CheckCompatibilityOf<T_,ARRAY_LENGTH_>(); \
      UniformTraits<GL_TYPE_>::UploadUsingPointer(location, ARRAY_LENGTH_, matrix_storage_convention, reinterpret_cast<const UniformArgumentType *>(&value)); \
    } \
  }

// OpenGL 2.1 uniform types

DEFINE_UNIFORM_SETTER(GL_BOOL);
DEFINE_UNIFORM_SETTER(GL_BOOL_VEC2);
DEFINE_UNIFORM_SETTER(GL_BOOL_VEC3);
DEFINE_UNIFORM_SETTER(GL_BOOL_VEC4);
DEFINE_UNIFORM_SETTER(GL_FLOAT);
DEFINE_UNIFORM_SETTER(GL_FLOAT_VEC2);
DEFINE_UNIFORM_SETTER(GL_FLOAT_VEC3);
DEFINE_UNIFORM_SETTER(GL_FLOAT_VEC4);
DEFINE_UNIFORM_SETTER(GL_INT);
DEFINE_UNIFORM_SETTER(GL_INT_VEC2);
DEFINE_UNIFORM_SETTER(GL_INT_VEC3);
DEFINE_UNIFORM_SETTER(GL_INT_VEC4);

DEFINE_MATRIX_UNIFORM_SETTER(GL_FLOAT_MAT2);
DEFINE_MATRIX_UNIFORM_SETTER(GL_FLOAT_MAT2x3);
DEFINE_MATRIX_UNIFORM_SETTER(GL_FLOAT_MAT2x4);
DEFINE_MATRIX_UNIFORM_SETTER(GL_FLOAT_MAT3x2);
DEFINE_MATRIX_UNIFORM_SETTER(GL_FLOAT_MAT3);
DEFINE_MATRIX_UNIFORM_SETTER(GL_FLOAT_MAT3x4);
DEFINE_MATRIX_UNIFORM_SETTER(GL_FLOAT_MAT4x2);
DEFINE_MATRIX_UNIFORM_SETTER(GL_FLOAT_MAT4x3);
DEFINE_MATRIX_UNIFORM_SETTER(GL_FLOAT_MAT4);

DEFINE_UNIFORM_SETTER(GL_SAMPLER_1D);
DEFINE_UNIFORM_SETTER(GL_SAMPLER_2D);
DEFINE_UNIFORM_SETTER(GL_SAMPLER_3D);
DEFINE_UNIFORM_SETTER(GL_SAMPLER_CUBE);
DEFINE_UNIFORM_SETTER(GL_SAMPLER_1D_SHADOW);
DEFINE_UNIFORM_SETTER(GL_SAMPLER_2D_SHADOW);

// OpenGL 3.3 uniform types

DEFINE_UNIFORM_SETTER(GL_UNSIGNED_INT);
DEFINE_UNIFORM_SETTER(GL_UNSIGNED_INT_VEC2);
DEFINE_UNIFORM_SETTER(GL_UNSIGNED_INT_VEC3);
DEFINE_UNIFORM_SETTER(GL_UNSIGNED_INT_VEC4);

DEFINE_UNIFORM_SETTER(GL_SAMPLER_1D_ARRAY);
DEFINE_UNIFORM_SETTER(GL_SAMPLER_2D_ARRAY);
DEFINE_UNIFORM_SETTER(GL_SAMPLER_1D_ARRAY_SHADOW);
DEFINE_UNIFORM_SETTER(GL_SAMPLER_2D_ARRAY_SHADOW);
DEFINE_UNIFORM_SETTER(GL_SAMPLER_2D_MULTISAMPLE);
DEFINE_UNIFORM_SETTER(GL_SAMPLER_2D_MULTISAMPLE_ARRAY);
DEFINE_UNIFORM_SETTER(GL_SAMPLER_CUBE_SHADOW);
DEFINE_UNIFORM_SETTER(GL_SAMPLER_BUFFER);
DEFINE_UNIFORM_SETTER(GL_SAMPLER_2D_RECT);
DEFINE_UNIFORM_SETTER(GL_SAMPLER_2D_RECT_SHADOW);
DEFINE_UNIFORM_SETTER(GL_INT_SAMPLER_1D);
DEFINE_UNIFORM_SETTER(GL_INT_SAMPLER_2D);
DEFINE_UNIFORM_SETTER(GL_INT_SAMPLER_3D);
DEFINE_UNIFORM_SETTER(GL_INT_SAMPLER_CUBE);
DEFINE_UNIFORM_SETTER(GL_INT_SAMPLER_1D_ARRAY);
DEFINE_UNIFORM_SETTER(GL_INT_SAMPLER_2D_ARRAY);
DEFINE_UNIFORM_SETTER(GL_INT_SAMPLER_2D_MULTISAMPLE);
DEFINE_UNIFORM_SETTER(GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY);
DEFINE_UNIFORM_SETTER(GL_INT_SAMPLER_BUFFER);
DEFINE_UNIFORM_SETTER(GL_INT_SAMPLER_2D_RECT);
DEFINE_UNIFORM_SETTER(GL_UNSIGNED_INT_SAMPLER_1D);
DEFINE_UNIFORM_SETTER(GL_UNSIGNED_INT_SAMPLER_2D);
DEFINE_UNIFORM_SETTER(GL_UNSIGNED_INT_SAMPLER_3D);
DEFINE_UNIFORM_SETTER(GL_UNSIGNED_INT_SAMPLER_CUBE);
DEFINE_UNIFORM_SETTER(GL_UNSIGNED_INT_SAMPLER_1D_ARRAY);
DEFINE_UNIFORM_SETTER(GL_UNSIGNED_INT_SAMPLER_2D_ARRAY);
DEFINE_UNIFORM_SETTER(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE);
DEFINE_UNIFORM_SETTER(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY);
DEFINE_UNIFORM_SETTER(GL_UNSIGNED_INT_SAMPLER_BUFFER);
DEFINE_UNIFORM_SETTER(GL_UNSIGNED_INT_SAMPLER_2D_RECT);

} // end of namespace Internal
} // end of namespace GL
} // end of namespace Leap
