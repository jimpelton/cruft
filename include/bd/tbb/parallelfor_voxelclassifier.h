//
// Created by Jim Pelton on 8/31/16.
//

#ifndef bd_parallelvoxelclassifier_h__
#define bd_parallelvoxelclassifier_h__

#include <bd/io/buffer.h>

#include <tbb/tbb.h>

#include <vector>
#include <functional>

namespace bd
{

/// \brief Classifies individual voxels as relevant or irrelevant.
///        The classification is saved in the voxel relevance map.
template<typename Ty, typename Function, typename Storage>
class ParallelForVoxelClassifier
{
public:

  ParallelForVoxelClassifier(Storage &map,
                          Buffer<Ty> const *buf,
                          Function const &relevant)
      : m_map{ &map }
      , m_buf{ buf }
      , m_isRel{ relevant }
  {
  }

  ParallelForVoxelClassifier(ParallelForVoxelClassifier const &rhs, tbb::split)
      : m_map{ rhs.m_map }
      , m_buf{ rhs.m_buf }
      , m_isRel{ rhs.m_isRel }
  {
  }

  ~ParallelForVoxelClassifier()
  {
  }

  void operator()(tbb::blocked_range<size_t> const &r) const
  {
    Ty const * const data{ m_buf->getPtr() };

    for(size_t i{ r.begin() }; i != r.end(); ++i) {
      (*m_map)[i] = m_isRel( data[i] );
    }
  }

private:
  Storage * const m_map;  ///< Relevance map
  Buffer<Ty> const * m_buf;
  Function const &m_isRel;

}; // class ParallelForVoxelClassifier

} // namespace bd

#endif // ! bd_parallelvoxelclassifier_h__
