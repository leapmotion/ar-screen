#pragma once

#include <algorithm>
#include "utility/EigenTypes.h"
#include "Leap/GL/ModelView.h"
#include "Leap/GL/Projection.h"
#include <memory>

// This class is a package for data necessary for rendering.
class RenderState {
public:

  RenderState () {
    // This should be the same as the default projection matrix in old versions of OpenGL.
    Leap::GL::Projection::SetOrthographic(m_ProjectionMatrix, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
  }

  const EigenTypes::Matrix4x4 &ProjectionMatrix () const { return m_ProjectionMatrix; }
  EigenTypes::Matrix4x4 &ProjectionMatrix () { return m_ProjectionMatrix; }

  // getters for modelview and projection
  const Leap::GL::ModelView& GetModelView() const { return m_ModelView; }
  Leap::GL::ModelView& GetModelView() { return m_ModelView; }

private:

  EigenTypes::Matrix4x4 m_ProjectionMatrix;
  Leap::GL::ModelView m_ModelView;
};
