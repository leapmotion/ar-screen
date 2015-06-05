#pragma once

// #include <algorithm>
#include <Eigen/Geometry>
#include "EigenTypes.h"
#include <ostream>
#include "SceneGraphNodeProperties.h"
#include <sstream>
#include <string>

// A Transform is a translation and a linear transformation, in the block matrix form
//   [ L T ]
//   [ 0 1 ]
// where L is a DIMxDIM matrix representing the linear transformation and T is a DIMx1 matrix
// (column EigenTypes::Vector) representing the translation.  The semantic is that this (DIM+1)x(DIM+1)
// matrix acts on
//   [ X ]
//   [ 1 ]
// where X is a DIMx1 matrix (the EigenTypes::Vector being acted upon).  This is equivalent to the expression
//   L*X + T.
// The DIMx(DIM+1) matrix [ L T ] is called the affine transformation, and if X' is the (DIM+1)x1
// column matrix written above (the one with the 1 at the bottom), then the transformation acts
// simply as
//   A*X'
// and produces the same DIMx1 column matrix value as the expression L*X + T.
template <typename Scalar, int DIM>
class AffineTransformValue
  :
  public Eigen::Transform<Scalar,DIM,Eigen::AffineCompact,Eigen::DontAlign>
{
public:

  typedef Eigen::Transform<Scalar,DIM,Eigen::AffineCompact,Eigen::DontAlign> Transform;

  // Default construction initializes to the identity.
  AffineTransformValue () { SetIdentity(); }

  using Transform::operator=;

  // This returns the block matrix
  //   [ L T ]
  //   [ 0 1 ]
  Eigen::Matrix<Scalar,DIM+1,DIM+1,Eigen::DontAlign> AsFullMatrix () const {
    Eigen::Matrix<Scalar,DIM+1,DIM+1,Eigen::DontAlign> full_matrix;
    full_matrix.template block<DIM,DIM+1>(0,0) = this->affine();
    full_matrix.template block<1,DIM>(DIM,0).setZero();
    full_matrix(DIM,DIM) = Scalar(1);
    return full_matrix;
  }

  bool operator == (const AffineTransformValue &other) const {
    return this->affine() == other.affine();
  }
  // Resets this transform to the identity value.
  void SetIdentity () { this->setIdentity(); }
  // Performs the value-specific operation, either on the left or the right.
  void Operate (const AffineTransformValue &operand, ::Operate operate) {
    switch (operate) {
    case ::Operate::ON_LEFT:  *this = operand * *this; break;
    case ::Operate::ON_RIGHT: *this = *this * operand; break;
    }
  }
  // This will invert the transform, returning true iff the inversion succeeded.
  // If false is returned, the value of this transformation is undefined and should
  // not be used.
  bool Invert () {
    // This check is somewhat arbitrary, and use of the determinant relies
    // heavily on the fact that only small values of DIM will be used.
    // In fact, the following static_assert is used to ensure that.
    static_assert(DIM < 4, "If you want a larger DIM, we will need to implement a better solution here");
    bool is_invertible = std::abs(this->linear().determinant()) > std::numeric_limits<Scalar>::epsilon();
    if (is_invertible) {
      *this = this->inverse(Eigen::Affine);
    }
    return is_invertible;
  }
  std::string TypeAsString () const { return "AffineTransformValue"; }
  // This will produce a stringified representation of the transform in the block format
  //   [ L T ]
  //   [ 0 1 ]
  std::string AsString () const {
    auto full_matrix = AsFullMatrix();
    std::ostringstream out;
    out << full_matrix;
    return out.str();
  }
};

// TODO: allow scalar types with different dynamic ranges, e.g. uint8_t [0,255] or int16_t [-32768,32767], etc?
template <typename Scalar>
class AlphaMaskValue {
public:

  // Default construction initializes to the identity.
  AlphaMaskValue () { SetIdentity(); }

  operator Scalar () const { return m_alpha_mask; }
  // This will clamp the value to within [0,1].
  void operator = (const Scalar &alpha_mask) {
    m_alpha_mask = std::min(std::max(alpha_mask, Scalar(0)), Scalar(1));
  }

