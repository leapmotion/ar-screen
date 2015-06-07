#include "stdafx.h"
#include "WindowManager.h"
#include "utility/Utilities.h"
#include "Globals.h"
#include "OSInterface/OSVirtualScreen.h"

FakeWindow::FakeWindow(OSWindow& window) : m_Window(window), m_UpdateSize(false), m_UpdatePosition(false), m_ForceUpdate(false) {
  m_Texture = std::shared_ptr<ImagePrimitive>(new ImagePrimitive());
}

void FakeWindow::Update(const WindowTransform& transform, bool texture, double deltaTime) {
  if (texture) {
    // todo: move CPU part of texture retrieval to a separate thread, only do GPU upload here
    m_Texture = m_Window.GetWindowTexture(m_Texture);
  }
  const OSPoint pos = m_Window.GetPosition();
  const OSSize size = m_Window.GetSize();
  Eigen::Vector2d windowPos(pos.x, pos.y);
  Eigen::Vector2d windowSize(size.width, size.height);

  if (m_UpdateSize) {
    const Eigen::Vector2d sizeDiff = deltaTime * m_SizeVel;
    windowSize += sizeDiff;
    OSSize newSize;
    newSize.width = windowSize.x();
    newSize.height = windowSize.y();
    m_Window.SetSize(newSize);
    windowPos -= 0.5 * sizeDiff; // window origin is top left corner, so resizing should move origin
  }

  if (m_UpdatePosition) {
    windowPos += deltaTime * m_PositionVel;
    OSPoint newPos;
    newPos.x = windowPos.x();
    newPos.y = windowPos.y();
    m_Window.SetPosition(newPos);
  }

  m_OSPosition = windowPos + 0.5*windowSize;
  m_OSPosition.y() *= -1.0;

  m_Texture->Translation() = transform.Forward(m_OSPosition);
  m_Texture->Translation().z() += 10.0 * m_Window.GetZOrder();
  const Eigen::Matrix3d scaleMatrix = (transform.scale * Eigen::Vector3d(1, -1, 1)).asDiagonal();
  //m_Texture->LinearTransformation() = faceCameraMatrix(m_Texture->Translation(), Globals::userPos) * scaleMatrix;
  m_Texture->LinearTransformation() = scaleMatrix;

  m_ForceUpdate = false;
}

void FakeWindow::Interact(const WindowTransform& transform, const HandInfoMap& hands, float deltaTime) {
  Eigen::vector<Eigen::Vector2d> movementsPerHand;
  Eigen::vector<Eigen::Vector2d> positionsPerHand;

  m_UpdatePosition = false;
  m_UpdateSize = false;

  m_PositionVel.setZero();
  m_SizeVel.setZero();

  for (const auto& it : hands) {
    const HandInfo& hand = *it.second;
    HandInfo::IntersectionVector intersections = hand.IntersectRectangle(*m_Texture);
    Eigen::Vector2d sumPixelMovement = Eigen::Vector2d::Zero();
    Eigen::Vector2d sumPixelPosition = Eigen::Vector2d::Zero();
    int numSum = 0;
    for (const auto& intersection : intersections) {
      const Eigen::Vector3d delta = 0.25 * intersection.confidence * deltaTime * intersection.velocity;
      sumPixelMovement += delta.head<2>() / transform.scale;
      sumPixelPosition += transform.Backward(intersection.point);
      numSum++;
    }

    if (numSum > 0) {
      movementsPerHand.push_back(sumPixelMovement);
      positionsPerHand.push_back(sumPixelPosition / numSum);
    }

    if (sumPixelMovement.squaredNorm() >= 1.0) {
      Eigen::Vector2d curVel = sumPixelMovement / deltaTime;
      curVel.y() *= -1.0;
      m_PositionVel += curVel;
      m_UpdatePosition = true;
    }
  }

  if (movementsPerHand.size() > 0) {
    m_Window.SetFocus();
    m_PositionVel /= movementsPerHand.size();
  }

  // scaling
  if (movementsPerHand.size() >= 2) {
    Eigen::Vector2d center = Eigen::Vector2d::Zero();
    for (size_t i = 0; i < positionsPerHand.size(); i++) {
      center += positionsPerHand[i];
    }
    center /= positionsPerHand.size();

    Eigen::Vector2d sizeDiff = Eigen::Vector2d::Zero();

    for (size_t i = 0; i < movementsPerHand.size(); i++) {
      const Eigen::Vector2d diff = positionsPerHand[i] - center;
      const Eigen::Vector2d& movement = movementsPerHand[i];
      const bool diffSignX = diff.x() > 0;
      const bool diffSignY = diff.y() > 0;
      const bool movementSignX = movement.x() > 0;
      const bool movementSignY = movement.y() > 0;
      const double moveXMult = (diffSignX == movementSignX) ? 1.0 : -1.0;
      const double moveYMult = (diffSignY == movementSignY) ? 1.0 : -1.0;
      sizeDiff.x() += moveXMult * std::fabs(movement.x());
      sizeDiff.y() += moveYMult * std::fabs(movement.y());
    }

    if (sizeDiff.squaredNorm() >= 1.0) {
      m_UpdateSize = true;
      m_SizeVel = sizeDiff / deltaTime;
    }
  }
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

  m_Windows[windowPtr]->m_ForceUpdate = true;
  //m_Windows[windowPtr]->Update(*m_WindowTransform, true, 0.0);
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

  q->second->m_ForceUpdate = true;
  //q->second->Update(*m_WindowTransform, true, 0.0);
}

void WindowManager::Tick(std::chrono::duration<double> deltaT) {
  static double t = 0;
  t += deltaT.count();
  AutowiredFast<OSVirtualScreen> fullScreen;
  if (fullScreen) {
    auto screen = fullScreen->PrimaryScreen();
    const Eigen::Vector2d screenOrigin(screen.Bounds().origin.x, screen.Bounds().origin.y);
    const Eigen::Vector2d screenSize(screen.Bounds().size.width, screen.Bounds().size.height);
    m_WindowTransform->center = screenOrigin + 0.5*screenSize;

    if (Globals::haveScreen) {
      const double physicalDiag = std::sqrt(Globals::screenWidth * Globals::screenWidth + Globals::screenHeight * Globals::screenHeight);
      m_WindowTransform->scale = physicalDiag / screenSize.norm();
      m_WindowTransform->rotation = Globals::screenBasis;
      m_WindowTransform->offset = Globals::screenPos + Eigen::Vector3d(0, Globals::screenHeight, 0);
    } else {
      m_WindowTransform->scale = 500 / screenSize.norm();
      m_WindowTransform->offset << 0, 300, -100.0;
    }
  }

  int maxZ = -1;
  int maxZForce = -1;
  for (const auto& it : m_Windows) {
    const int z = it.second->m_Window.GetZOrder();
    if (it.second->m_ForceUpdate) {
      maxZForce = std::max(maxZForce, z);
    }
    maxZ = std::max(maxZ, z);
  }

  m_RoundRobinCounter = (m_RoundRobinCounter+1) % m_Windows.size();
  int curCounter = 0;
  for (const auto& it : m_Windows) {
    const int z = it.second->m_Window.GetZOrder();
    const bool updateTexture = (curCounter == m_RoundRobinCounter || z == maxZ || (z == maxZForce && it.second->m_ForceUpdate));
    it.second->Update(*m_WindowTransform, updateTexture, deltaT.count());
    curCounter++;
  }
}
