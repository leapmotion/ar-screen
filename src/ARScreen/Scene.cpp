#include "stdafx.h"
#include "Scene.h"
#include "Leap/GL/Projection.h"
#include "WindowManager.h"
#include "Globals.h"

Scene::Scene() :
  m_ScreenPositionSmoother(Eigen::Vector3d::Zero()),
  m_ScreenRotationSmoother(Eigen::Matrix3d::Identity()),
  m_CalendarOpacity(0.0f),
  m_ButtonAnimation(1.0f),
  m_ActivationGesture(false),
  m_DeactivationGesture(false),
  m_ImageOpacity(1.0f),
  m_ScrollVel(0.0)
{
  m_ScreenPositionSmoother.SetSmoothStrength(0.9f);
  m_ScreenRotationSmoother.SetSmoothStrength(0.9f);
  m_CalendarOpacity.SetSmoothStrength(0.6f);
  m_CalendarOpacity.Update(0.1f);
  m_ImageOpacity.SetSmoothStrength(0.9f);
  m_ImageOpacity.Update(0.1f);
  m_ButtonAnimation.SetSmoothStrength(0.7f);
}

void Scene::Init() {
  m_InputRotation = EigenTypes::Matrix3x3::Identity();
  m_InputTranslation = EigenTypes::Vector3::Zero();

  m_ImagePassthrough = std::shared_ptr<ImagePassthrough>(new ImagePassthrough());
  m_ImagePassthrough->Init();

  m_Font = std::shared_ptr<TextureFont>(new TextureFont(100.0f, "Roboto-Regular.ttf", 1024, 1024));
  m_Font->Load();

  m_ClockText = std::shared_ptr<TextPrimitive>(new TextPrimitive());
  m_ClockText->SetText(L" ", m_Font);

  m_MouseSphere = std::shared_ptr<Sphere>(new Sphere());
  m_MouseSphere->Material().Uniform<AMBIENT_LIGHTING_PROPORTION>() = 1.0f;

  m_IntersectionDisk = std::shared_ptr<Disk>(new Disk());
  m_IntersectionDisk->Material().Uniform<AMBIENT_LIGHTING_PROPORTION>() = 1.0f;

  createUI();

  createNewsFeed();

  createPeople();
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
    m_ClockText->SetText(timeStrW, m_Font);
    m_ClockString = timeStrW;
  }

  m_CalendarOpacity.Update((Globals::timeBetweenFrames.count()));
  m_ButtonAnimation.Update((Globals::timeBetweenFrames.count()));
  m_ImageOpacity.Update((Globals::timeBetweenFrames.count()));
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
  m_ImagePassthrough->Draw(m_Renderer, m_ImageOpacity.Value());
  glEnable(GL_DEPTH_TEST);

  m_Renderer.GetModelView().Matrix() = view.cast<double>();

  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE);
  drawWindows();
  drawFakeMouse();
  drawUI();
  drawNewsFeed();
  drawPeople();

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

  double scrollVel = 0;
  for (const auto& it : m_TrackedHands) {
    const HandInfo& trackedHand = *it.second;
    HandInfo::IntersectionVector intersections = trackedHand.IntersectRectangle(*m_NewsFeedRect);
    for (const auto& intersection : intersections) {
      scrollVel += 0.25 * intersection.velocity.y();
    }
  }
  if (std::fabs(scrollVel) > 0.01) {
    m_ScrollVel.SetSmoothStrength(0.1f);
  } else {
    m_ScrollVel.SetSmoothStrength(0.8f);
  }
  m_ScrollVel.SetGoal(scrollVel);
  m_ScrollVel.Update(deltaTime);
  m_FeedScroll += deltaTime * m_ScrollVel.Value();

  if (manager) {
    // detect activation gesture
    int numActivatingHands = 0;
    int numDeactivatingHands = 0;
    if (m_TrackedHands.size() >= 2) {
      for (const auto& it : m_TrackedHands) {
        const HandInfo& trackedHand = *it.second;
        const Leap::Hand& hand = trackedHand.GetLastSeenHand();
        const bool confident = trackedHand.GetConfidence() > 0.8;
        const bool facingOutward = hand.palmNormal().y > 0.8f;
        const bool fast = hand.palmVelocity().magnitude() > 600;
        const float yNorm = hand.palmVelocity().normalized().y;
        const bool pulling = yNorm < -0.8f;
        const bool pushing = yNorm > 0.8f;
        if (confident && facingOutward && fast) {
          if (pulling) {
            numActivatingHands++;
          } else if (pushing) {
            numDeactivatingHands++;
          }
        }
      }
    }

    const double gestureTime = 0.15;
    if (numActivatingHands == 2) {
      if (!m_ActivationGesture) {
        m_GestureStart = Globals::curFrameTime;
        m_ActivationGesture = true;
      } else {
        const double timeDiff = (Globals::curFrameTime - m_GestureStart).count();
        if (timeDiff >= gestureTime && !manager->m_Active) {
          manager->Activate();
          m_ActivationGesture = false;
        }
      }
    } else {
      m_ActivationGesture = false;
    }
    
    if (numDeactivatingHands == 2) {
      if (!m_DeactivationGesture) {
        m_GestureStart = Globals::curFrameTime;
        m_DeactivationGesture = true;
      } else {
        const double timeDiff = (Globals::curFrameTime - m_GestureStart).count();
        if (timeDiff >= gestureTime && manager->m_Active) {
          manager->Deactivate();
          m_DeactivationGesture = false;
        }
      }
    } else {
      m_DeactivationGesture = false;
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
  if (manager && manager->m_Active) {
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
          m_IntersectionDisk->LinearTransformation() = Eigen::Matrix3d::Identity();
          PrimitiveBase::DrawSceneGraph(*m_IntersectionDisk, m_Renderer);
        }
      }
    }
  }
}

