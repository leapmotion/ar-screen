#pragma once

#include "OSInterface/OSWindow.h"
#include "OSInterface/OSWindowEvent.h"
#include "Primitives/Primitives.h"

class FakeWindow {
public:
  FakeWindow(OSWindow& window);
  void Update();
  std::shared_ptr<ImagePrimitive> m_Texture;
  OSWindow& m_Window;
};

class WindowManager : public OSWindowEvent {
public:

  void OnCreate(OSWindow& window) override;
  void OnDestroy(OSWindow& window) override;
  void OnResize(OSWindow& window) override;

  std::unordered_map<std::shared_ptr<OSWindow>, std::shared_ptr<FakeWindow>> m_Windows;
};
