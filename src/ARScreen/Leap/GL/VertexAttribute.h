#pragma once

#include "Leap/GL/GLHeaders.h"
#include "Leap/GL/Internal/VertexAttribute.h"

namespace Leap {
namespace GL {

// TODO: enabling use of the "normalized" boolean parameter in glVertexAttribPointer.
// This represents a single, typed attribute in a VertexBufferObject.  It is a single
// array of fixed-typed components, defined via VertexAttributeReflection by the
// ATTRIB_TYPE template parameter.  Using the "As" methods, the array can be accessed
// and modified as whatever POD type (of correct size) is desired.

/// @brief C++ class which abstracts the OpenGL concept of a vertex attribute.
/// @details A vertex attribute corresponds to an attribute variable in a shader, and each variable
/// has a GLenum-specified type (e.g. GL_BOOL, GL_FLOAT_VEC3, GL_FLOAT_MAT4, etc).  This class provides
/// typed storage for vertex attribute data, type information (such as base type and component count),
/// and convenient methods for enabling and disabling vertex attribute arrays.
///
/// Each vertex attribute is [potentially] a compound data type consisting of an array of base type
/// components.  For example, GL_FLOAT_VEC3 has base type GLfloat, base type enum GL_FLOAT, and component
/// count 3.  GL_BOOL has base type GLboolean, base type enum GL_BOOL, and component count 1.  GL_FLOAT_MAT2x3
/// has base type GLfloat, base type enum GL_FLOAT, and component count 6 (i.e. 2*3).
///
/// The data stored by VertexAttribute is simply an array of base type having length equal to the number
/// of components in the attribute type.
///
/// Access to the stored vertex attribute value is provided via the templatized ReinterpretAs<T> methods,
/// which return a const or non-const reference to this, reinterpret_cast'ed to the requested type.  This
/// allows access and manipulation of the vertex attribute data as any type that has the same memory layout.
/// For example,
/// @verbatim
/// VertexAttribute<GL_FLOAT_VEC3> a;
/// a.ReinterpretAs<std::array<GLfloat,3>>() = {{1.0f, 3.0f, 6.0f}};
/// @endverbatim
///
/// For more info, see https://www.opengl.org/wiki/Vertex_Attribute
template <GLenum ATTRIB_TYPE>
class VertexAttribute {
public:

  /// @brief The base type for the attribute (e.g. GLfloat, GLboolean, etc).
  typedef typename Internal::VertexAttributeReflection<ATTRIB_TYPE>::Component ComponentType;
  /// @brief The OpenGL enum specifying the base type (e.g. GL_FLOAT, GL_BOOL, etc).
  static const GLenum COMPONENT_TYPE_ENUM = Internal::VertexAttributeReflection<ATTRIB_TYPE>::TYPE_ENUM;
  /// @brief The number of components in the attribute (e.g. GL_FLOAT_VEC4 has 4 components).
  static const size_t COMPONENT_COUNT = Internal::VertexAttributeReflection<ATTRIB_TYPE>::COUNT;

  /// @brief Default constructor does not initialize the storage.
  VertexAttribute () { }
  /// @brief Templatized copy constructor assigns value to ReinterpretAs<T>(), thereby allowing initialization
  /// of this object from any compatible type.
  template <typename T>
  VertexAttribute (const T &value) {
    ReinterpretAs<T>() = value;
  }

  /// @brief Returns a const reference to the stored data reintepret_cast'ed to the requested type T.
  /// @details The type T must be a standard-layout type having the same size as this object, and must
  /// consist of exactly COMPONENT_COUNT elements of ComponentType.
  template <typename T>
  const T &ReinterpretAs () const {
    static_assert(sizeof(T) == COMPONENT_COUNT*sizeof(ComponentType), "T must be a standard-layout mapping directly onto the component array");
    static_assert(std::is_standard_layout<T>::value, "T must be a standard-layout mapping directly onto the component array");
    // TODO: somehow check that T consists of exactly COMPONENT_COUNT elements of ComponentType.
    return *reinterpret_cast<const T *>(&m_components[0]);
  }
  /// @brief Returns a non-const reference to the stored data reintepret_cast'ed to the requested type T.
  /// @details The type T must be a standard-layout type having the same size as this object, and must
  /// consist of exactly COMPONENT_COUNT elements of ComponentType.  This method can be used to modify the
  /// stored data as any compatible data type.
  template <typename T>
  T &ReinterpretAs () {
    static_assert(sizeof(T) == COMPONENT_COUNT*sizeof(ComponentType), "T must be a standard-layout mapping directly onto the component array");
    static_assert(std::is_standard_layout<T>::value, "T must be a standard-layout mapping directly onto the component array");
    // TODO: somehow check that T consists of exactly COMPONENT_COUNT elements of ComponentType.
    return *reinterpret_cast<T *>(&m_components[0]);
  }

  /// @brief Enable the vertex attribute array for the given attribute location and set the vertex attribute pointer.
  /// @details This method assumes that the vertex buffer that this attribute is associated with is currently
  /// bound.  An attribute's use is optional.  Specifically, if a particular vertex shader doesn't have a
  /// particular attribute present in a given VertexBufferObject, that VertexBufferObject can still be used -- the missing
  /// attributes will be ignored.  Specifying -1 for location indicates that this attribute should not be used.
  /// The calls to glVertexAttribPointer use the currently bound buffer object.
  static void Enable (GLint location, GLsizei stride, GLsizei offset) {
    // NOTE: There being no 
    //   "else { glDisableVertexAttribArray(...); }"
    // statement relies on each vertex attrib array being disabled to begin with.
    if (location != -1) {
      glEnableVertexAttribArray(location);
      glVertexAttribPointer(location, COMPONENT_COUNT, COMPONENT_TYPE_ENUM, GL_FALSE, stride, reinterpret_cast<void*>(offset));
    }
  }
  /// @brief Disables the vertexa ttribute array for the given attribute location.
  /// @details As in @c Enable, only does anything if location is not -1.
  static void Disable (GLint location) {
    if (location != -1) {
      glDisableVertexAttribArray(location);
    }
  }

private:

  ComponentType m_components[COMPONENT_COUNT];
};

} // end of namespace GL
} // end of namespace Leap
