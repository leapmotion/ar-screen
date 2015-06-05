#include "stdafx.h"
#include "PrimitiveGeometry.h"

#include "Leap/GL/Shader.h"

void PrimitiveGeometry::PushUnitSphere(int widthResolution, int heightResolution, PrimitiveGeometryMeshAssembler& mesh_assembler, double heightAngleStart, double heightAngleEnd, double widthAngleStart, double widthAngleEnd) {
  if (!mesh_assembler.IsInitialized()) {
    throw std::invalid_argument("Can't call PrimitiveGeometry::PushUnitSphere on a !IsInitialized() MeshAssembler.");
  }
  if (mesh_assembler.DrawMode() != GL_TRIANGLES) {
    throw std::invalid_argument("The PrimitiveGeometry::PushUnitSphere function requires mesh_assembler.DrawMode() to be GL_TRIANGLES.");
  }

  const float resFloatW = static_cast<float>(widthResolution);
  const float resFloatH = static_cast<float>(heightResolution);
  const float pi = static_cast<float>(M_PI);
  const float twoPi = static_cast<float>(2.0 * M_PI);

  const float widthStart = static_cast<float>(widthAngleStart);
  const float widthSweep = std::min(twoPi, static_cast<float>(widthAngleEnd - widthAngleStart));
  const float heightStart = static_cast<float>(heightAngleStart);
  const float heightSweep = std::min(pi, static_cast<float>(heightAngleEnd - heightAngleStart));

  for (int w=0; w<widthResolution; w++) {
    for (int h=0; h<heightResolution; h++) {
      const float wRatio1 = (w/resFloatW);
      const float wRatio2 = ((w+1)/resFloatW);
      const float hRatio1 = (h/resFloatH);
      const float hRatio2 = ((h+1)/resFloatH);
      const float inc1 = wRatio1 * widthSweep + widthStart;
      const float inc2 = wRatio2 * widthSweep + widthStart;
      const float inc3 = hRatio1 * heightSweep + heightStart;
      const float inc4 = hRatio2 * heightSweep + heightStart;

      // compute first position
      const float x1 = std::sin(inc1);
      const float y1 = std::cos(inc1);
      const float z1 = std::sin(inc3);

      // compute second position
      const float x2 = std::sin(inc2);
      const float y2 = std::cos(inc2);
      const float z2 = std::sin(inc4);

      // compute radii at these positions
      const float r1 = std::cos(inc3);
      const float r2 = std::cos(inc4);

      // form triangles and compute their vertices
      const EigenTypes::Vector3f v1(r1*x1, z1, r1*y1); // a
      const EigenTypes::Vector3f v2(r1*x2, z1, r1*y2); // b
      const EigenTypes::Vector3f v3(r2*x2, z2, r2*y2); // c
      const EigenTypes::Vector3f v4(r2*x1, z2, r2*y1); // d

      auto SphereVertex = [](const EigenTypes::Vector3f &v, float texW, float texH) {
        return PrimitiveGeometryMesh::VertexAttributes(v,                                 // position
                                                       v.normalized(),                    // normal
                                                       EigenTypes::Vector2f(texW, texH),  // texture coordinate
                                                       EigenTypes::Vector4f(1, 1, 1, 1)); // color (opaque white)
      };
      mesh_assembler.PushTriangle(SphereVertex(v1, wRatio1, hRatio1), SphereVertex(v2, wRatio2, hRatio1), SphereVertex(v3, wRatio2, hRatio2));
      mesh_assembler.PushTriangle(SphereVertex(v1, wRatio1, hRatio1), SphereVertex(v3, wRatio2, hRatio2), SphereVertex(v4, wRatio1, hRatio2));
    }
  }
}

