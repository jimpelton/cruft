#ifndef blockcollection2_h__
#define blockcollection2_h__

#include <bd/io/fileblock.h>
#include <bd/io/bufferedreader.h>
#include <bd/io/buffer.h>
#include <bd/log/logger.h>
#include <bd/util/util.h>
#include <bd/tbb/parallelminmax.h>
#include <bd/tbb/parallelblockstats.h>
#include <bd/tbb/parallelblockminmax.h>
#include <bd/volume/volume.h>
#include <bd/volume/valuerangefilter.h>

#include <tbb/parallel_reduce.h>
#include <tbb/blocked_range.h>

#include <glm/glm.hpp>

#include <numeric>
#include <functional>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cassert>
#include <iterator>


namespace bd
{

//////////////////////////////////////////////////////////////////////////////
/// \brief Creates a list of blocks from a large binary raw file. The data type
///        in the binary raw file is supplied by the template parameter.
///
/// \param Ty Data type in the istream this BlockCollection will be
///            generated from.
//////////////////////////////////////////////////////////////////////////////
template<typename Ty>
class FileBlockCollection
{

public:

  FileBlockCollection();


  FileBlockCollection(glm::u64vec3 volDims, glm::u64vec3 numBlocks);


  ~FileBlockCollection();


  //////////////////////////////////////////////////////////////////////////////
  /// \brief Marks blocks as empty and uploads GL textures if average is outside of [tmin..tmax].
  /// \param rawFile[in] Volume data set
  /// \param tmin[in] min average block value to filter against.
  /// \param tmax[in] max average block value to filter against.
  ///////////////////////////////////////////////////////////////////////////////
  void createFromRawFile(const std::string &file, size_t bufSize, std::function<bool(Ty)>);


  //////////////////////////////////////////////////////////////////////////////
  /// \brief Read a single block into buffer \c out.
  /// \param b[in]      The FileBlock for the block that will be read.
  /// \param index[in]  i,j,k coords of the block whos data to get.
  /// \param infile[in] The raw data file.
  /// \param out[out]   Destination space for block data.
  //////////////////////////////////////////////////////////////////////////////
//void fillBlockData(const FileBlock &b, std::istream &infile, Ty *out) const;


  //////////////////////////////////////////////////////////////////////////////
  /// \brief Add a reference to the pre-initialized block to this FileBlockCollection.
  /// \note  Adds block to non-empty list if block is not empty.
  /// \param b The block to add.
  //////////////////////////////////////////////////////////////////////////////
  void addBlock(const FileBlock& b);


  const Volume& volume()
  {
    return m_volume;
  }


  //////////////////////////////////////////////////////////////////////////////
  const std::vector<FileBlock*>& blocks() const
  {
    return m_blocks;
  }


  //////////////////////////////////////////////////////////////////////////////
  const std::vector<FileBlock*>& nonEmptyBlocks() const
  {
    return m_nonEmptyBlocks;
  }


private:

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Initializes \c nb blocks so that they fit within the extent of \c vd.
  void initBlocks();


  //////////////////////////////////////////////////////////////////////////////
  /// \brief Determine if blocks fit evenly within volume, else add blocks in
  ///        appropriate dimensions and update the volume with new block counts.
  /// \note Does not update the voxel dimensions of the volume.
  /// \return The updated block counts.
  glm::u64vec3 updateBlockCount();


  //////////////////////////////////////////////////////////////////////////////
  /// \brief Sum the given buffer in parallel.
  double doBufferSum(Buffer<Ty>*);


  //////////////////////////////////////////////////////////////////////////////
  /// \brief Find buffer global min/max.
  void doBufferMinMax(Buffer<Ty>*);


  //////////////////////////////////////////////////////////////////////////////
  /// \brief Find block local min/max and total values.
  void doBlockMinMax(Buffer<Ty>*);


  //////////////////////////////////////////////////////////////////////////////
  /// \brief Count the relevant voxels for each block in the given buffer.
  void doBlockVoxelRelavance(Buffer<Ty>*, std::function<bool(Ty)> const &);

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Compute the averages for each block after min/max and sum is found.
  void finishBlockAverages();


