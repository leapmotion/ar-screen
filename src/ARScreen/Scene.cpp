#include "stdafx.h"
#include "Scene.h"
#include "Leap/GL/Projection.h"

void Scene::Init() {
  m_InputRotation = EigenTypes::Matrix3x3::Identity();
  m_InputTranslation = EigenTypes::Vector3::Zero();

  m_ImagePassthrough = std::shared_ptr<ImagePassthrough>(new ImagePassthrough());
  m_ImagePassthrough->Init();

  m_Font = std::shared_ptr<TextureFont>(new TextureFont(100.0f, "fonts/Roboto-Regular.ttf", 1024, 1024));
  m_Font->Load();

  m_Text = std::shared_ptr<TextPrimitive>(new TextPrimitive());
  m_Text->SetText(L" ", m_Font);
}

void Scene::SetInputTransform(const EigenTypes::Matrix3x3& rotation, const EigenTypes::Vector3& translation) {
  m_InputRotation = rotation;
  m_InputTranslation = translation;
}

void Scene::Update(const std::deque<Leap::Frame>& frames) {
  for (size_t i = 0; i < frames.size(); i++) {
    Leap::Frame prevFrame = m_CurFrame;
    const double prevTimeSeconds = timestampToSeconds(prevFrame.timestamp());
    m_CurFrame = frames[i];
    const double curTimeSeconds = timestampToSeconds(m_CurFrame.timestamp());
    const float leapDeltaTime = static_cast<float>(curTimeSeconds - prevTimeSeconds);
    if (leapDeltaTime < 0.00001f) {
      continue;
    }

    updateTrackedHands(leapDeltaTime);
    leapInteract(leapDeltaTime);
  }

  if (!frames.empty()) {
    m_ImagePassthrough->Update(frames.back().images());
  }

  const std::string timeStr = getTimeString(-7);
  const std::wstring timeStrW(timeStr.begin(), timeStr.end());
  if (timeStrW != m_ClockString) {
    m_Text->SetText(timeStrW, m_Font);
    m_ClockString = timeStrW;
  }
}

void Scene::Render(const Eigen::Matrix4f& proj, const Eigen::Matrix4f& view, int eyeIdx) const {
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  m_Renderer.ProjectionMatrix() = proj.cast<double>();

  m_Renderer.GetModelView().Matrix().setIdentity();
  glEnable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);
  m_ImagePassthrough->SetActiveTexture(eyeIdx);
  m_ImagePassthrough->SetUseStencil(false);
  m_ImagePassthrough->Draw(m_Renderer);
  glEnable(GL_DEPTH_TEST);

  m_Renderer.GetModelView().Matrix() = view.cast<double>();

  // draw scene here
  PrimitiveBase::DrawSceneGraph(*m_Text, m_Renderer);
  m_Text->Translation().x() = -0.5f*m_Text->Size().x();

  m_Renderer.GetModelView().Matrix().setIdentity();
  glDisable(GL_DEPTH_TEST);
  m_ImagePassthrough->SetUseStencil(true);
  m_ImagePassthrough->Draw(m_Renderer);
  glEnable(GL_DEPTH_TEST);

  m_Renderer.GetModelView().Matrix() = view.cast<double>();
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_DEPTH_TEST);

  glDepthMask(GL_FALSE);
  drawHands();
  glDepthMask(GL_TRUE);
}

void Scene::updateTrackedHands(float deltaTime) {
  const double curTimeSeconds = timestampToSeconds(m_CurFrame.timestamp());

  // update
  const Leap::HandList hands = m_CurFrame.hands();
  for (int i = 0; i<hands.count(); i++) {
    const bool curLeft = hands[i].isLeft();
    const int id = hands[i].id();
    const auto itr = m_TrackedHands.find(id);
    if (itr == m_TrackedHands.end()) {
      m_TrackedHands[id] = std::shared_ptr<HandInfo>(new HandInfo());
    }
    m_TrackedHands[id]->Update(hands[i], deltaTime, m_InputRotation, m_InputTranslation);
  }

  // update hands that weren't matched this frame
  for (auto& element : m_TrackedHands) {
    HandInfo& trackedHand = *element.second;
    if (trackedHand.GetLastUpdateTime() != curTimeSeconds) {
      trackedHand.UpdateWithoutHand(deltaTime);
    }
  }

  // clean up
  static const float MIN_HAND_INFO_AGE = 0.5f;
  static const float MAX_HAND_INFO_AGE = 0.5f; // seconds since last update until hand info gets cleaned up
  static const double MIN_CONFIDENCE = 0.01;
  auto it = m_TrackedHands.begin();
  while (it != m_TrackedHands.end()) {
    HandInfo& cur = *(it->second);
    const float curAge = std::fabs(static_cast<float>(curTimeSeconds - cur.GetLastUpdateTime()));
    const float creationDiff = static_cast<float>(curTimeSeconds - cur.CreationTimeSeconds());
    const bool tooOld = curAge > MAX_HAND_INFO_AGE;
    const bool tooNew = creationDiff < MIN_HAND_INFO_AGE;
    const bool tooLow = cur.GetConfidence() < MIN_CONFIDENCE;
    if (tooOld || (tooLow && !tooNew)) {
      m_TrackedHands.erase(it++);
    } else {
      ++it;
    }
  }
}

void Scene::leapInteract(float deltaTime) {
}

void Scene::drawHands() const {
  for (const auto& element : m_TrackedHands) {
    const HandInfo& trackedHand = *element.second;
    trackedHand.DrawCapsuleHand(m_Renderer, m_InputRotation, m_InputTranslation, m_ImagePassthrough.get());
  }
}
