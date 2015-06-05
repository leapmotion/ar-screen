// Copyright (c) 2010 - 2014 Leap Motion. All rights reserved. Proprietary and confidential.
#pragma once

#include "OSGeometry.h"

class OSScreenBase {
  public:
    OSScreenBase() : m_bounds(OSRectZero) {}
    virtual ~OSScreenBase() {}

    inline OSRect Bounds() const { return m_bounds; }
    inline OSPoint Origin() const { return m_bounds.origin; }
    inline OSSize Size() const { return m_bounds.size; }
    inline float X() const { return m_bounds.origin.x; }
    inline float Y() const { return m_bounds.origin.y; }
    inline float Width() const { return m_bounds.size.width; }
    inline float Height() const { return m_bounds.size.height; }

    inline OSPoint ClipPosition(const OSPoint& position) const {
      const OSRect& rect = m_bounds;
      const float minX = OSRectGetMinX(rect);
      const float minY = OSRectGetMinY(rect);
      const float maxX = OSRectGetMaxX(rect);
      const float maxY = OSRectGetMaxY(rect);
      float x = position.x;
      float y = position.y;
      if (x <= minX) { x = minX; } else if (x >= maxX) { x = maxX - 1; }
      if (y <= minY) { y = minY; } else if (y >= maxY) { y = maxY - 1; }
      return OSPointMake(x, y);
    }

    inline OSPoint Normalize(const OSPoint& position) const {
      const auto& origin = m_bounds.origin;
      const auto& size = m_bounds.size;
      if (size.width > 0 && size.height > 0) {
        return OSPointMake((position.x - origin.x)/size.width, (position.y - origin.y)/size.height);
      }
      return OSPointZero;
    }

    inline OSPoint Denormalize(const OSPoint& position) const {
      const auto& origin = m_bounds.origin;
      const auto& size = m_bounds.size;

      return OSPointMake(position.x*size.width + origin.x, position.y*size.height + origin.y);
    }

    inline float AspectRatio() const {
      if (Height() < 1) {
        return 1.0f;
      }
      return Width()/Height();
    }

  protected:
    OSRect m_bounds;
};
