#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>

/// @brief Metafunction for defining how to load particularly typed resources.
/// @details To define how to load a paricular type U, one must template-specialize
/// ResourceLoader<U> with the following members/methods:
///   static const bool exists = true;
///   static std::shared_ptr<U> LoadResource (const std::string &name, ResourceManager<U> &calling_manager);
/// The LoadResource method must return a valid shared_ptr.  If that is not possible,
/// then [a subclass of] ResourceExceptionOfType<U> should be thrown.  The reason that
/// LoadResource takes a reference to a ResourceManager<U> is because it is not assumed
/// that ResourceLoader<U> is a singleton.  The manager parameter doesn't have to be used,
/// but could be e.g. if some other resource from the same ResourceManager<U> was to be
/// loaded.
template <typename T>
struct ResourceLoader {
  static const bool exists = false;
};

/// @brief Custom exception class specifically for Resource<T> and ResourceManager<T>.
/// @details This is the base class for all type-specific resource-loading exceptions,
/// and can be used to catch all resource-loading exceptions.
class ResourceException : public std::runtime_error {
public:
  ResourceException (const std::string &message) : std::runtime_error(message) { }
  virtual ~ResourceException () { }
};

/// @brief Type-specific resource-loading exception class.
/// @details Particular typed resource-loading exceptions can be caught using this class.
template <typename T>
class ResourceExceptionOfType : public ResourceException {
public:
  ResourceExceptionOfType (const std::string &message) : ResourceException(message) { }
  virtual ~ResourceExceptionOfType () { }
};

/// @brief A class for tracking non-redundant loading of resources of type T.
template <typename T>
class ResourceManager {
public:
  /// @brief This is the type of the named resource storage.
  typedef std::map<std::string,std::shared_ptr<T>> ResourceMap;

  ResourceManager(const std::string &basePath = "") { SetBasePath(basePath); }
  
  void SetBasePath(const std::string &basePath) {
    m_basePath = basePath;
    if (basePath.empty())
      return;

    const auto lastChar = m_basePath[m_basePath.size() - 1];
    if ( lastChar != '/' && lastChar != '\\'){
#if _WIN32
      m_basePath += '\\'; 
#else
      m_basePath += '/';
#endif
    }
  }

  //Returns the utf-8 encoded base path.
  const std::string& GetBasePath() const { return m_basePath; }

  /// @brief The map of loaded, named resources.
  /// @details Could be used during a cleanup step to check if there are still allocated
  /// resources right before an application is about to exit.
  const ResourceMap &Resources () const { return m_resources; }
  // TODO: store std::weak_ptr instead of shared_ptr.
  /// @brief Return the resource with given name, loading it if necessary.
  /// @details This calls ResourceLoader<T>::LoadResource, which should throw
  /// ResourceExceptionOfType<T> upon error.
  std::shared_ptr<T> Get (const std::string &name) {
    //std::cout << "ResourceManager::Get(\"" << name << "\"); ... ";
    typename ResourceMap::iterator resource_it = m_resources.find(name);
    if (resource_it != m_resources.end()) {
      //std::cout << "the resource was loaded already -- returning that one.\n";
      return resource_it->second;
    }

    //std::cout << "the resource was not already loaded -- loading it.\n";
    static_assert(ResourceLoader<T>::exists, "ResourceLoader<T> not defined -- template-specialize it to define");
    std::shared_ptr<T> resource;

    resource = ResourceLoader<T>::LoadResource(name, *this);
    //std::cout << "ResourceManager::Get(\"" << name << "\"); loaded successfully.\n";
    AddResource(name, resource);
    return resource;
  }

  /// @brief Explicitly add a named resource to the set of managed resources.
  /// @details This could be used for e.g. pre-loading procedurally-generated "fallback"
  /// or other special resources, like a "pass-through" shader program.
  /// A ResourceExceptionOfType<T> will be thrown if the given name is already loaded.
  void AddResource (const std::string &name, const std::shared_ptr<T> &resource) {
    typename ResourceMap::iterator resource_it = m_resources.find(name);
    if (resource_it != m_resources.end()) {
      throw ResourceExceptionOfType<T>("resource with name \"" + name + "\" has already been loaded for this particular resource type");
    }
    m_resources[name] = resource;
  }

private:
  std::string m_basePath = "";
  ResourceMap m_resources; ///< The map of name-indexed loaded resources.
};
