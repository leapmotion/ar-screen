#pragma once

//Taken from SFML 2.1
#if _WIN32
struct HWND__;
// Window handle is HWND (HWND__*) on Windows
typedef HWND__* WindowHandle;

#elif __APPLE__
// Window handle is NSWindow (void*) on Mac OS X - Cocoa
typedef void* WindowHandle;

#else
// Window handle is Window (unsigned long) on Unix - X11
typedef unsigned long WindowHandle;

#endif