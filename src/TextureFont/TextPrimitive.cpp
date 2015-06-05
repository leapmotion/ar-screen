#include "stdafx.h"
#include "TextPrimitive.h"
#include "utility/Shaders.h"

TextPrimitive::TextPrimitive() : m_size(EigenTypes::Vector2::Zero()), m_atlasID(0) { }

void TextPrimitive::SetText(const std::wstring& text, const std::shared_ptr<TextureFont>& font) {
  // origin of the primitive is located at bottom left corner of entire string
  m_font = font;
  float width = 0.0f;
  float height = 0.0f;
  font->GlyphsToGeometry(text, m_mesh, width, height);
  m_atlasID = font->AtlasTextureID();
  m_size << width, height;
  SetShader(getFontShader());
  Material().Uniform<AMBIENT_LIGHTING_PROPORTION>() = 1.0f;
  Material().Uniform<AMBIENT_LIGHT_COLOR>() = Leap::GL::Rgba<float>(1.0f);
  Material().Uniform<TEXTURE_MAPPING_ENABLED>() = true;
}

void TextPrimitive::DrawContents(RenderState& renderState) const {
  if (Material().Uniform<AMBIENT_LIGHT_COLOR>().A() < 0.0001f) {
    return;
  }
  glBindTexture(GL_TEXTURE_2D, m_atlasID);
  const Leap::GL::Shader &shader = Shader();
  auto locations = std::make_tuple(shader.LocationOfAttribute("position"),
                                   shader.LocationOfAttribute("normal"),
                                   shader.LocationOfAttribute("tex_coord"),
                                   shader.LocationOfAttribute("color"));
  m_mesh.Bind(locations);
  m_mesh.Draw();
  m_mesh.Unbind(locations);
  glBindTexture(GL_TEXTURE_2D, 0);
}

std::shared_ptr<Leap::GL::Shader> TextPrimitive::getFontShader() const {
  static std::shared_ptr<Leap::GL::Shader> shader;
  if (!shader) {
    static const std::string frag = R"frag(
#version 120

// These are the inputs from the vertex shader to the fragment shader, and must appear identically there.
varying vec3 out_position;
varying vec3 out_normal;
varying vec2 out_tex_coord;

uniform vec3 light_position;                // The position of the (single) light for diffuse reflectance.  It is assumed to be white.
uniform vec4 diffuse_light_color;           // The color for diffuse lighting.
uniform vec4 ambient_light_color;           // The color for ambient lighting.
uniform float ambient_lighting_proportion;  // Lighting color for each fragment is determined by linearly interpolating between and 
                                            // ambient lighting colors.  This variable is in the range [0,1].  A value of 0 or 1
                                            // specifies that the color is entirely diffuse or ambient, respectively.
uniform bool use_texture;                   // True iff texture mapping is to be used.
uniform sampler2D texture;                  // Defines the texture if texture mapping is to be used.

void main() {
  // Compute diffuse brightness: a value in [0,1] giving the proportion of reflected light from the light source.
  vec3 surface_normal = normalize(out_normal);
  vec3 light_dir = normalize(light_position - out_position);
  float diffuse_brightness = max(0.0, dot(light_dir, surface_normal));
  
  // Blend the ambient and diffuse lighting.

  vec4 diffuse_color = diffuse_light_color;
  diffuse_color.rgb = diffuse_brightness*diffuse_color.rgb;
  gl_FragColor = ambient_lighting_proportion*ambient_light_color + (1.0-ambient_lighting_proportion)*diffuse_color;
  // If texturing is enabled, include its influence in the color.
  if (use_texture) {
    // The fragment color is used as a color mask, hence the multiplication.
    gl_FragColor.a *= texture2D(texture, out_tex_coord).r;
  }
}
    )frag";

    shader = std::shared_ptr<Leap::GL::Shader>(new Leap::GL::Shader(Shaders::transformedVert, frag));
  }
  return shader;
}
