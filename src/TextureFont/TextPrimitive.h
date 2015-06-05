#pragma once

#include "Primitives/Primitives.h"
#include "TextureFont.h"
#include <string>
#include <memory>

class TextPrimitive : public PrimitiveBase {
public:
  TextPrimitive();
  void SetText(const std::wstring& text, const std::shared_ptr<TextureFont>& font);
  const EigenTypes::Vector2& Size() const { return m_size; }
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
protected:
  virtual void DrawContents(RenderState& renderState) const override;
private:
  std::shared_ptr<Leap::GL::Shader> getFontShader() const;
  EigenTypes::Vector2 m_size;
  unsigned int m_atlasID;
  PrimitiveGeometryMesh m_mesh;
  std::shared_ptr<TextureFont> m_font;
};
