#pragma once
#include <chrono>

namespace Globals {
  static bool haveOculus = false;
  static std::chrono::steady_clock::time_point prevFrameTime;
  static std::chrono::steady_clock::time_point curFrameTime;
  static std::chrono::duration<double> timeBetweenFrames;
  static Eigen::Vector3d userPos(0, 150, 400);
};
