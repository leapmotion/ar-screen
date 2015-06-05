#include "stdafx.h"
#include "OSAppManager.h"
#include "OSApp.h"

OSAppManager::OSAppManager(void):
  ContextMember("OSAppManager")
{
}

OSAppManager::~OSAppManager(void)
{
}

std::shared_ptr<OSApp> OSAppManager::GetApp(uint32_t pid) {
  std::lock_guard<std::mutex> lock(m_mutex);

  const std::wstring id = OSApp::GetAppIdentifier(pid);
  if (id.empty()) {
    return std::shared_ptr<OSApp>();
  }
  auto found = m_cache.find(id);
  if (found != m_cache.end()) {
    std::shared_ptr<OSApp> app = found->second.lock();
    if (app) {
      return app;
    }
  }
  std::shared_ptr<OSApp> app(OSApp::New(pid));
  if (app) {
    m_cache[id] = app;
  }
  return app;
}
