#pragma once

#include "ImagePassthrough.h"
#include "Primitives/RenderState.h"
#include "Primitives/Primitives.h"
#include "utility/EigenTypes.h"
#include "utility/Animation.h"
#include "utility/Utilities.h"

class HandInfo {
public:
  HandInfo();
  void Update(const Leap::Hand& hand, float deltaTime, const EigenTypes::Matrix3x3& rotation, const EigenTypes::Vector3& translation);
  void UpdateWithoutHand(float deltaTime);
  double GetLastUpdateTime() const { return m_lastUpdateTimeSeconds; }
  double GetConfidence() const { return m_confidence.Value(); }

  const Leap::Hand& GetLastSeenHand() const { return m_lastSeenHand; }

  void DrawSimpleHand(RenderState& renderer) const;
  void DrawCapsuleHand(RenderState& renderer, const EigenTypes::Matrix3x3& rotation, const EigenTypes::Vector3& translation, ImagePassthrough* passthrough) const;

  struct HandPoint {
    HandPoint() : point(EigenTypes::Vector3::Zero()), velocity(EigenTypes::Vector3::Zero()), isTip(false) {}
    void Update(const EigenTypes::Vector3& pos, float deltaTime) {
      velocity = (pos - point) / deltaTime;
      point = pos;
    }
    EigenTypes::Vector3 point;
    EigenTypes::Vector3 velocity;
    float radius;
    bool isTip;
    bool isExtended;
  };

  HandPoint& GetHandPoint(int idx) { return m_handPoints[idx]; }
  const HandPoint& GetHandPoint(int idx) const { return m_handPoints[idx]; }
  int NumExtendedFingers() const { return m_numExtendedFingers; }
  double CreationTimeSeconds() const { return m_creationTimeSeconds; }

  struct Intersection {
    Eigen::Vector3d point;
    double radius;
    double confidence;
    Eigen::Vector3d velocity;
  };

  typedef std::vector<Intersection, Eigen::aligned_allocator<Intersection>> IntersectionVector;

  IntersectionVector IntersectRectangle(const RectanglePrim& prim) const;
  IntersectionVector IntersectDisk(const Disk& prim) const;

  static const int BONES_PER_FINGER = static_cast<int>(Leap::Bone::TYPE_DISTAL) + 1;
  static const int POINTS_PER_FINGER = BONES_PER_FINGER + 1;
  static const int NUM_HAND_POINTS = POINTS_PER_FINGER * 5;
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
private:

  HandPoint m_handPoints[NUM_HAND_POINTS];
  double m_creationTimeSeconds;
  double m_lastUpdateTimeSeconds;

  Smoothed<double> m_confidence;
  int m_numExtendedFingers;

  Leap::Hand m_lastSeenHand;
  mutable bool m_needRiggedHandUpdate;
  bool m_firstUpdate;

  mutable std::shared_ptr<CapsulePrim> m_capsulePrim;
  mutable std::shared_ptr<RadialPolygonPrim> m_palmPrim;
  mutable std::shared_ptr<RadialPolygonPrim> m_armPrim;

};

typedef std::map<int, std::shared_ptr<HandInfo>> HandInfoMap;
