#pragma once

// stl
#define _USE_MATH_DEFINES
#include <cmath>
#include <ctime>
#include <vector>
#include <thread>
#include <mutex>
#include <cassert>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <memory>
#include <string>
#include <sstream>
#include <fstream>

// autowiring
#include <autowiring/autowiring.h>

// windows-specific
#ifdef _MSC_VER
#include "utility/WindowsIncludes.h"
#endif

// opengl
#if __APPLE__
#include <GL/glew.h>
#elif _WIN32
#define GLEW_STATIC
#include <GL/glew.h>
#endif

// sfml
#include <SFML/Window.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

// leap
#include <Leap.h>

// oculus
#include <OVR.h>
#include <OVR_Kernel.h>

// freeimage
#define FREEIMAGE_LIB
#include <FreeImage.h>

// eigen
#include <Eigen/Dense>
#include <Eigen/Geometry>
