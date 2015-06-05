#pragma once

namespace Shaders {

  static std::string imagesFrag = R"shader(
#version 120

//ray. This is specialized for overlays
//varying vec2 frag_ray;
varying vec3 out_position;
varying vec3 out_normal;
varying vec2 out_tex_coord;

// original texture
uniform bool use_texture;
uniform sampler2D texture;

// material
uniform vec4 diffuse_light_color;
uniform vec4 ambient_light_color;
uniform float ambient_lighting_proportion;

// distortion maps
uniform sampler2D distortion;

// controls
uniform float gamma;
uniform float brightness;
uniform bool use_stencil;
uniform float stencil_opacity;

void main(void) {
  vec2 texCoord = texture2D(distortion, out_tex_coord).xy;// +vec2(0.125, 0.125);

  gl_FragColor.rgb = brightness*vec3(pow(texture2D(texture, texCoord).r, gamma));
  gl_FragColor.a = 1.0;

  if (use_stencil) {
    float offset = 0.35;
    float mult = clamp((gl_FragColor.r - offset) * 10.0, 0.0, 1.0);
    gl_FragColor.a = stencil_opacity*mult;
  }
}
)shader";

  static std::string imagesHandsFrag = R"shader(
#version 120

varying vec3 out_position;
varying vec3 out_normal;
varying vec2 out_tex_coord;

// original texture
uniform bool use_texture;
uniform sampler2D texture;

// material
uniform vec4 diffuse_light_color;
uniform vec4 ambient_light_color;
uniform float ambient_lighting_proportion;

// distortion maps
uniform sampler2D distortion;

// controls
uniform float gamma;
uniform float brightness;
uniform bool use_stencil;

// screen
uniform float view_x;
uniform float view_width;
uniform float view_height;

uniform float l00;
uniform float l11;
uniform float l03;
uniform float opacity;

void main(void) {
  vec2 coord = gl_FragCoord.xy;
  coord.x = (coord.x - view_x) / view_width;
  coord.y = coord.y / view_height;
  
  coord = 2.0 * coord - vec2(1.0);

  vec2 temp1 = vec2(l03, 0.0);
  vec2 temp = vec2(l00, l11);

  vec2 tangent = (coord + temp1) / temp;

  vec2 outTexCoord = 0.125 * tangent + 0.5;

  vec2 texCoord = texture2D(distortion, outTexCoord).xy;

  gl_FragColor.rgb = brightness*vec3(pow(texture2D(texture, texCoord).r, gamma));
  gl_FragColor.a = 1.0;

  if (use_stencil) {
    float value = gl_FragColor.r;

    float offset1 = 0.175;
    float offset2 = 0.35;
    float falloff = 0.15;

    vec3 glowColor = vec3(0.6, 0.85, 1.0);
    if (value < offset1) {
      gl_FragColor.a = 0.0;
    } else if (value >= offset1 && value <= offset2) {
      float fade = smoothstep(0.0, 1.0, (value - offset1)/(offset2 - offset1));
      gl_FragColor.rgb = glowColor;
      gl_FragColor.a = fade*opacity;
    } else {
      float mult = smoothstep(0.0, 1.0, clamp((value - offset2) * 10.0, 0.0, 1.0));
      float fade = 1.0 - smoothstep(0.0, 1.0, clamp((value - offset2)/falloff, 0.0, 1.0));
      gl_FragColor.rgb = fade*glowColor + (1.0-fade)*gl_FragColor.rgb;
      float alpha = fade + (1.0 - fade)*mult;
      gl_FragColor.a = opacity*alpha;
    }
  }
}
)shader";

  static std::string transformedVert = R"shader(
#version 120

uniform mat4 projection_times_model_view_matrix;
uniform mat4 model_view_matrix;
uniform mat4 normal_matrix;

// attribute arrays
attribute vec3 position;
attribute vec3 normal;
attribute vec2 tex_coord;

// These are the inputs from the vertex shader to the fragment shader, and must appear identically there.
varying vec3 out_position;
varying vec3 out_normal;
varying vec2 out_tex_coord;

void main() {
  gl_Position = projection_times_model_view_matrix * vec4(position, 1.0);
  out_position = (model_view_matrix * vec4(position, 1.0)).xyz;
  out_normal = (normal_matrix * vec4(normal, 0.0)).xyz;
  out_tex_coord = tex_coord;
}
)shader";

};