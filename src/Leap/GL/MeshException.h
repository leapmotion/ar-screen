#pragma once

#include "Leap/GL/Exception.h"

namespace Leap {
namespace GL {

/// @brief Base class for all Leap::GL::Mesh exceptions.
class MeshException : public Exception {
public:

  MeshException (const std::string &message) : Exception(message) { }
};

} // end of namespace GL
} // end of namespace Leap
