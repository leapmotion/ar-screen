#include "stdafx.h"
#include "Scene.h"
#include "Leap/GL/Projection.h"
#include "WindowManager.h"
#include "Globals.h"

Scene::Scene() : m_ScreenPositionSmoother(Eigen::Vector3d::Zero()), m_ScreenRotationSmoother(Eigen::Matrix3d::Identity())
{
  m_ScreenPositionSmoother.SetSmoothStrength(0.9f);
  m_ScreenRotationSmoother.SetSmoothStrength(0.9f);
}

void Scene::Init() {
  m_InputRotation = EigenTypes::Matrix3x3::Identity();
  m_InputTranslation = EigenTypes::Vector3::Zero();

  m_ImagePassthrough = std::shared_ptr<ImagePassthrough>(new ImagePassthrough());
  m_ImagePassthrough->Init();

  m_Font = std::shared_ptr<TextureFont>(new TextureFont(100.0f, "Roboto-Regular.ttf", 1024, 1024));
  m_Font->Load();

  m_Text = std::shared_ptr<TextPrimitive>(new TextPrimitive());
  m_Text->SetText(L" ", m_Font);

  m_MouseSphere = std::shared_ptr<Sphere>(new Sphere());
  m_MouseSphere->Material().Uniform<AMBIENT_LIGHTING_PROPORTION>() = 1.0f;

  m_IntersectionDisk = std::shared_ptr<Disk>(new Disk());
  m_IntersectionDisk->Material().Uniform<AMBIENT_LIGHTING_PROPORTION>() = 1.0f;

  createUI();
}

void Scene::SetInputTransform(const EigenTypes::Matrix3x3& rotation, const EigenTypes::Vector3& translation) {
  m_InputRotation = rotation;
  m_InputTranslation = translation;
}

void Scene::Update(const std::deque<Leap::Frame>& frames) {
  for (size_t i = 0; i < frames.size(); i++) {
    m_PrevFrame = m_CurFrame;
    const double prevTimeSeconds = timestampToSeconds(m_PrevFrame.timestamp());
    m_CurFrame = frames[i];
    const double curTimeSeconds = timestampToSeconds(m_CurFrame.timestamp());
    const float leapDeltaTime = static_cast<float>(curTimeSeconds - prevTimeSeconds);
    if (leapDeltaTime < 0.00001f) {
      continue;
    }

    updateTrackedHands(leapDeltaTime);

    const double scale = m_InputRotation.col(0).norm();

    const Leap::TrackedQuad quad = m_CurFrame.trackedQuad();
    if (quad.isValid() && quad.visible()) {
      // ratio of tracked quad to monitor dimensions
      const double horizScale = 1.143;
      const double vertScale = 1.286;

      Globals::haveScreen = true;
      Globals::screenWidth = horizScale * scale * quad.width();
      Globals::screenHeight = vertScale * scale * quad.height();
      m_ScreenPositionSmoother.SetGoal(m_InputRotation * quad.position().toVector3<Eigen::Vector3d>() + m_InputTranslation);
      m_ScreenRotationSmoother.SetGoal(m_InputRotation * toEigen(quad.orientation()));
    }
    m_ScreenPositionSmoother.Update(leapDeltaTime);
    m_ScreenRotationSmoother.Update(leapDeltaTime);
  }
  Globals::screenPos = m_ScreenPositionSmoother.Value();
  Globals::screenBasis = m_ScreenRotationSmoother.Value();
  const double prevTimeSeconds = timestampToSeconds(m_PrevFrame.timestamp());
  const double curTimeSeconds = timestampToSeconds(m_CurFrame.timestamp());
  const float leapDeltaTime = static_cast<float>(curTimeSeconds - prevTimeSeconds);
  leapInteract(leapDeltaTime);

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

  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE);
  drawWindows();
  drawFakeMouse();
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
  AutowiredFast<WindowManager> manager;
  if (manager) {
    for (auto& it : manager->m_Windows) {
      FakeWindow& wind = *it.second;
      wind.Interact(*(manager->m_WindowTransform), m_TrackedHands, deltaTime);
    }
  }
}

