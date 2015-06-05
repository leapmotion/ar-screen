#include "stdafx.h"
#include "Primitives.h"

#include <cassert>
#include "Leap/GL/Texture2.h"

void GenericShape::DrawContents(RenderState& renderState) const {
  const Leap::GL::Shader &shader = Shader();
  auto locations = std::make_tuple(shader.LocationOfAttribute("position"),
                                   shader.LocationOfAttribute("normal"),
                                   shader.LocationOfAttribute("tex_coord"),
                                   shader.LocationOfAttribute("color"));
  m_mesh.Bind(locations);
  m_mesh.Draw();
  m_mesh.Unbind(locations);
}

Sphere::Sphere() : m_Radius(1) { }

void Sphere::MakeAdditionalModelViewTransformations (Leap::GL::ModelView &model_view) const {
  model_view.Scale(EigenTypes::Vector3::Constant(m_Radius));
}

void Sphere::DrawContents(RenderState& renderState) const {
  static PrimitiveGeometryMesh mesh;
  if (!mesh.IsInitialized()) {
    PrimitiveGeometryMeshAssembler mesh_assembler(GL_TRIANGLES);
    PrimitiveGeometry::PushUnitSphere(96, 48, mesh_assembler);
    mesh_assembler.InitializeMesh(mesh);
    assert(mesh.IsInitialized());
  }
  const Leap::GL::Shader &shader = Shader();
  auto locations = std::make_tuple(shader.LocationOfAttribute("position"),
                                   shader.LocationOfAttribute("normal"),
                                   shader.LocationOfAttribute("tex_coord"),
                                   shader.LocationOfAttribute("color"));
  mesh.Bind(locations);
  mesh.Draw();
  mesh.Unbind(locations);
}

Cylinder::Cylinder() : m_Radius(1), m_Height(1) { }

void Cylinder::MakeAdditionalModelViewTransformations (Leap::GL::ModelView &model_view) const {
  model_view.Scale(EigenTypes::Vector3(m_Radius, m_Height, m_Radius));
}

void Cylinder::DrawContents(RenderState& renderState) const {
  static PrimitiveGeometryMesh mesh;
  if (!mesh.IsInitialized()) {
    PrimitiveGeometryMeshAssembler mesh_assembler(GL_TRIANGLES);
    PrimitiveGeometry::PushUnitCylinder(50, 1, mesh_assembler);
    mesh_assembler.InitializeMesh(mesh);
    assert(mesh.IsInitialized());
  }
  const Leap::GL::Shader &shader = Shader();
  auto locations = std::make_tuple(shader.LocationOfAttribute("position"),
                                   shader.LocationOfAttribute("normal"),
                                   shader.LocationOfAttribute("tex_coord"),
                                   shader.LocationOfAttribute("color"));
  mesh.Bind(locations);
  mesh.Draw();
  mesh.Unbind(locations);
}

Box::Box() : m_Size(EigenTypes::Vector3::Constant(1.0)) { }

void Box::MakeAdditionalModelViewTransformations (Leap::GL::ModelView &model_view) const {
  model_view.Scale(m_Size);
}

void Box::DrawContents(RenderState& renderState) const {
  static PrimitiveGeometryMesh mesh;
  if (!mesh.IsInitialized()) {
    PrimitiveGeometryMeshAssembler mesh_assembler(GL_TRIANGLES);
    PrimitiveGeometry::PushUnitBox(mesh_assembler);
    mesh_assembler.InitializeMesh(mesh);
    assert(mesh.IsInitialized());
  }
  const Leap::GL::Shader &shader = Shader();
  auto locations = std::make_tuple(shader.LocationOfAttribute("position"),
                                   shader.LocationOfAttribute("normal"),
                                   shader.LocationOfAttribute("tex_coord"),
                                   shader.LocationOfAttribute("color"));
  mesh.Bind(locations);
  mesh.Draw();
  mesh.Unbind(locations);
}

Disk::Disk() : m_Radius(1) { }

void Disk::MakeAdditionalModelViewTransformations (Leap::GL::ModelView &model_view) const {
  model_view.Scale(EigenTypes::Vector3::Constant(m_Radius));
}

