#pragma once

#include <string>

namespace Leap {
namespace GL {

class Texture2;
class Texture2Params;

} // end of namespace GL
} // end of namespace Leap

// The only value that must be set in the passed-in Texture2Params is "target".
// If it is desired to specify any TexParameter values, this must be done before
// calling this function.  The other properties (width, height, internal format,
// pixel data format, pixel data type) are all determined from the loaded image.
// The Texture2Params property of the returned Texture2 will be fully populated
// with all the determined values.
// NOTE: In principle, the internal format property could be specified beforehand,
// in which case it would be a hint to OpenGL for how the texture should be stored
// internally.
Leap::GL::Texture2 *LoadGLTexture2UsingFreeImage (const std::string &filepath, const Leap::GL::Texture2Params &params);
