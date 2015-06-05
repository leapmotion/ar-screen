// Copyright (c) 2010 - 2014 Leap Motion. All rights reserved. Proprietary and confidential.
#pragma once

#if __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#else
#if _WIN32
  #undef min //stupid windows.h...
  #undef max
#endif
#include <algorithm>
#endif

#if __APPLE__
using OSPoint = CGPoint;
using OSSize = CGSize;
using OSRect = CGRect;
static const OSPoint& OSPointZero = CGPointZero;
static const OSSize& OSSizeZero = CGSizeZero;
static const OSRect& OSRectZero = CGRectZero;
static inline OSPoint OSPointMake(float x, float y) { return CGPointMake(x, y); }
static inline OSSize OSSizeMake(float width, float height) { return CGSizeMake(width, height); }
static inline OSRect OSRectMake(float x, float y, float width, float height) { return CGRectMake(x, y, width, height); }
static inline float OSRectGetMinX(const OSRect& r) { return CGRectGetMinX(r); }
static inline float OSRectGetMinY(const OSRect& r) { return CGRectGetMinY(r); }
static inline float OSRectGetMaxX(const OSRect& r) { return CGRectGetMaxX(r); }
static inline float OSRectGetMaxY(const OSRect& r) { return CGRectGetMaxY(r); }
static inline bool OSRectContainsPoint(const OSRect& r, const OSPoint& p) { return CGRectContainsPoint(r, p); }
static inline OSRect OSRectUnion(const OSRect& r1, const OSRect& r2) { return CGRectUnion(r1, r2); }
#else
struct OSPoint {
  OSPoint(float _x = 0, float _y = 0) : x(_x), y(_y) {}
  float x;
  float y;
};

struct OSSize {
  OSSize(float _width = 0, float _height = 0) : width(_width), height(_height) {}
  float width;
  float height;
};

struct OSRect {
  OSRect(float _x = 0, float _y = 0, float _width = 0, float _height = 0) : origin(_x, _y), size(_width, _height) {}
  OSPoint origin;
  OSSize size;
};

static OSPoint OSPointZero;
static inline OSPoint OSPointMake(float x, float y) { return OSPoint(x, y); }
static OSSize OSSizeZero;
static inline OSSize OSSizeMake(float width, float height) { return OSSize(width, height); }
static OSRect OSRectZero;
static inline OSRect OSRectMake(float x, float y, float width, float height) {
  return OSRect(x, y, width, height);
}

static inline float OSRectGetMinX(const OSRect& r) { return r.origin.x; }
static inline float OSRectGetMinY(const OSRect& r) { return r.origin.y; }
static inline float OSRectGetMaxX(const OSRect& r) { return (r.origin.x + r.size.width); }
static inline float OSRectGetMaxY(const OSRect& r) { return (r.origin.y + r.size.height); }

static inline bool OSRectContainsPoint(const OSRect& r, const OSPoint& p) {
  return (p.x >= r.origin.x && p.x < (r.origin.x + r.size.width) &&
          p.y >= r.origin.y && p.y < (r.origin.y + r.size.height));
}

static inline OSRect OSRectUnion(const OSRect& r1, const OSRect& r2)
{
  float x0 = std::min(r1.origin.x, r2.origin.x);
  float x1 = std::max(r1.origin.x + r1.size.width,
                        r2.origin.x + r2.size.width);
  float y0 = std::min(r1.origin.y, r2.origin.y);
  float y1 = std::max(r1.origin.y + r1.size.height,
                        r2.origin.y + r2.size.height);
  return OSRect(x0, y0, x1 - x0, y1 - y0);
}
#endif

#if !_WIN32
typedef struct _RECT {
  int32_t left;
  int32_t right;
  int32_t top;
  int32_t bottom;
} RECT, *PRECT;

typedef struct _POINT {
  int32_t x;
  int32_t y;
} POINT, *PPOINT;

typedef struct _SIZE {
  int32_t cx;
  int32_t cy;
} SIZE, *PSIZE;
#endif
