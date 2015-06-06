#include "stdafx.h"
#include "Scene.h"
#include "Leap/GL/Projection.h"
#include "WindowManager.h"
#include "Globals.h"

void Scene::Init() {
  m_InputRotation = EigenTypes::Matrix3x3::Identity();
  m_InputTranslation = EigenTypes::Vector3::Zero();

  m_ImagePassthrough = std::shared_ptr<ImagePassthrough>(new ImagePassthrough());
  m_ImagePassthrough->Init();

  m_Font = std::shared_ptr<TextureFont>(new TextureFont(100.0f, "Roboto-Regular.ttf", 1024, 1024));
  m_Font->Load();

  m_Text = std::shared_ptr<TextPrimitive>(new TextPrimitive());
  m_Text->SetText(L" ", m_Font);
  createUI();
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

  Autowired<WindowManager> manager;
  if (manager) {
    for (const auto& it : manager->m_Windows) {
      std::cout << "draw" << std::endl;
      PrimitiveBase::DrawSceneGraph(*it.second->m_Texture, m_Renderer);
    }
  }
  drawUI();

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

void Scene::createUI() {
  m_IconDisk = std::shared_ptr<Disk>(new Disk);
  m_IconPrimitive = std::shared_ptr<ImagePrimitive>(new ImagePrimitive());
  m_ExpandedPrimitive = std::shared_ptr<ImagePrimitive>(new ImagePrimitive());

  m_CalendarExpanded = GLTexture2ImageRef(new GLTexture2Image());
  m_CalendarExpanded->LoadPath("calendar-expand.png");

  m_CalendarIcon = GLTexture2ImageRef(new GLTexture2Image());
  m_CalendarIcon->LoadPath("calendar.png");

  m_EmailIcon = GLTexture2ImageRef(new GLTexture2Image());
  m_EmailIcon->LoadPath("email.png");

  m_PhoneIcon = GLTexture2ImageRef(new GLTexture2Image());
  m_PhoneIcon->LoadPath("phone.png");

  m_RecordIcon = GLTexture2ImageRef(new GLTexture2Image());
  m_RecordIcon->LoadPath("screen-record.png");

  m_TextsIcon = GLTexture2ImageRef(new GLTexture2Image());
  m_TextsIcon->LoadPath("texts.png");

  m_IconDisk->Material().Uniform<AMBIENT_LIGHTING_PROPORTION>() = 1.0f;
  m_IconDisk->SetRadius(20);

  m_IconDisk->AddChild(m_IconPrimitive);
  m_IconPrimitive->Translation() << 0, 0, 5.0;
}

void Scene::drawUI() const {
  const Leap::GL::Rgba<uint8_t> calendarColor(139, 138, 251);
  const Leap::GL::Rgba<uint8_t> emailColor(211, 107, 202);
  const Leap::GL::Rgba<uint8_t> phoneColor(87, 208, 193);
  const Leap::GL::Rgba<uint8_t> recordColor(65, 174, 229);
  const Leap::GL::Rgba<uint8_t> textColor(251, 55, 104);

  const double radius = m_IconDisk->Radius();
  const double spacing = 2.25 * radius;
  double curY = 150;
  double curX = 150;
  double curZ = -50;

  {
    m_IconDisk->Material().Uniform<AMBIENT_LIGHT_COLOR>() = calendarColor;
    m_IconPrimitive->SetTexture(m_CalendarIcon->GetTexture());
    m_IconPrimitive->SetScaleBasedOnTextureSize();
    const double scale = 1.5 * m_IconDisk->Radius() / m_IconPrimitive->Size().norm();
    m_IconDisk->Translation() << curX, curY, curZ;
    m_IconPrimitive->LinearTransformation() = scale * Eigen::Matrix3d::Identity();
    PrimitiveBase::DrawSceneGraph(*m_IconDisk, m_Renderer);
    curY -= spacing;
  }

  {
    m_IconDisk->Material().Uniform<AMBIENT_LIGHT_COLOR>() = emailColor;
    m_IconPrimitive->SetTexture(m_EmailIcon->GetTexture());
    m_IconPrimitive->SetScaleBasedOnTextureSize();
    const double scale = 1.5 * m_IconDisk->Radius() / m_IconPrimitive->Size().norm();
    m_IconDisk->Translation() << curX, curY, curZ;
    m_IconPrimitive->LinearTransformation() = scale * Eigen::Matrix3d::Identity();
    PrimitiveBase::DrawSceneGraph(*m_IconDisk, m_Renderer);
    curY -= spacing;
  }

  {
    m_IconDisk->Material().Uniform<AMBIENT_LIGHT_COLOR>() = phoneColor;
    m_IconPrimitive->SetTexture(m_PhoneIcon->GetTexture());
    m_IconPrimitive->SetScaleBasedOnTextureSize();
    const double scale = 1.5 * m_IconDisk->Radius() / m_IconPrimitive->Size().norm();
    m_IconDisk->Translation() << curX, curY, curZ;
    m_IconPrimitive->LinearTransformation() = scale * Eigen::Matrix3d::Identity();
    PrimitiveBase::DrawSceneGraph(*m_IconDisk, m_Renderer);
    curY -= spacing;
  }

  {
    m_IconDisk->Material().Uniform<AMBIENT_LIGHT_COLOR>() = recordColor;
    m_IconPrimitive->SetTexture(m_RecordIcon->GetTexture());
    m_IconPrimitive->SetScaleBasedOnTextureSize();
    const double scale = 1.5 * m_IconDisk->Radius() / m_IconPrimitive->Size().norm();
    m_IconDisk->Translation() << curX, curY, curZ;
    m_IconPrimitive->LinearTransformation() = scale * Eigen::Matrix3d::Identity();
    PrimitiveBase::DrawSceneGraph(*m_IconDisk, m_Renderer);
    curY -= spacing;
  }

  {
    m_IconDisk->Material().Uniform<AMBIENT_LIGHT_COLOR>() = textColor;
    m_IconPrimitive->SetTexture(m_TextsIcon->GetTexture());
    m_IconPrimitive->SetScaleBasedOnTextureSize();
    const double scale = 1.5 * m_IconDisk->Radius() / m_IconPrimitive->Size().norm();
    m_IconDisk->Translation() << curX, curY, curZ;
    m_IconPrimitive->LinearTransformation() = scale * Eigen::Matrix3d::Identity();
    PrimitiveBase::DrawSceneGraph(*m_IconDisk, m_Renderer);
    curY -= spacing;
  }

  {
    const double size = 4 * spacing;
    m_ExpandedPrimitive->SetTexture(m_CalendarExpanded->GetTexture());
    m_ExpandedPrimitive->SetScaleBasedOnTextureSize();
    const double scale = (size + 2*radius) / m_ExpandedPrimitive->Size().y();
    m_ExpandedPrimitive->Translation() << 300, 150 - size/2.0, curZ;
    m_ExpandedPrimitive->LinearTransformation() = scale * Eigen::Matrix3d::Identity();
    PrimitiveBase::DrawSceneGraph(*m_ExpandedPrimitive, m_Renderer);
  }
}
