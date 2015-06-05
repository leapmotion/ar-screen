#pragma once

#include <deque>
#include "Leap.h"
#include <mutex>

// TODO: make a "params" class for initializing a LeapListener (e.g. for
// setPolicyFlags(Leap::Controller::POLICY_BACKGROUND_FRAMES), or enabling
// certain gestures, etc).

// A small extension of the functionality of Leap::Listener which provides
// access to some simple properties for convenience, such as retrieving the
// most recent frame, an accumulated history of frames, or connection state.
class LeapListener : public Leap::Listener {
public:

  LeapListener();
  virtual ~LeapListener();

  bool IsConnected() const;
  const Leap::Frame &MostRecentFrame() const;
  std::deque<Leap::Frame> TakeAccumulatedFrames();

  virtual void onInit(const Leap::Controller&);
  virtual void onConnect(const Leap::Controller&);
  virtual void onDisconnect(const Leap::Controller&);
  virtual void onFrame(const Leap::Controller&);
  virtual void onFocusGained(const Leap::Controller&);
  virtual void onFocusLost(const Leap::Controller&);

private:

  /// This mutex protects all member variables for this class.
  mutable std::mutex m_mutex;
  bool m_isConnected;
  Leap::Frame m_mostRecentFrame;
  std::deque<Leap::Frame> m_accumulatedFrames;
};
