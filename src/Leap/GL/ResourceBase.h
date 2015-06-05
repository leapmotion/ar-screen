#pragma once

#include <cassert>

namespace Leap {
namespace GL {

/// @brief Static interface class for classes acquiring resources, providing initialize/shutdown methods.
/// @details This class offers the public methods IsInitialized, Initialize<...>, and Shutdown.
/// TODO write more about this.
/// A subclass C of this class must inherit ResourceBase<C> and implement the following methods.
/// - bool IsInitialized_Implementation () const;
/// - void Initialize_Implementation (...);
/// - void Shutdown_Implementation ();
///
/// The IsInitialized_Implementation method must return true if the resource is considered
/// initialized (the definition of "is initialized" depends on the resource type).
///
/// The Initialize_Implementation method may accept any arguments (and have any number of
/// overloads) and must bring the resource from an un-initialized state to an initialized
/// state.  Initialize_Implementation can assume that when it is called,
/// IsInitialized_Implementation returns false.  If Initialize_Implementation succeeds,
/// then IsInitialized_Implementation must return true.  An error should be indicated
/// by throwing an exception.
///
/// The Shutdown_Implementation method must bring the resource from an initialized state
/// to an un-initialized state.  Shutdown_Implementation can assume that when it is called,
/// IsInitialized_Implementation returns true.  Shutdown_Implementation must succeed
/// without throwing an exception, and once it returns, IsInitialized_Implementation must
/// return true.
///
/// Subclasses are responsible for calling Shutdown before or during their own destruction
/// (this can't be done in the ResourceBase destructor, because that is only called after
/// the subclass' destructor, which destroys all of its members).
template <typename Derived_>
class ResourceBase {
public:

  /// @brief Returns true if this resource has been initialized.
  /// @details The definition of "is initialized" is made by Derived_ via the method
  /// IsInitialized_Implementation.
  bool IsInitialized () const {
    return AsDerived().IsInitialized_Implementation();
  }
  /// @brief Initializes this resource, shutting down beforehand if necessary.
  /// @details The arguments to Initialize are passed directly into Initialize_Implementation
  /// which must be provided by Derived_.
  template <typename... Types_>
  void Initialize (Types_... args) {
    Shutdown();
    AsDerived().Initialize_Implementation(args...);
    assert(IsInitialized() && "Initialize_Implementation or IsInitialized_Implementation incorrectly defined.");
  }
  /// @brief Shuts down this resource if initialized, otherwise does nothing.
  /// @details The shutdown procedure is defined by the Shutdown_Implementation method
  /// of Derived_.
  void Shutdown () {
    if (IsInitialized()) {
      AsDerived().Shutdown_Implementation();
      assert(!IsInitialized() && "Shutdown_Implementation or IsInitialized_Implementation incorrectly defined.");
    }
  }

protected:

  const Derived_ &AsDerived () const { return *static_cast<const Derived_ *>(this); }
  Derived_ &AsDerived () { return *static_cast<Derived_ *>(this); }
};

} // end of namespace GL
} // end of namespace Leap
