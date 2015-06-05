#pragma once

#include <cassert>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "Leap/GL/Common.h"
#include "Leap/GL/GLHeaders.h" // convenience header for cross-platform GL includes
#include "Leap/GL/Error.h"
#include "Leap/GL/Internal/ShaderUniform.h"
#include "Leap/GL/Internal/UniformUploader.h"
#include "Leap/GL/ResourceBase.h"
#include "Leap/GL/ShaderException.h"

namespace Leap {
namespace GL {

enum class VariableIs { REQUIRED, OPTIONAL_NO_WARN, OPTIONAL_BUT_WARN };

/// @brief This class wraps compiling and binding GLSL shaders, as well as discovering and
/// setting their uniforms and attributes.
/// @details Some of the code was initially taken from Jerry Coffin's answer at
/// http://stackoverflow.com/questions/2795044/easy-framework-for-opengl-shaders-in-c-c
/// Currently only the OpenGL 2.1 standard uniform types are supported (in particular, this
/// is missing unsigned ints, unsigned int vectors, and a bunch of sampler types).
///
/// Upon successful linking, the shader program will be queried for all its active uniforms
/// and attributes, storing the relevant info (name, location, size, type) in a map which is
/// indexed by name.  These maps can be accessed via the ActiveUniformInfoMap and
/// ActiveAttributeInfoMap methods.
///
/// The only exceptions that this class explicitly throws derive from Leap::GL::ShaderException.
///
/// The UploadUniform and UploadUniformArray methods are frontends to the various glUniform*
/// functions, and each come in two varieties -- those taking a uniform name as a location
/// (in which case the uniform's integer-valued location will be looked up via
/// Shader::LocationOfUniform), and those taking integer-valued location (which were already looked
/// up via Shader::LocationOfUniform).  Each of the UploadUniform* methods takes a template
/// parameter indicating the OpenGL enum type name of the uniform being set (e.g. GL_FLOAT,
/// GL_BOOL_VEC3, GL_SAMPLER_2D, etc).
class Shader : public ResourceBase<Shader> {
public:

  /// @brief Stores information about a named variable in a shader program.
  class VarInfo {
  public:

    /// @brief Constructs a variable from its name, integer-valued location, [array] size, and type.
    /// @details The size parameter will be greater than 1 if the variable is an array.  The type
    /// parameter will be the GLenum type (e.g. GL_INT, GL_FLOAT_VEC2, GL_FLOAT_MAT2, etc) for the
    /// base type of the variable (if the type is an array, then the base type is the type of a
    /// single element).
    VarInfo (const std::string &name, GLint location, GLint size, GLenum type);

    /// @brief Returns the variable name.
    const std::string &Name () const { return m_name; }
    /// @brief Returns the integer-valued location of the variable.
    GLint Location () const { return m_location; }
    /// @brief Returns the array size of the variable.  Will return 1 for non-array types.
    GLint Size () const { return m_size; }
    // The Type defines what uniform modifier function can be used with each variable.
    // Note that sampler types must be set using integer values indicating which texture
    // unit is bound to it.  See http://www.opengl.org/wiki/Sampler_(GLSL)#Binding_textures_to_samplers
    GLenum Type () const { return m_type; }

  private:

    std::string m_name;
    GLint m_location;
    GLint m_size;
    GLenum m_type;
  };

  /// @brief Convenience typedef for a map of VarInfo objects indexed by variable name.
  typedef std::unordered_map<std::string,VarInfo> VarInfoMap;

  /// @brief Construct an un-Initialize-d Shader which has not acquired any GL (or other) resources.
  /// @details It will be necessary to call Initialize on this object to use it.
  Shader ();
  /// @brief Convenience constructor that will call Initialize with the given arguments.
  Shader (const std::string &vertex_shader_source, const std::string &fragment_shader_source);
  /// @brief Destructor will call Shutdown.
  ~Shader ();

  using ResourceBase<Shader>::IsInitialized;
  using ResourceBase<Shader>::Initialize;
  using ResourceBase<Shader>::Shutdown;

  /// @brief Returns the shader program handle, which is the integer "name" of this shader program in OpenGL.
  /// @details Will throw ShaderException if !IsInitialized().
  GLint ProgramHandle () const {
    if (!IsInitialized()) {
      throw ShaderException("A Shader that !IsInitialized() has no ProgramHandle value.");
    }
    return m_program_handle;
  }
  /// @brief This method should be called to bind this shader.
  /// @details Will throw ShaderException if !IsInitialized().
  void Bind () const {
    if (!IsInitialized()) {
      throw ShaderException("Can't Bind a Shader that !IsInitialized().");
    }
    THROW_UPON_GL_ERROR(glUseProgram(m_program_handle));
  }
  /// @brief This [static] method should be called when no shader program should be used.
  static void Unbind () {
    THROW_UPON_GL_ERROR(glUseProgram(0));
  }
  /// @brief Returns the currently bound shader program (the integer handle generated by OpenGL).
  /// @details This should only generate a GL error if it is called between glBegin and glEnd.
  static GLint CurrentlyBoundProgramHandle () {
    GLint current_program;
    glGetIntegerv(GL_CURRENT_PROGRAM, &current_program); // We know that GL_CURRENT_PROGRAM is a valid param.
    return current_program;
  }

