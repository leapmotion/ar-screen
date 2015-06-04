#pragma once
#include <autowiring/autowiring.h>
#include "Window.h"

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
  Window m_Window;
};
