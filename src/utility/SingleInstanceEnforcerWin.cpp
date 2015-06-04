#include "stdafx.h"
#include "SingleInstanceEnforcerWin.h"

#include <codecvt>
#include <locale>
#include <stdexcept>

SingleInstanceEnforcer::SingleInstanceEnforcer(const wchar_t* appName)
{
  const std::wstring appNameStr = std::wstring(L"Local\\") + appName;

  m_sharedMutex.reset(CreateMutexW(NULL, TRUE, appNameStr.c_str()));
  DWORD lastError = GetLastError();
  if (lastError != ERROR_SUCCESS) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
    throw std::runtime_error("Error creating shared mutex " + converter.to_bytes(appName));
  }
}
