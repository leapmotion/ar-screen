#include "stdafx.h"
#include "MediaInterfaceWin.h"

MediaInterface* MediaInterface::New(void) {
  return new MediaInterfaceWin;
}

void MediaInterfaceWin::SendMediaKeyCode(uint32_t vk) {
  keybd_event(vk, 0x22, KEYEVENTF_EXTENDEDKEY, 0);
  keybd_event(vk, 0x22, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
}

void MediaInterfaceWin::PlayPause(void) {
  SendMediaKeyCode(VK_MEDIA_PLAY_PAUSE);
}

void MediaInterfaceWin::Stop(void) {
  SendMediaKeyCode(VK_MEDIA_STOP);
}

void MediaInterfaceWin::Next(void) {
  SendMediaKeyCode(VK_MEDIA_NEXT_TRACK);
}

void MediaInterfaceWin::Prev(void) {
  SendMediaKeyCode(VK_MEDIA_PREV_TRACK);
}

void MediaInterfaceWin::VolumeUp(void) {
  SendMediaKeyCode(VK_VOLUME_UP);
}

void MediaInterfaceWin::VolumeDown(void) {
  SendMediaKeyCode(VK_VOLUME_DOWN);
}

void MediaInterfaceWin::Mute(void) {
  SendMediaKeyCode(VK_VOLUME_MUTE);
}