  /// @brief Returns a map, indexed by name, containing all the active uniforms in this shader program.
  /// @details Note that the shader compiler may determine that a uniform does not contribute to the
  /// output of the program and may compile it out, in which case, that uniform is considered not active,
  /// and will not appear in this map.  Furthermore, array uniforms may have shortened apparent length
  /// due to this optimization.  Thus this info map isn't really appropriate for strict type checking
  /// because during development, shaders may be debugged by commenting out certain parts of the shader
  /// code, in which case certain uniforms may get compiled out, but that should not be interpreted as
  /// a fatal error.  This shader does not need to be bound for this call to succeed.
  const VarInfoMap &ActiveUniformInfoMap () const {
    if (!IsInitialized()) {
      throw ShaderException("A Shader that !IsInitialized() has no ActiveUniformInfoMap value.");
    }
    return m_active_uniform_info_map;
  }
  /// @brief Returns a map, indexed by name, containing all the active attributes in this shader program.
  /// @details The shader compiler may optimize out unused attributes in a way analogous to that described
  /// in @c ActiveUniformMap.  This shader does not need to be bound for this call to succeed.
  const VarInfoMap &ActiveAttributeInfoMap () const {
    if (!IsInitialized()) {
      throw ShaderException("A Shader that !IsInitialized() has no ActiveAttributeInfoMap value.");
    }
    return m_active_attribute_info_map;
  }

  /// @brief Returns the location of the requested uniform (its handle into the GL apparatus) or -1 if not found.
  /// @details The -1 return value is what is used by the glUniform* functions as a sentinel value for "this
  /// uniform is not found, so do nothing silently".  This shader does not need to be bound for this call to succeed.
  GLint LocationOfUniform (const std::string &name) const {
    if (!IsInitialized()) {
      throw ShaderException("Can't call LocationOfUniform on a Shader that !IsInitialized().");
    }
    return glGetUniformLocation(m_program_handle, name.c_str());
  }
  /// @brief Returns the location of the requested attribute (its handle into the GL apparatus) or -1 if not found.
  /// @details The -1 return value is what is used by the glUniform* functions as a sentinel value for "this
  /// uniform is not found, so do nothing silently".  This shader does not need to be bound for this call to succeed.
  GLint LocationOfAttribute (const std::string &name) const {
    if (!IsInitialized()) {
      throw ShaderException("Can't call LocationOfUniform on a Shader that !IsInitialized().");
    }
    return glGetAttribLocation(m_program_handle, name.c_str());
  }

