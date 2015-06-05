// Copyright (c) 2010 - 2014 Leap Motion. All rights reserved. Proprietary and confidential.
#include "stdafx.h"
#include "OSVirtualScreen.h"

//
// OSVirtualScreen
//

OSVirtualScreen::OSVirtualScreen() : ContextMember("OSVirtualScreen")
{
}

OSVirtualScreen::~OSVirtualScreen()
{
}

OSScreen OSVirtualScreen::PrimaryScreen() const
{
  std::lock_guard<std::mutex> lock(m_mutex);

  uint32_t numDisplays = static_cast<uint32_t>(m_screens.size());
  if (numDisplays > 1) {
    for (uint32_t i = 0; i < numDisplays; i++) {
      if (m_screens[i].IsPrimary()) {
        return m_screens[i];
      }
    }
  }
  if (m_screens.empty()) {
    throw std::runtime_error("Unable to detect screens");
  }
  return m_screens[0];
}

OSScreen OSVirtualScreen::ClosestScreen(const OSPoint& position) const
{
  std::lock_guard<std::mutex> lock(m_mutex);

  uint32_t numDisplays = static_cast<uint32_t>(m_screens.size());

  if (numDisplays > 1) {
    for (uint32_t i = 0; i < numDisplays; i++) {
      if (OSRectContainsPoint(m_screens[i].Bounds(), position)) {
        return m_screens[i];
      }
    }
    int bestIndex = 0;
    float bestSquaredDistance = 0;
    for (uint32_t i = 0; i < numDisplays; i++) {
      OSPoint clipped = m_screens[i].ClipPosition(position);
      const float dx = (clipped.x - position.x);
      const float dy = (clipped.y - position.y);
      float squaredDistance = dx*dx + dy*dy;
      if (i == 0 || squaredDistance < bestSquaredDistance) {
        bestIndex = i;
        bestSquaredDistance = squaredDistance;
      }
    }
    return m_screens[bestIndex];
  }
  if (m_screens.empty()) {
    throw std::runtime_error("Unable to detect screens");
  }
  return m_screens[0];
}

void OSVirtualScreen::UpdateScreenSize()
{
  auto screens = GetScreens();
  std::unique_lock<std::mutex> lock(m_mutex);
  m_screens = screens;
  m_bounds = ComputeBounds(m_screens);
  lock.unlock();
  AutoFired<OSVirtualScreenListener> vsl;
  vsl(&OSVirtualScreenListener::OnScreenSizeChange)();
}

void OSVirtualScreen::UpdateScreenSaver()
{
  AutoFired<OSVirtualScreenListener> vsl;
  vsl(&OSVirtualScreenListener::OnScreenSaverChange)();
}

OSRect OSVirtualScreen::ComputeBounds(const std::vector<OSScreen>& screens)
{
  size_t numScreens = screens.size();

  if (numScreens == 1) {
    return screens[0].Bounds();
  } else if (numScreens > 1) {
    OSRect bounds = screens[0].Bounds();

    for (size_t i = 1; i < numScreens; i++) {
      bounds = OSRectUnion(bounds, screens[i].Bounds());
    }
    return bounds;
  } else {
    return OSRectZero;
  }
}
