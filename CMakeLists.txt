cmake_minimum_required(VERSION 3.0)

#CMAKE_OSX_* variables must be set prior to the first call to project() to function correctly.
#If this is not done, the first time you configure, they will contain bad values.
if(APPLE)
  if (NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Mac OS X build architectures" FORCE)
  endif()
  if (NOT CMAKE_OSX_SYSROOT)
    set(CMAKE_OSX_SYSROOT "macosx10.9" CACHE STRING "Mac OS X build environment" FORCE)
  endif()
  set(CMAKE_OSX_DEPLOYMENT_TARGET ${OSX_MINIMUM_VERSION} CACHE STRING "Mac OS X deployment target" FORCE)
  mark_as_advanced(CMAKE_OSX_DEPLOYMENT_TARGET)
endif()

project(ARScreen)

include(CTest)

set(CPACK_GENERATOR "ZIP")
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)

set(LEAP_COMPANY_NAME "Leap Motion")
set(APP_COPYRIGHT "Copyright © 2015 Leap Motion, Inc. All rights reserved.")

# Add our local modules directory and include our commonly used files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules")
include(LeapCMakeTemplates)

if(APPLE)
  set(CMAKE_XCODE_ATTRIBUTE_STRIP_INSTALLED_PRODUCT YES)
  set(CMAKE_XCODE_ATTRIBUTE_DEAD_CODE_STRIPPING YES)
endif()

leap_find_external_libraries(x86_vc12)
leap_use_standard_platform_settings()

include(TargetImportedLibraries)
include(AddPCH)
include(ConditionalSources)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

enable_testing()

find_package(Eigen 3.2.3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Glew 1.9.0 REQUIRED)
if(MSVC)
  find_package(FreeImage REQUIRED)
endif()

add_subdirectory(src)
include(CPack)
