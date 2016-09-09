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

/// \brief Classifies voxels as relevant or irrelevant.
///        The classification is saved in the voxel relevance map.
///        A 1 means relevant, a 0 means irrelevant.
template<typename Ty, typename Function>
class ParallelVoxelClassifier
{
public:

  ParallelVoxelClassifier(std::vector<bool> *map,
                          Buffer<Ty> const *buf,
                          Function const &relevant)
      : m_map{ map }
      , m_buf{ buf }
      , m_isRel{ relevant }
  {
  }

  ParallelVoxelClassifier(ParallelVoxelClassifier<Ty, Function> const &rhs, tbb::split)
      : m_map{ rhs.m_map }
      , m_buf{ rhs.m_buf }
      , m_isRel{ rhs.m_isRel }
  {
  }

  ~ParallelVoxelClassifier()
  {
  }

  void operator()(tbb::blocked_range<size_t> const &r) const
  {
    Ty const * const data{ m_buf->getPtr() };
    size_t const offset{ m_buf->getIndexOffset() };

    for(size_t i{ r.begin() }; i != r.end(); ++i) {
      (*m_map)[i+offset] = m_isRel( data[i] );
    }
  }

private:
  std::vector<bool> * m_map;  ///< Relevance map
  Buffer<Ty> const * m_buf;
  Function m_isRel;

}; // class ParallelVoxelClassifier

} // namespace bd

#endif // ! bd_parallelvoxelclassifier_h__
