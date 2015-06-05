#pragma once

#include <cassert>
#include "Leap/GL/Common.h"
#include "Leap/GL/GLHeaders.h"
#include "Leap/GL/Internal/Map.h"
#include "Leap/GL/Internal/ShaderFrontend.h"
#include "Leap/GL/Internal/UniformTraits.h"
#include "Leap/GL/ResourceBase.h"
#include "Leap/GL/Shader.h"
#include <sstream>

namespace Leap {
namespace GL {

/// @brief Provides a static C++ interface to a Shader.
/// @details Conceptually, a ShaderFrontend is a Shader and a set of named and typed uniforms for
/// which an interface to upload said uniforms will be provided.  On a more concrete level, a
/// ShaderFrontend is defined by several properties.
/// - A variadic list of uniform name/type info, provided as the template parameters to ShaderFrontend.
/// - A pointer to the Shader to which the uniforms are a part of.
/// - A tuple of uniform names, from which locations in the specified shader will be determined.
///
/// Once Initialize()d, a ShaderFrontend only has one relevant method, @c UploadUniforms, which takes
/// a UniformMap object and uploads the values specified in the corresponding values to the correpsonding
/// locations in the currently bound Shader (meaning that the relevant Shader must be bound for the
/// UploadUniforms operation to work).
///
/// The set of named and typed uniforms are specified via @c UniformSpecification, which has several
/// alias templates for convenience:
/// - @c Uniform
/// - @c UniformArray
/// - @c MatrixUniform
/// - @c MatrixUniformArray
/// The UniformNameType_ of ShaderFrontend should be an enum type which contains enums naming the
/// relevant uniforms.  The variadic UniformMappings_ parameter should be a list of UniformSpecification
/// types which define the name, GL type (GL_BOOL, GL_FLOAT_VEC3, etc), and C++ type (bool,
/// std::array<GLfloat,3>, etc) of each uniform.  The GL type will be used as the GL_TYPE_ parameter in
/// the call to Shader::UploadUniform<GL_TYPE_>(...) which is made in UploadUniforms.  The C++ type will
/// be used as the representation for that uniform value in UniformMap.  See the documentation for
/// UniformMap for more information.
template <typename UniformNameType_, typename... UniformMappings_>
class ShaderFrontend : public ResourceBase<ShaderFrontend<UniformNameType_,UniformMappings_...>> {
private:

  typedef Internal::Typle_t<UniformMappings_...> UniformMappingsTyple;
  static const size_t UNIFORM_COUNT = Internal::Length_f<UniformMappingsTyple>::V;
  typedef typename Internal::OnEach_f<UniformMappingsTyple,Internal::UniformNameOf_f>::T UniformNames;
  typedef typename Internal::OnEach_f<UniformMappingsTyple,Internal::GlTypeMappingOf_f>::T GlTypeMappings;
  typedef typename Internal::OnEach_f<UniformMappingsTyple,Internal::ArrayLengthMappingOf_f>::T ArrayLengthMappings;
  typedef typename Internal::OnEach_f<UniformMappingsTyple,Internal::CppTypeMappingOf_f>::T CppTypeMappings;
  typedef typename Internal::OnEach_f<UniformMappingsTyple,Internal::MatrixStorageConventionMappingOf_f>::T MatrixStorageConventionMappings;
  typedef Internal::TypeMap_t<GlTypeMappings> GlTypeMap;
  typedef Internal::TypeMap_t<ArrayLengthMappings> ArrayLengthMap;
  typedef Internal::TypeMap_t<CppTypeMappings> CppTypeMap;
  typedef Internal::TypeMap_t<MatrixStorageConventionMappings> MatrixStorageConventionMap;

  // template <UniformNameType_ NAME_> struct IndexOfUniform_f { static const size_t V = Internal::IndexIn_f<UniformNames,UniformName_t<NAME_>>::V; };

  typedef Internal::Map_t<Internal::TypeMap_t<CppTypeMappings>> UniformMapBaseClass;
  template <UniformNameType_ NAME_> using UniformName_t = Internal::Value_t<UniformNameType_,NAME_>;

public:

