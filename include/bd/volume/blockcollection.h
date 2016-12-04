#ifndef block_collection_h__
#define block_collection_h__

#include <bd/volume/block.h>
#include <bd/io/indexfile.h>
#include <bd/io/buffer.h>
#include <bd/util/util.h>

#include <functional>
#include <list>
#include <future>

namespace bd
{

class BlockMemoryManager
{

public:

  /// \brief Manage memory and block texture cache on GPU.
  BlockMemoryManager();


  /// \brief Manage memory and block texture cache on GPU.
  /// \param blockSize - bytes for single block
  /// \param gpuMem - bytes to use on GPU for block textures
  /// \param cpuMem - bytes to use on CPU for block texture cache
  /// \param bW - block width
  /// \param bH - block height
  /// \param bD - block depth
  BlockMemoryManager(size_t blockSize, size_t gpuMem, size_t cpuMem,
                     int bW, int bH, int bD, std::vector<Block*> const &blocks);


  ~BlockMemoryManager();


  Block *
  operator[](size_t idx);


  void
  init(DataType type, int bW, int bH, int bD);


  /// Evict blocks from GPU if space is needed.
  /// Load blocks to GPU.
  void
  update(std::vector<Block*> &blocks);


private:


  void
  loadToGpu(Block const *b);


  int m_maxGpuBlocks;
  int m_maxCpuBlocks;

  std::list<Block *> m_gpu;
  std::list<Block *> m_cpu;
  std::vector<Texture> m_texs;  // Textures available on the GPU.

  std::vector<Block *> m_blockList;

  char * m_data;

};

class BlockCollection
{
public:
  BlockCollection();

  BlockCollection(std::unique_ptr<BlockMemoryManager> man);

  ~BlockCollection();


  BlockCollection(BlockCollection const &) = delete;


  BlockCollection(BlockCollection const &&) = delete;


  void
  initBlocksFromIndexFile(std::shared_ptr<IndexFile const> index);


  /// \brief Initializes \c blocks from the provided vector of FileBlock.
  /// \note Blocks are sized to fit it within the world-extent of the volume data.
  /// \param fileBlocks[in] The FileBlocks generated from the IndexFile.
  /// \param vd[in]  The voxel dimensions of the volume data
  /// \param numblocks[in]  The number of blocks to generate in each dimension.
  void
  initBlocksFromFileBlocks(std::vector<bd::FileBlock> const &fileBlocks,
                           glm::f32vec3 const &vd,
                           glm::u64vec3 const &numblocks);


//  void
//  filterBlocks(std::function<bool(bd::Block const &)> filter);


  void
  filterBlocksByROVRange(double rov_min, double rov_max);

  void
  updateBlockCache();

  bool
  initBlockTextures(std::string const &rawFile, bd::DataType type);


  std::vector<Block *> const&
  blocks() const;


  std::vector<Block *>&
  blocks();


  std::vector<Block *> const&
  nonEmptyBlocks() const;


  std::vector<Block *>&
  nonEmptyBlocks();


  /// Find the largest non-empty block and return the number of voxels.
  /// \return size_t that is the number of voxels in the largest block.
  uint64_t
  findLargestBlock(std::vector<Block *> &blocks);


private:

  /// \brief initialize block texture from the raw data at \c file.
  template <class Ty>
  bool
  do_initBlockTextures(std::string const &file);


  /// \brief Fills \c blockBuffer with the data from \c infile corresponding to block (i,j,k).
  /// \param b[in] The block for which data should be read.
  /// \param infile[in] Source data
  /// \param blockBuffer[out] Destination space for data.
  template <class Ty>
  void
  fillBlockData(Block const &b, std::istream &infile, Ty *blockBuffer) const;

  std::vector<Block *> m_blocks;
  std::vector<Block *> m_nonEmptyBlocks;

  Volume m_volume;

