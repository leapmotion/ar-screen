// Copyright (c) 2010 - 2014 Leap Motion. All rights reserved. Proprietary and confidential.
#include "stdafx.h"
#include "OSWindowMac.h"
#include "OSAppManager.h"
#include "OSApp.h"
#include "Primitives/Primitives.h"
#include "Leap/GL/Texture2.h"

#include <AppKit/NSWindow.h>
#include <Foundation/NSArray.h>

#include <cassert>

OSWindowMac::OSWindowMac(NSDictionary* info) :
  m_windowID([[info objectForKey:(id)kCGWindowNumber] unsignedIntValue]),
  m_overlayWindowID(0),
  m_overlayOffset(NSZeroPoint),
  m_imageRef(nullptr),
  m_info([info retain]),
  m_mark(0)
{
  @autoreleasepool {
    const pid_t pid = static_cast<pid_t>([[m_info objectForKey:(id)kCGWindowOwnerPID] intValue]);
    AutowiredFast<OSAppManager> appManager;
    if (appManager) {
      m_app = appManager->GetApp(pid);
    }
  }
}

OSWindowMac::~OSWindowMac(void)
{
  [m_info release];
}

void OSWindowMac::UpdateInfo(NSDictionary* info) {
  assert([[info objectForKey:(id)kCGWindowNumber] unsignedIntValue] == m_windowID);
  [m_info release];
  m_info = [info retain];
}

bool OSWindowMac::SetOverlayWindow(CGWindowID overlayWindowID, const CGPoint& overlayOffset) {
  if (m_overlayWindowID == overlayWindowID) {
    if (m_overlayWindowID == 0 ||
       (m_overlayOffset.x == overlayOffset.x &&
        m_overlayOffset.y == overlayOffset.y)) {
      return false;
    }
  } else {
    m_overlayWindowID = overlayWindowID;
  }
  m_overlayOffset.x = overlayOffset.x;
  m_overlayOffset.y = overlayOffset.y;
  return true;
}

bool OSWindowMac::IsValid(void) {
  @autoreleasepool {
    CFArrayRef windowInfo = CGWindowListCopyWindowInfo(kCGWindowListOptionIncludingWindow, m_windowID);
    bool isValid = [(__bridge id)windowInfo count] > 0;
    CFRelease(windowInfo);
    return isValid;
  }
}

uint32_t OSWindowMac::GetOwnerPid(void) {
  return static_cast<uint32_t>([[m_info objectForKey:(id)kCGWindowOwnerPID] intValue]);
}

void OSWindowMac::TakeSnapshot(void) {
  CGImageRef imageRef = CGWindowListCreateImage(CGRectNull, kCGWindowListOptionIncludingWindow,
                                                m_windowID, kCGWindowImageBoundsIgnoreFraming |
                                                            kCGWindowImageNominalResolution);
  // If this window has an overlay window, apply the overlay to our image
  if (m_overlayWindowID) {
    CGImageRef overlayImageRef = CGWindowListCreateImage(CGRectNull, kCGWindowListOptionIncludingWindow,
                                                         m_overlayWindowID, kCGWindowImageBoundsIgnoreFraming |
                                                                            kCGWindowImageNominalResolution);
    if (overlayImageRef) {
      CGColorSpaceRef rgb = CGColorSpaceCreateDeviceRGB();
      // Determine actual window size
      CGRect bounds = NSZeroRect;
      @autoreleasepool {
        NSDictionary* windowBounds = [m_info objectForKey:(id)kCGWindowBounds];
        CGRectMakeWithDictionaryRepresentation(reinterpret_cast<CFDictionaryRef>(windowBounds), &bounds);
      }
      const CGRect originalRect{{0.0f, 0.0f},
                                {static_cast<CGFloat>(CGImageGetWidth(imageRef)),
                                 static_cast<CGFloat>(CGImageGetHeight(imageRef))}};
      // Adjust the overlay offset based on window decorations
      const CGFloat dx = (originalRect.size.width - bounds.size.width)*0.5f;
      const CGFloat dy = (originalRect.size.height - bounds.size.height)*0.71f; // Approx. 71% is bottom decoration
      // Adjust the offset due to the draw-origin being the bottom-left corner, but the overlay-origin is top-left
      const CGRect overlayRect{{m_overlayOffset.x + dx,
                                bounds.size.height - CGImageGetHeight(overlayImageRef) - m_overlayOffset.y + dy},
                               {static_cast<CGFloat>(CGImageGetWidth(overlayImageRef)),
                                static_cast<CGFloat>(CGImageGetHeight(overlayImageRef))}};
      CGContextRef contextRef =
          CGBitmapContextCreate(nullptr, CGImageGetWidth(imageRef), CGImageGetHeight(imageRef),
                                8, 0, rgb, kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little);
      CGContextDrawImage(contextRef, originalRect, imageRef);
      CGContextDrawImage(contextRef, overlayRect, overlayImageRef);
      CGImageRef compositeImageRef = CGBitmapContextCreateImage(contextRef);
      CGContextRelease(contextRef);
      CGColorSpaceRelease(rgb);
      CFRelease(overlayImageRef);

      if (compositeImageRef) {
        CFRelease(imageRef);
        imageRef = compositeImageRef;
      }
    }
  }
  imageRef = m_imageRef.exchange(imageRef);
  if (imageRef) {
    CFRelease(imageRef);
  }
}

