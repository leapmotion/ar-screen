include_directories(
  .

  # Need to make sure all of our tests can find gtest
  ${PROJECT_SOURCE_DIR}/contrib/gtest-1.7.0/fused-src
)

# Enable the use of project directories on any platforms that support it
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

# Add all subdirectories:
file(GLOB all_subdirectories RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)
foreach(subdirectory ${all_subdirectories})
  set(curdir "${CMAKE_CURRENT_SOURCE_DIR}/${subdirectory}")
  if(NOT ${subdirectory} STREQUAL "." AND EXISTS "${curdir}/CMakeLists.txt")
    add_subdirectory(${subdirectory})
  endif()
endforeach()

set(FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} PARENT_SCOPE)

set(ARScreen_SOURCES
    main.cpp
)

set(_add_executable_options "")
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") # This is the correct way to detect Mac OS X operating system -- see http://www.openguru.com/2009/04/cmake-detecting-platformoperating.html
    set(_add_executable_options "MACOSX_BUNDLE")
endif()

add_executable(ARScreen ${_add_executable_options} ${ARScreen_SOURCES})

target_link_libraries(ARScreen OculusSDK::OculusSDK SFML::SFML Eigen::Eigen)
