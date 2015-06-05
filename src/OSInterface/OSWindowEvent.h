#pragma once

class OSWindowEvent
{
public:
  virtual ~OSWindowEvent(void) {}

  /// <summary>
  /// Fired when a window is created or made visible to the user
  /// </summary>
  virtual void OnCreate(OSWindow& window) {}

  /// <summary>
  /// Fired when a window is destroyed or made invisible to the user
  /// </summary>
  /// <remarks>
  /// A window may be considered invisible to the user if it's properly invisible, cloaked, or minimized
  /// </remarks>
  virtual void OnDestroy(OSWindow& window) {}

  /// <summary>
  /// Fired when a window is resized
  /// </summary>
  virtual void OnResize(OSWindow& window) {}
};
