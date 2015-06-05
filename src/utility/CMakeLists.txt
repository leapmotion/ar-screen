
set(utility_SOURCES
  Animation.h
  EigenTypes.h
  PlatformInitializer.h
  SamplePrimitives.h
  SamplePrimitives.cpp
  Shaders.h
  Updatable.h
  Utilities.h
)

add_windows_sources(utility_SOURCES
  HandleUtilitiesWin.h
  PlatformInitializerWin.h
  PlatformInitializerWin.cpp
  SingleInstanceEnforcerWin.h
  SingleInstanceEnforcerWin.cpp
  WindowsIncludes.h
)

add_mac_sources(utility_SOURCES
  PlatformInitializerMac.h
  PlatformInitializerMac.mm
)

add_pch(utility_SOURCES "stdafx.h" "stdafx.cpp")

add_library(utility ${utility_SOURCES})
set_property(TARGET utility PROPERTY FOLDER "Common")
target_link_libraries(utility LeapGL Autowiring::Autowiring Eigen::Eigen)

if(MSVC)
  target_link_libraries(utility shlwapi)
elseif(APPLE)
  find_library(APPKIT_FRAMEWORK AppKit)
  mark_as_advanced(APPKIT_FRAMEWORK)
  target_link_libraries(utility ${APPKIT_FRAMEWORK})
endif()
