#pragma once
#include "OSWindowMonitor.h"

class OSWindowEvent;
class OSWindowWin;

class OSWindowMonitorWin:
  public OSWindowMonitor
{
public:
  OSWindowMonitorWin(void);
  ~OSWindowMonitorWin(void);

  // OSWindowMonitor overrides:
  void Scan() override;

private:
  mutable std::mutex m_lock;

  // OS window event:
  AutoFired<OSWindowEvent> m_oswe;

  // Current collection of known top-level windows
  typedef std::unordered_map<HWND, std::shared_ptr<OSWindowWin>> t_knownWindows;
  t_knownWindows m_knownWindows;

public:
  // OSWindowMonitor overrides:
  void Enumerate(const std::function<void(OSWindow&)>& callback) const override;
  std::shared_ptr<OSWindow> WindowFromPoint(OSPoint point) override;
};
