#pragma once

#include <ostream>
#include "SceneGraphNodeProperty.h"
#include <sstream>
#include <string>

// This is a tuple of SceneGraphNodeProperty types.
template <typename Derived, typename... Properties_> class NodeProperties;

// Recursive definition of NodeProperties.  
template <typename Derived, typename HeadProperty, typename... BodyProperties_>
class NodeProperties<Derived,HeadProperty,BodyProperties_...> {
public:

  // The nullptr_t is used because there is no derived type for BodyProperties.
  typedef NodeProperties<std::nullptr_t,BodyProperties_...> BodyProperties;

  NodeProperties () { }

  bool operator == (const NodeProperties &other) const {
    return Head() == other.Head() && Body() == other.Body();
  }

  const HeadProperty &Head () const { return m_head; }
  HeadProperty &Head () { return m_head; }
  const BodyProperties &Body () const { return m_body; }
  BodyProperties &Body () { return m_body; }

  Derived Inverse () const {
    Derived retval(this->AsDerived());
    retval.Invert();
    return retval;
  }

  void SetIdentity () {
    m_head.SetIdentity();
    m_body.SetIdentity();
  }
  void Apply (const NodeProperties &other, Operate operate) {
    m_head.Apply(other.m_head, operate);
    m_body.Apply(other.m_body, operate);
  }
  void Invert () {
    m_head.Invert();
    m_body.Invert();
  }
  std::string AsString () const {
    std::ostringstream out;
    out << Head()
        << Body().AsString();
    return out.str();
  }

  const Derived &AsDerived () const { return *static_cast<const Derived *>(this); }
  Derived &AsDerived () { return *static_cast<Derived *>(this); }

private:

  HeadProperty m_head;
  BodyProperties m_body;
};

// Base-case definition of NodeProperties.
template <typename Derived, typename HeadProperty>
class NodeProperties<Derived,HeadProperty> {
public:

  NodeProperties () { }

  bool operator == (const NodeProperties &other) const {
    return Head() == other.Head();
  }

  const HeadProperty &Head () const { return m_head; }
  HeadProperty &Head () { return m_head; }

  Derived Inverse () const {
    Derived retval(this->AsDerived());
    retval.Invert();
    return retval;
  }

  void SetIdentity () {
    m_head.SetIdentity();
  }
  void Apply (const NodeProperties &other, Operate operate) {
    m_head.Apply(other.m_head, operate);
  }
  void Invert () {
    m_head.Invert();
  }
  std::string AsString () const {
    std::ostringstream out;
    out << Head();
    return out.str();
  }

  const Derived &AsDerived () const { return *static_cast<const Derived *>(this); }
  Derived &AsDerived () { return *static_cast<Derived *>(this); }

private:

  HeadProperty m_head;
};

template <typename Derived, typename... Properties_>
Derived operator * (NodeProperties<Derived,Properties_...> lhs,
                    const NodeProperties<Derived,Properties_...> &rhs) {
  lhs.Apply(rhs, Operate::ON_RIGHT);
  return lhs.AsDerived();
}

template <typename Derived, typename... Properties_>
std::ostream &operator << (std::ostream &out, const NodeProperties<Derived,Properties_...> &p) {
  return out << p.AsString();
}
