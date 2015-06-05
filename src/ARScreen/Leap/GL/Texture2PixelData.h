#pragma once

#include "Leap/GL/GLHeaders.h" // convenience header for cross-platform GL includes
#include "Leap/GL/Texture2Exception.h"
#include <unordered_map>

namespace Leap {
namespace GL {

;
/// @brief Container class for the payload in texture writing/reading operations in Texture2.
/// @details This class specifies formatted data for use in texture writing/reading operations.
/// The data it specifies is the following:
/// - A pointer to a readable and/or writeable buffer of untyped data (i.e. a void pointer.
/// - The number of bytes in said buffer.
/// - The format of that data; in particular, a "format" (e.g. GL_RGB, GL_RGBA) and a "type" (e.g. GL_UNSIGNED_BYTE, GL_FLOAT).
/// - Pixel-store parameters.
///
/// A Texture2PixelData object can be "empty" (indicated by @c IsEmpty).  An empty Texture2PixelData
/// object has no format or type and is constructed via the default constructor.  Its associated buffer
/// pointer is null, and the number of bytes in the pointer is 0.
///
/// A non-empty Texture2PixelData object can be readable and/or writeable.  A non-empty Texture2PixelData
/// object must have valid format and type properties, and the associated buffer pointer must be non-null
/// and the number of bytes in the buffer must be positive.  A readable Texture2PixelData object can be
/// used in texture upload operations (e.g. @c Texture2::Initialize, @c Texture2::TexSubImage), and a
/// writeable Texture2PixelData object can be used in texture download operations (e.g.
/// @c Texture2::GetTexImage).
///
/// The only exceptions that this class explicitly throws derive from Texture2Exception.
///
/// These are invaluable resources:
/// - http://www.opengl.org/wiki/Common_Mistakes
/// - http://www.opengl.org/wiki/Pixel_Transfer#Pixel_layout
///
/// The OpenGL API website docs incorrectly formatted some of the formulas for glPixelStorei,
/// but the MSDN website had them correctly formatted: http://msdn.microsoft.com/en-us/library/windows/desktop/dd940385(v=vs.85).aspx
class Texture2PixelData {
public:

  /// @brief Returns the number of components in format.  E.g. ComponentsInFormat(GL_RGBA) is 4.
  static size_t ComponentsInFormat (GLenum format);
  /// @brief Returns the number of bytes in the type.  E.g. BytesInType(GL_UNSIGNED_BYTE) is 1.
  static size_t BytesInType (GLenum type);

  /// @brief Convenience typedef for the pixel store parameter mapping.
  typedef std::unordered_map<GLenum,GLint> GLPixelStoreiParameterMap;

  /// @brief Specifies an IsEmpty pixel data structure.
  /// @details Default, valid format and type values are assigned, but they aren't used by Texture2 if IsEmpty
  /// returns is true.  This indicates that OpenGL should allocate texture memory automatically.
  Texture2PixelData ();
  /// @brief Specifies a pixel format/type and read-only pixel data.
  /// @details The pointer must be non-null, and raw_data_byte_count must be positive.  The way the data is
  /// interpreted by OpenGL depends on the pixel store parameters (see glPixelStorei).
  Texture2PixelData (GLenum format, GLenum type, const void *readable_raw_data, size_t raw_data_byte_count);
  /// @brief Specifies a pixel format/type and pixel data that is readable and writable.
  /// @details The pointer must be non-null, and raw_data_byte_count must be positive.  The way the data is
  /// interpreted by OpenGL depends on the pixel store parameters (see glPixelStorei).
  Texture2PixelData (GLenum format, GLenum type, void *readable_and_writeable_raw_data, size_t raw_data_byte_count);

  /// @brief Returns the format of this object.
  /// @details If IsEmpty(), then this will return an arbitrary but valid value.
  GLenum Format () const { return m_format; }
  /// @brief Returns the format of this object.
  /// @details If IsEmpty(), then this will return an arbitrary but valid value.
  GLenum Type () const { return m_type; }
  /// @brief Returns true if and only if this object is empty (i.e. refers to no data).
  bool IsEmpty () const { return m_raw_data_byte_count == 0; }
  /// @brief Returns true if and only if this object is non-empty and has a readable buffer.
  bool IsReadable () const { return m_readable_raw_data != nullptr; }
  /// @brief Returns true if and only if this object is non-empty and has a writeable buffer.
  bool IsWriteable () const { return m_writeable_raw_data != nullptr; }
  /// @brief Returns a non-null pointer if this IsReadable, otherwise returns nullptr.
  const void *ReadableRawData () const { return m_readable_raw_data; }
  /// @brief Returns a non-null pointer if this IsWriteable, otherwise returns nullptr.
  void *WriteableRawData () const { return m_writeable_raw_data; }
  /// @brief If IsReadable or IsWriteable, returns the number of bytes of data in the array returned
  /// by each of ReadableRawData and WriteableRawData (whichever one is relevant), otherwise returns 0.
  size_t RawDataByteCount () const { return m_raw_data_byte_count; }

  /// @brief Returns true iff pname exists in the PixelStorei parameter map (i.e. SetPixelStoreiParameter has been called with pname).
  bool HasPixelStoreiParameter (GLenum pname) const { return m_pixel_store_i_parameter.find(pname) != m_pixel_store_i_parameter.end(); }
  /// @brief Returns the value of the requested PixelStorei parameter mapping, or throws Texture2Exception if not set.
  GLint PixelStoreiParameter (GLenum pname) const;
  /// @brief Returns a const reference to the PixelStorei parameter map.
  const GLPixelStoreiParameterMap &PixelStoreiParameterMap () const { return m_pixel_store_i_parameter; }

  /// @brief Sets the format and type of the pixel data.
  /// @details This will throw Texture2Exception if either argument is not valid.
  void SetFormatAndType (GLenum format, GLenum type);
  /// @brief Clears the raw data pointers and byte count, thereby making this IsEmpty.
  /// @details Does not modify the format/type or pixel store params.
  void MakeEmpty ();
  /// @brief Sets a readable raw pixel data pointer (making IsReadable() true and IsWriteable() false).
  void MakeReadable (const void *readable_raw_data, size_t raw_data_byte_count);
  /// @brief Sets a readable-and-writable raw pixel data pointer (making IsReadable() and IsWriteable() both true).
  void MakeReadableAndWriteable (void *readable_and_writable_raw_pixel_data, size_t raw_data_byte_count);
  /// @brief Record that the PixelStorei parameter given by pname will be set to the value given in param.
  void SetPixelStoreiParameter (GLenum pname, GLint param);
  /// @brief Clears the PixelStorei parameter map.
  void ClearPixelStoreiParameterMap () { m_pixel_store_i_parameter.clear(); }

private:

  GLenum m_format;
  GLenum m_type;
  const void *m_readable_raw_data;
  void *m_writeable_raw_data;
  size_t m_raw_data_byte_count;
  GLPixelStoreiParameterMap m_pixel_store_i_parameter;
};

} // end of namespace GL
} // end of namespace Leap
