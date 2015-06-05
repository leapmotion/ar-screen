#include "LeapListener.h"

LeapListener::LeapListener()
  :
  m_isConnected(false)
{ }

LeapListener::~LeapListener() { }

bool LeapListener::IsConnected() const {
  std::unique_lock<std::mutex> lock(m_mutex);
  return m_isConnected;
}

const Leap::Frame &LeapListener::MostRecentFrame() const {
  std::unique_lock<std::mutex> lock(m_mutex);
  return m_mostRecentFrame;
}

std::deque<Leap::Frame> LeapListener::TakeAccumulatedFrames() {
  std::unique_lock<std::mutex> lock(m_mutex);
  std::deque<Leap::Frame> frames(std::move(m_accumulatedFrames));
  m_accumulatedFrames.clear();
  lock.unlock();
  return frames;
}

void LeapListener::onInit(const Leap::Controller& controller) {
}

void LeapListener::onConnect(const Leap::Controller& controller) {
  std::unique_lock<std::mutex> lock(m_mutex);
  // TODO: turn this into something configurable
  controller.setPolicyFlags(Leap::Controller::POLICY_BACKGROUND_FRAMES);
  controller.enableGesture(Leap::Gesture::TYPE_SCREEN_TAP);
  controller.enableGesture(Leap::Gesture::TYPE_KEY_TAP);
  controller.enableGesture(Leap::Gesture::TYPE_CIRCLE);
  controller.enableGesture(Leap::Gesture::TYPE_SWIPE);
  m_isConnected = true;
}

void LeapListener::onDisconnect(const Leap::Controller& controller) {
  std::unique_lock<std::mutex> lock(m_mutex);

  m_accumulatedFrames.clear();
  m_mostRecentFrame = Leap::Frame();
  m_isConnected = false;
}

void LeapListener::onFocusGained(const Leap::Controller& controller) {
  std::unique_lock<std::mutex> lock(m_mutex);

  m_accumulatedFrames.clear();
  m_mostRecentFrame = Leap::Frame();
  // TODO (?) track an "is focused" flag?
}

void LeapListener::onFocusLost(const Leap::Controller& controller) {
  std::unique_lock<std::mutex> lock(m_mutex);

  m_accumulatedFrames.clear();
  m_mostRecentFrame = Leap::Frame();
  // TODO (?) track an "is focused" flag?
}

void LeapListener::onFrame(const Leap::Controller& controller) {
  std::unique_lock<std::mutex> lock(m_mutex);

  if (m_mostRecentFrame.isValid()) {
    int history = 0;

    // Find historical frames that we are missing
    while (controller.frame(++history).id() > m_mostRecentFrame.id()) {
      ;;
    }
    // Add those historical frames to our deque
    while (--history > 0) {
      m_accumulatedFrames.push_back(controller.frame(history));
    }
  }
  m_mostRecentFrame = controller.frame();
  m_accumulatedFrames.push_back(m_mostRecentFrame);
}

