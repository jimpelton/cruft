#ifndef block_collection_h__
#define block_collection_h__

#include <bd/volume/block.h>
#include <bd/io/indexfile.h>
#include <bd/io/buffer.h>
#include <bd/util/util.h>
#include <bd/io/bufferpool.h>

#include <functional>
#include <list>
#include <vector>
#include <future>

namespace bd
{
class BlockCollection;
class BlockMemoryManager;

class BlockLoader
{
public:

  BlockLoader(BlockMemoryManager *man,
              std::string const & filename,
              size_t maxblocks,
              glm::vec2 const &slabdims);


  ~BlockLoader();


  int
  operator()(std::string const &raw_name);


  void
  pushLoadQueue(Block* b);


  void
  stop();


private:

  Block *
  popLoadQueue();

  void
  fillBlockData(Block *b, std::istream *infile) const;

  char*
  popCPUBuffer(std::list<char*> &);

  Texture*
  popTexture(std::vector<Texture*> &);

private:

  BlockMemoryManager *m_man;
  std::queue<Block*> m_loadQueue;
  std::mutex m_loadQueueLock;
  std::condition_variable_any m_loadQueueWait;

  std::atomic_bool m_stopThread;


}; // class BlockLoader

class BlockMemoryManager
{

public:

  /// \brief Manage memory and block texture cache on GPU.
  BlockMemoryManager() = delete;


  /// \brief Manage memory and block texture cache on GPU.
  /// \param blockBytes - bytes for single block
  /// \param gpuMem - bytes to use on GPU for block textures
  /// \param cpuMem - bytes to use on CPU for block texture cache
  /// \param slabDims - voxel dimensions of a slab in the volume
  /// \param blocks - block depth
  BlockMemoryManager(size_t blockBytes,
                     size_t gpuMem,
                     size_t cpuMem,
                     glm::u64vec3 const &largestBlock,
                     glm::u64vec2 const &slabDims,
                     std::vector<Block*> const &blocks);


  ~BlockMemoryManager();


//  Block *
//  operator[](size_t idx);

  /// \brief Allocated cpu mem and create OpenGL textures.
  void
  init(DataType type,
         glm::u64vec3 const &blockDims,
         std::string const &rawPath);


  void
  evictGpuNonVisible(std::list<Block *> &gpu);


  /// Evict blocks from GPU if space is needed.
  /// Load blocks to GPU.
//  void
//  update(std::vector<Block*> &blocks);


  /// \brief Assign block it's texture data and a vacant texture
  ///        if necessary.
  /// \param blocks - load texture data for these blocks
  /// \param empty_bufs - put the data into these buffers
  /// \param infile - read raw data from this stream
  Block *
  preloadGpuData(Block *b);

  /// \brief submit block for async loading.
  void
  asyncLoadBlock(Block *b);


private:

//  void
//  fillBlockData(Block *b, std::istream *raw) const;


  Texture*
  popTexture();

  /// Get a pointer to cpu memory that can be used for new block data.
  char*
  popCPUBuffer();


  void
  pushCPUBuffer(char *buf);

  /// Grab a block from the non-cpu resident blocks.
  Block*
  popLoadQueue();

//  void
//  pushLoadQueue(Block *b);

  /// Push a block that is loaded to cpu mem and ready for upload to GPU.
  void
  pushGPUReadyBlock(Block *b);

  /// Copy the blocks ready for CPU upload into \c readies
  void
  getGPUReadyBlocks(std::vector<Block *> &readies);

private:

  size_t m_cpuMemBytes;
  size_t m_gpuMemBytes;
  size_t m_maxGpuBlocks;
  size_t m_maxCpuBlocks;
  glm::u64vec2 m_volumeSlabDims;    ///< Dims for a slab in the volume.

  BlockCollection *m_collection;

  /// Blocks resident in gpu memory (visible or non-visible).
  /// Thread access: 
  ///   1. Single thead
//  std::list<Block *> m_gpu;

  /// Available CPU resident texture data buffers. 
  /// Thread access: 
  ///   1. Single thead
//  std::vector<char *> m_cpu;

  /// Textures that can be used for blocks needing to be uploaded.
  /// Thread access: 
  ///   1. Single thead
//  std::vector<Texture *> m_texs;    ///< Textures available on the GPU.
  std::vector<Block *> m_allBlocks; ///< All the blocks!

  char * m_data;  ///< CPU resident texture data.


  /// Visible blocks not in GPU memory, but in cpu memory.
  /// Thread access:
  ///   1. Consumer: some other thread (copies and clears the list)
  ///   2. Producer: Load thread pushes after loading data to cpu mem
  std::vector<Block *> m_readyForGPU;
  std::mutex m_readyForGpuLock;

