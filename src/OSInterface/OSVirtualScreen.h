// Copyright (c) 2010 - 2014 Leap Motion. All rights reserved. Proprietary and confidential.
#pragma once

#include "OSScreen.h"
#include "OSCursor.h"
#include <vector>
#include <mutex>

class OSCursor;

class OSVirtualScreenListener {
  public:
    OSVirtualScreenListener(void) {}
    virtual ~OSVirtualScreenListener(void) {}

    virtual void OnScreenSizeChange(void) {}
    virtual void OnScreenSaverChange(void) {}
};

class OSVirtualScreen :
  public ContextMember,
  public OSScreenBase
{
  public:
    OSVirtualScreen();
    virtual ~OSVirtualScreen();

    static OSVirtualScreen* New(void);

    std::vector<OSScreen> Screens() const {
      std::lock_guard<std::mutex> lock(m_mutex);
      return m_screens;
    }

    OSScreen PrimaryScreen() const;
    OSScreen ClosestScreen(const OSPoint& position) const;

    virtual bool IsScreenSaverActive() const = 0;

  protected:
    virtual std::vector<OSScreen> GetScreens() const = 0;

    void UpdateScreenSize();
    void UpdateScreenSaver();

  private:
    OSRect ComputeBounds(const std::vector<OSScreen>& screens);
    OSPoint ClipPosition(const OSPoint& position, uint32_t* index) const;

    mutable std::mutex m_mutex;
    std::vector<OSScreen> m_screens;

    AutoRequired<OSCursor> m_cursor;
};
