set (LeapGL_SOURCES
  BufferObject.h
  ColorComponent.h
  Common.h
  Error.h
  Exception.h
  GLHeaders.h
  Internal/ColorComponent.h
  Internal/Map.h
  Internal/Meta.h
  Internal/ShaderFrontend.h
  Internal/ShaderUniform.h
  Internal/Tuple.h
  Internal/TypeMap.h
  Internal/Typle.h
  Internal/UniformTraits.h
  Internal/UniformUploader.h
  Internal/VertexAttribute.h
  Mesh.h
  MeshAssembler.h
  MeshException.h
  ModelView.h
  Projection.h
  ResourceBase.h
  Rgb.h
  Rgba.h
  Shader.h
  ShaderException.h
  ShaderFrontend.h
  ShaderMatrices.h
  Texture2.h
  Texture2Exception.h
  Texture2Params.h
  Texture2PixelData.h
  VertexAttribute.h
  VertexBufferObject.h
  VertexBufferObjectException.h
  BufferObject.cpp
  ModelView.cpp
  Projection.cpp
  Shader.cpp
  ShaderMatrices.cpp
  Texture2.cpp
  Texture2Params.cpp
  Texture2PixelData.cpp
)

add_pch(LeapGL_SOURCES "stdafx.h" "stdafx.cpp")

add_library(LeapGL ${LeapGL_SOURCES})
target_link_libraries(LeapGL
  Glew::Glew
  OpenGL::OpenGL
  Eigen::Eigen
  FreeImage::FreeImage
)
