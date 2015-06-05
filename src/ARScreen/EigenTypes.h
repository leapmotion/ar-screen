#pragma once

#include <Eigen/Dense>
#include <Eigen/StdVector> // This is not nessecary in c++11 according to stack overflow.

#include <map>
#include <set>
#include <type_traits>

namespace Eigen {
  // standard library containers
  template<typename _T>
  using vector = std::vector<_T, Eigen::aligned_allocator<_T>>;

  template<typename _T, typename _C = std::less<_T>>
  using set = std::set<_T, _C, Eigen::aligned_allocator<_T>>;

  template<typename _K, typename _V, typename _C = std::less<_K>>
  using map = std::map<_K, _V, _C, Eigen::aligned_allocator<std::pair<_K, _V>>>;
} // end of namespace Eigen

namespace EigenTypes {

// NOTE: The use of Eigen::DontAlign removes the guarantee that the statically-sized
// Eigen matrix/vector types will be aligned to 16-byte memory boundaries and therefore
// the corresponding matrix/vector operations aren't guaranteed to be vectorized.
// See http://eigen.tuxfamily.org/dox/group__TopicUnalignedArrayAssert.html and
// http://eigen.tuxfamily.org/dox/group__TopicStructHavingEigenMembers.html for more info.

// geometry storage types
typedef double MATH_TYPE;
// matrices
typedef Eigen::Matrix<MATH_TYPE, 1, 1, Eigen::DontAlign> Matrix1x1;
typedef Eigen::Matrix<MATH_TYPE, 2, 2, Eigen::DontAlign> Matrix2x2;
typedef Eigen::Matrix<MATH_TYPE, 2, 3, Eigen::DontAlign> Matrix2x3;
typedef Eigen::Matrix<MATH_TYPE, 3, 3, Eigen::DontAlign> Matrix3x3;
typedef Eigen::Matrix<MATH_TYPE, 3, 2, Eigen::DontAlign> Matrix3x2;
typedef Eigen::Matrix<MATH_TYPE, 4, 4, Eigen::DontAlign> Matrix4x4;
typedef Eigen::Matrix<MATH_TYPE, Eigen::Dynamic, Eigen::Dynamic> MatrixD;
typedef Eigen::Matrix<float, 2, 2, Eigen::DontAlign> Matrix2x2f;
typedef Eigen::Matrix<float, 3, 3, Eigen::DontAlign> Matrix3x3f;
typedef Eigen::Matrix<float, 4, 4, Eigen::DontAlign> Matrix4x4f;

// vectors
typedef Eigen::Matrix<MATH_TYPE, 1, 1, Eigen::DontAlign> Vector1;
typedef Eigen::Matrix<MATH_TYPE, 2, 1, Eigen::DontAlign> Vector2;
typedef Eigen::Matrix<MATH_TYPE, 3, 1, Eigen::DontAlign> Vector3;
typedef Eigen::Matrix<MATH_TYPE, 4, 1, Eigen::DontAlign> Vector4;
typedef Eigen::Matrix<MATH_TYPE, 5, 1, Eigen::DontAlign> Vector5;
typedef Eigen::Matrix<MATH_TYPE, 6, 1, Eigen::DontAlign> Vector6;
typedef Eigen::Matrix<MATH_TYPE, 7, 1, Eigen::DontAlign> Vector7;
typedef Eigen::Matrix<MATH_TYPE, 8, 1, Eigen::DontAlign> Vector8;
typedef Eigen::Matrix<MATH_TYPE, 9, 1, Eigen::DontAlign> Vector9;
typedef Eigen::Matrix<MATH_TYPE, 10, 1, Eigen::DontAlign> Vector10;
typedef Eigen::Matrix<MATH_TYPE, Eigen::Dynamic, 1> VectorD;
typedef Eigen::Matrix<float, 2, 1, Eigen::DontAlign> Vector2f;
typedef Eigen::Matrix<float, 3, 1, Eigen::DontAlign> Vector3f;
typedef Eigen::Matrix<float, 4, 1, Eigen::DontAlign> Vector4f;

typedef Eigen::vector<Vector2> stdvectorV2;
typedef Eigen::vector<Vector3> stdvectorV3;
typedef Eigen::vector<Vector2f> stdvectorV2f;
typedef Eigen::vector<Vector3f> stdvectorV3f;
typedef Eigen::vector<Vector4f> stdvectorV4f;

} // end of namespace EigenTypes

// This function projects or embeds a vector in a different dimensional vector space.
// Another way to phrase this is that it truncates or extends a vector as necessary.
// Let T denote TARGET_DIM and let S denote SOURCE_DIM.
// If T == S, then this returns the input exactly.
// If T < S, then (v_1, ..., v_T, v_{T+1}, ..., v_S) maps to (v_1, ..., v_T).
// If T > S, then (v_1, ..., V_T) maps to (v_1, ..., v_S, x, ..., x) (with T - S values x),
// where x is the value of extension_component (default is 0).
template <int TARGET_DIM, typename Scalar, int SOURCE_DIM, int OPTIONS>
typename std::enable_if<(TARGET_DIM<SOURCE_DIM),Eigen::Matrix<Scalar,TARGET_DIM,1,OPTIONS>>::type
  VectorAdaptToDim (const Eigen::Matrix<Scalar,SOURCE_DIM,1,OPTIONS> &v, Scalar extension_component)
{
  return v.template block<TARGET_DIM,1>(0,0);
}

