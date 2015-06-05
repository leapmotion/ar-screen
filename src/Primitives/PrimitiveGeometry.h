#pragma once

#include "utility/EigenTypes.h"
#include "Leap/GL/BufferObject.h"
#include "Leap/GL/Mesh.h"
#include "Leap/GL/MeshAssembler.h"
#include "RenderState.h"

#include <map>
#include <vector>

namespace Leap {
namespace GL {

class Shader;

} // end of namespace GL
} // end of namespace Leap

typedef Leap::GL::Mesh<Leap::GL::VertexAttribute<GL_FLOAT_VEC3>, // Position
                       Leap::GL::VertexAttribute<GL_FLOAT_VEC3>, // Normal vector
                       Leap::GL::VertexAttribute<GL_FLOAT_VEC2>, // 2D texture coordinate
                       Leap::GL::VertexAttribute<GL_FLOAT_VEC4>  // RGBA color
                       > PrimitiveGeometryMesh;
typedef Leap::GL::MeshAssembler<Leap::GL::VertexAttribute<GL_FLOAT_VEC3>, // Position
                                Leap::GL::VertexAttribute<GL_FLOAT_VEC3>, // Normal vector
                                Leap::GL::VertexAttribute<GL_FLOAT_VEC2>, // 2D texture coordinate
                                Leap::GL::VertexAttribute<GL_FLOAT_VEC4>  // RGBA color
                                > PrimitiveGeometryMeshAssembler;

namespace PrimitiveGeometry {

// TODO: make the PrimitiveGeometryMeshAssembler argument first.

// Functions for populating a PrimitiveGeometryMeshAssembler object with some simple shapes.  These functions assume that
// the draw mode of the mesh is GL_TRIANGLES.
void PushUnitSphere(int widthResolution, int heightResolution, PrimitiveGeometryMeshAssembler& mesh_assembler, double heightAngleStart = -M_PI/2.0, double heightAngleEnd = M_PI/2.0, double widthAngleStart = 0, double widthAngleEnd = 2.0*M_PI);
void PushUnitCylinder(int radialResolution, int verticalResolution, PrimitiveGeometryMeshAssembler& mesh_assembler, float radiusBottom = 1.0f, float radiusTop = 1.0f, double angleStart = 0, double angleEnd = 2.0*M_PI);
void PushUnitSquare(PrimitiveGeometryMeshAssembler &mesh_assembler);
void PushUnitDisk(size_t resolution, PrimitiveGeometryMeshAssembler &mesh_assembler);
void PushUnitBox(PrimitiveGeometryMeshAssembler &mesh_assembler);

} // end of namespace PrimitiveGeometry
