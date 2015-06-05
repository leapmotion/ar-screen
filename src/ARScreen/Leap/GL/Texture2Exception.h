#pragma once

#include "Leap/GL/Exception.h"

namespace Leap {
namespace GL {

/// @brief Base class for all Leap::GL::Texture2 exceptions.
class Texture2Exception : public Exception {
public:

  Texture2Exception (const std::string &message) : Exception(message) { }
};

} // end of namespace GL
} // end of namespace Leap
