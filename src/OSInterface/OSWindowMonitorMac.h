#pragma once
#include "OSWindowMonitor.h"

#include <atomic>

class OSWindowEvent;
class OSWindowMac;

class OSWindowMonitorMac:
  public OSWindowMonitor
{
public:
  OSWindowMonitorMac(void);
  ~OSWindowMonitorMac(void);

  // OSWindowMonitor overrides:
  void Scan() override;

private:
  mutable std::mutex m_lock;

  // OS window event:
  AutoFired<OSWindowEvent> m_oswe;

  // Current collection of known top-level windows
  typedef std::unordered_map<CGWindowID, std::shared_ptr<OSWindowMac>> t_knownWindows;
  t_knownWindows m_knownWindows;
  std::atomic<uint32_t> m_mark;

public:
  // OSWindowMonitor overrides:
  void Enumerate(const std::function<void(OSWindow&)>& callback) const override;
  std::shared_ptr<OSWindow> WindowFromPoint(OSPoint point) override;
};