  std::unique_ptr<BlockMemoryManager> m_man;

};


template <typename Ty>
bool
BlockCollection::do_initBlockTextures(std::string const &file)
{
  Dbg() << "Initializing block textures";


  std::ifstream is(file, std::ios::binary);
  if (!is.is_open()) {
    Err() << "Could not open rawfile: " << file << ".";
    return false;
  }


  if (m_nonEmptyBlocks.size() == 0) {
    Warn() << "There are no non-empty blocks to initialize textures for.";
    return false;
  }

  // init buffers for the raw values and a
  // texture buffer that contains normalized values.
  size_t buf_size{ findLargestBlock(m_nonEmptyBlocks) };
  Ty *buf{ new Ty[buf_size] };
  float *tex{ new float[buf_size] };


  int i{ 0 };
  double const vol_min = m_volume.min(); //m_indexFile->getHeader().vol_min;
  double const diff{ m_volume.max() - vol_min };

  // Generate textures for each non-empty block
  std::cout << std::endl;
  int every = 0.1f * m_nonEmptyBlocks.size();
  every = every == 0 ? 1 : every;

  for (Block *b : m_nonEmptyBlocks) {

    if (i % every == 0) {
      std::cout << "\rInitializing texture block " << ++i
                << "/" << m_nonEmptyBlocks.size();
    }


    // Read data for this block from the disk.
    fillBlockData<Ty>(*b, is, buf);


    // Normalize the data prior to generating the texture.
    for (size_t idx{ 0 }; idx < buf_size; ++idx) {
      tex[idx] = (buf[idx] - vol_min) / diff;
    }

    //TODO: try not normalizing the texture data and see what happens... :)
    b->texture().subImage3D(bd::Texture::Format::RED,
                            0,0,0,
                            b->voxel_extent().x,
                            b->voxel_extent().y,
                            b->voxel_extent().z,
                            DataType::Float,
                            tex);
  } //for
  std::cout << " ...done." << std::endl;

  delete[] buf;
  delete[] tex;

  return true;
}

namespace
{
///
/// \tparam Ty
/// \param blocks - load texture data for these blocks
/// \param empty_bufs - put the data into these buffers
/// \param infile - read raw data from this stream
  template<class Ty>
  void
  async_load_data(std::vector<Block*> &blocks,
                  std::vector<Buffer<Ty> *> &empty_bufs,
                  std::istream &infile)
  {
    for (Block b : blocks) {
      infile.seekg(b.fileBlock().data_offset);
    }



//    std::async(std::launch::async, )
  }
}

template <typename Ty>
void
BlockCollection::fillBlockData(Block const &b, std::istream &infile,
                               Ty *blockBuffer) const
{
  // block's dimensions in voxels
  glm::u64vec3 const &be{ b.voxel_extent() };

  // volume's dimensions in voxels.
  glm::u64vec2 const &ve{ m_volume.voxelDims().x,
                          m_volume.voxelDims().y };

  // start element = block index w/in volume * block size
  const glm::u64vec3 start{ b.ijk() * be };

  // block end element = block voxel start voxelDims + block size
  const glm::u64vec3 end{ start + be };

  // byte offset into file to read from
  size_t offset{ b.fileBlock().data_offset };

  // Loop through rows and slabs of volume reading rows of voxels into memory.
  const size_t blockRowLength{ be.x };
  for (auto slab = start.z; slab < end.z; ++slab) {
    for (auto row = start.y; row < end.y; ++row) {

      // seek to start of row
      infile.seekg(offset, infile.beg);

      // read the bytes of current row
      infile.read(reinterpret_cast<char *>(blockBuffer), blockRowLength * sizeof(Ty));
      blockBuffer += blockRowLength;

      // offset of next row
      offset = bd::to1D(start.x, row + 1, slab, ve.x, ve.y);
      offset *= sizeof(Ty);
    }
  }


}

} // namespace bd

#endif // !block_collection_h__
