#pragma once

#include "Primitives.h"

#include "Leap/GL/Texture2.h"

// Design notes for TexturedFrame
// ------------------------------
// This primitive will provide a configurable, measured, 9-cell frame of the following form:
//
//   +---+-------------------+---+
//   | 0 |         1         | 2 |
//   +---+-------------------+---+
//   |   |                   |   |
//   | 3 |         4         | 5 |
//   |   |                   |   |
//   +---+-------------------+---+
//   | 6 |         7         | 8 |
//   +---+-------------------+---+
//
// The outer cells constitute the "border" of the frame.  The "outer rectangle" of the frame
// is made up by the outer edges of the border:
//
//   +---+-------------------+---+
//   | 0           1           2 |
//   +                           +
//   |                           |
//   | 3           4           5 |
//   |                           |
//   +                           +
//   | 6           7           8 |
//   +---+-------------------+---+
//
// The "inner rectangle" of the frame is made up by the inner edges of the border:
//
//                                
//     0           1           2  
//       +-------------------+    
//       |                   |    
//     3 |         4         | 5  
//       |                   |    
//       +-------------------+    
//     6           7           8  
//                                
//
// The inner rectangle must not exceed the outer rectangle (though they are allowed to be the same
// size).  The inner and outer rectangles' edges are configurable based on a "basis rectangle", which
// must be specified by the user of this primitive.  The default values are:
// - The inner rectangle will be set to the basis rectangle.
// - The outer rectangle will extend half a unit out from the basis rectangle in each direction.
//
// Each rectangle edge is independent, and so non-symmetric borders can be defined.  The inner and
// outer rectangles are controlled by an offset value for each edge based on the corresponding edge
// of the basis rectangle.  The offsets for the outer rectangle define how much to grow the edges of
// the basis rectangle to obtain the corresponding edges of the outer rectangle.  Similarly, the
// offsets for the inner rectangle define how much to shrink the edges of the basis rectangle to
// obtain the corresponding edges of the inner rectangle, though it can't shrink to below zero width
// or zero height.  In the parlance of edge offsets, the outer rectangle offsets' default values are
// each 1, while the inner rectangle offsets' default values are each 0.
//
// The whole frame has a single texture applied to it, though each cell has particular texture
// coordinates assigned to it, so that each cell has a well-defined region of texture that is mapped
// to it.  The texture may stretch if the aspect ratio is not preserved, but this is by design.
// The default texture coordinates are given by defining the u and v coordinates for each of the
// rectangle edges (labeled):
//
//   OUTER  INNER           INNER  OUTER
//   LEFT   LEFT            RIGHT  RIGHT
//
// v  0.0  0.25              0.75  1.0 
//     +---+-------------------+---+ 1.0    OUTER TOP
// ^   | 0 |         1         | 2 |
// |   +---+-------------------+---+ 0.75   INNER TOP
// |   |   |                   |   |
// |   | 3 |         4         | 5 |
// |   |   |                   |   |
// |   +---+-------------------+---+ 0.25   INNER BOTTOM
// |   | 6 |         7         | 8 |
//     +---+-------------------+---+ 0.0    OUTER BOTTOM
//
//      --------------->  u
//
// This choice is proportional to the default sizes of the cells, making the texture appear unstretched.
// The texture defaults to uninitialized (there is no natural default).  If there is no texture set
// when this primitive is drawn, then nothing is drawn.
class TexturedFrame : public PrimitiveBase {
public:
  
  enum class Rectangle : size_t { INNER = 0, OUTER };
  enum class RectangleEdge : size_t { TOP = 0, RIGHT, BOTTOM, LEFT };
  
  TexturedFrame ();
  virtual ~TexturedFrame();
  
  const EigenTypes::Vector2 &BasisRectangleSize () const { return m_basis_rectangle_size; }
  double RectangleEdgeOffset (Rectangle rect, RectangleEdge edge) const { return m_rectangle_edge_offset[static_cast<size_t>(rect)][static_cast<size_t>(edge)]; }
  float RectangleEdgeTextureCoordinate (Rectangle rect, RectangleEdge edge) const { return m_rectangle_edge_texture_coordinate[static_cast<size_t>(rect)][static_cast<size_t>(edge)]; }
  const std::shared_ptr<Leap::GL::Texture2> &Texture () const { return m_texture; }
  
  // The rectangle will be centered at the origin, and will extend half of each size component in each direction.
  void SetBasisRectangleSize (const EigenTypes::Vector2 &size);
  // The inner (resp. outer) rectangles' edges will extend in (resp. out) from the basis rectangle
  // by the given offset.  Offsets are clamped to be nonnegative, so any negative value specified
  // here will be interpreted as zero.
  void SetRectangleEdgeOffset (Rectangle rect, RectangleEdge edge, double offset);
  // Sets the texture coordinate for the given rectangle edge (see diagram for labeling).
  void SetRectangleEdgeTextureCoordinate (Rectangle rect, RectangleEdge edge, float tex_coord);
  // Set the texture for this primitive.  The texture coordinates will be unchanged.
  void SetTexture (const std::shared_ptr<Leap::GL::Texture2> &texture) { m_texture = texture; }
  
protected:

  virtual void DrawContents (RenderState& renderState) const override;
  
  void ForceRecomputeMesh () { m_recompute_mesh = true; }

private:
  
  void RecomputeMeshIfNecessary () const;

  static const size_t RECTANGLE_COUNT = 2;
  static const size_t RECTANGLE_EDGE_COUNT = 4;
  static const size_t CELL_COUNT = 9;

  EigenTypes::Vector2 m_basis_rectangle_size;
  double m_rectangle_edge_offset[RECTANGLE_COUNT][RECTANGLE_EDGE_COUNT];
  float m_rectangle_edge_texture_coordinate[RECTANGLE_COUNT][RECTANGLE_EDGE_COUNT];
  std::shared_ptr<Leap::GL::Texture2> m_texture;
  mutable bool m_recompute_mesh;
  mutable PrimitiveGeometryMesh m_mesh;
};
