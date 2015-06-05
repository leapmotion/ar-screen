#include "stdafx.h"
#include "OSWindowMonitorMac.h"
#include "OSWindowMac.h"
#include "OSWindowEvent.h"

#include <AppKit/NSWindow.h>
#include <cfloat>

OSWindowMonitorMac::OSWindowMonitorMac(void)
{
  // Trigger initial enumeration
  Scan();
}

OSWindowMonitorMac::~OSWindowMonitorMac(void)
{
}

OSWindowMonitor* OSWindowMonitor::New(void) {
  return new OSWindowMonitorMac;
}

void OSWindowMonitorMac::Enumerate(const std::function<void(OSWindow&)>& callback) const {
  std::lock_guard<std::mutex> lk(m_lock);
  for(const auto& knownWindow : m_knownWindows)
    callback(*knownWindow.second);
}

void OSWindowMonitorMac::Scan() {
  static int s_pid = static_cast<int>(getpid());
  const uint32_t mark = ++m_mark;
  int zOrder = 0;
  CGWindowID overlayWindowID = 0;
  CGRect overlayBounds = NSZeroRect;
  std::unique_lock<std::mutex> lk(m_lock);
  size_t previousCount = m_knownWindows.size();
  lk.unlock();

  @autoreleasepool {
    CFArrayRef windowInfo = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly |
                                                       kCGWindowListExcludeDesktopElements, kCGNullWindowID);
    NSArray* windowArray = (__bridge id)windowInfo;
    // Loop through the windows
    for (NSDictionary* entry in windowArray) {
      // Do additional filtering of windows
      if ([[entry objectForKey:(id)kCGWindowLayer] intValue] != 0 ||
          [[entry objectForKey:(id)kCGWindowAlpha] floatValue] < FLT_EPSILON ||
          [[entry objectForKey:(id)kCGWindowOwnerName] length] == 0 ||
          [[entry objectForKey:(id)kCGWindowOwnerPID] intValue] == s_pid ||
          [[entry objectForKey:(id)kCGWindowSharingState] intValue] == kCGWindowSharingNone) {
        continue;
      }
      const CGWindowID windowID = [[entry objectForKey:(id)kCGWindowNumber] unsignedIntValue];
      if (windowID == 0) {
        continue;
      }

      // Check to see if this window may be an overlay window...
      if ([[entry objectForKey:(id)kCGWindowName] length] == 0) {
        // The assumption here is that only windows without names are overlay windows
        NSRunningApplication* runningApp =
          [NSRunningApplication
             runningApplicationWithProcessIdentifier:[[entry objectForKey:(id)kCGWindowOwnerPID] intValue]];

        // Only launched applications will have a launch date. Otherwise, we are assuming that it is an overlay.
        if ([runningApp launchDate] == nil) {
          NSDictionary* windowBounds = [entry objectForKey:(id)kCGWindowBounds];
          overlayBounds = NSZeroRect;
          CGRectMakeWithDictionaryRepresentation(reinterpret_cast<CFDictionaryRef>(windowBounds), &overlayBounds);
          overlayWindowID = (overlayBounds.size.width > 0 && overlayBounds.size.height > 0) ? windowID : 0;
          continue;
        }
      }
      CGPoint overlayOffset = NSZeroPoint;
      if (overlayWindowID) { // ...or if it is possibly the parent of an overlay window
        NSDictionary* windowBounds = [entry objectForKey:(id)kCGWindowBounds];
        CGRect bounds = NSZeroRect;
        CGRectMakeWithDictionaryRepresentation(reinterpret_cast<CFDictionaryRef>(windowBounds), &bounds);
        if (CGRectContainsRect(bounds, overlayBounds)) {
          overlayOffset.x = overlayBounds.origin.x - bounds.origin.x;
          overlayOffset.y = overlayBounds.origin.y - bounds.origin.y;
        } else {
          overlayWindowID = 0;
        }
      }
      lk.lock();
      // See if we already know about this window.
      auto found = m_knownWindows.find(windowID);
      if (found == m_knownWindows.end()) {
        auto window = std::make_shared<OSWindowMac>(entry);
        window->SetOverlayWindow(overlayWindowID, overlayOffset);
        window->SetMark(mark);
        window->SetZOrder(zOrder--);
        m_knownWindows[windowID] = window;
        lk.unlock();
        // Fire notifications off while outside of the lock:
        m_oswe(&OSWindowEvent::OnCreate)(*window);
      } else {
        auto& window = found->second;
        auto prvSize = window->GetSize();
        window->UpdateInfo(entry);
        const bool overlayChanged = window->SetOverlayWindow(overlayWindowID, overlayOffset);
        window->SetMark(mark);
        window->SetZOrder(zOrder--);
        --previousCount; // Saw this window last time, decrement the count
        auto newSize = window->GetSize();
        const bool wasResized = newSize.height != prvSize.height || newSize.width != prvSize.width;
        lk.unlock();
        if (wasResized || overlayChanged) {
          m_oswe(&OSWindowEvent::OnResize)(*window);
        }
      }
      overlayWindowID = 0;
    }
    CFRelease(windowInfo);
  }
  // If we can account for all of the previously seen windows, there is no need to check for destroyed windows.
  if (previousCount == 0) {
    return;
  }

  // Sweep through the known windows to find those that are not marked as expected
  std::vector<std::shared_ptr<OSWindowMac>> pending;
  lk.lock();
  for (auto knownWindow = m_knownWindows.begin(); knownWindow != m_knownWindows.end(); ) {
    auto& window = knownWindow->second;
    if (window->Mark() != mark) {
      pending.push_back(window);
      knownWindow = m_knownWindows.erase(knownWindow);
    } else {
      ++knownWindow;
    }
  }
  lk.unlock();
  // Fire notifications off while outside of the lock:
  for (auto& window : pending) {
    m_oswe(&OSWindowEvent::OnDestroy)(*window);
  }
}

std::shared_ptr<OSWindow> OSWindowMonitorMac::WindowFromPoint(OSPoint point) {
  std::shared_ptr<OSWindowMac> topmost;

  std::lock_guard<std::mutex> lk(m_lock);
  for(auto q : m_knownWindows) {
    // Point in rect?
    auto pos = q.second->GetPosition();

    // Compute the difference in position
    OSPoint delta {point.x - pos.x, point.y - pos.y};

    if(delta.x < 0.0f || delta.y < 0.0f)
      // Input point to the left or above top edge of this window
      continue;

    auto size = q.second->GetSize();
    if(size.width < point.x || size.height < point.y)
      // To the right or below the bottom edge of this window
      continue;

    if(!topmost)
      // No prior topmost, assign
      topmost = q.second;
    else if(topmost->GetZOrder() < q.second->GetZOrder())
      // Better fit found
      topmost = q.second;
  }

  return topmost;
}
