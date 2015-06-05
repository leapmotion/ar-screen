#pragma once

class MediaInterface:
  public ContextMember
{
public:
  MediaInterface(void);
  ~MediaInterface(void);

  static MediaInterface* New(void);

public:
  virtual void PlayPause(void) = 0;
  virtual void Stop(void) = 0;
  virtual void Next(void) = 0;
  virtual void Prev(void) = 0;
  virtual void VolumeUp(void) = 0;
  virtual void VolumeDown(void) = 0;
  virtual void Mute(void) = 0;
};

