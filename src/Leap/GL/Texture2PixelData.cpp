#include "stdafx.h"
#include "Leap/GL/Texture2PixelData.h"

#include <cassert>

namespace Leap {
namespace GL {

// ////////////////////////////////////////////////////////////////////////////////////////////////
// Texture2PixelData
// ////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: somehow make this less version-specific (?), or come up with a version-agnostic way
// to determine the size of each pixel from given pixel data format and type.
size_t Texture2PixelData::ComponentsInFormat (GLenum format) {
  // Allowable OpenGL 2.1 values: GL_COLOR_INDEX, GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA, GL_RGB, GL_BGR, GL_RGBA, GL_BGRA, GL_LUMINANCE, GL_LUMINANCE_ALPHA
  // Allowable OpenGL 3.3 values: GL_RED, GL_RG, GL_RGB, GL_BGR, GL_RGBA, GL_BGRA, GL_DEPTH_COMPONENT, GL_DEPTH_STENCIL
  // Overlap between 2.1 and 3.3: GL_RED, GL_RGB, GL_BGR, GL_RGBA, GL_BGRA
  // Only in OpenGL 2.1         : GL_COLOR_INDEX, GL_GREEN, GL_BLUE, GL_ALPHA, GL_LUMINANCE, GL_LUMINANCE_ALPHA
  // Only in OpenGL 3.3         : GL_RG, GL_DEPTH_COMPONENT, GL_DEPTH_STENCIL
  switch (format) {
    case GL_COLOR_INDEX:
    case GL_RED:
    case GL_GREEN:
    case GL_BLUE:
    case GL_ALPHA:
    case GL_LUMINANCE:
    case GL_DEPTH_COMPONENT:  return 1;

    case GL_LUMINANCE_ALPHA:
    case GL_RG:
    case GL_DEPTH_STENCIL:    return 2;

    case GL_RGB:
    case GL_BGR:              return 3;

    case GL_RGBA:
    case GL_BGRA:             return 4;

    default: throw Texture2Exception("invalid pixel format; must be one of GL_COLOR_INDEX, GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA, GL_RG, GL_RGB, GL_BGR, GL_RGBA, GL_BGRA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_DEPTH_COMPONENT, GL_DEPTH_STENCIL");
  }
}

size_t Texture2PixelData::BytesInType (GLenum type) {
  // Allowable OpenGL 2.1 values: GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT, GL_UNSIGNED_BYTE_3_3_2, GL_UNSIGNED_BYTE_2_3_3_REV, GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_SHORT_5_6_5_REV, GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_SHORT_4_4_4_4_REV, GL_UNSIGNED_SHORT_5_5_5_1, GL_UNSIGNED_SHORT_1_5_5_5_REV, GL_UNSIGNED_INT_8_8_8_8, GL_UNSIGNED_INT_8_8_8_8_REV, GL_UNSIGNED_INT_10_10_10_2, GL_UNSIGNED_INT_2_10_10_10_REV, GL_BITMAP, 
  // Allowable OpenGL 3.3 values: GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT, GL_UNSIGNED_BYTE_3_3_2, GL_UNSIGNED_BYTE_2_3_3_REV, GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_SHORT_5_6_5_REV, GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_SHORT_4_4_4_4_REV, GL_UNSIGNED_SHORT_5_5_5_1, GL_UNSIGNED_SHORT_1_5_5_5_REV, GL_UNSIGNED_INT_8_8_8_8, GL_UNSIGNED_INT_8_8_8_8_REV, GL_UNSIGNED_INT_10_10_10_2, GL_UNSIGNED_INT_2_10_10_10_REV.
  // Overlap between 2.1 and 3.3: all but GL_BITMAP, which only occurs in OpenGL 2.1.  This one will not be supported for now.
  switch (type) {
    case GL_UNSIGNED_BYTE:
    case GL_BYTE:
    case GL_UNSIGNED_BYTE_3_3_2:
    case GL_UNSIGNED_BYTE_2_3_3_REV:      return 1;
      
    case GL_UNSIGNED_SHORT:
    case GL_SHORT:
    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_SHORT_5_6_5_REV:
    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_4_4_4_4_REV:
    case GL_UNSIGNED_SHORT_5_5_5_1:
    case GL_UNSIGNED_SHORT_1_5_5_5_REV:   return 2;
      
    case GL_UNSIGNED_INT:
    case GL_INT:
    case GL_FLOAT:
    case GL_UNSIGNED_INT_8_8_8_8:
    case GL_UNSIGNED_INT_8_8_8_8_REV:
    case GL_UNSIGNED_INT_10_10_10_2:
    case GL_UNSIGNED_INT_2_10_10_10_REV:  return 4;
      
    default: throw Texture2Exception("invalid pixel type; must be one of GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_FLOAT, GL_UNSIGNED_BYTE_3_3_2, GL_UNSIGNED_BYTE_2_3_3_REV, GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_SHORT_5_6_5_REV, GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_SHORT_4_4_4_4_REV, GL_UNSIGNED_SHORT_5_5_5_1, GL_UNSIGNED_SHORT_1_5_5_5_REV, GL_UNSIGNED_INT_8_8_8_8, GL_UNSIGNED_INT_8_8_8_8_REV, GL_UNSIGNED_INT_10_10_10_2, GL_UNSIGNED_INT_2_10_10_10_REV.");
  }
}

Texture2PixelData::Texture2PixelData () {
  SetFormatAndType(GL_RGBA, GL_UNSIGNED_BYTE); // Reasonable defaults that will not be used anyway if IsEmpty.
  MakeEmpty();
}

Texture2PixelData::Texture2PixelData (GLenum format, GLenum type, const void *readable_raw_data, size_t raw_data_byte_count) {
  SetFormatAndType(format, type);
  MakeReadable(readable_raw_data, raw_data_byte_count);
}

Texture2PixelData::Texture2PixelData (GLenum format, GLenum type, void *readable_and_writeable_raw_data, size_t raw_data_byte_count) {
  SetFormatAndType(format, type);
  MakeReadableAndWriteable(readable_and_writeable_raw_data, raw_data_byte_count);
}

GLint Texture2PixelData::PixelStoreiParameter (GLenum pname) const {
  // TODO: validate that pname is a valid argument for this function (see docs of glPixelStorei)
  auto it = m_pixel_store_i_parameter.find(pname);
  if (it == m_pixel_store_i_parameter.end()) {
    throw Texture2Exception("specified GLint-valued PixelStorei parameter not found and/or specified");
  }
  return it->second;
}

void Texture2PixelData::SetFormatAndType (GLenum format, GLenum type) {
  // TODO: validity checking for format and type.
  m_format = format;
  m_type = type;

  // The following calls do checks for validity (basically checking that the format and type are each valid
  // in the sense that they're acceptable values for OpenGL 2.1 or OpenGL 3.3), throwing Texture2Exception
  // if their arguments are invalid.
  size_t bytes_in_pixel = ComponentsInFormat(format)*BytesInType(type);
  // NOTE: TEMPORARY hacky handling of GL_UNPACK_ALIGNMENT, so that the assumption that all pixel
  // data is layed out contiguously in the raw pixel data is correct (it isn't necessarily, as
  // OpenGL has a row alignment feature, which depends on GL_UNPACK_ALIGNMENT).
  if (bytes_in_pixel % 4 != 0) {
    // The default is 4, so if our pixels don't align to 4 bytes, just hackily set it to 1.
    // This probably slows things down a bit, so TODO this should be re-engineered correctly later.
//     SetPixelStoreiParameter(GL_UNPACK_ALIGNMENT, 1);
    // If this value is overridden, it's assumed that the overrider knows what they're doing.
  }
}

void Texture2PixelData::MakeEmpty () {
  m_readable_raw_data = nullptr;
  m_writeable_raw_data = nullptr;
  m_raw_data_byte_count = 0;
  assert(IsEmpty());
}

void Texture2PixelData::MakeReadable (const void *readable_raw_data, size_t raw_data_byte_count) {
  if (readable_raw_data == nullptr)
    throw Texture2Exception("readable_raw_data must be non-null.");
  if (raw_data_byte_count == 0)
    throw Texture2Exception("raw_data_byte_count must be positive, indicating the size of the buffer specified by readable_raw_data.");
  m_readable_raw_data = readable_raw_data;
  m_writeable_raw_data = nullptr;
  m_raw_data_byte_count = raw_data_byte_count;
  assert(!IsEmpty());
  assert(IsReadable());
  assert(!IsWriteable());
}

void Texture2PixelData::MakeReadableAndWriteable (void *readable_and_writeable_raw_data, size_t raw_data_byte_count) {
  if (readable_and_writeable_raw_data == nullptr)
    throw Texture2Exception("readable_and_writeable_raw_data must be non-null.");
  if (raw_data_byte_count == 0)
    throw Texture2Exception("raw_data_byte_count must be positive, indicating the size of the buffer specified by readable_and_writeable_raw_data.");
  m_readable_raw_data = readable_and_writeable_raw_data;
  m_writeable_raw_data = readable_and_writeable_raw_data;
  m_raw_data_byte_count = raw_data_byte_count;
  assert(!IsEmpty());
  assert(IsReadable());
  assert(IsWriteable());
}

void Texture2PixelData::SetPixelStoreiParameter (GLenum pname, GLint param) {
  // TODO: validate that pname is a valid argument for this function (see docs of glPixelStorei)
  m_pixel_store_i_parameter[pname] = param;
}

} // end of namespace GL
} // end of namespace Leap
