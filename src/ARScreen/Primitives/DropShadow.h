#pragma once

#include "TexturedFrame.h"

// Design notes for DropShadow
// ---------------------------
// This is intended to be used primarily for providing drop shadows for perfectly rectangular objects,
// though the "blurred" configuration of it could be used for "blob" shadows, e.g. for irregular icons.
// Only a single procedurally generated texture will be created.
//
// The shadow will be rendered using TexturedFrame, with the underlying texture being an alpha blended
// disk -- opaque at the center, fading out in a radial gradient to transparent at the edge of the disk.
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
// These cells are rendered with various parts of the gradient disk texture.  Cell 4 is stretched out
// from the center of the disk (meaning that it uses only the center texel for its texture coordinates),
// so cell 4 is entirely opaque.  Cells 0, 2, 6, 8, are the upper left, upper right, lower left, and
// lower right corners of the disk, meaning that they are rendered as rounded corners.  Cells 1, 3, 5, 7
// are continuous extrusions of the sides of cells 0, 2, 6, 8, meaning that they are linear gradients.
// In the parlance of physics, cell 4 is the umbra of the shadow, and cells 0, 1, 2, 3, 5, 6, 7, 8 are
// the penumbra.
//
// The parameters controlling a drop shadow are as follows:
// - Basis rectangle size -- Determines which rectangle this is a shadow of; see TexturedFrame
// - Shadow radius -- Determines how thick the alpha-blended gradient region is.  A value of 0 will
//   produce a completely sharp shadow, while larger and larger radius values will produce blurrier
//   and blurrier shadows.  Effectively this is done by reducing the size of cell 4.
//
// Note: If the shadow radius is set such that cell 4 has nonnegative width and/or height, the rendered
// shadow should be a more or less physically accurate convolution-defined shadow, having an umbra
// and penumbra.  If the width and/or height of cell 4 would be negative, then this no longer holds.
// I may implement fully correct shadow rendering for this case in the future.  Essentially the shadow
// radius allows the distance from the rectangle to the surface its shadow is being casted on, and the
// shadow radius increasing to the point where the width and/or height of cell 4 goes negative means
// that the umbra disappears, and the maximum opacity of the shadow drops below 100%.
class DropShadow : public TexturedFrame {
public:
  
  // This sets the texture and texture coordinates of the shadow.
  DropShadow();
  virtual ~DropShadow() { }
  
  void SetShadowRadius (double shadow_radius);
  
private:
  
  static std::shared_ptr<Leap::GL::Texture2> ms_shadow_texture;
  
  double m_shadow_radius;
};
