//
// Created by jim on 9/14/16.
//

#ifndef bd_octree_h
#define bd_octree_h

#include <glm/glm.hpp>
#include <array>

namespace bd
{

    /// \brief A very simple Octree implementation.
    /// Template Parameters:
    ///   VecTy -  The type of 3D vector to use for min, max corner points.
    ///            For example, I want to use  an int type vector for 
    ///            because my octree will subdivide an array, and I want to use
    ///            min max points for array indexes.
    ///   DataTy - The data type stored in this tree.
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


  void


private:
  std::array<Octree<VecTy, DataTy>*, 8> m_children;
  VecTy m_origin;
  VecTy m_halfDim;

  DataTy *m_data;

}; // class Octree

} // namespace bd

#endif // ! bd_octree_h
