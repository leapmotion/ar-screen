// Copyright (c) 2010 - 2014 Leap Motion. All rights reserved. Proprietary and confidential.
#include "stdafx.h"
#include "OSAppMac.h"
#include "Primitives/Primitives.h"
#include "Leap/GL/Texture2.h"

#include <AppKit/NSGraphicsContext.h>
#include <AppKit/NSRunningApplication.h>
#include <Foundation/NSData.h>
#include <Foundation/NSString.h>
#include <Foundation/NSURL.h>

OSAppMac::OSAppMac(uint32_t pid) : OSApp(pid)
{
  @autoreleasepool {
    NSRunningApplication* runningApp = [NSRunningApplication runningApplicationWithProcessIdentifier:pid];
    const char* name = [[runningApp localizedName] UTF8String];
    if (name) {
      m_name = std::string(name);
    }
    m_icon = [[runningApp icon] retain];
  }
}

OSAppMac::~OSAppMac()
{
  [m_icon release];
}

std::wstring OSApp::GetAppIdentifier(uint32_t pid) {
  std::wstring appPath;

  @autoreleasepool {
    NSRunningApplication* runningApp = [NSRunningApplication runningApplicationWithProcessIdentifier:pid];
    NSString* url = [[runningApp executableURL] absoluteString];
    if (url) {
      static_assert(sizeof(wchar_t) == 4, "Expecting 32-bit wchar_t type");
      NSData* data = [url dataUsingEncoding:NSUTF32LittleEndianStringEncoding];
      appPath = std::wstring(reinterpret_cast<const wchar_t*>([data bytes]), [data length]/sizeof(wchar_t));
    }
  }
  return appPath;
}

OSApp* OSApp::New(uint32_t pid) {
  return new OSAppMac(pid);
}

std::string OSAppMac::GetAppName(void) const {
  return m_name;
}

std::shared_ptr<ImagePrimitive> OSAppMac::GetIconTexture(std::shared_ptr<ImagePrimitive> img) const {
  if (!m_icon) {
    return img;
  }
  const size_t width = 256;
  const size_t height = 256;
  const size_t bytesPerRow = width*4;
  const size_t totalBytes = bytesPerRow*height;

  std::unique_ptr<uint8_t[]> dstBytes(new uint8_t[totalBytes]);
  if (!dstBytes.get()) {
    return img;
  }
  ::memset(dstBytes.get(), 0, totalBytes);
  @autoreleasepool {
    CGColorSpaceRef rgb = CGColorSpaceCreateDeviceRGB();
    CGContextRef contextRef = CGBitmapContextCreate(dstBytes.get(), width, height, 8, bytesPerRow, rgb,
                                                    kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little);
    NSGraphicsContext* gc = [NSGraphicsContext graphicsContextWithGraphicsPort:contextRef flipped:NO];
    [NSGraphicsContext saveGraphicsState];
    [NSGraphicsContext setCurrentContext:gc];

    const NSSize imageSize = [m_icon size];
    const CGFloat scaleX = width/imageSize.width;
    const CGFloat scaleY = height/imageSize.height;
    const CGFloat scale = (scaleX >= scaleY) ? scaleX : scaleY;
    const NSSize scaledImageSize = NSMakeSize(imageSize.width * scale, imageSize.height * scale);
    const CGFloat xoffset = (imageSize.width*scaleX - scaledImageSize.width)/2.0;
    const CGFloat yoffset = (imageSize.height*scaleY - scaledImageSize.height)/2.0;
    const NSRect rect = NSMakeRect(xoffset, yoffset, scaledImageSize.width, scaledImageSize.height);

    [m_icon drawInRect:rect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];
    [NSGraphicsContext restoreGraphicsState];
    CGContextRelease(contextRef);
    CGColorSpaceRelease(rgb);
  }
  // See if the texture underlying image was resized or not. If so, we need to create a new texture
  std::shared_ptr<Leap::GL::Texture2> texture = img->Texture();
  if (texture) {
    const auto& params = texture->Params();
    if (params.Height() != height || params.Width() != width) {
      texture.reset();
    }
  }
  Leap::GL::Texture2PixelData pixelData{GL_BGRA, GL_UNSIGNED_BYTE, dstBytes.get(), totalBytes};
  if (texture) {
    texture->TexSubImage(pixelData);
  } else {
    Leap::GL::Texture2Params params{static_cast<GLsizei>(width), static_cast<GLsizei>(height)};
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
