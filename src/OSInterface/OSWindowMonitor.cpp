#include "stdafx.h"
#include "OSWindowMonitor.h"
#include "OSAppManager.h"

OSWindowMonitor::OSWindowMonitor(void):
  ContextMember("OSWindowMonitor")
{
  AutoRequired<OSAppManager>();
}

OSWindowMonitor::~OSWindowMonitor(void)
{
}

void OSWindowMonitor::Tick(std::chrono::duration<double> deltaT) {
  if (m_scanEnabled) {
    Scan();
  }
}
