#include "stdafx.h"
#include "Leap/GL/ModelView.h"

namespace Leap {
namespace GL {

ModelView::ModelView() {
  m_stack.push_back(EigenTypes::Matrix4x4::Identity());
}

const EigenTypes::Matrix4x4& ModelView::Matrix() const {
  return m_stack.back();
}

EigenTypes::Matrix4x4& ModelView::Matrix() {
  return m_stack.back();
}

void ModelView::LoadIdentity() {
  m_stack.back().setIdentity();
}

void ModelView::LookAt(const EigenTypes::Vector3& eye, const EigenTypes::Vector3& center, const EigenTypes::Vector3& up) {
  EigenTypes::Matrix4x4& mat = m_stack.back();
  LookAt(mat, eye, center, up);
}

void ModelView::Translate(const EigenTypes::Vector3& translation) {
  // TODO: replace with some utility function
  EigenTypes::Matrix4x4 mat = EigenTypes::Matrix4x4::Identity();
  mat(0, 3) = translation[0];
  mat(1, 3) = translation[1];
  mat(2, 3) = translation[2];
  m_stack.back() *= mat;
}

void ModelView::Rotate(const EigenTypes::Vector3& axis, double angleRadians) {
  // TODO: replace with some utility function
  EigenTypes::Matrix4x4 mat;
  const double c = std::cos(angleRadians);
  const double s = std::sin(angleRadians);
  const double C = (1 - c);
  mat << axis[0] * axis[0] * C + c, axis[0] * axis[1] * C - axis[2] * s, axis[0] * axis[2] * C + axis[1] * s, 0,
         axis[1] * axis[0] * C + axis[2] * s, axis[1] * axis[1] * C + c, axis[1] * axis[2] * C - axis[0] * s, 0,
         axis[2] * axis[0] * C - axis[1] * s, axis[2] * axis[1] * C + axis[0] * s, axis[2] * axis[2] * C + c, 0,
                                           0,                                   0,                         0, 1;
  m_stack.back() *= mat;
}

void ModelView::Scale(const EigenTypes::Vector3& scale) {
  // TODO: replace with some utility function
  EigenTypes::Matrix4x4 mat = EigenTypes::Matrix4x4::Identity();
  mat(0, 0) = scale[0];
  mat(1, 1) = scale[1];
  mat(2, 2) = scale[2];
  m_stack.back() *= mat;
}

void ModelView::Multiply(const EigenTypes::Matrix4x4& transform) {
  m_stack.back() *= transform;
}

void ModelView::Multiply(const EigenTypes::Matrix3x3& linear) {
  EigenTypes::Matrix4x4 affine;
  affine.block<3,3>(0,0) = linear;
  affine.block<3,1>(0,3).setZero();
  affine.block<1,3>(3,0).setZero();
  affine(3,3) = EigenTypes::MATH_TYPE(1);
  Multiply(affine);
}

void ModelView::Push() {
  m_stack.push_back(m_stack.back());
}

void ModelView::Pop() {
  assert(m_stack.size() > 1);
  m_stack.pop_back();
}

void ModelView::Clear() {
  m_stack.clear();
  m_stack.push_back(EigenTypes::Matrix4x4::Identity());
}

namespace {

void ComputeViewFrameInWorldCoordinates (
  EigenTypes::Vector3 &x,
  EigenTypes::Vector3 &y,
  EigenTypes::Vector3 &z,
  const EigenTypes::Vector3 &eye_position,
  const EigenTypes::Vector3 &focus_position,
  const EigenTypes::Vector3 &up_direction)
{
  z = eye_position - focus_position;
  assert(z.squaredNorm() > std::numeric_limits<double>::epsilon());
  z.normalize();

  x = up_direction.cross(z);
  assert(x.squaredNorm() > std::numeric_limits<double>::epsilon());
  x.normalize();

  y = z.cross(x);
  y.normalize();
}

void VerifyAffineTransformation (const EigenTypes::Matrix4x4 &matrix) {
  assert(matrix(3,0) == 0.0 && "matrix must be an affine transformation.");
  assert(matrix(3,1) == 0.0 && "matrix must be an affine transformation.");
  assert(matrix(3,2) == 0.0 && "matrix must be an affine transformation.");
  assert(matrix(3,3) == 1.0 && "matrix must be an affine transformation.");
}

} // end of anonymous namespace

void ModelView::LookAt (
  EigenTypes::Matrix4x4 &model_view_matrix,
  const EigenTypes::Vector3 &eye_position,
  const EigenTypes::Vector3 &focus_position,
  const EigenTypes::Vector3 &up_direction)
{
  EigenTypes::Vector3 x, y, z;
  ComputeViewFrameInWorldCoordinates(x, y, z, eye_position, focus_position, up_direction);

  model_view_matrix.block<1,3>(0,0) = x.transpose();
  model_view_matrix.block<1,3>(1,0) = y.transpose();
  model_view_matrix.block<1,3>(2,0) = z.transpose();
  model_view_matrix.block<1,3>(3,0).setZero();
  model_view_matrix.block<3,1>(0,3) = model_view_matrix.block<3,3>(0,0) * -eye_position;
  model_view_matrix(3,3) = 1.0;

  VerifyAffineTransformation(model_view_matrix);
}

void ModelView::LookAt_Inverse (
  EigenTypes::Matrix4x4 &matrix,
  const EigenTypes::Vector3 &eye_position,
  const EigenTypes::Vector3 &focus_position,
  const EigenTypes::Vector3 &up_direction)
{
  EigenTypes::Vector3 x, y, z;
  ComputeViewFrameInWorldCoordinates(x, y, z, eye_position, focus_position, up_direction);

  matrix.block<3,1>(0,0) = x;
  matrix.block<3,1>(0,1) = y;
  matrix.block<3,1>(0,2) = z;
  matrix.block<3,1>(0,3) = eye_position;
  matrix.block<1,3>(3,0).setZero();
  matrix(3,3) = 1.0;

  VerifyAffineTransformation(matrix);
}


} // end of namespace GL
} // end of namespace Leap
