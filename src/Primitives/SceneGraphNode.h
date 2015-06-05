#pragma once

#include "utility/EigenTypes.h"
#include <memory>
#include "SceneGraphNodeProperties.h"
#include <unordered_set>

// This class contains base functionality common to all primitives:
// - translation
// - linear transformation
// - Parent/child transform hierarchy (in progress)
// - Local/global coordinate system conversion (in progress)

// Design for parent-to-child property propagation
// -----------------------------------------------
// A scene graph node will have a set of properties.  Let P denote a set of properties,
// and let SGN<P> denote the scene graph node type having properties P.  Every node in
// a scene graph tree must have the same set of properties (i.e. the node type in a scene
// graph must be uniform).
//
// A property must have a well-defined "identity" and "application operation".  Properties
// will be applied in a stack in order to accumulate values to define the "global" property
// values for each node.
//
// For each property p in P, a node N will store its "local" property value.  This can be
// thought of as a "delta" from the parent node's "global" property value.  The root node
// has no parent, so its parent's global property values are defined by convention to be
// the identity.  The "global" property value of a node is defined to be the node's local
// property value applied to the global property value of its parent.  This concludes a
// full, inductive definition of the local and global property values of nodes.
//
// Properties that we care about:
// - Affine transformations (positioning and transforming the child relative to the parent,
//   in the parent's coordinates).  The identity is the identity transform and application
//   operation is transform multiplication.
// - Alpha mask (propagating transparency to all children of a partially transparent object).
//   The identity for alpha mask is 1.0 and the application operation is multiplication.
// - Name -- the identity is the empty string and application operation is concatenation with
//   a '/' or perhaps '.' character.
//
// Example:
//   Scene graph         Local name       Global name        Local alpha      Global alpha
//       A                   "A"             "/A"                1.0              1.0
//        \    '
//         B                 "B"             "/A/B"              0.8              0.8
//        /
//       C                   "C"             "/A/B/C"            0.7              0.56
//
// The property "delta" from a node N's ancestor A to itself can be defined to be the application
// to the identity of the local properties of each node in the line of ancestry, starting
// with A and ending with N (inclusive).
//
// Additionally, some of the property application operations may be invertible, in which case
// the deltas can be inverted.  The application operation allows any deltas to be composed.
// For example, this allows the affine coordinate transformation between two nodes' local
// coordinate systems to be computed.
//
// Invertible property application operations:
// - Affine transformation: operation is transform (matrix) multiplication and the inverse
//   operation is multiplication of inverse of transform (matrix).
// - Name: operation is concatenation with '/' separators and the inverse operation
//   is application with the local name ".." (just as in ordinary file systems).
//
// Traversal of a scene graph could be done via the iterator pattern.  A well-defined
// traversal order gives a linear order on the nodes.  An iterator would point to the
// "current" node and that node's global properties.  The iteration procedure would
// perform the stack operations necessary to compute and provide each node's global
// property values.  Probably for now all we care about is doing depth-first traversal,
// though other orders could be implemented relatively easily as an orthogonal feature.
//
// Another feature that is required by our use cases is a "property application mode" which
// would allow different types of property application.  Let $ indicate the application
// operation.  The current modes are:
// - APPLY:
//   child's global property = parent's global property $ child's local property
// - REPLACE:
//   child's global property = child's local property

// see http://en.wikipedia.org/wiki/Scene_graph
// The Properties type must have a default constructor that initializes all member
// properties to their respective "identity" values.
// TODO: make a "Derived" template param which is used as the type of the parent
// and child nodes, so no casting needs to be done.
template <typename Properties>
class SceneGraphNode : public std::enable_shared_from_this<SceneGraphNode<Properties>> {
public:

  typedef std::vector<std::shared_ptr<SceneGraphNode>,
    Eigen::aligned_allocator<std::shared_ptr<SceneGraphNode>>
  > ChildSet;

  // This initializes all local properties to their respective identity values.
  SceneGraphNode() { }
  virtual ~SceneGraphNode() { }

  using std::enable_shared_from_this<SceneGraphNode>::shared_from_this;

  const std::shared_ptr<SceneGraphNode>& Parent () const { return m_parent.lock(); }
  const ChildSet& Children() const { return m_children; }
  ChildSet& Children() { return m_children; }

