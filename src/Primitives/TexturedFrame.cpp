#include "stdafx.h"
#include "TexturedFrame.h"

TexturedFrame::TexturedFrame() {
  SetBasisRectangleSize(EigenTypes::Vector2(1.0, 1.0));

  // Offsets for inner rectangle.
  SetRectangleEdgeOffset(Rectangle::INNER, RectangleEdge::TOP,    0.0);
  SetRectangleEdgeOffset(Rectangle::INNER, RectangleEdge::RIGHT,  0.0);
  SetRectangleEdgeOffset(Rectangle::INNER, RectangleEdge::BOTTOM, 0.0);
  SetRectangleEdgeOffset(Rectangle::INNER, RectangleEdge::LEFT,   0.0);
  
  // Offsets for inner rectangle.
  SetRectangleEdgeOffset(Rectangle::OUTER, RectangleEdge::TOP,    1.0);
  SetRectangleEdgeOffset(Rectangle::OUTER, RectangleEdge::RIGHT,  1.0);
  SetRectangleEdgeOffset(Rectangle::OUTER, RectangleEdge::BOTTOM, 1.0);
  SetRectangleEdgeOffset(Rectangle::OUTER, RectangleEdge::LEFT,   1.0);
  
  // Vertical edges' texture coordinates.
  SetRectangleEdgeTextureCoordinate(Rectangle::OUTER,  RectangleEdge::LEFT, 0.0f);
  SetRectangleEdgeTextureCoordinate(Rectangle::INNER,  RectangleEdge::LEFT, 0.25f);
  SetRectangleEdgeTextureCoordinate(Rectangle::INNER, RectangleEdge::RIGHT, 0.75f);
  SetRectangleEdgeTextureCoordinate(Rectangle::OUTER, RectangleEdge::RIGHT, 1.0f);

  // Horizontal edges' texture coordinates.
  SetRectangleEdgeTextureCoordinate(Rectangle::OUTER, RectangleEdge::BOTTOM, 0.0f);
  SetRectangleEdgeTextureCoordinate(Rectangle::INNER, RectangleEdge::BOTTOM, 0.25f);
  SetRectangleEdgeTextureCoordinate(Rectangle::INNER,    RectangleEdge::TOP, 0.75f);
  SetRectangleEdgeTextureCoordinate(Rectangle::OUTER,    RectangleEdge::TOP, 1.0f);

  // Set up the material properties regarding textures
  Material().Uniform<TEXTURE_MAPPING_ENABLED>() = true;
  
  m_recompute_mesh = true;
}

TexturedFrame::~TexturedFrame() { }

void TexturedFrame::SetBasisRectangleSize(const EigenTypes::Vector2& size) {
  if (m_basis_rectangle_size != size) {
    m_basis_rectangle_size = size;
    m_recompute_mesh = true;
  }
}

void TexturedFrame::SetRectangleEdgeOffset(TexturedFrame::Rectangle rect, TexturedFrame::RectangleEdge edge, double offset) {
  double &o = m_rectangle_edge_offset[size_t(rect)][size_t(edge)];
  offset = std::max(0.0, offset);
  if (o != offset) {
    o = offset;
    m_recompute_mesh = true;
  }
}

void TexturedFrame::SetRectangleEdgeTextureCoordinate(TexturedFrame::Rectangle rect, TexturedFrame::RectangleEdge edge, GLfloat tex_coord) {
  GLfloat &tc = m_rectangle_edge_texture_coordinate[size_t(rect)][size_t(edge)];
  if (tc != tex_coord) {
    tc = tex_coord;
    m_recompute_mesh = true;
  }
}

void TexturedFrame::DrawContents(RenderState& renderState) const {
  if (!m_texture) {
    return; // If the texture is not set, don't draw anything.
  }
  
  RecomputeMeshIfNecessary();
  // assert(!m_recompute_mesh);

  glEnable(GL_TEXTURE_2D);
  m_texture->Bind();
  {
    const Leap::GL::Shader &shader = Shader();
    auto locations = std::make_tuple(shader.LocationOfAttribute("position"),
                                     shader.LocationOfAttribute("normal"),
                                     shader.LocationOfAttribute("tex_coord"),
                                     shader.LocationOfAttribute("color"));
    m_mesh.Bind(locations);
    m_mesh.Draw();
    m_mesh.Unbind(locations);
  }
  m_texture->Unbind();
  glDisable(GL_TEXTURE_2D);
}

