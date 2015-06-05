#ifndef __GLTexture2Image_H__
#define __GLTexture2Image_H__

// Components
#include "Leap/GL/Texture2.h"

#include <memory>
#include <string>

/// This class is essentially a wrapper between a std::string path and Texture2.
class GLTexture2Image
{
public:
  GLTexture2Image();
  
  void Bind(int textureUnit = 0) const;
  
  void Clear();
  
  const std::string& GetPath() const;
  
  std::shared_ptr<Leap::GL::Texture2> GetTexture() const;
  
  bool IsEmpty() const;
  
  bool IsLoaded() const;
  
  bool Load();
  
  /// Load an image from a given filepath
  bool LoadPath(const std::string& filePath);
  
  //bool LoadResource(const std::string& resourcePath);
  
  void SetPath(const std::string& path);
  
  void Unbind() const;
  
protected:
  bool                                m_Loaded;
  std::string                         m_Path;
  std::shared_ptr<Leap::GL::Texture2> m_Texture;
  mutable int                         m_TextureUnit;
};

typedef std::shared_ptr<GLTexture2Image> GLTexture2ImageRef;

#endif
