#include "Leap/GL/Shader.h"

// #include <iostream> // TEMP

namespace Leap {
namespace GL {

// ////////////////////////////////////////////////////////////////////////////////////////////////
// Shader::VarInfo
// ////////////////////////////////////////////////////////////////////////////////////////////////

Shader::VarInfo::VarInfo (const std::string &name, GLint location, GLint size, GLenum type)
  :
  m_name(name),
  m_location(location),
  m_size(size),
  m_type(type)
{
  if (m_name.empty()) {
    throw ShaderException("shader variable must have nonempty name");
  }
  // I've commented this out because sometimes I get gl_ModelViewProjectionMatrixTranspose showing up at location -1.
//   if (m_location < 0) {
//     throw ShaderException("shader variable \"" + name + "\" must have nonnegative location (index in variable list)");
//   }
  if (m_size <= 0) {
    throw ShaderException("shader variable must have positive size");
  }
  // There is probably no way to reasonably check the validity of type that isn't
  // specifying a hardcoded list of acceptable values.
}

// ////////////////////////////////////////////////////////////////////////////////////////////////
// Shader
// ////////////////////////////////////////////////////////////////////////////////////////////////

Shader::Shader ()
  : m_program_handle(0)
  , m_vertex_shader(0)
  , m_fragment_shader(0)
{ }

Shader::Shader (const std::string &vertex_shader_source, const std::string &fragment_shader_source)
  : m_program_handle(0)
  , m_vertex_shader(0)
  , m_fragment_shader(0)
{
  Initialize(vertex_shader_source, fragment_shader_source);
}

Shader::~Shader () {
  Shutdown();
}

const std::string &Shader::VariableTypeString (GLenum type) {
  auto it = OPENGL_3_3_UNIFORM_TYPE_MAP.find(type);
  if (it == OPENGL_3_3_UNIFORM_TYPE_MAP.end()) {
    throw ShaderException("specified type is not a valid uniform type in OpenGL 3.3");
  }
  return it->second;
}

GLuint Shader::Compile (GLuint type, const std::string &source) {
  GLuint shader = glCreateShader(type);
  const GLchar *source_ptr = source.c_str();
  glShaderSource(shader, 1, &source_ptr, NULL);
  glCompileShader(shader);
  GLint compiled;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (!compiled) {
    GLint length;
    // This length includes the null terminating character.
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    std::string log(length, ' ');
    // Extract the log, which should contain compile/link error messages.
    glGetShaderInfoLog(shader, length, &length, &log[0]);
    // Resize the log based on the actual length.
    log.resize(length);
     std::cerr << "GL shader compile/link error: " << log << '\n';
    throw ShaderException(log);
  }
  return shader;
}

const std::unordered_map<GLenum,std::string> Shader::OPENGL_2_1_UNIFORM_TYPE_MAP{
  { GL_FLOAT, "float" },
  { GL_FLOAT_VEC2, "vec2" },
  { GL_FLOAT_VEC3, "vec3" },
  { GL_FLOAT_VEC4, "vec4" },
  { GL_INT, "int" },
  { GL_INT_VEC2, "ivec2" },
  { GL_INT_VEC3, "ivec3" },
  { GL_INT_VEC4, "ivec4" },
  { GL_BOOL, "bool" },
  { GL_BOOL_VEC2, "bvec2" },
  { GL_BOOL_VEC3, "bvec3" },
  { GL_BOOL_VEC4, "bvec4" },
  { GL_FLOAT_MAT2, "mat2" },
  { GL_FLOAT_MAT3, "mat3" },
  { GL_FLOAT_MAT4, "mat4" },
  { GL_FLOAT_MAT2x3, "mat2x3" },
  { GL_FLOAT_MAT2x4, "mat2x4" },
  { GL_FLOAT_MAT3x2, "mat3x2" },
  { GL_FLOAT_MAT3x4, "mat3x4" },
  { GL_FLOAT_MAT4x2, "mat4x2" },
  { GL_FLOAT_MAT4x3, "mat4x3" },
  { GL_SAMPLER_1D, "sampler1D" },
  { GL_SAMPLER_2D, "sampler2D" },
  { GL_SAMPLER_3D, "sampler3D" },
  { GL_SAMPLER_CUBE, "samplerCube" },
  { GL_SAMPLER_1D_SHADOW, "sampler1DShadow" },
  { GL_SAMPLER_2D_SHADOW, "sampler2DShadow" },
};

const std::unordered_map<GLenum,std::string> Shader::OPENGL_3_3_UNIFORM_TYPE_MAP{
  { GL_FLOAT, "float" },
  { GL_FLOAT_VEC2, "vec2" },
  { GL_FLOAT_VEC3, "vec3" },
  { GL_FLOAT_VEC4, "vec4" },
  { GL_INT, "int" },
  { GL_INT_VEC2, "ivec2" },
  { GL_INT_VEC3, "ivec3" },
  { GL_INT_VEC4, "ivec4" },
  { GL_UNSIGNED_INT, "uint" },
  { GL_UNSIGNED_INT_VEC2, "uvec2" },
  { GL_UNSIGNED_INT_VEC3, "uvec3" },
  { GL_UNSIGNED_INT_VEC4, "uvec4" },
  { GL_BOOL, "bool" },
  { GL_BOOL_VEC2, "bvec2" },
  { GL_BOOL_VEC3, "bvec3" },
  { GL_BOOL_VEC4, "bvec4" },
  { GL_FLOAT_MAT2, "mat2" },
  { GL_FLOAT_MAT3, "mat3" },
  { GL_FLOAT_MAT4, "mat4" },
  { GL_FLOAT_MAT2x3, "mat2x3" },
  { GL_FLOAT_MAT2x4, "mat2x4" },
  { GL_FLOAT_MAT3x2, "mat3x2" },
  { GL_FLOAT_MAT3x4, "mat3x4" },
  { GL_FLOAT_MAT4x2, "mat4x2" },
  { GL_FLOAT_MAT4x3, "mat4x3" },
  { GL_SAMPLER_1D, "sampler1D" },
  { GL_SAMPLER_2D, "sampler2D" },
  { GL_SAMPLER_3D, "sampler3D" },
  { GL_SAMPLER_CUBE, "samplerCube" },
  { GL_SAMPLER_1D_SHADOW, "sampler1DShadow" },
  { GL_SAMPLER_2D_SHADOW, "sampler2DShadow" },
  { GL_SAMPLER_1D_ARRAY, "sampler1DArray" },
  { GL_SAMPLER_2D_ARRAY, "sampler2DArray" },
  { GL_SAMPLER_1D_ARRAY_SHADOW, "sampler1DArrayShadow" },
  { GL_SAMPLER_2D_ARRAY_SHADOW, "sampler2DArrayShadow" },
  { GL_SAMPLER_2D_MULTISAMPLE, "sampler2DMS" },
  { GL_SAMPLER_2D_MULTISAMPLE_ARRAY, "sampler2DMSArray" },
  { GL_SAMPLER_CUBE_SHADOW, "samplerCubeShadow" },
  { GL_SAMPLER_BUFFER, "samplerBuffer" },
  { GL_SAMPLER_2D_RECT, "sampler2DRect" },
  { GL_SAMPLER_2D_RECT_SHADOW, "sampler2DRectShadow" },
  { GL_INT_SAMPLER_1D, "isampler1D" },
  { GL_INT_SAMPLER_2D, "isampler2D" },
  { GL_INT_SAMPLER_3D, "isampler3D" },
  { GL_INT_SAMPLER_CUBE, "isamplerCube" },
  { GL_INT_SAMPLER_1D_ARRAY, "isampler1DArray" },
  { GL_INT_SAMPLER_2D_ARRAY, "isampler2DArray" },
  { GL_INT_SAMPLER_2D_MULTISAMPLE, "isampler2DMS" },
  { GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY, "isampler2DMSArray" },
  { GL_INT_SAMPLER_BUFFER, "isamplerBuffer" },
  { GL_INT_SAMPLER_2D_RECT, "isampler2DRect" },
  { GL_UNSIGNED_INT_SAMPLER_1D, "usampler1D" },
  { GL_UNSIGNED_INT_SAMPLER_2D, "usampler2D" },
  { GL_UNSIGNED_INT_SAMPLER_3D, "usampler3D" },
  { GL_UNSIGNED_INT_SAMPLER_CUBE, "usamplerCube" },
  { GL_UNSIGNED_INT_SAMPLER_1D_ARRAY, "usampler2DArray" },
  { GL_UNSIGNED_INT_SAMPLER_2D_ARRAY, "usampler2DArray" },
  { GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE, "usampler2DMS" },
  { GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY, "usampler2DMSArray" },
  { GL_UNSIGNED_INT_SAMPLER_BUFFER, "usamplerBuffer" },
  { GL_UNSIGNED_INT_SAMPLER_2D_RECT, "usampler2DRect" },
};

void Shader::Initialize_Implementation (const std::string &vertex_shader_source, const std::string &fragment_shader_source) {
  m_vertex_shader = Compile(GL_VERTEX_SHADER, vertex_shader_source);
  m_fragment_shader = Compile(GL_FRAGMENT_SHADER, fragment_shader_source);
  m_program_handle = glCreateProgram();
  glAttachShader(m_program_handle, m_vertex_shader);
  glAttachShader(m_program_handle, m_fragment_shader);
  glLinkProgram(m_program_handle);

  // Populate the uniform map.
  {  
    GLint active_uniforms = 0;
    glGetProgramiv(m_program_handle, GL_ACTIVE_UNIFORMS, &active_uniforms);
    // std::cout << "active uniforms = " << active_uniforms << '\n';
    
    GLint active_uniform_max_length = 0;
    glGetProgramiv(m_program_handle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &active_uniform_max_length);
    // std::cout << "active uniform max length = " << active_uniform_max_length << '\n';
    
    for (GLint index = 0; index < active_uniforms; ++index) {
      std::string name(active_uniform_max_length, ' ');
      GLsizei length;
      GLint size;
      GLenum type;
      glGetActiveUniform(m_program_handle, index, active_uniform_max_length, &length, &size, &type, &name[0]);
      name.resize(length);
      // Chop off array subscript if present.
      if (*name.rbegin() == ']') {
        assert(name.find_last_of('[') != std::string::npos && "If ']' is the last char, then '[' better also be present.");
        name.resize(name.find_last_of('['));
      }
      GLint location = glGetUniformLocation(m_program_handle, name.c_str());
      // std::cout << "uniform " << index << " -- name \"" << name << "\", location = " << location << ", size = " << size << ", type = " << VariableTypeString(type) << '\n';
      if (location >= 0) {
        m_active_uniform_info_map.emplace(std::make_pair(name, VarInfo(name, location, size, type)));
      }
    }
  }

  // Populate the attribute map.
  {
    GLint active_attribs = 0;
    glGetProgramiv(m_program_handle, GL_ACTIVE_ATTRIBUTES, &active_attribs);
    // std::cout << "active attribs = " << active_attribs << '\n';
    
    GLint active_attrib_max_length = 0;
    glGetProgramiv(m_program_handle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &active_attrib_max_length);
    // std::cout << "active attrib max length = " << active_attrib_max_length << '\n';
    
    for (GLint index = 0; index < active_attribs; ++index) {
      std::string name(active_attrib_max_length, ' ');
      GLsizei length;
      GLint size;
      GLenum type;
      glGetActiveAttrib(m_program_handle, index, active_attrib_max_length, &length, &size, &type, &name[0]);
      name.resize(length);
      GLint location = glGetAttribLocation(m_program_handle, name.c_str());
      // std::cout << "attrib " << index << " -- name \"" << name << "\", location = " << location << ", size = " << size << ", type = " << VariableTypeString(type) << '\n';
      if (location >= 0) {
        m_active_attribute_info_map.emplace(std::make_pair(name, VarInfo(name, location, size, type)));
      }
    }
  }
}

void Shader::Shutdown_Implementation () {
  glDeleteProgram(m_program_handle);
  glDeleteShader(m_vertex_shader);
  glDeleteShader(m_fragment_shader);
  m_program_handle = 0;
  m_vertex_shader = 0;
  m_fragment_shader = 0;
}

} // end of namespace GL
} // end of namespace Leap
