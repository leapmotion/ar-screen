#include "stdafx.h"
#include "Globals.h"

bool Globals::haveOculus = false;
std::chrono::steady_clock::time_point Globals::prevFrameTime;
std::chrono::steady_clock::time_point Globals::curFrameTime;
std::chrono::duration<double> Globals::timeBetweenFrames;
Eigen::Vector3d Globals::userPos(0, 150, 300);
Eigen::Vector3d Globals::glowColor(0.7, 0.9, 1.0);
bool Globals::haveScreen = false;
double Globals::screenWidth = 1.0;
double Globals::screenHeight = 1.0;
Eigen::Vector3d Globals::screenPos(Eigen::Vector3d::Zero());
Eigen::Matrix3d Globals::screenBasis(Eigen::Matrix3d::Identity());
