#pragma once

#include "ImagePassthrough.h"
#include "HandInfo.h"
#include "TextureFont/TextPrimitive.h"
#include "GLTexture2Image/GLTexture2Image.h"
#include "utility/Animation.h"

class Scene {
public:
  Scene();
  void Init();
  void SetInputTransform(const EigenTypes::Matrix3x3& rotation, const EigenTypes::Vector3& translation);
  void Update(const std::deque<Leap::Frame>& frames);
  void Render(const Eigen::Matrix4f& proj, const Eigen::Matrix4f& view, int eyeIdx) const;
private:

  void updateTrackedHands(float deltaTime);
  void leapInteract(float deltaTime);
  void drawHands() const;
  void drawFakeMouse() const;
  void drawWindows() const;
  void createUI();
  void drawUI() const;
  void createNewsFeed();
  void drawNewsFeed() const;
  static Leap::GL::Rgba<float> makeIntersectionDiskColor(double confidence);

  EigenTypes::Matrix3x3 m_InputRotation;
  EigenTypes::Vector3 m_InputTranslation;
  mutable RenderState m_Renderer;
  std::shared_ptr<ImagePassthrough> m_ImagePassthrough;

  Leap::Frame m_PrevFrame;
  Leap::Frame m_CurFrame;
  HandInfoMap m_TrackedHands;

  std::shared_ptr<TextureFont> m_Font;
  std::shared_ptr<TextPrimitive> m_ClockText;
  std::wstring m_ClockString;
  std::shared_ptr<Sphere> m_MouseSphere;

  std::shared_ptr<Disk> m_AnimationDisk;
  std::shared_ptr<Disk> m_IconDisk;
  std::shared_ptr<ImagePrimitive> m_IconPrimitive;
  std::shared_ptr<ImagePrimitive> m_ExpandedPrimitive;
  GLTexture2ImageRef m_CalendarExpanded;
  GLTexture2ImageRef m_CalendarIcon;
  GLTexture2ImageRef m_EmailIcon;
  GLTexture2ImageRef m_PhoneIcon;
  GLTexture2ImageRef m_RecordIcon;
  GLTexture2ImageRef m_TextsIcon;

  std::shared_ptr<Disk> m_IntersectionDisk;
  Smoothed<Eigen::Vector3d> m_ScreenPositionSmoother;
  Smoothed<Eigen::Matrix3d> m_ScreenRotationSmoother;

  mutable bool m_ButtonCooldown;
  mutable Smoothed<float> m_CalendarOpacity;
  mutable Smoothed<float> m_ButtonAnimation;

  std::chrono::steady_clock::time_point m_GestureStart;
  bool m_ActivationGesture;
  bool m_DeactivationGesture;

  std::vector<std::shared_ptr<TextPrimitive>> m_NewsFeedItems;
  mutable double m_FeedScroll;
  std::shared_ptr<RectanglePrim> m_NewsFeedRect;
};
