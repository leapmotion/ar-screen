
set (LeapListener_SOURCES
  LeapListener.cpp
  LeapListener.h
)

add_pch(LeapListener_SOURCES "stdafx.h" "stdafx.cpp")

add_library(LeapListener ${LeapListener_SOURCES})
set_property(TARGET LeapListener PROPERTY FOLDER "Common")
target_link_libraries(LeapListener LeapGL Leap::Leap)
