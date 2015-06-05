// Copyright (c) 2010 - 2014 Leap Motion. All rights reserved. Proprietary and confidential.
#include "stdafx.h"
#include "OSScreen.h"
#include "Primitives/Primitives.h"
#include "Leap/GL/Texture2.h"
#include <ShellScalingApi.h>

#include <fstream>
#include <cmath>

#define FREEIMAGE_LIB
#include "FreeImage.h"

HMODULE hshcore = LoadLibrary("shcore");
auto g_GetDpiForMonitor = (decltype(GetDpiForMonitor)*) GetProcAddress(hshcore, "GetDpiForMonitor");

struct StaticFreeImageLoader {
  StaticFreeImageLoader() {
    FreeImage_Initialise(FALSE);
  }
  ~StaticFreeImageLoader() {
    FreeImage_DeInitialise();
  }
};

void OSScreen::Update()
{
  MONITORINFOEX info;
  info.cbSize = sizeof(MONITORINFOEX);
  GetMonitorInfo(m_screenID, &info);
  info.rcMonitor.left;
  info.rcMonitor.top;
  info.rcMonitor.right;
  info.rcMonitor.bottom;
  m_bounds = OSRect(static_cast<float>(info.rcMonitor.left),
                    static_cast<float>(info.rcMonitor.top),
                    static_cast<float>(info.rcMonitor.right - info.rcMonitor.left),
                    static_cast<float>(info.rcMonitor.bottom - info.rcMonitor.top));
  m_isPrimary = ((info.dwFlags & MONITORINFOF_PRIMARY) == MONITORINFOF_PRIMARY);

  // Try to get the DPI for this monitor, if we can find the entrypoint:
  if(g_GetDpiForMonitor) {
    UINT dpiX;
    UINT dpiY;
    g_GetDpiForMonitor(m_screenID, MDT_RAW_DPI, &dpiX, &dpiY);
    m_pixelsPerInch = static_cast<float>(std::sqrt(dpiX * dpiX + dpiY * dpiY));
  }
  else
    // Default on Windows is 96/in
    m_pixelsPerInch = 96;
}

std::shared_ptr<ImagePrimitive> OSScreen::GetBackgroundTexture(std::shared_ptr<ImagePrimitive> img) const
{
  WCHAR pvParam[1024] = {0};
  if (!SystemParametersInfoW(SPI_GETDESKWALLPAPER, 1024, pvParam, 0)) {
    return img;
  }
  std::wstring pathString(pvParam);
  if (pathString.empty()) {
    return img;
  }

  static StaticFreeImageLoader s_fil;

  FIBITMAP* dib = nullptr;
  FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilenameU(pathString.c_str());
  if (fif != FIF_UNKNOWN) {
    dib = FreeImage_LoadU(fif, pathString.c_str(), 0);
  }
  if (!dib) {
    // Guess at a few image formats...
    FREE_IMAGE_FORMAT search[] = {FIF_BMP, FIF_JPEG, FIF_PNG, FIF_TIFF, FIF_GIF, FIF_UNKNOWN};
    int index = 0;
    while ((fif = search[index]) != FIF_UNKNOWN) {
      dib = FreeImage_LoadU(fif, pathString.c_str(), 0);
      if (dib) {
        break;
      }
      index++;
    }
    if (!dib) {
      return img;
    }
  }
  { // We will just convert to 32-bits, as that is likely the format anyway
    FIBITMAP* dib32 = FreeImage_ConvertTo32Bits(dib);
    FreeImage_Unload(dib);
    dib = dib32;
  }
  if (!dib) {
    return img;
  }
  FreeImage_FlipVertical(dib); // For some reason, the image is flipped vertically without this

  const size_t width = static_cast<size_t>(FreeImage_GetWidth(dib));
  const size_t height = static_cast<size_t>(FreeImage_GetHeight(dib));
  const size_t bytesPerRow = static_cast<size_t>(FreeImage_GetPitch(dib));
  const size_t stride = bytesPerRow/4;
  const size_t totalBytes = bytesPerRow*height;
  const uint8_t* dstBytes = FreeImage_GetBits(dib);

  std::shared_ptr<Leap::GL::Texture2> texture = img->Texture();
  if (texture) {
    const auto& params = texture->Params();
    if (params.Height() != height || params.Width() != width) {
      texture.reset();
    }
  }

  Leap::GL::Texture2PixelData pixelData{GL_BGRA, GL_UNSIGNED_BYTE, dstBytes, totalBytes};
  pixelData.SetPixelStoreiParameter(GL_UNPACK_ROW_LENGTH, stride);
  if (texture) {
    texture->TexSubImage(pixelData);
  } else {
    Leap::GL::Texture2Params params{static_cast<GLsizei>(width), static_cast<GLsizei>(height)};
    params.SetTarget(GL_TEXTURE_2D);
    params.SetInternalFormat(GL_RGBA8);
    params.SetTexParameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    params.SetTexParameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    params.SetTexParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    params.SetTexParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    texture = std::make_shared<Leap::GL::Texture2>(params, pixelData);
    img->SetTexture(texture);
    img->SetScaleBasedOnTextureSize();
  }

  FreeImage_Unload(dib);

  return img;
}
