#pragma once

#include "Leap/GL/GLHeaders.h"
#include "Leap/GL/BufferObject.h"
#include "Leap/GL/Internal/Meta.h"
#include "Leap/GL/ResourceBase.h"
#include "Leap/GL/VertexAttribute.h"
#include "Leap/GL/VertexBufferObjectException.h"

#include <cassert>
#include <tuple>
#include <type_traits>
#include <vector>

namespace Leap {
namespace GL {

/// @brief Encapsulates the concept of an OpenGL vertex buffer object.
/// @details A vertex buffer object is an array of vertex attributes that are uploaded 
/// to the GPU for use in a vertex shader program.  The vertex attributes correspond to
/// "attribute" variables in the vertex shader.  The vertex shader program is called on each
/// vertex, and each vertex must have each of the required attributes defined for it.  Thus
/// the set of attributes must be well-defined.  This is done via the variadic AttributeTypes
/// template parameter(s).
///
/// The only exceptions that this class explicitly throws derive from
/// Leap::GL::VertexBufferObjectException.
///
/// For more info, see https://www.opengl.org/wiki/Vertex_Specification#Vertex_Buffer_Object
template <typename... AttributeTypes>
class VertexBufferObject : public ResourceBase<VertexBufferObject<AttributeTypes...>> {
public:

  /// @brief Convenience typedef for the type of a single vertex attribute.
  typedef std::tuple<AttributeTypes...> Attributes;
  /// @brief Number of attributes specified in this VertexBufferObject.
  static const size_t ATTRIBUTE_COUNT = std::tuple_size<Attributes>::value;
  /// @brief Data type which holds the locations for the respective attributes.
  typedef typename Internal::UniformTuple<ATTRIBUTE_COUNT,GLint>::T AttributeLocations;

  /// @brief Construct an un-Initialize-d VertexBufferObject which has not acquired any GL (or other) resources.
  /// @details It will be necessary to call Initialize on this object to use it.
  VertexBufferObject ()
    : m_usage_pattern(GL_INVALID_ENUM)
  { }
  /// @brief Convenience constructor that will call Initialize with the given arguments.
  VertexBufferObject (const Attributes *vertex_attribute_data, size_t vertex_count, GLenum usage_pattern)
    : m_usage_pattern(GL_INVALID_ENUM)
  {
    Initialize(usage_pattern, vertex_attribute_data, vertex_count);
  }
  /// @brief Destructor will call Shutdown.
  ~VertexBufferObject () {
    Shutdown();
  }

  using ResourceBase<VertexBufferObject<AttributeTypes...>>::IsInitialized;
  using ResourceBase<VertexBufferObject<AttributeTypes...>>::Initialize;
  using ResourceBase<VertexBufferObject<AttributeTypes...>>::Shutdown;

  /// @brief Returns the usage pattern used in upload operations.
  GLenum UsagePattern () const {
    if (!IsInitialized()) {
      throw VertexBufferObjectException("A VertexBufferObject that !IsInitialized() has no UsagePattern value.");
    }
    return m_usage_pattern;
  }

  /// @brief This method calls glEnableVertexAttribArray and glVertexAttribPointer on each
  /// of the vertex attributes given valid locations (i.e. not equal to -1).
  /// @details The tuple argument attribute_locations must correspond exactly to Attributes
  /// (which is a tuple of VertexAttribute types defined by this VertexBufferObject).
  void Enable (const AttributeLocations &attribute_locations) const {
    if (!IsInitialized()) {
      throw VertexBufferObjectException("Can't call VertexBufferObject::Enable on a VertexBufferObject that is !IsInitialized().");
    }
    m_gl_buffer_object.Bind();
    // Begin iterated binding of vertex attributes starting at the 0th one.
    EnableAndIterate<0>(attribute_locations, sizeof(Attributes));
    m_gl_buffer_object.Unbind();
  }
  /// @brief This method calls glDisableVertexAttribArray on each of the vertex attributes
  /// given valid locations (i.e. not equal to -1).
  /// @details This method is analogous to the Enable method.
  static void Disable (const AttributeLocations &attribute_locations) {
    // Begin iterated unbinding of vertex attributes starting at the 0th one.
    DisableAndIterate<0>(attribute_locations);
  }

private:

  // This is one iteration of the Enable method.  It calls the next iteration.
  template <size_t INDEX>
  static typename std::enable_if<(INDEX<ATTRIBUTE_COUNT),void>::type
    EnableAndIterate (const AttributeLocations &locations, size_t stride)
  {
    // Get the INDEXth attribute type.
    typedef typename std::tuple_element<INDEX,Attributes>::type AttributeType;
    // Get the INDEXth location value.
    GLint location = std::get<INDEX>(locations);
    // Compute the offset of the current attribute into the Attributes tuple type.  It is NOT
    // necessarily layed out in memory in the same order as the variadic template parameters!
    static const Attributes A; // This is not actually used for anything at runtime, just for determining the offset.
    static const size_t OFFSET_OF_INDEXth_ATTRIBUTE = static_cast<size_t>(reinterpret_cast<const uint8_t *>(&std::get<INDEX>(A)) - reinterpret_cast<const uint8_t *>(&A));
    // Call the Enable method of the INDEXth attribute type with the INDEXth location value, etc.
    AttributeType::Enable(location, static_cast<GLsizei>(stride), static_cast<GLsizei>(OFFSET_OF_INDEXth_ATTRIBUTE));
    // Increment INDEX and call this method again (this is a meta-program for loop).
    EnableAndIterate<INDEX+1>(locations, stride);
  }
  // This is the end of the iteration in the Enable method.
  template <size_t INDEX>
  static typename std::enable_if<(INDEX>=ATTRIBUTE_COUNT),void>::type
    EnableAndIterate (const AttributeLocations &locations, size_t stride)
  {
    // Iteration complete -- do nothing.
  }

  // This is one iteration of the Disable method.  It calls the next iteration.
  template <size_t INDEX>
  static typename std::enable_if<(INDEX<ATTRIBUTE_COUNT),void>::type
    DisableAndIterate (const AttributeLocations &locations)
  {
    // Get the INDEXth attribute type.
    typedef typename std::tuple_element<INDEX,Attributes>::type AttributeType;
    // Get the INDEXth location value.
    GLint location = std::get<INDEX>(locations);
    // Call the Disable method of the INDEXth attribute type with the INDEXth location value.
    AttributeType::Disable(location);
    // Increment INDEX and call this method again (this is a meta-program for loop).
    DisableAndIterate<INDEX+1>(locations);
  }
  // This is the end of the iteration in the Disable method.
  template <size_t INDEX>
  static typename std::enable_if<(INDEX>=ATTRIBUTE_COUNT),void>::type
    DisableAndIterate (const AttributeLocations &locations)
  {
    // Iteration complete -- do nothing.
  }

  friend class ResourceBase<VertexBufferObject<AttributeTypes...>>;

  bool IsInitialized_Implementation () const { return m_gl_buffer_object.IsInitialized(); }
  void Initialize_Implementation (const Attributes *vertex_attribute_data, size_t vertex_count, GLenum usage_pattern) {
    assert(m_usage_pattern == GL_INVALID_ENUM);
    assert(!m_gl_buffer_object.IsInitialized());

    if (vertex_attribute_data == nullptr) {
      throw VertexBufferObjectException("vertex_attribute_data must be a valid pointer.");
    }
    if (vertex_count == 0) {
      throw VertexBufferObjectException("vertex_count must be positive.");
    }

    switch (usage_pattern) {
      case GL_STREAM_DRAW:
      case GL_STREAM_READ:
      case GL_STREAM_COPY:
      case GL_STATIC_DRAW:
      case GL_STATIC_READ:
      case GL_STATIC_COPY:
      case GL_DYNAMIC_DRAW:
      case GL_DYNAMIC_READ:
      case GL_DYNAMIC_COPY:
        m_usage_pattern = usage_pattern;
        break; // Ok
      default:
        throw VertexBufferObjectException("usage_pattern must be one of GL_STREAM_DRAW, GL_STREAM_READ, GL_STREAM_COPY, GL_STATIC_DRAW, GL_STATIC_READ, GL_STATIC_COPY, GL_DYNAMIC_DRAW, GL_DYNAMIC_READ, GL_DYNAMIC_COPY.");
    }

    // TODO: correct exception handling with cleanup

    m_gl_buffer_object.Initialize(GL_ARRAY_BUFFER);
    m_gl_buffer_object.Bind();
    m_gl_buffer_object.BufferData(static_cast<const void *>(vertex_attribute_data), vertex_count*sizeof(Attributes), m_usage_pattern);
    m_gl_buffer_object.Unbind();

    assert(IsInitialized());
  }
  void Shutdown_Implementation () {
    m_usage_pattern = GL_INVALID_ENUM;
    m_gl_buffer_object.Shutdown();
    assert(!IsInitialized());
  }

  GLenum m_usage_pattern;
  BufferObject m_gl_buffer_object;
};

} // end of namespace GL
} // end of namespace Leap