void Disk::DrawContents(RenderState& renderState) const {
  static PrimitiveGeometryMesh mesh;
  if (!mesh.IsInitialized()) {
    PrimitiveGeometryMeshAssembler mesh_assembler(GL_TRIANGLES);
    PrimitiveGeometry::PushUnitDisk(75, mesh_assembler);
    mesh_assembler.InitializeMesh(mesh);
    assert(mesh.IsInitialized());
  }
  const Leap::GL::Shader &shader = Shader();
  auto locations = std::make_tuple(shader.LocationOfAttribute("position"),
                                   shader.LocationOfAttribute("normal"),
                                   shader.LocationOfAttribute("tex_coord"),
                                   shader.LocationOfAttribute("color"));
  mesh.Bind(locations);
  mesh.Draw();
  mesh.Unbind(locations);
}

RectanglePrim::RectanglePrim() : m_Size(1, 1) { }

void RectanglePrim::MakeAdditionalModelViewTransformations (Leap::GL::ModelView &model_view) const {
  model_view.Scale(EigenTypes::Vector3(m_Size.x(), m_Size.y(), 1.0));
}

void RectanglePrim::DrawContents(RenderState& renderState) const {
  static PrimitiveGeometryMesh mesh;
  if (!mesh.IsInitialized()) {
    PrimitiveGeometryMeshAssembler mesh_assembler(GL_TRIANGLES);
    PrimitiveGeometry::PushUnitSquare(mesh_assembler);
    mesh_assembler.InitializeMesh(mesh);
    assert(mesh.IsInitialized());
  }

  bool useTexture = bool(m_texture); // If there is a valid texture, enable texturing.
  if (useTexture) {
    glEnable(GL_TEXTURE_2D);
    m_texture->Bind();
  }
  {
    const Leap::GL::Shader &shader = Shader();
    auto locations = std::make_tuple(shader.LocationOfAttribute("position"),
                                     shader.LocationOfAttribute("normal"),
                                     shader.LocationOfAttribute("tex_coord"),
                                     shader.LocationOfAttribute("color"));
    mesh.Bind(locations);
    mesh.Draw();
    mesh.Unbind(locations);
  }
  if (useTexture) {
    glDisable(GL_TEXTURE_2D);
    m_texture->Unbind();
  }
}

ImagePrimitive::ImagePrimitive(void) :
  ImagePrimitive(nullptr)
{}

ImagePrimitive::ImagePrimitive(const std::shared_ptr<Leap::GL::Texture2> &texture) {
  SetTexture(texture);
  SetScaleBasedOnTextureSize();
  Material().Uniform<AMBIENT_LIGHTING_PROPORTION>() = 1.0f;
  Material().Uniform<TEXTURE_MAPPING_ENABLED>() = true;
}

void ImagePrimitive::SetScaleBasedOnTextureSize () {
  if(Texture())
    SetSize(EigenTypes::Vector2(Texture()->Params().Width(), Texture()->Params().Height()));
}

PartialDisk::PartialDisk() : m_RecomputeMesh(true), m_InnerRadius(0.5), m_OuterRadius(1), m_StartAngle(0), m_EndAngle(2*M_PI) { }

void PartialDisk::DrawContents(RenderState& renderState) const {
  if (m_InnerRadius >= m_OuterRadius || m_StartAngle >= m_EndAngle) {
    // don't proceed if the shape is empty
    return;
  }

  if (m_RecomputeMesh) {
    RecomputeMesh();
  }

  const Leap::GL::Shader &shader = Shader();
  auto locations = std::make_tuple(shader.LocationOfAttribute("position"),
                                   shader.LocationOfAttribute("normal"),
                                   shader.LocationOfAttribute("tex_coord"),
                                   shader.LocationOfAttribute("color"));
  m_mesh.Bind(locations);
  m_mesh.Draw();
  m_mesh.Unbind(locations);
}

