#include "stdafx.h"
#include "OSWindow.h"
#include "utility/SamplePrimitives.h"
#include "Primitives/Primitives.h"
#include "Leap/GL/Texture2.h"

OSWindow::OSWindow(void):
  m_zOrder(1)
{
}

OSWindow::~OSWindow(void)
{
}

std::shared_ptr<ImagePrimitive> OSWindow::GetWindowTexture(std::shared_ptr<ImagePrimitive> img) {
  auto sz = GetSize();
  return MakePatternedTexture((size_t) sz.width, (size_t) sz.height);
}