void TexturedFrame::RecomputeMeshIfNecessary() const {
  if (!m_recompute_mesh) {
    return;
  }
  
  m_mesh.Shutdown();
  
  const double bx = 0.5 * m_basis_rectangle_size(0);
  const double by = 0.5 * m_basis_rectangle_size(1);
  // The first index indicates x (0) or y (1).
  double rectangle_edge[2][4]{
    {
      -bx - RectangleEdgeOffset(Rectangle::OUTER, RectangleEdge::LEFT),
      -bx + RectangleEdgeOffset(Rectangle::INNER, RectangleEdge::LEFT),
       bx - RectangleEdgeOffset(Rectangle::INNER, RectangleEdge::RIGHT),
       bx + RectangleEdgeOffset(Rectangle::OUTER, RectangleEdge::RIGHT)
    },
    {
      -by - RectangleEdgeOffset(Rectangle::OUTER, RectangleEdge::BOTTOM),
      -by + RectangleEdgeOffset(Rectangle::INNER, RectangleEdge::BOTTOM),
       by - RectangleEdgeOffset(Rectangle::INNER, RectangleEdge::TOP),
       by + RectangleEdgeOffset(Rectangle::OUTER, RectangleEdge::TOP)
    }
  };
  // Ensure that these are non-decreasing (the inner rectangle can't have negative width or height).
  for (size_t i = 0; i < 2; ++i) {
    if (rectangle_edge[i][1] > rectangle_edge[i][2]) {
      // If the inner rectangle has negative width/height, make it have zero width/height,
      // adjusting its offsets to meet in the middle.
      rectangle_edge[i][1] = rectangle_edge[i][2] = 0.5*(rectangle_edge[i][1]+rectangle_edge[i][2]);
    }
  }
  float rectangle_edge_texture_coordinate[2][4]{
    {
      RectangleEdgeTextureCoordinate(Rectangle::OUTER, RectangleEdge::LEFT),
      RectangleEdgeTextureCoordinate(Rectangle::INNER, RectangleEdge::LEFT),
      RectangleEdgeTextureCoordinate(Rectangle::INNER, RectangleEdge::RIGHT),
      RectangleEdgeTextureCoordinate(Rectangle::OUTER, RectangleEdge::RIGHT),
    },
    {
      RectangleEdgeTextureCoordinate(Rectangle::OUTER, RectangleEdge::BOTTOM),
      RectangleEdgeTextureCoordinate(Rectangle::INNER, RectangleEdge::BOTTOM),
      RectangleEdgeTextureCoordinate(Rectangle::INNER, RectangleEdge::TOP),
      RectangleEdgeTextureCoordinate(Rectangle::OUTER, RectangleEdge::TOP),
    }
  };
  // Ensure that these are non-decreasing (the "inner rectangle" of the texture coordinates
  // can't have negative width or height).
  for (size_t i = 0; i < 2; ++i) {
    if (rectangle_edge_texture_coordinate[i][1] > rectangle_edge_texture_coordinate[i][2]) {
      // If the inner rectangle has negative width/height, make it have zero width/height,
      // adjusting its offsets to meet in the middle.
      rectangle_edge_texture_coordinate[i][1] = rectangle_edge_texture_coordinate[i][2] = 0.5f*(rectangle_edge_texture_coordinate[i][1]+rectangle_edge_texture_coordinate[i][2]);
    }
  }

  static const EigenTypes::Vector3f NORMAL = EigenTypes::Vector3f::UnitZ();
  static const EigenTypes::Vector4f COLOR = EigenTypes::Vector4f::Constant(1.0f);
  // The spatial layout of the vertices is:
  //    [0][3]    [1][3]    [2][3]    [3][3]
  //    [0][2]    [1][2]    [2][2]    [3][2]
  //    [0][1]    [1][1]    [2][1]    [3][1]
  //    [0][0]    [1][0]    [2][0]    [3][0]
  PrimitiveGeometryMesh::VertexAttributes vertex_attributes[4][4];
  for (size_t u = 0; u < 4; ++u) {
    for (size_t v = 0; v < 4; ++v) {
      vertex_attributes[u][v] = std::make_tuple(EigenTypes::Vector3f(static_cast<float>(rectangle_edge[0][u]), static_cast<float>(rectangle_edge[1][v]), 0.0f),
                                                NORMAL,
                                                EigenTypes::Vector2f(rectangle_edge_texture_coordinate[0][u], rectangle_edge_texture_coordinate[1][v]),
                                                COLOR);
    }
  }

  PrimitiveGeometryMeshAssembler mesh_assembler(GL_TRIANGLES);
  for (size_t u = 0; u < 3; ++u) {
    for (size_t v = 0; v < 3; ++v) {
      mesh_assembler.PushQuad(vertex_attributes[u+0][v+0],
                              vertex_attributes[u+1][v+0],
                              vertex_attributes[u+1][v+1],
                              vertex_attributes[u+0][v+1]);
    }
  }
  mesh_assembler.InitializeMesh(m_mesh);
  assert(m_mesh.IsInitialized());
  
  m_recompute_mesh = false;
}
