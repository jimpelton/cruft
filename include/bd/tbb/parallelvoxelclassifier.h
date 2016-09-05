//
// Created by Jim Pelton on 8/31/16.
//

#ifndef CRUFTERLY_PARALLELVOXELCLASSIFIER_H
#define CRUFTERLY_PARALLELVOXELCLASSIFIER_H

#include <bd/io/buffer.h>

#include <tbb/tbb.h>

#include <vector>
#include <functional>

namespace bd
{

/// \brief Classifies voxels as relevant or irrelevant.
///        The classification is saved in the voxel relevance map.
///        A 1 means relevant, a 0 means irrelevant.
template<typename Ty>
class ParallelVoxelClassifier
{
public:

  ParallelVoxelClassifier(std::vector<bool> *map,
                          Ty const * input,
                          std::function<bool(Ty)> relevant)
      : m_map{ map }
      , m_input{ input }
      , m_isRel{ relevant }
  {
  }

  ParallelVoxelClassifier(ParallelVoxelClassifier<Ty> const &rhs, tbb::split)
      : m_map{ rhs.m_map }
      , m_input{ rhs.m_input }
      , m_isRel{ rhs.m_isRel }
  {
  }

  ~ParallelVoxelClassifier()
  {
  }

  void operator()(tbb::blocked_range<size_t> const &r) const
  {
    for(size_t i{ r.begin() }; i != r.end(); ++i) {
      (*m_map)[i] = m_isRel( m_input[i] );
    }
  }

private:
  std::vector<bool> * m_map;  ///< Relevance map
  Ty const * const m_input;
  std::function<bool(Ty)> &m_isRel;

}; // class ParallelVoxelClassifier

} // namespace bd

#endif // ! CRUFTERLY_PARALLELVOXELCLASSIFIER_H
