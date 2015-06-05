#pragma once

#include "Leap/GL/ColorComponent.h"
#include "Leap/GL/Rgb.h"

namespace Leap {
namespace GL {

// TODO: template parameter for switching the order of the components, e.g. BGRA, which is a common
// ordering on some machines.

/// @brief Provides a standard-layout RGBA data type whose components type is ColorComponent<T>.
/// @details See @c ColorComponent for details on color components and operations.
/// The components are ordered in order (R, then G, then B, then A), so that they map directly
/// to "raw" formats such as ones used in texel-loading operations like glTexImage2D.
/// The color operations described in the documentation for @c ColorComponent are also
/// available for Rgba -- color addition, masking, clamping, and blending -- they are each
/// done component-wise.
///
/// The behavior of this class is analogous to Rgb<T>, except that the alpha component is treated
/// somewhat specially in the constructor.  Note that an alpha value of 1 (i.e. ColorComponent<T>::One())
/// is totally opaque, while an alpha value of 0 (i.e. ColorComponent<T>::Zero()) is totally transparent.
template <typename T>
class Rgba {
public:

  /// @brief Typedef for the ColorComponent type for each of the R, G, B, and A components.
  typedef ColorComponent<T> Component;
  /// @brief Number of components in this color type (four).
  static const size_t COMPONENT_COUNT = 4;
  /// @brief Static method which produces the Rgba<T> having components all zero -- transparent black.
  static Rgba Zero () { return Rgba(ColorComponent<T>::Zero()); }
  /// @brief Static method which produces the Rgba<T> having components all one (in the sense
  /// described in @c ColorComponent) -- opaque white.
  static Rgba One () { return Rgba(ColorComponent<T>::One()); }

  /// @brief Constructs an uninitialized Rgba value.
  Rgba () { }
  /// @brief Constructs an Rgba value with identical component values.
  Rgba (const ColorComponent<T> &x) : Rgba(x, x, x, x) { }
  /// @brief Constructs an Rgba value having identical R,G,B component equal to x, and alpha component a.
  /// @details The default alpha value is "opaque" (ColorComponent<T>::One()).
  Rgba (const ColorComponent<T> &x, const ColorComponent<T> &a) : Rgba(x, x, x, a) { }
  /// @brief Constructs an Rgba value from given components.
  /// @details The default alpha value is "opaque" (ColorComponent<T>::One()).
  Rgba (const ColorComponent<T> &r, const ColorComponent<T> &g, const ColorComponent<T> &b, const ColorComponent<T> &a = ColorComponent<T>::One())
    :
    m_rgb(r, g, b),
    m_alpha(a)
  { }
  /// @brief Construct an Rgba value from an Rgb value and an alpha value.
  /// @details The default alpha value is "opaque" (ColorComponent<T>::One()).
  Rgba (const Rgb<T> &rgb, const ColorComponent<T> &a = ColorComponent<T>::One())
    :
    m_rgb(rgb),
    m_alpha(a)
  { }
  /// @brief Dynamic conversion from Rgba value with different component type.
  /// @details This is guaranteed to scale the dynamic range of the components appropriately.
  template <typename U>
  Rgba (const Rgba<U> &other) {
    m_rgb = other.Rgb();
    m_alpha = other.A();
  }

  /// @brief Equality operator.
  /// @details Defined via direct memory comparison.
  bool operator == (const Rgba &other) const {
    return memcmp(static_cast<const void *>(this), static_cast<const void *>(&other), sizeof(Rgba)) == 0;
  }

  /// @brief This method can be used to access the components as whatever standard-layout type is desired (e.g. some library-specific vector type).
  /// @details This const version can be used to access the components via some other standard-layout type, e.g. std::array<T,4>.
  template <typename U>
  const U &ReinterpretAs () const {
    static_assert(sizeof(U) == sizeof(Rgba), "U must be a standard-layout type mapping directly onto this object");
    static_assert(std::is_standard_layout<U>::value, "U must be a standard-layout type mapping directly onto this object");
    // TODO: somehow check that U is a POD consisting only of type ColorComponent
    return *reinterpret_cast<const U *>(this);
  }
  /// @brief This method can be used to access the components as whatever standard-layout type is desired (e.g. some library-specific vector type).
  /// @details This non-const version can be used to modify the components via some other standard-layout type, e.g. std::array<T,4>.
  template <typename U>
  U &ReinterpretAs () {
    static_assert(sizeof(U) == sizeof(Rgba), "U must be a standard-layout type mapping directly onto this object");
    static_assert(std::is_standard_layout<U>::value, "U must be a standard-layout type mapping directly onto this object");
    // TODO: somehow check that U is a POD consisting only of type ColorComponent
    return *reinterpret_cast<U *>(this);
  }

