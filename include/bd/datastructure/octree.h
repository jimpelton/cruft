//
// Created by jim on 9/14/16.
//

#ifndef bd_octree_h
#define bd_octree_h

#include <array>

namespace bd
{

template<typename Ty>
class Octree
{
public:
  Octree()
  {
  }


  ~Octree()
  {
  }


private:
  std::array<Octree<Ty>*, 8> m_children;

}; // class Octree

} // namespace bd

#endif // ! bd_octree_h
