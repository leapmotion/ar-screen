#pragma once

//#include "GLShaderLoader.h"
#include "LambertianMaterial.h"
#include "Leap/GL/Shader.h"
#include "Leap/GL/ShaderMatrices.h"
#include "SceneGraphNode.h"
#include "SceneGraphNodeValues.h"
#include "ShaderBindingScopeGuard.h"
#include "RenderState.h"
//#include "Resource.h"

// This is the base class for drawable, geometric primitives.  It inherits SceneGraphNode<...>
// which provides the "scene graph" design pattern (see Wikipedia article on scene graph),
// as well as some convenience methods.  The template parameter DIM allows 2D or 3D (or even
// 1D) primitives to be defined.
template <int DIM>
class Primitive : public SceneGraphNode<ParticularSceneGraphNodeProperties<EigenTypes::MATH_TYPE,DIM,float>> {
public:

  static void DrawSceneGraph(const Primitive &root, RenderState &render_state) {
    // TODO: the existing model view matrix can be inputted as the initial state of global_properties
    // in the call to DepthFirstTraverse.
    root.template DepthFirstTraverse<Primitive>([&render_state](const Primitive &node, const Properties &global_properties) {
      node.Draw(render_state, global_properties);
    });
  }
  // Computes a "squash and stretch" volume-preserving shearing matrix based on a velocity vector
  // The speed denominator controls the shear strength such that a higher value gives less shear
  // When the magnitude of the velocity is equal to speedDenom, the object will be twice as long
  // See http://en.wikipedia.org/wiki/Squash_and_stretch and http://en.wikipedia.org/wiki/Shear_mapping
  static EigenTypes::Matrix3x3 SquashStretchTransform(const EigenTypes::Vector3& velocity, const EigenTypes::Vector3& viewDirection, double speedDenom = 900.0) {
    // compute velocity magnitude and direction
    const double speed = velocity.norm();
    static const double EPSILON = 0.0001;
    if (speedDenom < EPSILON || speed < EPSILON*speedDenom) {
      return EigenTypes::Matrix3x3::Identity();
    }
    const EigenTypes::Vector3 direction = velocity / speed;

    // compute stretch and squash multipliers (volume preserving)
    const double stretch = 1.0 + std::min(1.0, speed / speedDenom);
    const double squash = std::sqrt(1.0 / stretch);

    // compute velocity basis and its inverse for rotation
    EigenTypes::Matrix3x3 velocityBasis(EigenTypes::Matrix3x3::Identity());
    velocityBasis.col(0) = direction;
    velocityBasis.col(1) = direction.cross(viewDirection);
    velocityBasis.col(2) = viewDirection;
    const EigenTypes::Matrix3x3 velocityBasisInv = velocityBasis.inverse();

    // compute scale matrix for deformation
    const EigenTypes::Matrix3x3 scaleMatrix = EigenTypes::Vector3(stretch, squash, squash).asDiagonal();

    // undo rotation, deform, then rotate back
    return velocityBasis * scaleMatrix * velocityBasisInv;
  }

  typedef ParticularSceneGraphNodeProperties<EigenTypes::MATH_TYPE,DIM,float> Properties;
  typedef SceneGraphNode<ParticularSceneGraphNodeProperties<EigenTypes::MATH_TYPE,DIM,float>> Parent_SceneGraphNode;
  typedef typename Properties::AffineTransformValue_::Transform Transform;

  Primitive() {
    //SetShader(Resource<Leap::GL::Shader>("material"));
  }
  virtual ~Primitive() { }

  const Leap::GL::Shader &Shader () const {
    assert(m_shader);
    return *m_shader;
  }

  const LambertianMaterial &Material () const { return *m_material; }
  LambertianMaterial &Material () { return *m_material; }