void Scene::createUI() {
  m_IconDisk = std::shared_ptr<Disk>(new Disk());
  m_AnimationDisk = std::shared_ptr<Disk>(new Disk());
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
  m_ButtonCooldown = false;
  m_CalendarPressed = false;
  m_DarkModePressed = false;
}

void Scene::drawUI() const {
  const Leap::GL::Rgba<uint8_t> calendarColor(139, 138, 251);
  const Leap::GL::Rgba<uint8_t> emailColor(211, 107, 202);
  const Leap::GL::Rgba<uint8_t> phoneColor(87, 208, 193);
  const Leap::GL::Rgba<uint8_t> recordColor(65, 174, 229);
  const Leap::GL::Rgba<uint8_t> textColor(251, 55, 104);
  const Leap::GL::Rgba<float> clockColor(1.0f, 1.0f, 1.0f, 1.0f);

  const double radius = m_IconDisk->Radius();
  const double spacing = 2.25 * radius;
  double curX = 350;
  double curY = 100 + Globals::globalHeightOffset;
  double curZ = 175;

  const float blend = m_ButtonAnimation.Value();
  const float alpha = 0.85f * SmootherStep(1.0f - blend);
  m_AnimationDisk->Material().Uniform<AMBIENT_LIGHT_COLOR>().A() = alpha;
  const double origRadius = 1.1*m_IconDisk->Radius();
  const double targetRadius = origRadius * 2.0;
  m_AnimationDisk->SetRadius((1.0f-blend)*origRadius+ blend*targetRadius);
  m_AnimationDisk->Translation() << 0.0, 0.0, -2.0;

  {
    const Eigen::Matrix3d calendarExpandedMatrix = Eigen::Matrix3d::Identity();
    //const Eigen::Matrix3d calendarExpandedMatrix = faceCameraMatrix(m_ExpandedPrimitive->Translation(), m_InputTranslation);
    const double size = 4 * spacing;
    m_ExpandedPrimitive->SetTexture(m_CalendarExpanded->GetTexture());
    m_ExpandedPrimitive->SetScaleBasedOnTextureSize();
    const double scale = (size + 2*radius) / m_ExpandedPrimitive->Size().y();
    m_ExpandedPrimitive->Translation() << spacing + scale*0.5*m_ExpandedPrimitive->Size().x(), radius - scale*0.5*m_ExpandedPrimitive->Size().y(), 0.0;
    m_ExpandedPrimitive->LinearTransformation() = scale * calendarExpandedMatrix;
  }

  {
    m_IconDisk->Material().Uniform<AMBIENT_LIGHT_COLOR>() = calendarColor;
    m_IconPrimitive->SetTexture(m_CalendarIcon->GetTexture());
    m_IconPrimitive->SetScaleBasedOnTextureSize();
    const double scale = 1.5 * m_IconDisk->Radius() / m_IconPrimitive->Size().norm();
    m_IconDisk->Translation() << curX, curY, curZ;
    m_IconDisk->LinearTransformation() = faceCameraMatrix(m_IconDisk->Translation(), Globals::userPos, false);
    m_IconPrimitive->LinearTransformation() = scale * Eigen::Matrix3d::Identity();
    const bool showCalendar = m_CalendarOpacity.Value() > 0.0001f;
    m_ExpandedPrimitive->Material().Uniform<AMBIENT_LIGHT_COLOR>().A() = m_CalendarOpacity.Value();
    if (showCalendar) {
      m_IconDisk->AddChild(m_ExpandedPrimitive);
    }
    if (alpha > 0.00001f && m_CalendarPressed) {
      m_IconDisk->AddChild(m_AnimationDisk);
    }
    PrimitiveBase::DrawSceneGraph(*m_IconDisk, m_Renderer);
    if (showCalendar) {
      m_IconDisk->RemoveChild(m_ExpandedPrimitive);
    }
    if (alpha > 0.00001f && m_CalendarPressed) {
      m_IconDisk->RemoveChild(m_AnimationDisk);
    }
    curY -= spacing;
  }

  for (const auto& it : m_TrackedHands) {
    const HandInfo& trackedHand = *it.second;
    HandInfo::IntersectionVector intersections = trackedHand.IntersectDisk(*m_IconDisk);
    if (intersections.empty()) {
      if (m_ButtonCooldown && m_ButtonAnimation.Value() > 0.99f) {
        m_ButtonCooldown = false;
        m_CalendarPressed = false;
        m_DarkModePressed = false;
      }
    } else {
      if (!m_ButtonCooldown) {
        m_ButtonCooldown = true;
        m_CalendarPressed = true;
        if (m_CalendarOpacity.Goal() == 0.0f) {
          m_CalendarOpacity.SetGoal(1.0f);
        } else {
          m_CalendarOpacity.SetGoal(0.0f);
        }
        m_ButtonAnimation.SetImmediate(0.0f);
        m_ButtonAnimation.SetGoal(1.0f);
      }
    }
  }

  {
    m_IconDisk->Material().Uniform<AMBIENT_LIGHT_COLOR>() = emailColor;
    m_IconPrimitive->SetTexture(m_EmailIcon->GetTexture());
    m_IconPrimitive->SetScaleBasedOnTextureSize();
    const double scale = 1.5 * m_IconDisk->Radius() / m_IconPrimitive->Size().norm();
    m_IconDisk->Translation() << curX, curY, curZ;
    m_IconDisk->LinearTransformation() = faceCameraMatrix(m_IconDisk->Translation(), Globals::userPos, false);
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
    m_IconDisk->LinearTransformation() = faceCameraMatrix(m_IconDisk->Translation(), Globals::userPos, false);
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
    m_IconDisk->LinearTransformation() = faceCameraMatrix(m_IconDisk->Translation(), Globals::userPos, false);
    m_IconPrimitive->LinearTransformation() = scale * Eigen::Matrix3d::Identity();
    if (alpha > 0.00001f && m_DarkModePressed) {
      m_IconDisk->AddChild(m_AnimationDisk);
    }
    PrimitiveBase::DrawSceneGraph(*m_IconDisk, m_Renderer);
    if (alpha > 0.00001f && m_DarkModePressed) {
      m_IconDisk->RemoveChild(m_AnimationDisk);
    }
    curY -= spacing;
  }

  for (const auto& it : m_TrackedHands) {
    const HandInfo& trackedHand = *it.second;
    HandInfo::IntersectionVector intersections = trackedHand.IntersectDisk(*m_IconDisk);
    if (intersections.empty()) {
      if (m_ButtonCooldown && m_ButtonAnimation.Value() > 0.99f) {
        m_ButtonCooldown = false;
        m_CalendarPressed = false;
        m_DarkModePressed = false;
      }
    } else {
      if (!m_ButtonCooldown) {
        m_ButtonCooldown = true;
        m_DarkModePressed = true;
        if (m_ImageOpacity.Goal() == 0.0f) {
          m_ImageOpacity.SetGoal(1.0f);
        } else {
          m_ImageOpacity.SetGoal(0.0f);
        }
        m_ButtonAnimation.SetImmediate(0.0f);
        m_ButtonAnimation.SetGoal(1.0f);
      }
    }
  }

  {
    m_IconDisk->Material().Uniform<AMBIENT_LIGHT_COLOR>() = textColor;
    m_IconPrimitive->SetTexture(m_TextsIcon->GetTexture());
    m_IconPrimitive->SetScaleBasedOnTextureSize();
    const double scale = 1.5 * m_IconDisk->Radius() / m_IconPrimitive->Size().norm();
    m_IconDisk->Translation() << curX, curY, curZ;
    m_IconDisk->LinearTransformation() = faceCameraMatrix(m_IconDisk->Translation(), Globals::userPos, false);
    m_IconPrimitive->LinearTransformation() = scale * Eigen::Matrix3d::Identity();
    PrimitiveBase::DrawSceneGraph(*m_IconDisk, m_Renderer);
    curY -= spacing;
  }

  {
    const double clockScale = 0.3;
    m_ClockText->Translation() << curX, curY - spacing, curZ;
    const Eigen::Matrix3d rotation = faceCameraMatrix(m_ClockText->Translation(), Globals::userPos, true);

    m_ClockText->Material().Uniform<AMBIENT_LIGHT_COLOR>() = clockColor;
    m_ClockText->LinearTransformation() = clockScale * rotation;
    PrimitiveBase::DrawSceneGraph(*m_ClockText, m_Renderer);
  }
}

