#pragma once

#include "EigenTypes.h"
#include "Leap.h"

static EigenTypes::Matrix3x3 RotationMatrixFromEulerAngles(EigenTypes::MATH_TYPE pitch, EigenTypes::MATH_TYPE yaw, EigenTypes::MATH_TYPE roll) {
  EigenTypes::Matrix3x3 m;
  m = Eigen::AngleAxis<EigenTypes::MATH_TYPE>(pitch, EigenTypes::Vector3::UnitX())
    * Eigen::AngleAxis<EigenTypes::MATH_TYPE>(yaw, EigenTypes::Vector3::UnitY())
    * Eigen::AngleAxis<EigenTypes::MATH_TYPE>(roll, EigenTypes::Vector3::UnitZ());
  return m;
}

static void getCurTimeGMT(int gmtPlus, int& hours, int& minutes, int& seconds, bool& am) {
  std::time_t curTime = std::time(NULL);
  std::tm* t = std::gmtime(&curTime);
  hours = (t->tm_hour + gmtPlus);
  if (hours < 0) {
    hours += 24;
  } else if (hours > 24) {
    hours -= 24;
  }
  am = hours < 12;
  hours = hours % 12;
  if (hours == 0) {
    hours = 12;
  }
  minutes = t->tm_min;
  seconds = t->tm_sec;
}

static std::string getTimeString(int gmtPlus) {
  int hours, minutes, seconds;
  bool am;
  getCurTimeGMT(gmtPlus, hours, minutes, seconds, am);
  std::stringstream ss;
  ss << hours;
  ss << ":";
  ss << std::setfill('0') << std::setw(2) << minutes;
  ss << ":";
  ss << std::setfill('0') << std::setw(2) << seconds;
  ss << " " << (am ? "am" : "pm");
  return ss.str();
}

static double radiansToDegrees(double radians) {
  return (180.0 / M_PI) * radians;
}

static double degreesToRadians(double degrees) {
  return (M_PI / 180.0) * degrees;
}

static double timestampToSeconds(int64_t timestampMicrosecs) {
  return 1.0E-6 * timestampMicrosecs;
}

static Eigen::Matrix3d toEigen(const Leap::Matrix& mat) {
  Eigen::Matrix3d result;
  result.col(0) << mat.xBasis.x, mat.xBasis.y, mat.xBasis.z;
  result.col(1) << mat.yBasis.x, mat.yBasis.y, mat.yBasis.z;
  result.col(2) << mat.zBasis.x, mat.zBasis.y, mat.zBasis.z;
  return result;
}

static Eigen::Matrix3d faceCameraMatrix(const Eigen::Vector3d& translation, const Eigen::Vector3d& center) {
#if 0
  const Eigen::Vector3d diff = (center - translation).normalized();
  Eigen::Vector3d up = Eigen::Vector3d::UnitY();
  const Eigen::Vector3d side = up.cross(diff).normalized();
  up = diff.cross(side).normalized();
#else
  const Eigen::Vector3d up = Eigen::Vector3d::UnitY();
  Eigen::Vector3d diff = (center - translation).normalized();
  const Eigen::Vector3d side = up.cross(diff).normalized();
  diff = side.cross(up);
#endif
  Eigen::Matrix3d result;
  result << side, up, diff;
  return result;
}

static bool IntersectPlane(const Eigen::Vector3d& rayOrigin, const Eigen::Vector3d& rayDir, const Eigen::Vector3d& center, const Eigen::Vector3d& normal, double& t) {
  const double temp = -(normal.dot(rayOrigin - center)) / normal.dot(rayDir);
  if (temp > 0 && temp < t) {
    t = temp;
    return true;
  }
  return false;
}
