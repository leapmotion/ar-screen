#pragma once

namespace Globals {
  static bool haveOculus = false;
  static std::chrono::system_clock::time_point prevFrameTime;
  static std::chrono::system_clock::time_point curFrameTime;
  static std::chrono::duration<double> timeBetweenFrames;
};
