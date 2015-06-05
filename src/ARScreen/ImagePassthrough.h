#pragma once

#include "Primitives/Primitives.h"
#include "Leap/GL/Texture2.h"
#include "LeapListener/LeapListener.h"

class ImagePassthrough {
public:
  ImagePassthrough();
  void Init();
  void SetActiveTexture(int activeTexture) { m_ActiveTexture = activeTexture; }
  void Update(const Leap::ImageList& images);
  void SetUseStencil(bool use) { m_UseStencil = use; }
  void DrawStencilObject(PrimitiveBase* obj, RenderState& renderState, float viewWidth, float viewX, float viewHeight, float l00, float l11, float l03, float opacity) const;
  void Draw(RenderState& renderState) const;

private:

  std::shared_ptr<Leap::GL::Shader> m_Shader;
  std::shared_ptr<Leap::GL::Shader> m_HandsShader;
  std::shared_ptr<RectanglePrim> m_Quad;

  void updateImage(int idx, const Leap::Image& image);
  void updateDistortion(int idx, const Leap::Image& image);

  static const int NUM_CAMERAS = 2;

  std::shared_ptr<Leap::GL::Texture2> m_Textures[NUM_CAMERAS];
  std::shared_ptr<Leap::GL::Texture2> m_Distortion[NUM_CAMERAS];
  int m_ActiveTexture;

  bool m_UseStencil;
  bool m_Color;

  size_t m_ImageBytes[NUM_CAMERAS];
  size_t m_DistortionBytes[NUM_CAMERAS];
};
