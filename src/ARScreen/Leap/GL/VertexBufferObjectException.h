#pragma once

#include "Leap/GL/Exception.h"

namespace Leap {
namespace GL {

/// @brief Base class for all Leap::GL::VertexBufferObject exceptions.
class VertexBufferObjectException : public Exception {
public:

  VertexBufferObjectException (const std::string &message) : Exception(message) { }
};

} // end of namespace GL
} // end of namespace Leap
