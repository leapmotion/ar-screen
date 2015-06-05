#pragma once

#include <ostream>
#include <sstream>
#include <string>

// Defines how the NodeProperty::Apply method should work.
enum class ApplyType : int { OPERATE = 0, REPLACE };

// Stringifies the ApplyType enum.
inline std::string ApplyTypeAsString (ApplyType apply_type) {
  switch (apply_type) {
    case ApplyType::OPERATE: return "ApplyType::OPERATE";
    case ApplyType::REPLACE: return "ApplyType::REPLACE";
    default: assert(false && "missing cases"); return "<missing ApplyType case>";
  }
}

// Defines on which side of a property another property should operate.  This
// value only matters for when the ApplyType for an Apply call is OPERATE.
enum class Operate { ON_LEFT, ON_RIGHT };

// The Value template parameter must be a class having the following methods:
//   bool operator == (const AffineTransformValue &other);
//   void SetIdentity ();
//   void Operate (const AffineTransformValue &operand, ::Operate operate);
//   bool Invert ();
//   std::string AsString () const;

// A NodeProperty has a "validity" flag, indicating if the value contained within
// has meaning.  The primary occasion that a NodeProperty will be invalid is if
// its inverse was computed when no inverse is well-defined (e.g. dividing by zero).
// The "apply type" attribute indicates how this NodeProperty will act on another
// when applied.  See ApplyType enum.  Note that a REPLACE apply type is non-
// invertible, so inverting such a NodeProperty will cause invalidation.
template <typename ValueType_>
class NodeProperty {
public:

  typedef ValueType_ ValueType;

  // Initialize the property to the "identity" value, as defined by the Derived class.
  NodeProperty ()
    :
    m_is_valid(true),
    m_apply_type(::ApplyType::OPERATE)
  { }

  bool operator == (const NodeProperty &other) const {
    // The validity flags must be the same
    if (this->m_is_valid != other.m_is_valid) {
      return false;
    }
    // If the validity flags are the same and false, then the properties are equal,
    // because the apply type and values are irrelevant.
    if (!this->m_is_valid) {
      return true;
    }
    // Otherwise, the apply types and values must be identical.
    return this->m_apply_type == other.m_apply_type &&
           this->m_value == other.m_value;
  }

  bool IsValid () const { return m_is_valid; }
  ::ApplyType ApplyType () const { return m_apply_type; }
  const ValueType &Value () const { return m_value; }
  ValueType &Value () { return m_value; }

  void SetApplyType (::ApplyType apply_type) { m_apply_type = apply_type; }
  NodeProperty Inverse () const {
    NodeProperty retval(*this);
    retval.Invert();
    return retval;
  }

  void SetIdentity () { m_value.SetIdentity(); }
  // The given property represents a "delta" which should be applied to this one.  There are
  // different ways to do this, given by the ApplyType enum.
  //   - ApplyType::OPERATE indicates that the given property Q should be applied on the left
  //     or right of this property P, left/right is defined by the operate parameter.  The
  //     convention is that a property acts on the left of its ultimate operand (e.g. coordinate
  //     transforms act on EigenTypes::Vectors, so for a coordinate transform T acting on on a EigenTypes::Vector V,
  //     the action is defined to be T*V).
  //   - ApplyType::REPLACE simply replaces this property with the given one.  The operate
  //     parameter is not used.
  void Apply (const NodeProperty &other, Operate operate) {
    switch (other.m_apply_type) {
      case ::ApplyType::OPERATE:
        m_is_valid = m_is_valid && other.m_is_valid;
        if (m_is_valid) { // Don't bother operating if invalid.
          m_value.Operate(other.m_value, operate);
        }
        break;
      case ::ApplyType::REPLACE:
        m_is_valid = other.m_is_valid;
        m_apply_type = ::ApplyType::REPLACE;
        m_value = other.m_value;
        break;
    }
  }
  // Inverting an ApplyType::OPERATE property may result in invalidation, depending
  // on its value.  Inverting an ApplyType::REPLACE necessarily results in invalidation.
  void Invert () {
    switch (m_apply_type) {
      case ::ApplyType::OPERATE:
        if (m_is_valid) { // Don't bother inverting if already invalid.
          m_is_valid = m_value.Invert();
        }
        break;
      case ::ApplyType::REPLACE:
        m_is_valid = false; // There is no way to invert a replacement.
        break;
    }
  }
  // Returns a string representation of this property.  Should include the type.
  std::string AsString () const {
    std::ostringstream out;
    out << m_value.TypeAsString() << '\n'
        << "    is valid : " << std::boolalpha << m_is_valid << '\n';
    if (m_is_valid) {
      out << "    apply type : " << ApplyTypeAsString(m_apply_type) << '\n';
      std::string value_as_string(m_value.AsString());
      // Replace newlines with newlines and indentation.
      ReplaceAllIn(value_as_string, "\n", "\n            ");
      out << "    value : " << value_as_string << '\n';
    }
    return out.str();
  }

private:

  bool m_is_valid;
  ::ApplyType m_apply_type;
  ValueType m_value;

  static void ReplaceAllIn (std::string &s, const std::string &replace_me, const std::string &replacement) {
    size_t pos = 0;
    while ((pos = s.find(replace_me, pos)) != s.npos) {
      s.replace(pos, replace_me.size(), replacement);
      pos += replacement.size();
    }
  }
};

template <typename ValueType_>
NodeProperty<ValueType_> operator * (NodeProperty<ValueType_> lhs, const NodeProperty<ValueType_> &rhs) {
  lhs.Apply(rhs, ::Operate::ON_RIGHT);
  return lhs;
}

template <typename ValueType_>
std::ostream &operator << (std::ostream &out, const NodeProperty<ValueType_> &property) {
  return out << property.AsString();
}
