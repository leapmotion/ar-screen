#pragma once

#include "Primitives.h"
#include <string>

struct texture_font_t;
struct texture_atlas_t;

class TextureFont {
public:
  TextureFont(float ptSize, const std::string& fontFilename, size_t atlasWidth = 512, size_t atlasHeight = 512);
  ~TextureFont();
  void Load(const std::wstring& additionalGlyphs = L"");
  unsigned int AtlasTextureID() const;
  void GlyphsToGeometry(const std::wstring& glyphs, PrimitiveGeometryMesh& mesh, float& totalWidth, float& totalHeight) const;
private:
  texture_font_t* m_Font;
  texture_atlas_t* m_Atlas;
  bool m_Loaded;
};
