#pragma once
#include <chrono>

class Updatable {
public:
  /// <summary>
  /// A time-computed update notification, invoked periodically to notify listeners of the passage of time
  /// </summary>
  virtual void Tick(std::chrono::duration<double> deltaT) = 0;
};