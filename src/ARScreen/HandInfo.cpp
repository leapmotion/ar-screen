#include "stdafx.h"
#include "HandInfo.h"

HandInfo::HandInfo() :
  m_lastUpdateTimeSeconds(0.0),
  m_confidence(0.0),
  m_needRiggedHandUpdate(true),
  m_creationTimeSeconds(0.0),
  m_firstUpdate(true),
  m_numExtendedFingers(0)
{
  m_capsulePrim = std::shared_ptr<CapsulePrim>(new CapsulePrim());
  //m_capsulePrim->Material().Uniform<AMBIENT_LIGHTING_PROPORTION>() = 1.0f;

  m_palmPrim = std::shared_ptr<RadialPolygonPrim>(new RadialPolygonPrim());
  //m_palmPrim->Material().Uniform<AMBIENT_LIGHTING_PROPORTION>() = 1.0f;

  m_armPrim = std::shared_ptr<RadialPolygonPrim>(new RadialPolygonPrim());
  //m_armPrim->Material().Uniform<AMBIENT_LIGHTING_PROPORTION>() = 1.0f;
}

void HandInfo::Update(const Leap::Hand& hand, float deltaTime, const EigenTypes::Matrix3x3& rotation, const EigenTypes::Vector3& translation) {
  assert(hand.isValid());

  const double curTimeSeconds = timestampToSeconds(hand.frame().timestamp());

  if (m_firstUpdate) {
    m_creationTimeSeconds = curTimeSeconds;
    m_firstUpdate = false;
  }

  const EigenTypes::Vector3 palmPosition = rotation * hand.palmPosition().toVector3<EigenTypes::Vector3>() + translation;
  const float falloffMult = 1.0f;

  const float timeVisibleMult = SmootherStep(std::min(1.0f, 6.0f*static_cast<float>(curTimeSeconds - m_creationTimeSeconds)));
  const float confidenceMult = SmootherStep(std::min(1.0f, 2.0f * hand.confidence() * hand.confidence()));
  m_confidence.SetSmoothStrength(0.5f);
  m_confidence.SetGoal(timeVisibleMult * confidenceMult * falloffMult);
  m_confidence.Update(deltaTime);

  int pointIdx = 0;

  const float scale = static_cast<float>(rotation.col(0).norm());

  const Leap::FingerList fingers = hand.fingers();
  m_numExtendedFingers = 0;
  for (int j = 0; j < fingers.count(); j++) {
    for (int k = 0; k < BONES_PER_FINGER; k++) {
      const Leap::Bone::Type type = static_cast<Leap::Bone::Type>(k);
      const Leap::Bone bone = fingers[j].bone(type);
      if (type == Leap::Bone::TYPE_METACARPAL) {
        const EigenTypes::Vector3 prevPos = rotation * bone.prevJoint().toVector3<EigenTypes::Vector3>() + translation;
        m_handPoints[pointIdx].Update(prevPos, deltaTime);
        m_handPoints[pointIdx].radius = scale*0.5f*fingers[j].width();
        pointIdx++;
      }

      const EigenTypes::Vector3 nextPos = rotation * bone.nextJoint().toVector3<EigenTypes::Vector3>() + translation;
      m_handPoints[pointIdx].Update(nextPos, deltaTime);
      m_handPoints[pointIdx].isTip = k == (BONES_PER_FINGER - 1);
      m_handPoints[pointIdx].isExtended = fingers[j].isExtended();
      m_handPoints[pointIdx].radius = scale*0.5f*fingers[j].width();
      if (m_handPoints[pointIdx].isTip) {
        m_handPoints[pointIdx].radius *= 0.25f;
      }
      pointIdx++;
    }
    if (fingers[j].isExtended()) {
      m_numExtendedFingers++;
    }
  }

  m_lastSeenHand = hand;

  m_lastUpdateTimeSeconds = curTimeSeconds;

  m_needRiggedHandUpdate = true;
}

void HandInfo::UpdateWithoutHand(float deltaTime) {
  m_confidence.SetSmoothStrength(0.8f);
  m_confidence.SetGoal(0.0);
  m_confidence.Update(deltaTime);
  m_numExtendedFingers = 0;

  for (int i = 0; i < NUM_HAND_POINTS; i++) {
    m_handPoints[i].velocity.setZero();
  }
}

