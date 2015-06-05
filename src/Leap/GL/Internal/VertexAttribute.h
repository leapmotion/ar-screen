#pragma once

#include "Leap/GL/GLHeaders.h"

namespace Leap {
namespace GL {
// The contents of the Internal namespace are not intended to be used publicly, and provide
// no guarantee as to the stability of their API.  The classes and functions are used
// internally in the implementation of the publicly-presented classes.
namespace Internal {

// This is a metafunction which defines the component type and component count
// for the various shader attribute types.  TODO: figure out if there are others,
// e.g. samplers, that can be shader attributes.
template <GLenum ATTRIB_TYPE> struct VertexAttributeReflection;

#define LEAP_GL_VERTEX_ATTRIBUTE_REFLECTION(ATTRIB_TYPE, ComponentType, COMPONENT_TYPE_ENUM, COMPONENT_COUNT) \
  template <> struct VertexAttributeReflection<ATTRIB_TYPE> { \
    typedef ComponentType Component; \
    static const GLenum TYPE_ENUM = COMPONENT_TYPE_ENUM; \
    static const size_t COUNT = COMPONENT_COUNT; \
  }

LEAP_GL_VERTEX_ATTRIBUTE_REFLECTION(GL_FLOAT,             GLfloat,   GL_FLOAT,        1);
LEAP_GL_VERTEX_ATTRIBUTE_REFLECTION(GL_FLOAT_VEC2,        GLfloat,   GL_FLOAT,        2);
LEAP_GL_VERTEX_ATTRIBUTE_REFLECTION(GL_FLOAT_VEC3,        GLfloat,   GL_FLOAT,        3);
LEAP_GL_VERTEX_ATTRIBUTE_REFLECTION(GL_FLOAT_VEC4,        GLfloat,   GL_FLOAT,        4);
LEAP_GL_VERTEX_ATTRIBUTE_REFLECTION(GL_INT,               GLint,     GL_INT,          1);
LEAP_GL_VERTEX_ATTRIBUTE_REFLECTION(GL_INT_VEC2,          GLint,     GL_INT,          2);
LEAP_GL_VERTEX_ATTRIBUTE_REFLECTION(GL_INT_VEC3,          GLint,     GL_INT,          3);
LEAP_GL_VERTEX_ATTRIBUTE_REFLECTION(GL_INT_VEC4,          GLint,     GL_INT,          4);
LEAP_GL_VERTEX_ATTRIBUTE_REFLECTION(GL_UNSIGNED_INT,      GLuint,    GL_UNSIGNED_INT, 1);
LEAP_GL_VERTEX_ATTRIBUTE_REFLECTION(GL_UNSIGNED_INT_VEC2, GLuint,    GL_UNSIGNED_INT, 2);
LEAP_GL_VERTEX_ATTRIBUTE_REFLECTION(GL_UNSIGNED_INT_VEC3, GLuint,    GL_UNSIGNED_INT, 3);
LEAP_GL_VERTEX_ATTRIBUTE_REFLECTION(GL_UNSIGNED_INT_VEC4, GLuint,    GL_UNSIGNED_INT, 4);
LEAP_GL_VERTEX_ATTRIBUTE_REFLECTION(GL_BOOL,              GLboolean, GL_BOOL,         1);
LEAP_GL_VERTEX_ATTRIBUTE_REFLECTION(GL_BOOL_VEC2,         GLboolean, GL_BOOL,         2);
LEAP_GL_VERTEX_ATTRIBUTE_REFLECTION(GL_BOOL_VEC3,         GLboolean, GL_BOOL,         3);
LEAP_GL_VERTEX_ATTRIBUTE_REFLECTION(GL_BOOL_VEC4,         GLboolean, GL_BOOL,         4);
LEAP_GL_VERTEX_ATTRIBUTE_REFLECTION(GL_FLOAT_MAT2,        GLfloat,   GL_FLOAT,      2*2);
LEAP_GL_VERTEX_ATTRIBUTE_REFLECTION(GL_FLOAT_MAT2x3,      GLfloat,   GL_FLOAT,      2*3);
LEAP_GL_VERTEX_ATTRIBUTE_REFLECTION(GL_FLOAT_MAT2x4,      GLfloat,   GL_FLOAT,      2*4);
LEAP_GL_VERTEX_ATTRIBUTE_REFLECTION(GL_FLOAT_MAT3x2,      GLfloat,   GL_FLOAT,      3*2);
LEAP_GL_VERTEX_ATTRIBUTE_REFLECTION(GL_FLOAT_MAT3,        GLfloat,   GL_FLOAT,      3*3);
LEAP_GL_VERTEX_ATTRIBUTE_REFLECTION(GL_FLOAT_MAT3x4,      GLfloat,   GL_FLOAT,      3*4);
LEAP_GL_VERTEX_ATTRIBUTE_REFLECTION(GL_FLOAT_MAT4x2,      GLfloat,   GL_FLOAT,      4*2);
LEAP_GL_VERTEX_ATTRIBUTE_REFLECTION(GL_FLOAT_MAT4x3,      GLfloat,   GL_FLOAT,      4*3);
LEAP_GL_VERTEX_ATTRIBUTE_REFLECTION(GL_FLOAT_MAT4,        GLfloat,   GL_FLOAT,      4*4);

#undef LEAP_GL_VERTEX_ATTRIBUTE_REFLECTION

} // end of namespace Internal
} // end of namespace GL
} // end of namespace Leap
