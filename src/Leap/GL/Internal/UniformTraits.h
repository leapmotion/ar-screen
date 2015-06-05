#pragma once

#include "Leap/GL/Common.h"
#include "Leap/GL/GLHeaders.h" // convenience header for cross-platform GL includes
#include "Leap/GL/ShaderException.h"

namespace Leap {
namespace GL {
// The contents of the Internal namespace are not intended to be used publicly, and provide
// no guarantee as to the stability of their API.  The classes and functions are used
// internally in the implementation of the publicly-presented classes.
namespace Internal {

template <GLenum GL_TYPE_> struct UniformTraits { static const bool IS_DEFINED = false; };

#define DEFINE_UNIFORM_TRAITS(GL_TYPE_, GLtype_, COMPONENT_COUNT_, glUniform, glUniformv, GLUniformArgumentType_) \
  template <> \
  struct UniformTraits<GL_TYPE_> { \
    static const bool IS_DEFINED = true; \
    static const bool IS_MATRIX_TYPE = false; \
    typedef GLtype_ GLtype; \
    typedef GLUniformArgumentType_ UniformArgumentType; \
    static const size_t COMPONENT_COUNT = COMPONENT_COUNT_; \
    template <typename... Types_> \
    static void UploadUsingValues (GLint location, Types_... args) { \
      static_assert(sizeof...(Types_) == COMPONENT_COUNT_, "Expected exactly " #COMPONENT_COUNT_ " value arguments after the location argument."); \
      glUniform(location, args...); \
    } \
    static void UploadUsingPointer (GLint location, GLsizei count, const UniformArgumentType *value) { \
      glUniformv(location, count, value); \
    } \
    template <typename T_, size_t ARRAY_LENGTH_> \
    static typename std::enable_if<ARRAY_LENGTH_==1>::type CheckCompatibilityOf () { \
      static_assert(std::is_standard_layout<T_>::value, "T_ must be a standard-layout type mapping directly onto a " #GLUniformArgumentType_ "[" #COMPONENT_COUNT_ "]."); \
      static_assert(sizeof(T_) == ARRAY_LENGTH_*COMPONENT_COUNT*sizeof(UniformArgumentType), "T_ must be a standard-layout type mapping directly onto a " #GLUniformArgumentType_ "[" #COMPONENT_COUNT_ "]."); \
    } \
    template <typename T_, size_t ARRAY_LENGTH_> \
    static typename std::enable_if<(ARRAY_LENGTH_>1)>::type CheckCompatibilityOf () { \
      static_assert(std::is_standard_layout<T_>::value, "T_ must be a standard-layout type mapping directly onto a " #GLUniformArgumentType_ "[" #COMPONENT_COUNT_ "][ARRAY_LENGTH_]."); \
      static_assert(sizeof(T_) == ARRAY_LENGTH_*COMPONENT_COUNT*sizeof(UniformArgumentType), "T_ must be a standard-layout type mapping directly onto a " #GLUniformArgumentType_ "[" #COMPONENT_COUNT_ "][ARRAY_LENGTH_]."); \
    } \
  }

#define DEFINE_MATRIX_UNIFORM_TRAITS(GL_TYPE_, GLtype_, ROWS_, COLUMNS_, glUniformMatrixv) \
  template <> \
  struct UniformTraits<GL_TYPE_> { \
    static const bool IS_DEFINED = true; \
    static const bool IS_MATRIX_TYPE = true; \
    typedef GLtype_ GLtype; \
    typedef GLtype_ UniformArgumentType; \
    static const size_t COMPONENT_COUNT = ROWS_*COLUMNS_; \
    static void UploadUsingPointer (GLint location, GLsizei count, MatrixStorageConvention matrix_storage_convention, const UniformArgumentType *value) { \
      glUniformMatrixv(location, count, matrix_storage_convention == MatrixStorageConvention::ROW_MAJOR ? GL_TRUE : GL_FALSE, value); \
    } \
    template <typename T_, size_t ARRAY_LENGTH_> \
    static typename std::enable_if<ARRAY_LENGTH_==1>::type CheckCompatibilityOf () { \
      static_assert(std::is_standard_layout<T_>::value, "T_ must be a standard-layout type mapping directly onto a " #GLtype_ "[" #ROWS_ "*" #COLUMNS_ "]."); \
      static_assert(sizeof(T_) == ARRAY_LENGTH_*COMPONENT_COUNT*sizeof(UniformArgumentType), "T_ must be a standard-layout type mapping directly onto a " #GLtype_ "[" #ROWS_ "*" #COLUMNS_ "]."); \
    } \
    template <typename T_, size_t ARRAY_LENGTH_> \
    static typename std::enable_if<(ARRAY_LENGTH_>1)>::type CheckCompatibilityOf () { \
      static_assert(std::is_standard_layout<T_>::value, "T_ must be a standard-layout type mapping directly onto a " #GLtype_ "[" #ROWS_ "*" #COLUMNS_ "][ARRAY_LENGTH_]."); \
      static_assert(sizeof(T_) == ARRAY_LENGTH_*COMPONENT_COUNT*sizeof(UniformArgumentType), "T_ must be a standard-layout type mapping directly onto a " #GLtype_ "[" #ROWS_ "*" #COLUMNS_ "][ARRAY_LENGTH_]."); \
    } \
  }

// OpenGL 2.1 uniform types

DEFINE_UNIFORM_TRAITS(GL_BOOL,           GLboolean, 1,  glUniform1i,  glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_BOOL_VEC2,      GLboolean, 2,  glUniform2i,  glUniform2iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_BOOL_VEC3,      GLboolean, 3,  glUniform3i,  glUniform3iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_BOOL_VEC4,      GLboolean, 4,  glUniform4i,  glUniform4iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_FLOAT,            GLfloat, 1,  glUniform1f,  glUniform1fv, GLfloat);
DEFINE_UNIFORM_TRAITS(GL_FLOAT_VEC2,       GLfloat, 2,  glUniform2f,  glUniform2fv, GLfloat);
DEFINE_UNIFORM_TRAITS(GL_FLOAT_VEC3,       GLfloat, 3,  glUniform3f,  glUniform3fv, GLfloat);
DEFINE_UNIFORM_TRAITS(GL_FLOAT_VEC4,       GLfloat, 4,  glUniform4f,  glUniform4fv, GLfloat);
DEFINE_UNIFORM_TRAITS(GL_INT,                GLint, 1,  glUniform1i,  glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_INT_VEC2,           GLint, 2,  glUniform2i,  glUniform2iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_INT_VEC3,           GLint, 3,  glUniform3i,  glUniform3iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_INT_VEC4,           GLint, 4,  glUniform4i,  glUniform4iv, GLint);

DEFINE_MATRIX_UNIFORM_TRAITS(GL_FLOAT_MAT2,   GLfloat, 2, 2, glUniformMatrix2fv);
DEFINE_MATRIX_UNIFORM_TRAITS(GL_FLOAT_MAT2x3, GLfloat, 2, 3, glUniformMatrix2x3fv);
DEFINE_MATRIX_UNIFORM_TRAITS(GL_FLOAT_MAT2x4, GLfloat, 2, 4, glUniformMatrix2x4fv);
DEFINE_MATRIX_UNIFORM_TRAITS(GL_FLOAT_MAT3x2, GLfloat, 3, 2, glUniformMatrix3x2fv);
DEFINE_MATRIX_UNIFORM_TRAITS(GL_FLOAT_MAT3,   GLfloat, 3, 3, glUniformMatrix3fv);
DEFINE_MATRIX_UNIFORM_TRAITS(GL_FLOAT_MAT3x4, GLfloat, 3, 4, glUniformMatrix3x4fv);
DEFINE_MATRIX_UNIFORM_TRAITS(GL_FLOAT_MAT4x2, GLfloat, 4, 2, glUniformMatrix4x2fv);
DEFINE_MATRIX_UNIFORM_TRAITS(GL_FLOAT_MAT4x3, GLfloat, 4, 3, glUniformMatrix4x3fv);
DEFINE_MATRIX_UNIFORM_TRAITS(GL_FLOAT_MAT4,   GLfloat, 4, 4, glUniformMatrix4fv);

DEFINE_UNIFORM_TRAITS(GL_SAMPLER_1D,                                GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_SAMPLER_2D,                                GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_SAMPLER_3D,                                GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_SAMPLER_CUBE,                              GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_SAMPLER_1D_SHADOW,                         GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_SAMPLER_2D_SHADOW,                         GLint, 1, glUniform1i, glUniform1iv, GLint);

// OpenGL 3.3 uniform types

DEFINE_UNIFORM_TRAITS(GL_UNSIGNED_INT,      GLuint, 1, glUniform1ui, glUniform1uiv, GLuint);
DEFINE_UNIFORM_TRAITS(GL_UNSIGNED_INT_VEC2, GLuint, 2, glUniform2ui, glUniform2uiv, GLuint);
DEFINE_UNIFORM_TRAITS(GL_UNSIGNED_INT_VEC3, GLuint, 3, glUniform3ui, glUniform3uiv, GLuint);
DEFINE_UNIFORM_TRAITS(GL_UNSIGNED_INT_VEC4, GLuint, 4, glUniform4ui, glUniform4uiv, GLuint);

DEFINE_UNIFORM_TRAITS(GL_SAMPLER_1D_ARRAY,                          GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_SAMPLER_2D_ARRAY,                          GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_SAMPLER_1D_ARRAY_SHADOW,                   GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_SAMPLER_2D_ARRAY_SHADOW,                   GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_SAMPLER_2D_MULTISAMPLE,                    GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_SAMPLER_2D_MULTISAMPLE_ARRAY,              GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_SAMPLER_CUBE_SHADOW,                       GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_SAMPLER_BUFFER,                            GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_SAMPLER_2D_RECT,                           GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_SAMPLER_2D_RECT_SHADOW,                    GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_INT_SAMPLER_1D,                            GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_INT_SAMPLER_2D,                            GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_INT_SAMPLER_3D,                            GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_INT_SAMPLER_CUBE,                          GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_INT_SAMPLER_1D_ARRAY,                      GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_INT_SAMPLER_2D_ARRAY,                      GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_INT_SAMPLER_2D_MULTISAMPLE,                GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY,          GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_INT_SAMPLER_BUFFER,                        GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_INT_SAMPLER_2D_RECT,                       GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_UNSIGNED_INT_SAMPLER_1D,                   GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_UNSIGNED_INT_SAMPLER_2D,                   GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_UNSIGNED_INT_SAMPLER_3D,                   GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_UNSIGNED_INT_SAMPLER_CUBE,                 GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_UNSIGNED_INT_SAMPLER_1D_ARRAY,             GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_UNSIGNED_INT_SAMPLER_2D_ARRAY,             GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE,       GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY, GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_UNSIGNED_INT_SAMPLER_BUFFER,               GLint, 1, glUniform1i, glUniform1iv, GLint);
DEFINE_UNIFORM_TRAITS(GL_UNSIGNED_INT_SAMPLER_2D_RECT,              GLint, 1, glUniform1i, glUniform1iv, GLint);

} // end of namespace Internal
} // end of namespace GL
} // end of namespace Leap