void HandInfo::DrawCapsuleHand(RenderState& renderer, const EigenTypes::Matrix3x3& rotation, const EigenTypes::Vector3& translation, ImagePassthrough* passthrough) const {
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  const float viewX = static_cast<float>(viewport[0]);
  const float viewWidth = static_cast<float>(viewport[2]);
  const float viewHeight = static_cast<float>(viewport[3]);

  const EigenTypes::Matrix4x4& mat = renderer.ProjectionMatrix();
  const float l00 = static_cast<float>(mat(0, 0));
  const float l11 = static_cast<float>(mat(1, 1));
  const float l03 = static_cast<float>(mat(0, 2));

  const Leap::Hand& hand = m_lastSeenHand;

  const double radiusMult = 1.5;

  const float opacity = static_cast<float>(m_confidence.Value());

  const EigenTypes::Matrix3x3 armBasis = rotation * toEigen(hand.arm().basis());
  const EigenTypes::Matrix3x3 handBasis = rotation * toEigen(hand.basis());
  const EigenTypes::Vector3 palmPosition = rotation * hand.palmPosition().toVector3<EigenTypes::Vector3>() + translation;

  const EigenTypes::Matrix3x3 basisRot = RotationMatrixFromEulerAngles(M_PI / 2.0, 0.0, M_PI);
  const Leap::FingerList fingers = hand.fingers();
  for (int i = 0; i<5; i++) {
    const Leap::Finger finger = fingers[i];
    for (int j = 1; j<4; j++) {
      const Leap::Bone bone = finger.bone(static_cast<Leap::Bone::Type>(j));
      const EigenTypes::Matrix3x3 boneBasis = rotation * toEigen(bone.basis());
      m_capsulePrim->Translation() = rotation * bone.center().toVector3<EigenTypes::Vector3>() + translation;
      m_capsulePrim->SetHeight(bone.length());
      m_capsulePrim->SetRadius(radiusMult*0.5*bone.width());
      m_capsulePrim->LinearTransformation() = boneBasis * basisRot;
      passthrough->DrawStencilObject(m_capsulePrim.get(), renderer, viewWidth, viewX, viewHeight, l00, l11, l03, opacity);
    }
  }

  {
    const double palmRadius = 15;
    const int numPalmPoints = 4;
    m_palmPrim->SetNumSides(numPalmPoints);
    const double halfPalmWidth = std::max(0.1, 0.5*hand.palmWidth() - palmRadius);
    const double halfPalmHeight = 1.2*halfPalmWidth;
    const EigenTypes::Vector2 palmPoints[] ={
      { -halfPalmWidth, -halfPalmHeight },
      { -halfPalmWidth, halfPalmHeight },
      { halfPalmWidth, halfPalmHeight },
      { halfPalmWidth, -halfPalmHeight }
    };
    const EigenTypes::Vector3 palmOffset(2.0, 0.0, 14.0);
    for (int i = 0; i < numPalmPoints; i++) {
      m_palmPrim->SetPoint(i, palmPoints[i]);
    }
    m_palmPrim->SetRadius(radiusMult*palmRadius);
    m_palmPrim->Translation() = palmPosition + handBasis *palmOffset;
    m_palmPrim->LinearTransformation() = handBasis;
    passthrough->DrawStencilObject(m_palmPrim.get(), renderer, viewWidth, viewX, viewHeight, l00, l11, l03, opacity);
  }

  {
    const double armRadius = 22.0;
    const Leap::Arm arm = hand.arm();
    const EigenTypes::Vector3 armCenter = arm.center().toVector3<EigenTypes::Vector3>();
    const EigenTypes::Vector3 elbow = arm.elbowPosition().toVector3<EigenTypes::Vector3>();
    const double halfArmWidth = std::max(0.1, 0.5*arm.width() - armRadius);
    const double halfArmHeight = (armCenter - elbow).norm() - armRadius;
    const int numArmPoints = 4;
    m_armPrim->SetNumSides(numArmPoints);
    const EigenTypes::Vector2 armPoints[] ={
      { -halfArmWidth, -halfArmHeight },
      { -halfArmWidth, halfArmHeight },
      { halfArmWidth, halfArmHeight },
      { halfArmWidth, -halfArmHeight }
    };
    for (int i = 0; i < numArmPoints; i++) {
      m_armPrim->SetPoint(i, armPoints[i]);
    }
    m_armPrim->SetRadius(radiusMult*armRadius);
    m_armPrim->Translation() = rotation * armCenter + translation;
    m_armPrim->LinearTransformation() = armBasis;
    passthrough->DrawStencilObject(m_armPrim.get(), renderer, viewWidth, viewX, viewHeight, l00, l11, l03, opacity);
  }
}

HandInfo::IntersectionVector HandInfo::IntersectRectangle(const RectanglePrim& prim) const {
  const Eigen::Vector3d center = prim.Translation();
  const Eigen::Matrix3d linear = prim.LinearTransformation();
  const Eigen::Vector3d normal = prim.LinearTransformation().col(2).normalized();
  const Eigen::Vector3d scale(linear.row(0).norm(), linear.row(1).norm(), linear.row(2).norm());
  IntersectionVector intersections;

  Intersection intersection;
  const double confidence = GetConfidence();
  int pointIdx = 0;
  for (int i=0; i<5; i++) {
    for (int j=0; j<HandInfo::BONES_PER_FINGER; j++) {
      const auto& point1 = GetHandPoint(pointIdx);
      const auto& point2 = GetHandPoint(pointIdx+1);

      const bool dot1Sign = (point1.point - center).dot(normal) > 0;
      const bool dot2Sign = (point2.point - center).dot(normal) > 0;

      if (dot1Sign != dot2Sign) {
        const Eigen::Vector3d diff = (point2.point - point1.point);
        const double distBetweenPoints = diff.norm();
        const Eigen::Vector3d dir = diff/distBetweenPoints;

        double t = DBL_MAX;
        if (IntersectPlane(point1.point, dir, prim.Translation(), normal, t)) {
          const Eigen::Vector3d surfacePoint = point1.point + t * dir;

          const Eigen::Vector3d untransformed = linear.inverse() * (surfacePoint - center);
          if (std::fabs(untransformed.x()) < 0.5*prim.Size().x() && std::fabs(untransformed.y()) < 0.5*prim.Size().y()) {
            const double ratio = t / distBetweenPoints; 
            intersection.radius = (1.0-ratio)*point1.radius + ratio*point2.radius;
            intersection.velocity = (1.0-ratio)*point1.velocity + ratio*point2.velocity;
            intersection.point = surfacePoint + normal;
            intersection.confidence = confidence;
            intersections.push_back(intersection);
          }
        }
      }

      pointIdx++;
    }
    pointIdx++;
  }

  return intersections;
}