// See comment for other version of this.
template <int TARGET_DIM, typename Scalar, int SOURCE_DIM, int OPTIONS>
typename std::enable_if<(TARGET_DIM==SOURCE_DIM),Eigen::Matrix<Scalar,TARGET_DIM,1,OPTIONS>>::type
  VectorAdaptToDim (const Eigen::Matrix<Scalar,SOURCE_DIM,1,OPTIONS> &v, Scalar extension_component)
{
  return v;
}

// See comment for other version of this.
template <int TARGET_DIM, typename Scalar, int SOURCE_DIM, int OPTIONS>
typename std::enable_if<(TARGET_DIM>SOURCE_DIM),Eigen::Matrix<Scalar,TARGET_DIM,1,OPTIONS>>::type
  VectorAdaptToDim (const Eigen::Matrix<Scalar,SOURCE_DIM,1,OPTIONS> &v, Scalar extension_component)
{
  static const int REMAINING_DIM = TARGET_DIM - SOURCE_DIM;
  static_assert(REMAINING_DIM > 0, "the condition C used in std::enable_if<C,...> is wrong.");
  Eigen::Matrix<Scalar,TARGET_DIM,1,OPTIONS> retval;
  retval.template block<SOURCE_DIM,1>(0,0) = v;
  // Set the remaining components to extension_component.
  for (int i = SOURCE_DIM; i < TARGET_DIM; ++i) {
    retval(i) = extension_component;
  }
  return retval;
}

// TODO: write analogous VectorAdaptToDim for row vectors (the above is only for column vectors).

// This function projects or embeds a square matrix in a different dimensional square matrix space.
// Another way to phrase this is that is truncates or extends-by-identity a square matrix as necessary.
// Let T denote TARGET_DIM and let S denote SOURCE_DIM.
// If T == S, then this returns the input exactly.
// If T < S, and the input is a block matrix
//   [ A b ]
//   [ d c ]
// with A being TxT, then the output is the matrix A.
// If T > S, and the input is a matrix M, then the output is a block matrix
//   [ M 0 ]
//   [ 0 D ]
// where D is the diagonal matrix necessary to fill the block matrix out to the target dimension,
// whose diagonal components are extension_component.  If extension_component is 1, then D is
// the identity matrix of the appropriate size.
template <int TARGET_DIM, typename Scalar, int SOURCE_DIM, int OPTIONS>
typename std::enable_if<(TARGET_DIM<SOURCE_DIM),Eigen::Matrix<Scalar,TARGET_DIM,TARGET_DIM,OPTIONS>>::type
  SquareMatrixAdaptToDim (const Eigen::Matrix<Scalar,SOURCE_DIM,SOURCE_DIM,OPTIONS> &m, Scalar extension_component)
{
  return m.template block<TARGET_DIM,TARGET_DIM>(0,0);
}

// See comment for other version of this.
template <int TARGET_DIM, typename Scalar, int SOURCE_DIM, int OPTIONS>
typename std::enable_if<(TARGET_DIM==SOURCE_DIM),Eigen::Matrix<Scalar,TARGET_DIM,TARGET_DIM,OPTIONS>>::type
  SquareMatrixAdaptToDim (const Eigen::Matrix<Scalar,SOURCE_DIM,SOURCE_DIM,OPTIONS> &m, Scalar extension_component)
{
  return m;
}

// See comment for other version of this.
template <int TARGET_DIM, typename Scalar, int SOURCE_DIM, int OPTIONS>
typename std::enable_if<(TARGET_DIM>SOURCE_DIM),Eigen::Matrix<Scalar,TARGET_DIM,TARGET_DIM,OPTIONS>>::type
  SquareMatrixAdaptToDim (const Eigen::Matrix<Scalar,SOURCE_DIM,SOURCE_DIM,OPTIONS> &m, Scalar extension_component)
{
  static const int REMAINING_DIM = TARGET_DIM - SOURCE_DIM;
  static_assert(REMAINING_DIM > 0, "the condition C used in std::enable_if<C,...> is wrong.");
  Eigen::Matrix<Scalar,TARGET_DIM,TARGET_DIM,OPTIONS> retval;
  retval.template block<SOURCE_DIM,SOURCE_DIM>(0,0) = m;
  retval.template block<REMAINING_DIM,SOURCE_DIM>(SOURCE_DIM,0).setZero();
  retval.template block<SOURCE_DIM,REMAINING_DIM>(0,SOURCE_DIM).setZero();
  // Set the remaining block to be a diagonal scalar matrix using extension_component
  for (int r = SOURCE_DIM; r < TARGET_DIM; ++r) {
    for (int c = SOURCE_DIM; c < TARGET_DIM; ++c) {
      retval(r,c) = (r == c) ? extension_component : Scalar(0);
    }
  }
  return retval;
}

//Marshaling functions
//NOTE:I really, really tried to make this a template function, but got stuck in template hell and did not
//have time to make it work.  This is actually fairly robust, so we'll use it untill someone more familiar
//with Eigen has the time to sort this out. --WG
// TODO: NOTE: Use VectorAdaptToDim and SquareMatrixAdaptToDim instead.
#define ProjectVector(_outDim, data) data.block<_outDim,1>(0,0)
