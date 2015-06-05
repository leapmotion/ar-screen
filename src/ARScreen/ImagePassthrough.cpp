#include "stdafx.h"
#include "ImagePassthrough.h"
#include "utility/Shaders.h"

ImagePassthrough::ImagePassthrough() :
  m_ActiveTexture(0),
  m_UseStencil(false),
  m_Color(false)
{
  for (int i=0; i<NUM_CAMERAS; i++) {
    m_ImageBytes[i] = 0;
    m_DistortionBytes[i] = 0;
  }
}

void ImagePassthrough::Init() {
  m_Shader = std::shared_ptr<Leap::GL::Shader>(new Leap::GL::Shader(Shaders::transformedVert, Shaders::imagesFrag));

  m_HandsShader = std::shared_ptr<Leap::GL::Shader>(new Leap::GL::Shader(Shaders::transformedVert, Shaders::imagesHandsFrag));

  m_Quad = std::shared_ptr<RectanglePrim>(new RectanglePrim());
  m_Quad->SetShader(m_Shader);
  m_Quad->Material().Uniform<TEXTURE_MAPPING_ENABLED>() = true;
  m_Quad->Material().Uniform<AMBIENT_LIGHTING_PROPORTION>() = 1.0f;
  m_Quad->Material().Uniform<AMBIENT_LIGHT_COLOR>() = Leap::GL::Rgba<float>(1.0f, 1.0f, 1.0f, 1.0f);

  m_Quad->LinearTransformation() = EigenTypes::Vector3(8, 8, 1).asDiagonal();// *RotationMatrixFromEulerAngles(M_PI / 2.0, 0.0, M_PI);
  m_Quad->Translation() << 0, 0, -1;
}

void ImagePassthrough::Update(const Leap::ImageList& images) {
  assert(images.count() == NUM_CAMERAS);
  for (int i=0; i<images.count(); i++) {
    updateImage(i, images[i]);
    updateDistortion(i, images[i]);
  }
}

void ImagePassthrough::DrawStencilObject(PrimitiveBase* obj, RenderState& renderState, float viewWidth, float viewX, float viewHeight, float l00, float l11, float l03, float opacity) const {
  if (m_ImageBytes[m_ActiveTexture] == 0 || m_DistortionBytes[m_ActiveTexture] == 0) {
    return;
  }
  if (opacity < 0.02f) {
    return;
  }
  m_HandsShader->Bind();
  m_HandsShader->UploadUniform<GL_FLOAT>("gamma", m_Color ? 0.56f : 0.8f);
  m_HandsShader->UploadUniform<GL_FLOAT>("brightness", 1.0f);
  m_HandsShader->UploadUniform<GL_BOOL>("use_texture", true);
  m_HandsShader->UploadUniform<GL_SAMPLER_2D>("texture", 0);
  m_HandsShader->UploadUniform<GL_SAMPLER_2D>("distortion", 1);
  m_HandsShader->UploadUniform<GL_BOOL>("use_stencil", m_UseStencil);
  m_HandsShader->UploadUniform<GL_BOOL>("use_color", m_Color);
  m_HandsShader->UploadUniform<GL_FLOAT>("view_width", viewWidth);
  m_HandsShader->UploadUniform<GL_FLOAT>("view_height", viewHeight);
  m_HandsShader->UploadUniform<GL_FLOAT>("view_x", viewX);
  m_HandsShader->UploadUniform<GL_FLOAT>("l00", l00);
  m_HandsShader->UploadUniform<GL_FLOAT>("l11", l11);
  m_HandsShader->UploadUniform<GL_FLOAT>("l03", l03);
  m_HandsShader->UploadUniform<GL_FLOAT>("opacity", opacity);
  m_HandsShader->Unbind();

  obj->SetShader(m_HandsShader);

  m_Textures[m_ActiveTexture]->Bind(0);
  m_Distortion[m_ActiveTexture]->Bind(1);
  PrimitiveBase::DrawSceneGraph(*obj, renderState);
  m_Distortion[m_ActiveTexture]->Unbind();
  m_Textures[m_ActiveTexture]->Unbind();
}

