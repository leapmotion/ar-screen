#pragma once

#include "ResourceManager.h"
#include "Singleton.h"

#include <cassert>
#include <iostream>
#include <memory>

/// @brief Convenience class for loading/using managed resources.
/// @details Technically this class is unnecessary, as ResourceManager<T>
/// returns std::shared_ptr<T> types, and the use of the ResourceManager<T>
/// singleton could be done explicitly (see Load).  Effectively this class
/// just ties ResourceManager<T> in together with Singleton<ResourceManager<T>>.
/// 
/// Source files that define template specializations of ResourceLoader<T>
/// do not need to include this header file, as they only have to return a
/// std::shared_ptr<T>.
template <typename T>
class Resource : public std::shared_ptr<T> {
public:

  /// @brief This using declaration is here so that assignment from std::shared_ptr<T> works.
  using std::shared_ptr<T>::operator=; 

  /// @brief Construct an "empty" resource.
  /// @details This can be used, with a later call to Load, if it is not desired
  /// to load the resource upon construction of this object.
  Resource () : std::shared_ptr<T>(nullptr) { }
  /// @brief Construct a resource via Load using the given name.
  explicit Resource (const std::string &name) {
    Load(name);
  }
  /// @brief Load the resource of the given name via Singleton<ResourceManager<T>>.
  /// @details The ResourceManager<T> singleton will be created if it doesn't already exist.
  /// Failure to load will cause a ResourceException<T> to be thrown.
  void Load (const std::string &name) {
    try {
      this->operator=(Singleton<ResourceManager<T>>::SafeRef().Get(name));
      assert(bool(*this)); // ResourceManager<T> is guaranteed to return a valid shared_ptr.
    } catch (const std::exception &e) {
      std::cerr << "exception \"" << e.what() << "\" thrown while loading resource \"" << name << "\"\n";
      throw; // rethrow
    }
  }
};
