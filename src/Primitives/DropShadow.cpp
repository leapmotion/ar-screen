#include "stdafx.h"
#include "DropShadow.h"

#include "Leap/GL/Texture2.h"

std::shared_ptr<Leap::GL::Texture2> DropShadow::ms_shadow_texture;

DropShadow::DropShadow() {
  // If the shadow texture singleton isn't created yet, create it.
  if (!ms_shadow_texture) {
    static size_t const WIDTH = 256;
    static size_t const HEIGHT = 256;
    
    Leap::GL::Texture2Params params(WIDTH, HEIGHT, GL_LUMINANCE_ALPHA); // Luminance for greyscale, alpha for blending
    params.SetTexParameteri(GL_GENERATE_MIPMAP, GL_TRUE);
    params.SetTexParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    params.SetTexParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    params.SetTexParameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    params.SetTexParameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    struct LuminanceAlpha { float luminance, alpha; };
    std::vector<LuminanceAlpha> pixels(params.Width()*params.Height());
    static const EigenTypes::Vector2f TEXTURE_ORIGIN(0.5f*float(params.Width()-1), 0.5f*float(params.Height()-1));
    static const EigenTypes::Vector2f TEXTURE_SCALE(1.0f/TEXTURE_ORIGIN(0), 1.0f/TEXTURE_ORIGIN(1));
    for (GLsizei y = 0; y < params.Height(); ++y) {
      for (GLsizei x = 0; x < params.Width(); ++x) {
        EigenTypes::Vector2f tex_coord(EigenTypes::Vector2f(x,y) - TEXTURE_ORIGIN);
        tex_coord = tex_coord.cwiseProduct(TEXTURE_SCALE);
        // When norm is 0, the alpha should be 1.  When the norm is 1, the alpha should be 0.
        // When the norm is greater than 1, the alpha should be clamped to 0.
        // Otherwise, use the sqrt of radial distance to calculate the alpha
        // Using the sqrt is not physically correct, but produces a smoother appearance (e.g., closer to how shadows appear on Mac)
        pixels[y*params.Width()+x] = LuminanceAlpha{0.0f, std::max(0.0f, 1.0f-std::sqrt(tex_coord.norm()))};
      }
    }
    Leap::GL::Texture2PixelData pixel_data(GL_LUMINANCE_ALPHA, GL_FLOAT, pixels.data(), pixels.size()*sizeof(LuminanceAlpha));

    ms_shadow_texture = std::make_shared<Leap::GL::Texture2>(params, pixel_data);
  }
  
  SetTexture(ms_shadow_texture);
  
  SetRectangleEdgeTextureCoordinate(Rectangle::OUTER, RectangleEdge::LEFT,   0.0f);
  SetRectangleEdgeTextureCoordinate(Rectangle::INNER, RectangleEdge::LEFT,   0.5f); // The inner rectangle's texture coordinates should
  SetRectangleEdgeTextureCoordinate(Rectangle::INNER, RectangleEdge::RIGHT,  0.5f); // be collapsed completely to the center of the texture.
  SetRectangleEdgeTextureCoordinate(Rectangle::OUTER, RectangleEdge::RIGHT,  1.0f);
  
  SetRectangleEdgeTextureCoordinate(Rectangle::OUTER, RectangleEdge::BOTTOM, 0.0f);
  SetRectangleEdgeTextureCoordinate(Rectangle::INNER, RectangleEdge::BOTTOM, 0.5f); // The inner rectangle's texture coordinates should
  SetRectangleEdgeTextureCoordinate(Rectangle::INNER, RectangleEdge::TOP,    0.5f); // be collapsed completely to the center of the texture.
  SetRectangleEdgeTextureCoordinate(Rectangle::OUTER, RectangleEdge::TOP,    1.0f);
  
  SetShadowRadius(1); // Reasonable default
}

void DropShadow::SetShadowRadius(double shadow_radius) {
  shadow_radius = std::max(0.0, shadow_radius);
  if (m_shadow_radius != shadow_radius) {
    m_shadow_radius = shadow_radius;

    double offset = 0.5*m_shadow_radius;
    SetRectangleEdgeOffset(Rectangle::OUTER, RectangleEdge::RIGHT,  offset);
    SetRectangleEdgeOffset(Rectangle::OUTER, RectangleEdge::TOP,    offset);
    SetRectangleEdgeOffset(Rectangle::OUTER, RectangleEdge::LEFT,   offset);
    SetRectangleEdgeOffset(Rectangle::OUTER, RectangleEdge::BOTTOM, offset);
    
    SetRectangleEdgeOffset(Rectangle::INNER, RectangleEdge::RIGHT,  offset);
    SetRectangleEdgeOffset(Rectangle::INNER, RectangleEdge::TOP,    offset);
    SetRectangleEdgeOffset(Rectangle::INNER, RectangleEdge::LEFT,   offset);
    SetRectangleEdgeOffset(Rectangle::INNER, RectangleEdge::BOTTOM, offset);
    
    ForceRecomputeMesh();
  }
}

