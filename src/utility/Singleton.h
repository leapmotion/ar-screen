#pragma once

#include <stdexcept>

/// @brief Custom exception class specifically for Singleton<T>.
/// @details This is the base class for all type-specific singleton exceptions,
/// and can be used to catch all singleton exceptions.
class SingletonException : public std::runtime_error {
public:
  SingletonException (const std::string &message) : std::runtime_error(message) { }
  virtual ~SingletonException () { }
};

/// @brief Type-specific singleton exception class.
/// @details Particular typed singleton exceptions can be caught using this class.
template <typename T>
class SingletonExceptionOfType : public SingletonException {
public:
  SingletonExceptionOfType (const std::string &message) : SingletonException(message) { }
  virtual ~SingletonExceptionOfType () { }
};

/// @brief A class which provides a potentially lazily initialized singleton object
/// of a particular, templated type.
/// @details The CreateInstance and DestroyInstance methods can be used to explicitly
/// create or destroy the singleton.  SafeRef will create the singleton if it doesn't
/// already exist. This is essentially used as a really shitty single-context version
/// of autowiring at this point, and should be replaced with it.

template <typename T>
class Singleton {
public:
  /// @brief Explicitly create the T singleton.
  /// @details Allocates the singleton on the heap via new.  If the singleton
  /// exists already, this throws a SingletonExceptionOfType<T>.  This method
  /// should be used if the singleton needs to be constructed with particular
  /// parameters before being used.
  template<typename... Arguments>
  static void CreateInstance(Arguments... params) {
    if (s_inst != nullptr) {
      throw SingletonExceptionOfType<T>("Singleton of this type already exists");
    }
    s_inst = new T(params...);
  }
  /// @brief Ensure that the T singleton exists.
  /// @details Allocates the singleton on the heap via new with no arguments.
  static void EnsureInstanceExists() {
    if (s_inst == nullptr){
      s_inst = new T();
    }
  }
  /// @brief Explicitly destroy the T singleton if it exists.
  /// @details Deallocates the singleton via delete.
  static void DestroyInstance () {
    if (s_inst != nullptr) {
      delete s_inst;
      s_inst = nullptr;
    }
  }

  /// @brief Returns true iff the T singleton has been created and currently exists.
  static bool Exists () { return s_inst; }
  /// @brief Returns a pointer to the T singleton.  Will be nullptr if the singleton
  /// doesn't exist (i.e. if Exists returns false).
  static T *Ptr () { return s_inst; }
  /// @brief Returns a reference to the T singleton.  Will throw SingletonExceptionOfType<T> if the
  /// singleton doesn't exist (i.e. if Exists returns false).
  static T &FastRef () {
    if (s_inst == nullptr) {
      throw SingletonExceptionOfType<T>("can't return reference to singleton that hasn't been created yet");
    }
    return *s_inst;
  }
  /// @brief Returns a reference to the T singleton.  Will create the T singleton if
  /// it doesn't already exist.
  static T &SafeRef () { 
    EnsureInstanceExists();
    return *s_inst;
  }

private:

  static T *s_inst; ///< A pointer to the T singleton.  Is nullptr iff the singleton doesn't exist.
};

/// @brief Definition of the static singleton pointer.
template<class T>
T *Singleton<T>::s_inst = nullptr;
