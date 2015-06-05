#include "TextureFont.h"
#include "freetype-gl.h"
#include <algorithm>
#include <assert.h>

static const std::wstring SUPPORTED_GLYPHS = L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

TextureFont::TextureFont(float ptSize, const std::string& fontFilename, size_t atlasWidth, size_t atlasHeight) :
  m_Font(nullptr),
  m_Atlas(nullptr),
  m_Loaded(false)
{
  m_Atlas = texture_atlas_new(atlasWidth, atlasHeight, 1);
  m_Font = texture_font_new_from_file(m_Atlas, ptSize, fontFilename.c_str());
}

TextureFont::~TextureFont() {
  texture_font_delete(m_Font);
  texture_atlas_delete(m_Atlas);
  m_Loaded = false;
}

void TextureFont::Load(const std::wstring& additionalGlyphs) {
  assert(m_Atlas);
  assert(m_Font);
  assert(!m_Loaded);

  std::wstring glyphs = SUPPORTED_GLYPHS + additionalGlyphs;

  // remove any duplicates
  std::sort(glyphs.begin(), glyphs.end());
  glyphs.erase(std::unique(glyphs.begin(), glyphs.end()), glyphs.end());

  // create texture atlas
  texture_font_load_glyphs(m_Font, glyphs.c_str());

  m_Loaded = true;
}

unsigned int TextureFont::AtlasTextureID() const {
  assert(m_Atlas);
  assert(m_Loaded);

  return m_Atlas->id;
}

void TextureFont::GlyphsToGeometry(const std::wstring& glyphs, PrimitiveGeometryMesh& mesh, float& totalWidth, float& totalHeight) const {
  assert(m_Font);
  assert(m_Atlas);
  assert(m_Loaded);

  mesh.Shutdown();

  PrimitiveGeometryMeshAssembler assembler(GL_TRIANGLES);
  auto GlyphVertex = [](float x, float y, float s, float t) {
    const EigenTypes::Vector3f pos(x, y, 0.0f);
    const EigenTypes::Vector3f normal(EigenTypes::Vector3f::UnitZ());
    const EigenTypes::Vector2f texCoords(s, t);
    const EigenTypes::Vector4f color(EigenTypes::Vector4f::Constant(1.0f));
    return PrimitiveGeometryMesh::VertexAttributes(pos, normal, texCoords, color);
  };

  float minX = FLT_MAX;
  float maxX = -FLT_MAX;
  float minY = FLT_MAX;
  float maxY = -FLT_MAX;
  float pen = 0.0f;

  for (size_t i = 0; i < glyphs.size(); i++) {
    texture_glyph_t* glyph = texture_font_get_glyph(m_Font, glyphs[i]);
    if (!glyph) {
      continue;
    }
    const float kerning = (i > 0) ? texture_glyph_get_kerning(glyph, glyphs[i - 1]) : 0.0f;
    pen += kerning;
    const float x0 = pen + static_cast<float>(glyph->offset_x);
    const float y0 = static_cast<float>(glyph->offset_y);
    const float x1 = x0 + static_cast<float>(glyph->width);
    const float y1 = y0 - static_cast<float>(glyph->height);

    minX = std::min(minX, x0);
    minY = std::min(minY, std::min(y0, y1));
    maxX = std::max(maxX, x1);
    maxY = std::max(maxY, std::max(y0, y1));

    const float s0 = glyph->s0;
    const float t0 = glyph->t0;
    const float s1 = glyph->s1;
    const float t1 = glyph->t1;

    assembler.PushTriangle(GlyphVertex(x0, y0, s0, t0), GlyphVertex(x0, y1, s0, t1), GlyphVertex(x1, y1, s1, t1));
    assembler.PushTriangle(GlyphVertex(x0, y0, s0, t0), GlyphVertex(x1, y1, s1, t1), GlyphVertex(x1, y0, s1, t0));

    pen += glyph->advance_x;
  }

  totalWidth = (maxX - minX);
  totalHeight = (maxY - minY);

  assembler.InitializeMesh(mesh);
  assert(mesh.IsInitialized());
}