void PrimitiveGeometry::PushUnitCylinder(int radialResolution, int verticalResolution, PrimitiveGeometryMeshAssembler& mesh_assembler, float radiusBottom, float radiusTop, double angleStart, double angleEnd) {
  if (!mesh_assembler.IsInitialized()) {
    throw std::invalid_argument("Can't call PrimitiveGeometry::PushUnitSphere on a !IsInitialized() MeshAssembler.");
  }
  if (mesh_assembler.DrawMode() != GL_TRIANGLES) {
    throw std::invalid_argument("The PrimitiveGeometry::PushUnitCylinder function requires mesh_assembler.DrawMode() to be GL_TRIANGLES.");
  }

  const float radialRes = 1.0f / static_cast<float>(radialResolution);
  const float verticalRes = 1.0f / static_cast<float>(verticalResolution);
  
  const float twoPi = static_cast<float>(2.0 * M_PI);
  const float start = static_cast<float>(angleStart);
  const float sweep = std::min(twoPi, static_cast<float>(angleEnd - angleStart));

  for (int w=0; w<radialResolution; w++) {
    const float inc1 = w * radialRes * sweep + start;
    const float inc2 = (w+1) * radialRes * sweep + start;

    const float c1 = std::cos(inc1);
    const float c2 = std::cos(inc2);
    const float s1 = std::sin(inc1);
    const float s2 = std::sin(inc2);

    // vectors perpendicular from center axis to wall
    const EigenTypes::Vector3f p1(c1, 0, s1);
    const EigenTypes::Vector3f p2(c2, 0, s2);

    for (int h=0; h<verticalResolution; h++) {
      const float ratio1 = h*verticalRes;
      const float ratio2 = (h+1)*verticalRes;

      const float h1 = ratio1 - 0.5f;
      const float h2 = ratio2 - 0.5f;

      const float r1 = (1.0f-ratio1)*radiusBottom + ratio1*radiusTop;
      const float r2 = (1.0f-ratio2)*radiusBottom + ratio2*radiusTop;

      const EigenTypes::Vector3f v1(r1*c1, h1, r1*s1);
      const EigenTypes::Vector3f v2(r2*c1, h2, r2*s1);
      const EigenTypes::Vector3f v3(r1*c2, h1, r1*s2);
      const EigenTypes::Vector3f v4(r2*c2, h2, r2*s2);

      // tangents
      const EigenTypes::Vector3f t1 = v2 - v1;
      const EigenTypes::Vector3f t2 = v4 - v3;

      // binormals
      const EigenTypes::Vector3f b1 = (p1.cross(t1));
      const EigenTypes::Vector3f b2 = (p2.cross(t2));

      // normals
      const EigenTypes::Vector3f n1 = t1.cross(b1).normalized();
      const EigenTypes::Vector3f n2 = t2.cross(b2).normalized();

      // geom.PushTri(MakeVertexAttributes(v1, n1), MakeVertexAttributes(v2, n1), MakeVertexAttributes(v3, n2));
      // geom.PushTri(MakeVertexAttributes(v4, n2), MakeVertexAttributes(v3, n2), MakeVertexAttributes(v2, n1));
      auto CylinderVertex = [](const EigenTypes::Vector3f &v, const EigenTypes::Vector3f &n) {
        return PrimitiveGeometryMesh::VertexAttributes(v,                                 // position
                                                       n,                                 // normal
                                                       EigenTypes::Vector2f(0, 0),        // texture coordinate
                                                       EigenTypes::Vector4f(1, 1, 1, 1)); // color (opaque white)
      };
      mesh_assembler.PushTriangle(CylinderVertex(v1, n1), CylinderVertex(v2, n1), CylinderVertex(v3, n2));
      mesh_assembler.PushTriangle(CylinderVertex(v4, n2), CylinderVertex(v3, n2), CylinderVertex(v2, n1));
    }
  }
}

void PrimitiveGeometry::PushUnitSquare(PrimitiveGeometryMeshAssembler& mesh_assembler) {
  if (!mesh_assembler.IsInitialized()) {
    throw std::invalid_argument("Can't call PrimitiveGeometry::PushUnitSphere on a !IsInitialized() MeshAssembler.");
  }
  if (mesh_assembler.DrawMode() != GL_TRIANGLES) {
    throw std::invalid_argument("The PrimitiveGeometry::PushUnitSquare function requires mesh_assembler.DrawMode() to be GL_TRIANGLES.");
  }

  static const GLfloat X = 0.5f;
  static const EigenTypes::Vector3f POSITIONS[4] = {
    EigenTypes::Vector3f(-X, -X, 0),
    EigenTypes::Vector3f( X, -X, 0),
    EigenTypes::Vector3f( X,  X, 0),
    EigenTypes::Vector3f(-X,  X, 0)
  };
  static const EigenTypes::Vector2f TEX_COORDS[4] = {
    EigenTypes::Vector2f(0, 0),
    EigenTypes::Vector2f(1, 0),
    EigenTypes::Vector2f(1, 1),
    EigenTypes::Vector2f(0, 1)
  };
  
  // all vertices have the same normal
  const EigenTypes::Vector3f normal(EigenTypes::Vector3f::UnitZ());
  const EigenTypes::Vector4f color(EigenTypes::Vector4f::Constant(1.0f)); // opaque white

  mesh_assembler.PushTriangle(PrimitiveGeometryMesh::VertexAttributes(POSITIONS[0], normal, TEX_COORDS[0], color),
                              PrimitiveGeometryMesh::VertexAttributes(POSITIONS[1], normal, TEX_COORDS[1], color),
                              PrimitiveGeometryMesh::VertexAttributes(POSITIONS[2], normal, TEX_COORDS[2], color));
  mesh_assembler.PushTriangle(PrimitiveGeometryMesh::VertexAttributes(POSITIONS[0], normal, TEX_COORDS[0], color),
                              PrimitiveGeometryMesh::VertexAttributes(POSITIONS[2], normal, TEX_COORDS[2], color),
                              PrimitiveGeometryMesh::VertexAttributes(POSITIONS[3], normal, TEX_COORDS[3], color));
}

