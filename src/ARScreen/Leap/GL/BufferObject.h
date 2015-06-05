#pragma once

#include "Leap/GL/GLHeaders.h"
#include "Leap/GL/ResourceBase.h"
#include <string>

namespace Leap {
namespace GL {

/// @brief A C++ abstraction for the buffer object concept (untyped storage of data) in OpenGL.
/// @details This is a low-level class that is used by other higher-level classes such as
/// VertexBufferObject.  This class uses ResourceBase to implement consistent resource acquisition
/// and release conventions.  See https://www.opengl.org/wiki/Buffer_Object for more info on
/// the general buffer object concept.
class BufferObject : public ResourceBase<BufferObject> {
public:

  /// @brief Construct an un-Initialize-d BufferObject which has not acquired any GL (or other) resources.
  /// @details It will be necessary to call Initialize on this object to use it.
  BufferObject ();
  /// @brief Convenience constructor that will call Initialize with the given arguments.
  BufferObject (GLenum type);
  /// @brief Destructor will call Shutdown.
  ~BufferObject ();

  using ResourceBase<BufferObject>::IsInitialized;
  using ResourceBase<BufferObject>::Initialize;
  using ResourceBase<BufferObject>::Shutdown;

  /// @brief Binds this buffer object for the type specified in Initialize.
  /// @details The work is done via glBindBuffer.  Will throw a Leap::GL::Exception if
  /// !IsInitialized or if there was an error in the glBindBuffer operation.
  void Bind () const;
  /// @brief Unbinds this buffer object for the type specified in Initialize.
  /// @details The work is done via glBindBuffer.  Will throw a Leap::GL::Exception if
  /// !IsInitialized or if there was an error in the glBindBuffer operation.
  void Unbind () const;

  /// @brief Specifies the data to be stored in this buffer.
  /// @details The work is done via glBufferData.  Will throw a Leap::GL::Exception if
  /// !IsInitialized or if there was an error in the glBufferData operation.
  void BufferData (const void* data, GLsizeiptr size, GLenum usage_pattern);
  /// @brief Specifies the data to be altered in this buffer.
  /// @details The work is done via glBufferSubData.  Will throw a Leap::GL::Exception if
  /// !IsInitialized or if there was an error in the glBufferSubData operation.
  void BufferSubData (const void* data, int count);
  /// @brief Returns the number of bytes of data stored by this buffer.
  GLsizeiptr Size () const { return m_SizeInBytes; }
  /// @brief Maps the contents of this buffer to memory to which a pointer is returned.
  /// @details The work is done via glMapBuffer.  The Bind and Unbind methods will be called
  /// before and after glMapBuffer, respectively.  Will throw a Leap::GL::Exception if
  /// !IsInitialized or if there was an error in the glMapBuffer operation.
  void* MapBuffer (GLenum access);
  /// @brief Ends a `MapBuffer` session.
  /// @details The work is done via glUnmapBuffer.  The Bind and Unbind methods will be called
  /// before and after glMapBuffer, respectively.  Will throw a Leap::GL::Exception if
  /// !IsInitialized or if there was an error in the glUnmapBuffer operation.
  bool UnmapBuffer ();

private:

  friend class ResourceBase<BufferObject>;

  /// @brief Returns true iff this object has been Initialize-d.
  bool IsInitialized_Implementation () const { return m_BufferAddress != 0; }
  /// @brief Initialize this buffer object 
  void Initialize_Implementation (GLenum type);
  /// @brief Releases any the GL resources acquired by this buffer object.
  void Shutdown_Implementation ();

  GLuint m_BufferAddress;
  GLenum m_BufferType;
  GLsizeiptr m_SizeInBytes;
};

} // end of namespace GL
} // end of namespace Leap