  template <UniformNameType_ NAME_> struct GlTypeOfUniform_f { static const GLenum V = Internal::Eval_f<GlTypeMap,UniformName_t<NAME_>>::T::V; };
  template <UniformNameType_ NAME_> struct ArrayLengthOfUniform_f { static const size_t V = Internal::Eval_f<ArrayLengthMap,UniformName_t<NAME_>>::T::V; };
  template <UniformNameType_ NAME_> struct CppTypeOfUniform_f { typedef typename Internal::Eval_f<CppTypeMap,UniformName_t<NAME_>>::T T; };
  template <UniformNameType_ NAME_> struct MatrixStorageConventionOfUniform_f { typedef typename Internal::Eval_f<MatrixStorageConventionMap,UniformName_t<NAME_>>::T T; };
  typedef Internal::Tuple_t<typename Internal::UniformTyple_f<std::string,Internal::Length_f<UniformMappingsTyple>::V>::T> UniformIds; // TODO: this should be Map_t
  typedef Internal::Tuple_t<typename Internal::UniformTyple_f<GLint,Internal::Length_f<UniformMappingsTyple>::V>::T> UniformLocations; // TODO: this should be Map_t

  /// @brief This is the type which stores the data required by UploadUniforms.
  /// @details A UniformMap is essentially a tuple whose components are indexed by particular
  /// enum values.  Access to these components is provided via the @c val method.  Examples:
  /// @verbatim
  /// enum class Material { COLOR, TEXTURING_ENABLED, TEXTURE_UNIT };
  /// typedef ShaderFrontend<Material,
  ///                        Uniform<Material, Material::COLOR, GL_FLOAT_VEC3,std::array<GLfloat,3>>,
  ///                        Uniform<Material, Material::TEXTURING_ENABLED, GL_BOOL, bool>,
  ///                        Uniform<Material, Material::TEXTURE_UNIT, GL_SAMPLER_2D, GLint>> Frontend;
  /// typedef Frontend::UniformMap Uniforms; // Layed out in memory the same as:
  ///                                        // struct {
  ///                                        //   std::array<GLfloat,3> color;
  ///                                        //   bool texturing_enabled;
  ///                                        //   GLint texture_unit;
  ///                                        // }
  /// Uniforms uniforms; // Leaves all components uninitialized.
  /// uniforms.val<COLOR>() = std::array<GLfloat,3>{{1.0f, 0.0f, 0.0f}}; // Red
  /// uniforms.val<TEXTURING_ENABLED>() = true;
  /// uniforms.val<TEXTURE_UNIT>() = 4;
  /// Frontend *frontend = ...;
  /// frontend->UploadUniforms(uniforms);
  /// @endverbatim
  /// A UniformMap can also be constructed with member initialization, as one would do with a tuple.
  /// For example,
  /// @verbatim
  /// Uniforms uniforms(std::array<GLfloat,3>{{1.0f, 0.0f, 0.0f}}, true, 4);
  /// @endverbatim
  class UniformMap : public UniformMapBaseClass {
  public:
    UniformMap (UniformMapBaseClass const &m) : UniformMapBaseClass(m) { }
    template <typename... Types_>
    UniformMap (Types_... args) : UniformMapBaseClass(args...) { }
    template <UniformNameType_ NAME_>
    typename UniformMapBaseClass::template val_const_ReturnType_f<UniformName_t<NAME_>>::T val () const {
      return UniformMapBaseClass::template val<UniformName_t<NAME_>>();
    }
    template <UniformNameType_ NAME_>
    typename UniformMapBaseClass::template val_ReturnType_f<UniformName_t<NAME_>>::T val () {
      return UniformMapBaseClass::template val<UniformName_t<NAME_>>();
    }
    using UniformMapBaseClass::values;
  };

  /// @brief Construct an un-Initialize-d ShaderFrontend which has not acquired any GL (or other) resources.
  /// @details It will be necessary to call Initialize on this object to use it.
  ShaderFrontend ()
    : m_shader(nullptr)
  { }
  /// @brief Convenience constructor that will call Initialize with the given arguments.
  template <typename... Types_>
  ShaderFrontend (const Shader *shader, const UniformIds &uniform_ids)
    : m_shader(nullptr)
  {
    Initialize(shader, uniform_ids);
  }
  /// @brief Destructor will call Shutdown.
  ~ShaderFrontend () {
    Shutdown();
  }

  using ResourceBase<ShaderFrontend<UniformNameType_,UniformMappings_...>>::IsInitialized;
  using ResourceBase<ShaderFrontend<UniformNameType_,UniformMappings_...>>::Initialize;
  using ResourceBase<ShaderFrontend<UniformNameType_,UniformMappings_...>>::Shutdown;

  /// @brief Uploads the values in the given UniformMap to the corresponding locations specified
  /// during the initialization of this ShaderFrontend.
  /// @details The associated Shader must be bound for this call to work.
  void UploadUniforms (const UniformMap &uniforms) const {
    if (!IsInitialized()) {
      throw ShaderException("Can't call UploadUniforms on a ShaderFrontend that !IsInitialized().");
    }
    assert(Shader::CurrentlyBoundProgramHandle() == m_shader->ProgramHandle() && "This shader must be bound in order to upload uniforms.");
    UploadUniform<0>(uniforms);
  }

private:

