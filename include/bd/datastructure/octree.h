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

namespace bd
{

/// \brief A very simple Octree implementation.
/// Template Parameters:
///   VecTy -  The type of 3D vector to use for origin coordinates.
///   DataTy - The data type stored in this tree.
///
template<typename VecTy, typename DataTy>
class Octree
{
public:
  Octree(VecTy const &origin, VecTy const &halfDim)
    : m_origin{ origin }
    , m_halfDim{ halfDim }
    , m_data{ nullptr }
  {
  }


  ~Octree()
  {
  }

  // Determine which octant of the tree would contain 'point'
  int
  getOctantContainingPoint(VecTy const & point) const
  {
    int oct = 0;
    if(point.x >= m_origin.x) oct |= 4;
    if(point.y >= m_origin.y) oct |= 2;
    if(point.z >= m_origin.z) oct |= 1;
    return oct;
  }

  bool
  isLeafNode() const
  {
    // We are a leaf iff we have no children. Since we either have none, or
    // all eight, it is sufficient to just check the first.
    return m_children[0] == NULL;
  }


  void
  insert(DataTy *point)
  {
    // If this node doesn't have a data point yet assigned
    // and it is a leaf, then we're done!
    if(isLeafNode()) {
      if(m_data == nullptr) {
        m_data = point;
        return;
      } else {
        // We're at a leaf, but there's already something here
        // We will split this node so that it has 8 child octants
        // and then insert the old data that was here, along with
        // this new data point

        // Save this data point that was here for a later re-insert
        DataTy *oldPoint = m_data;
        m_data = NULL;

        // Split the current node and create new empty trees for each
        // child octant.
        for(int i=0; i<8; ++i) {
          // Compute new bounding box for this child
          VecTy newOrigin = m_origin;
          newOrigin.x += m_halfDim.x * (i&4 ? .5f : -.5f);
          newOrigin.y += m_halfDim.y * (i&2 ? .5f : -.5f);
          newOrigin.z += m_halfDim.z * (i&1 ? .5f : -.5f);
          m_children[i] = new Octree(newOrigin, m_halfDim * 0.5f);
        }

        // Re-insert the old point, and insert this new point
        // (We wouldn't need to insert from the root, because we already
        // know it's guaranteed to be in this section of the tree)
        m_children[getOctantContainingPoint(oldPoint->getPosition())]->insert(oldPoint);
        m_children[getOctantContainingPoint(point->getPosition())]->insert(point);
      }
    } else {
      // We are at an interior node. Insert recursively into the
      // appropriate child octant
      int octant = getOctantContainingPoint(point->getPosition());
      m_children[octant]->insert(point);
    }
  }

  // This is a really simple routine for querying the tree for points
  // within a bounding box defined by min/max points (bmin, bmax)
  // All results are pushed into 'results'
  void
  getPointsInsideBox(const VecTy& bmin, const VecTy& bmax, std::vector<Octree*>& results)
  {
    // If we're at a leaf node, just see if the current data point is inside
    // the query bounding box
    if(isLeafNode()) {
      if(m_data!=NULL) {
        const VecTy& p = m_data->getPosition();
        if(p.x>bmax.x || p.y>bmax.y || p.z>bmax.z) return;
        if(p.x<bmin.x || p.y<bmin.y || p.z<bmin.z) return;
        results.push_back(m_data);
      }
    } else {
      // We're at an interior node of the tree. We will check to see if
      // the query bounding box lies outside the octants of this node.
      for(int i=0; i<8; ++i) {
        // Compute the min/max corners of this child octant
        VecTy cmax = m_children[i]->origin + m_children[i]->halfDimension;
        VecTy cmin = m_children[i]->origin - m_children[i]->halfDimension;

        // If the query rectangle is outside the child's bounding box,
        // then continue
        if(cmax.x<bmin.x || cmax.y<bmin.y || cmax.z<bmin.z) continue;
        if(cmin.x>bmax.x || cmin.y>bmax.y || cmin.z>bmax.z) continue;

        // At this point, we've determined that this child is intersecting
        // the query bounding box
        m_children[i]->getPointsInsideBox(bmin,bmax,results);
      }
    }
  }

private:
  std::array<Octree<VecTy, DataTy>*, 8> m_children;
  VecTy m_origin;
  VecTy m_halfDim;

  DataTy *m_data;

}; // class Octree

} // namespace bd

#endif // ! bd_octree_h
