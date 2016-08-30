#ifndef parallelblockstats_h__
#define parallelblockstats_h__

#include <bd/io/fileblock.h>
#include <bd/volume/volume.h>
#include <bd/io/buffer.h>


#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

#include <functional>


namespace bd
{

/// \brief Compute the relevance of voxels in the blocked_range.
template<typename Ty>
class ParallelBlockStats{



public:


  ParallelBlockStats(Buffer<Ty> *b, Volume const *v, std::function<bool(Ty)> isEmpty)
    : m_data{ b->ptr() }
    , m_volume{ v }
    , m_voxelStart{ b->index() }
    , m_empties{ nullptr }
    , isRelevant{ isEmpty }
  {
    m_empties = new uint64_t[m_volume->lower().total_block_count()]();
  }

  ParallelBlockStats(ParallelBlockStats<Ty> &o, tbb::split)
      : m_data{ o.m_data }
      , m_volume{ o.m_volume }
      , m_voxelStart{ o.m_voxelStart }
      , m_empties{ nullptr }
      , isRelevant{ o.isRelevant }
  {
    m_empties = new uint64_t[o.m_volume->lower().total_block_count()]();
  }

  ~ParallelBlockStats()
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
      vIdx = i + m_voxelStart;
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
  join(ParallelBlockStats<Ty> const &rhs)
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

}; // class ParallelBlockStats

} // namespace preproc

#endif // ! parallelblockstats_h__

//  FileBlock* copyBlocks(size_t rBegin, size_t rEnd) const
//  {
//
//    uint64_t vIdx, bI, bJ, bK, bIdx;
//    vIdx =  rBegin + voxel_start;
//    bI = (vIdx % vdX) / bdX;
//    bJ = ((vIdx / vdX) % vdY) / bdY;
//    bK = ((vIdx / vdX) / vdY) / bdZ;
//    uint64_t startBlock = bI + bcX * (bJ + bK * bcY);
//
//    // end block
//    vIdx =  rEnd + voxel_start;
//    bI = (vIdx % vdX) / bdX;
//    bJ = ((vIdx / vdX) % vdY) / bdY;
//    bK = ((vIdx / vdX) / vdY) / bdZ;
//    uint64_t endBlock = bI + bcX * (bJ + bK * bcY);
//
//    // Copy the blocks covered by the blocked_range r.
//    size_t sz{ endBlock - startBlock };
//    FileBlock *my_blocks = new FileBlock[sz];
//    for(size_t i{ 0 }; i < sz; ++i) {
//      my_blocks[i] = *m_blocks[startBlock];
//      startBlock++;
//    }
//
//    return my_blocks;
//  }


//  FileBlock* copyBlocks(const FileBlock * const *theBlocks, size_t numBlocks) {
//    m_myBlocks = new FileBlock[numBlocks];
//    for(size_t i{ 0 }; i<numBlocks; ++i)
//      m_myBlocks[i] = *theBlocks[i];
//
//    return m_myBlocks;
//  }
