#include <GL/glew.h>

#include <bd/volume/blockcollection.h>

#include <bd/log/gl_log.h>
#include <bd/log/logger.h>
#include <bd/util/util.h>
#include <bd/io/indexfile.h>

namespace bd
{

BlockMemoryManager::BlockMemoryManager()
{
}


BlockMemoryManager::BlockMemoryManager(size_t blockBytes,
                                       size_t gpuMem,
                                       size_t cpuMem,
                                       glm::u64vec3 const &blkDims,
                                       glm::u64vec2 const &slabDims,
                                       std::vector<Block *> const &blocks)
  : m_maxGpuBlocks{ }
  , m_maxCpuBlocks{ }
  , m_gpu{ }
  , m_cpu{ }
  , m_blockList{ blocks }
{
  if (blockBytes > 0) {

    m_maxGpuBlocks = static_cast<int>(gpuMem / blockBytes);

    m_maxCpuBlocks = static_cast<int>(cpuMem / blockBytes);

  } else {
    Warn() << "Block size of zero given to the memory manager. Only one block will be allocated.";
    m_maxGpuBlocks = 1;
  }



}


BlockMemoryManager::~BlockMemoryManager()
{
}


Block*
BlockMemoryManager::operator[](size_t idx)
{
  assert(false && "not implemented");
}


void
BlockMemoryManager::init(DataType type, glm::u64vec3 const &bd)
{
  m_data = new char[m_maxCpuBlocks * to_sizeType(type)];

  Texture::GenTextures3d(m_maxGpuBlocks, type, Texture::Format::RED,
                         (int)bd.x, (int)bd.y, (int)bd.z, &m_texs);
}


void
BlockMemoryManager::update(std::vector<Block*> &visibleBlocks)
{
  using iterator = std::list<Block*>::iterator;

  // evict blocks from gpu list.
  iterator i = m_gpu.begin();
  while(i != m_gpu.end()) {
    Block *b = *i;
    if (! b->visible()) {
      m_texs.push_back(b->texture());
      b->texture(nullptr);
      i = m_gpu.erase(i);
    } else {
      ++i;
    }
  }

  // load as many visible blocks into GPU as possible.
  for (Block * b : visibleBlocks) {
    if (m_gpu.size() < m_maxGpuBlocks){
      load_data(b);
    } else {
      break;
    }
  }
}


Block *
BlockMemoryManager::load_data(Block *b)
{
  if (b->status() & Block::GPU_RES) {

    return b;

  } else if (b->status() & Block::CPU_RES) {

    Texture *t{ popTexture() };
    b->texture(t);
    m_gpu.push_back(b);
    return b;

  } else {

    // block needs to be read of disk
    Texture *t{ popTexture() };
    char * buf{ popBuffer() };
    fillBlockData(b, buf);
    b->pixelData(buf);
    b->texture(t);

  }
}


void
BlockMemoryManager::fillBlockData(Block *b, char *blockBuffer) const
{
  // block's dimensions in voxels
  glm::u64vec3 const be{ b->voxel_extent() };
  // start element = block index w/in volume * block size
  glm::u64vec3 const start{ b->ijk() * be };
  // block end element = block voxel start voxelDims + block size
  glm::u64vec3 const end{ start + be };

  size_t const blockRowLength{ be.x };
  size_t const sizeType{ to_sizeType(b->texture().dataType()) };

  // byte offset into file to read from
  size_t offset{ b->fileBlock().data_offset };

  // Loop through rows and slabs of volume reading rows of voxels into memory.
  for (auto slab = start.z; slab < end.z; ++slab) {
    for (auto row = start.y; row < end.y; ++row) {

      // seek to start of row
      m_infile->seekg(offset);

      // read the bytes of current row
      m_infile->read(blockBuffer, blockRowLength * sizeType);
      blockBuffer += blockRowLength;

      // offset of next row
      offset = bd::to1D(start.x, row + 1, slab, m_slabDims.x, m_slabDims.y);
      offset *= sizeType;
    }
  }
}


BlockCollection::BlockCollection()
  : BlockCollection{ nullptr }
{
}

BlockCollection::BlockCollection(std::unique_ptr<BlockMemoryManager> man)
    : m_blocks()
    , m_nonEmptyBlocks()
    , m_volume{ }
    , m_man{ std::move(man) }
{
}


///////////////////////////////////////////////////////////////////////////////
BlockCollection::~BlockCollection()
{
}


///////////////////////////////////////////////////////////////////////////////
void
BlockCollection::initBlocksFromIndexFile(std::shared_ptr<IndexFile const> index)
{
  m_volume = index->getVolume();

  initBlocksFromFileBlocks(index->getFileBlocks(),
                           m_volume.worldDims(),
                           m_volume.block_count());

}


///////////////////////////////////////////////////////////////////////////////
void
BlockCollection::initBlocksFromFileBlocks(std::vector<FileBlock> const &fileBlocks,
                                          glm::f32vec3 const &vd,
                                          glm::u64vec3 const &nb)
{

  if (fileBlocks.size() == 0)
  {
    bd::Warn() << "No blocks in list of file blocks to initialize.";
    return;
  }

  m_blocks.reserve(fileBlocks.size());
  m_nonEmptyBlocks.reserve(fileBlocks.size());
  
  int every = 0.1f * fileBlocks.size();
  every = every == 0 ? 1 : every;
  
  auto idx = 0ull;
  for (auto k = 0ull; k < nb.z; ++k) {
    for (auto j = 0ull; j < nb.y; ++j) {
      for (auto i = 0ull; i < nb.x; ++i) {

        if (idx % every == 0) {
          std::cout << "\rCreating block " << idx;
        }
    
        Block *block{ new Block{{ i, j, k }, fileBlocks[idx] }};
        m_blocks.push_back(block);

        idx++;
      }
    }
  }
  std::cout << std::endl;

//  std::sort(m_blocks.begin(), m_blocks.end(),
//            [](Block const *lhs, Block const *rhs) -> bool {
//              return lhs->fileBlock().rov < rhs->fileBlock().rov;
//            });

}


///////////////////////////////////////////////////////////////////////////////
//void
//BlockCollection::filterBlocks(std::function<bool(Block const &)> notEmpty)
//{
//  m_nonEmptyBlocks.clear();
//
//  for (Block *b : m_blocks) {
//    uint64_t idx{ b->fileBlock().block_index };
//
//    if (notEmpty(*b)) {
//      m_nonEmptyBlocks.push_back(b);
//    }
//  }
//}


///////////////////////////////////////////////////////////////////////////////
void
BlockCollection::filterBlocksByROVRange(double rov_min, double rov_max)
{
  //TODO: move filterBlocksByROVRange to mem manager.
  m_nonEmptyBlocks.clear();
  
  size_t bytes{ 0 };

  for (Block *b : m_blocks) {

    double rov = b->fileBlock().rov;

    if (rov >= rov_min && rov <= rov_max) {
//      bytes += b->fileBlock().data_bytes;
      b->visible(true);
      m_nonEmptyBlocks.push_back(b);
    } else {
      b->visible(false);
    }

  } // for
}


///////////////////////////////////////////////////////////////////////////////
void
BlockCollection::updateBlockCache()
{
  m_man->update(m_nonEmptyBlocks);
}


///////////////////////////////////////////////////////////////////////////////
bool
BlockCollection::initBlockTextures(std::string const &file, bd::DataType type)
{
  if (m_blocks.size() == 0)
  {
    bd::Warn() << "No blocks to initialize textures for.";
    return true;
  }

  bool rval = false;
  
  switch (type) {
    case DataType::UnsignedCharacter:
      rval = do_initBlockTextures<unsigned char>(file);
      break;
    case DataType::UnsignedShort:
      rval = do_initBlockTextures<unsigned short>(file);
      break;
    case DataType::Double:
      rval = do_initBlockTextures<double>(file);
      break;
    case DataType::Float:
    default:
      rval = do_initBlockTextures<float>(file);
      break;
  }

  return rval;

}


///////////////////////////////////////////////////////////////////////////////
std::vector<Block *> const &
BlockCollection::blocks() const
{
  return m_blocks;
}


///////////////////////////////////////////////////////////////////////////////
std::vector<Block *> &
BlockCollection::blocks()
{
  return m_blocks;
}


///////////////////////////////////////////////////////////////////////////////
std::vector<Block *> const &
BlockCollection::nonEmptyBlocks() const
{
  return m_nonEmptyBlocks;
}


///////////////////////////////////////////////////////////////////////////////
std::vector<Block *> &
BlockCollection::nonEmptyBlocks()
{
  return m_nonEmptyBlocks;
}


///////////////////////////////////////////////////////////////////////////////
uint64_t
BlockCollection::findLargestBlock(std::vector<Block *> &blocks)
{
  // find the largest element by comparing voxel counts for each nonEmptyBlock.
  auto largest =
      std::max_element(
          blocks.begin(),
          blocks.end(),

          [](Block *lhs, Block *rhs) -> bool {

            // compare number of voxels

            size_t lhs_vox =
                lhs->voxel_extent().x *
                    lhs->voxel_extent().y *
                    lhs->voxel_extent().z;

            size_t rhs_vox =
                rhs->voxel_extent().x *
                    rhs->voxel_extent().y *
                    rhs->voxel_extent().z;

            return lhs_vox < rhs_vox;
          });

  Block *blk{ *largest };
  return blk->voxel_extent().x * blk->voxel_extent().y * blk->voxel_extent().z;

}


//IndexFile const &
//BlockCollection::indexFile() const
//{
//  return *m_indexFile;
//}


///////////////////////////////////////////////////////////////////////////////
//glm::u64vec3
//BlockCollection::blockDims()
//{
//  return m_blockDims;
//}
//
//
/////////////////////////////////////////////////////////////////////////////////
//void
//BlockCollection::blockDims(const glm::u64vec3& voxelDims)
//{
//  m_blockDims = voxelDims;
//}
//
//
/////////////////////////////////////////////////////////////////////////////////
//glm::u64vec3
//BlockCollection::volDims()
//{
//  return m_voxelDims;
//}
//
//
/////////////////////////////////////////////////////////////////////////////////
//void
//BlockCollection::volDims(const glm::u64vec3& voldims)
//{
//  m_voxelDims = voldims;
//}


///////////////////////////////////////////////////////////////////////////////
// nb: number of blocks
// vd: volume voxel dimensions
// blocks: out parameter to be filled with blocks.
//void
//BlockCollection::initBlocks(glm::u64vec3 nb, glm::u64vec3 vd)
//{
//  m_blockDims = vd / nb;
//  m_voxelDims = vd;
//  m_numBlocks = nb;
//
//  // block world voxelDims
//  glm::vec3 wld_dims{ 1.0f / glm::vec3(nb) };
//
//
//  Dbg() << "Starting block init: "
//      "Number of blocks: " <<
//      nb.x << ", " << nb.y << ", " << nb.z <<
//      " Volume dimensions: "
//          <<  vd.x << ", " << vd.y << ", " << vd.z <<
//      " Block dimensions: "
//          << ", " << wld_dims.x << ", " << wld_dims.y << ", " << wld_dims.z;
//
//  // Loop through all our blocks (identified by <bx,by,bz>) and populate block fields.
//  for (auto bz = 0ull; bz < nb.z; ++bz)
//    for (auto by = 0ull; by < nb.y; ++by)
//      for (auto bx = 0ull; bx < nb.x; ++bx) {
//        // i,j,k block identifier
//        glm::u64vec3 blkId{ bx, by, bz };
//        // lower left corner in world coordinates
//        glm::vec3 worldLoc{ (wld_dims * glm::vec3(blkId)) - 0.5f }; // - 0.5f;
//        // origin (centroid) in world coordiates
//        glm::vec3 blk_origin{ (worldLoc + (worldLoc + wld_dims)) * 0.5f };
//
//        Block blk{ glm::u64vec3{bx, by, bz}, , blk_origin };
//        m_blocks.push_back(blk);
//      }
//
//  Dbg() << "Finished block init: total blocks is " << m_blocks.size();
//}



///////////////////////////////////////////////////////////////////////////////
//void BlockCollection::filterBlocks(const float* data, /*unsigned int sampler,*/
//                                   float tmin, float tmax)
//{
//  size_t blkPoints{ bd::vecCompMult(m_blockDims) };
//
//  float* image{ new float[blkPoints] };
//
//  for (auto& b : m_blocks) {
//
//    glm::u64vec3 ijk{ b.ijk() };
//    fillBlockData(ijk, data, image);
//    float avg{ 0.0f };
//    for (size_t i = 0; i < blkPoints; ++i) {
//      avg += image[i];
//    }
//    avg /= blkPoints;
//    b.avg(avg);
//
//    //TODO: call filter function.
//    if (avg < tmin || avg > tmax) {
//      b.empty(true);
//    } else {
//      //      b.texture().samplerLocation(sampler);
//      //      b.texture().textureUnit(0);
//      b.texture().genGLTex3d(image, Texture::Format::RED, Texture::Format::RED,
//         m_blockDims.x, m_blockDims.y, m_blockDims.z);
//
//      if (b.texture().id() == 0) {
//        Err() << "Failed to allocate a gl texture for block ("
//            << ijk.x << ", " << ijk.y << ", " << ijk.z << ").";
//      }
//
//      m_nonEmptyBlocks.push_back(&b);
//    }
//  } // for auto
//
//  delete[] image;
//  Info() << m_blocks.size() - m_nonEmptyBlocks.size() << "/"
//      << m_blocks.size() << " blocks marked empty.";
//}

//void
//BlockCollection::fillBlockData(const glm::u64vec3 &ijk,
//    const glm::u64vec3 &vox_dims,
//    const float* in_data,
//    float* out_blockData)
//{
//  size_t imageIdx{ 0 };
//
//  // block start = block index * block size
//  glm::u64vec3 bst{ ijk * vox_dims };
//
//  // block end = block start + block size
//  glm::u64vec3 end{ bst + vox_dims };
//
//  auto vol_dims_x = m_indexFile->getHeader().volume_extent[0];
//  auto vol_dims_y = m_indexFile->getHeader().volume_extent[1];
//
//  for (auto k = bst.z; k < end.z; ++k)
//    for (auto j = bst.y; j < end.y; ++j)
//      for (auto i = bst.x; i < end.x; ++i) {
//        size_t dataIdx{ bd::to1D(i, j, k, vol_dims_x, vol_dims_y) };
//        float val{ in_data[dataIdx] };
//        out_blockData[imageIdx++] = val;
//      }
//}


} // namespace bd


