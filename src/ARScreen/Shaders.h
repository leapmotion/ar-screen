#pragma once

namespace Shaders {

  static std::string imagesFrag = R"shader(
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
uniform float stencil_opacity;
uniform bool use_color;

float width = 672.0;
float height = 600.0;
float irscale = 1.0;

float corr_ir_g = 0.1;
float corr_ir_rb = 0.1;
float corr_r_b = 0.2;
float corr_g_rb = 0.2;

float redShift = 0.44;
float greenShift = 0;

vec2 r_offset = vec2(-0.5, 0);
vec2 g_offset = vec2(-0.5, 0.5);
vec2 b_offset = vec2(0, 0.5);

void main(void) {
  vec2 texCoord = texture2D(distortion, out_tex_coord).xy;

  float iramt;
  if (use_color) {
    float dx = 1.0/width;
    float dy = 1.0/height;

    float rscale = brightness*(1.0 + redShift - 0.5*greenShift);
    float gscale = brightness*(1.0 + greenShift);
    float bscale = brightness*(1.0 - redShift - 0.5*greenShift);
   
    vec2 redOffset = vec2(dx, dy)*r_offset;
    vec2 greenOffset = vec2(dx, dy)*g_offset;
    vec2 blueOffset = vec2(dx, dy)*b_offset;

    float ir_lf = texture2D(texture, texCoord).a;
    float ir_l = texture2D(texture, texCoord + vec2(-dx, 0)).a;
    float ir_t = texture2D(texture, texCoord + vec2(0, -dy)).a;
    float ir_r = texture2D(texture, texCoord + vec2(dx, 0)).a;
    float ir_b = texture2D(texture, texCoord + vec2(0, dy)).a;
    float ir_hf = ir_lf - 0.25*(ir_l + ir_t + ir_r + ir_b);

    float r_lf = texture2D(texture, texCoord + redOffset).b;
    float r_l = texture2D(texture, texCoord + redOffset + vec2(-dx, 0)).b;
    float r_t = texture2D(texture, texCoord + redOffset + vec2(0, -dy)).b;
    float r_r = texture2D(texture, texCoord + redOffset + vec2(dx, 0)).b;
    float r_b = texture2D(texture, texCoord + redOffset + vec2(0, dy)).b;
    float r_hf = r_lf - 0.25*(r_l + r_t + r_r + r_b);

    float g_lf = texture2D(texture, texCoord + greenOffset).r;
    float g_l = texture2D(texture, texCoord + greenOffset + vec2(-dx, 0)).r;
    float g_t = texture2D(texture, texCoord + greenOffset + vec2(0, -dy)).r;
    float g_r = texture2D(texture, texCoord + greenOffset + vec2(dx, 0)).r;
    float g_b = texture2D(texture, texCoord + greenOffset + vec2(0, dy)).r;
    float g_hf = g_lf - 0.25*(g_l + g_t + g_r + g_b);

    float b_lf = texture2D(texture, texCoord + blueOffset).g;
    float b_l = texture2D(texture, texCoord + blueOffset + vec2(-dx, 0)).g;
    float b_t = texture2D(texture, texCoord + blueOffset + vec2(0, -dy)).g;
    float b_r = texture2D(texture, texCoord + blueOffset + vec2(dx, 0)).g;
    float b_b = texture2D(texture, texCoord + blueOffset + vec2(0, dy)).g;
    float b_hf = b_lf - 0.25*(b_l + b_t + b_r + b_b);
    
    const mat4 transformation = mat4(5.6220, -1.5456, 0.3634, -0.1106, -1.6410, 3.1944, -1.7204, 0.0189, 0.1410, 0.4896, 10.8399, -0.1053, -3.7440, -1.9080, -8.6066, 1.0000);
    const mat4 conservative = mat4(5.6220, 0.0000, 0.3634, 0.0000, 0.0000, 3.1944, 0.0000, 0.0189, 0.1410, 0.4896, 10.8399, 0.0000, 0.0000, 0.0000, 0.0000, 1.0000);

    const mat4 transformation_filtered = mat4(5.0670, -1.2312, 0.8625, -0.0507, -1.5210, 3.1104, -2.0194, 0.0017, -0.8310, -0.3000, 13.1744, -0.1052, -2.4540, -1.3848, -10.9618, 1.0000);
    const mat4 conservative_filtered = mat4(5.0670, 0.0000, 0.8625, 0.0000, 0.0000, 3.1104, 0.0000, 0.0017, 0.0000, 0.0000, 13.1744, 0.0000, 0.0000, 0.0000, 0.0000, 1.0000);

    vec4 input_lf = vec4(r_lf, g_lf, b_lf, ir_lf);

    // input_lf = bilateral_a*bilateral(texCoord, input_lf) + (1-bilateral_a)*input_lf;
    input_lf.r += ir_hf*corr_ir_rb + g_hf*corr_g_rb + b_hf*corr_r_b;
    input_lf.g += ir_hf*corr_ir_g + r_hf*corr_g_rb + b_hf*corr_g_rb;
    input_lf.b += ir_hf*corr_ir_rb + r_hf*corr_r_b + g_hf*corr_g_rb;

    vec4 output_lf, output_lf_fudge;
    output_lf = transformation*input_lf;
    output_lf_fudge = conservative*input_lf;
    //vec4 output_lf_gray = gray*input_lf;

    float fudge_threshold = 0.5;
    float ir_fudge_threshold = 0.95;
    float ir_fudge_factor = 0.333*(r_lf + g_lf + b_lf);

    float rfudge = r_lf > fudge_threshold ? (r_lf - fudge_threshold)/(1.0 - fudge_threshold) : 0;
    float gfudge = g_lf > fudge_threshold ? (g_lf - fudge_threshold)/(1.0 - fudge_threshold) : 0;
    float bfudge = b_lf > fudge_threshold ? (b_lf - fudge_threshold)/(1.0 - fudge_threshold) : 0;
    float irfudge = ir_fudge_factor > ir_fudge_threshold ? (ir_fudge_factor - ir_fudge_threshold)/(1.0 - ir_fudge_threshold) : 0;
    rfudge *= rfudge;
    gfudge *= gfudge;
    bfudge *= bfudge;
    irfudge *= irfudge;

    gl_FragColor.r = rfudge*output_lf_fudge.r + (1-rfudge)*output_lf.r;
    gl_FragColor.g = gfudge*output_lf_fudge.g + (1-gfudge)*output_lf.g;
    gl_FragColor.b = bfudge*output_lf_fudge.b + (1-bfudge)*output_lf.b;
    float ir_out = irfudge*output_lf_fudge.a + (1-irfudge)*output_lf.a;

    gl_FragColor.r *= rscale;
    gl_FragColor.g *= gscale;
    gl_FragColor.b *= bscale;
    ir_out *= irscale;

    iramt = pow(ir_out, gamma);

    gl_FragColor.rgb = pow(gl_FragColor.rgb, vec3(gamma));
    gl_FragColor.a = 1.0;
  } else {
    gl_FragColor.rgb = brightness*vec3(pow(texture2D(texture, texCoord).r, gamma));
    gl_FragColor.a = 1.0;
  }

  if (use_stencil) {
    float offset = 0.35;
    float value = gl_FragColor.r;
    if (use_color) {
      value = iramt;
    }
    float mult = clamp((value - offset) * 10.0, 0.0, 1.0);
    gl_FragColor.a = stencil_opacity*smoothstep(0.0, 1.0, mult);
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
uniform bool use_color;

// screen
uniform float view_x;
uniform float view_width;
uniform float view_height;

uniform float l00;
uniform float l11;
uniform float l03;
uniform float opacity;

float width = 672.0;
float height = 600.0;
float irscale = 1.0;

float corr_ir_g = 0.1;
float corr_ir_rb = 0.1;
float corr_r_b = 0.2;
float corr_g_rb = 0.2;

float redShift = 0.44;
float greenShift = 0;

vec2 r_offset = vec2(-0.5, 0);
vec2 g_offset = vec2(-0.5, 0.5);
vec2 b_offset = vec2(0, 0.5);

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

  float iramt;
  if (use_color) {
    float dx = 1.0/width;
    float dy = 1.0/height;

    float rscale = brightness*(1.0 + redShift - 0.5*greenShift);
    float gscale = brightness*(1.0 + greenShift);
    float bscale = brightness*(1.0 - redShift - 0.5*greenShift);
   
    vec2 redOffset = vec2(dx, dy)*r_offset;
    vec2 greenOffset = vec2(dx, dy)*g_offset;
    vec2 blueOffset = vec2(dx, dy)*b_offset;

    float ir_lf = texture2D(texture, texCoord).a;
    float ir_l = texture2D(texture, texCoord + vec2(-dx, 0)).a;
    float ir_t = texture2D(texture, texCoord + vec2(0, -dy)).a;
    float ir_r = texture2D(texture, texCoord + vec2(dx, 0)).a;
    float ir_b = texture2D(texture, texCoord + vec2(0, dy)).a;
    float ir_hf = ir_lf - 0.25*(ir_l + ir_t + ir_r + ir_b);

    float r_lf = texture2D(texture, texCoord + redOffset).b;
    float r_l = texture2D(texture, texCoord + redOffset + vec2(-dx, 0)).b;
    float r_t = texture2D(texture, texCoord + redOffset + vec2(0, -dy)).b;
    float r_r = texture2D(texture, texCoord + redOffset + vec2(dx, 0)).b;
    float r_b = texture2D(texture, texCoord + redOffset + vec2(0, dy)).b;
    float r_hf = r_lf - 0.25*(r_l + r_t + r_r + r_b);

    float g_lf = texture2D(texture, texCoord + greenOffset).r;
    float g_l = texture2D(texture, texCoord + greenOffset + vec2(-dx, 0)).r;
    float g_t = texture2D(texture, texCoord + greenOffset + vec2(0, -dy)).r;
    float g_r = texture2D(texture, texCoord + greenOffset + vec2(dx, 0)).r;
    float g_b = texture2D(texture, texCoord + greenOffset + vec2(0, dy)).r;
    float g_hf = g_lf - 0.25*(g_l + g_t + g_r + g_b);

    float b_lf = texture2D(texture, texCoord + blueOffset).g;
    float b_l = texture2D(texture, texCoord + blueOffset + vec2(-dx, 0)).g;
    float b_t = texture2D(texture, texCoord + blueOffset + vec2(0, -dy)).g;
    float b_r = texture2D(texture, texCoord + blueOffset + vec2(dx, 0)).g;
    float b_b = texture2D(texture, texCoord + blueOffset + vec2(0, dy)).g;
    float b_hf = b_lf - 0.25*(b_l + b_t + b_r + b_b);
    
    const mat4 transformation = mat4(5.6220, -1.5456, 0.3634, -0.1106, -1.6410, 3.1944, -1.7204, 0.0189, 0.1410, 0.4896, 10.8399, -0.1053, -3.7440, -1.9080, -8.6066, 1.0000);
    const mat4 conservative = mat4(5.6220, 0.0000, 0.3634, 0.0000, 0.0000, 3.1944, 0.0000, 0.0189, 0.1410, 0.4896, 10.8399, 0.0000, 0.0000, 0.0000, 0.0000, 1.0000);

    const mat4 transformation_filtered = mat4(5.0670, -1.2312, 0.8625, -0.0507, -1.5210, 3.1104, -2.0194, 0.0017, -0.8310, -0.3000, 13.1744, -0.1052, -2.4540, -1.3848, -10.9618, 1.0000);
    const mat4 conservative_filtered = mat4(5.0670, 0.0000, 0.8625, 0.0000, 0.0000, 3.1104, 0.0000, 0.0017, 0.0000, 0.0000, 13.1744, 0.0000, 0.0000, 0.0000, 0.0000, 1.0000);

    vec4 input_lf = vec4(r_lf, g_lf, b_lf, ir_lf);

    input_lf.r += ir_hf*corr_ir_rb + g_hf*corr_g_rb + b_hf*corr_r_b;
    input_lf.g += ir_hf*corr_ir_g + r_hf*corr_g_rb + b_hf*corr_g_rb;
    input_lf.b += ir_hf*corr_ir_rb + r_hf*corr_r_b + g_hf*corr_g_rb;

    vec4 output_lf, output_lf_fudge;
    output_lf = transformation*input_lf;
    output_lf_fudge = conservative*input_lf;

    float fudge_threshold = 0.5;
    float ir_fudge_threshold = 0.95;
    float ir_fudge_factor = 0.333*(r_lf + g_lf + b_lf);

    float rfudge = r_lf > fudge_threshold ? (r_lf - fudge_threshold)/(1.0 - fudge_threshold) : 0;
    float gfudge = g_lf > fudge_threshold ? (g_lf - fudge_threshold)/(1.0 - fudge_threshold) : 0;
    float bfudge = b_lf > fudge_threshold ? (b_lf - fudge_threshold)/(1.0 - fudge_threshold) : 0;
    float irfudge = ir_fudge_factor > ir_fudge_threshold ? (ir_fudge_factor - ir_fudge_threshold)/(1.0 - ir_fudge_threshold) : 0;
    rfudge *= rfudge;
    gfudge *= gfudge;
    bfudge *= bfudge;
    irfudge *= irfudge;

    gl_FragColor.r = rfudge*output_lf_fudge.r + (1-rfudge)*output_lf.r;
    gl_FragColor.g = gfudge*output_lf_fudge.g + (1-gfudge)*output_lf.g;
    gl_FragColor.b = bfudge*output_lf_fudge.b + (1-bfudge)*output_lf.b;
    float ir_out = irfudge*output_lf_fudge.a + (1-irfudge)*output_lf.a;

    gl_FragColor.r *= rscale;
    gl_FragColor.g *= gscale;
    gl_FragColor.b *= bscale;
    ir_out *= irscale;

    iramt = smoothstep(0.0, 1.0, pow(clamp((ir_out-0.01)*10, 0.0, 1.0), gamma));

    gl_FragColor.rgb = pow(gl_FragColor.rgb, vec3(gamma));
    gl_FragColor.a = 1.0;
  } else {
    gl_FragColor.rgb = brightness*vec3(pow(texture2D(texture, texCoord).r, gamma));
    gl_FragColor.a = 1.0;
  }

  if (use_stencil) {
    float value = gl_FragColor.r;
    if (use_color) { 
      value = iramt;
    }

    float offset1 = 0.01;
    float offset2 = 0.3;
    float falloff = 0.5;

    vec3 glowColor = vec3(0.7, 0.9, 1.0);
    if (value < offset1) {
      gl_FragColor.a = 0.0;
    } else if (value >= offset1 && value <= offset2) {
      float ratio = (value - offset1)/(offset2 - offset1);
      float fade = smoothstep(0.0, 1.0, ratio);
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
  
  vec3 eye = normalize(-out_position);
  vec3 normal = normalize(out_normal);
  float edgeMult = dot(normal, eye);

  gl_FragColor.a *= (edgeMult * edgeMult);

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