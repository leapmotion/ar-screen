#pragma once

#include "Leap/GL/GLHeaders.h" // convenience header for cross-platform GL includes

namespace Leap {
namespace GL {

// For use in specifying the storage convention for matrix-valued arguments to various functions.
enum MatrixStorageConvention { COLUMN_MAJOR, ROW_MAJOR };

// For use in ShaderFrontend in specifying a uniform.
template <typename Name_, Name_ NAME_, GLenum GL_TYPE_, size_t ARRAY_LENGTH_, typename CppType_, MatrixStorageConvention MATRIX_STORAGE_CONVENTION_> struct UniformSpecification;

template <typename Name_, Name_ NAME_, GLenum GL_TYPE_, typename CppType_>
using Uniform = UniformSpecification<Name_,NAME_,GL_TYPE_,1,CppType_,ROW_MAJOR>; // The value for MATRIX_STORAGE_CONVENTION_ is unused.

template <typename Name_, Name_ NAME_, GLenum GL_TYPE_, size_t ARRAY_LENGTH_, typename CppType_>
using UniformArray = UniformSpecification<Name_,NAME_,GL_TYPE_,ARRAY_LENGTH_,CppType_,ROW_MAJOR>; // The value for MATRIX_STORAGE_CONVENTION_ is unused.

template <typename Name_, Name_ NAME_, GLenum GL_TYPE_, typename CppType_, MatrixStorageConvention MATRIX_STORAGE_CONVENTION_>
using MatrixUniform = UniformSpecification<Name_,NAME_,GL_TYPE_,1,CppType_,MATRIX_STORAGE_CONVENTION_>;

template <typename Name_, Name_ NAME_, GLenum GL_TYPE_, size_t ARRAY_LENGTH_, typename CppType_, MatrixStorageConvention MATRIX_STORAGE_CONVENTION_>
using MatrixUniformArray = UniformSpecification<Name_,NAME_,GL_TYPE_,ARRAY_LENGTH_,CppType_,MATRIX_STORAGE_CONVENTION_>;

} // end of namespace GL
} // end of namespace Leap
