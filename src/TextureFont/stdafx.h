#pragma once

#include <autowiring/autowiring.h>
#include <algorithm>
#include <assert.h>

// opengl
#if __APPLE__
#include <GL/glew.h>
#elif _WIN32
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#ifdef _MSC_VER
  #include "utility/WindowsIncludes.h"
#endif

#include "freetype-gl.h"