  // these are virtual so that particular behavior can be added while adding/removing nodes.
  // any overrides should make sure to call the base class' version of the method, of course.
  virtual void AddChild(std::shared_ptr<SceneGraphNode> child) {
    m_children.emplace_back(child);
    try {
      child->m_parent = shared_from_this();
    } catch (const std::bad_weak_ptr&) {
      child->m_parent.reset(); // Unable to obtain weak pointer (parent most likely isn't a shared pointer)
    }
  }
  virtual void RemoveChild(std::shared_ptr<SceneGraphNode> child) {
    auto found = std::find(std::begin(m_children), std::end(m_children), child);
    if (found != std::end(m_children)) {
      m_children.erase(found);
    }
  }
  virtual void RemoveFromParent() {
    std::shared_ptr<SceneGraphNode> parent = m_parent.lock();
    if (parent) {
      try {
        parent->RemoveChild(shared_from_this());
      } catch (const std::bad_weak_ptr&) {}
      m_parent.reset();
    }
  }

  // TODO: make iterator-based traversal
  template <typename DerivedNode>
  void DepthFirstTraverse (const std::function<void(const DerivedNode &node,
                                                    const Properties &global_properties)> &callback,
                           const Properties &parent_global_properties = Properties()) const {
    // Using the parent's global properties, compute this node's global properties.
    Properties global_properties(parent_global_properties);
    global_properties.Apply(LocalProperties(), Operate::ON_RIGHT);
    assert(dynamic_cast<const DerivedNode *>(this) != nullptr && "this node isn't actually of the requested DerivedNode type");
    // Call the callback on this node.
    callback(*static_cast<const DerivedNode *>(this), global_properties);
    // Call this function recursively on all child nodes.
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
      assert(bool(*it));
      const SceneGraphNode &child = **it;
      child.DepthFirstTraverse(callback, global_properties);
    }
  }
  template <typename DerivedNode>
  void DepthFirstTraverse (const std::function<void(DerivedNode &node,
                                                    const Properties &global_properties)> &callback,
                           const Properties &parent_global_properties = Properties()) {
    // Using the parent's global properties, compute this node's global properties.
    Properties global_properties(parent_global_properties);
    global_properties.Apply(LocalProperties(), Operate::ON_RIGHT);
    assert(dynamic_cast<DerivedNode *>(this) != nullptr && "this node isn't actually of the requested DerivedNode type");
    // Call the callback on this node.
    callback(*static_cast<DerivedNode *>(this), global_properties);
    // Call this function recursively on all child nodes.
    for (auto it = m_children.begin(); it != m_children.end(); ++it) {
      assert(bool(*it));
      SceneGraphNode &child = **it;
      child.DepthFirstTraverse(callback, global_properties);
    }
  }

  // // Traverse this tree, depth-first, calling preTraversal (if not nullptr)
  // // on this node, then calling this function recursively on the child nodes, then
  // // calling postTraversal (if not nullptr) on this node.
  // template<class _FnPre, class _FnPost>
  // void DepthFirstTraverse(_FnPre preTraversal, _FnPost postTraversal) {
  //   CallFunction(preTraversal, *this);
  //   for (auto it = m_children.begin(); it != m_children.end(); ++it) {
  //     const std::shared_ptr<SceneGraphNode> &child = *it;
  //     assert(bool(child));
  //     child->DepthFirstTraverse(preTraversal, postTraversal);
  //   }
  //   CallFunction(postTraversal,*this);
  // }

  // //As above, but const.
  // template<class _FnPre, class _FnPost>
  // void DepthFirstTraverse(_FnPre preTraversal, _FnPost postTraversal) const {
  //   CallFunction(preTraversal, *this);
  //   for (auto it = m_children.begin(); it != m_children.end(); ++it) {
  //     const std::shared_ptr<const SceneGraphNode> &child = *it;
  //     assert(bool(child));
  //     child->DepthFirstTraverse(preTraversal, postTraversal);
  //   }
  //   CallFunction(postTraversal, *this);
  // }

  // The local properties give this node's properties as a "delta" to its parents'.
  const Properties &LocalProperties () const { return m_local_properties; }
  Properties &LocalProperties () { return m_local_properties; }
  Properties GlobalProperties () const { return PropertiesDeltaToRootNode(); }
  // TODO: (maybe) provide GlobalProperties method which applies the ancestor property stack

  // This returns the "global properties" of this node, i.e. the composition
  // of the properties of the ancestor line of this node.
  //
  // As an example, if one of the properties is the affine transformation giving
  // the transformation from a child's coordinate system to its parent's, then
  // the corresponding property in the return value will be the affine transformation
  // giving the transformation from this node's coordinate system to the global
  // coordinate system.
  Properties PropertiesDeltaToRootNode () const {
    Properties retval; // Initialized to the identity properties.
    auto node = shared_from_this();
    // Apply, on the left, each ancestor's local properties.  Continue
    // until the root is reached.
    do {
      retval.Apply(node->LocalProperties(), Operate::ON_LEFT);
      node = node->m_parent.lock();
    } while (node);
    return retval;
  }
  // This returns the inverse of the PropertiesDeltaToRootNode() value.
  //
  // As an example, if one of the properties is the affine transformation giving
  // the transformation from a child's coordinate system to its parent's, then
  // the corresponding property in the return value will be the affine transformation
  // giving the transformation from the global coordinate system to this node's
  // coordinate system.
  Properties PropertiesDeltaFromRootNode () const { return PropertiesDeltaToRootNode().Inverse(); }

  // Compute the property delta from this node to the given node.  This is defined
  // to be the Properties value necessary to apply to this node's global properties
  // to get the other node's global properties.  Some of the properties may not
  // be invertible.  Each non-invertible property delta will be invalid.  Some of
  // the invertible property values may not have an inverse (analogous to the
  // inability to divide by zero or invert a singular matrix), in which case, those
  // property deltas will be invalid.
  //
  // As an example, if one of the properties is the affine transformation giving
  // the transformation from a child's coordinate system to its parent's, then
  // the delta for that property will be the affine transformation taking this
  // node's coordinate system to the other node's coordinate system.
  Properties PropertiesDeltaTo (const SceneGraphNode &other) const {
    auto closest_common_ancestor = ClosestCommonAncestor(other);

    // Compute the properties of this node with respect to the common ancestor.
    // For convenience in later comments, call this A.
    Properties this_properties_stack;
    this_properties_stack.SetIdentity();
    auto traversal_node = shared_from_this();
    while (traversal_node != closest_common_ancestor) {
      // Because we are traversing from this node up through its ancestry,
      // each properties must be applied on the left.
      this_properties_stack.Apply(traversal_node->LocalProperties(), Operate::ON_LEFT);
      traversal_node = traversal_node->m_parent.lock();
    }

    // Compute the properties of the other node with respect to the common ancestor.
    // For convenience in later comments, call this B.
    Properties other_properties_stack;
    other_properties_stack.SetIdentity();
    traversal_node = other.shared_from_this();
    while (traversal_node != closest_common_ancestor) {
      // Because we are traversing from the other node up through its ancestry,
      // each properties must be applied on the left.
      other_properties_stack.Apply(traversal_node->LocalProperties(), Operate::ON_LEFT);
      traversal_node = traversal_node->m_parent.lock();
    }

    // The property delta is computed by first applying A and then applying B inverse
    // i.e. B^{-1} * A.  Thus B is applied on the left of A.
    other_properties_stack.Invert();
    this_properties_stack.Apply(other_properties_stack, Operate::ON_LEFT);
    return this_properties_stack;
  }

  // // This computes the transformation taking points in this node's coordinate
  // // system and produces those points expressed in the coordinate system of
  // // the other node.
  // //
  // // The transform for a node gives its transform taking its parent's coordinate
  // // system to its coordinate system.
  // Transform ComputeTransformToCoordinatesOf (const SceneGraphNode &other) const {
  //   auto closest_common_ancestor = ClosestCommonAncestor(other);

  //   // Compute the transformation from this node's coordinate system to the
  //   // common ancestor's.  For convenience in later comments, call this A.
  //   Transform this_transform_stack;
  //   this_transform_stack.setIdentity();
  //   auto traversal_node = shared_from_this();
  //   while (traversal_node != closest_common_ancestor) {
  //     // A node's transform gives the node-to-parent coordinate transformation.
  //     this_transform_stack = this_transform_stack * traversal_node->FullTransform();
  //     traversal_node = traversal_node->m_parent.lock();
  //   }

  //   // Compute the transformation from the other node's coordinate system
  //   // to the common ancestor's.  For convenience in later comments, call this B.
  //   Transform other_transform_stack;
  //   other_transform_stack.setIdentity();
  //   traversal_node = other.shared_from_this();
  //   while (traversal_node != closest_common_ancestor) {
  //     other_transform_stack = other_transform_stack * traversal_node->FullTransform();
  //     traversal_node = traversal_node->m_parent.lock();
  //   }

  //   // TODO: somehow check that other_transform_stack is actually invertible (it's not
  //   // clear that this functionality is directly provided via Eigen::Transform).

  //   // The total transformation is first applying A and then applying B inverse.
  //   // Because transforms act on the left of vectors, this ordering of the operands
  //   // (as B^{-1} * A) is necessary.
  //   return other_transform_stack.inverse(Eigen::Affine) * this_transform_stack;
  // }

  std::shared_ptr<const SceneGraphNode> RootNode() const {
    auto node = shared_from_this();
    auto parent = node->m_parent.lock();
    while (parent) {
      node = parent;
      parent = node->m_parent.lock();
    }
    return node;
  }

  // Transform ComputeTransformToGlobalCoordinates() const {
  //   return ComputeTransformToCoordinatesOf(*RootNode());
  // }

  // Transform ComputeTransformFromGlobalCoordinates() const {
  //   return RootNode()->ComputeTransformToCoordinatesOf(*this);
  // }

  // This will return an empty shared_ptr if there was no common ancestor, which
  // should happen if and only if the two nodes come from different scene graph trees.
  std::shared_ptr<const SceneGraphNode> ClosestCommonAncestor (const SceneGraphNode &other) const {
    // TODO: develop ancestry lists for each, then find the last common one, root-down.
    std::vector<std::shared_ptr<const SceneGraphNode>> this_ancestors;
    std::vector<std::shared_ptr<const SceneGraphNode>> other_ancestors;
    this->AppendAncestors(this_ancestors);
    other.AppendAncestors(other_ancestors);

    std::shared_ptr<SceneGraphNode> retval;
    // "zip" together the lists, starting at the end (which is the root), and determine
    // the last matching ancestor.
    auto this_it = this_ancestors.rbegin();
    auto other_it = other_ancestors.rbegin();
    while (this_it != this_ancestors.rend() && other_it != other_ancestors.rend() && *this_it == *other_it) {
      ++this_it;
      ++other_it;
    }
    // we went one too far, so back up one.
    --this_it;
    // if this iterator is valid, then return what it points to.
    if (this_it >= this_ancestors.rbegin()) {
      return *this_it;
    } else { // otherwise return empty.
      return std::shared_ptr<const SceneGraphNode>();
    }
  }

