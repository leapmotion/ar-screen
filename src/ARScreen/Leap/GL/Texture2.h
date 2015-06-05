#pragma once

#include "Leap/GL/GLHeaders.h" // convenience header for cross-platform GL includes
#include "Leap/GL/ResourceBase.h"
#include "Leap/GL/Texture2Params.h"
#include "Leap/GL/Texture2PixelData.h"
#include "Leap/GL/Texture2Exception.h"

namespace Leap {
namespace GL {

/// @brief This class wraps creation and use of 2-dimensional GL textures.
/// @details There are two associated classes: @c Texture2Params and @c Texture2PixelData.
/// Texture2Params specifies the persistent properties of a Texture2.  Texture2PixelData
/// is a temporary container for pixel read/write operations.  The Texture2 class
/// uses the resource interface defined by @c ResourceBase.
///
/// The Texture2Params object is specified only during initialization (either via
/// @c Initialize or the corresponding constructor), and is stored almost unmodified --
/// the only value that is potentially modified is the internal format of the texture --
/// the actual internal format will be determined and stored upon initialization.
/// The @c Params method can be used to access the Texture2Params which reflect the
/// actual state of an initialized Texture2.
/// 
/// The only exceptions that this class explicitly throws derive from Leap::GL::Texture2Exception.
///
/// This is an invaluable resource: http://www.opengl.org/wiki/Common_Mistakes
class Texture2 : public ResourceBase<Texture2> {
public:
  
  /// @brief Disallow copy-construction.
  Texture2 (const Texture2 &rhs) = delete;

  // TODO: allow construction from an existing texture handle and Texture2Params object
  // (e.g. where a texture was generated from outside of LeapGL and we want to use it
  // via LeapGL classes).

  /// @brief Construct an un-Initialize-d Texture2 which has not acquired any GL (or other) resources.
  /// @details It will be necessary to call Initialize on this object to use it.
  Texture2 ();
  /// @brief Convenience constructor that will call Initialize with the given arguments.
  Texture2 (const Texture2Params &params, const Texture2PixelData &pixel_data = Texture2PixelData());
  /// @details Will call Shutdown.
  ~Texture2 ();

  using ResourceBase<Texture2>::IsInitialized;
  using ResourceBase<Texture2>::Initialize;
  using ResourceBase<Texture2>::Shutdown;

  /// @brief This method should be called to bind this texture.
  /// @details This will throw Texture2Exception if this texture !IsInitialized().
  void Bind(int textureUnit = 0) const {
    if (!IsInitialized()) {
      throw Texture2Exception("Can't Bind a Texture2 that !IsInitialized().");
    }
    m_TextureUnit = textureUnit;
    glActiveTexture(GL_TEXTURE0 + m_TextureUnit);
    glBindTexture(m_params.Target(), m_texture_name);
  }
  /// @brief This method should be called when no texture should be used (for the active texture unit).
  /// @details This will throw Texture2Exception if this texture !IsInitialized().
  void Unbind () const {
    if (!IsInitialized()) {
      throw Texture2Exception("Can't Unbind a Texture2 that !IsInitialized().");
    }
    glActiveTexture(GL_TEXTURE0 + m_TextureUnit);
    glBindTexture(m_params.Target(), 0);
  }

  /// @brief Returns the texture name (GLuint assigned by OpenGL upon generation).
  /// @details This will throw Texture2Exception if this texture !IsInitialized().
  GLuint Id () {
    if (!IsInitialized()) {
      throw Texture2Exception("A Texture2 that !IsInitialized() has no Id value.");
    }
    return m_texture_name;
  }
  /// @brief Returns the Texture2Params which reflect the actual initialized state.
  /// @details This will throw Texture2Exception if this texture !IsInitialized().
  const Texture2Params &Params() const {
    if (!IsInitialized()) {
      throw Texture2Exception("A Texture2 that !IsInitialized() has no Params value.");
    }
    return m_params;
  }

  /// @brief Updates the contents of this texture from the specified pixel data, without changing Params.
  /// @details This method is the abstraction of glTexSubImage2D (and in fact calls it).
  void TexSubImage (const Texture2PixelData &pixel_data);
  /// @brief Extracts the contents of this texture to the specified pixel data.
  /// @details This method is the abstraction of glGetTexImage2D (and in fact calls it).
  void GetTexImage (Texture2PixelData &pixel_data);
  
private:

  void VerifyPixelDataOrThrow (const Texture2PixelData &pixel_data) const;

  friend class ResourceBase<Texture2>;

  bool IsInitialized_Implementation () const { return m_texture_name != 0; }
  // Construct a Texture2 with the specified parameters and pixel data. The pixel data is only
  // passed into glTexImage2D, and is not stored.  The default value for pixel_data is "empty",
  // which indicates that while the texture memory will be allocated for it, it will not be
  // initialized.  An exception will be thrown upon error.
  void Initialize_Implementation (const Texture2Params &params, const Texture2PixelData &pixel_data = Texture2PixelData());
  // Frees the allocated resources if IsInitialized(), otherwise does nothing (i.e. this method is
  // safe to call multiple times, and has no effect after the resources are freed).
  void Shutdown_Implementation ();

  Texture2Params m_params;
  GLuint m_texture_name;
  mutable int m_TextureUnit;
};

} // end of namespace GL
} // end of namespace Leap
