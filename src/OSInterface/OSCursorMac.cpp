#include "stdafx.h"
#include "OSCursorMac.h"

#include <cmath>

OSCursor* OSCursor::New(void) {
  return new OSCursorMac;
}

OSPoint OSCursorMac::GetCursorPos(void) const {
  CGEventRef event = CGEventCreate(0);
  OSPoint retVal = CGEventGetLocation(event);
  retVal.x = std::floor(retVal.x);
  retVal.y = std::floor(retVal.y);
  CFRelease(event);
  return retVal;
}

void OSCursorMac::SetCursorPos(OSPoint point) const {
  CGWarpMouseCursorPosition(point);
}