void Scene::drawHands() const {
  for (const auto& it : m_TrackedHands) {
    const HandInfo& trackedHand = *it.second;
    trackedHand.DrawCapsuleHand(m_Renderer, m_InputRotation, m_InputTranslation, m_ImagePassthrough.get());
  }
}

void Scene::drawFakeMouse() const {
  AutowiredFast<WindowManager> manager;
  static Leap::GL::Rgba<float> defaultColor(0.9f, 0.9f, 0.9f, 1.0f);
  static Leap::GL::Rgba<float> leftClickColor(0.3f, 0.5f, 1.0f, 1.0f);
  static Leap::GL::Rgba<float> rightClickColor(1.0f, 0.5f, 0.3f, 1.0f);
  static const double defaultRadius = 6.0;
  static const double clickRadius = 4.5;
  if (manager) {
    auto pos = sf::Mouse::getPosition();
    const Eigen::Vector2d mousePos(pos.x, -pos.y);
    const auto& transform = manager->m_WindowTransform;
    const Eigen::Vector3d mouse3D = transform->Forward(mousePos);
    m_MouseSphere->Translation() = mouse3D;
    const bool leftPressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);
    const bool rightPressed = sf::Mouse::isButtonPressed(sf::Mouse::Right);
    Leap::GL::Rgba<float> color = defaultColor;
    if (leftPressed) {
      color = leftClickColor;
    } else if (rightPressed) {
      color = rightClickColor;
    }
    m_MouseSphere->Material().Uniform<AMBIENT_LIGHT_COLOR>() = color;
    m_MouseSphere->SetRadius((leftPressed || rightPressed) ? clickRadius : defaultRadius);
    PrimitiveBase::DrawSceneGraph(*m_MouseSphere, m_Renderer);
  }
}

