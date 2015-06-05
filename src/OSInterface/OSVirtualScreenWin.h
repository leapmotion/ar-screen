// Copyright (c) 2010 - 2014 Leap Motion. All rights reserved. Proprietary and confidential.
#pragma once

#include "OSVirtualScreen.h"

class OSVirtualScreenWin :
  public OSVirtualScreen
{
  public:
    OSVirtualScreenWin();
    virtual ~OSVirtualScreenWin();

    bool IsScreenSaverActive() const override;

  protected:
    virtual std::vector<OSScreen> GetScreens() const override;

  private:
    friend class OSVirtualScreenHelperClass;

    LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
    static BOOL CALLBACK EnumerateDisplays(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);

    HWND m_hWnd;
};
