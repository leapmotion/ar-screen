#include "stdafx.h"
#include "AudioVolumeInterfaceMac.h"

AudioVolumeInterfaceMac::AudioVolumeInterfaceMac(void) {
}

AudioVolumeInterface* AudioVolumeInterface::New(void) {
  return new AudioVolumeInterfaceMac;
}

float AudioVolumeInterfaceMac::GetVolume(void) {
  float volume = 0;
  AudioDeviceID audioDevice = GetAudioDeviceID();
  if (!audioDevice) {
    return volume;
  }
  // Try to get the master channel. If it fails, try to then get left channel and right channel
  for (AudioObjectPropertyElement channel = 0; channel < 3; channel++) {
    AudioObjectPropertyAddress propertyAddress = { kAudioDevicePropertyVolumeScalar,
                                                   kAudioDevicePropertyScopeOutput,
                                                   channel };
    Float32 theVolume = 0;
    if (AudioObjectHasProperty(audioDevice, &propertyAddress)) {
      UInt32 size = sizeof(theVolume);
      AudioObjectGetPropertyData(audioDevice, &propertyAddress, 0, nullptr, &size, &theVolume);
      float channelVolume = static_cast<float>(theVolume);
      if (channelVolume > volume) {
        volume = channelVolume;
      }
      if (channel == 0) { // If master channel, nothing left to do
        break;
      }
    }
  }
  return volume;
}

void AudioVolumeInterfaceMac::SetVolume(float volume) {
  AudioDeviceID audioDevice = GetAudioDeviceID();
  if (!audioDevice) {
    return;
  }
  // Try to set the master channel. If it fails, try to then set left channel and right channel
  for (AudioObjectPropertyElement channel = 0; channel < 3; channel++) {
    AudioObjectPropertyAddress propertyAddress = { kAudioDevicePropertyVolumeScalar,
                                                   kAudioDevicePropertyScopeOutput,
                                                   channel };
    if (AudioObjectHasProperty(audioDevice, &propertyAddress)) {
      Float32 theVolume = static_cast<Float32>(volume);
      UInt32 size = sizeof(theVolume);
      AudioObjectSetPropertyData(audioDevice, &propertyAddress, 0, nullptr, size, &theVolume);
      if (channel == 0) { // If master channel, nothing left to do
        break;
      }
    }
  }
}

void AudioVolumeInterfaceMac::SetMute(bool mute) {
  AudioDeviceID audioDevice = GetAudioDeviceID();
  if (!audioDevice) {
    return;
  }
  // Try to set the master channel. If it fails, try to then set left channel and right channel
  for (AudioObjectPropertyElement channel = 0; channel < 3; channel++) {
    AudioObjectPropertyAddress propertyAddress = { kAudioDevicePropertyMute,
                                                   kAudioDevicePropertyScopeOutput,
                                                   channel };
    if (AudioObjectHasProperty(audioDevice, &propertyAddress)) {
      UInt32 theMute = mute ? 1 : 0;
      UInt32 size = sizeof(theMute);
      AudioObjectSetPropertyData(audioDevice, &propertyAddress, 0, nullptr, size, &theMute);
      if (channel == 0) { // If master channel, nothing left to do
        break;
      }
    }
  }
}

bool AudioVolumeInterfaceMac::IsMuted(void) {
  bool muted = false;
  bool setMutedState = false;
  AudioDeviceID audioDevice = GetAudioDeviceID();
  if (!audioDevice) {
    return muted;
  }
  // Try to get the master channel. If it fails, try to then get left channel and right channel
  for (AudioObjectPropertyElement channel = 0; channel < 3; channel++) {
    AudioObjectPropertyAddress propertyAddress = { kAudioDevicePropertyMute,
                                                   kAudioDevicePropertyScopeOutput,
                                                   channel };
    UInt32 theMute = 0;
    if (AudioObjectHasProperty(audioDevice, &propertyAddress)) {
      UInt32 size = sizeof(theMute);
      AudioObjectGetPropertyData(audioDevice, &propertyAddress, 0, nullptr, &size, &theMute);
      muted = (theMute != 0) && (!setMutedState || muted);
      if (channel == 0) { // If master channel, nothing left to do
        break;
      }
      setMutedState = true;
    }
  }
  return muted;
}

AudioDeviceID AudioVolumeInterfaceMac::GetAudioDeviceID() {
  AudioObjectPropertyAddress outputDeviceAddress = { kAudioHardwarePropertyDefaultOutputDevice,
                                                     kAudioObjectPropertyScopeGlobal,
                                                     kAudioObjectPropertyElementMaster };
  AudioDeviceID outputDeviceID = 0;
  UInt32 size = sizeof(AudioDeviceID);
  AudioObjectGetPropertyData(kAudioObjectSystemObject, &outputDeviceAddress, 0, nullptr, &size, &outputDeviceID);
  return outputDeviceID;
}
