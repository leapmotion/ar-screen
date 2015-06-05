#include "stdafx.h"
#include "Leap/GL/ShaderMatrices.h"

namespace Leap {
namespace GL {

ShaderMatrices::ShaderMatrices (const Shader *shader,
                                const std::string &projection_times_model_view_matrix_id,
                                const std::string &model_view_matrix_id,
                                const std::string &normal_matrix_id)
  : m_frontend(shader, Frontend::UniformIds(projection_times_model_view_matrix_id, model_view_matrix_id, normal_matrix_id))
{ }

void ShaderMatrices::UploadUniforms (const EigenTypes::Matrix4x4 &model_view, const EigenTypes::Matrix4x4 &projection) {
  Frontend::UniformMap uniforms;
  // Derive the necessary matrices, stored using floats, which is what is required by OpenGL.
  uniforms.val<ShaderMatrix::PROJECTION_TIMES_MODEL_VIEW>() = (projection * model_view).cast<float>();
  uniforms.val<ShaderMatrix::MODEL_VIEW>() = model_view.cast<float>(); // same as model_view, but cast to float.
  // The inverse transpose is the correct transformation in order to keep normal EigenTypes::Vectors
  // actually perpendicular to "tangent" EigenTypes::Vectors.  If model_view is an isometry, then
  // the inverse transpose is itself.
  uniforms.val<ShaderMatrix::NORMAL>() = model_view.inverse().transpose().cast<float>();
  // Upload the uniforms
  m_frontend.UploadUniforms(uniforms);
}

} // end of namespace GL
} // end of namespace Leap
