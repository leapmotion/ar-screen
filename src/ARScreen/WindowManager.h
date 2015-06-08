#pragma once

#include "OSInterface/OSWindow.h"
#include "OSInterface/OSWindowEvent.h"
#include "utility/Updatable.h"
#include "utility/Animation.h"
#include "Primitives/Primitives.h"
#include "HandInfo.h"
#include "Globals.h"

struct WindowTransform {
  WindowTransform() : scale(1.0), center(Eigen::Vector2d::Zero()), offset(Eigen::Vector3d::Zero()), rotation(Eigen::Matrix3d::Identity()) {}
  Eigen::Vector3d Forward(const Eigen::Vector2d& pos) const {
    const Eigen::Vector2d adjusted = scale * (pos - center);
    Eigen::Vector3d result3D = offset + rotation * Eigen::Vector3d(adjusted.x(), adjusted.y(), 0.0);
    result3D.y() += Globals::globalHeightOffset;
    return result3D;
  }
  Eigen::Vector2d Backward(const Eigen::Vector3d& pos) const {
    Eigen::Vector3d adjusted = pos;
    adjusted.y() -= Globals::globalHeightOffset;
    adjusted = (rotation.inverse() * adjusted - offset) / scale;
    return Eigen::Vector2d(adjusted.x(), adjusted.y()) + center;
  }
  double scale;
  Eigen::Matrix3d rotation;
  Eigen::Vector2d center;
  Eigen::Vector3d offset;
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

class FakeWindow {
public:
  FakeWindow(OSWindow& window);
  void Update(const WindowTransform& transform, bool texture, double deltaTime);
  void Interact(const WindowTransform& transform, const HandInfoMap& hands, float deltaTime);
  std::shared_ptr<ImagePrimitive> m_Texture;
  OSWindow& m_Window;
  Eigen::Vector2d m_OSPosition;
  bool m_ForceUpdate;
  bool m_UpdateSize;
  bool m_UpdatePosition;
  Eigen::Vector2d m_SizeVel;
  Eigen::Vector2d m_PositionVel;
  Smoothed<double> m_ZOrder;
  Smoothed<Eigen::Vector3d, 10> m_PositionOffset;
  Smoothed<float, 10> m_Opacity;
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

class WindowManager :
  public OSWindowEvent,
  public Updatable {
public:
  WindowManager();

  void OnCreate(OSWindow& window) override;
  void OnDestroy(OSWindow& window) override;
  void OnResize(OSWindow& window) override;

  void Activate();
  void Deactivate();

  void GetZRange(int& min, int& max) const;

  // Updatable overrides:
  void Tick(std::chrono::duration<double> deltaT) override;

  std::unordered_map<std::shared_ptr<OSWindow>, std::shared_ptr<FakeWindow>> m_Windows;
  int m_RoundRobinCounter;
  std::shared_ptr<WindowTransform> m_WindowTransform;
  bool m_Active;
};