void PartialDisk::RecomputeMesh() const {
  double sweepAngle = m_EndAngle - m_StartAngle;
  if (sweepAngle > 2*M_PI) {
    sweepAngle = 2*M_PI;
  }

  static const double DESIRED_ANGLE_PER_SEGMENT = 0.1; // radians
  const int numSegments = static_cast<int>(sweepAngle / DESIRED_ANGLE_PER_SEGMENT) + 1;
  const double anglePerSegment = sweepAngle / numSegments;

  m_mesh.Shutdown();
  PrimitiveGeometryMeshAssembler mesh_assembler(GL_TRIANGLES);

  auto PartialDiskVertex = [](const EigenTypes::Vector3f &p) {
    const EigenTypes::Vector3f normal(EigenTypes::Vector3f::UnitZ());
    const EigenTypes::Vector2f tex_coords(EigenTypes::Vector2f::Zero());
    const EigenTypes::Vector4f color(EigenTypes::Vector4f::Constant(1.0f)); // opaque white
    return PrimitiveGeometryMesh::VertexAttributes(p, normal, tex_coords, color);
  };

  double curAngle = m_StartAngle;
  const double cosStart = std::cos(m_StartAngle);
  const double sinStart = std::sin(m_StartAngle);
  EigenTypes::Vector3f prevInner(static_cast<float>(m_InnerRadius*cosStart), static_cast<float>(m_InnerRadius*sinStart), 0.0f);
  EigenTypes::Vector3f prevOuter(static_cast<float>(m_OuterRadius*cosStart), static_cast<float>(m_OuterRadius*sinStart), 0.0f);
  for (int i=0; i<numSegments; i++) {
    curAngle += anglePerSegment;

    const double cosCur = std::cos(curAngle);
    const double sinCur = std::sin(curAngle);

    const EigenTypes::Vector3f curInner(static_cast<float>(m_InnerRadius*cosCur), static_cast<float>(m_InnerRadius*sinCur), 0.0f);
    const EigenTypes::Vector3f curOuter(static_cast<float>(m_OuterRadius*cosCur), static_cast<float>(m_OuterRadius*sinCur), 0.0f);

    mesh_assembler.PushTriangle(PartialDiskVertex(prevInner), PartialDiskVertex(prevOuter), PartialDiskVertex(curOuter));
    mesh_assembler.PushTriangle(PartialDiskVertex(curOuter), PartialDiskVertex(curInner), PartialDiskVertex(prevInner));

    prevInner = curInner;
    prevOuter = curOuter;
  }

  mesh_assembler.InitializeMesh(m_mesh);
  assert(m_mesh.IsInitialized());
  m_RecomputeMesh = false;
}

PartialDiskWithTriangle::PartialDiskWithTriangle()
  :
  m_TriangleSide(OUTSIDE),
  m_TrianglePosition(0.5),
  m_TriangleWidth(0.1),
  m_TriangleOffset(0.35)
{ }

