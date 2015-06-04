set(ARScreen_VERSION_MAJOR "0")
set(ARScreen_VERSION_MINOR "1")
set(ARScreen_VERSION_PATCH "0")
set(ARScreen_VERSION "${ARScreen_VERSION_MAJOR}.${ARScreen_VERSION_MINOR}.${ARScreen_VERSION_PATCH}")
set(ARScreen_APPNAME "Augmented Reality Screen")
set(ARScreen_APPDESC "Leap Motion ARScreen")

set(ARScreen_SOURCES
  ARScreen.cpp
  ARScreen.h
  Window.h
  Window.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/Version.h
)

add_pch(ARScreen_SOURCES "stdafx.h" "stdafx.cpp")

set(_add_executable_options "")
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") # This is the correct way to detect Mac OS X operating system -- see http://www.openguru.com/2009/04/cmake-detecting-platformoperating.html
    set(_add_executable_options "MACOSX_BUNDLE")
endif()

configure_file(Version.h.in Version.h)

add_executable(ARScreen ${_add_executable_options} ${ARScreen_SOURCES})

set_property(TARGET ARScreen PROPERTY FOLDER "Applications")

target_link_libraries(ARScreen utility)
target_link_libraries(ARScreen OpenGL::OpenGL OculusSDK::OculusSDK SFML::SFML Eigen::Eigen Leap::Leap Glew::Glew FreeImage::FreeImage)
