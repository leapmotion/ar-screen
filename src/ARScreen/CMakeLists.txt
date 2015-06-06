set(ARScreen_VERSION_MAJOR "0")
set(ARScreen_VERSION_MINOR "1")
set(ARScreen_VERSION_PATCH "0")
set(ARScreen_VERSION "${ARScreen_VERSION_MAJOR}.${ARScreen_VERSION_MINOR}.${ARScreen_VERSION_PATCH}")
set(ARScreen_APPNAME "Augmented Reality Screen")
set(ARScreen_APPDESC "Leap Motion ARScreen")

set(ARScreen_RESOURCES
  ${CMAKE_SOURCE_DIR}/fonts/Roboto-Regular.ttf
)

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
  WindowManager.h
  WindowManager.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/Version.h
)


include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_pch(ARScreen_SOURCES "stdafx.h" "stdafx.cpp")
add_conditional_sources(ARScreen_SOURCES "" GROUP_NAME "Resource Files" "FILES" ${ARScreen_RESOURCES})

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
  OSInterface
  Leap::Leap
  SFML::SFML
  Eigen::Eigen
  TextureFont
)
target_strip(ARScreen)

if(WIN32)
  set_property(TARGET ARScreen APPEND PROPERTY RESOURCE ${ARScreen_RESOURCES})
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set_target_properties(ARScreen PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/src/ARScreen/ARScreen.plist.in)
endif()

foreach(_resource ${ARScreen_RESOURCES})
  if(WIN32)
    add_custom_command(TARGET ARScreen POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different \"${_resource}\" \"$<TARGET_FILE_DIR:ARScreen>\"
    )
  elseif(APPLE)
    get_filename_component(_resource ${_resource} ABSOLUTE)
    get_target_property(_is_bundle ARScreen MACOSX_BUNDLE)
    if(_is_bundle)
      add_custom_command(TARGET ARScreen POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory \"$<TARGET_FILE_DIR:ARScreen>/../Resources/\"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different \"${_resource}\" \"$<TARGET_FILE_DIR:ARScreen>/../Resources/\"
      )
    endif()
  endif()
endforeach()

# Install commands for the Resources
install(TARGETS ARScreen COMPONENT ARScreen
  RUNTIME DESTINATION "."
  BUNDLE DESTINATION "."
  RESOURCE DESTINATION "."
)