void PartialDiskWithTriangle::RecomputeMesh() const {
  double sweepAngle = m_EndAngle - m_StartAngle;
  if (sweepAngle > 2*M_PI) {
    sweepAngle = 2*M_PI;
  }

  static const double DESIRED_ANGLE_PER_SEGMENT = 0.1; // radians
  int numSegments = static_cast<int>(sweepAngle / DESIRED_ANGLE_PER_SEGMENT) + 1;
  const double anglePerSegment = sweepAngle / numSegments;

  m_mesh.Shutdown();
  PrimitiveGeometryMeshAssembler mesh_assembler(GL_TRIANGLES);

  auto PartialDiskVertex = [](const EigenTypes::Vector3f &p) {
    const EigenTypes::Vector3f normal(EigenTypes::Vector3f::UnitZ());
    const EigenTypes::Vector2f tex_coords(EigenTypes::Vector2f::Zero());
    const EigenTypes::Vector4f color(EigenTypes::Vector4f::Constant(1.0f)); // opaque white
    return PrimitiveGeometryMesh::VertexAttributes(p, normal, tex_coords, color);
  };

  double curAngle = m_StartAngle;
  const double cosStart = std::cos(m_StartAngle);
  const double sinStart = std::sin(m_StartAngle);
  EigenTypes::Vector3f prevInner(static_cast<float>(m_InnerRadius*cosStart), static_cast<float>(m_InnerRadius*sinStart), 0.0f);
  EigenTypes::Vector3f prevOuter(static_cast<float>(m_OuterRadius*cosStart), static_cast<float>(m_OuterRadius*sinStart), 0.0f);

  bool haveStarted = false;
  bool havePassedMidpoint = false;
  bool havePassedEnd = false;
  bool haveTakenCareOfExtraAngle = false;

  const double triangleAngle = sweepAngle * m_TriangleWidth;
  const double triangleStart = m_TrianglePosition * sweepAngle + m_StartAngle - triangleAngle / 2.0;
  const double triangleEnd = triangleStart + triangleAngle;
  const double triangleMidpoint = 0.5*(triangleStart + triangleEnd);

  while (curAngle < (m_EndAngle - 0.001)) {
    curAngle += anglePerSegment;

    if (!haveStarted && curAngle > triangleStart) {
      curAngle = triangleStart;
      haveStarted = true;
    } else if (!havePassedMidpoint && curAngle > triangleMidpoint) {
      curAngle = triangleMidpoint;
      havePassedMidpoint = true;
    } else if (!havePassedEnd && curAngle > triangleEnd) {
      curAngle = triangleEnd;
      havePassedEnd = true;
    } else if (havePassedEnd && !haveTakenCareOfExtraAngle) {
      haveTakenCareOfExtraAngle = true;
      curAngle = m_StartAngle + anglePerSegment * (static_cast<int>((curAngle-m_StartAngle) / anglePerSegment));
    }

    double innerRadius = m_InnerRadius;
    double outerRadius = m_OuterRadius;
    if (curAngle >= triangleStart && curAngle <= triangleEnd) {
      double ratio = (curAngle - triangleStart) / (triangleAngle);
      double mult = -2 * std::abs(ratio-0.5) + 1;
      const double triangleHeight = m_TriangleOffset * (m_OuterRadius - m_InnerRadius);
      if (m_TriangleSide == INSIDE) {
        innerRadius -= mult * triangleHeight;
      } else if (m_TriangleSide == OUTSIDE) {
        outerRadius += mult * triangleHeight;
      }
    }

    const double cosCur = std::cos(curAngle);
    const double sinCur = std::sin(curAngle);

    const EigenTypes::Vector3f curInner(static_cast<float>(innerRadius*cosCur), static_cast<float>(innerRadius*sinCur), 0.0f);
    const EigenTypes::Vector3f curOuter(static_cast<float>(outerRadius*cosCur), static_cast<float>(outerRadius*sinCur), 0.0f);

    mesh_assembler.PushTriangle(PartialDiskVertex(prevInner), PartialDiskVertex(prevOuter), PartialDiskVertex(curOuter));
    mesh_assembler.PushTriangle(PartialDiskVertex(curOuter), PartialDiskVertex(curInner), PartialDiskVertex(prevInner));

    prevInner = curInner;
    prevOuter = curOuter;
  }

  mesh_assembler.InitializeMesh(m_mesh);
  assert(m_mesh.IsInitialized());
  m_RecomputeMesh = false;
}

PartialSphere::PartialSphere() : m_RecomputeMesh(true), m_Radius(1), m_StartWidthAngle(0), m_EndWidthAngle(M_PI), m_StartHeightAngle(0), m_EndHeightAngle(M_PI) { }

void PartialSphere::MakeAdditionalModelViewTransformations(Leap::GL::ModelView &model_view) const {
  model_view.Scale(EigenTypes::Vector3::Constant(m_Radius));
}

void PartialSphere::DrawContents(RenderState& renderState) const {
  if (m_StartWidthAngle >= m_EndWidthAngle || m_StartHeightAngle >= m_EndHeightAngle) {
    // don't proceed if the shape is empty
    return;
  }

  if (m_RecomputeMesh) {
    RecomputeMesh();
  }

  const Leap::GL::Shader &shader = Shader();
  auto locations = std::make_tuple(shader.LocationOfAttribute("position"),
                                   shader.LocationOfAttribute("normal"),
                                   shader.LocationOfAttribute("tex_coord"),
                                   shader.LocationOfAttribute("color"));
  m_mesh.Bind(locations);
  m_mesh.Draw();
  m_mesh.Unbind(locations);
}

void PartialSphere::RecomputeMesh() const {
  static const double DESIRED_ANGLE_PER_SEGMENT = 0.1; // radians
  const double heightSweep = std::min(M_PI, m_EndHeightAngle - m_StartHeightAngle);
  const double widthSweep = std::min(2.0 * M_PI, m_EndWidthAngle - m_StartWidthAngle);
  const int numWidth = static_cast<int>(widthSweep / DESIRED_ANGLE_PER_SEGMENT) + 1;
  const int numHeight = static_cast<int>(heightSweep / DESIRED_ANGLE_PER_SEGMENT) + 1;
  m_mesh.Shutdown();
  PrimitiveGeometryMeshAssembler mesh_assembler(GL_TRIANGLES);
  PrimitiveGeometry::PushUnitSphere(numWidth, numHeight, mesh_assembler, m_StartHeightAngle, m_EndHeightAngle, m_StartWidthAngle, m_EndWidthAngle);
  mesh_assembler.InitializeMesh(m_mesh);
  assert(m_mesh.IsInitialized());
  m_RecomputeMesh = false;
}

