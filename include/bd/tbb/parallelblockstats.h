#ifndef blockaverage_h__
#define blockaverage_h__

#include <bd/io/fileblock.h>
#include <bd/volume/volume.h>
#include <bd/io/buffer.h>


#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

#include <functional>


namespace bd
{

//////////////////////////////////////////////////////////////////////////
/// \brief Use Intel TBB to compute the average voxel value within
///        each block.
//////////////////////////////////////////////////////////////////////////
template<typename Ty>
class ParallelBlockStats{

private:
  const Ty * const data;
  const FileBlock * const * m_blocks;


  const uint64_t vdX, vdY;         // volume dims along X, Y axis
  const uint64_t bdX, bdY, bdZ;    // block dims along X, Y, Z axis
  const uint64_t bcX, bcY, bcZ;    // block count along X, Y, Z axis
  const uint64_t voxel_start;      // global voxel index this buffer starts at.

  std::function<bool(Ty)> isRelevant; //< Is the element a relevant voxel or not.

  FileBlock *m_myBlocks;

public:
  const FileBlock* blocks() { return m_myBlocks; }

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


  FileBlock* copyBlocks(const FileBlock * const *theBlocks, size_t numBlocks) {
    m_myBlocks = new FileBlock[numBlocks];
    for(size_t i{ 0 }; i<numBlocks; ++i)
      m_myBlocks[i] = *theBlocks[i];

    return m_myBlocks;
  }


  void operator()(const tbb::blocked_range<size_t> &r)
  {
    const Ty * const a{ data };
    FileBlock * const my_blocks{ copyBlocks(m_blocks, bcX*bcY*bcZ) };

    uint64_t vIdx, bI, bJ, bK, bIdx;
    for (size_t i{ r.begin() }; i != r.end(); ++i) {

      // Convert vIdx (the voxel index within the entire volume data set)
      // into a 3D index, then convert the 3D voxel index obtained from
      // vIdx into a 3D block index to be used in determining if our voxel
      // is within the part of the volume covered by our blocks.
      vIdx = i + voxel_start;
      bI = (vIdx % vdX) / bdX;
      bJ = ((vIdx / vdX) % vdY) / bdY;
      bK = ((vIdx / vdX) / vdY) / bdZ;

      if (bI < bcX && bJ < bcY && bK < bcZ) {
        Ty val = a[i];

        // Convert the 3D block index into a 1D block index and fetch the
        // block from the array of blocks.
        bIdx = bI + bcX * (bJ + bK * bcY);
        FileBlock *b = &my_blocks[bIdx];

        if (! isRelevant(val))
          b->empty_voxels += 1;

        //TODO: Make writes to FileBlock data thread safe (need thread-local storage).

        // Accumulate block-specific values.
        if (val < b->min_val) { b->min_val = val; }
        if (val > b->max_val) { b->max_val = val; }
        b->total_val += val;
      }
    }

    m_myBlocks = my_blocks;
  }

  void join(const ParallelBlockStats<Ty> &y)
  {

  }

  ParallelBlockStats(ParallelBlockStats<Ty> &o, tbb::split)
    : data{ o.data }
    , m_blocks{ o.m_blocks }
//    , m_myBlocks{ o.m_myBlocks }
    , vdX{ o.vdX }
    , vdY{ o.vdY }
    , bdX{ o.bdX }
    , bdY{ o.bdY }
    , bdZ{ o.bdZ }
    , bcX{ o.bcX }
    , bcY{ o.bcY }
    , bcZ{ o.bcZ }
    , voxel_start{ o.voxel_start }
    , isRelevant{ o.isRelevant }
    {

    }


//  ParallelBlockStats(const ParallelBlockStats<Ty> &o)
//    : data{ o.data }
//    , m_blocks{ o.m_blocks }
//    , m_myBlocks{ o.m_myBlocks }
//    , vdX{ o.vdX }
//    , vdY{ o.vdY }
//    , bdX{ o.bdX }
//    , bdY{ o.bdY }
//    , bdZ{ o.bdZ }
//    , bcX{ o.bcX }
//    , bcY{ o.bcY }
//    , bcZ{ o.bcZ }
//    , voxel_start{ o.voxel_start }
//    , isRelevant{ o.isRelevant }
//  { }

  ParallelBlockStats(Buffer<Ty> *b, const Volume *v, FileBlock * const * blocks,
      std::function<bool(Ty)> isEmpty)
    : data{ b->ptr() }
    , m_blocks{ blocks }
    , m_myBlocks{ nullptr }
    , vdX{ v->dims().x }
    , vdY{ v->dims().y }
    , bdX{ v->lower().block_dims().x }
    , bdY{ v->lower().block_dims().y }
    , bdZ{ v->lower().block_dims().z }
    , bcX{ v->lower().block_count().x }
    , bcY{ v->lower().block_count().y }
    , bcZ{ v->lower().block_count().z }
    , voxel_start{ b->index() }
    , isRelevant{ isEmpty }
  { }

  ~ParallelBlockStats() { }

}; // class ParallelBlockStats

} // namespace preproc

#endif // ! blockaverage_h__