void PrimitiveGeometry::PushUnitDisk(size_t resolution, PrimitiveGeometryMeshAssembler& mesh_assembler) {
  if (!mesh_assembler.IsInitialized()) {
    throw std::invalid_argument("Can't call PrimitiveGeometry::PushUnitSphere on a !IsInitialized() MeshAssembler.");
  }
  if (mesh_assembler.DrawMode() != GL_TRIANGLES) {
    throw std::invalid_argument("The PrimitiveGeometry::PushUnitDisk function requires mesh_assembler.DrawMode() to be GL_TRIANGLES.");
  }

  auto UnitDiskVertex = [](const EigenTypes::Vector3f &p) {
    const EigenTypes::Vector3f normal(EigenTypes::Vector3f::UnitZ());
    const EigenTypes::Vector2f tex_coords(EigenTypes::Vector2f::Zero());
    const EigenTypes::Vector4f color(EigenTypes::Vector4f::Constant(1.0f)); // opaque white
    return PrimitiveGeometryMesh::VertexAttributes(p, normal, tex_coords, color);
  };

  const EigenTypes::Vector3f center(EigenTypes::Vector3f::Zero());

  const float resFloat = static_cast<float>(resolution);
  const float twoPi = static_cast<float>(2.0 * M_PI);

  for (size_t i=0; i<resolution; i++) {
    const float inc1 = (i/resFloat) * twoPi;
    const float inc2 = ((i+1)/resFloat) * twoPi;

    const float c1 = std::cos(inc1);
    const float c2 = std::cos(inc2);
    const float s1 = std::sin(inc1);
    const float s2 = std::sin(inc2);

    const EigenTypes::Vector3f p1(c1, s1, 0.0f);
    const EigenTypes::Vector3f p2(c2, s2, 0.0f);

    mesh_assembler.PushTriangle(UnitDiskVertex(center), UnitDiskVertex(p1), UnitDiskVertex(p2));
  }
}

void PrimitiveGeometry::PushUnitBox(PrimitiveGeometryMeshAssembler& mesh_assembler) {
  if (!mesh_assembler.IsInitialized()) {
    throw std::invalid_argument("Can't call PrimitiveGeometry::PushUnitSphere on a !IsInitialized() MeshAssembler.");
  }
  if (mesh_assembler.DrawMode() != GL_TRIANGLES) {
    throw std::invalid_argument("The PrimitiveGeometry::PushUnitBox function requires mesh_assembler.DrawMode() to be GL_TRIANGLES.");
  }

  auto PushUnitBoxQuad = [&mesh_assembler](const EigenTypes::Vector3f &p0,
                                 const EigenTypes::Vector3f &p1,
                                 const EigenTypes::Vector3f &p2,
                                 const EigenTypes::Vector3f &p3)
  {
    const EigenTypes::Vector3f normal((p2-p1).cross(p0-p1).normalized());
    const EigenTypes::Vector2f tex_coords(EigenTypes::Vector2f::Zero());
    const EigenTypes::Vector4f color(EigenTypes::Vector4f::Constant(1.0f)); // opaque white
    mesh_assembler.PushQuad(PrimitiveGeometryMesh::VertexAttributes(p0, normal, tex_coords, color),
                            PrimitiveGeometryMesh::VertexAttributes(p1, normal, tex_coords, color),
                            PrimitiveGeometryMesh::VertexAttributes(p2, normal, tex_coords, color),
                            PrimitiveGeometryMesh::VertexAttributes(p3, normal, tex_coords, color));
  };

  // In order for this to be a unit box, its side lengths must be unit.
  const float x = 0.5f;
  const EigenTypes::Vector3f p000(-x, -x, -x);
  const EigenTypes::Vector3f p001(-x, -x,  x);
  const EigenTypes::Vector3f p010(-x,  x, -x);
  const EigenTypes::Vector3f p011(-x,  x,  x);
  const EigenTypes::Vector3f p100( x, -x, -x);
  const EigenTypes::Vector3f p101( x, -x,  x);
  const EigenTypes::Vector3f p110( x,  x, -x);
  const EigenTypes::Vector3f p111( x,  x,  x);

  PushUnitBoxQuad(p010, p000, p001, p011);
  PushUnitBoxQuad(p100, p110, p111, p101);
  PushUnitBoxQuad(p000, p100, p101, p001);
  PushUnitBoxQuad(p110, p010, p011, p111);
  PushUnitBoxQuad(p010, p110, p100, p000);
  PushUnitBoxQuad(p001, p101, p111, p011);
}
