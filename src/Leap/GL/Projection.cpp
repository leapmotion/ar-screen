#include "stdafx.h"
#include <cassert>
#include <cmath>
#include "Leap/GL/Projection.h"
#include <limits>

namespace Leap {
namespace GL {
namespace Projection {

namespace {

void CheckOrthographicValues (double width, double height, double depth, double near_clip_depth) {
  // The width, height, depth, and near_clip_depth must all be nonzero (by an epsilon-margin)
  assert(std::abs(width) > std::numeric_limits<double>::epsilon());
  assert(std::abs(height) > std::numeric_limits<double>::epsilon());
  assert(std::abs(depth) > std::numeric_limits<double>::epsilon());
  assert(std::abs(near_clip_depth) > std::numeric_limits<double>::epsilon());
}

void CheckPerspectiveValues (double width, double height, double depth, double near_clip_depth) {
  // The width, height, depth, and near_clip_depth must all be nonzero (by an epsilon-margin)
  assert(std::abs(width) > std::numeric_limits<double>::epsilon());
  assert(std::abs(height) > std::numeric_limits<double>::epsilon());
  assert(std::abs(depth) > std::numeric_limits<double>::epsilon());
  assert(std::abs(near_clip_depth) > std::numeric_limits<double>::epsilon());
  assert(depth*near_clip_depth > 0.0 && "The depth and near_clip_depth values must have the same sign (otherwise there is a singularity in the transformation).");
}

} // end of anonymous namespace

void SetOrthographic (
  EigenTypes::Matrix4x4 &projection_matrix,
  double left, double right,
  double bottom, double top,
  double near_clip_depth, double far_clip_depth)
{
  const double width = right - left;
  const double height = top - bottom;
  const double depth = far_clip_depth - near_clip_depth;

  CheckOrthographicValues(width, height, depth, near_clip_depth);

  const double l00 = 2.0 / width;
  const double l11 = 2.0 / height;
  const double l22 = -2.0 / depth;
  const double t0 = -(right + left) / width;
  const double t1 = -(bottom + top) / height;
  const double t2 = -(far_clip_depth + near_clip_depth) / depth;
  projection_matrix << l00, 0.0, 0.0,  t0,
                       0.0, l11, 0.0,  t1,
                       0.0, 0.0, l22,  t2,
                       0.0, 0.0, 0.0, 1.0;
}

void SetOrthographic_UsingSymmetricViewBox (
  EigenTypes::Matrix4x4 &projection_matrix,
  double width, double height,
  double near_clip_depth, double far_clip_depth)
{
  const double depth = far_clip_depth - near_clip_depth;

  CheckOrthographicValues(width, height, depth, near_clip_depth);

  const double l00 = 2.0 / width;
  const double l11 = 2.0 / height;
  const double l22 = -2.0 / depth;
  const double t2 = -(far_clip_depth + near_clip_depth) / depth;
  projection_matrix << l00, 0.0, 0.0, 0.0,
                       0.0, l11, 0.0, 0.0,
                       0.0, 0.0, l22,  t2,
                       0.0, 0.0, 0.0, 1.0;
}

void SetPerspective (
  EigenTypes::Matrix4x4 &projection_matrix,
  double near_clip_left, double near_clip_right,
  double near_clip_bottom, double near_clip_top,
  double near_clip_depth, double far_clip_depth)
{
  const double near_clip_width = near_clip_right - near_clip_left;
  const double near_clip_height = near_clip_top - near_clip_bottom;
  const double depth = far_clip_depth - near_clip_depth;

  CheckPerspectiveValues(near_clip_width, near_clip_height, depth, near_clip_depth);

  const double denom = 1.0 / (near_clip_depth - far_clip_depth);
  const double l00 = 2.0 * near_clip_depth / near_clip_width;
  const double l11 = 2.0 * near_clip_depth / near_clip_height;
  const double l22 = (near_clip_depth + far_clip_depth) * denom;
  const double t0 = (near_clip_right + near_clip_left) / near_clip_width;
  const double t1 = (near_clip_top + near_clip_bottom) / near_clip_height;
  const double t2 = 2.0 * near_clip_depth * far_clip_depth * denom;
  projection_matrix << l00, 0.0,  0.0,  t0,
                       0.0, l11,  0.0,  t1,
                       0.0, 0.0,  l22,  t2,
                       0.0, 0.0, -1.0, 0.0;
}

void SetPerspective_UsingFOVAndAspectRatio (
  EigenTypes::Matrix4x4 &projection_matrix,
  double horiz_FOV_radians, double width_over_height,
  double near_clip_depth, double far_clip_depth)
{
  assert(std::abs(horiz_FOV_radians) > std::numeric_limits<double>::epsilon());
  assert(std::abs(width_over_height) > std::numeric_limits<double>::epsilon());

  // Derive width and height and forward the call.
  const double near_clip_width = 2.0 * near_clip_depth * std::tan(horiz_FOV_radians / 2.0);
  const double near_clip_height = near_clip_width / width_over_height;
  SetPerspective_UsingSymmetricFrustumNearClipSize(projection_matrix, near_clip_width, near_clip_height, near_clip_depth, far_clip_depth);
}

void SetPerspective_UsingSymmetricFrustumNearClipSize (
  EigenTypes::Matrix4x4 &projection_matrix,
  double near_clip_width, double near_clip_height,
  double near_clip_depth, double far_clip_depth)
{
  const double depth = far_clip_depth - near_clip_depth;

  CheckPerspectiveValues(near_clip_width, near_clip_height, depth, near_clip_depth);

  const double denom = -1.0 / depth;
  const double l00 = 2.0 * near_clip_depth / near_clip_width;
  const double l11 = 2.0 * near_clip_depth / near_clip_height;
  const double l22 = (near_clip_depth + far_clip_depth) * denom;
  const double t2 = 2.0 * near_clip_depth * far_clip_depth * denom;
  projection_matrix << l00, 0.0,  0.0, 0.0,
                       0.0, l11,  0.0, 0.0,
                       0.0, 0.0,  l22,  t2,
                       0.0, 0.0, -1.0, 0.0;
}

} // end of namespace Projection
} // end of namespace GL
} // end of namespace Leap
