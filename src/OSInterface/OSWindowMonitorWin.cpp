#include "stdafx.h"
#include "OSWindowMonitorWin.h"
#include "OSWindowWin.h"
#include "OSWindowEvent.h"

OSWindowMonitorWin::OSWindowMonitorWin(void)
{
  // Trigger initial enumeration
  Scan();
}

OSWindowMonitorWin::~OSWindowMonitorWin(void)
{
}

OSWindowMonitor* OSWindowMonitor::New(void) {
  return new OSWindowMonitorWin;
}

void OSWindowMonitorWin::Enumerate(const std::function<void(OSWindow&)>& callback) const {
  std::lock_guard<std::mutex> lk(m_lock);
  for(auto& knownWindow : m_knownWindows)
    callback(*knownWindow.second);
}

struct enum_cblock {
  std::unordered_map<HWND, int> hwnds;
  int index;
};

void OSWindowMonitorWin::Scan() {
  enum_cblock block;
  block.index = 0;

  // Enumerate all top-level windows that we know about right now:
  EnumWindows(
    [] (HWND hwnd, LPARAM lParam) -> BOOL {
      auto& block = *(enum_cblock*) lParam;

      // Short-circuit if this window can't be seen
      if(!IsWindowVisible(hwnd))
        return true;

      // Don't bother enumerating iconic (minimized) windows, either
      if(IsIconic(hwnd))
        return true;

      // See if we are the last active visible popup
      HWND hwndWalk = GetAncestor(hwnd, GA_ROOTOWNER);

      LONG style = GetWindowLong(hwndWalk, GWL_EXSTYLE);
      if(style & WS_EX_TOOLWINDOW)
        // No toolbar windows, they are not allowed to appear in the topmost window list by definition
        return true;
      
      // Do not try to enumerate anything we own
      DWORD pid = 0;
      GetWindowThreadProcessId(hwnd, &pid);
      if(pid == GetCurrentProcessId())
        return true;
      
      for(HWND hwndTry = hwndWalk; hwndTry; ) {
        // Advance to the next spot:
        std::tie(hwndTry, hwndWalk) = std::make_tuple(GetLastActivePopup(hwndWalk), hwndTry);

        if(hwndTry == hwndWalk)
          // We haven't moved, we can end here
          break;

        if(IsWindowVisible(hwndTry))
          // Popup is visible, end here
          break;
      }

      if(hwndWalk == hwnd)
        // Add this window in, it would appear in the alt-tab list:
        block.hwnds[hwnd] = block.index--;
      return true;
    },
    (LPARAM) &block
  );

  // Figure out which windows are gone:
  std::unordered_map<HWND, std::shared_ptr<OSWindowWin>> pending;
  {
    std::lock_guard<std::mutex> lk(m_lock);
    for(auto knownWindow : m_knownWindows) {
      auto q = block.hwnds.find(knownWindow.first);
      if (q == block.hwnds.end())
        // Window was gone the last time we enumerated, give up
        pending.insert(knownWindow);
      else
        // Found this window, update its z-order
        knownWindow.second->SetZOrder(q->second);
    }

    for(auto q : pending)
      m_knownWindows.erase(q.first);
  }

  // Fire destroyed notifications off while outside of the lock:
  for(auto q : pending)
    m_oswe(&OSWindowEvent::OnDestroy)(*q.second);
  pending.clear();

  // Fire all resize events as needed:
  for(auto& q : m_knownWindows)
    q.second->CheckSize(m_oswe);

  // Create any windows which have been added:
  for(auto q : block.hwnds)
    if(!m_knownWindows.count(q.first)) {
      auto wnd = std::make_shared<OSWindowWin>(q.first);
      wnd->SetZOrder(q.second);
      pending[q.first] = wnd;
    }

  // Add to the collection:
  {
    std::lock_guard<std::mutex> lk(m_lock);
    m_knownWindows.insert(pending.begin(), pending.end());
  }

  // Creation notifications now
  for(auto q : pending)
    m_oswe(&OSWindowEvent::OnCreate)(*q.second);
}

std::shared_ptr<OSWindow> OSWindowMonitorWin::WindowFromPoint(OSPoint point) {
  POINT pt = {
    (LONG)point.x,
    (LONG)point.y
  };
  HWND hwnd = ::WindowFromPoint(pt);
  if(!hwnd)
    // No window here, give up
    return nullptr;

  std::unique_lock<std::mutex> lk(m_lock);

  // See if this window already exists:
  auto q = m_knownWindows.find(hwnd);
  if(q != m_knownWindows.end())
    return q->second;

  // Window doesn't exist, we need to create a representation and fire a create event
  auto createdWindow = std::make_shared<OSWindowWin>(hwnd);
  m_knownWindows[hwnd] = createdWindow;
  lk.unlock();
  m_oswe(&OSWindowEvent::OnCreate)(*createdWindow);
  return createdWindow;
}