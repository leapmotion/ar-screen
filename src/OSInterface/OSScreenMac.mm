// Copyright (c) 2010 - 2014 Leap Motion. All rights reserved. Proprietary and confidential.
#include "stdafx.h"
#include "OSScreen.h"
#include "Primitives/Primitives.h"
#include "Leap/GL/Texture2.h"

#include <AppKit/NSScreen.h>
#include <AppKit/NSGraphicsContext.h>
#include <AppKit/NSImage.h>
#include <AppKit/NSWorkspace.h>
#include <Foundation/NSKeyValueCoding.h>
#include <Foundation/NSValue.h>
#include <OpenGL/OpenGL.h>

#include <cmath>

void OSScreen::Update()
{
  m_isPrimary = CGDisplayIsMain(m_screenID);
  m_bounds = CGDisplayBounds(m_screenID);
  const CGSize screenSizeInMM = CGDisplayScreenSize(m_screenID);
  const float widthInches = screenSizeInMM.width/25.4f;
  const float heightInches = screenSizeInMM.height/25.4f;
  const float diagonalInches = std::sqrt(widthInches*widthInches + heightInches*heightInches);
  if (std::abs(diagonalInches) < FLT_EPSILON) {
    m_pixelsPerInch = 96.0f;
    return;
  }
  const float diagonalPixels = std::sqrt(m_bounds.size.width*m_bounds.size.width +
                                         m_bounds.size.height*m_bounds.size.height);
  const float pixelsPerInch = diagonalPixels/diagonalInches;
  if (pixelsPerInch < FLT_EPSILON) {
    m_pixelsPerInch = 96.0f;
    return;
  }
  m_pixelsPerInch = pixelsPerInch;
}

std::shared_ptr<ImagePrimitive> OSScreen::GetBackgroundTexture(std::shared_ptr<ImagePrimitive> img) const
{
  // There must be a current OpenGL context in order to obtain a texture
  if (CGLGetCurrentContext() == nullptr) {
    return img;
  }
  @autoreleasepool {
    NSScreen* screen = nil;
    for (NSScreen* item in [NSScreen screens]) {
      NSNumber* number = [[item deviceDescription] valueForKey:@"NSScreenNumber"];
      if (m_screenID == [number unsignedIntValue]) {
        screen = item;
        break;
      }
    }
    if (!screen) {
      return img;
    }
    const size_t width = static_cast<size_t>(Width());
    const size_t height = static_cast<size_t>(Height());
    const size_t bytesPerRow = width*4;
    const size_t totalBytes = bytesPerRow*height;

    std::unique_ptr<uint8_t[]> dstBytes(new uint8_t[totalBytes]);
    if (!dstBytes.get()) {
      return img;
    }
    ::memset(dstBytes.get(), 0, totalBytes);

    NSImage* nsImage =
        [[NSImage alloc] initWithContentsOfURL:[[NSWorkspace sharedWorkspace] desktopImageURLForScreen:screen]];
    if (!nsImage) {
      return img;
    }
    CGColorSpaceRef rgb = CGColorSpaceCreateDeviceRGB();
    CGContextRef cgContextRef =
      CGBitmapContextCreate(dstBytes.get(), width, height, 8, bytesPerRow, rgb, kCGImageAlphaPremultipliedLast);
    NSGraphicsContext* gc = [NSGraphicsContext graphicsContextWithGraphicsPort:cgContextRef flipped:NO];
    [NSGraphicsContext saveGraphicsState];
    [NSGraphicsContext setCurrentContext:gc];

    const NSSize imageSize = [nsImage size];
    const CGFloat scaleX = width/imageSize.width;
    const CGFloat scaleY = height/imageSize.height;
    const CGFloat scale = (scaleX >= scaleY) ? scaleX : scaleY;
    const NSSize scaledImageSize = NSMakeSize(imageSize.width * scale, imageSize.height * scale);
    const CGFloat xoffset = (imageSize.width*scaleX - scaledImageSize.width)/2.0;
    const CGFloat yoffset = (imageSize.height*scaleY - scaledImageSize.height)/2.0;
    const NSRect rect = NSMakeRect(xoffset, yoffset, scaledImageSize.width, scaledImageSize.height);

    [nsImage drawInRect:rect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];
    [NSGraphicsContext restoreGraphicsState];
    CGContextRelease(cgContextRef);
    CGColorSpaceRelease(rgb);
    [nsImage release];

    std::shared_ptr<Leap::GL::Texture2> texture = img->Texture();
    if (texture) {
      const auto& params = texture->Params();
      if (params.Height() != height || params.Width() != width) {
        texture.reset();
      }
    }
    Leap::GL::Texture2PixelData pixelData{GL_RGBA, GL_UNSIGNED_BYTE, dstBytes.get(), totalBytes};
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

    return img;
  }
}
