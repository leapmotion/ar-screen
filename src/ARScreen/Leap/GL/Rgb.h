#pragma once

#include "Leap/GL/ColorComponent.h"

namespace Leap {
namespace GL {

// T is given as the template parameter to ColorComponent, and ColorComponent<T> is the type of
// each of the R, G, B components in this class.
// TODO: template parameter for switching the order of the components, e.g. BGR, which is a common
// ordering on some machines.

/// @brief Provides a standard-layout RGB data type whose components type is ColorComponent<T>.
/// @details See @c ColorComponent for details on color components and operations.
/// The components are ordered in order (R, then G, then B), so that they map directly
/// to "raw" formats such as ones used in texel-loading operations like glTexImage2D.
/// The color operations described in the documentation for @c ColorComponent are also
/// available for Rgb -- color addition, masking, clamping, and blending -- they are each
/// done component-wise.
template <typename T>
class Rgb {
public:

  /// @brief Typedef for the ColorComponent type for each of the R, G, and B components.
  typedef ColorComponent<T> Component;
  /// @brief Number of components in this color type (three).
  static const size_t COMPONENT_COUNT = 3;
  /// @brief Static method which produces the Rgb<T> having components all zero -- black.
  static Rgb Zero () { return Rgb(ColorComponent<T>::Zero()); }
  /// @brief Static method which produces the Rgb<T> having components all one (in the sense
  /// described in @c ColorComponent) -- white.
  static Rgb One () { return Rgb(ColorComponent<T>::One()); }

  /// @brief Constructs an uninitialized Rgb value.
  Rgb () { }
  /// @brief Constructs an Rgb value with identical component values.
  Rgb (const ColorComponent<T> &x) : Rgb(x, x, x) { }
  /// @brief Constructs an Rgb value from given components.
  Rgb (const ColorComponent<T> &r, const ColorComponent<T> &g, const ColorComponent<T> &b) {
    m_data[0] = r;
    m_data[1] = g;
    m_data[2] = b;
  }
  /// @brief Dynamic conversion from Rgb value with different component type.
  /// @details This is guaranteed to scale the dynamic range of the components appropriately.
  template <typename U>
  Rgb (const Rgb<U> &other) {
    R() = other.R();
    G() = other.G();
    B() = other.B();
  }

  /// @brief Equality operator.
  /// @details Defined via direct memory comparison.
  bool operator == (const Rgb &other) const {
    return memcmp(static_cast<const void *>(this), static_cast<const void *>(&other), sizeof(Rgb)) == 0;
  }

  /// @brief This method can be used to access the components as whatever standard-layout type is desired (e.g. some library-specific vector type).
  /// @details This const version can be used to modify the components via some other standard-layout type, e.g. std::array<T,3>.
  template <typename U>
  const U &ReinterpretAs () const {
    static_assert(sizeof(U) == sizeof(Rgb), "U must be a standard-layout type mapping directly onto this object");
    static_assert(std::is_standard_layout<U>::value, "U must be a standard-layout type mapping directly onto this object");
    // TODO: somehow check that U consists only of type ColorComponent
    return *reinterpret_cast<const U *>(this);
  }
  /// @brief This method can be used to access the components as whatever standard-layout type is desired (e.g. some library-specific vector type).
  /// @details This non-const version can be used to modify the components via some other standard-layout type, e.g. std::array<T,3>.
  template <typename U>
  U &ReinterpretAs () {
    static_assert(sizeof(U) == sizeof(Rgb), "U must be a standard-layout type mapping directly onto this object");
    static_assert(std::is_standard_layout<U>::value, "U must be a standard-layout type mapping directly onto this object");
    // TODO: somehow check that U consists only of type ColorComponent
    return *reinterpret_cast<U *>(this);
  }

  // TODO: handle luminance somehow (maybe via global function)

  /// @brief Const accessor for the red component.
  const ColorComponent<T> &R () const { return m_data[0]; }
  /// @brief Const accessor for the green component.
  const ColorComponent<T> &G () const { return m_data[1]; }
  /// @brief Const accessor for the blue component.
  const ColorComponent<T> &B () const { return m_data[2]; }
  /// @brief Non-const accessor for the red component.
  ColorComponent<T> &R () { return m_data[0]; }
  /// @brief Non-const accessor for the green component.
  ColorComponent<T> &G () { return m_data[1]; }
  /// @brief Non-const accessor for the blue component.
  ColorComponent<T> &B () { return m_data[2]; }

  /// @brief Adds another Rgb value into this one, component-wise.
  /// @details Note that this may overflow the component range (which is defined for
  /// each valid component by ColorComponent<T>::Zero() and ColorComponent<T>::One()),
  /// and for integral component types, this will also involve value wrap-around.
  void operator += (const Rgb &other) {
    for (size_t i = 0; i < COMPONENT_COUNT; ++i) {
      m_data[i] += other.m_data[i];
    }
  }
  /// @brief Returns the sum of two Rgb values.
  Rgb operator + (const Rgb &other) const {
    Rgb retval(*this);
    retval += other;
    return retval;
  }
  /// @brief Masks this Rgb value with another one, component-wise.
  /// @details See @c ColorComponent for more information on the masking operation.
  void operator *= (const Rgb &other) {
    for (size_t i = 0; i < COMPONENT_COUNT; ++i) {
      m_data[i] *= other.m_data[i];
    }
  }
  /// @brief Returns the masking of two Rgb values.
  Rgb operator * (const Rgb &other) {
    Rgb retval(*this);
    retval *= other;
    return retval;
  }
  /// @brief Masks each component of this Rgb value with a single ColorComponent value.
  void operator *= (const ColorComponent<T> &masking_factor) {
    for (size_t i = 0; i < COMPONENT_COUNT; ++i) {
      m_data[i] += masking_factor;
    }
  }
  /// @brief Returns the masking of this Rgb value with a single ColorComponent value.
  Rgb operator * (const ColorComponent<T> &masking_factor) {
    Rgb retval(*this);
    retval *= masking_factor;
    return retval;
  }

  /// @brief Calls Clamp on each component in this Rgb object.
  /// @details See @c ColorComponent::Clamp for more info.
  void Clamp () {
    for (size_t i = 0; i < COMPONENT_COUNT; ++i) {
      ClampComponent(m_data[i]);
    }
  }
  /// @brief Returns the clamped value.
  Rgb Clamped () const {
    Rgb retval(*this);
    retval.Clamp();
    return retval;
  }

  /// @brief Calls BlendWith on each component in this Rgb object with the corresponding component in blend_target.
  /// @details The blend_parameter argument is used for each component's BlendWith operation.  See
  /// @c ColorComponent::BlendWith for more info.
  void BlendWith (const Rgb &blend_target, const ColorComponent<T> &blend_parameter) {
    for (size_t i = 0; i < COMPONENT_COUNT; ++i) {
      m_data[i].BlendWith(blend_target.m_data[i], blend_parameter);
    }
  }
  /// @brief Returns the blended value.
  /// @details See @c Rgb::BlendWith for more info.
  Rgb BlendedWith (const Rgb &blend_target, const ColorComponent<T> &blend_parameter) const {
    Rgb retval(*this);
    retval.BlendWith(blend_target, blend_parameter);
    return retval;
  }

private:

  /// @brief The array of ColorComponent instances making up this Rgb object, in order R, G, then B.
  ColorComponent<T> m_data[COMPONENT_COUNT];
};

} // end of namespace GL
} // end of namespace Leap
