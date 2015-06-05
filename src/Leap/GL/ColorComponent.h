#pragma once

#include "Leap/GL/Internal/ColorComponent.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace Leap {
namespace GL {

/// @brief Represents the value of a color component (e.g. r, g, b, alpha, luminance, etc.)
/// within the range [0,1].
///
/// @details Conceptually, a color component is a value in the range [0,1].  Let C(x)
/// denote the value x interpreted as a color component.  The template parameter T defines
/// the type used to internally represent that value, though the representation maps a
/// particular dynamic range of T onto the conceptual range of color component values
/// [C(0),C(1)].
///
/// The operations of color addition, masking, clamping, and blending are defined with respect
/// to their color components C values, not the internal representation values.  Thus the
/// result of these operations is invariant with respect to the type T (except for variations
/// due to the limitations of precision of the different types).
/// 
/// The following are the allowable types T with their ranges corresponding to [C(0),C(1)].
/// - uint8_t  : [ uint8_t(0), uint8_t(2^8-1) ]
/// - uint16_t : [uint16_t(0),uint16_t(2^16-1)]
/// - uint32_t : [uint32_t(0),uint32_t(2^32-1)]
/// - float    : [   float(0),   float(1)     ]
/// - double   : [  double(0),  double(1)     ]
///
/// Technically, uint64_t and long double are allowed, though their conversion and masking
/// operations are not ideal and may have suboptimal precision.
///
/// The ColorComponent<T>::Zero and ColorComponent<T>::One methods produce the color component
/// values C(0) and C(1) respectively for the given T (e.g. ColorComponent<uint8_t>::One() is
/// represented internally as uint8_t(255)).
/// 
/// Note that constructing a ColorComponent<T> from a ColorComponent<U> (for a type U which may
/// be different than T) will convert the values appropriately, though there may be a loss in
/// precision depending on T and U.  The AsComponent<U>() accessor is an explicit conversion which
/// produces the corresponding ColorComponent<U> value.
/// 
/// Addition of ColorComponent values works as expected.  Multiplication (aka masking) operates
/// using the conceptual C values.  For example, ColorComponent<T>::One() equals C(1), so
/// if v is any ColorComponent<T> object, then ColorComponent<T>::One()*v is always equal to v.
/// Similarly, ColorComponent<T>::Zero()*v is always equal to ColorComponent<T>::Zero().
/// 
/// For some types T, it is possible to represent color component values outside the range of
/// [C(0),C(1)] (e.g. the floating point types).  The Clamp operation clamps values to be within
/// the range [C(0),C(1)] -- if the value is below C(0), it is set to C(0), and if the value is
/// above C(1), it is set to C(1).  For types T whose entire numerical range is used (e.g.
/// the unsigned integral types), it is impossible to represent a color component outside the
/// range [C(0),C(1)], so the Clamp operation is a no-op.
///
/// Finally, the BlendWith operation provides a convenient means to linearly interpolate between
/// two ColorComponent<T> values using a ColorComponent<T> value as the blending parameter.
/// This could be used, for example, to fade smoothly between two images.
template <typename T>
struct ColorComponent {
  static_assert(Internal::ComponentValueTraits<T>::IS_DEFINED, "Type T is not a valid underlying type for a color component.");

  /// @brief Produces the value C(0).
  static ColorComponent Zero () { return ColorComponent(Internal::ComponentValueTraits<T>::Min()); }
  /// @brief Produces the value C(1).
  static ColorComponent One () { return ColorComponent(Internal::ComponentValueTraits<T>::Max()); }

