#pragma once
#include "MediaInterface.h"

class MediaInterfaceMac:
  public MediaInterface
{
public:
  void PlayPause(void) override;
  void Stop(void) override;
  void Next(void) override;
  void Prev(void) override;
  void VolumeUp(void) override;
  void VolumeDown(void) override;
  void Mute(void) override;
private:
  void SendSpecialKeyEvent(int32_t keyType, uint32_t mask, bool isDown);
  void SendSpecialKeyEventPair(int32_t keyType, uint32_t mask = 0) {
    SendSpecialKeyEvent(keyType, mask, true);
    SendSpecialKeyEvent(keyType, mask, false);
  }
};
