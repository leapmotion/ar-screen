#pragma once
#include "OSGeometry.h"
#include <cstdint>
#include <memory>

class ImagePrimitive;
class OSApp;
class OSWindowNode;

/// <summary>
/// A platform-independent representation of a single window
/// </summary>
class OSWindow:
  public std::enable_shared_from_this<OSWindow>
{
public:
  OSWindow(void);
  virtual ~OSWindow(void);

protected:
  std::shared_ptr<OSApp> m_app;
  int m_zOrder;

public:
  /// <summary>
  /// Returns the z-order for this window with respect to other topmost windows
  /// </summary>
  /// <remarks>
  /// The return value of this function can change as window stacking order evolves.  A
  /// value of 0 is reserved for the topmost window.  Values less than zero indicate windows
  /// that are further in the background.  There is no guarantee that z-order values are
  /// continuous.  Positive values are defined to be invalid.
  /// </remarks>
  int GetZOrder(void) const { return m_zOrder; }

  /// <returns>True if this window is still valid</returns>
  /// <remarks>
  /// A window handle can become invalid for many reasons.  The most likely cause, generally,
  /// is that the window itself has closed while enumeration is underway.  Representations of
  /// invalid handles should generally be destroyed as quickly as possible, as no operations
  /// on an invalid handle can be valid.
  /// </remarks>
  virtual bool IsValid(void) = 0;

  /// <returns>
  /// The PID of the owning application
  /// </returns>
  virtual uint32_t GetOwnerPid(void) = 0;

  /// <returns>
  /// The application named by
  /// </returns>
  virtual std::shared_ptr<OSApp> GetOwnerApp(void) const { return m_app; }

  /// <returns>
  /// A unique identifier for this window
  /// </returns>
  virtual uint64_t GetWindowID(void) const = 0;

  /// <summary>
  /// Copies an image corresponding to a window into the specified image primitive
  /// </summary>
  /// <param name="img">The prior texture, or nullptr if this is the first invocation</param>
  /// <returns>
  /// Either an updated version of the passed texture, or a new texture.  The returned ImagePrimitive
  /// may potentially not have any bound texture in cases where the window's texture could not be
  /// captured.
  /// </returns>
  virtual std::shared_ptr<ImagePrimitive> GetWindowTexture(std::shared_ptr<ImagePrimitive> img) = 0;

  /// <returns>
  /// True if this window has focus
  /// </returns>
  virtual bool GetFocus(void) = 0;

  /// <summary>
  /// Makes this window a foreground window
  /// </summary>
  virtual void SetFocus(void) = 0;

  /// <returns>
  /// The title of this window
  /// </returns>
  /// <remarks>
  /// The window title is not the same as the application name in that the window title is
  /// only what's represented to the user, and therefore can change over time
  /// </remarks>
  virtual std::wstring GetTitle(void) = 0;

  /// <returns>
  /// The location of the top-left corner of the window
  /// </returns>
  virtual OSPoint GetPosition(void) = 0;

  /// <returns>
  /// The real size of this window
  /// </retunrs>
  virtual OSSize GetSize(void) = 0;

  /// <summary>
  /// Prevents the specified window from being rendered on-screen
  /// </summary>
  virtual void Cloak(void) = 0;

  /// <summary>
  /// Reverses a prior call to Cloak
  /// </summary>
  virtual void Uncloak(void) = 0;

  /// <returns>
  /// True if this window is visible, false otherwise
  /// </returns>
  /// <remarks>
  /// A prior call to Cloak can make this window invisible.
  /// </remarks>
  virtual bool IsVisible(void) const = 0;

  virtual void SetPosition(const OSPoint& pos) = 0;

  virtual void SetSize(const OSSize& size) = 0;

  bool operator==(const OSWindow& rhs) const {
    return GetWindowID() == rhs.GetWindowID();
  }
};