  /// @brief Uploads a uniform of type GL_TYPE_ to given location, or does nothing if location is -1.
  /// @details This is essentially a strongly-typed C++ frontend for the various glUniform* functions.
  /// There are a few different ways to call this method.  Here are some examples:
  /// @verbatim
  /// shader.UploadUniform<GL_FLOAT>(loc0, 0.5f);                   // Calls glUniform1f(loc0, 0.5f).
  /// shader.UploadUniform<GL_FLOAT_VEC2>(loc1, 0.8f, 0.9f);        // Calls glUniform2f(loc1, 0.8f, 0.9f).
  /// shader.UploadUniform<GL_BOOL_VEC3>(loc2, true, false, true);  // Calls glUniform3i(loc2, GLint(true), GLint(false), GLint(true)).
  /// std::array<GLfloat,4> v{{0.1f, 0.2f, 0.3f, 0.4f}};
  /// shader.UploadUniform<GL_FLOAT_VEC4>(loc3, v);                 // Calls glUniform4fv(loc3, 1, reinterpret_cast<const GLfloat *>(&v)).
  /// shader.UploadUniform<GL_FLOAT_VEC4,std::array<GLfloat,4>>(loc3, {{0.1f, 0.2f, 0.3f, 0.4f}}); // Equivalent to previous.
  /// std::array<GLfloat,6> m{{1.0f, 2.0f, 3.0f,
  ///                          4.0f, 5.0f, 6.0f}};
  /// MatrixStorageConvention c = ...;
  /// shader.UploadUniform<GL_FLOAT_MAT2x3>(loc4, m, c);            // Calls glUniformMatrix2x3fv(loc4, 1, c == ROW_MAJOR, reintepret_cast<const GLfloat *>(&m)).
  /// @endverbatim
  /// Note that attempting to upload a vector type (e.g. GL_FLOAT_VEC3) to an array uniform of that base type (in this
  /// case GL_FLOAT) is an error.  Use @c UploadUniformArray instead.
  template <GLenum GL_TYPE_, typename... Types_>
  static void UploadUniform (GLint location, Types_... args) {
    Internal::UniformUploader<GL_TYPE_>::Upload(location, args...);
  }
  /// @brief Convenience frontend for the static version of UploadUniform which calls LocationOfUniform
  /// on the provided name in order to derive the integer-valued uniform location.
  template <GLenum GL_TYPE_, typename... Types_>
  void UploadUniform (const std::string &name, Types_... args) const {
    if (!IsInitialized()) {
      throw ShaderException("Can't call [the name version of] UploadUniform on a Shader that !IsInitialized().");
    }
    Internal::UniformUploader<GL_TYPE_>::Upload(LocationOfUniform(name), args...);
  }
  /// @brief Uploads an array of uniforms of base type GL_TYPE_ and array length ARRAY_LENGTH_ to given location,
  /// or does nothing if location is -1.
  /// @details This is essentially a strongly-typed C++ frontend for the various glUniform*v functions.
  /// There are a few different ways to call this method.  Here are some examples:
  /// @verbatim
  /// std::array<GLfloat,4> v{{0.1f, 0.2f, 0.3f, 0.4f}};
  /// shader.UploadUniformArray<GL_FLOAT,4>(loc0, v);               // Calls glUniform1fv(loc0, 4, reinterpret_cast<const GLfloat *>(&v)).
  /// shader.UploadUniformArray<GL_FLOAT_VEC2,2>(loc1, v);          // Calls glUniform2fv(loc1, 2, reinterpret_cast<const GLfloat *>(&v)).
  /// shader.UploadUniformArray<GL_FLOAT_VEC2,2,std::array<GLfloat,4>>(loc1, {{0.1f, 0.2f, 0.3f, 0.4f}}); // Equivalent to previous.
  /// std::array<GLfloat,8> m = ...;
  /// MatrixStorageConvention c = ...;
  /// shader.UploadUniform<GL_FLOAT_MAT2x2,2>(loc2, m, c);          // Calls glUniformMatrix2fv(loc2, 2, c == ROW_MAJOR, reintepret_cast<const GLfloat *>(&m)).
  /// @endverbatim
  template <GLenum GL_TYPE_, size_t ARRAY_LENGTH_, typename... Types_>
  static void UploadUniformArray (GLint location, Types_... args) {
    Internal::UniformUploader<GL_TYPE_>::template UploadArray<ARRAY_LENGTH_>(location, args...);
  }
  /// @brief Convenience frontend for the static version of UploadUniformArray which calls LocationOfUniform
  /// on the provided name in order to derive the integer-valued uniform location.
  template <GLenum GL_TYPE_, size_t ARRAY_LENGTH_, typename... Types_>
  void UploadUniformArray (const std::string &name, Types_... args) const {
    if (!IsInitialized()) {
      throw ShaderException("Can't call [the name version of] UploadUniformArray on a Shader that !IsInitialized().");
    }
    Internal::UniformUploader<GL_TYPE_>::template UploadArray<ARRAY_LENGTH_>(LocationOfUniform(name), args...);
  }

  // Returns (enum_name_string, type_name_string) for the given shader variable type.  Throws an
  // error if that type is not a shader variable type.
  /// @brief Returns the GLSL variable type identifier (e.g. float, vec2, bool, mat2, sampler2D, etc)
  /// corresponding to the GLenum valued variable type (respectively GL_FLOAT, GL_FLOAT_VEC2, GL_BOOL,
  /// GL_FLOAT_MAT2, GL_SAMPLER_2D, etc).
  static const std::string &VariableTypeString (GLenum type);
  
  /// @brief The uniform type map for OpenGL 2.1 (see glGetActiveAttrib and glGetActiveUniform in the
  /// OpenGL 2.1 API docs at https://www.opengl.org/sdk/docs/man2/ ).
  static const std::unordered_map<GLenum,std::string> OPENGL_2_1_UNIFORM_TYPE_MAP;
  /// @brief The uniform type map for OpenGL 3.3 (see glGetActiveAttrib and glGetActiveUniform in the
  /// OpenGL 3.3 API docs at https://www.opengl.org/sdk/docs/man3/ ).
  static const std::unordered_map<GLenum,std::string> OPENGL_3_3_UNIFORM_TYPE_MAP;

private:

  // Compiles the specified type of shader program, using the given source.  If an error
  // in encountered, a std::logic_error is thrown.
  static GLuint Compile (GLuint type, const std::string &source);

  friend class ResourceBase<Shader>;

  bool IsInitialized_Implementation () const { return m_program_handle != 0; }
  // Construct a shader with given vertex and fragment programs.
  void Initialize_Implementation (const std::string &vertex_shader_source, const std::string &fragment_shader_source);
  // Frees the allocated resources.
  void Shutdown_Implementation ();

  /// @brief Handle to the vertex shader in the GL apparatus.
  GLuint m_vertex_shader;
  /// @brief Handle to the fragment shader in the GL apparatus.
  GLuint m_fragment_shader;
  /// @brief Handle to the shader program in the GL apparatus.
  GLuint m_program_handle;

  VarInfoMap m_active_uniform_info_map;
  VarInfoMap m_active_attribute_info_map;
};

} // end of namespace GL
} // end of namespace Leap
