
set (OculusVR_SOURCES
  OculusVR.cpp
  OculusVR.h
)

add_pch(OculusVR_SOURCES "stdafx.h" "stdafx.cpp")

add_library(OculusVR ${OculusVR_SOURCES})
set_property(TARGET OculusVR PROPERTY FOLDER "Common")
target_link_libraries(OculusVR LeapGL OculusSDK::OculusSDK)
