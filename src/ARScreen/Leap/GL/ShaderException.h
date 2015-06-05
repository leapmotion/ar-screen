#pragma once

#include "Leap/GL/Exception.h"

namespace Leap {
namespace GL {

/// @brief Base class for all Leap::GL::Shader exceptions.
class ShaderException : public Exception {
public:

  ShaderException (const std::string &message) : Exception(message) { }
};

} // end of namespace GL
} // end of namespace Leap