CapsulePrim::CapsulePrim() : m_Radius(1), m_Height(1) { }

void CapsulePrim::DrawContents(RenderState& renderState) const {
  static bool loaded = false;
  static PrimitiveGeometryMesh cap;
  static PrimitiveGeometryMesh body;
  if (!loaded) {
    PrimitiveGeometryMeshAssembler mesh_assembler(GL_TRIANGLES);
    PrimitiveGeometry::PushUnitSphere(24, 12, mesh_assembler, -M_PI/2.0, 0);
    mesh_assembler.InitializeMesh(cap);

    mesh_assembler.Initialize(GL_TRIANGLES); // This calls Shutdown first
    PrimitiveGeometry::PushUnitCylinder(24, 1, mesh_assembler);
    mesh_assembler.InitializeMesh(body);
    loaded = true;
  }

  Leap::GL::ModelView& modelView = renderState.GetModelView();

  const Leap::GL::Shader &shader = Shader();
  auto locations = std::make_tuple(shader.LocationOfAttribute("position"),
                                   shader.LocationOfAttribute("normal"),
                                   shader.LocationOfAttribute("tex_coord"),
                                   shader.LocationOfAttribute("color"));

  // draw body
  modelView.Push();
  modelView.Scale(EigenTypes::Vector3(m_Radius, m_Height, m_Radius));
  ManuallySetMatricesAndUploadMatrixUniforms(modelView.Matrix(), renderState.ProjectionMatrix());
  body.Bind(locations);
  body.Draw();
  body.Unbind(locations);
  modelView.Pop();

  // draw first end cap
  modelView.Push();
  modelView.Translate(EigenTypes::Vector3(0, -m_Height/2.0, 0));
  modelView.Scale(EigenTypes::Vector3::Constant(m_Radius));
  ManuallySetMatricesAndUploadMatrixUniforms(modelView.Matrix(), renderState.ProjectionMatrix());
  cap.Bind(locations);
  cap.Draw();
  cap.Unbind(locations);
  modelView.Pop();

  // draw second end cap
  modelView.Push();
  modelView.Translate(EigenTypes::Vector3(0, m_Height/2.0, 0));
  modelView.Scale(EigenTypes::Vector3(m_Radius, -m_Radius, m_Radius));
  ManuallySetMatricesAndUploadMatrixUniforms(modelView.Matrix(), renderState.ProjectionMatrix());
  cap.Bind(locations);
  cap.Draw();
  cap.Unbind(locations);
  modelView.Pop();
}

BiCapsulePrim::BiCapsulePrim() : m_RecomputeMesh(true), m_Radius1(1), m_Radius2(1), m_Height(1) { }

void BiCapsulePrim::DrawContents(RenderState& renderState) const {
  if (m_RecomputeMesh) {
    RecomputeMesh();
  }

  Leap::GL::ModelView& modelView = renderState.GetModelView();

  const Leap::GL::Shader &shader = Shader();
  auto locations = std::make_tuple(shader.LocationOfAttribute("position"),
                                   shader.LocationOfAttribute("normal"),
                                   shader.LocationOfAttribute("tex_coord"),
                                   shader.LocationOfAttribute("color"));

  // draw body
  double bodyHeight = m_Height + (m_BodyOffset2 - m_BodyOffset1);
  modelView.Push();
  modelView.Translate(EigenTypes::Vector3(0, (m_BodyOffset1+m_BodyOffset2)/2.0, 0));
  modelView.Scale(EigenTypes::Vector3(1.0, bodyHeight, 1.0));
  ManuallySetMatricesAndUploadMatrixUniforms(modelView.Matrix(), renderState.ProjectionMatrix());
  m_Body.Bind(locations);
  m_Body.Draw();
  m_Body.Unbind(locations);
  modelView.Pop();

  // draw first end cap
  modelView.Push();
  modelView.Translate(EigenTypes::Vector3(0, -m_Height/2.0, 0));
  modelView.Scale(EigenTypes::Vector3::Constant(m_Radius1));
  ManuallySetMatricesAndUploadMatrixUniforms(modelView.Matrix(), renderState.ProjectionMatrix());
  m_Cap1.Bind(locations);
  m_Cap1.Draw();
  m_Cap1.Unbind(locations);
  modelView.Pop();

  // draw second end cap
  modelView.Push();
  modelView.Translate(EigenTypes::Vector3(0, m_Height/2.0, 0));
  modelView.Scale(EigenTypes::Vector3(m_Radius2, -m_Radius2, m_Radius2));
  ManuallySetMatricesAndUploadMatrixUniforms(modelView.Matrix(), renderState.ProjectionMatrix());
  m_Cap2.Bind(locations);
  m_Cap2.Draw();
  m_Cap2.Unbind(locations);
  modelView.Pop();
}