std::shared_ptr<ImagePrimitive> OSWindowMac::GetWindowTexture(std::shared_ptr<ImagePrimitive> img) {
  TakeSnapshot();

  CGImageRef imageRef = CGImageRetain(m_imageRef);

  if (!imageRef) {
    return img;
  }

  CFDataRef dataRef = CGDataProviderCopyData(CGImageGetDataProvider(imageRef));
  if (dataRef) {
    const uint8_t* dstBytes = CFDataGetBytePtr(dataRef);
    const size_t bytesPerRow = CGImageGetBytesPerRow(imageRef);
    assert(bytesPerRow % 4 == 0);
    const size_t stride = bytesPerRow/4;
    const size_t width = CGImageGetWidth(imageRef);
    const size_t height = CGImageGetHeight(imageRef);
    const size_t totalBytes = bytesPerRow*height;

    // See if the texture underlying image was resized or not. If so, we need to create a new texture
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
      params.SetTexParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

      texture = std::make_shared<Leap::GL::Texture2>(params, pixelData);
      img->SetTexture(texture);
      img->SetScaleBasedOnTextureSize();
    }
    texture->Bind();
    glGenerateMipmap(GL_TEXTURE_2D);
    texture->Unbind();
    CFRelease(dataRef);
  }

  CFRelease(imageRef);

  return img;
}

bool OSWindowMac::GetFocus(void) {
  // FIXME
  return false;
}

extern "C" AXError _AXUIElementGetWindow(AXUIElementRef, CGWindowID*);

void OSWindowMac::SetFocus(void) {
  const pid_t pid = static_cast<pid_t>([[m_info objectForKey:(id)kCGWindowOwnerPID] intValue]);
  if (!pid) {
    return;
  }
  // First make the window the top-level window for the application
  AXUIElementRef appRef = AXUIElementCreateApplication(pid);
  if (appRef) {
    CFArrayRef winRefs;
    AXUIElementCopyAttributeValues(appRef, kAXWindowsAttribute, 0, 99999, &winRefs);
    if (winRefs) {
      for (auto i = 0; i < CFArrayGetCount(winRefs); i++) {
        AXUIElementRef winRef = (AXUIElementRef)CFArrayGetValueAtIndex(winRefs, i);
        CGWindowID winID = 0;
        _AXUIElementGetWindow(winRef, &winID);
        if (winID == m_windowID) {
          AXUIElementPerformAction(winRef, kAXRaiseAction);
          AXUIElementSetAttributeValue(winRef, kAXMainAttribute, kCFBooleanTrue);
          break;
        }
      }
      CFRelease(winRefs);
    } else {
      // Attempt to use AppleScript to raise the right window...
      std::ostringstream oss;
      oss << "tell application \"" << m_app->GetAppName() << "\"\n"
          << "\tset theWindow to window id " << m_windowID << "\n"
          << "\ttell theWindow\n"
          << "\t\tif index of theWindow is not 1 then\n"
          << "\t\t\tset index to 1\n"
          << "\t\t\tset visible to false\n"
          << "\t\tend if\n"
          << "\t\tset visible to true\n"
          << "\tend tell\n"
          << "end tell\n";
      @try {
        @autoreleasepool {
          NSString* script = [NSString stringWithUTF8String:oss.str().c_str()];
          NSAppleScript* as = [[NSAppleScript alloc] initWithSource:script];
          NSDictionary* errInfo = nullptr;
          [as executeAndReturnError:&errInfo];
        }
      }
      @catch (NSException*) {}
    }
    CFRelease(appRef);
  }
  @autoreleasepool {
    // Then bring the application to front
    [[NSRunningApplication runningApplicationWithProcessIdentifier:pid]
     activateWithOptions:NSApplicationActivateIgnoringOtherApps];
  }
}

std::wstring OSWindowMac::GetTitle(void) {
  std::wstring retVal;
  @autoreleasepool {
    NSString *title = [m_info objectForKey:(id)kCGWindowName];
    if (title) {
      static_assert(sizeof(wchar_t) == 4, "Expecting 32-bit wchar_t type");
      NSData* data = [title dataUsingEncoding:NSUTF32LittleEndianStringEncoding];
      retVal = std::wstring(reinterpret_cast<const wchar_t*>([data bytes]), [data length]/sizeof(wchar_t));
    }
  }
  return retVal;
}

OSPoint OSWindowMac::GetPosition(void) {
  OSPoint retVal;
  @autoreleasepool {
    NSDictionary* windowBounds = [m_info objectForKey:(id)kCGWindowBounds];
    CGRect bounds = NSZeroRect;
    CGRectMakeWithDictionaryRepresentation(reinterpret_cast<CFDictionaryRef>(windowBounds), &bounds);
    retVal.x = bounds.origin.x;
    retVal.y = bounds.origin.y;
  }
  return retVal;
}

OSSize OSWindowMac::GetSize(void) {
  OSSize retVal;
  retVal.width = 0;
  retVal.height = 0;
  @autoreleasepool {
    NSDictionary* windowBounds = [m_info objectForKey:(id)kCGWindowBounds];
    CGRect bounds = NSZeroRect;
    CGRectMakeWithDictionaryRepresentation(reinterpret_cast<CFDictionaryRef>(windowBounds), &bounds);
    retVal.width = bounds.size.width;
    retVal.height = bounds.size.height;
  }
  return retVal;
}

void OSWindowMac::Cloak(void) {
  // FIXME
}

void OSWindowMac::Uncloak(void) {
  // FIXME
}

bool OSWindowMac::IsVisible(void) const {
  // FIXME
  return true;
}

void OSWindowMac::SetPosition(const OSPoint& pos) {
  // FIXME
}

void OSWindowMac::SetSize(const OSSize& size) {
  // FIXME
}
