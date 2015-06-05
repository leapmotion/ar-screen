// Copyright (c) 2010 - 2014 Leap Motion. All rights reserved. Proprietary and confidential.
#include "stdafx.h"
#include "OSVirtualScreenMac.h"
#include <Foundation/NSString.h>
#include <Foundation/NSDistributedNotificationCenter.h>
#include <Foundation/NSConnection.h>

@interface ScreenSaverMonitor : NSObject {
@private
  OSVirtualScreenMac* _object;
  BOOL _isScreenSaverActive;
}

- (id) initWithObject:(OSVirtualScreenMac*) object;
- (void) dealloc;
- (void) onNotification:(NSNotification*) notification;
- (BOOL) isScreenSaverActive;

@end

@implementation ScreenSaverMonitor
- (id) initWithObject:(OSVirtualScreenMac*) object
{
  self = [super init];
  if (self != nil) {
    _object = object;
    _isScreenSaverActive = NO;
    if (_object != nil) {
      NSDistributedNotificationCenter* center = [NSDistributedNotificationCenter defaultCenter];
      [center addObserver:self selector:@selector(onNotification:) name:@"com.apple.screensaver.didstart" object:nil];
      [center addObserver:self selector:@selector(onNotification:) name:@"com.apple.screensaver.didstop"  object:nil];
    }
  }
  return self;
}

- (void) dealloc
{
  if (_object != nil) {
    NSDistributedNotificationCenter* center = [NSDistributedNotificationCenter defaultCenter];
    [center removeObserver:self name:@"com.apple.screensaver.didstop"  object:nil];
    [center removeObserver:self name:@"com.apple.screensaver.didstart" object:nil];
  }
  [super dealloc];
}

- (void) onNotification:(NSNotification*) notification
{
  NSString* name = [notification name];
  _isScreenSaverActive = [name isEqual:@"com.apple.screensaver.didstart"];
  _object->BridgeUpdateScreenSaver();
}

- (BOOL) isScreenSaverActive
{
  return _isScreenSaverActive;
}
@end

OSVirtualScreen* OSVirtualScreen::New(void)
{
  return new OSVirtualScreenMac;
}

//
// OSVirtualScreenMac
//

OSVirtualScreenMac::OSVirtualScreenMac()
{
  m_screenSaverMonitor = reinterpret_cast<void*>([[ScreenSaverMonitor alloc] initWithObject:this]);
  CGDisplayRegisterReconfigurationCallback(ConfigurationChangeCallback, this);
  UpdateScreenSize();
}

OSVirtualScreenMac::~OSVirtualScreenMac()
{
  CGDisplayRemoveReconfigurationCallback(ConfigurationChangeCallback, this);
  [reinterpret_cast<ScreenSaverMonitor*>(m_screenSaverMonitor) release];
}

// Called when the display configuration changes
void OSVirtualScreenMac::ConfigurationChangeCallback(CGDirectDisplayID display,
                                                     CGDisplayChangeSummaryFlags flags,
                                                     void *that)
{
  if ((flags & kCGDisplayBeginConfigurationFlag) == 0 && that) {
    static_cast<OSVirtualScreenMac*>(that)->UpdateScreenSize();
  }
}

bool OSVirtualScreenMac::IsScreenSaverActive() const
{
  return [reinterpret_cast<ScreenSaverMonitor*>(m_screenSaverMonitor) isScreenSaverActive];
}

std::vector<OSScreen> OSVirtualScreenMac::GetScreens() const
{
  std::vector<OSScreen> screens;
  uint32_t numDisplays = 0;

  if (CGGetActiveDisplayList(0, 0, &numDisplays) == kCGErrorSuccess && numDisplays > 0) {
    CGDirectDisplayID *screenIDs = new CGDirectDisplayID[numDisplays];

    if (screenIDs) {
      if (CGGetActiveDisplayList(numDisplays, screenIDs, &numDisplays) == kCGErrorSuccess) {
        for (int i = 0; i < numDisplays; i++) {
          screens.push_back(OSScreen(screenIDs[i]));
        }
      }
      delete [] screenIDs;
    }
  }
  if (screens.empty()) {
    screens.push_back(OSScreen(CGMainDisplayID()));
  }
  return screens;
}
