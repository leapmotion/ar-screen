#pragma once
#include <chrono>

class Globals {
public:
  static bool haveOculus;
  static std::chrono::steady_clock::time_point prevFrameTime;
  static std::chrono::steady_clock::time_point curFrameTime;
  static std::chrono::duration<double> timeBetweenFrames;
  static Eigen::Vector3d userPos;
  static Eigen::Vector3d offset;
  static Eigen::Vector3d glowColor;
};
