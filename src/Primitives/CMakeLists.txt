
set (Primitives_SOURCES
  DropShadow.h
  DropShadow.cpp
  LambertianMaterial.h
  PrimitiveBase.h
  PrimitiveGeometry.h
  PrimitiveGeometry.cpp
  Primitives.h
  Primitives.cpp
  RenderState.h
  SceneGraphNode.h
  SceneGraphNodeProperties.h
  SceneGraphNodeProperty.h
  SceneGraphNodeValues.h
  ScopeGuard.h
  ShaderBindingScopeGuard.h
  SVGPrimitive.h
  SVGPrimitive.cpp
  TexturedFrame.h
  TexturedFrame.cpp
)

add_pch(Primitives_SOURCES "stdafx.h" "stdafx.cpp")

add_library(Primitives ${Primitives_SOURCES})
set_property(TARGET Primitives PROPERTY FOLDER "Common")
target_link_libraries(Primitives LeapGL NanoSVG::NanoSVG PolyPartition::PolyPartition)
