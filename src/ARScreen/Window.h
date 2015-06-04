#pragma once

#include <SFML/Window/Window.hpp>

struct WindowParams {
  WindowParams() :
    windowWidth(640),
    windowHeight(480),
    windowPosX(100),
    windowPosY(100),
    fullscreen(false),
    vsync(false),
    antialias(true),
    windowTitle("ARScreen")
  { }

  int windowWidth;
  int windowHeight;
  int windowPosX;
  int windowPosY;
  bool fullscreen;
  bool vsync;
  bool antialias;
  std::string windowTitle;
};

class Window {
public:
  void Init(const WindowParams& params);

  void Present() const;

  bool PollEvent(sf::Event& e);

  void SetWindowSize(int width, int height);
  void SetWindowPos(int x, int y);

#if _WIN32
  HWND GetHWND() { return m_HWND; }
#endif

private:
  sf::ContextSettings m_Settings;
  mutable sf::Window m_Window;
  sf::WindowHandle m_WindowHandle;
  WindowParams m_Params;
#if _WIN32
  HWND m_HWND;
#endif
};
