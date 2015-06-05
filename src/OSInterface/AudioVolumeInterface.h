#pragma once

class AudioVolumeInterface:
  public ContextMember
{
public:
  AudioVolumeInterface(void);

  static AudioVolumeInterface* New(void);

private:

public:
  /// <returns>
  /// Volume on current primary audio device, in the range [0, 1], referring to the percentage of maximum value
  /// </returns>
  virtual float GetVolume(void) = 0;

  /// <summary>
  /// Mutator counterpart to GetVolume
  /// </returns>
  virtual void SetVolume(float volume) = 0;

  /// <summary>
  /// Turn on or off muting of the primary audio device
  /// </summary>
  virtual void SetMute(bool mute) = 0;

  /// <returns>
  /// Indicate whether or not the volume control for the primary audio device is muted
  /// </returns>
  virtual bool IsMuted(void) = 0;
};
