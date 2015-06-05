#include "stdafx.h"
#include "Window.h"

void Window::Init(const WindowParams& params) {
  m_Params = params;
  sf::Uint32 windowStyle = sf::Style::Default;

  m_Settings.antialiasingLevel = 0;
  if (m_Params.antialias) {
    m_Settings.antialiasingLevel = 16;
  }

  if (m_Params.fullscreen) {
    sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
    m_Params.windowPosX = 0;
    m_Params.windowPosY = 0;
    m_Params.windowWidth = desktopMode.width;
    m_Params.windowHeight = desktopMode.height-1;
    windowStyle = sf::Style::Fullscreen;
  }

  m_Settings.depthBits = 24;

  m_Window.setFramerateLimit(0);

  m_Window.create(sf::VideoMode(m_Params.windowWidth, m_Params.windowHeight, 32U), m_Params.windowTitle, windowStyle, m_Settings);
  m_Window.setVisible(false);
  m_Window.setVerticalSyncEnabled(m_Params.vsync);
  m_Window.setPosition(sf::Vector2i(m_Params.windowPosX, m_Params.windowPosY));
  m_Window.setVisible(true);
}

void Window::Present() const {
  m_Window.display();
}

bool Window::PollEvent(sf::Event& e) {
  return m_Window.pollEvent(e);
}

void Window::SetWindowSize(int width, int height) {
  m_Window.setSize(sf::Vector2u(width, height));
}

void Window::SetWindowPos(int x, int y) {
  m_Window.setPosition(sf::Vector2i(x, y));
}
