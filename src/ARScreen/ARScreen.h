#pragma once
#include <autowiring/autowiring.h>
#include "LeapListener/LeapListener.h"
#include "OculusVR/OculusVR.h"
#include "Window.h"
#include "Scene.h"

struct ARScreenContext {};

class Updatable;

class ARScreen :
  public DispatchQueue,
  public ExceptionFilter
{
public:
  ARScreen(void);
  ~ARScreen(void);

public:
  void Main(void);
  void Filter(void) override;

  void HandleWindowEvents();
  void Update();
  void Render();

  AutoFired<Updatable> m_update;

  Scene m_Scene;
  Window m_Window;
  OculusVR m_Oculus;
  Leap::Controller m_Controller;
  LeapListener m_Listener;
};
