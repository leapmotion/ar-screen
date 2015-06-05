// Copyright (c) 2010 - 2014 Leap Motion. All rights reserved. Proprietary and confidential.
#pragma once

#include "OSVirtualScreen.h"

class OSVirtualScreenMac :
  public OSVirtualScreen
{
  public:
    OSVirtualScreenMac();
    virtual ~OSVirtualScreenMac();

    bool IsScreenSaverActive() const override;

    // Used by the ScreenSaverMonitor Objective-C class (which cannot be friended)
    void BridgeUpdateScreenSaver() { UpdateScreenSaver(); }

  protected:
    virtual std::vector<OSScreen> GetScreens() const override;

  private:
    void* m_screenSaverMonitor;

    static void ConfigurationChangeCallback(CGDirectDisplayID display, CGDisplayChangeSummaryFlags flags, void *that);
};
