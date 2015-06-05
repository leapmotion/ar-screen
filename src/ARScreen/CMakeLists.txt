set(ARScreen_VERSION_MAJOR "0")
set(ARScreen_VERSION_MINOR "1")
set(ARScreen_VERSION_PATCH "0")
set(ARScreen_VERSION "${ARScreen_VERSION_MAJOR}.${ARScreen_VERSION_MINOR}.${ARScreen_VERSION_PATCH}")
set(ARScreen_APPNAME "Augmented Reality Screen")
set(ARScreen_APPDESC "Leap Motion ARScreen")

set(ARScreen_SOURCES
  Animation.h
  ARScreen.cpp
  ARScreen.h
  EigenTypes.h
  Window.cpp
  Window.h
  ${CMAKE_CURRENT_BINARY_DIR}/Version.h
)

set (GLTexture2FreeImage_SOURCES
  GLTexture2FreeImage/GLTexture2FreeImage.cpp
  GLTexture2FreeImage/GLTexture2FreeImage.h
)

set (GLTexture2Image_SOURCES
  GLTexture2Image/GLTexture2Image.cpp
  GLTexture2Image/GLTexture2Image.h
)
  
set (LeapGL_SOURCES
  Leap/GL/BufferObject.h
  Leap/GL/ColorComponent.h
  Leap/GL/Common.h
  Leap/GL/Error.h
  Leap/GL/Exception.h
  Leap/GL/GLHeaders.h
  Leap/GL/Internal/ColorComponent.h
  Leap/GL/Internal/EigenTypes.h
  Leap/GL/Internal/Map.h
  Leap/GL/Internal/Meta.h
  Leap/GL/Internal/ShaderFrontend.h
  Leap/GL/Internal/ShaderUniform.h
  Leap/GL/Internal/Tuple.h
  Leap/GL/Internal/TypeMap.h
  Leap/GL/Internal/Typle.h
  Leap/GL/Internal/UniformTraits.h
  Leap/GL/Internal/UniformUploader.h
  Leap/GL/Internal/VertexAttribute.h
  Leap/GL/Mesh.h
  Leap/GL/MeshAssembler.h
  Leap/GL/MeshException.h
  Leap/GL/ModelView.h
  Leap/GL/Projection.h
  Leap/GL/ResourceBase.h
  Leap/GL/Rgb.h
  Leap/GL/Rgba.h
  Leap/GL/Shader.h
  Leap/GL/ShaderException.h
  Leap/GL/ShaderFrontend.h
  Leap/GL/ShaderMatrices.h
  Leap/GL/Texture2.h
  Leap/GL/Texture2Exception.h
  Leap/GL/Texture2Params.h
  Leap/GL/Texture2PixelData.h
  Leap/GL/VertexAttribute.h
  Leap/GL/VertexBufferObject.h
  Leap/GL/VertexBufferObjectException.h
  Leap/GL/BufferObject.cpp
  Leap/GL/ModelView.cpp
  Leap/GL/Projection.cpp
  Leap/GL/Shader.cpp
  Leap/GL/ShaderMatrices.cpp
  Leap/GL/Texture2.cpp
  Leap/GL/Texture2Params.cpp
  Leap/GL/Texture2PixelData.cpp
)

set (LeapListener_SOURCES
  LeapListener/LeapListener.cpp
  LeapListener/LeapListener.h
)

set (OculusVR_SOURCES
  OculusVR/OculusVR.cpp
  OculusVR/OculusVR.h
)

set (Primitives_SOURCES
  Primitives/DropShadow.h
  Primitives/LambertianMaterial.h
  Primitives/PrimitiveBase.h
  Primitives/PrimitiveGeometry.h
  Primitives/Primitives.h
  Primitives/RenderState.h
  Primitives/ScopeGuard.h
  Primitives/ShaderBindingScopeGuard.h
  Primitives/SVGPrimitive.h
  Primitives/TexturedFrame.h
  Primitives/DropShadow.cpp
  Primitives/PrimitiveGeometry.cpp
  Primitives/Primitives.cpp
  Primitives/SVGPrimitive.cpp
  Primitives/TexturedFrame.cpp
)

set (SceneGraph_SOURCES
  SceneGraph/SceneGraphNode.h
  SceneGraph/SceneGraphNodeProperties.h
  SceneGraph/SceneGraphNodeProperty.h
  SceneGraph/SceneGraphNodeValues.h
)

#set (TextureFont_SOURCES
#  TextPrimitive.cpp
#  TextPrimitive.h
#  TextureFont.cpp
#  TextureFont.h
#)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories("GLTexture2FreeImage")
include_directories("GLTexture2Image")
include_directories("Leap/GL")
include_directories("LeapListener")
include_directories("OculusVR")
include_directories("Primitives")
include_directories("SceneGraph")

add_pch(ARScreen_SOURCES "stdafx.h" "stdafx.cpp")

set(_add_executable_options "")
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(_add_executable_options ${_add_executable_options} MACOSX_BUNDLE)
endif()

configure_file(Version.h.in Version.h)

add_executable(ARScreen
  ${_add_executable_options}
  ${ARScreen_SOURCES}
  ${GLTexture2FreeImage_SOURCES}
  ${GLTexture2Image_SOURCES}
  ${LeapGL_SOURCES}
  ${LeapListener_SOURCES}
  ${OculusVR_SOURCES}
  ${Primitives_SOURCES}
  ${SceneGraph_SOURCES}
) 

set_property(TARGET ARScreen PROPERTY FOLDER "Applications")

target_link_libraries(ARScreen
  utility
  NanoSVG::NanoSVG
  OpenGL::OpenGL
  OculusSDK::OculusSDK
  SFML::SFML
  Eigen::Eigen
  Leap::Leap
  Glew::Glew
  FreeImage::FreeImage
  PolyPartition::PolyPartition
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
