#pragma once

#include "EigenTypes.h"
#include "Leap/GL/ShaderFrontend.h"

namespace Leap {
namespace GL {

class Shader;

/// @brief Presents an interface specifically tailored to shaders having particular, common matrix uniforms.
/// @details The matrix uniforms are
/// uniform mat4 projection_times_model_view_matrix
/// uniform mat4 model_view_matrix
/// uniform mat4 normal_matrix
/// These quantities are all derived from the model view matrix and the projection matrix, which is done
/// in a call to @c UploadUniforms.
class ShaderMatrices {
public:

  /// @brief Associates the given Shader and uses the given uniform names for the corresponding
  /// matrix uniforms.
  /// @details Leaving any of the matrix names empty will cause the corresponding uniforms to go unused.
  ShaderMatrices (const Shader *shader,
                  const std::string &projection_times_model_view_matrix_id = "projection_times_model_view_matrix",
                  const std::string &model_view_matrix_id = "model_view_matrix",
                  const std::string &normal_matrix_id = "normal_matrix");

  /// @brief Given the model-view and projection matrices, calculates the three matrices discussed in
  /// @c ShaderMatrices and uploads them.
  /// @details The associated Shader must be bound for this call to succeed.
  void UploadUniforms (const EigenTypes::Matrix4x4 &model_view, const EigenTypes::Matrix4x4 &projection);

private:

  enum class ShaderMatrix {
    PROJECTION_TIMES_MODEL_VIEW,
    MODEL_VIEW,
    NORMAL
  };

  template <ShaderMatrix NAME_, GLenum GL_TYPE_, typename CppType_, MatrixStorageConvention MATRIX_STORAGE_CONVENTION_>
  using ShaderMatrixUniform = Leap::GL::MatrixUniform<ShaderMatrix,NAME_,GL_TYPE_,CppType_,MATRIX_STORAGE_CONVENTION_>;

  typedef ShaderFrontend<ShaderMatrix,
                         ShaderMatrixUniform<ShaderMatrix::PROJECTION_TIMES_MODEL_VIEW,GL_FLOAT_MAT4,EigenTypes::Matrix4x4f,COLUMN_MAJOR>,
                         ShaderMatrixUniform<ShaderMatrix::MODEL_VIEW,GL_FLOAT_MAT4,EigenTypes::Matrix4x4f,COLUMN_MAJOR>,
                         ShaderMatrixUniform<ShaderMatrix::NORMAL,GL_FLOAT_MAT4,EigenTypes::Matrix4x4f,COLUMN_MAJOR>> Frontend;
  Frontend m_frontend;
};

} // end of namespace GL
} // end of namespace Leap
