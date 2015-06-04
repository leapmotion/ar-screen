#pragma once

#include "SingleInstanceEnforcerWin.h"
class PlatformInitializer
{
public:
  /// <summary>
  /// Calls CoInitialize for multithreaded COM, or throws an exception if this could not be done
  /// </summary>
  PlatformInitializer(void);
  ~PlatformInitializer(void);

private:
  const HRESULT m_hr;
  SingleInstanceEnforcer m_enforcer;

public:
  operator HRESULT() const { return m_hr; }
};
