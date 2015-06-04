#pragma once
#include <autowiring/autowiring.h>

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
};