  bool operator == (const AlphaMaskValue &other) const {
    return m_alpha_mask == other.m_alpha_mask;
  }
  // Resets this alpha mask to the identity value (1).
  void SetIdentity () { m_alpha_mask = Scalar(1); }
  // Performs the value-specific operation, either on the left or the right.
  void Operate (const AlphaMaskValue &operand, ::Operate operate) {
    // Because multiplication is commutative, the left/right choice is irrelevant.
    m_alpha_mask *= operand.m_alpha_mask;
  }
  // Alpha masking can't be inverted.
  bool Invert () { return false; }
  std::string TypeAsString () const { return "AlphaMaskValue"; }
  std::string AsString () const {
    std::ostringstream out;
    out << m_alpha_mask;
    return out.str();
  }

private:

  Scalar m_alpha_mask;
};

class NameValue {
public:

  // Default construction initializes to the identity (empty string).
  NameValue () { SetIdentity(); }

  operator const std::string & () const { return m_name; }
  void operator = (const std::string &name) {
    m_name = name;
  }

  bool operator == (const NameValue &other) const {
    return m_name == other.m_name;
  }
  // Resets this name to the identity value (empty string).
  void SetIdentity () { m_name.clear(); }
  // Performs the value-specific operation, either on the left or the right.
  void Operate (const NameValue &operand, ::Operate operate) {
    std::string separator(m_name.empty() || operand.m_name.empty() ? "" : "/");
    switch (operate) {
    case ::Operate::ON_LEFT:  m_name = operand.m_name + separator + m_name; break;
    case ::Operate::ON_RIGHT: m_name += separator + operand.m_name;         break;
    }
  }
  // TODO: ".." based inversion.
  bool Invert () { return false; }
  std::string TypeAsString () const { return "NameValue"; }
  std::string AsString () const {
    std::ostringstream out;
    out << m_name;
    return out.str();
  }

private:

  std::string m_name;
};









// This is a particular set of properties to be used in SceneGraphNode:
// - Affine transformation (for position/orientation/size/shearing)
// - Alpha masking (for transparency blending).
template <typename AffineTransformScalar, int AFFINE_TRANSFORM_DIM, typename AlphaMaskScalar>
class ParticularSceneGraphNodeProperties
  :
  public NodeProperties<ParticularSceneGraphNodeProperties<AffineTransformScalar,AFFINE_TRANSFORM_DIM,AlphaMaskScalar>,
                        NodeProperty<AffineTransformValue<AffineTransformScalar,AFFINE_TRANSFORM_DIM>>,
                        NodeProperty<AlphaMaskValue<AlphaMaskScalar>>>
{
public:

  typedef NodeProperty<AffineTransformValue<AffineTransformScalar,AFFINE_TRANSFORM_DIM>> AffineTransformProperty_;
  typedef NodeProperty<AlphaMaskValue<AlphaMaskScalar>> AlphaMaskProperty_;
  // typedef NodeProperty<NameValue> NameProperty_;

  typedef AffineTransformValue<AffineTransformScalar,AFFINE_TRANSFORM_DIM> AffineTransformValue_;
  typedef AlphaMaskValue<AlphaMaskScalar> AlphaMaskValue_;
  // typedef NameValue NameValue_;

  // Named accessors

  const AffineTransformProperty_ &AffineTransformProperty () const { return this->Head(); }
  AffineTransformProperty_ &AffineTransformProperty () { return this->Head(); }
  const AlphaMaskProperty_ &AlphaMaskProperty () const { return this->Body().Head(); }
  AlphaMaskProperty_ &AlphaMaskProperty () { return this->Body().Head(); }
  // const NameProperty_ &NameProperty () const { return this->Body().Body().Head(); }
  // NameProperty_ &NameProperty () { return this->Body().Body().Head(); }

  const AffineTransformValue_ &AffineTransform () const { return this->Head().Value(); }
  AffineTransformValue_ &AffineTransform () { return this->Head().Value(); }
  const AlphaMaskValue_ &AlphaMask () const { return this->Body().Head().Value(); }
  AlphaMaskValue_ &AlphaMask () { return this->Body().Head().Value(); }
  // const NameValue_ &Name () const { return this->Body().Body().Head().Value(); }
  // NameValue_ &Name () { return this->Body().Body().Head().Value(); }
};
