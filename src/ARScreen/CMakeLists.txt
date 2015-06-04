set(ARScreen_SOURCES
    main.cpp
)

set(_add_executable_options "")
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") # This is the correct way to detect Mac OS X operating system -- see http://www.openguru.com/2009/04/cmake-detecting-platformoperating.html
    set(_add_executable_options "MACOSX_BUNDLE")
endif()

add_executable(ARScreen ${_add_executable_options} ${ARScreen_SOURCES})

target_link_libraries(ARScreen OculusSDK::OculusSDK SFML::SFML Eigen::Eigen)
