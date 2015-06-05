#pragma once

#include <cstdint>
#include "Leap/GL/BufferObject.h"
#include "Leap/GL/Error.h"
#include "Leap/GL/MeshException.h"
#include "Leap/GL/ResourceBase.h"
#include "Leap/GL/VertexBufferObject.h"
#include <map>
#include <vector>

namespace Leap {
namespace GL {

/// @brief Provides an abstract means for creating indexed vertex buffer objects.
/// @details A mesh must be Initialize()d with a "draw mode" (see
/// http://www.lighthouse3d.com/tutorials/glsl-core-tutorial/primitive-assembly/ for a visual description
/// of the various draw modes).  There is a vector of vertex attributes elements which are accessible via
/// the IntermediateVertices method and should be populated before calling UploadIntermediateVertices.
/// The UploadIntermediateVertices method will compute a map of unique vertices and compute the index array
/// which will be used in the Draw method (supplied to glDrawElements).
///
/// This class inherits ResourceBase and thereby follows the resource conventions specified there.
///
/// The @c MeshAssembler class can be used to construct mesh vertex data in a convenient way.
template <typename... AttributeTypes>
class Mesh : public ResourceBase<Mesh<AttributeTypes...>> {
public:

  typedef VertexBufferObject<AttributeTypes...> VBO;
  typedef typename VBO::Attributes VertexAttributes;

  /// @brief Construct an un-Initialize-d Mesh which has not acquired any GL (or other) resources.
  /// @details It will be necessary to call Initialize on this object to use it.
  Mesh ()
    : m_draw_mode(GL_INVALID_ENUM)
    , m_index_count(0)
  { }
  /// @brief Convenience constructor that will call Initialize with the given arguments.
  Mesh (const VertexAttributes *vertex_attribute_data, size_t vertex_count, GLenum draw_mode)
    : m_draw_mode(GL_INVALID_ENUM)
    , m_index_count(0)
  {
    Initialize(vertex_attribute_data, vertex_count, draw_mode);
  }
  /// @brief Destructor will call Shutdown.
  ~Mesh () {
    Shutdown();
  }

  using ResourceBase<Mesh<AttributeTypes...>>::IsInitialized;
  using ResourceBase<Mesh<AttributeTypes...>>::Initialize;
  using ResourceBase<Mesh<AttributeTypes...>>::Shutdown;

  /// @brief Returns the draw mode (see the API docs for glDrawElements) passed to Initialize.
  /// @details Throws MeshException if !IsInitialized.
  GLenum DrawMode () const {
    if (!IsInitialized()) {
      throw MeshException("A Mesh object has no DrawMode value if !IsInitialized.");
    }
    return m_draw_mode;
  }

  // TODO: think about if these functions really belong here, or if the user should call the VBO and
  // index buffer bind methods and the draw methods themselves.

  /// @brief Binds this Mesh so that it is ready to be Draw()n.
  /// @details Binds this mesh (vertex buffer, index buffer) so that it can be rendered
  /// via glDrawElements.  The locations of the respective vertex attributes must be supplied
  /// here.  Specifying -1 for any individual attribute location will cause that attribute to
  /// go unused -- this allows shaders that don't have all the attributes in this Mesh to still
  /// be usable.
  // TODO: write about what GL operations actually happen.
  void Bind (typename VBO::AttributeLocations &attribute_locations) const {
    if (!IsInitialized()) {
      throw MeshException("Can't Bind a Mesh if it !IsInitialized.");
    }
    // This calls glEnableVertexAttribArray and glVertexAttribPointer on the relevant things.
    m_vertex_buffer.Enable(attribute_locations);
    m_index_buffer.Bind();
  }
  /// @brief Draws a bound Mesh by calling glDrawElements.
  void Draw () const {
    if (!IsInitialized()) {
      throw MeshException("Can't Draw a Mesh if it !IsInitialized.");
    }
    THROW_UPON_GL_ERROR(glDrawElements(m_draw_mode, m_index_count, GL_UNSIGNED_INT, 0));
  }
  /// @brief Unbinds this mesh.
  /// @details Must pass in the same attribute_locations as to the call to Bind.
  // TODO: write about what GL operations actually happen.
  void Unbind (typename VBO::AttributeLocations &attribute_locations) const {
    if (!IsInitialized()) {
      throw MeshException("Can't Unbind a Mesh if it !IsInitialized.");
    }
    m_index_buffer.Unbind();
    // This calls glDisableVertexAttribArray on the relevant things.
    m_vertex_buffer.Disable(attribute_locations);
  }

private:

