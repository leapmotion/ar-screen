#include "stdafx.h"
#include "ARScreen.h"
#include "Globals.h"
#include "OSInterface/AudioVolumeInterface.h"
#include "OSInterface/MediaInterface.h"
#include "OSInterface/OSVirtualScreen.h"
#include "OSInterface/OSWindowMonitor.h"
#include "WindowManager.h"
#include "utility/PlatformInitializer.h"
#include "utility/Utilities.h"

#if _WIN32
#include "Mirror.h"
#endif

#include <iostream>
#include <stdexcept>

int main(int argc, char **argv)
{
  PlatformInitializer init;
  AutoCurrentContext ctxt;

  ctxt->Initiate();
  AutoRequired<ARScreen> arScreen;

  try {
    // Handoff to the main loop:
    arScreen->Main();
  }
  catch (std::exception& e) {
    std::cout << e.what() << std::endl;
  }

  ctxt->SignalShutdown(true);
  return 0;
}

#if _WIN32
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, INT)
{
  return main(__argc, __argv);
}
#endif

ARScreen::ARScreen(void)
{
}

ARScreen::~ARScreen(void) {}

void ARScreen::Main(void) {
  AutoCreateContextT<ARScreenContext> arScreenCtxt;
  arScreenCtxt->Initiate();
  CurrentContextPusher pshr(arScreenCtxt);

  WindowParams params;
  params.antialias = true;
  params.vsync = false;
  params.fullscreen = true;
  m_Window.Init(params);
  if (glewInit() != GLEW_OK) {
    throw std::runtime_error("Unable to initialize glew");
  }
  FreeImage_Initialise();

  // these should be created after GL context creation
  AutoRequired<WindowManager>();
  AutoRequired<OSVirtualScreen>();
  AutoRequired<OSWindowMonitor>()->EnableScan(true);
  AutoRequired<AudioVolumeInterface>();
  AutoRequired<MediaInterface>();

  m_Oculus.SetWindow(static_cast<OculusVR::WindowHandle>(m_Window.GetWindowHandle()));
  if (!m_Oculus.Init()) {
    Globals::haveOculus = false;
    std::cout << "No Oculus detected" << std::endl;
    m_ShowMirror = false;
  } else {
    Globals::haveOculus = true;
    const ovrVector2i windowsPos = m_Oculus.GetWindowsPos();
    m_Window.SetWindowPos(windowsPos.x, windowsPos.y);
    m_Window.SetWindowSize(m_Oculus.GetHMDWidth(), m_Oculus.GetHMDHeight());
    m_ShowMirror = true;
    m_Oculus.DismissHealthWarning();
  }

  m_Scene.Init();
  m_Controller.addListener(m_Listener);

  InitMirror();

  // Dispatch events until told to quit:
  Globals::prevFrameTime = std::chrono::steady_clock::now();
  for(AutoCurrentContext ctxt; !ctxt->IsShutdown(); ) {
    // Handle autowiring events:
    DispatchAllEvents();

    // Handle SFML events
    HandleWindowEvents();

    Globals::curFrameTime = std::chrono::steady_clock::now();
    Globals::timeBetweenFrames = Globals::curFrameTime - Globals::prevFrameTime;
    Globals::elapsedTimeSeconds += Globals::timeBetweenFrames.count();

    // Main operations
    Update();
    Render();

    Globals::prevFrameTime = Globals::curFrameTime;
  }
}

void ARScreen::Filter(void) {
  try {
    throw;
  }
  catch (std::exception& ex) {
    std::cerr << ex.what() << std::endl;
  }
}

void ARScreen::HandleWindowEvents() {
  sf::Event e;
  while (m_Window.PollEvent(e)) {
    if (e.type == sf::Event::KeyPressed) {
      if (e.key.code == sf::Keyboard::Escape) {
        exit(0);
      }
    } else if (e.type == sf::Event::Closed) {
      exit(0);
    } else if (e.type == sf::Event::Resized) {

    } else if (e.type == sf::Event::MouseButtonPressed) {

    } else if (e.type == sf::Event::MouseButtonReleased) {

    } else if (e.type == sf::Event::MouseMoved) {

    } else if (e.type == sf::Event::MouseWheelMoved) {

    }
  }
}

void ARScreen::Update() {
  m_update(&Updatable::Tick)(Globals::timeBetweenFrames);
  m_Scene.Update(m_Listener.TakeAccumulatedFrames());
}

void ARScreen::Render() {
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // if using transparent window, clear alpha value must be 0
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (Globals::haveOculus) {
    m_Oculus.BeginFrame();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if 0
    const float leapBaseline = 40.0f;
#else
    const float leapBaseline = 64.0f;
#endif

    const float OCULUS_BASELINE = 64.0f; // TODO: Get this value directly from the SDK

    const EigenTypes::Matrix4x4f avgView = 0.5f*(m_Oculus.EyeView(0) + m_Oculus.EyeView(1));
    EigenTypes::Matrix4x4f inputTransform = avgView.inverse();
    EigenTypes::Matrix3x3f conventionConv;
    conventionConv << -EigenTypes::Vector3f::UnitX(), -EigenTypes::Vector3f::UnitZ(), -EigenTypes::Vector3f::UnitY();
#if 1
    inputTransform.block<3, 3>(0, 0) *= (OCULUS_BASELINE / leapBaseline) * conventionConv;
#else
    inputTransform.block<3, 3>(0, 0) *= conventionConv;
#endif

    EigenTypes::Matrix3x3 rotation = inputTransform.block<3, 3>(0, 0).cast<double>();
    EigenTypes::Vector3 translation = inputTransform.block<3, 1>(0, 3).cast<double>();
    m_Scene.SetInputTransform(rotation, translation);

    for (int i=0; i<2; i++) {
      const ovrRecti& rect = m_Oculus.EyeViewport(i);
      const Eigen::Matrix4f proj = m_Oculus.EyeProjection(i);
      Eigen::Matrix4f view = m_Oculus.EyeView(i);
      const Eigen::Vector3f pos = m_Oculus.EyePosition(i);

      glViewport(rect.Pos.x, rect.Pos.y, rect.Size.w, rect.Size.h);

      m_Scene.Render(proj, view, i);
    }

    m_Oculus.EndFrame();
  } else {
    //m_earthLayer->Render(real_time_delta);


    glFlush();
    m_Window.Present();
  }

}

void ARScreen::InitMirror() {
#if _WIN32
  if (m_ShowMirror) {
    m_MirrorThread = std::thread(RunMirror, m_Window.GetWindowHandle(), std::ref(m_MirrorHWND));
  }
#endif
}

void ARScreen::ShutdownMirror() {
  if (m_MirrorThread.joinable()) {
#if _WIN32
    PostMessage(m_MirrorHWND, WM_CLOSE, 0, 0);
#endif
    m_MirrorThread.join();
  }
}

//Some laptops have dual integrated and discrete GPUs.  Since the default renderer is not always
//the discrete one, this code tries to force using the discrete graphics
#if _WIN32
typedef unsigned long DWORD;
extern "C" {
  _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
#endif
