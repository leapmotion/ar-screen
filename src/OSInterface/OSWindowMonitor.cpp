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
