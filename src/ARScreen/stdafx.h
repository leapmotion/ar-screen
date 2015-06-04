#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#include <autowiring/autowiring.h>

#ifdef _MSC_VER
  #include <Windows.h>
#undef min //WTF windows.h
#undef max
#endif