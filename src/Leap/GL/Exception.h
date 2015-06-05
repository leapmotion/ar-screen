#pragma once

#include <stdexcept>
#include <string>

namespace Leap {
namespace GL {

/// @brief Base class for all Leap::GL exceptions.
class Exception : public std::runtime_error {
public:
  Exception (const std::string &message) : std::runtime_error(message) { }
};

} // end of namespace GL
} // end of namespace Leap
