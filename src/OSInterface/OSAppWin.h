#pragma once
#include "OSApp.h"

class OSAppWin:
  public OSApp
{
public:
  OSAppWin(uint32_t pid);
  ~OSAppWin();

  std::string GetAppName(void) const override;
  std::shared_ptr<ImagePrimitive> GetIconTexture(std::shared_ptr<ImagePrimitive> img) const override;
};

static_assert(!std::is_abstract<OSAppWin>::value, "OSAppWin is meant to be a concrete type");
