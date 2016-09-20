//
// Created by jim on 9/14/16.
//
// This code was originally from https://github.com/brandonpelfrey/SimpleOctree
// I have templated the code and in the near future will change it to use
// a flat array for node storage instead of pointers.
//

#ifndef bd_octree_h
#define bd_octree_h

#include <glm/glm.hpp>
#include <array>
#include <vector>
#include <tuple>

namespace bd
{

/// \brief A very simple Octree implementation.
/// Template Parameters:
///   Vec3Ty -  The type of 3D vector to use for origin coordinates.
///   DataTy - The data type stored in this tree.
///
template<typename DataTy, typename Vec3Ty=glm::vec3>
class Octree
{

public:

  ~Octree()
  {
  }

  Octree(Vec3Ty const &origin, Vec3Ty const &halfDim, int maxDepth, int /*startDepth=0*/)
      : m_origin{ origin }
      , m_halfDim{ halfDim }
      , m_depth{ 0 }
      , m_data{ nullptr }
  {
    m_maxDepth = maxDepth;
  }

private:

  Octree(Vec3Ty const &origin, Vec3Ty const &halfDim, int depth)
      : m_origin{ origin }
      , m_halfDim{ halfDim }
      , m_depth{ depth }
      , m_data{ nullptr }
  {
  }

public:

  // Determine which octant of the tree would contain 'point'
  int
  getOctantContainingPoint(Vec3Ty const &point) const
  {
    int oct = 0;
    if (point.x >= m_origin.x) {
      oct |= 4;
    }
    if (point.y >= m_origin.y) {
      oct |= 2;
    }
    if (point.z >= m_origin.z) {
      oct |= 1;
    }
    return oct;
  }


  bool
  isLeafNode() const
  {
    // We are a leaf iff we have no children. Since we either have none, or
    // all eight, it is sufficient to just check the first.
    return m_nodes[8 * m_depth + 1] == EmptyNode;
  }


  void
  insert(DataTy *data, Vec3Ty point)
  {
    // If this node doesn't have a data point yet assigned
    // and it is a leaf, then we're done!
    if (isLeafNode()) {
      if (m_data == nullptr) {
        m_data = data;
        return;
      } else {
        // We're at a leaf, but there's already something here
        // We will split this node so that it has 8 child octants
        // and then insert the old data that was here, along with
        // this new data point

        // Save this data point that was here for a later re-insert
        DataTy *oldData= m_data;
        Vec3Ty oldPoint = m_origin;
        m_data = nullptr;

        // Split the current node and create new empty trees for each
        // child octant.
        for (int i = 1; i <= 8; ++i) {
          // Compute new bounding box for this child
          Vec3Ty newOrigin = m_origin;

          newOrigin.x +=
              static_cast<decltype(newOrigin.x)>(m_halfDim.x * ( i & 4 ? 0.5f : -0.5f ));
          newOrigin.y +=
              static_cast<decltype(newOrigin.y)>(m_halfDim.y * ( i & 2 ? 0.5f : -0.5f ));
          newOrigin.z +=
              static_cast<decltype(newOrigin.z)>(m_halfDim.z * ( i & 1 ? 0.5f : -0.5f ));

          m_nodes[8 * m_depth + i] = new Octree<DataTy, Vec3Ty>(newOrigin, m_halfDim * 0.5f, m_depth + 1);
        }

        // Re-insert the old point, and insert this new point
        // (We wouldn't need to insert from the root, because we already
        // know it's guaranteed to be in this section of the tree)
        m_nodes[8 * m_depth + getOctantContainingPoint(oldPoint)]->insert(oldData, oldPoint);
        m_nodes[8 * m_depth + getOctantContainingPoint(point)]->insert(data, point);
      }
    } else {
      // We are at an interior node. Insert recursively into the
      // appropriate child octant
      int octant = getOctantContainingPoint(point);
      m_nodes[8 * m_depth + octant]->insert(data, point);
    }
  }


  bool
  operator==(Octree const &rhs) const
  {
    return std::tie(m_origin, m_halfDim, m_depth) ==
        std::tie(rhs.m_origin, rhs.m_halfDim, rhs.m_depth);
  }


  bool
  operator!=(Octree const &rhs) const
  {
    return !( rhs == *this );
  }


  // This is a really simple routine for querying the tree for points
  // within a bounding box defined by min/max points (bmin, bmax)
  // All results are pushed into 'results'
//  void
//  getPointsInsideBox(const VecTy &bmin, const VecTy &bmax, std::vector<Octree *> &results)
//  {
//    // If we're at a leaf node, just see if the current data point is inside
//    // the query bounding box
//    if (isLeafNode()) {
//      if (m_data != nullptr) {
//        const VecTy &p = m_data->getPosition();
//        if (p.x > bmax.x || p.y > bmax.y || p.z > bmax.z) {
//          return;
//        }
//        if (p.x < bmin.x || p.y < bmin.y || p.z < bmin.z) {
//          return;
//        }
//        results.push_back(m_data);
//      }
//    } else {
//      // We're at an interior node of the tree. We will check to see if
//      // the query bounding box lies outside the octants of this node.
//      for (int i = 0; i < 8; ++i) {
//        // Compute the min/max corners of this child octant
//        VecTy cmax = m_children[i]->origin + m_children[i]->halfDimension;
//        VecTy cmin = m_children[i]->origin - m_children[i]->halfDimension;
//
//        // If the query rectangle is outside the child's bounding box,
//        // then continue
//        if (cmax.x < bmin.x || cmax.y < bmin.y || cmax.z < bmin.z) {
//          continue;
//        }
//        if (cmin.x > bmax.x || cmin.y > bmax.y || cmin.z > bmax.z) {
//          continue;
//        }
//
//        // At this point, we've determined that this child is intersecting
//        // the query bounding box
//        m_children[i]->getPointsInsideBox(bmin, bmax, results);
//      }
//    }
//  }


private:
  static std::vector<Octree<DataTy, Vec3Ty>*> m_nodes;
  static int m_maxDepth;
  Vec3Ty m_origin;
  Vec3Ty m_halfDim;
  int m_depth;   ///< Depth of this node.
  DataTy *m_data; ///< The data inside this node.

}; // class Octree

template<class DataTy, class Vec3Ty>
int Octree<DataTy, Vec3Ty>::m_maxDepth{ 0 };

template<class DataTy, class Vec3Ty>
std::vector<Octree<DataTy, Vec3Ty> *>
    Octree<DataTy, Vec3Ty>::m_nodes;

} // namespace bd

#endif // ! bd_octree_h
