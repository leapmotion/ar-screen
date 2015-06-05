#pragma once
#include "OSCursor.h"

class OSCursorWin:
  public OSCursor
{
public:
  // OSCursor overrides:
  OSPoint GetCursorPos(void) const override;
  void SetCursorPos(OSPoint point) const override;
};