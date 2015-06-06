cmake_minimum_required(VERSION 3.0)

project(ARScreen)

include(CTest)

set(CPACK_GENERATOR "ZIP")
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)

set(LEAP_COMPANY_NAME "Leap Motion")
set(APP_COPYRIGHT "Copyright © 2015 Leap Motion, Inc. All rights reserved.")

# Add our local modules directory and include our commonly used files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules")
include(LeapCMakeTemplates)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(CMAKE_XCODE_ATTRIBUTE_STRIP_INSTALLED_PRODUCT YES)
  set(CMAKE_XCODE_ATTRIBUTE_DEAD_CODE_STRIPPING YES)
endif()

leap_find_external_libraries(x86_vc12)
leap_use_standard_platform_settings()

macro(target_strip Target)
  # primary function of this macro: strip symbols at link time on Mac and Linux
  # secondary function: avoid MSVC warnings in Debug by specifying /nodefaultlib
  # tertiary function: avoid MSVC warnings about combining /incremental with /ltcg
  if(MSVC)
    if(USE_PRODUCTION_FLAGS)
      set_target_properties(${Target} PROPERTIES LINK_FLAGS_DEBUG "/INCREMENTAL:NO /NODEFAULTLIB:MSVCRT")
    else()
      set_target_properties(${Target} PROPERTIES LINK_FLAGS_DEBUG "/NODEFAULTLIB:MSVCRT")
    endif()
  elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set_target_properties(${Target} PROPERTIES LINK_FLAGS_RELEASE "-Xlinker -unexported_symbol -Xlinker \"*\" -Xlinker -dead_strip -Xlinker -dead_strip_dylibs")
  endif()
endmacro()

include(TargetImportedLibraries)
include(AddPCH)
include(ConditionalSources)
include(CMakeFindDependencyMacro)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

enable_testing()

#Find installed autowiring, or point to local version with autowiring_DIR
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
  set(autowiring_ARCHITECTURE "arm")
else()
  if(DEFINED CMAKE_OSX_ARCHITECTURES)
    set(autowiring_ARCHITECTURE ${CMAKE_OSX_ARCHITECTURES})
  endif()
  if(BUILD_64_BIT)
    set(autowiring_ARCHITECTURE "amd64")
  else()
    set(autowiring_ARCHITECTURE "x86")
  endif()
endif()
find_package(autowiring 0.5.4 REQUIRED)
find_package(Eigen 3.2.3 REQUIRED)
find_package(FreeImage REQUIRED)
find_package(Glew 1.9.0 REQUIRED)
find_package(Leap REQUIRED)
find_package(nanosvg REQUIRED)
find_package(polypartition REQUIRED)
find_package(OculusSDK REQUIRED)
find_package(OpenGL REQUIRED)
set(SFML_STATIC_LIBRARIES TRUE)
find_package(SFML 2.1 REQUIRED)
if (APPLE)
find_package(Freetype REQUIRED)
endif()
find_package(Freetype-gl REQUIRED)

include_directories(${CMAKE_BINARY_DIR})
include_directories(src)
include_directories(${autowiring_INCLUDE_DIR})

add_subdirectory(src)
include(CPack)
