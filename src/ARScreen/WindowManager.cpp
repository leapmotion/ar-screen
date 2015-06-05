#include "stdafx.h"
#include "WindowManager.h"

FakeWindow::FakeWindow(OSWindow& window) : m_Window(window) {
  m_Texture = std::shared_ptr<ImagePrimitive>(new ImagePrimitive());
}

void FakeWindow::Update() {
  std::cout << "updated" << std::endl;
  m_Texture = m_Window.GetWindowTexture(m_Texture);
}

void WindowManager::OnCreate(OSWindow& window) {
  std::cout << "create" << std::endl;
  std::shared_ptr<OSWindow> windowPtr = window.shared_from_this();
  if (m_Windows.count(windowPtr)) {
    // Already have this entry, no reason to hit this again
    return;
  }

  m_Windows[windowPtr] = std::shared_ptr<FakeWindow>(new FakeWindow(window));
  m_Windows[windowPtr]->Update();
}

void WindowManager::OnDestroy(OSWindow& window) {
  auto q = m_Windows.find(window.shared_from_this());
  if (q == m_Windows.end()) {
    // Short-circuit, we can't find this window in our map
    return;
  }

  // Tell ExposeView that the window is gone, and that shutdown operations on this window should
  // take place.
  //m_exposeView->RemoveExposeWindow(q->second);
  m_Windows.erase(q);
}

void WindowManager::OnResize(OSWindow& window) {
  auto q = m_Windows.find(window.shared_from_this());
  if (q == m_Windows.end()) {
    // Short-circuit, we can't find this window in our map
    return;
  }

  q->second->Update();

  // Tell ExposeView that this window needs to be updated
  //m_exposeView->UpdateExposeWindow(q->second);
}
