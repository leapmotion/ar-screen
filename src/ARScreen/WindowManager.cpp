#include "stdafx.h"
#include "WindowManager.h"
#include "utility/Utilities.h"
#include "Globals.h"
#include "OSInterface/OSVirtualScreen.h"

FakeWindow::FakeWindow(OSWindow& window) : m_Window(window) {
  m_Texture = std::shared_ptr<ImagePrimitive>(new ImagePrimitive());
}

void FakeWindow::Update(const WindowTransform& transform, bool texture) {
  if (texture) {
    // todo: move CPU part of texture retrieval to a separate thread, only do GPU upload here
    m_Texture = m_Window.GetWindowTexture(m_Texture);
  }
  const Eigen::Vector2d windowPos(m_Window.GetPosition().x, m_Window.GetPosition().y);
  const Eigen::Vector2d windowSize(m_Window.GetSize().width, m_Window.GetSize().height);
  m_OSPosition = windowPos + 0.5*windowSize;

  m_Texture->Translation() << transform.scale * (m_OSPosition - transform.center), 20*m_Window.GetZOrder();
  m_Texture->Translation() += transform.offset;
  m_Texture->Translation().y() *= -1.0;
  const Eigen::Matrix3d scaleMatrix = (transform.scale * Eigen::Vector3d(1, -1, 1)).asDiagonal();
  m_Texture->LinearTransformation() = faceCameraMatrix(m_Texture->Translation(), Globals::userPos) * scaleMatrix;
}

WindowManager::WindowManager() : m_RoundRobinCounter(0)
{
  m_WindowTransform = std::shared_ptr<WindowTransform>(new WindowTransform());
}

void WindowManager::OnCreate(OSWindow& window) {
  std::shared_ptr<OSWindow> windowPtr = window.shared_from_this();
  if (m_Windows.count(windowPtr)) {
    // Already have this entry, no reason to hit this again
    return;
  }

  m_Windows[windowPtr] = std::shared_ptr<FakeWindow>(new FakeWindow(window));
  m_Windows[windowPtr]->Update(*m_WindowTransform, true);
}

void WindowManager::OnDestroy(OSWindow& window) {
  auto q = m_Windows.find(window.shared_from_this());
  if (q == m_Windows.end()) {
    // Short-circuit, we can't find this window in our map
    return;
  }

  m_Windows.erase(q);
}

void WindowManager::OnResize(OSWindow& window) {
  auto q = m_Windows.find(window.shared_from_this());
  if (q == m_Windows.end()) {
    // Short-circuit, we can't find this window in our map
    return;
  }

  q->second->Update(*m_WindowTransform, true);
}

void WindowManager::Tick(std::chrono::duration<double> deltaT) {
  AutowiredFast<OSVirtualScreen> fullScreen;
  if (fullScreen) {
    auto screen = fullScreen->PrimaryScreen();
    const Eigen::Vector2d screenOrigin(screen.Bounds().origin.x, screen.Bounds().origin.y);
    const Eigen::Vector2d screenSize(screen.Bounds().size.width, screen.Bounds().size.height);
    const Eigen::Vector2d fullScreenSize(fullScreen->Bounds().size.width, fullScreen->Bounds().size.height);
    m_WindowTransform->center = screenOrigin + 0.5*screenSize;
    m_WindowTransform->scale = 1500 / fullScreenSize.norm();
    m_WindowTransform->offset << 0, -100, -200.0;
  }

  int maxZ = 0;
  for (const auto& it : m_Windows) {
    const int z = it.second->m_Window.GetZOrder();
    maxZ = std::max(maxZ, z);
  }

  m_RoundRobinCounter = (m_RoundRobinCounter+1) % m_Windows.size();
  int curCounter = 0;
  for (const auto& it : m_Windows) {
    const int z = it.second->m_Window.GetZOrder();
    const bool updateTexture = (curCounter == m_RoundRobinCounter || z == maxZ);
    it.second->Update(*m_WindowTransform, updateTexture);
    curCounter++;
  }
}
