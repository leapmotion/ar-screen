// Copyright (c) 2010 - 2014 Leap Motion. All rights reserved. Proprietary and confidential.
#include "stdafx.h"
#include "OSVirtualScreenWin.h"

OSVirtualScreen* OSVirtualScreen::New(void)
{
  return new OSVirtualScreenWin;
}

//
// OSVirtualScreenHelperClass
//

class OSVirtualScreenHelperClass {
  public:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static ATOM GetAtom() { static OSVirtualScreenHelperClass s_instance; return s_instance.m_atom; }

  private:
    OSVirtualScreenHelperClass();
    ~OSVirtualScreenHelperClass();

    WNDCLASSW m_wndClass;
    ATOM m_atom;
};

OSVirtualScreenHelperClass::OSVirtualScreenHelperClass()
{
  m_wndClass.style = CS_NOCLOSE;
  m_wndClass.lpfnWndProc = WndProc;
  m_wndClass.cbWndExtra = sizeof(void*);
  m_wndClass.cbClsExtra = 0;
  m_wndClass.hInstance = nullptr;
  m_wndClass.hIcon = nullptr;
  m_wndClass.hCursor = nullptr;
  m_wndClass.hbrBackground = nullptr;
  m_wndClass.lpszMenuName = nullptr;
  m_wndClass.lpszClassName = L"Leap::Desktop";
  m_atom = RegisterClassW(&m_wndClass);
}

OSVirtualScreenHelperClass::~OSVirtualScreenHelperClass()
{
  UnregisterClassW(L"Leap::Desktop", nullptr);
}

LRESULT CALLBACK OSVirtualScreenHelperClass::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  LONG_PTR val = GetWindowLongPtr(hwnd, GWLP_USERDATA);
  if (val) {
    return reinterpret_cast<OSVirtualScreenWin*>(val)->WndProc(uMsg, wParam, lParam);
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

//
// OSVirtualScreenWin
//

OSVirtualScreenWin::OSVirtualScreenWin()
{
  m_hWnd = ::CreateWindowExW(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT,
                             MAKEINTRESOURCEW(OSVirtualScreenHelperClass::GetAtom()),
                             L"", WS_POPUP | WS_VISIBLE, 0, 0, 0, 0, nullptr, nullptr, nullptr, this);
  ::SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);
  ::ShowWindow(m_hWnd, SW_HIDE);
  UpdateScreenSize();
}

OSVirtualScreenWin::~OSVirtualScreenWin()
{
  if (m_hWnd) {
    ::DestroyWindow(m_hWnd);
  }
}

bool OSVirtualScreenWin::IsScreenSaverActive() const
{
  BOOL isActive = FALSE;
  SystemParametersInfo(SPI_GETSCREENSAVERRUNNING, 0, &isActive, 0);
  return (isActive != FALSE);
}

std::vector<OSScreen> OSVirtualScreenWin::GetScreens() const
{
  std::vector<OSScreen> screens;
  EnumDisplayMonitors(0, 0, EnumerateDisplays, reinterpret_cast<LPARAM>(&screens));
  return screens;
}

LRESULT OSVirtualScreenWin::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if (uMsg == WM_DISPLAYCHANGE) {
    UpdateScreenSize();
  }
  return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
}

BOOL CALLBACK OSVirtualScreenWin::EnumerateDisplays(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
  std::vector<OSScreen>& screens = *reinterpret_cast<std::vector<OSScreen>*>(dwData);
  screens.push_back(OSScreen(hMonitor));
  return true;
}
