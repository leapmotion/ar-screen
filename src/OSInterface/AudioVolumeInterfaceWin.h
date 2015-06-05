#pragma once
#include "AudioVolumeInterface.h"

struct IAudioEndpointVolume;
struct IAudioSessionManager;
struct IMMDevice;
struct IMMDeviceEnumerator;
struct ISimpleAudioVolume;

class AudioVolumeInterfaceWin:
  public AudioVolumeInterface
{
public:
  AudioVolumeInterfaceWin(void);

private:
  CComPtr<IMMDeviceEnumerator> m_devEnumerator;
  CComPtr<IMMDevice> m_pEndpoint;
  CComPtr<IAudioEndpointVolume> m_pAudioEndpointVolume;

public:
  // AudioVolumeInterface overrides:
  float GetVolume(void) override;
  void SetVolume(float volume) override;
  void SetMute(bool mute) override;
  bool IsMuted(void) override;
};
