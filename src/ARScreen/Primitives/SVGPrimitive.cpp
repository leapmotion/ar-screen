#include "SVGPrimitive.h"

#include <Eigen/StdVector>
#define NANOSVG_ALL_COLOR_KEYWORDS  // Include full list of color keywords.
#define NANOSVG_IMPLEMENTATION
#include <nanosvg.h>
#include <polypartition.h>

#include <cfloat>
#include <cassert>

class Curve {
  public:
    struct Bezier {
      EigenTypes::Vector2f b[4];
    };

    Curve(float tolerance = 1.0f);
    ~Curve();

    void Append(const Bezier& bezier);

    const std::vector<TPPLPoint>& Points() const { return m_points; }

  private:
    void Subdivide(const Bezier& bezier, Bezier& left, Bezier& right);
    bool IsSufficientlyFlat(const Bezier& bezier);
    float m_tolerance;

    std::vector<TPPLPoint> m_points;
};

Curve::Curve(float tolerance) : m_tolerance(16.0f*tolerance*tolerance) // 16*tolerance^2
{
}

Curve::~Curve()
{
}

void Curve::Append(const Bezier& bezier) {
  if (IsSufficientlyFlat(bezier)) {
    if (m_points.empty()) {
      TPPLPoint point;
      point.x = bezier.b[0].x();
      point.y = bezier.b[0].y();
      m_points.emplace_back(std::move(point));
    }
    TPPLPoint point;
    point.x = bezier.b[3].x();
    point.y = bezier.b[3].y();
    const auto dx = point.x - m_points[0].x;
    if (std::abs(dx) < FLT_EPSILON) {
      const auto dy = point.y - m_points[0].y;
      if (std::abs(dy) < FLT_EPSILON) {
        return;
      }
    }
    m_points.emplace_back(std::move(point));
  } else {
    Bezier left, right;
    Subdivide(bezier, left, right);
    Append(left);
    Append(right);
  }
}

void Curve::Subdivide(const Bezier& bezier, Bezier& left, Bezier& right) {
  const EigenTypes::Vector2f middle = 0.5f*(bezier.b[1] + bezier.b[2]);
  left.b[0] = bezier.b[0];
  left.b[1] = 0.5f*(bezier.b[0] + bezier.b[1]);
  left.b[2] = 0.5f*(left.b[1] + middle);
  right.b[3] = bezier.b[3];
  right.b[2] = 0.5f*(bezier.b[2] + bezier.b[3]);
  right.b[1] = 0.5f*(middle + right.b[2]);
  left.b[3] = right.b[0] = 0.5f*(left.b[2] + right.b[1]);
}

bool Curve::IsSufficientlyFlat(const Bezier& bezier) {
  EigenTypes::Vector2f u = 3.0f*bezier.b[1] - 2.0f*bezier.b[0] - bezier.b[3];
  EigenTypes::Vector2f v = 3.0f*bezier.b[2] - 2.0f*bezier.b[3] - bezier.b[0];
  return (u.cwiseProduct(u).cwiseMax(v.cwiseProduct(v)).sum() < m_tolerance);
}

SVGPrimitive::SVGPrimitive(const std::string& svg) :
  m_Image(nullptr),
  m_RecomputeMesh(false)
{
  m_Origin << 0.0, 0.0;
  m_Size << 0.0, 0.0;
  if (!svg.empty()) {
    Set(svg);
  }
}

SVGPrimitive::~SVGPrimitive()
{
  if (m_Image) {
    nsvgDelete(m_Image);
  }
}

void SVGPrimitive::DrawContents(RenderState& renderState) const {
  if (m_RecomputeMesh) {
    const_cast<SVGPrimitive*>(this)->RecomputeChildren(); // This object's children need to be recomputed
  }
}

