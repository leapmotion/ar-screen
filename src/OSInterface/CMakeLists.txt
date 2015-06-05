set(OSInterface_SOURCES
  OSApp.h
  OSAppManager.h
  OSAppManager.cpp
  OSGeometry.h
  OSCursor.h
  OSCursor.cpp
  OSScreen.h
  OSScreenBase.h
  OSVirtualScreen.h
  OSVirtualScreen.cpp
  OSWindow.h
  OSWindow.cpp
  OSWindowEvent.h
  OSWindowHandle.h
  OSWindowMonitor.h
  OSWindowMonitor.cpp
)

add_windows_sources(
  OSInterface_SOURCES
  OSAppWin.h
  OSAppWin.cpp
  OSCursorWin.h
  OSCursorWin.cpp
  OSScreenWin.cpp
  OSWindowWin.h
  OSWindowWin.cpp
  OSWindowMonitorWin.h
  OSWindowMonitorWin.cpp
  OSVirtualScreenWin.h
  OSVirtualScreenWin.cpp
)

add_mac_sources(
  OSInterface_SOURCES
  OSAppMac.h
  OSAppMac.mm
  OSCursorMac.h
  OSCursorMac.cpp
  OSScreenMac.mm
  OSVirtualScreenMac.h
  OSVirtualScreenMac.mm
  OSWindowMac.h
  OSWindowMac.mm
  OSWindowMonitorMac.h
  OSWindowMonitorMac.mm
)

add_library(OSInterface ${OSInterface_SOURCES})
set_property(TARGET OSInterface PROPERTY FOLDER "Common")

target_link_libraries(OSInterface PUBLIC Autowiring utility)
target_package(OSInterface Leap REQUIRED LINK_TYPE PUBLIC)
target_package(OSInterface Glew REQUIRED LINK_TYPE PUBLIC)

target_link_libraries(OSInterface PUBLIC Primitives Animation)

if(APPLE)
  find_library(FOUNDATION_FRAMEWORK Foundation)
  find_library(APPKIT_FRAMEWORK AppKit)
  find_library(CORE_AUDIO_FRAMEWORK CoreAudio)
  mark_as_advanced(FOUNDATION_FRAMEWORK)
  mark_as_advanced(APPKIT_FRAMEWORK)
  mark_as_advanced(CORE_AUDIO_FRAMEWORK)
  target_link_libraries(OSInterface PUBLIC ${CORE_AUDIO_FRAMEWORK} ${APPKIT_FRAMEWORK} ${FOUNDATION_FRAMEWORK})
endif()

if(MSVC)
  target_package(OSInterface FreeImage REQUIRED LINK_TYPE PUBLIC)
  set_property(TARGET OSInterface APPEND PROPERTY LINK_FLAGS "/NODEFAULTLIB:libcmt.lib")
  target_link_libraries(OSInterface PUBLIC opengl32 dwmapi psapi)
  target_link_libraries(OSInterface PUBLIC d3d11 dcomp) #used by CompositionEngineWin
endif()