void BiCapsulePrim::RecomputeMesh() const {
  const double sideAngle = M_PI/2.0 - std::acos((m_Radius1 - m_Radius2)/m_Height);

  const double sinSideAngle = std::sin(sideAngle);
  const double cosSideAngle = std::cos(sideAngle);
  m_BodyOffset1 = sinSideAngle * m_Radius1;
  m_BodyOffset2 = sinSideAngle * m_Radius2;
  m_BodyRadius1 = cosSideAngle * m_Radius1;
  m_BodyRadius2 = cosSideAngle * m_Radius2;

  m_Cap1.Shutdown();
  m_Cap2.Shutdown();
  m_Body.Shutdown();

  PrimitiveGeometryMeshAssembler mesh_assembler(GL_TRIANGLES);
  PrimitiveGeometry::PushUnitSphere(24, 12, mesh_assembler, -M_PI/2.0, sideAngle);
  mesh_assembler.InitializeMesh(m_Cap1);
  assert(m_Cap1.IsInitialized());

  mesh_assembler.Initialize(GL_TRIANGLES); // This calls Shutdown first.
  PrimitiveGeometry::PushUnitSphere(24, 12, mesh_assembler, -M_PI/2.0, -sideAngle);
  mesh_assembler.InitializeMesh(m_Cap2);
  assert(m_Cap2.IsInitialized());

  mesh_assembler.Initialize(GL_TRIANGLES); // This calls Shutdown first.
  PrimitiveGeometry::PushUnitCylinder(24, 1, mesh_assembler, static_cast<float>(m_BodyRadius1), static_cast<float>(m_BodyRadius2));
  mesh_assembler.InitializeMesh(m_Body);
  assert(m_Body.IsInitialized());

  m_RecomputeMesh = false;
}

PartialCylinder::PartialCylinder() : m_RecomputeMesh(true), m_Radius(1), m_Height(1), m_StartAngle(0), m_EndAngle(2.0*M_PI) { }

void PartialCylinder::MakeAdditionalModelViewTransformations(Leap::GL::ModelView &model_view) const {
  model_view.Scale(EigenTypes::Vector3(m_Radius, m_Height, m_Radius));
}

void PartialCylinder::DrawContents(RenderState& renderState) const {
  if (m_StartAngle >= m_EndAngle) {
    // don't proceed if the shape is empty
    return;
  }

  if (m_RecomputeMesh) {
    RecomputeMesh();
  }

  const Leap::GL::Shader &shader = Shader();
  auto locations = std::make_tuple(shader.LocationOfAttribute("position"),
                                   shader.LocationOfAttribute("normal"),
                                   shader.LocationOfAttribute("tex_coord"),
                                   shader.LocationOfAttribute("color"));
  m_mesh.Bind(locations);
  m_mesh.Draw();
  m_mesh.Unbind(locations);
}

void PartialCylinder::RecomputeMesh() const {
  m_mesh.Shutdown();
  PrimitiveGeometryMeshAssembler mesh_assembler(GL_TRIANGLES);
  PrimitiveGeometry::PushUnitCylinder(30, 1, mesh_assembler, 1.0f, 1.0f, m_StartAngle, m_EndAngle);
  mesh_assembler.InitializeMesh(m_mesh);
  assert(m_mesh.IsInitialized());
  m_RecomputeMesh = false;
}

