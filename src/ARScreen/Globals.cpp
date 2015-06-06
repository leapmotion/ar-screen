#include "stdafx.h"
#include "Globals.h"

bool Globals::haveOculus = false;
std::chrono::steady_clock::time_point Globals::prevFrameTime;
std::chrono::steady_clock::time_point Globals::curFrameTime;
std::chrono::duration<double> Globals::timeBetweenFrames;
Eigen::Vector3d Globals::userPos(0, 150, 400);
Eigen::Vector3d Globals::offset(0, 0, 0);
Eigen::Vector3d Globals::glowColor(0.7, 0.9, 1.0);
