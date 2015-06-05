#pragma once

#include "EigenTypes.h"
#include "Leap/GL/GLHeaders.h"
#include "Leap/GL/Rgba.h"
#include "Leap/GL/ShaderFrontend.h"

enum LambertianMaterialProperty {
  LIGHT_POSITION,
  DIFFUSE_LIGHT_COLOR,
  AMBIENT_LIGHT_COLOR,
  AMBIENT_LIGHTING_PROPORTION,
  TEXTURE_MAPPING_ENABLED,
  TEXTURE_UNIT_INDEX
};

template <LambertianMaterialProperty NAME_, GLenum GL_TYPE_, typename CppType_>
using LambertianMaterialUniform = Leap::GL::Uniform<LambertianMaterialProperty,NAME_,GL_TYPE_,CppType_>;

typedef Leap::GL::ShaderFrontend<LambertianMaterialProperty,
                                 LambertianMaterialUniform<LIGHT_POSITION,GL_FLOAT_VEC3,EigenTypes::Vector3f>,
                                 LambertianMaterialUniform<DIFFUSE_LIGHT_COLOR,GL_FLOAT_VEC4,Leap::GL::Rgba<float>>,
                                 LambertianMaterialUniform<AMBIENT_LIGHT_COLOR,GL_FLOAT_VEC4,Leap::GL::Rgba<float>>,
                                 LambertianMaterialUniform<AMBIENT_LIGHTING_PROPORTION,GL_FLOAT,float>,
                                 LambertianMaterialUniform<TEXTURE_MAPPING_ENABLED,GL_BOOL,GLint>,
                                 LambertianMaterialUniform<TEXTURE_UNIT_INDEX,GL_SAMPLER_2D,GLint>> LambertianMaterialBaseClass;

class LambertianMaterial : public LambertianMaterialBaseClass
{
public:

  LambertianMaterial () { }
  template <typename... Types_>
  LambertianMaterial (const Leap::GL::Shader *shader, const UniformIds &uniform_ids, Types_... args)
    : LambertianMaterialBaseClass(shader, uniform_ids)
    , m_uniforms(args...)
  { }

  template <LambertianMaterialProperty NAME_>
  const typename CppTypeOfUniform_f<NAME_>::T &Uniform () const {
    if (!IsInitialized()) {
      throw Leap::GL::ShaderException("A LambertianMaterial that !IsInitialized() has no Uniform<...> value.");
    }
    return m_uniforms.template val<NAME_>();
  }
  template <LambertianMaterialProperty NAME_>
  typename CppTypeOfUniform_f<NAME_>::T &Uniform () {
    if (!IsInitialized()) {
      throw Leap::GL::ShaderException("A LambertianMaterial that !IsInitialized() has no Uniform<...> value.");
    }
    return m_uniforms.template val<NAME_>();
  }
  const UniformMap &Uniforms () const {
    if (!IsInitialized()) {
      throw Leap::GL::ShaderException("A LambertianMaterial that !IsInitialized() has no Uniforms value.");
    }
    return m_uniforms;
  }
  UniformMap &Uniforms () {
    if (!IsInitialized()) {
      throw Leap::GL::ShaderException("A LambertianMaterial that !IsInitialized() has no Uniforms value.");
    }
    return m_uniforms;
  }

  void UploadUniforms () const {
    LambertianMaterialBaseClass::UploadUniforms(m_uniforms);
  }

private:

  LambertianMaterialBaseClass::UniformMap m_uniforms;
};