  //////////////////////////////////////////////////////////////////////////////
  /// \brief Compute and save a few stats from provided raw file.
  void computeVolumeStatistics(BufferedReader<Ty> & r, std::function<bool(Ty)> const &);


  //////////////////////////////////////////////////////////////////////////////
  /// \brief Compute and save a few stats for each block.
  //////////////////////////////////////////////////////////////////////////////
//  void computeBlockStatistics(BufferedReader<Ty> &r);


private: //members  

  Volume m_volume;

  std::vector<FileBlock*> m_blocks;
  std::vector<FileBlock*> m_nonEmptyBlocks;


}; // class FileBlockCollection


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
FileBlockCollection<Ty>::FileBlockCollection()
    : FileBlockCollection({ 0, 0, 0 }, { 0, 0, 0 })
{
}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
FileBlockCollection<Ty>::FileBlockCollection(glm::u64vec3 volDims, glm::u64vec3 numBlocks)
    : m_volume{ volDims, numBlocks }
    , m_blocks{ }
    , m_nonEmptyBlocks{ }
{
}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
FileBlockCollection<Ty>::~FileBlockCollection()
{
  std::cout << "FileBlockCollection destructor\n";
  for(auto b : m_blocks) {
    delete b;
  }
}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
glm::u64vec3
FileBlockCollection<Ty>::updateBlockCount()
{
  glm::u64vec3 bc{ volume().lower().block_count() };
  glm::u64vec3 vd{ volume().dims() };

  const glm::u64vec3 rem{ vd%bc };
  if (rem.x>0) {
    bc.x += 1;
    Info() << "Increased x-axis block count by 1.";
  }
  if (rem.y>0) {
    bc.y += 1;
    Info() << "Increased y-axis block count by 1.";
  }
  if (rem.z>0) {
    bc.z += 1;
    Info() << "Increased z-axis block count by 1.";
  }

  // save the new block count
  m_volume.lower().block_count(bc);

  return bc;
}


///////////////////////////////////////////////////////////////////////////////
// bc: number of blocks
// vd: volume voxel dimensions
// bd: block dimensions
template<typename Ty>
void
FileBlockCollection<Ty>::initBlocks()
{
  //glm::u64vec3 bc{ updateBlockCount() };
  glm::u64vec3 bc{ m_volume.lower().block_count() };
  glm::u64vec3 vd{ m_volume.dims() };
  glm::u64vec3 bd{ m_volume.lower().block_dims() };

  // block world dims
  glm::vec3 wld_dims{ 1.0f/glm::vec3(bc) };

  Info() << "Starting block init: "
      " # blocks: "
      << bc.x << ", " << bc.y << ", " << bc.z <<
      " Vol dim: "
          << vd.x << ", " << vd.y << ", " << vd.z <<
      " Block dim: "
          << ", " << wld_dims.x << ", " << wld_dims.y << ", " << wld_dims.z;


  // Loop through all our blocks (identified by <bxi,byj,bzk>) and populate block fields.
  for (auto bzk = 0ull; bzk<bc.z; ++bzk)
    for (auto byj = 0ull; byj<bc.y; ++byj)
      for (auto bxi = 0ull; bxi<bc.x; ++bxi) {
        // i,j,k block identifier
        const glm::u64vec3 blkId{ bxi, byj, bzk };
        // lower left corner in world coordinates
        const glm::vec3 worldLoc{ wld_dims*glm::vec3(blkId)-0.5f }; // - 0.5f;
        // origin (centroid) in world coordiates
        const glm::vec3 blkOrigin{ (worldLoc+(worldLoc+wld_dims))*0.5f };
        // voxel start of block within volume
        const glm::u64vec3 startVoxel{ blkId*m_volume.lower().block_dims() };

        FileBlock* blk = new FileBlock(); // { std::make_shared<FileBlock>() };
        blk->block_index = bd::to1D(bxi, byj, bzk, bc.x, bc.y);
        blk->data_offset = bd::to1D(startVoxel.x, startVoxel.y, startVoxel.z, vd.x, vd.y);
        blk->data_offset *= sizeof(Ty);

        blk->voxel_dims[0] = static_cast< decltype(blk->voxel_dims[0]) >(bd.x);
        blk->voxel_dims[1] = static_cast< decltype(blk->voxel_dims[1]) >(bd.y);
        blk->voxel_dims[2] = static_cast< decltype(blk->voxel_dims[2]) >(bd.z);

        blk->world_oigin[0] = blkOrigin.x;
        blk->world_oigin[1] = blkOrigin.y;
        blk->world_oigin[2] = blkOrigin.z;

        m_blocks.push_back(blk);
      }

  Info() << "Finished block init: total blocks is " << m_blocks.size();
}


//////////////////////////////////////////////////////////////////////////////
template<typename Ty>
double
FileBlockCollection<Ty>::doBufferSum(Buffer<Ty>* buf)
{
  Ty* p = buf->ptr();

  Info() << "CO: Summing buffer\n";
  double volsum{ tbb::parallel_reduce(
      tbb::blocked_range<Ty*>(p, p+buf->elements()),
      0.0,
      [ ](tbb::blocked_range<Ty*>& br, double partial_sum) -> double {
        return std::accumulate(br.begin(), br.end(), partial_sum);
      },
      std::plus<double>()
  ) };

  m_volume.total( m_volume.total() + volsum );

  return volsum;
}


//////////////////////////////////////////////////////////////////////////////
template<typename Ty>
void
FileBlockCollection<Ty>::doBufferMinMax(Buffer<Ty>* buf)
{
  tbb::blocked_range<size_t> range(0, buf->elements());
  ParallelMinMax<Ty> mm(buf);
  tbb::parallel_reduce(range, mm);

  double vol_min{ m_volume.min() };
  double vol_max{ m_volume.max() };

  if (mm.min_value < vol_min) {
    m_volume.min(mm.min_value);
  }

  if (mm.max_value > vol_max) {
    m_volume.max(mm.max_value);
  }
}


//////////////////////////////////////////////////////////////////////////////
template<typename Ty>
void
FileBlockCollection<Ty>::doBlockMinMax(Buffer<Ty> *buf)
{
  ParallelBlockMinMax<Ty> blockMinMax{ &m_volume, buf };

  tbb::blocked_range<size_t> range{ 0, buf->elements() };
  tbb::parallel_reduce(range, blockMinMax);

  // update the blocks with this buffers min max data.
  MinMaxPairDouble const * pairs{ blockMinMax.pairs() };
  for(uint64_t i{ 0 }; i < m_volume.lower().total_block_count(); ++i) {
    FileBlock * b{ m_blocks[i] };
    if (b->min_val > pairs[i].min) b->min_val = pairs[i].min;
    if (b->max_val < pairs[i].max) b->max_val = pairs[i].max;
    b->total_val += pairs[i].total;
  }

}

///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
void
FileBlockCollection<Ty>::doBlockVoxelRelavance(Buffer<Ty> *buf,
    std::function<bool(Ty)> const &f)
{
  ParallelBlockStats<Ty> stats{ buf, &m_volume, f };

  tbb::blocked_range<size_t> range{ 0, buf->elements() };
  tbb::parallel_reduce(range, stats);
  uint64_t const *empties{ stats.empties() };
  for(uint64_t i{ 0 }; i < m_volume.lower().total_block_count(); ++i) {
    FileBlock * b{ m_blocks[i] };
    b->empty_voxels += empties[i];
  }

}

//////////////////////////////////////////////////////////////////////////////
template<typename Ty>
void
FileBlockCollection<Ty>::finishBlockAverages()
{
//  uint64_t volume_empty_voxels{ 0 };
  for (FileBlock* b : m_blocks) {
    uint64_t total_vox{ b->voxel_dims[0]*b->voxel_dims[1]*b->voxel_dims[2] };
    b->avg_val = b->total_val / total_vox;
//    volume_empty_voxels += b->empty_voxels;
  }

//  m_volume.emptyVoxels(volume_empty_voxels);
}


//////////////////////////////////////////////////////////////////////////////
template<typename Ty>
void
FileBlockCollection<Ty>::computeVolumeStatistics(BufferedReader<Ty>& r,
    std::function<bool(Ty)> const & isRelevant)
{
  Info() << "Computing volume statistics...";

  size_t total_bytes_processed{ 0 };
//  Ty vol_min{ std::numeric_limits<Ty>::max() };
//  Ty vol_max{ std::numeric_limits<Ty>::lowest() };


  while (r.hasNext()) {
    Buffer<Ty>* buf = r.waitNext();
    Dbg() << "CO: Got buffer of " << buf->elements() << " elements.";

    Dbg() << "CO: Computing volume stats for this buffer.";
    // Sum values in this buffer
    doBufferSum(buf);
    // Compute the min/max values of the volume and a bunch of work on each block.
    doBufferMinMax(buf);

    Dbg() << "CO: Computing block stats for this buffer.";
    doBlockMinMax(buf);
    doBlockVoxelRelavance(buf, isRelevant);


    Dbg() << "CO: Returning empty buffer.";
    r.waitReturn(buf);


//    Dbg() << "bufMin: " << double(mm.min_value) << " bufMax: " << double(mm.max_value)
//           << " volMin: " << double(vol_min) << " volMax: " << double(vol_max);

    total_bytes_processed += buf->elements()*sizeof(Ty);

  } // while(...

  // Save final volume min/max/avg.
  m_volume.avg( m_volume.total() /
                    (m_volume.dims().x * m_volume.dims().y * m_volume.dims().z) );

  // Average the blocks!
  finishBlockAverages();

  Info() << "Done computing volume statistics "
      << m_volume.min() << ", " << m_volume.max() << ", " << m_volume.avg()
      << "\n Total bytes processed: " << total_bytes_processed;

}


//////////////////////////////////////////////////////////////////////////////
template<typename Ty>
void
FileBlockCollection<Ty>::addBlock(const FileBlock& b)
{
  FileBlock* ptr{ new FileBlock(b) }; //{ std::make_shared<FileBlock>(b) };
  m_blocks.push_back(ptr);

  if (!b.is_empty) {
    m_nonEmptyBlocks.push_back(ptr);
  }

}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
void
FileBlockCollection<Ty>::createFromRawFile
(
  const std::string &file,
  size_t bufSize,
  std::function<bool(Ty)> isVoxelEmpty
)
{
  BufferedReader<Ty> r{ bufSize };
  if (!r.open(file)) {
    throw std::runtime_error("Could not open file "+file);
  }

  r.start();

  initBlocks();

  computeVolumeStatistics(r, isVoxelEmpty);


  Info() << m_blocks.size()-m_nonEmptyBlocks.size() << "/" << m_blocks.size() <<
      " blocks marked empty.";
}


///////////////////////////////////////////////////////////////////////////////
//template<typename Ty>
//void
//FileBlockCollection<Ty>::fillBlockData
//(
//    const FileBlock& b,
//    std::istream& infile,
//    Ty* blockBuffer
//) const
//{
//  const glm::u64vec3 &nb{ m_volume.lower().block_count() };
//  const glm::u64vec3 &bd{ m_volume.lower().block_dims() };
//  const glm::u64vec3 &vd{ m_volume.dims() };
//  // Convert 1D block index to 3D i,j,k indices.
//  glm::u64vec3 index{
//      b.block_index%nb.x,
//      (b.block_index/nb.x)%nb.y,
//      (b.block_index/nb.x)/nb.y
//  };
//
//  // start element = block index w/in volume * block size
//  const glm::u64vec3 start{ index*bd};
//  // block end element = block voxel start dims + block size
//  const glm::u64vec3 end{ start+bd};
//
//  size_t offset{ b.data_offset };
//
//  // Loop through rows and slabs of volume reading rows of voxels into memory.
//  const size_t blockRowLength{ bd.x };
//  for (auto slab = start.z; slab<end.z; ++slab) {
//    for (auto row = start.y; row<end.y; ++row) {
//
//      // seek to start of row
//      infile.seekg(offset, infile.beg);
//
//      // read the bytes of current row
//      infile.read(reinterpret_cast<char*>(blockBuffer), blockRowLength*sizeof(Ty));
//      blockBuffer += blockRowLength;
//
//      // offset of next row
//      offset = bd::to1D(start.x, row+1, slab, vd.x, vd.y);
//      offset *= sizeof(Ty);
//    }
//  }
//}



////////////////////////////////////////////////////////////////////////////////
//template<typename Ty>
//void
//FileBlockCollection<Ty>::computeBlockStatistics(BufferedReader<Ty> &r)
//{
//  Info() << "Computing block statistics for " << m_blocks.size() << " blocks.";
//
//  while(r.hasNext()) {
//    Buffer<Ty> *buf = r.waitNext();
//    Info() << "CO: Got buffer of " << buf->elements() << " elements.";
//    Ty *p = buf->ptr();
//
//  }
//  const Ty *buf{ r.buffer_ptr() };
//
//  // voxel index within the entire volume
//  size_t vol_idx = 0;
//
//  // the 1D block index of current block
//  size_t blk_idx = 0;
//  FileBlock *currBlock{ m_blocks[blk_idx] };
//
//
////  uint64_t dist_to_right_block_edge = currBlock->voxel_dims[0];
////  uint64_t dist_to_bottom_block_edge = currBlock->voxel_dims[0];
////  uint64_t block_edge = 0 + dist_to_right_block_edge;
//
//  while (r.hasNextFill()) {
//
//    // the number of voxels read last fill.
//    size_t voxels_read = r.fillBuffer();
//
//    if (voxels_read<=0) {
//      std::cout << "\nreadsz: " << voxels_read << " bytes." << std::endl;
//      break;
//    }
//
//    // loop through buffer
//    currBlock = m_blocks[blk_idx];
//    uint64_t voxelsInBlockRow{ currBlock->voxel_dims[0] };
//    size_t vox{ 0 };
//
//    for (; vox < voxelsInBlockRow; ++vox) {
//      Ty val{ buf[vox] };
//
//      currBlock->min_val = std::min<
//          decltype(currBlock->min_val)>(currBlock->min_val, val);
//
//      currBlock->max_val = std::max<
//          decltype(currBlock->max_val)>(currBlock->max_val, val);
//
//      currBlock->avg_val += vox;
//
//    }
//
//    //TODO: compute next block index
//
//    vol_idx += vox;
//
//  } // while(r.hasNext...


// Determine which block this voxel falls into:         //
// 1st compute 3D block index from 1D voxel index,      //
// then compute 1D block index from 3D block index      //
//    auto vX = (vol_idx%m_volDims.x);
//    auto vY = ((vol_idx/m_volDims.x)%m_volDims.y);
//    auto vZ = ((vol_idx/m_volDims.x)/m_volDims.y);
//    size_t blockIdx{
//        bd::to1D(
//            vX/m_blockDims.x,
//            vY/m_blockDims.y,
//            vZ/m_blockDims.z,
//            m_numBlocks.x,
//            m_numBlocks.y) };

//    for (size_t buf_idx{ 0 }; buf_idx < elems; ++buf_idx, ++vol_idx) {
//      Ty voxel =  buf[buf_idx];
//
//
////      blockIndexFile << xi << "," << yi << ","  << zi << "," << blockIdx << "\n";
//
//      try {
//        // Populate block that this voxel falls into with stats values.
//        FileBlock *b{ m_blocks.at(blockIdx) };
//
//        b->min_val = std::min<decltype(b->min_val)>(b->min_val, voxel);
//        b->max_val = std::max<decltype(b->max_val)>(b->max_val, voxel);
//        b->avg_val += voxel;
//
//      } catch (std::out_of_range e) {
//        std::cerr << e.what() << std::endl;
////        blockIndexFile.flush();
////        blockIndexFile.close();
//        return;
//      }
//    } // for(...
//  } // while(

//  for(FileBlock* b : m_blocks) {
//    b->avg_val /= b->voxel_dims[0] * b->voxel_dims[1] * b->voxel_dims[2];
//  }
//
//  //std::cout << "\nDone computing block statistics.\n";
//  Info() << "Done computing block statistics for " << m_blocks.size() << " blocks.";
//}

} // namespace bd

#endif // !blockcollection2_h__


