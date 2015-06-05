#include "stdafx.h"
#include "AudioVolumeInterfaceWin.h"
#include <Endpointvolume.h>
#include <Mmdeviceapi.h>

AudioVolumeInterfaceWin::AudioVolumeInterfaceWin(void) {
  // Enumerate all volume devices, find one we can talk to
  m_devEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL);
  if(!m_devEnumerator)
    throw std::runtime_error("Failed to create a multimedia device enumerator");

  HRESULT hr = m_devEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &m_pEndpoint);
  if(FAILED(hr))
    throw std::runtime_error("Failed to get a default audio endpoint");

  hr = m_pEndpoint->Activate(__uuidof(*m_pAudioEndpointVolume), CLSCTX_ALL, nullptr, (void**) &m_pAudioEndpointVolume);
  if(FAILED(hr))
    throw std::runtime_error("Cannot open a handle to the audio volume session manager");
}

AudioVolumeInterface* AudioVolumeInterface::New(void) {
  return new AudioVolumeInterfaceWin;
}

float AudioVolumeInterfaceWin::GetVolume(void) {
  float level;
  m_pAudioEndpointVolume->GetMasterVolumeLevelScalar(&level);
  return level;
}

void AudioVolumeInterfaceWin::SetMute(bool mute) {
  BOOL bMute = mute ? TRUE : FALSE;
  HRESULT hr = m_pAudioEndpointVolume->SetMute(bMute, nullptr);
  if(FAILED(hr))
    throw std::runtime_error("Failed to set muting state");
}

bool AudioVolumeInterfaceWin::IsMuted(void) {
  BOOL bMute = FALSE;
  HRESULT hr = m_pAudioEndpointVolume->GetMute(&bMute);
  if(FAILED(hr))
    throw std::runtime_error("Failed to obtain muting state");
  return bMute != FALSE;
}

void AudioVolumeInterfaceWin::SetVolume(float volume) {
  m_pAudioEndpointVolume->SetMasterVolumeLevelScalar(volume, nullptr);
}
