// Copyright (c) 2010 - 2014 Leap Motion. All rights reserved. Proprietary and confidential.
#pragma once

#include "OSScreenBase.h"

#if _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

class ImagePrimitive;

#if __APPLE__
using OSDisplayID = CGDirectDisplayID;
#elif _WIN32
using OSDisplayID = HMONITOR;
#else
using OSDisplayID = unsigned long;
#endif

class OSScreen : public OSScreenBase {
  public:
    virtual ~OSScreen() {}

    OSDisplayID ID() const { return m_screenID; }
    bool IsPrimary() const { return m_isPrimary; }

    float PixelsPerInch() const { return m_pixelsPerInch; }

    std::shared_ptr<ImagePrimitive> GetBackgroundTexture(std::shared_ptr<ImagePrimitive> img) const;

  private:
    OSScreen(const OSDisplayID& screenID) :
      m_screenID(screenID), m_isPrimary(false), m_pixelsPerInch(96.0f) { Update(); }

    void Update();

    OSDisplayID m_screenID;
    bool m_isPrimary;
    float m_pixelsPerInch;

    friend class OSVirtualScreenMac;
    friend class OSVirtualScreenWin;
};
