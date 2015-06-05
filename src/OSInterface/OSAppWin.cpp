#include "stdafx.h"
#include "OSAppWin.h"
#include "Primitives/Primitives.h"
#include "Leap/GL/Texture2.h"

#include <ShlObj.h>
#include <psapi.h>

OSAppWin::OSAppWin(uint32_t pid) : OSApp(pid)
{
}

OSAppWin::~OSAppWin()
{
}

// Use the module file name as the application unique identifier
std::wstring OSApp::GetAppIdentifier(uint32_t pid) {
  HANDLE processHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
  if (processHandle == nullptr) {
    return std::wstring();
  }
  WCHAR filename[MAX_PATH];
  std::wstring path;

  DWORD maxPath = MAX_PATH;
  if (QueryFullProcessImageNameW(processHandle, 0, filename, &maxPath) != 0) {
    path = std::wstring(filename, maxPath);
  }
  CloseHandle(processHandle);
  return path;
}

OSApp* OSApp::New(uint32_t pid) {
  return new OSAppWin(pid);
}

std::string OSAppWin::GetAppName(void) const {
  return std::string(); // FIXME
}

std::shared_ptr<ImagePrimitive> OSAppWin::GetIconTexture(std::shared_ptr<ImagePrimitive> img) const {
  const size_t dimension = 256;
  const size_t totalBytes = dimension*dimension*4;

  std::wstring pathNative = m_id;

  std::unique_ptr<uint8_t[]> dstBytes(new uint8_t[totalBytes]);
  if (!dstBytes.get()) {
    return img;
  }
  ::memset(dstBytes.get(), 0, totalBytes);

  IShellItemImageFactory *pImageFactory = nullptr;
  if (SUCCEEDED(SHCreateItemFromParsingName(pathNative.c_str(), nullptr, IID_PPV_ARGS(&pImageFactory)))) {
    SIZE sz = { dimension, dimension };
    HBITMAP thumbnail;

    if (SUCCEEDED(pImageFactory->GetImage(sz, SIIGBF_BIGGERSIZEOK | SIIGBF_RESIZETOFIT, &thumbnail))) {
      GetBitmapBits(thumbnail, totalBytes, dstBytes.get());
      DeleteObject(thumbnail);
    }
    pImageFactory->Release();
  }

  // See if the texture underlying image was resized or not. If so, we need to create a new texture
  std::shared_ptr<Leap::GL::Texture2> texture = img->Texture();
  if (texture) {
    const auto& params = texture->Params();
    if (params.Height() != dimension || params.Width() != dimension) {
      texture.reset();
    }
  }
  Leap::GL::Texture2PixelData pixelData{ GL_BGRA, GL_UNSIGNED_BYTE, dstBytes.get(), totalBytes };
  if (texture) {
    texture->TexSubImage(pixelData);
  } else {
    Leap::GL::Texture2Params params{ static_cast<GLsizei>(dimension), static_cast<GLsizei>(dimension) };
    params.SetTarget(GL_TEXTURE_2D);
    params.SetInternalFormat(GL_RGBA8);
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
