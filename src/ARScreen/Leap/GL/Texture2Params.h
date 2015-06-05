#pragma once

#include "Leap/GL/GLHeaders.h" // convenience header for cross-platform GL includes
#include "Leap/GL/Texture2Exception.h"
#include <unordered_map>

namespace Leap {
namespace GL {

/// @brief This class encapsulates the persistent properties of a Texture2 instance.
/// @details In order to use Texture2, an instance of the Texture2Params must be specified and passed
/// into the Initialize method of Texture2 (or the corresponding constructor).  The Texture2Params
/// class specifies the following texture properties:
/// - Width (see glTexImage2D)
/// - Height (see glTexImage2D)
/// - Target (e.g. GL_TEXTURE_2D, GL_TEXTURE_CUBE_MAP_POSITIVE_X, etc) (see glTexImage2D)
/// - Internal format (e.g. GL_RGBA, GL_LUMINANCE_ALPHA, etc) (see glTexImage2D)
/// - Texture parameters (float and integer valued) (see glTexParameterf and glTexParameteri)
///
/// The width, height, target, and internal format properties are passed directly to glTexImage2D
/// during the initialization of Texture2.  The texture parameters are set via glTexPropertiesf
/// and glTexPropertiesi during initialization, and then restored to their original values after
/// initialization.
///
/// Note that when a Texture2 is initialized using a Texture2Params, the actual Texture2Params
/// value may be different from that which was passed in.  In particular, the internal format
/// may be different.  All other properties will be the same.
///
/// The only exceptions that this class explicitly throws derive from Texture2Exception.
///
/// This is an invaluable resource: http://www.opengl.org/wiki/Common_Mistakes
class Texture2Params {
public:

  /// @brief Convenience typedef for the GLfloat-valued texture parameters.
  typedef std::unordered_map<GLenum,GLfloat> GLTexParameterfMap;
  /// @brief Convenience typedef for the GLint-valued texture parameters.
  typedef std::unordered_map<GLenum,GLint> GLTexParameteriMap;

  /// @brief Reasonable default for the target parameter.
  static const GLenum DEFAULT_TARGET          = GL_TEXTURE_2D;
  /// @brief Reasonable default for the internal format parameter.
  static const GLenum DEFAULT_INTERNAL_FORMAT = GL_RGBA8;

  /// @brief Constructs a texture params object with the given paramers.
  /// @details The target, format (format), type (type), internal_format parameters
  /// are documented in the glTexImage2D docs; different versions:
  /// OpenGL 2.1: https://www.opengl.org/sdk/docs/man2/
  /// OpenGL 3.3: https://www.opengl.org/sdk/docs/man3/
  /// More at http://www.opengl.org/wiki/Image_Formats though that document may reflect an OpenGL 
  /// version later than 2.1.
  Texture2Params (GLsizei width, GLsizei height, GLenum internal_format = DEFAULT_INTERNAL_FORMAT);
  /// @brief Default constructor sets necessary values that have no reasonable default values to
  /// invalid values which *must* be filled in.  The values are the values obtained via Clear().
  Texture2Params () { Clear(); }

  // Accessors for glTexImage2D properties.

  /// @brief Returns the target parameter that will be used in Texture2::Initialize (ultimately in glTexImage2D).
  GLenum Target () const { return m_target; }
  /// @brief Returns the width parameter that will be used in Texture2::Initialize (ultimately in glTexImage2D).
  GLsizei Width () const { return m_size[0]; }
  /// @brief Returns the height parameter that will be used in Texture2::Initialize (ultimately in glTexImage2D).
  GLsizei Height () const { return m_size[1]; }
  /// @brief Returns the internal format parameter that will be used in Texture2::Initialize (ultimately in glTexImage2D).
  /// @details Note that the internal format of an initialized Texture2 may be different than the one
  /// "requested" via Texture2::Initialize.
  GLint InternalFormat () const { return m_internal_format; }
  /// @brief Convenience accessor for retrieving the size as any standard-layout type having the same size as GLint[2].
  template <typename T>
  const T &SizeAs () const {
    static_assert(sizeof(T) == sizeof(GLint)*2, "T_ must be a standard-layout type consisting of exactly 2 GLint components");
    static_assert(std::is_standard_layout<T>::value, "T_ must be a standard-layout type consisting of exactly 2 GLint components");
    // TODO: somehow check that T_ actually consists of 2 GLints.
    return *reinterpret_cast<const T *>(&m_size[0]);
  }

  // Accessors for glTexParameter* properties.

  /// @brief Returns true iff the given parameter name has been mapped to a value in the GLfloat texture parameter map.
  bool HasTexParameterf (GLenum pname) const { return m_tex_parameter_f.find(pname) != m_tex_parameter_f.end(); }
  /// @brief If pname is present in the GLfloat-valued texture parameter map, return the mapped value.  Otherwise throw Texture2Exception.
  GLfloat TexParameterf (GLenum pname) const;
  /// @brief Returns a const reference to the GLfloat-valued texture parameter map directly.
  const GLTexParameterfMap &TexParameterfMap () const { return m_tex_parameter_f; }
  /// @brief Clears the GLfloat-valued texture parameter map.
  void ClearTexParameterfMap () { m_tex_parameter_f.clear(); }
  /// @brief Returns true iff the given parameter name has been mapped to a value in the GLint texture parameter map.
  bool HasTexParameteri (GLenum pname) const { return m_tex_parameter_i.find(pname) != m_tex_parameter_i.end(); }
  /// @brief If pname is present in the GLint-valued texture parameter map, return the mapped value.  Otherwise throw Texture2Exception.
  GLint TexParameteri (GLenum pname) const;
  /// @brief Returns a const reference to the GLint-valued texture parameter map directly.
  const GLTexParameteriMap &TexParameteriMap () const { return m_tex_parameter_i; }
  /// @brief Clears the GLint-valued texture parameter map.
  void ClearTexParameteriMap () { m_tex_parameter_i.clear(); }

  // Modifiers for glTexImage2D properties.

  /// @brief Modifier for the target parameter.
  void SetTarget (GLenum target) { m_target = target; }
  /// @brief Modifier for the width parameter.
  void SetWidth (GLsizei width) { m_size[0] = width; }
  /// @brief Modifier for the height parameter.
  void SetHeight (GLsizei height) { m_size[1] = height; }
  /// @brief Modifier for the internal format parameter.
  void SetInternalFormat (GLint internal_format) { m_internal_format = internal_format; }

  // Modifiers for glTexParameter* properties.

  /// @brief Sets the named GLfloat texture parameter to the given value.
  void SetTexParameterf (GLenum pname, GLfloat value);
  /// @brief Sets the named GLint texture parameter to the given value.
  void SetTexParameteri (GLenum pname, GLint value);

  /// @brief Clears all parameters back to their default, neutral values.
  void Clear ();

private:

  GLenum m_target;
  GLsizei m_size[2];
  GLint m_internal_format;
  GLTexParameterfMap m_tex_parameter_f;
  GLTexParameteriMap m_tex_parameter_i;
};

} // end of namespace GL
} // end of namespace Leap
