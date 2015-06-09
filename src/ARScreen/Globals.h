#pragma once
#include <chrono>

class Globals {
public:
  static bool haveOculus;
  static std::chrono::steady_clock::time_point prevFrameTime;
  static std::chrono::steady_clock::time_point curFrameTime;
  static std::chrono::duration<double> timeBetweenFrames;
  static Eigen::Vector3d userPos;
  static Eigen::Vector3d glowColor;
  static bool haveScreen;
  static double screenWidth;
  static double screenHeight;
  static Eigen::Vector3d screenPos;
  static Eigen::Matrix3d screenBasis;
  static double elapsedTimeSeconds;
  static double globalHeightOffset;
  static double globalZOffset;
};
