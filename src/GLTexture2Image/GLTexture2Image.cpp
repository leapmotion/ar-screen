#include "GLTexture2Image.h"

// Components
#include "GLTexture2FreeImage.h"
//#include "GLTexture2Loader.h"
//#include "Resource.h"
//#include "SDLController.h"

#include <iostream>
#include <assert.h>

GLTexture2Image::GLTexture2Image()
  : m_Loaded(false)
{
}

void GLTexture2Image::Bind(int textureUnit) const
{
  assert(m_Loaded && m_Texture);
  m_Texture->Bind(textureUnit);
}

void GLTexture2Image::Clear()
{
  m_Loaded = false;
  m_Path.clear();
  m_Texture.reset();
}

const std::string& GLTexture2Image::GetPath() const
{
  return m_Path;
}

std::shared_ptr<Leap::GL::Texture2> GLTexture2Image::GetTexture() const
{
  return m_Texture;
}

bool GLTexture2Image::IsEmpty() const
{
  return m_Path.empty();
}

bool GLTexture2Image::IsLoaded() const
{
  return m_Loaded;
}

bool GLTexture2Image::Load()
{
  if (IsEmpty()) {
    return false;
  }
  
  if (IsLoaded()) {
    return true;
  }
  
  m_Loaded = LoadPath(m_Path);
  
  return m_Loaded;
}

bool GLTexture2Image::LoadPath(const std::string& filePath)
{
  if (filePath.empty()) {
    return false;
  }
  
  try {
    // Copied from Components/GLTexture2Loader.cpp
    Leap::GL::Texture2Params params;
    params.SetTarget(GL_TEXTURE_2D);
    params.SetTexParameteri(GL_GENERATE_MIPMAP, GL_TRUE);
    params.SetTexParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    params.SetTexParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    params.SetTexParameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    params.SetTexParameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    m_Texture = std::shared_ptr<Leap::GL::Texture2>(LoadGLTexture2UsingFreeImage(filePath, params));
    m_Path = filePath;
    m_Loaded = true;
    
  } catch (std::runtime_error&) {
    // m_Texture and m_Texture should not have been set/changed, so return false
    m_Loaded = false;
  }
  
  return m_Loaded;
}
//
//bool GLTexture2Image::LoadResource(const std::string& resourcePath)
//{
//  if (resourcePath.empty()) {
//    return false;
//  }
//  
//  try {
//    m_Texture = Resource<Texture2>(SDLController::BasePath() + resourcePath);
//    m_Path = resourcePath;
//    m_Loaded = true;
//  } catch (std::runtime_error&) {
//    // m_Texture and m_Texture should not have been set/changed, so return false
//    m_Loaded = false;
//  }
//  
//  return m_Loaded;
//}


void GLTexture2Image::SetPath(const std::string& path)
{
  if (m_Path != path) {
    m_Path = path;
    m_Loaded = false;
    m_Texture.reset();
  }
}

void GLTexture2Image::Unbind() const
{
  assert(m_Loaded && m_Texture);
  m_Texture->Unbind();
}
