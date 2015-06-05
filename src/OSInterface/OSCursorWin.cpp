#include "stdafx.h"
#include "OSCursorWin.h"

OSCursor* OSCursor::New(void) {
  return new OSCursorWin;
}

OSPoint OSCursorWin::GetCursorPos(void) const {
  POINT retVal;
  ::GetCursorPos(&retVal);
  return OSPoint{(float) retVal.x, (float) retVal.y};
}

void OSCursorWin::SetCursorPos(OSPoint point) const {
  ::SetCursorPos((int)point.x, (int)point.y);
}