#include "stdafx.h"
#include "MediaInterfaceMac.h"

#include <ApplicationServices/ApplicationServices.h>
#include <AppKit/NSEvent.h>
#include <Foundation/Foundation.h>
#include <IOKit/hidsystem/ev_keymap.h>

MediaInterface* MediaInterface::New(void) {
  return new MediaInterfaceMac;
}

void MediaInterfaceMac::PlayPause(void) {
  SendSpecialKeyEventPair(NX_KEYTYPE_PLAY);
}

void MediaInterfaceMac::Stop(void) {
  // No concept of Stop on Mac
}

void MediaInterfaceMac::Next(void) {
  SendSpecialKeyEventPair(NX_KEYTYPE_FAST);
}

void MediaInterfaceMac::Prev(void) {
  SendSpecialKeyEventPair(NX_KEYTYPE_REWIND);
}

void MediaInterfaceMac::VolumeUp(void) {
  // Include shift-key modifier so the control doesn't produce "pop" noise
  SendSpecialKeyEventPair(NX_KEYTYPE_SOUND_UP, NSShiftKeyMask);
}

void MediaInterfaceMac::VolumeDown(void) {
  // Include shift-key modifier so the control doesn't produce "pop" noise
  SendSpecialKeyEventPair(NX_KEYTYPE_SOUND_DOWN, NSShiftKeyMask);
}

void MediaInterfaceMac::Mute(void) {
  // Include shift-key modifier so the control doesn't produce "pop" noise
  SendSpecialKeyEventPair(NX_KEYTYPE_MUTE, NSShiftKeyMask);
}

void MediaInterfaceMac::SendSpecialKeyEvent(int32_t keyType, uint32_t mask, bool isDown) {
  const NSUInteger flags = (isDown ? NX_KEYDOWN : NX_KEYUP) << 8;
  const NSInteger data1 = (keyType << 16) | flags;
  mask = ((mask & 0xFFFF0000) >> 16) | (mask & 0xFFFF0000);

  // Create a system-defined event in order to send our special-key event
  NSEvent* event = [NSEvent otherEventWithType:NSSystemDefined
                            location:NSZeroPoint
                            modifierFlags:mask | 0x100
                            timestamp:0
                            windowNumber:0
                            context:nullptr
                            subtype:NX_SUBTYPE_AUX_CONTROL_BUTTONS
                            data1:data1
                            data2:-1];
  if (!event) {
    return;
  }
  @autoreleasepool {
    CGEventRef eventRef = [event CGEvent];
    CGEventPost(kCGHIDEventTap, eventRef);
  }
  [event release];
}