void ImagePassthrough::Draw(RenderState& renderState) const {
  if (m_ImageBytes[m_ActiveTexture] == 0 || m_DistortionBytes[m_ActiveTexture] == 0) {
    return;
  }
  m_Shader->Bind();
  m_Shader->UploadUniform<GL_FLOAT>("gamma", m_Color ? 0.56f : 0.8f);
  m_Shader->UploadUniform<GL_FLOAT>("brightness", 1.0f);
  m_Shader->UploadUniform<GL_BOOL>("use_texture", true);
  m_Shader->UploadUniform<GL_SAMPLER_2D>("texture", 0);
  m_Shader->UploadUniform<GL_SAMPLER_2D>("distortion", 1);
  m_Shader->UploadUniform<GL_BOOL>("use_stencil", m_UseStencil);
  m_Shader->UploadUniform<GL_BOOL>("use_color", m_Color);
  m_Shader->UploadUniform<GL_FLOAT>("stencil_opacity", 0.35f);
  m_Shader->Unbind();

  m_Quad->SetTexture(m_Textures[m_ActiveTexture]);

  m_Textures[m_ActiveTexture]->Bind(0);
  m_Distortion[m_ActiveTexture]->Bind(1);
  PrimitiveBase::DrawSceneGraph(*m_Quad, renderState);
  m_Distortion[m_ActiveTexture]->Unbind();
  m_Textures[m_ActiveTexture]->Unbind();
}

void ImagePassthrough::updateImage(int idx, const Leap::Image& image) {
  GLenum format = (image.width() == 640) ? GL_LUMINANCE : GL_RGBA;
  m_Color = (format == GL_RGBA);

  std::shared_ptr<Leap::GL::Texture2>& tex = m_Textures[idx];
  const unsigned char* data = image.data();
  const int width = image.width();
  const int height = image.height();
  const int bytesPerPixel = image.bytesPerPixel();
  const size_t numBytes = static_cast<size_t>(width * height * bytesPerPixel);
  Leap::GL::Texture2PixelData pixelData(format, GL_UNSIGNED_BYTE, data, numBytes);
  if (!tex || numBytes != m_ImageBytes[idx]) {
    Leap::GL::Texture2Params params(static_cast<GLsizei>(width), static_cast<GLsizei>(height));
    params.SetTarget(GL_TEXTURE_2D);
    params.SetInternalFormat(format);
    params.SetTexParameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    params.SetTexParameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    params.SetTexParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    params.SetTexParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    tex = std::shared_ptr<Leap::GL::Texture2>(new Leap::GL::Texture2(params, pixelData));
    m_ImageBytes[idx] = numBytes;
  } else {
    tex->TexSubImage(pixelData);
  }
}

void ImagePassthrough::updateDistortion(int idx, const Leap::Image& image) {
  std::shared_ptr<Leap::GL::Texture2>& distortion = m_Distortion[idx];
  const float* data = image.distortion();
  const int width = image.distortionWidth()/2;
  const int height = image.distortionHeight();
  const int bytesPerPixel = 2 * sizeof(float); // XY per pixel
  const size_t numBytes = static_cast<size_t>(width * height * bytesPerPixel);
  Leap::GL::Texture2PixelData pixelData(GL_RG, GL_FLOAT, data, numBytes);
  if (!distortion || numBytes != m_DistortionBytes[idx]) {
    Leap::GL::Texture2Params params(static_cast<GLsizei>(width), static_cast<GLsizei>(height));
    params.SetTarget(GL_TEXTURE_2D);
    params.SetInternalFormat(GL_RG32F);
    params.SetTexParameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    params.SetTexParameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    params.SetTexParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    params.SetTexParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    distortion = std::shared_ptr<Leap::GL::Texture2>(new Leap::GL::Texture2(params, pixelData));
    m_DistortionBytes[idx] = numBytes;
  } else {
    distortion->TexSubImage(pixelData);
  }
}
