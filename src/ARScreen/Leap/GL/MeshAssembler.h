#pragma once

#include "Leap/GL/Mesh.h"
#include "Leap/GL/MeshException.h"
#include "Leap/GL/ResourceBase.h"
#include <vector>

namespace Leap {
namespace GL {

/// @brief MeshAssembler provides a means for generating arrays of vertices to supply to Mesh::Initialize.
/// @details A MeshAssembler object must be initialized with a draw mode (e.g. GL_POINTS, GL_LINES,
/// GL_TRIANGLES, etc -- see glDrawElements), indicating how the vertex data should be used for
/// rendering.  A MeshAssembler object has a vector of vertex attributes which are collected by the
/// Push* methods, or can be accessed/modified directly using the Vertices methods.
///
/// This class inherits ResourceBase and thereby follows the resource conventions specified there.
template <typename... AttributeTypes>
class MeshAssembler : public ResourceBase<MeshAssembler<AttributeTypes...>> {
public:

  typedef typename Mesh<AttributeTypes...>::VertexAttributes VertexAttributes;

  /// @brief Construct an un-Initialize-d MeshAssembler which has not acquired any GL (or other) resources.
  /// @details It will be necessary to call Initialize on this object to use it.
  MeshAssembler ()
    : m_draw_mode(GL_INVALID_ENUM)
  { }
  /// @brief Convenience constructor that will call Initialize with the given arguments.
  MeshAssembler (GLenum draw_mode)
    : m_draw_mode(GL_INVALID_ENUM)
  {
    Initialize(draw_mode);
  }
  /// @brief Destructor will call Shutdown.
  ~MeshAssembler () {
    Shutdown();
  }

  using ResourceBase<MeshAssembler<AttributeTypes...>>::IsInitialized;
  using ResourceBase<MeshAssembler<AttributeTypes...>>::Initialize;
  using ResourceBase<MeshAssembler<AttributeTypes...>>::Shutdown;

  /// @brief Returns the draw mode for this MeshAssembler, or throws MeshException if !IsInitialized.
  GLenum DrawMode () const {
    if (!IsInitialized()) {
      throw MeshException("MeshAssembler has no DrawMode value if !IsInitialized().");
    }
    return m_draw_mode;
  }

  /// @brief Returns a const reference to the vertices for this MeshAssembler, or throws MeshException if !IsInitialized.
  const std::vector<VertexAttributes> &Vertices () const {
    if (!IsInitialized()) {
      throw MeshException("MeshAssembler has no Vertices value if !IsInitialized().");
    }
    return m_vertices;
  }
  /// @brief Returns a non-const reference to the vertices for this MeshAssembler, or throws MeshException if !IsInitialized.
  std::vector<VertexAttributes> &Vertices () {
    if (!IsInitialized()) {
      throw MeshException("MeshAssembler has no Vertices value if !IsInitialized().");
    }
    return m_vertices;
  }

  /// @brief Calls Initialize on the specified Mesh using the current vertices.
  /// @details Note that the Mesh will be Shutdown if it IsInitialized.
  void InitializeMesh (Mesh<AttributeTypes...> &mesh) const {
    if (!IsInitialized()) {
      throw MeshException("Can't call InitializeMesh on a MeshAssembler object that !IsInitialized().");
    }
    mesh.Initialize(m_vertices.data(), m_vertices.size(), m_draw_mode);
  }