RadialPolygonPrim::RadialPolygonPrim() : m_RecomputeMesh(true), m_Radius(1) { }

void RadialPolygonPrim::DrawContents(RenderState& renderState) const {
  if (m_RecomputeMesh) {
    RecomputeMesh();
  }

  Leap::GL::ModelView& modelView = renderState.GetModelView();

  const Leap::GL::Shader &shader = Shader();
  auto locations = std::make_tuple(shader.LocationOfAttribute("position"),
                                   shader.LocationOfAttribute("normal"),
                                   shader.LocationOfAttribute("tex_coord"),
                                   shader.LocationOfAttribute("color"));

  // draw top polygon face
  modelView.Push();
  modelView.Translate(EigenTypes::Vector3(0, m_Radius, 0));
  ManuallySetMatricesAndUploadMatrixUniforms(modelView.Matrix(), renderState.ProjectionMatrix());
  // m_Polygon.Draw(Shader(), GL_TRIANGLES);
  m_Polygon.Bind(locations);
  m_Polygon.Draw();
  m_Polygon.Unbind(locations);
  modelView.Pop();

  // draw bottom polygon face
  modelView.Push();
  modelView.Scale(EigenTypes::Vector3(1, -1, 1));
  modelView.Translate(EigenTypes::Vector3(0, m_Radius, 0));
  ManuallySetMatricesAndUploadMatrixUniforms(modelView.Matrix(), renderState.ProjectionMatrix());
  m_Polygon.Bind(locations);
  m_Polygon.Draw();
  m_Polygon.Unbind(locations);
  modelView.Pop();

  // draw each side consisting of a partial sphere and half-cylinder
  for (size_t i=0; i<m_Sides.size(); i++) {
    modelView.Push();
    modelView.Translate(m_Sides[i].m_Origin);
    modelView.Multiply(m_Sides[i].m_SphereBasis);
    modelView.Scale(EigenTypes::Vector3::Constant(m_Radius));
    ManuallySetMatricesAndUploadMatrixUniforms(modelView.Matrix(), renderState.ProjectionMatrix());
    m_Sides[i].m_SphereJoint.Bind(locations);
    m_Sides[i].m_SphereJoint.Draw();
    m_Sides[i].m_SphereJoint.Unbind(locations);
    modelView.Pop();

    modelView.Push();
    modelView.Translate(m_Sides[i].m_Origin);
    modelView.Multiply(m_Sides[i].m_CylinderBasis);
    modelView.Scale(EigenTypes::Vector3(m_Radius, m_Sides[i].m_Length, m_Radius));
    modelView.Translate(EigenTypes::Vector3(0, 0.5, 0));
    ManuallySetMatricesAndUploadMatrixUniforms(modelView.Matrix(), renderState.ProjectionMatrix());
    m_CylinderBody.Bind(locations);
    m_CylinderBody.Draw();
    m_CylinderBody.Unbind(locations);
    modelView.Pop();
  }
}

