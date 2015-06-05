set(ARScreen_VERSION_MAJOR "0")
set(ARScreen_VERSION_MINOR "1")
set(ARScreen_VERSION_PATCH "0")
set(ARScreen_VERSION "${ARScreen_VERSION_MAJOR}.${ARScreen_VERSION_MINOR}.${ARScreen_VERSION_PATCH}")
set(ARScreen_APPNAME "Augmented Reality Screen")
set(ARScreen_APPDESC "Leap Motion ARScreen")

set(ARScreen_SOURCES
  ARScreen.cpp
  ARScreen.h
  GLShaderLoader.h
  Globals.cpp
  Globals.h
  HandInfo.h
  HandInfo.cpp
  ImagePassthrough.cpp
  ImagePassthrough.h
  Resource.h
  ResourceManager.h
  Scene.h
  Scene.cpp
  Window.cpp
  Window.h
  ${CMAKE_CURRENT_BINARY_DIR}/Version.h
)

#set (TextureFont_SOURCES
#  TextPrimitive.cpp
#  TextPrimitive.h
#  TextureFont.cpp
#  TextureFont.h
#)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_pch(ARScreen_SOURCES "stdafx.h" "stdafx.cpp")

set(_add_executable_options "")
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(_add_executable_options ${_add_executable_options} MACOSX_BUNDLE)
endif()

configure_file(Version.h.in Version.h)

add_executable(ARScreen
  ${_add_executable_options}
  ${ARScreen_SOURCES}
)

set_property(TARGET ARScreen PROPERTY FOLDER "Applications")

target_link_libraries(ARScreen
  utility
  GLTexture2Image
  Primitives
  LeapGL
  LeapListener
  OculusVR
  Leap::Leap
  SFML::SFML
  Eigen::Eigen
)
target_strip(ARScreen)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set_target_properties(ARScreen PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/src/ARScreen/ARScreen.plist.in)
endif()

# Install commands for the Resources
install(TARGETS ARScreen COMPONENT ARScreen
  RUNTIME DESTINATION "."
  BUNDLE DESTINATION "."
  RESOURCE DESTINATION "."
)
