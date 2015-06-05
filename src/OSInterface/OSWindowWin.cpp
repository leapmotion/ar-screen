#include "stdafx.h"
#include "OSWindowWin.h"
#include "OSWindowEvent.h"
#include "OSAppManager.h"
#include "OSApp.h"
#include "Primitives/Primitives.h"
#include "Leap/GL/Texture2.h"

#include <dwmapi.h>

OSWindowWin::OSWindowWin(HWND hwnd):
  hwnd{hwnd},
  m_phBitmapBits{nullptr}
{
  m_szBitmap.cx = 0;
  m_szBitmap.cy = 0;
  m_prevSize = m_szBitmap;

  AutowiredFast<OSAppManager> appManager;
  if (appManager) {
    m_app = appManager->GetApp(OSWindowWin::GetOwnerPid());
  }
}

OSWindowWin::~OSWindowWin(void)
{
}

void OSWindowWin::CheckSize(AutoFired<OSWindowEvent>& evt) {
  RECT rect;
  GetWindowRect(hwnd, &rect);

  SIZE sz;
  sz.cx = rect.right - rect.left;
  sz.cy = rect.bottom - rect.top;

  // Detect changes and then fire as needed:
  if(m_prevSize.cx != sz.cx || m_prevSize.cy != sz.cy)
    evt(&OSWindowEvent::OnResize)(*this);
  m_prevSize = sz;
}

bool OSWindowWin::IsValid(void) {
  return !!IsWindow(hwnd);
}

uint32_t OSWindowWin::GetOwnerPid(void) {
  DWORD pid;
  GetWindowThreadProcessId(hwnd, &pid);
  return pid;
}

std::shared_ptr<ImagePrimitive> OSWindowWin::GetWindowTexture(std::shared_ptr<ImagePrimitive> img)  {
  HDC hdc = GetWindowDC(hwnd);
  auto cleanhdc = MakeAtExit([&] {ReleaseDC(hwnd, hdc); });

  SIZE bmSz;
  {
    RECT rc;
    GetWindowRect(hwnd, &rc);
    bmSz.cx = rc.right - rc.left;
    bmSz.cy = rc.bottom - rc.top;
  }
  if(!bmSz.cx || !bmSz.cy)
    // Cannot create a window texture, window is gone
    return img;

  if(m_szBitmap.cx != bmSz.cx || m_szBitmap.cy != bmSz.cy) {
    BITMAPINFO bmi;
    auto& hdr = bmi.bmiHeader;
    hdr.biSize = sizeof(bmi.bmiHeader);
    hdr.biWidth = bmSz.cx;
    hdr.biHeight = -bmSz.cy;
    hdr.biPlanes = 1;
    hdr.biBitCount = 32;
    hdr.biCompression = BI_RGB;
    hdr.biSizeImage = 0;
    hdr.biXPelsPerMeter = 0;
    hdr.biYPelsPerMeter = 0;
    hdr.biClrUsed = 0;
    hdr.biClrImportant = 0;

    // Create a DC to be used for rendering
    m_hBmpDC.reset(CreateCompatibleDC(hdc));

    // Create the bitmap where the window will be rendered:
    m_hBmp.reset(CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &m_phBitmapBits, nullptr, 0));

    // Attach our bitmap where it needs to be:
    SelectObject(m_hBmpDC.get(), m_hBmp.get());

    // Update the size to reflect the new bitmap dimensions
    m_szBitmap = bmSz;
  }

  // Bit blit time to get at those delicious pixels
  BitBlt(m_hBmpDC.get(), 0, 0, m_szBitmap.cx, m_szBitmap.cy, hdc, 0, 0, SRCCOPY);

  // See if the texture underlying image was resized or not. If so, we need to create a new texture
  std::shared_ptr<Leap::GL::Texture2> texture = img->Texture();
  if (texture) {
    const auto& params = texture->Params();
    if (params.Height() != m_szBitmap.cy || params.Width() != m_szBitmap.cx) {
      texture.reset();
    }
  }
  Leap::GL::Texture2PixelData pixelData{ GL_BGRA, GL_UNSIGNED_BYTE, m_phBitmapBits, static_cast<size_t>(m_szBitmap.cx * m_szBitmap.cy * 4) };
  if (texture) {
    texture->TexSubImage(pixelData);
  } else {
    Leap::GL::Texture2Params params{ static_cast<GLsizei>(m_szBitmap.cx), static_cast<GLsizei>(m_szBitmap.cy) };
    params.SetTarget(GL_TEXTURE_2D);
    params.SetInternalFormat(GL_RGB8);
    params.SetTexParameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    params.SetTexParameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    params.SetTexParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    params.SetTexParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    texture = std::make_shared<Leap::GL::Texture2>(params, pixelData);
    img->SetTexture(texture);
    img->SetScaleBasedOnTextureSize();
  }
  texture->Bind();
  glGenerateMipmap(GL_TEXTURE_2D);
  texture->Unbind();

  return img;
}

bool OSWindowWin::GetFocus(void) {
  HWND foreground = GetForegroundWindow();
  return !!IsChild(foreground, hwnd);
}

void OSWindowWin::SetFocus(void) {
  ::SetForegroundWindow(hwnd);
}

std::wstring OSWindowWin::GetTitle(void) {
  std::wstring retVal(256, 0);
  int nch = GetWindowTextW(hwnd, &retVal[0], retVal.size());
  retVal.resize(nch);
  return retVal;
}

OSPoint OSWindowWin::GetPosition(void) {
  RECT rect;
  GetWindowRect(hwnd, &rect);

  OSPoint retVal;
  retVal.x = (float) rect.left;
  retVal.y = (float) rect.top;
  return retVal;
}

OSSize OSWindowWin::GetSize(void) {
  RECT rect;
  GetWindowRect(hwnd, &rect);

  OSSize retVal;
  retVal.width = (float) (rect.right - rect.left);
  retVal.height = (float) (rect.bottom - rect.top);
  return retVal;
}

void OSWindowWin::Cloak(void) {
  BOOL cloak = TRUE;
  ::DwmSetWindowAttribute(hwnd, DWMWA_CLOAK, &cloak, sizeof(cloak));
}

void OSWindowWin::Uncloak(void) {
  BOOL cloak = FALSE;
  ::DwmSetWindowAttribute(hwnd, DWMWA_CLOAK, &cloak, sizeof(cloak));
}

bool OSWindowWin::IsVisible(void) const {
  return !!::IsWindowVisible(hwnd);
}
