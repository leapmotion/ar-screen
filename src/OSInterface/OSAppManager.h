#pragma once
#include <string>
#include <memory>
#include <mutex>
#include <unordered_map>

class OSApp;

class OSAppManager:
  public ContextMember
{
public:
  OSAppManager(void);
  ~OSAppManager(void);

  /// <summary>
  /// Obtain an OSApp corresponding to the specified process identifier
  /// </summary>
  std::shared_ptr<OSApp> GetApp(uint32_t pid);

private:
  static std::wstring GetAppIdentifier(uint32_t pid);

  std::mutex m_mutex;

  std::unordered_map<std::wstring, std::weak_ptr<OSApp>> m_cache;
};
