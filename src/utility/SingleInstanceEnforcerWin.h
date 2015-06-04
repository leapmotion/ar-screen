#pragma once

#include "HandleUtilitiesWin.h"
#include <Windows.h>

class SingleInstanceEnforcer
{
public:
  SingleInstanceEnforcer(const wchar_t* appName);
private:
  unique_ptr_of<HANDLE> m_sharedMutex;
};
