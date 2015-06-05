#pragma once

#include "Leap/GL/GLHeaders.h" // convenience header for cross-platform GL includes

namespace Leap {
namespace GL {
// The contents of the Internal namespace are not intended to be used publicly, and provide
// no guarantee as to the stability of their API.  The classes and functions are used
// internally in the implementation of the publicly-presented classes.
namespace Internal {

// helper metafunction for simplifying the uniform modifiers
template <typename GLType_, size_t COMPONENT_COUNT_> struct UniformFunction { static const bool exists = false; };

// helper metafunction for simplifying the uniform matrix modifiers
template <size_t ROWS_, size_t COLUMNS_> struct UniformMatrixFunction { static const bool exists = false; };

// Template specializations of UniformFunction and UniformMatrixFunction.

/// @cond false
// we don't want these showing up in the class list.
template <> struct UniformFunction<GLint,1> { static const bool exists = true; static void eval (GLint location, GLsizei count, const GLint *value) { glUniform1iv(location, count, value); } };
template <> struct UniformFunction<GLint,2> { static const bool exists = true; static void eval (GLint location, GLsizei count, const GLint *value) { glUniform2iv(location, count, value); } };
template <> struct UniformFunction<GLint,3> { static const bool exists = true; static void eval (GLint location, GLsizei count, const GLint *value) { glUniform3iv(location, count, value); } };
template <> struct UniformFunction<GLint,4> { static const bool exists = true; static void eval (GLint location, GLsizei count, const GLint *value) { glUniform4iv(location, count, value); } };
template <> struct UniformFunction<GLfloat,1> { static const bool exists = true; static void eval (GLint location, GLsizei count, const GLfloat *value) { glUniform1fv(location, count, value); } };
template <> struct UniformFunction<GLfloat,2> { static const bool exists = true; static void eval (GLint location, GLsizei count, const GLfloat *value) { glUniform2fv(location, count, value); } };
template <> struct UniformFunction<GLfloat,3> { static const bool exists = true; static void eval (GLint location, GLsizei count, const GLfloat *value) { glUniform3fv(location, count, value); } };
template <> struct UniformFunction<GLfloat,4> { static const bool exists = true; static void eval (GLint location, GLsizei count, const GLfloat *value) { glUniform4fv(location, count, value); } };

template <> struct UniformMatrixFunction<2,2> { static const bool exists = true; static void eval (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) { glUniformMatrix2fv(location, count, transpose, value); } };
template <> struct UniformMatrixFunction<2,3> { static const bool exists = true; static void eval (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) { glUniformMatrix2x3fv(location, count, transpose, value); } };
template <> struct UniformMatrixFunction<2,4> { static const bool exists = true; static void eval (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) { glUniformMatrix2x4fv(location, count, transpose, value); } };
template <> struct UniformMatrixFunction<3,2> { static const bool exists = true; static void eval (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) { glUniformMatrix3x2fv(location, count, transpose, value); } };
template <> struct UniformMatrixFunction<3,3> { static const bool exists = true; static void eval (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) { glUniformMatrix3fv(location, count, transpose, value); } };
template <> struct UniformMatrixFunction<3,4> { static const bool exists = true; static void eval (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) { glUniformMatrix3x4fv(location, count, transpose, value); } };
template <> struct UniformMatrixFunction<4,2> { static const bool exists = true; static void eval (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) { glUniformMatrix4x2fv(location, count, transpose, value); } };
template <> struct UniformMatrixFunction<4,3> { static const bool exists = true; static void eval (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) { glUniformMatrix4x3fv(location, count, transpose, value); } };
template <> struct UniformMatrixFunction<4,4> { static const bool exists = true; static void eval (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) { glUniformMatrix4fv(location, count, transpose, value); } };
/// @endcond

} // end of namespace Internal
} // end of namespace GL
} // end of namespace Leap
