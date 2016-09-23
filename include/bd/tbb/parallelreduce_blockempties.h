#ifndef bd_parallelblockstats_h
#define bd_parallelblockstats_h

#include <bd/io/fileblock.h>
#include <bd/io/buffer.h>
#include <bd/volume/volume.h>


#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

#include <functional>


namespace bd
{

/// \brief Counts the number of empty voxels in each block.
///
template<typename Ty>
class ParallelReduceBlockEmpties
{
public:

  ParallelReduceBlockEmpties(Buffer<Ty> *b, Volume const *v, std::function<bool(Ty)> isEmpty)
    : m_data{ b->getPtr() }
    , m_volume{ v }
    , m_voxelStart{ b->getIndexOffset() }
    , m_empties{ nullptr }
    , isRelevant{ isEmpty }
  {
    m_empties = new uint64_t[m_volume->lower().total_block_count()]();
  }

  ParallelReduceBlockEmpties(ParallelReduceBlockEmpties<Ty> &o, tbb::split)
      : m_data{ o.m_data }
      , m_volume{ o.m_volume }
      , m_voxelStart{ o.m_voxelStart }
      , m_empties{ nullptr }
      , isRelevant{ o.isRelevant }
  {
    m_empties = new uint64_t[o.m_volume->lower().total_block_count()]();
  }

  ~ParallelReduceBlockEmpties()
  {
    if (m_empties) {
      delete [] m_empties;
    }
  }

  void
  operator()(tbb::blocked_range<size_t> const &r)
  {
    Ty const * const a{ m_data };

    uint64_t const vdX{ m_volume->dims().x };
    uint64_t const vdY{ m_volume->dims().y };
    uint64_t const bdX{ m_volume->lower().block_dims().x };
    uint64_t const bdY{ m_volume->lower().block_dims().y };
    uint64_t const bdZ{ m_volume->lower().block_dims().z };
    uint64_t const bcX{ m_volume->lower().block_count().x };
    uint64_t const bcY{ m_volume->lower().block_count().y };
    uint64_t const bcZ{ m_volume->lower().block_count().z };
    uint64_t const voxelStart{ m_voxelStart };

    uint64_t vIdx, // voxel 1D index
        bI,        // block i index
        bJ,        // block j index
        bK,        // block k index
        bIdx;      // block 1D index

    for (size_t i{ r.begin() }; i != r.end(); ++i) {

      // Convert vIdx (the voxel index within the entire volume data set)
      // into a 3D index, then convert the 3D voxel index obtained from
      // vIdx into a 3D block index to be used in determining if our voxel
      // is within the part of the volume covered by our blocks.
      vIdx = i + voxelStart;
      bI = (vIdx % vdX) / bdX;
      bJ = ((vIdx / vdX) % vdY) / bdY;
      bK = ((vIdx / vdX) / vdY) / bdZ;

      if (bI < bcX && bJ < bcY && bK < bcZ) {
        Ty val = a[i];
        // Convert the 3D block index into a 1D block index and fetch the
        // block from the array of blocks.
        bIdx = bI + bcX * (bJ + bK * bcY);
        if (! isRelevant(val))
          m_empties[bIdx] += 1;
      }
    }
  }

  void
  join(ParallelReduceBlockEmpties<Ty> const &rhs)
  {
    for(uint64_t i{ 0 }; i < m_volume->lower().total_block_count(); ++i) {
      m_empties[i] += rhs.m_empties[i];
    }
  }


  uint64_t const *
  empties() const
  {
    return m_empties;
  }

private:
  Ty const * const m_data;
  Volume const * const m_volume;
  size_t const m_voxelStart;
  uint64_t * m_empties;

  std::function<bool(Ty)> isRelevant; //< Is the element a relevant voxel or not.

}; // class ParallelReduceBlockEmpties

} // namespace preproc

#endif // ! bd_parallelblockstats_h

