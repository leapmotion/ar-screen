#pragma once

#include "ImagePassthrough.h"
#include "HandInfo.h"
#include "TextureFont/TextPrimitive.h"

class Scene {
public:
  void Init();
  void SetInputTransform(const EigenTypes::Matrix3x3& rotation, const EigenTypes::Vector3& translation);
  void Update(const std::deque<Leap::Frame>& frames);
  void Render(const Eigen::Matrix4f& proj, const Eigen::Matrix4f& view, int eyeIdx) const;
private:

  void updateTrackedHands(float deltaTime);
  void leapInteract(float deltaTime);
  void drawHands() const;

  EigenTypes::Matrix3x3 m_InputRotation;
  EigenTypes::Vector3 m_InputTranslation;
  mutable RenderState m_Renderer;
  std::shared_ptr<ImagePassthrough> m_ImagePassthrough;

  Leap::Frame m_CurFrame;
  typedef std::map<int, std::shared_ptr<HandInfo>> HandInfoMap;
  HandInfoMap m_TrackedHands;

  std::shared_ptr<TextureFont> m_Font;
  std::shared_ptr<TextPrimitive> m_Text;
  std::wstring m_ClockString;
};
