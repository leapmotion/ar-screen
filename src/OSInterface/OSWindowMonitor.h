#pragma once
#include "OSGeometry.h"
#include <functional>

class OSWindow;

/// <summary>
/// Top-level monitor type used to monitor the OS for window creation and destruction events
/// </summary>
/// <seealso cref="OSWindowEvent">
/// This type describes the circumstances under which important events are raised
/// </seealso>
class OSWindowMonitor:
  public ContextMember
{
public:
  OSWindowMonitor(void);
  ~OSWindowMonitor(void);

  static OSWindowMonitor* New(void);

  /// <summary>
  /// Routine to scan and update the window list
  /// </summary>
  virtual void Scan(void) = 0;

public:
  /// <summary>
  /// Enumeration routine, used to list all windows presently known to the monitor
  /// </summary>
  virtual void Enumerate(const std::function<void(OSWindow&)>& callback) const = 0;

  /// <summary>
  /// Returns the topmost os window under the specified point
  /// <summary>
  /// <remarks>
  /// If a window under the requested point was not formerly enumerated, a call to this
  /// method may potentially result in an OnCreate event being raised
  /// </remarks>
  virtual std::shared_ptr<OSWindow> WindowFromPoint(OSPoint point) = 0;
};
