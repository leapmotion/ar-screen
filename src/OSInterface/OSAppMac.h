#pragma once
#include "OSApp.h"

#include <AppKit/NSImage.h>

class OSAppMac:
  public OSApp
{
public:
  OSAppMac(uint32_t pid);
  ~OSAppMac();

  std::string GetAppName(void) const override;
  std::shared_ptr<ImagePrimitive> GetIconTexture(std::shared_ptr<ImagePrimitive> img) const override;

private:
  std::string m_name;
  NSImage *m_icon;
};

static_assert(!std::is_abstract<OSAppMac>::value, "OSAppMac is meant to be a concrete type");
