#pragma once
#include <string>
#include <memory>

class ImagePrimitive;

class OSApp
{
public:
  ~OSApp(void) {}

  /// <returns>
  /// The UTF-8 encoded, localized, user-presentable name of the application
  /// </returns>
  virtual std::string GetAppName(void) const = 0;

  /// <summary>
  /// Renders the application icon in the image primitive
  /// </summary>
  virtual std::shared_ptr<ImagePrimitive> GetIconTexture(std::shared_ptr<ImagePrimitive> img) const = 0;

  /// <summary>
  /// Compares this instance to another instance
  /// </summary>
  bool operator==(const OSApp& rhs) const { return m_id == rhs.m_id; }

protected:
  OSApp(uint32_t pid) : m_id(GetAppIdentifier(pid)) {}

  // Application unique identifier
  const std::wstring m_id;

private:
  static std::wstring GetAppIdentifier(uint32_t pid);
  static OSApp* New(uint32_t pid);

  friend class OSAppManager;
};
