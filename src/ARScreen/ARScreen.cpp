#include "stdafx.h"
#include "ARScreen.h"
#include "utility/PlatformInitializer.h"
#include <iostream>

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

  // Dispatch events until told to quit:
  auto then = std::chrono::steady_clock::now();
  for(AutoCurrentContext ctxt; !ctxt->IsShutdown(); ) {
    // Handle autowiring events:
    DispatchAllEvents();
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
