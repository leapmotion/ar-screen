#pragma once
#include "AudioVolumeInterface.h"

#include <CoreAudio/AudioHardware.h>

class AudioVolumeInterfaceMac:
  public AudioVolumeInterface
{
public:
  AudioVolumeInterfaceMac(void);

private:
  static AudioDeviceID GetAudioDeviceID();

public:
  // AudioVolumeInterface overrides:
  float GetVolume(void) override;
  void SetVolume(float volume) override;
  void SetMute(bool mute) override;
  bool IsMuted(void) override;
};
