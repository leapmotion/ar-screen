#pragma once

/// @file GLHeaders.h
///
/// This header is only intended to take care of the annoying OpenGL header
/// includes in a cross-platform way.  In particular, it uses GLEW to do the
/// heavy lifting, but also does some of its own platform-specific compilation.
///
/// The function Leap::GL::InitializeGlew should be used to initialize the OpenGL
/// apparatus before any GL calls are made.

#if __APPLE__
  #include <GL/glew.h>
#elif _WIN32
  #define GLEW_STATIC
  #include <GL/glew.h>

  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif

  #ifndef NOMINMAX
    #define NOMINMAX
  #endif

  #include "windows.h"
#else
  #include <GL/glew.h>
#endif

#include <ostream>
#include "Leap/GL/Exception.h"

namespace Leap {
namespace GL {

/// @brief Initialize the OpenGL apparatus.
/// @details This will throw a Leap::GL::Exception if the initialization failed.
/// The optional out parameter can be used to specify a std::ostream to print some
/// diagnostic information to.  The default is nullptr, which suppresses printout.
inline void InitializeGlew (std::ostream *out = nullptr) {
  if (glewInit() != GLEW_OK) {
    throw Leap::GL::Exception("Glew initialization failed");
  }
  if (out != nullptr) {
    *out << "GL_VERSION = \"" << glGetString(GL_VERSION) << "\"\n";
    *out << "GL_RENDERER = \"" << glGetString(GL_RENDERER) << "\"\n";
    *out << "GL_VENDOR = \"" << glGetString(GL_VENDOR) << "\"\n";
    // *out << "GL_EXTENSIONS = \"" << glGetString(GL_EXTENSIONS) << "\"\n";
  }
}

} // end of namespace GL
} // end of namespace Leap