void RadialPolygonPrim::RecomputeMesh() const {
  // assumptions: 
  // - average of all polygon vertices lies within the shape
  // - the angle between any two sides is no greater than 180 degrees

  static const double EPSILON = 1E-7;
  const int numPoints = static_cast<int>(m_Sides.size());
  assert(numPoints > 0);
  if (numPoints <= 0) {
    return;
  }



  // compute the centroid of the polygon
  EigenTypes::Vector3 avgPoint(EigenTypes::Vector3::Zero());
  for (int i=0; i<numPoints; i++) {
    avgPoint += m_Sides[i].m_Origin;
  }
  avgPoint /= numPoints;

  // normal of the main face
  const EigenTypes::Vector3 normal = EigenTypes::Vector3::UnitY();

  auto PushPolygonTriangle = [](PrimitiveGeometryMeshAssembler &mesh_assembler,
                                const EigenTypes::Vector3f &p0,
                                const EigenTypes::Vector3f &p1,
                                const EigenTypes::Vector3f &p2)
  {
    const EigenTypes::Vector3f normal((p2-p1).cross(p0-p1).normalized());
    const EigenTypes::Vector2f tex_coords(EigenTypes::Vector2f::Zero());
    const EigenTypes::Vector4f color(EigenTypes::Vector4f::Constant(1.0f)); // opaque white
    mesh_assembler.PushTriangle(PrimitiveGeometryMesh::VertexAttributes(p0, normal, tex_coords, color),
                                PrimitiveGeometryMesh::VertexAttributes(p1, normal, tex_coords, color),
                                PrimitiveGeometryMesh::VertexAttributes(p2, normal, tex_coords, color));
  };

  m_Polygon.Shutdown();
  PrimitiveGeometryMeshAssembler mesh_assembler(GL_TRIANGLES);
  PrimitiveGeometryMeshAssembler sphere_joint_mesh_assembler;
  for (int i=0; i<numPoints; i++) {
    // retrieve the two polygon sides meeting at this point
    const EigenTypes::Vector3& curPoint = m_Sides[i].m_Origin;
    const EigenTypes::Vector3& prevPoint = m_Sides[i == 0 ? numPoints-1 : i-1].m_Origin;
    const EigenTypes::Vector3& nextPoint = m_Sides[(i+1)%numPoints].m_Origin;
    const EigenTypes::Vector3 vec1 = (curPoint - prevPoint);
    const EigenTypes::Vector3 vec2 = (nextPoint - curPoint);

    // length of the side used for the partial cylinder
    m_Sides[i].m_Length = vec1.norm();

    // calculate angle at this vertex of the polygon
    const double denom = vec1.squaredNorm() * vec2.squaredNorm();
    double angle = 0;
    if (denom > EPSILON) {
      const double val = vec1.dot(vec2) / std::sqrt(denom);
      if (val <= -1.0) {
        angle = M_PI;
      } else {
        angle = std::acos(val);
      }
    }

    // if this vertex is locally concave, the partial sphere is hidden inside the shape
    if (vec1.cross(vec2).y() < 0) {
      angle = 0;
    }
    
    // create partial sphere to join the two cylindrical sides
    static const double DESIRED_ANGLE_PER_SEGMENT = 0.1; // radians
    const int numWidth = static_cast<int>(angle / DESIRED_ANGLE_PER_SEGMENT) + 1;
    m_Sides[i].m_SphereJoint.Shutdown();
    sphere_joint_mesh_assembler.Initialize(GL_TRIANGLES);
    PrimitiveGeometry::PushUnitSphere(numWidth, 16, sphere_joint_mesh_assembler, -M_PI/2.0, M_PI/2.0, 0, angle);
    sphere_joint_mesh_assembler.InitializeMesh(m_Sides[i].m_SphereJoint);
    assert(m_Sides[i].m_SphereJoint.IsInitialized());
    sphere_joint_mesh_assembler.Shutdown();

    // compute bases of partial sphere and partial cylinder at this vertex
    const EigenTypes::Vector3 tangent = vec1.normalized();
    const EigenTypes::Vector3 binormal = tangent.cross(normal);
    EigenTypes::Matrix3x3& sphereBasis = m_Sides[i].m_SphereBasis;
    sphereBasis.col(0) = tangent;
    sphereBasis.col(1) = normal;
    sphereBasis.col(2) = binormal;
    EigenTypes::Matrix3x3& cylinderBasis = m_Sides[i].m_CylinderBasis;
    cylinderBasis.col(0) = normal;
    cylinderBasis.col(1) = -tangent;
    cylinderBasis.col(2) = binormal;

    // add the vertex to the main face of the polygon
    const EigenTypes::Vector3f p0 = nextPoint.cast<float>();
    const EigenTypes::Vector3f p1 = avgPoint.cast<float>();
    const EigenTypes::Vector3f p2 = curPoint.cast<float>();
    PushPolygonTriangle(mesh_assembler, p0, p1, p2);
  }

  mesh_assembler.InitializeMesh(m_Polygon);
  assert(m_Polygon.IsInitialized());

  // create a single unit half-cylinder to be reused for all sides
  m_CylinderBody.Shutdown();
  mesh_assembler.Initialize(GL_TRIANGLES);
  PrimitiveGeometry::PushUnitCylinder(16, 1, mesh_assembler, 1.0f, 1.0f, 0.0, M_PI);
  mesh_assembler.InitializeMesh(m_CylinderBody);
  assert(m_CylinderBody.IsInitialized());

  m_RecomputeMesh = false;
}