  //Must be compatible with the default material (ie, use the same names for the matrix inputs)
  void SetShader(const std::shared_ptr<Leap::GL::Shader> &shader) {
    if (!shader) {
      throw std::runtime_error("Must specify a valid shader.");
    }
    std::shared_ptr<LambertianMaterial> material;
    auto uniform_ids = LambertianMaterial::UniformIds("light_position", "diffuse_light_color", "ambient_light_color", "ambient_lighting_proportion", "use_texture", "texture");
    if (m_material) {
      // Use existing material values for uniforms for re-initialized material.
      material = std::make_shared<LambertianMaterial>(shader.get(), uniform_ids, m_material->Uniforms());
    } else {
      // Use default values for uniforms.
      material =
        std::make_shared<LambertianMaterial>(
          shader.get(),
          uniform_ids,
          EigenTypes::Vector3f::Zero(),  // light_position
          Leap::GL::Rgba<float>::One(),  // diffuse_light_color -- Rgba<float>::One() is opaque white
          Leap::GL::Rgba<float>::One(),  // ambient_light_color -- Rgba<float>::One() is opaque white
          1.0f,                          // ambient_lighting_proportion
          GL_FALSE,                      // use_texture
          0);                            // texture
    }

    m_shader = shader;
    m_material = material;
    m_shader_matrices = std::make_shared<Leap::GL::ShaderMatrices>(m_shader.get());
  }

  typename Transform::ConstTranslationPart Translation () const { return this->LocalProperties().AffineTransform().translation(); }
  typename Transform::TranslationPart Translation () { return this->LocalProperties().AffineTransform().translation(); }
  typename Transform::ConstLinearPart LinearTransformation () const { return this->LocalProperties().AffineTransform().linear(); }
  typename Transform::LinearPart LinearTransformation () { return this->LocalProperties().AffineTransform().linear(); }

  void Draw(RenderState &render_state, const Properties &global_properties) const {
    assert(m_shader);
    assert(m_material);
    assert(m_shader_matrices);

    // Set the model view (TODO: change this to not be in the RenderState, since it's tracked by DepthFirstTraverse)
    Leap::GL::ModelView& model_view = render_state.GetModelView();
    // TODO: make a ScopeGuard for model view matrix.
    model_view.Push();
    model_view.Multiply(SquareMatrixAdaptToDim<4>(global_properties.AffineTransform().AsFullMatrix(), EigenTypes::MATH_TYPE(1)));
    MakeAdditionalModelViewTransformations(model_view);

    ShaderBindingScopeGuard bso(Shader(), BindFlags::BIND_AND_UNBIND); // binds shader now, unbinds upon end of scope.

    m_material->UploadUniforms();
    m_shader_matrices->UploadUniforms(model_view.Matrix(), render_state.ProjectionMatrix());

    DrawContents(render_state);

    model_view.Pop(); // TODO: once the ScopeGuard for the model view matrix is created, this goes away.
  }

  // This method should be overridden in any subclass that needs to do secondary
  // transformations (e.g. scaling based on a sphere's 'radius' member).
  virtual void MakeAdditionalModelViewTransformations (Leap::GL::ModelView &model_view) const { }

protected:

  // This method should be overridden in each subclass to draw the particular geometry that it represents.
  virtual void DrawContents(RenderState &render_state) const = 0;

  // Temporary hack to allow multiple model-matrix primitives (CapsulePrim, BiCapsulePrim)
  void ManuallySetMatricesAndUploadMatrixUniforms (const EigenTypes::Matrix4x4 &model_view, const EigenTypes::Matrix4x4 &projection) const {
    m_shader_matrices->UploadUniforms(model_view, projection);
  }

private:

  std::shared_ptr<Leap::GL::Shader> m_shader;
  std::shared_ptr<LambertianMaterial> m_material;
  std::shared_ptr<Leap::GL::ShaderMatrices> m_shader_matrices;
};

typedef Primitive<3> Primitive3;
// TODO: Deprecate the identifier "PrimitiveBase" and use Primitive3 instead.
typedef Primitive3 PrimitiveBase;
