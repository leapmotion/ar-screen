#pragma once
#include "OSCursor.h"

class OSCursorMac:
  public OSCursor
{
public:
  // OSCursor overrides:
  OSPoint GetCursorPos(void) const override;
  void SetCursorPos(OSPoint point) const override;
};