void Scene::drawWindows() const {
  AutowiredFast<WindowManager> manager;
  if (manager) {
    for (const auto& it : manager->m_Windows) {
      PrimitiveBase::DrawSceneGraph(*it.second->m_Texture, m_Renderer);
      for (const auto& it2 : m_TrackedHands) {
        const HandInfo& trackedHand = *it2.second;
        HandInfo::IntersectionVector intersections = trackedHand.IntersectRectangle(*it.second->m_Texture);
        for (const auto& intersection : intersections) {
          m_IntersectionDisk->Translation() = intersection.point;
          m_IntersectionDisk->SetRadius(1.25*intersection.radius);
          m_IntersectionDisk->Material().Uniform<AMBIENT_LIGHT_COLOR>() = makeIntersectionDiskColor(intersection.confidence);
          PrimitiveBase::DrawSceneGraph(*m_IntersectionDisk, m_Renderer);
        }
      }
    }
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
  m_ShowCalendar = false;
  m_ButtonCooldown = false;
}

void Scene::drawUI() const {
  const Leap::GL::Rgba<uint8_t> calendarColor(139, 138, 251);
  const Leap::GL::Rgba<uint8_t> emailColor(211, 107, 202);
  const Leap::GL::Rgba<uint8_t> phoneColor(87, 208, 193);
  const Leap::GL::Rgba<uint8_t> recordColor(65, 174, 229);
  const Leap::GL::Rgba<uint8_t> textColor(251, 55, 104);

  const double radius = m_IconDisk->Radius();
  const double spacing = 2.25 * radius;
  double curX = 250;
  double curY = 150;
  double curZ = 100;

  {
    const double clockScale = 0.25;
    m_Text->Translation() << curX, curY + spacing, curZ;
    const Eigen::Matrix3d rotation = faceCameraMatrix(m_Text->Translation(), m_InputTranslation);

    m_Text->Material().Uniform<AMBIENT_LIGHT_COLOR>() = Leap::GL::Rgba<float>(1.0f, 1.0f, 1.0f, 1.0f);
    m_Text->LinearTransformation() = clockScale * rotation;
    PrimitiveBase::DrawSceneGraph(*m_Text, m_Renderer);
  }

  {
    const Eigen::Matrix3d calendarExpandedMatrix = Eigen::Matrix3d::Identity();
    //const Eigen::Matrix3d calendarExpandedMatrix = faceCameraMatrix(m_ExpandedPrimitive->Translation(), m_InputTranslation);
    const double size = 4 * spacing;
    m_ExpandedPrimitive->SetTexture(m_CalendarExpanded->GetTexture());
    m_ExpandedPrimitive->SetScaleBasedOnTextureSize();
    const double scale = (size + 2*radius) / m_ExpandedPrimitive->Size().y();
    //m_ExpandedPrimitive->Translation() << 50, 150 - size/2.0, curZ;
    m_ExpandedPrimitive->Translation() << spacing + scale*0.5*m_ExpandedPrimitive->Size().x(), radius - scale*0.5*m_ExpandedPrimitive->Size().y(), 0.0;
    m_ExpandedPrimitive->LinearTransformation() = scale * calendarExpandedMatrix;
    //PrimitiveBase::DrawSceneGraph(*m_ExpandedPrimitive, m_Renderer);
  }

  {
    m_IconDisk->Material().Uniform<AMBIENT_LIGHT_COLOR>() = calendarColor;
    m_IconPrimitive->SetTexture(m_CalendarIcon->GetTexture());
    m_IconPrimitive->SetScaleBasedOnTextureSize();
    const double scale = 1.5 * m_IconDisk->Radius() / m_IconPrimitive->Size().norm();
    m_IconDisk->Translation() << curX, curY, curZ;
    m_IconDisk->LinearTransformation() = faceCameraMatrix(m_IconDisk->Translation(), m_InputTranslation);
    m_IconPrimitive->LinearTransformation() = scale * Eigen::Matrix3d::Identity();
    if (m_ShowCalendar) {
      m_IconDisk->AddChild(m_ExpandedPrimitive);
    }
    PrimitiveBase::DrawSceneGraph(*m_IconDisk, m_Renderer);
    if (m_ShowCalendar) {
      m_IconDisk->RemoveChild(m_ExpandedPrimitive);
    }
    curY -= spacing;
  }

  for (const auto& it : m_TrackedHands) {
    const HandInfo& trackedHand = *it.second;
    HandInfo::IntersectionVector intersections = trackedHand.IntersectDisk(*m_IconDisk);
    if (intersections.empty()) {
      if (m_ButtonCooldown) {
        m_ButtonCooldown = false;
      }
    } else {
      if (!m_ButtonCooldown) {
        m_ShowCalendar = !m_ShowCalendar;
        m_ButtonCooldown = true;
      }
    }
  }

  {
    m_IconDisk->Material().Uniform<AMBIENT_LIGHT_COLOR>() = emailColor;
    m_IconPrimitive->SetTexture(m_EmailIcon->GetTexture());
    m_IconPrimitive->SetScaleBasedOnTextureSize();
    const double scale = 1.5 * m_IconDisk->Radius() / m_IconPrimitive->Size().norm();
    m_IconDisk->Translation() << curX, curY, curZ;
    m_IconDisk->LinearTransformation() = faceCameraMatrix(m_IconDisk->Translation(), m_InputTranslation);
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
    m_IconDisk->LinearTransformation() = faceCameraMatrix(m_IconDisk->Translation(), m_InputTranslation);
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
    m_IconDisk->LinearTransformation() = faceCameraMatrix(m_IconDisk->Translation(), m_InputTranslation);
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
    m_IconDisk->LinearTransformation() = faceCameraMatrix(m_IconDisk->Translation(), m_InputTranslation);
    m_IconPrimitive->LinearTransformation() = scale * Eigen::Matrix3d::Identity();
    PrimitiveBase::DrawSceneGraph(*m_IconDisk, m_Renderer);
    curY -= spacing;
  }
}

Leap::GL::Rgba<float> Scene::makeIntersectionDiskColor(double confidence) {
  Leap::GL::Rgba<float> color;
  color.R() = Globals::glowColor.x();
  color.G() = Globals::glowColor.y();
  color.B() = Globals::glowColor.z();
  color.A() = confidence;
  return color;
}
