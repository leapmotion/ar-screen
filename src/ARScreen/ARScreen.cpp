#include "stdafx.h"
#include "ARScreen.h"
#include "utility/PlatformInitializer.h"
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
  m_Window.Init(params);
  if (glewInit() != GLEW_OK) {
    throw std::runtime_error("Unable to initialize glew");
  }
  FreeImage_Initialise();

  // Dispatch events until told to quit:
  auto then = std::chrono::steady_clock::now();
  for(AutoCurrentContext ctxt; !ctxt->IsShutdown(); ) {
    // Handle autowiring events:
    DispatchAllEvents();

    // Handle SFML events
    HandleWindowEvents();
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