void Scene::createNewsFeed() {
  const std::vector<std::string> feedStrings ={
    "You have five unread email messages",
    "Your car repairs will be completed tomorrow afternoon",
    "Bob Simmons has added you as a connection on LinkedIn",
    "You have two new friend requests on Facebook",
    "Your anniversary is in a few weeks",
    "Project proposal is due today at 5PM",
    "Rachel's birthday is tomorrow",
    "There is construction on the Bay bridge tonight",
    "Weather this weekend will be mostly sunny",
    "Golden State Warriors have won the NBA Finals",
    "Steven invited you to catch up over drinks on Friday",
    "Apple announced iOS 9 this morning",
    "All BART trains are experiencing heavy delays",
    "You have three phone screens next week",
    "Donate to Nepal earthquake relief",
    "Your subscription to Lorem Ipsum expires next Tuesday",
    "Ralph Johnson started a new job at Google today",
    "Your next meeting is in 45 minutes",
    "You've burned 330 calories so far today",
    "Marvin Porter starts on your team next week"
  };

  m_NewsFeedRect = std::shared_ptr<RectanglePrim>(new RectanglePrim());
  for (const std::string& str : feedStrings) {
    std::shared_ptr<TextPrimitive> feedItem = std::shared_ptr<TextPrimitive>(new TextPrimitive());
    feedItem->SetText(std::wstring(str.begin(), str.end()), m_Font);
    m_NewsFeedItems.push_back(feedItem);
    m_NewsFeedRect->AddChild(feedItem);
    feedItem->Material().Uniform<AMBIENT_LIGHT_COLOR>().A() = 0.0f;
  }

  m_NewsFeedRect->Material().Uniform<AMBIENT_LIGHT_COLOR>() = Leap::GL::Rgba<float>(0.7f, 0.9f, 1.0f, 0.15f);
  m_FeedScroll = 10000;
}