  /// @brief Construct a ColorComponent with uninitialized value.
  ColorComponent () { }
  /// @brief Construct a ColorComponent with type-specific value.
  /// @details Note that the way that this value will be interpreted depends on the dynamic
  /// range of the underlying type (for example, uint8_t uses the range [uint8_t(0),uint8_t(255)]
  /// to represent the color component range [C(0),C(1)], whereas float uses the range
  /// [float(0),float(1)] to do the same).
  ColorComponent (const T &value) : m_value(value) { }
  /// @brief Copy constructor.
  ColorComponent (const ColorComponent &other) : m_value(other.m_value) { }
  /// @brief Dynamic conversion from ColorComponent value with different type.
  /// @details This costructor is guaranteed to scale the dynamic range appropriately,
  /// though there may be some loss in precision depending on the types T and U.
  template <typename U>
  ColorComponent (const ColorComponent<U> &other) {
    Internal::ConvertComponentValue(other.Value(), m_value);
  }

  /// @brief Returns this component converted to use a different underlying type.
  /// @details Note that this may result in a loss of precision (e.g.
  /// ColorComponent<uint32_t>::As<uint16_t>).
  template <typename U>
  ColorComponent<U> AsComponent () const {
    ColorComponent<U> retval;
    Internal::ConvertComponentValue(m_value, retval.Value());
    return retval;
  }

  /// @brief Const accessor for the internal representation of the color component value.
  const T &Value () const { return m_value; }
  /// @brief Non-const accessor for the internal representation of the color component value.
  T &Value () { return m_value; }
  /// @brief Conversion operator for the internal representation type (const).
  operator const T & () const { return m_value; }
  /// @brief Conversion operator for the internal representation type (non-const).
  operator T & () { return m_value; }

  /// @brief Color component addition (combines lightnesses).
  void operator += (const ColorComponent &other) {
    m_value += other.m_value;
  }
  /// @brief Color component addition (combines lightnesses).
  ColorComponent operator + (const ColorComponent &other) {
    ColorComponent retval(*this);
    retval += other;
    return retval;
  }
  /// @brief Masks this ColorComponent with other in-place.
  /// @details This is really just multiplication of the ColorComponent values as interpreted
  /// in the range [C(0),C(1)].  Masking with Zero() gives Zero(), and masking a component value
  /// V with the value of One() gives V.
  void operator *= (const ColorComponent &other) {
    m_value = Internal::ComponentValueMask(m_value, other.m_value);
  }
  /// @brief This is the color component masking operation (non-in-place).
  ColorComponent operator * (const ColorComponent &other) const {
    ColorComponent retval(*this);
    retval *= other;
    return retval;
  }

  /// @brief Clamps this component to within its dynamic range (in-place).
  /// @details For types T which use their entire dynamic range (e.g. the unsigned integer types)
  /// this is a no-op, because it's impossible for the value to be outside the range.
  void Clamp () {
    m_value = std::min(std::max(m_value, Zero().Value()), One().Value());
  }
  /// @brief Returns the clamped value.
  ColorComponent Clamped () const {
    ColorComponent retval(*this);
    retval.Clamp();
    return retval;
  }

  /// @brief In-place Linear blending between two ColorComponents, given a blending parameter (of the same type).
  /// @details Does nothing if blend_parameter is Zero() (the blended value is this color component),
  /// sets this color component to blend_target if blend_parameter is One(), and sets this color component
  /// to the linear interpolation between the two in the general case.
  void BlendWith (const ColorComponent &blend_target, const ColorComponent &blend_parameter) {
    ColorComponent one_minus_blend_parameter(One() - blend_parameter);
    m_value =   Internal::ComponentValueMask(m_value, one_minus_blend_parameter.m_value) 
              + Internal::ComponentValueMask(blend_target.m_value, blend_parameter.m_value);
  }
  /// @brief Returns the linear blending between two ColorComponents.
  ColorComponent BlendedWith (const ColorComponent &blend_target, const ColorComponent &blend_parameter) {
    ColorComponent retval(*this);
    retval.BlendWith(blend_target, blend_parameter);
    return retval;
  }

private:

  /// @brief Internal representation of the color component, whose interpretation depends on the type T.
  T m_value;
};

} // end of namespace GL
} // end of namespace Leap