private:

  // //Silly function call wrapper that allows you to also pass nullptr as a function if you want.
  // template<class _Fn, typename... _Args>
  // static void CallFunction(_Fn function, _Args&& ... args) { function(args...); }

  // template<typename... _Args>
  // static void CallFunction(std::nullptr_t, _Args&&...) {}

  // This populates a vector with the ancestors of this node, starting with this node,
  // then its parent, then its parent's parent, etc (i.e. this node, going toward the root).
  void AppendAncestors (std::vector<std::shared_ptr<const SceneGraphNode>> &ancestors) const {
    ancestors.emplace_back(shared_from_this());
    std::shared_ptr<SceneGraphNode> parent(m_parent.lock());
    if (parent) {
      parent->AppendAncestors(ancestors);
    }
  }

  // // The transform member gives the coordinate transformation (an affine transformation)
  // // from this node's coordinate system to its parent's.  For the root node, the "parent
  // // coordinate system" is the standard coordinate system (which can be thought of as
  // // some kind of global coordinates).
  // Transform m_transform;

  Properties m_local_properties;

  // This uses a weak_ptr to avoid a cycle of shared_ptrs which would then be indestructible.
  std::weak_ptr<SceneGraphNode> m_parent;
  // This is the set of all child nodes.
  ChildSet m_children;
};
