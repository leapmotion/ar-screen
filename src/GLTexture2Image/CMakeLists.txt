
set (GLTexture2Image_SOURCES
  GLTexture2FreeImage.cpp
  GLTexture2FreeImage.h
  GLTexture2Image.cpp
  GLTexture2Image.h
)

add_pch(GLTexture2Image_SOURCES "stdafx.h" "stdafx.cpp")

add_library(GLTexture2Image ${GLTexture2Image_SOURCES})
set_property(TARGET GLTexture2Image PROPERTY FOLDER "Common")
target_link_libraries(GLTexture2Image LeapGL FreeImage::FreeImage)
