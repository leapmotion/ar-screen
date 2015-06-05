#pragma once
#include "MediaInterface.h"

class MediaInterfaceWin:
  public MediaInterface
{
private:
  static void SendMediaKeyCode(uint32_t vk);

public:
  void PlayPause(void) override;
  void Stop(void) override;
  void Next(void) override;
  void Prev(void) override;
  void VolumeUp(void) override;
  void VolumeDown(void) override;
  void Mute(void) override;
};

