#include "stdafx.h"
#include "ARScreen.h"
#include "utility/PlatformInitializer.h"
#include <iostream>
#include <stdexcept>
#include "Globals.h"

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
  m_Window.Init(params);
  if (glewInit() != GLEW_OK) {
    throw std::runtime_error("Unable to initialize glew");
  }
  FreeImage_Initialise();

#if _WIN32
  m_Oculus.SetWindow(m_Window.GetHWND());
#endif
  if (!m_Oculus.Init()) {
    Globals::haveOculus = false;
    std::cout << "No oculus detected" << std::endl;
  } else {
    Globals::haveOculus = true;
    const ovrVector2i windowsPos = m_Oculus.GetWindowsPos();
    m_Window.SetWindowPos(windowsPos.x, windowsPos.y);
    m_Window.SetWindowSize(m_Oculus.GetHMDWidth(), m_Oculus.GetHMDHeight());
  }

  m_Controller.addListener(m_Listener);

  // Dispatch events until told to quit:
  Globals::prevFrameTime = std::chrono::steady_clock::now();
  for(AutoCurrentContext ctxt; !ctxt->IsShutdown(); ) {
    // Handle autowiring events:
    DispatchAllEvents();

    // Handle SFML events
    HandleWindowEvents();

    Globals::curFrameTime = std::chrono::steady_clock::now();
    Globals::timeBetweenFrames = Globals::curFrameTime - Globals::prevFrameTime;

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

void ARScreen::Render() {

}