  // TODO: handle luminance somehow (maybe via global function)

  /// @brief Const accessor for the red component.
  const ColorComponent<T> &R () const { return m_rgb.R(); }
  /// @brief Const accessor for the green component.
  const ColorComponent<T> &G () const { return m_rgb.G(); }
  /// @brief Const accessor for the blue component.
  const ColorComponent<T> &B () const { return m_rgb.B(); }
  /// @brief Const accessor for the alpha component.
  const ColorComponent<T> &A () const { return m_alpha; }
  /// @brief Non-const accessor for the red component.
  ColorComponent<T> &R () { return m_rgb.R(); }
  /// @brief Non-const accessor for the green component.
  ColorComponent<T> &G () { return m_rgb.G(); }
  /// @brief Non-const accessor for the blue component.
  ColorComponent<T> &B () { return m_rgb.B(); }
  /// @brief Non-const accessor for the alpha component.
  ColorComponent<T> &A () { return m_alpha; }

  /// @brief Const accessor for the Rgb<T> portion of this Rgba<T> value.
  const Leap::GL::Rgb<T> &Rgb () const { return m_rgb; }
  /// @brief Non-const accessor for the Rgb<T> portion of this Rgba<T> value.
  Leap::GL::Rgb<T> &Rgb () { return m_rgb; }

  /// @brief Adds another Rgba value into this one, component-wise.
  /// @details Note that this may overflow the component range (which is defined for
  /// each valid component by ColorComponent<T>::Zero() and ColorComponent<T>::One()),
  /// and for integral component types, this will also involve value wrap-around.
  void operator += (const Rgba &other) {
    m_rgb += other.m_rgb;
    m_alpha += other.m_alpha;
  }
  /// @brief Returns the sum of two Rgba values.
  Rgba operator + (const Rgba &other) const {
    Rgba retval(*this);
    retval += other;
    return retval;
  }
  /// @brief Masks this Rgba value with another one, component-wise.
  /// @details See @c ColorComponent for more information on the masking operation.
  void operator *= (const Rgba &other) {
    m_rgb *= other.m_rgb;
    m_alpha *= other.m_alpha;
  }
  /// @brief Returns the masking of two Rgba values.
  Rgba operator * (const Rgba &other) {
    Rgba retval(*this);
    retval *= other;
    return retval;
  }
  /// @brief Masks each component of this Rgba value with a single ColorComponent value.
  void operator *= (const ColorComponent<T> &masking_factor) {
    m_rgb *= masking_factor;
    m_alpha *= masking_factor;
  }
  /// @brief Returns the masking of this Rgba value with a single ColorComponent value.
  Rgba operator * (const ColorComponent<T> &masking_factor) {
    Rgba retval(*this);
    retval *= masking_factor;
    return retval;
  }

  /// @brief Calls Clamp on each component in this Rgba object.
  /// @details See @c ColorComponent::Clamp for more info.
  void Clamp () {
    m_rgb.Clamp();
    m_alpha.Clamp();
  }
  /// @brief Returns the clamped value.
  Rgba Clamped () const {
    Rgba retval(*this);
    retval.Clamp();
    return retval;
  }

  /// @brief Calls BlendWith on each component in this Rgba object with the corresponding component in blend_target.
  /// @details The blend_parameter argument is used for each component's BlendWith operation.  See
  /// @c ColorComponent::BlendWith for more info.
  void BlendWith (const Rgba &blend_target, const ColorComponent<T> &blend_parameter) {
    m_rgb.BlendWith(blend_target.m_rgb, blend_parameter);
    m_alpha.BlendWith(blend_target.m_alpha, blend_parameter);
  }
  /// @brief Returns the blended value.
  /// @details See @c Rgba::BlendWith for more info.
  Rgba BlendedWith (const Rgba &blend_target, const ColorComponent<T> &blend_parameter) const {
    Rgba retval(*this);
    retval.BlendWith(blend_target, blend_parameter);
    return retval;
  }

  /// @brief Multiply (mask) this object's RGB components by its alpha component.
  /// @details This is a common operation in texture mapping partially transparent textures,
  /// in which this procedure is done as a one-time pre-computation to avoid a multiply in
  /// each blended texel operation.
  void PremultiplyAlpha () {
    m_rgb *= m_alpha;
  }
  /// @brief Returns the premultiplied value.
  Rgba PremultipliedAlpha () const {
    Rgba retval(*this);
    retval.PremultiplyAlpha();
    return retval;
  }

private:

  /// @brief The RGB components of this Rgba object.
  Leap::GL::Rgb<T> m_rgb;
  /// @brief The alpha component of this Rgba object.
  ColorComponent<T> m_alpha;
};

} // end of namespace GL
} // end of namespace Leap
