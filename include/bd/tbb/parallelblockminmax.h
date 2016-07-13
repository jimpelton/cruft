//
// Created by jim on 6/15/16.
//




#ifndef CRUFTERLY_PARALLELBLOCKMINMAX_H_H
#define CRUFTERLY_PARALLELBLOCKMINMAX_H_H

#include <bd/volume/volume.h>
#include <bd/io/buffer.h>

#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

namespace bd
{

template<typename Ty>
class MinMaxPair
{
public:
  MinMaxPair()
    : min{ std::numeric_limits<Ty>::max() }
    , max{ std::numeric_limits<Ty>::lowest() }
    , total{ 0 }
  { }

  Ty min;
  Ty max;
  Ty total;
};

class MinMaxPairDouble : public MinMaxPair<double> {};

/// \brief Compute the min and max values for each block
///        associated with values in the BlockedRange.
/// \note  For use with parallel_reduce() in Intel TBB.
template<typename Ty>
class ParallelBlockMinMax
{
public:
  ParallelBlockMinMax(Volume const *v, Buffer<Ty> const *b)
    : m_volume{ v }
    , m_data{ b->ptr() }
    , m_voxelStart{ b->index() }
    , m_pairs{ new MinMaxPairDouble[ v->lower().total_block_count() ] }
  {
  }

  ~ParallelBlockMinMax()
  {
    if (m_pairs)
      delete [] m_pairs;
  }

  ParallelBlockMinMax(ParallelBlockMinMax const &o, tbb::split)
    : m_volume{ o.m_volume }
    , m_data{ o.m_data }
    , m_voxelStart{ o.m_voxelStart }
    , m_pairs{ new MinMaxPairDouble[ o.m_volume->lower().total_block_count() ] }
  {
  }

  void join(ParallelBlockMinMax &rhs)
  {
    for(uint64_t i{ 0 }; i < m_volume->lower().total_block_count(); ++i) {
      if (m_pairs[i].min > rhs.m_pairs[i].min)
        m_pairs[i].min = rhs.m_pairs[i].min;

      if (m_pairs[i].max < rhs.m_pairs[i].max)
        m_pairs[i].max = rhs.m_pairs[i].max;

      m_pairs[i].total += rhs.m_pairs[i].total;
    }
  }


  void operator()(tbb::blocked_range<size_t> const & r)
  {
    const uint64_t
    vdX = m_volume->dims().x,
    vdY = m_volume->dims().y,
    bdX = m_volume->lower().block_dims().x,
    bdY = m_volume->lower().block_dims().y,
    bdZ = m_volume->lower().block_dims().z,
    bcX = m_volume->lower().block_count().x,
    bcY = m_volume->lower().block_count().y,
    bcZ = m_volume->lower().block_count().z;

    Ty const * const a{ m_data };

    for (size_t i{ r.begin() }; i != r.end(); ++i) {

      // Convert vIdx (the voxel index within the entire volume data set)
      // into a 3D index, then convert the 3D voxel index obtained from
      // vIdx into a 3D block index to be used in determining if our voxel
      // is within the part of the volume covered by our blocks.
      uint64_t vIdx, bI, bJ, bK, bIdx;
      vIdx = i + m_voxelStart;
      bI = (vIdx % vdX) / bdX;
      bJ = ((vIdx / vdX) % vdY) / bdY;
      bK = ((vIdx / vdX) / vdY) / bdZ;

      if (bI < bcX && bJ < bcY && bK < bcZ) {
        Ty val = a[i];

        // Convert the 3D block index into a 1D block index and fetch the
        // block from the array of blocks.
        bIdx = bI + bcX * (bJ + bK * bcY);
        MinMaxPairDouble *b = &m_pairs[bIdx];

        // Accumulate block-specific values.
        if (val < b->min) { b->min = val; }
        if (val > b->max) { b->max = val; }
        b->total += val;
      }
    }
  }

//  static void SetGlobalBlocksPtr(FileBlock const * const *ptr, size_t sz)
//  {
//    if (GLOBAL_BLOCKS == nullptr) {
//      GLOBAL_BLOCKS = ptr;
//      NUM_BLOCKS = sz;
//    }
//  }

  Volume const * const m_volume;
  Ty const * const m_data;
  size_t const m_voxelStart;
  MinMaxPairDouble * const m_pairs;



private:
  // static FileBlock const * const *GLOBAL_BLOCKS = nullptr;

//  void copyBlocks()
//  {
//    for(size_t i{ 0 }; i < NUM_BLOCKS; ++i) {
//      m_pairs[i] = *GLOBAL_BLOCKS[i];
//    }
//  }

}; // class ParallelBlockMinMax

} // namespace bd

#endif //CRUFTERLY_PARALLELBLOCKMINMAX_H_H