  friend class ResourceBase<Mesh<AttributeTypes...>>;

  bool IsInitialized_Implementation () const { return m_draw_mode != GL_INVALID_ENUM; }
  void Initialize_Implementation (const VertexAttributes *vertex_attribute_data, size_t vertex_count, GLenum draw_mode) {
    if (vertex_attribute_data == nullptr) {
      throw MeshException("vertex_attribute_data must be a valid pointer.");
    }
    if (vertex_count == 0) {
      throw MeshException("vertex_count must be positive.");
    }

    switch (draw_mode) {
      case GL_POINTS:
      case GL_LINE_STRIP:
      case GL_LINE_LOOP:
      case GL_LINES:
      case GL_LINE_STRIP_ADJACENCY:
      case GL_LINES_ADJACENCY:
      case GL_TRIANGLE_STRIP:
      case GL_TRIANGLE_FAN:
      case GL_TRIANGLES:
      case GL_TRIANGLE_STRIP_ADJACENCY:
      case GL_TRIANGLES_ADJACENCY:
        m_draw_mode = draw_mode;
        break;
      default:
        throw MeshException("Invalid draw mode -- must be one of GL_POINTS, GL_LINE_STRIP, GL_LINE_LOOP, GL_LINES, GL_LINE_STRIP_ADJACENCY, GL_LINES_ADJACENCY, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_TRIANGLES, GL_TRIANGLE_STRIP_ADJACENCY and GL_TRIANGLES_ADJACENCY (see OpenGL 3.3 docs for glDrawElements).");
    }

    // TODO: move into the function which uses this.
    struct VertexAttributesCompare {
      bool operator () (const VertexAttributes &lhs, const VertexAttributes &rhs) const {
        return memcmp(reinterpret_cast<const void *>(&lhs), reinterpret_cast<const void *>(&rhs), sizeof(VertexAttributes)) < 0;
      }
    };
    typedef std::map<VertexAttributes,GLuint,VertexAttributesCompare> VertexIndexMap;

    // Eliminate duplicate vertices using a temporary map.
    VertexIndexMap vertex_attributes_index_map;
    std::vector<GLuint> indices;
    std::vector<VertexAttributes> unique_vertex_attributes;
    // This loop will reduce vertex_attribute_data down into unique_vertex_attributes.
    for (size_t i = 0; i < vertex_count; ++i) {
      const VertexAttributes &vertex_attributes = vertex_attribute_data[i];
      auto mapped_vertex_attribute = vertex_attributes_index_map.find(vertex_attributes);
      // If the current vertex is not in the vbo map already, add it.
      if (mapped_vertex_attribute == vertex_attributes_index_map.end()) {
        GLuint new_index = static_cast<GLuint>(vertex_attributes_index_map.size());
        indices.push_back(new_index);
        unique_vertex_attributes.push_back(vertex_attributes);
        vertex_attributes_index_map[vertex_attributes] = new_index;
      } else { // Otherwise, add the existing vertex's index.
        indices.push_back(mapped_vertex_attribute->second);
      }
    }

    // TODO: correct exception handling with cleanup

    m_vertex_buffer.Initialize(unique_vertex_attributes.data(), unique_vertex_attributes.size(), GL_STATIC_DRAW);

    m_index_count = indices.size();

    m_index_buffer.Initialize(GL_ELEMENT_ARRAY_BUFFER);
    m_index_buffer.Bind();
    // If only the vertex attribute data is going to change (and not the )
    m_index_buffer.BufferData(static_cast<const void *>(indices.data()), indices.size()*sizeof(GLuint), GL_STATIC_DRAW);
    m_index_buffer.Unbind();
  }
  void Shutdown_Implementation () {
    m_draw_mode = GL_INVALID_ENUM;
    m_vertex_buffer.Shutdown();
    m_index_count = 0;
    m_index_buffer.Shutdown();
  }

  // The draw mode that will be used in Draw().
  GLenum m_draw_mode;
  // This is the vertex buffer object for uploaded vertex attribute data.
  VBO m_vertex_buffer;
  // This is the number of indices used to pass to glDrawElements when drawing the VBO.
  // This could be derived from m_index_buffer.Size() / sizeof(GLuint).
  size_t m_index_count;
  // This is the buffer containing the index elements.
  BufferObject m_index_buffer;
};

} // end of namespace GL
} // end of namespace Leap
