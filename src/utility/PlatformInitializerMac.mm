#include "stdafx.h"
#include "PlatformInitializerMac.h"
#include <Foundation/NSObjCRuntime.h>
#include <mach-o/dyld.h>
#include <objc/runtime.h>
#include <unistd.h>

#include <AppKit/NSApplication.h>
#include <ApplicationServices/ApplicationServices.h>
#include <Security/Authorization.h>

PlatformInitializer::PlatformInitializer(void)
{
  // Make sure that 'NSApp' is created
  [NSApplication sharedApplication];

  // Change the current directory to be that of the either the executable or,
  // preferably, the Resources directory if the executable is within an
  // application bundle.
  char execPath[PATH_MAX+1] = {0};
  uint32_t pathSize = sizeof(execPath);
  if (!_NSGetExecutablePath(execPath, &pathSize)) {
    char fullPath[PATH_MAX+1] = {0};
    if (realpath(execPath, fullPath)) {
      std::string path(fullPath);
      size_t pos = path.find_last_of('/');

      if (pos != std::string::npos) {
        path.erase(pos+1);
      }
      if (!path.empty()) {
        chdir(path.c_str());
      }
      const char* resources = "../Resources";
      if (!access(resources, R_OK)) {
        chdir(resources);
      }
    }
  }
}

PlatformInitializer::~PlatformInitializer(void)
{

}
