#pragma once
#include "OSGeometry.h"
#include <autowiring/autowiring.h>

class OSCursor:
  public ContextMember
{
public:
  OSCursor(void);
  virtual ~OSCursor(void) {}

  static OSCursor* New(void);

public:
  /// <returns>
  /// The position of the mouse cursor at the time of the call
  /// </returns>
  virtual OSPoint GetCursorPos(void) const = 0;

  /// <returns>
  /// The position of the mouse cursor at the time of the call
  /// </returns>
  virtual void SetCursorPos(OSPoint point) const = 0;
};