void Scene::drawNewsFeed() const {
  const double feedHeight = 250.0;
  const double feedWidth = 350.0;

  m_NewsFeedRect->SetSize(Eigen::Vector2d(feedWidth, feedHeight));
  m_NewsFeedRect->Translation() << -350, 50 + Globals::globalHeightOffset, 250;
  m_NewsFeedRect->LinearTransformation() = faceCameraMatrix(m_NewsFeedRect->Translation(), Globals::userPos, false);

  double curY = 0;
  const double spacing = 20.0;
  size_t itemIdx = 0;
  bool primed = false;
  bool started = false;
  bool done = false;
  while (!done) {
    const double itemY = curY + m_FeedScroll;
    const double distFromEdge = std::min(std::max(feedHeight/2.0 - itemY, 0.0), std::max(itemY + feedHeight/2.0, 0.0));
    const double alphaMult = SmootherStep(std::max(0.0, std::min(1.0, distFromEdge/(2.0*spacing))));

    const std::shared_ptr<TextPrimitive>& item = m_NewsFeedItems[itemIdx];
    const Leap::GL::Rgba<float> itemColor(1.0f, 1.0f, 1.0f, alphaMult);
    item->Material().Uniform<AMBIENT_LIGHT_COLOR>() = itemColor;
    if (alphaMult > 0.0001) {
      item->Translation() << -feedWidth/2.0 + 0.5*spacing, itemY, 2;
      const double scale = 0.125;
      item->LinearTransformation() = scale * Eigen::Matrix3d::Identity();
      started = true;
    }
    if (started && alphaMult == 0.0) {
      done = true;
    }
    curY -= spacing;
    itemIdx = (itemIdx + 1) % m_NewsFeedItems.size();
  }

  for (const auto& it : m_TrackedHands) {
    const HandInfo& trackedHand = *it.second;
    HandInfo::IntersectionVector intersections = trackedHand.IntersectRectangle(*m_NewsFeedRect);
    for (const auto& intersection : intersections) {
      m_IntersectionDisk->Translation() = intersection.point;
      m_IntersectionDisk->SetRadius(1.25*intersection.radius);
      m_IntersectionDisk->Material().Uniform<AMBIENT_LIGHT_COLOR>() = makeIntersectionDiskColor(intersection.confidence);
      m_IntersectionDisk->LinearTransformation() = m_NewsFeedRect->LinearTransformation();
      PrimitiveBase::DrawSceneGraph(*m_IntersectionDisk, m_Renderer);
    }
  }

  PrimitiveBase::DrawSceneGraph(*m_NewsFeedRect, m_Renderer);
}