  template <size_t INDEX_>
  typename std::enable_if<(INDEX_<UNIFORM_COUNT)>::type CheckType (const UniformIds &uniform_ids) const {
    assert(m_shader != nullptr);
    typedef typename Internal::Element_f<UniformNames,INDEX_>::T UniformName;
    static const GLenum GL_TYPE_ = Internal::Eval_f<GlTypeMap,UniformName>::T::V;
    static const size_t ARRAY_LENGTH = Internal::Eval_f<ArrayLengthMap,UniformName>::T::V;
    if (m_uniform_locations.template el<INDEX_>() != -1) {
      const auto &uniform_id = uniform_ids.template el<INDEX_>();
      auto it = m_shader->ActiveUniformInfoMap().find(uniform_id);
      assert(it != m_shader->ActiveUniformInfoMap().end() && "This should never happen.");
      assert(Shader::OPENGL_3_3_UNIFORM_TYPE_MAP.find(GL_TYPE_) != Shader::OPENGL_3_3_UNIFORM_TYPE_MAP.end() && "Invalid uniform type.");
      const auto &info = it->second;
      if (GL_TYPE_ != info.Type()) {
        throw ShaderException("For uniform \"" + uniform_id + ", ShaderFrontend was looking for type " + Shader::OPENGL_3_3_UNIFORM_TYPE_MAP.at(GL_TYPE_) +
                              " but the actual type was " + Shader::OPENGL_3_3_UNIFORM_TYPE_MAP.at(info.Type()) + '.');
      }
      if (ARRAY_LENGTH != info.Size()) {
        std::ostringstream out;
        out << "For uniform \"" << uniform_id << ", ShaderFrontend was looking for array length " << ARRAY_LENGTH <<
               " but the actual array length was " << info.Size() << '.';
        throw ShaderException(out.str());
      }
    }
    // Iterate.
    CheckType<INDEX_+1>(uniform_ids);
  }
  template <size_t INDEX_>
  typename std::enable_if<(INDEX_>=UNIFORM_COUNT)>::type CheckType (const UniformIds &uniform_ids) const {
    // Done with the iteration.
  }

  template <size_t INDEX_>
  typename std::enable_if<(INDEX_<UNIFORM_COUNT)>::type UploadUniform (const UniformMap &uniforms) const {
    typedef typename Internal::Element_f<UniformNames,INDEX_>::T UniformName;
    static const GLenum GL_TYPE_ = Internal::Eval_f<GlTypeMap,UniformName>::T::V;
    static const size_t ARRAY_LENGTH = Internal::Eval_f<ArrayLengthMap,UniformName>::T::V;
    static const MatrixStorageConvention MATRIX_STORAGE_CONVENTION = Internal::Eval_f<MatrixStorageConventionMap,UniformName>::T::V;
    // Upload the uniform.
    Internal::UniformizedInterface_UploadArray<GL_TYPE_,ARRAY_LENGTH,MATRIX_STORAGE_CONVENTION>(m_uniform_locations.template el<INDEX_>(), uniforms.template val<UniformName::V>());
    // Iterate.
    UploadUniform<INDEX_+1>(uniforms);
  }
  template <size_t INDEX_>
  typename std::enable_if<(INDEX_>=UNIFORM_COUNT)>::type UploadUniform (const UniformMap &uniforms) const {
    // Done with iteration.
  }

  friend class ResourceBase<ShaderFrontend<UniformNameType_,UniformMappings_...>>;

  bool IsInitialized_Implementation () const { return m_shader != nullptr; }
  template <typename... Types_>
  void Initialize_Implementation (const Shader *shader, const UniformIds &uniform_ids) {
    if (shader == nullptr) {
      throw ShaderException("shader must be a non-null pointer.");
    }
    m_shader = shader;

    // Store the uniform locations from the shader using the uniform names.
    for (size_t i = 0; i < UNIFORM_COUNT; ++i) {
      m_uniform_locations.as_array()[i] = glGetUniformLocation(m_shader->ProgramHandle(), uniform_ids.as_array()[i].c_str());
    }
    // Compile-time checking of types.
    Leap::GL::Internal::CheckUniformTypes<UniformMappingsTyple>::Check();
    // Run-time checking of types.
    CheckType<0>(uniform_ids);
  }
  // Frees the allocated resources if IsInitialized(), otherwise does nothing (i.e. this method is
  // safe to call multiple times, and has no effect after the resources are freed).
  void Shutdown_Implementation () {
    m_shader = nullptr;
  }

  const Shader *m_shader;
  UniformLocations m_uniform_locations;
};

} // end of namespace GL
} // end of namespace Leap
