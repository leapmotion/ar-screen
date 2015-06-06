#pragma once

#include "OSInterface/OSWindow.h"
#include "OSInterface/OSWindowEvent.h"
#include "utility/Updatable.h"
#include "Primitives/Primitives.h"

struct WindowTransform {
  WindowTransform() : scale(1.0), center(Eigen::Vector2d::Zero()), offset(Eigen::Vector3d::Zero()) {}
  double scale;
  Eigen::Vector2d center;
  Eigen::Vector3d offset;
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

class FakeWindow {
public:
  FakeWindow(OSWindow& window);
  void Update(const WindowTransform& transform, bool texture);
  std::shared_ptr<ImagePrimitive> m_Texture;
  OSWindow& m_Window;
  Eigen::Vector2d m_OSPosition;
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

  // Updatable overrides:
  void Tick(std::chrono::duration<double> deltaT) override;

  std::unordered_map<std::shared_ptr<OSWindow>, std::shared_ptr<FakeWindow>> m_Windows;
  int m_RoundRobinCounter;
  std::shared_ptr<WindowTransform> m_WindowTransform;
};