  /// Visible blocks that need loading from disk.
  /// Thread access: 
  ///   1. Consumer: Load thread
  ///   2. Producer: some other thread
//  std::vector<Block *> m_loadQueue;
  /// Load thread waits on m_loadQueue for new blocks.
//  std::mutex m_loadQueueLock;
//  std::condition_variable_any m_loadQueueWait;

  std::future<int> m_loadThreadFuture;
//  std::atomic_bool m_stopThread;

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


  void
  filterBlocks(std::function<bool(bd::Block const &)> filter);


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

  std::vector<Block *> m_blocks;
  std::vector<Block *> m_nonEmptyBlocks;

  Volume m_volume;

  std::unique_ptr<BlockMemoryManager> m_man;

};


//template <typename Ty>
//bool
//BlockCollection::do_initBlockTextures(std::string const &file)
//{
//  Dbg() << "Initializing block textures";
//
//
//  std::ifstream is(file, std::ios::binary);
//  if (!is.is_open()) {
//    Err() << "Could not open rawfile: " << file << ".";
//    return false;
//  }
//
//
//  if (m_nonEmptyBlocks.size() == 0) {
//    Warn() << "There are no non-empty blocks to initialize textures for.";
//    return false;
//  }
//
//  // init buffers for the raw values and a
//  // texture buffer that contains normalized values.
//  size_t buf_size{ findLargestBlock(m_nonEmptyBlocks) };
//  Ty *buf{ new Ty[buf_size] };
//  float *tex{ new float[buf_size] };
//
//
//  int i{ 0 };
//  double const vol_min = m_volume.min(); //m_indexFile->getHeader().vol_min;
//  double const diff{ m_volume.max() - vol_min };
//
//  // Generate textures for each non-empty block
//  std::cout << std::endl;
//  int every = 0.1f * m_nonEmptyBlocks.size();
//  every = every == 0 ? 1 : every;
//
//  for (Block *b : m_nonEmptyBlocks) {
//
//    if (i % every == 0) {
//      std::cout << "\rInitializing texture block " << ++i
//                << "/" << m_nonEmptyBlocks.size();
//    }
//
//
//    // Read data for this block from the disk.
//    fillBlockData<Ty>(*b, is, buf);
//
//
//    // Normalize the data prior to generating the texture.
//    for (size_t idx{ 0 }; idx < buf_size; ++idx) {
//      tex[idx] = (buf[idx] - vol_min) / diff;
//    }
//
//    //TODO: try not normalizing the texture data and see what happens... :)
//    b->texture().subImage3D(0,0,0,
//                            (int)b->voxel_extent().x,
//                            (int)b->voxel_extent().y,
//                            (int)b->voxel_extent().z,
//                            bd::Texture::Format::RED,
//                            DataType::Float,
//                            tex);
//  } //for
//  std::cout << " ...done." << std::endl;
//
//  delete[] buf;
//  delete[] tex;
//
//  return true;
//}



//template <typename Ty>
//void
//BlockCollection::fillBlockData(Block const &b, std::istream &infile,
//                               Ty *blockBuffer) const
//{
//  // block's dimensions in voxels
//  glm::u64vec3 const &be{ b.voxel_extent() };
//
//  // volume's dimensions in voxels.
//  glm::u64vec2 const &ve{ m_volume.voxelDims().x,
//                          m_volume.voxelDims().y };
//
//  // start element = block index w/in volume * block size
//  const glm::u64vec3 start{ b.ijk() * be };
//
//  // block end element = block voxel start voxelDims + block size
//  const glm::u64vec3 end{ start + be };
//
//  // byte offset into file to read from
//  size_t offset{ b.fileBlock().data_offset };
//
//  // Loop through rows and slabs of volume reading rows of voxels into memory.
//  const size_t blockRowLength{ be.x };
//  for (auto slab = start.z; slab < end.z; ++slab) {
//    for (auto row = start.y; row < end.y; ++row) {
//
//      // seek to start of row
//      infile.seekg(offset, infile.beg);
//
//      // read the bytes of current row
//      infile.read(reinterpret_cast<char *>(blockBuffer), blockRowLength * sizeof(Ty));
//      blockBuffer += blockRowLength;
//
//      // offset of next row
//      offset = bd::to1D(start.x, row + 1, slab, ve.x, ve.y);
//      offset *= sizeof(Ty);
//    }
//  }
//
//
//}

} // namespace bd

#endif // !block_collection_h__
