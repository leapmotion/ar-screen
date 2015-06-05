#include "Leap/GL/Texture2Params.h"

namespace Leap {
namespace GL {

Texture2Params::Texture2Params (GLsizei width, GLsizei height, GLenum internal_format)
  :
  m_target(DEFAULT_TARGET),
  m_internal_format(internal_format)
{ 
  m_size[0] = width;
  m_size[1] = height;
}

GLfloat Texture2Params::TexParameterf (GLenum pname) const {
  // TODO: validate that pname is a valid argument for this function (see docs of glTexParameteri/f)
  GLTexParameterfMap::const_iterator it = m_tex_parameter_f.find(pname);
  if (it == m_tex_parameter_f.end()) {
    throw Texture2Exception("specified GLfloat-valued texture parameter not found and/or specified");
  }
  return it->second;
}

GLint Texture2Params::TexParameteri (GLenum pname) const {
  // TODO: validate that pname is a valid argument for this function (see docs of glTexParameteri/f)
  GLTexParameteriMap::const_iterator it = m_tex_parameter_i.find(pname);
  if (it == m_tex_parameter_i.end()) {
    throw Texture2Exception("specified GLint-valued texture parameter not found and/or specified");
  }
  return it->second;
}

void Texture2Params::SetTexParameterf (GLenum pname, GLfloat value) {
  // TODO: validate that pname is a valid argument for this function (see docs of glTexParameteri/f)
  m_tex_parameter_f[pname] = value;
}

void Texture2Params::SetTexParameteri (GLenum pname, GLint value) {
  // TODO: validate that pname is a valid argument for this function (see docs of glTexParameteri/f)
  m_tex_parameter_i[pname] = value;
}

void Texture2Params::Clear () {
  m_target = DEFAULT_TARGET;
  m_size[0] = 0;
  m_size[1] = 0;
  m_internal_format = DEFAULT_INTERNAL_FORMAT;
  m_tex_parameter_f.clear();
  m_tex_parameter_i.clear();
}

} // end of namespace GL
} // end of namespace Leap
