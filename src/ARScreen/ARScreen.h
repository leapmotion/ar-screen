#pragma once
#include <autowiring/autowiring.h>
#include "LeapListener.h"
#include "OculusVR.h"
#include "Window.h"
#include "Scene.h"

struct ARScreenContext {};

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

  Scene m_Scene;
  Window m_Window;
  OculusVR m_Oculus;
  Leap::Controller m_Controller;
  LeapListener m_Listener;
};
