
set (TextureFont_SOURCES
  TextPrimitive.cpp
  TextPrimitive.h
  TextureFont.cpp
  TextureFont.h
)

add_pch(TextureFont_SOURCES "stdafx.h" "stdafx.cpp")

add_library(TextureFont ${TextureFont_SOURCES})
set_property(TARGET TextureFont PROPERTY FOLDER "Common")
target_link_libraries(TextureFont Primitives Freetype-gl::Freetype-gl ${FREETYPE_LIBRARY})