  /// @brief Push a single vertex to the vertices vector.
  /// @details This is to be used for any draw mode -- the user is responsible for adding
  /// vertices using the convention defined by the draw mode, and in particular is the
  /// only method allowable for adding vertices for some draw modes.
  template <typename... Types_>
  void PushVertex (Types_... args) {
    if (!IsInitialized()) {
      throw MeshException("Can't push vertex data into a MeshAssembler that !IsInitialized().");
    }
    m_vertices.emplace_back(args...);
  }
  /// @brief Push two vertices which define a single line segment.
  /// @details This is to be used only when the draw mode is GL_LINES.
  void PushLine (const VertexAttributes &v0,
                 const VertexAttributes &v1) {
    if (!IsInitialized()) {
      throw MeshException("Can't push vertex data into a MeshAssembler that !IsInitialized().");
    }
    if (m_draw_mode != GL_LINES) {
      throw MeshException("Mesh::PushLine is only defined if the draw mode is GL_LINES.");
    }
    m_vertices.emplace_back(v0);
    m_vertices.emplace_back(v1);
  }
  /// @brief Push an ordered list three vertices which define a single triangle.
  /// @details This is to be used only when the draw mode is GL_TRIANGLES.
  void PushTriangle (const VertexAttributes &v0,
                     const VertexAttributes &v1,
                     const VertexAttributes &v2) {
    if (!IsInitialized()) {
      throw MeshException("Can't push vertex data into a MeshAssembler that !IsInitialized().");
    }
    if (m_draw_mode != GL_TRIANGLES) {
      throw MeshException("Mesh::PushTriangle is only defined if the draw mode is GL_TRIANGLES.");
    }
    m_vertices.emplace_back(v0);
    m_vertices.emplace_back(v1);
    m_vertices.emplace_back(v2);
  }
  /// @brief Push an ordered list of four vertices which define a two triangles and therefore a quadrilateral.
  /// @details This is to be used only when the draw mode is GL_TRIANGLES.
  void PushQuad (const VertexAttributes &v0,
                 const VertexAttributes &v1,
                 const VertexAttributes &v2,
                 const VertexAttributes &v3) {
    if (!IsInitialized()) {
      throw MeshException("Can't push vertex data into a MeshAssembler that !IsInitialized().");
    }
    if (m_draw_mode != GL_TRIANGLES) {
      throw MeshException("Mesh::PushQuad is only defined if the draw mode is GL_TRIANGLES.");
    }
    // Triangle 1
    m_vertices.emplace_back(v0);
    m_vertices.emplace_back(v1);
    m_vertices.emplace_back(v2);
    // Triangle 2
    m_vertices.emplace_back(v0);
    m_vertices.emplace_back(v2);
    m_vertices.emplace_back(v3);
  }
  /// @brief Push an ordered list of four vertices which define a single `line adjacency`.
  /// @details This is to be used only when the draw mode is GL_LINES_ADJACENCY.
  void PushLineAdjacency (const VertexAttributes &v0,
                          const VertexAttributes &v1,
                          const VertexAttributes &v2,
                          const VertexAttributes &v3) {
    if (!IsInitialized()) {
      throw MeshException("Can't push vertex data into a MeshAssembler that !IsInitialized().");
    }
    if (m_draw_mode != GL_LINES_ADJACENCY) {
      throw MeshException("Mesh::PushLineAdjacency is only defined if the draw mode is GL_LINES_ADJACENCY.");
    }
    m_vertices.emplace_back(v0);
    m_vertices.emplace_back(v1);
    m_vertices.emplace_back(v2);
    m_vertices.emplace_back(v3);
  }
  /// @brief Push an ordered list of six vertices which define a single `triangle adjacency`.
  /// @details This is to be used only when the draw mode is GL_TRIANGLES_ADJACENCY.
  void PushTriangleAdjacency (const VertexAttributes &v0,
                              const VertexAttributes &v1,
                              const VertexAttributes &v2,
                              const VertexAttributes &v3,
                              const VertexAttributes &v4,
                              const VertexAttributes &v5) {
    if (!IsInitialized()) {
      throw MeshException("Can't push vertex data into a MeshAssembler that !IsInitialized().");
    }
    if (m_draw_mode != GL_LINES) {
      throw MeshException("Mesh::PushTriangleAdjacency is only defined if the draw mode is GL_TRIANGLES_ADJACENCY.");
    }
    m_vertices.emplace_back(v0);
    m_vertices.emplace_back(v1);
    m_vertices.emplace_back(v2);
    m_vertices.emplace_back(v3);
    m_vertices.emplace_back(v4);
    m_vertices.emplace_back(v5);
  }

private:

  friend class ResourceBase<MeshAssembler<AttributeTypes...>>;

  bool IsInitialized_Implementation () const { return m_draw_mode != GL_INVALID_ENUM; }
  void Initialize_Implementation (GLenum draw_mode) {
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
  }
  void Shutdown_Implementation () {
    m_draw_mode = GL_INVALID_ENUM;
    m_vertices.clear();
  }

  // Indicates what geometric primitives the vertices are specifying (e.g. GL_TRIANGLES, GL_LINE_STRIP, etc).
  GLenum m_draw_mode;
  // This intermediate storage for vertex data as it is being generated, and may be cleared during upload.
  std::vector<VertexAttributes> m_vertices;
};

} // end of namespace GL
} // end of namespace Leap
