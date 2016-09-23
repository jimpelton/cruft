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

    /**
     * +--------------------------------------------------+
     * |   Giant block of data                            |
     * +--------------------------------------------------+
     *                      ^
     *                      |
     *                      data points to start of buffer
     *                      offset = element index of data
     * Since m_map is a ptr to vector<bool>, accesses need to
     * be offset into the same element index that the buffer
     * starts at.
     */
    size_t mIdx{ r.begin() + offset };
    for(size_t i{ r.begin() }; i != r.end(); ++i, ++mIdx) {
      (*m_map)[mIdx] = m_isRel( data[i] );
    }
  }

private:
  std::vector<bool> * const m_map;  ///< Relevance map
  Buffer<Ty> const * m_buf;
  Function const &m_isRel;

}; // class ParallelVoxelClassifier

} // namespace bd

#endif // ! bd_parallelvoxelclassifier_h__