void SVGPrimitive::Set(const std::string& svg)
{
  if (m_Image) {
    nsvgDelete(m_Image);
    m_Image = nullptr;
    Children().clear();
  }
  std::string svgCopy{svg}; // Make a copy so that nanosvg can modify its contents (horrors)
  m_Image = nsvgParse(const_cast<char*>(svgCopy.c_str()), "px", 96.0f);
  if (m_Image) {
    float bounds[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    bool isFirst = true;

    for (NSVGshape* shape = m_Image->shapes; shape != NULL; shape = shape->next) {
      if (isFirst) {
        isFirst = false;
        bounds[0] = shape->bounds[0];
        bounds[1] = shape->bounds[1];
        bounds[2] = shape->bounds[2];
        bounds[3] = shape->bounds[3];
      } else {
        if (shape->bounds[0] < bounds[0]) { bounds[0] = shape->bounds[0]; }
        if (shape->bounds[1] < bounds[1]) { bounds[1] = shape->bounds[1]; }
        if (shape->bounds[2] > bounds[2]) { bounds[2] = shape->bounds[2]; }
        if (shape->bounds[3] > bounds[3]) { bounds[3] = shape->bounds[3]; }
      }
    }
    m_Origin << static_cast<EigenTypes::MATH_TYPE>(bounds[0]), static_cast<EigenTypes::MATH_TYPE>(bounds[1]);
    m_Size << static_cast<EigenTypes::MATH_TYPE>(bounds[2] - bounds[0]), static_cast<EigenTypes::MATH_TYPE>(bounds[3] - bounds[1]);
    m_RecomputeMesh = true;
  }
}

void SVGPrimitive::RecomputeChildren() {
  static const EigenTypes::Vector3f NORMAL(EigenTypes::Vector3f::UnitZ());
  static const EigenTypes::Vector2f TEX_COORD(0, 0);   // tex coords not used
  static const EigenTypes::Vector4f COLOR(1, 1, 1, 1); // opaque white

  m_RecomputeMesh = false;
  if (m_Image) {
    Children().clear();
    for (NSVGshape* shape = m_Image->shapes; shape != NULL; shape = shape->next) {
      const uint32_t fillColor = shape->fill.color;
      const uint32_t strokeColor = shape->stroke.color;
      const float opacity = shape->opacity;
      const float strokeWidth = shape->strokeWidth;
      const bool doFill = (fillColor & 0xFF000000) != 0 && shape->fill.type == NSVG_PAINT_COLOR;
      const bool doStroke = (strokeColor & 0xFF000000) != 0 && strokeWidth > FLT_EPSILON &&
                            (shape->fill.type == NSVG_PAINT_COLOR || shape->fill.type == NSVG_PAINT_NONE);

      if ((!doFill && !doStroke) || opacity <= FLT_EPSILON) {
        continue; // Nothing to do...
      }
      std::list<TPPLPoly> polys;
      std::vector<std::shared_ptr<GenericShape>> strokes;

      for (NSVGpath* path = shape->paths; path != NULL; path = path->next) {
        Curve curve(0.5f);
        for (int i = 0; i < path->npts-1; i += 3) {
          const float* p = &path->pts[i*2];
          Curve::Bezier bezier;
          bezier.b[0] << p[0], p[1];
          bezier.b[1] << p[2], p[3];
          bezier.b[2] << p[4], p[5];
          bezier.b[3] << p[6], p[7];
          curve.Append(bezier);
        }
        const auto& points = curve.Points();
        if (!points.empty()) {
          if (doFill) {
            TPPLPoly poly;
            size_t numPoints = points.size();

            poly.Init(static_cast<long>(numPoints));
            for (size_t i = 0; i < numPoints; i++) {
              poly[static_cast<int>(i)] = points[i];
            }
            // We are assuming that ONLY the last path is not a hole
            if (path->next) {
              poly.SetHole(true);
              poly.SetOrientation(TPPL_CW);
            } else {
              poly.SetHole(false);
              poly.SetOrientation(TPPL_CCW);
            }
            polys.emplace_back(std::move(poly));
          }
        }
        if (doStroke) {
          const bool isClosed = path->closed != '\0';
          auto genericShape = std::shared_ptr<GenericShape>(new GenericShape());
          PrimitiveGeometryMeshAssembler mesh_assembler(isClosed ? GL_LINE_LOOP : GL_LINE_STRIP);
          // We don't yet handle stroke widths. For now, simulate a stroke width less than 1 by adjusting the alpha
          const float simulatedStrokeWidth = strokeWidth >= 1.0f ? 1.0f : strokeWidth;
          const float alpha = static_cast<float>((strokeColor >> 24) & 0xFF)/255.0f;
          const float blue  = static_cast<float>((strokeColor >> 16) & 0xFF)/255.0f;
          const float green = static_cast<float>((strokeColor >>  8) & 0xFF)/255.0f;
          const float red   = static_cast<float>( strokeColor        & 0xFF)/255.0f;
          const Leap::GL::Rgba<float> color(red, green, blue, alpha*opacity*simulatedStrokeWidth);
          genericShape->Material().Uniform<AMBIENT_LIGHT_COLOR>() = color;
          genericShape->Material().Uniform<AMBIENT_LIGHTING_PROPORTION>() = 1.0f;
          const auto& points = curve.Points();
          for (const auto& pt : points) {
            const EigenTypes::Vector3f point(static_cast<float>(pt.x), static_cast<float>(pt.y), 0.0f);
            // The arguments to PrimitiveGeometryMesh::VertexAttributes must be actual vector
            // types, and not Eigen expression templates (e.g. EigenTypes::Vector3f::UnitZ()).
            mesh_assembler.PushVertex(point, NORMAL, TEX_COORD, COLOR);
          }
          mesh_assembler.InitializeMesh(genericShape->Mesh());
          assert(genericShape->Mesh().IsInitialized());
          // Gather the strokes; they will be applied after the fill
          strokes.push_back(genericShape);
        }
      }
      // Add the fill (if applicable)
      if (!polys.empty()) {
        TPPLPartition partition;
        std::list<TPPLPoly> triangles;

        if (!partition.Triangulate_EC(&polys, &triangles)) {
          continue; // Failed to triangulate!
        }
        auto genericShape = std::shared_ptr<GenericShape>(new GenericShape());
        PrimitiveGeometryMeshAssembler mesh_assembler(GL_TRIANGLES);

        const float alpha = static_cast<float>((fillColor >> 24) & 0xFF)/255.0f;
        const float blue  = static_cast<float>((fillColor >> 16) & 0xFF)/255.0f;
        const float green = static_cast<float>((fillColor >>  8) & 0xFF)/255.0f;
        const float red   = static_cast<float>( fillColor        & 0xFF)/255.0f;
        const Leap::GL::Rgba<float> color(red, green, blue, alpha*opacity);
        genericShape->Material().Uniform<AMBIENT_LIGHT_COLOR>() = color;
        genericShape->Material().Uniform<AMBIENT_LIGHTING_PROPORTION>() = 1.0f;
        for (auto& triangle : triangles) {
          assert(triangle.GetNumPoints() == 3);
          const EigenTypes::Vector3f point1(static_cast<float>(triangle[0].x), static_cast<float>(triangle[0].y), 0.0f);
          const EigenTypes::Vector3f point2(static_cast<float>(triangle[1].x), static_cast<float>(triangle[1].y), 0.0f);
          const EigenTypes::Vector3f point3(static_cast<float>(triangle[2].x), static_cast<float>(triangle[2].y), 0.0f);
          mesh_assembler.PushTriangle(PrimitiveGeometryMesh::VertexAttributes(point1, NORMAL, TEX_COORD, COLOR),
                                      PrimitiveGeometryMesh::VertexAttributes(point2, NORMAL, TEX_COORD, COLOR), 
                                      PrimitiveGeometryMesh::VertexAttributes(point3, NORMAL, TEX_COORD, COLOR));
        }
        mesh_assembler.InitializeMesh(genericShape->Mesh());
        assert(genericShape->Mesh().IsInitialized());
        AddChild(genericShape);
      }
      // Add any strokes after the fill
      if (!strokes.empty()) {
        for (auto& stroke : strokes) {
          AddChild(stroke);
        }
      }
    }
  }
}