void Scene::createPeople() {
  m_Person1 = GLTexture2ImageRef(new GLTexture2Image());
  m_Person1->LoadPath("david.png");

  m_Person2 = GLTexture2ImageRef(new GLTexture2Image());
  m_Person2->LoadPath("jimmy.png");

  m_Person3 = GLTexture2ImageRef(new GLTexture2Image());
  m_Person3->LoadPath("jon.png");

  m_PersonBG = std::shared_ptr<Disk>(new Disk());
  m_PersonPrimitive = std::shared_ptr<ImagePrimitive>(new ImagePrimitive());

  m_PersonBG->AddChild(m_PersonPrimitive);
  m_PersonBG->SetRadius(35);
  m_PersonPrimitive->Translation() << 0, 0, 2.0;
}

void Scene::drawPeople() const {
  const double radius = m_PersonBG->Radius();
  const double spacing = 3 * radius;
  double curX = -spacing;
  double curY = -125 + Globals::globalHeightOffset;
  double curZ = -325;

  const Leap::GL::Rgba<float> bgColor(1.0f, 1.0f, 1.0f, 0.15f);
  const Leap::GL::Rgba<float> notifyColor(1.0f, 1.0f, 1.0f, 0.5f);

  {
    m_PersonBG->Material().Uniform<AMBIENT_LIGHT_COLOR>() = bgColor;
    m_PersonBG->Translation() << curX, curY, curZ;
    m_PersonBG->LinearTransformation() = faceCameraMatrix(m_PersonBG->Translation(), Globals::userPos, true);
    m_PersonPrimitive->SetTexture(m_Person1->GetTexture());
    m_PersonPrimitive->SetScaleBasedOnTextureSize();
    const double scale = 2.5 * radius / m_PersonPrimitive->Size().norm();
    m_PersonPrimitive->LinearTransformation() = scale * Eigen::Matrix3d::Identity();
    PrimitiveBase::DrawSceneGraph(*m_PersonBG, m_Renderer);
    curX += spacing;
  }

  {
    m_PersonBG->Material().Uniform<AMBIENT_LIGHT_COLOR>() = bgColor;
    m_PersonBG->Translation() << curX, curY, curZ;
    m_PersonBG->LinearTransformation() = faceCameraMatrix(m_PersonBG->Translation(), Globals::userPos, true);
    m_PersonPrimitive->SetTexture(m_Person2->GetTexture());
    m_PersonPrimitive->SetScaleBasedOnTextureSize();
    const double scale = 2.5 * radius / m_PersonPrimitive->Size().norm();
    m_PersonPrimitive->LinearTransformation() = scale * Eigen::Matrix3d::Identity();
    PrimitiveBase::DrawSceneGraph(*m_PersonBG, m_Renderer);
    curX += spacing;
  }

  {
    const double delayedTime = std::max(0.0, Globals::elapsedTimeSeconds - 15.0);
    const double mult = 0.5*(std::sin(6*delayedTime) + 1.0);
    const float blend = SmootherStep(mult*mult*mult*mult);
    m_PersonBG->Material().Uniform<AMBIENT_LIGHT_COLOR>() = bgColor.BlendedWith(notifyColor, blend);
    m_PersonBG->Translation() << curX, curY, curZ;
    m_PersonBG->LinearTransformation() = faceCameraMatrix(m_PersonBG->Translation(), Globals::userPos, true);
    m_PersonPrimitive->SetTexture(m_Person3->GetTexture());
    m_PersonPrimitive->SetScaleBasedOnTextureSize();
    const double scale = 2.5 * radius / m_PersonPrimitive->Size().norm();
    m_PersonPrimitive->LinearTransformation() = scale * Eigen::Matrix3d::Identity();
    PrimitiveBase::DrawSceneGraph(*m_PersonBG, m_Renderer);
    curX += spacing;
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
