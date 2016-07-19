#include <bd/volume/blockcollection.h>

#include <bd/log/gl_log.h>
#include <bd/log/logger.h>
#include <bd/util/util.h>
#include <bd/io/indexfile.h>

namespace bd
{
//glm::u64vec3 BlockCollection::m_blockDims{ 0, 0, 0 };
//glm::u64vec3 BlockCollection::m_volDims{ 0, 0, 0 };
//glm::u64vec3 BlockCollection::m_numBlocks{ 0, 0, 0 };

BlockCollection::BlockCollection()
{
}


BlockCollection::~BlockCollection()
{
}


void
BlockCollection::initBlocksFromIndexFile(std::string const & fileName)
{
  m_indexFile = bd::IndexFile::fromBinaryIndexFile(fileName);
  bd::IndexFileHeader const &header = m_indexFile->getHeader();

  Dbg() << "Initializing blocks from index file: " << fileName;
  initBlocksFromFileBlocks(m_indexFile->blocks(),
                           { header.numblocks[0],
                             header.numblocks[1],
                             header.numblocks[2] });

//  Dbg() << "Initializing non-empty block textures";
//  initBlockTextures(fileName);
}


void
BlockCollection::initBlocksFromFileBlocks
(
  std::vector< FileBlock * > const fileBlocks,
  glm::u64vec3 nb
)
{
  auto idx = 0ull;
  for (auto k = 0ull; k < nb.z; ++k)
    for (auto j = 0ull; j < nb.y; ++j)
      for (auto i = 0ull; i < nb.x; ++i) {
        std::cout << "\rCreating block " << idx;
        FileBlock *file_block{ fileBlocks[idx] };
        Block *block{ new Block{{ i, j, k },
                                { 1.0f / nb.x, 1.0f / nb.y, 1.0f / nb.z },
                                *file_block }};

        m_blocks.push_back(block);

        idx++;
      }
}


void
BlockCollection::filterBlocks(std::function< bool(Block const *) > isEmpty)
{
  for (Block *b : m_blocks) {
    if (!isEmpty(b)) {
      m_nonEmptyBlocks.push_back(b);
    }
  }
}


bool
BlockCollection::initBlockTextures(std::string const & file)
{
  bool rval = false;
  DataType type{ IndexFileHeader::getType(m_indexFile->getHeader()) };
  switch (type) {
  case DataType::UnsignedCharacter:
    do_initBlockTextures< unsigned char >(file);
    break;
  case DataType::UnsignedShort:
    do_initBlockTextures< unsigned short >(file);
    break;
  case DataType::Float:
  default:
    do_initBlockTextures< float >(file);
    break;
  }

  return rval;

}


template< typename Ty >
bool
BlockCollection::do_initBlockTextures
(
  std::string const &file
)
{
  std::ifstream is(file, std::ios::binary);
  if (!is.is_open()) {
    Err() << "Could not open rawfile: " << file << ".";
    return false;
  }

  //TODO: use Region from Volume class to get block voxel extents.
  size_t buf_size{
      m_blocks[0]->voxel_extent().x *
      m_blocks[0]->voxel_extent().y *
      m_blocks[0]->voxel_extent().z };

  Ty *buf{ new Ty[buf_size] };
  float *tex{ new float[buf_size] };

  std::cout << std::endl;
  int i{ 0 };
  for (auto *b : m_nonEmptyBlocks) {
    std::cout << "\rInitializing texture block " << ++i << "/" << m_nonEmptyBlocks.size();
    fillBlockData< Ty >(*b, is, buf);
    for (size_t idx{ 0 }; idx < buf_size; ++idx) {
      tex[idx] = buf[idx] / static_cast<float>(m_indexFile->getHeader().vol_max);
    }

    b->texture().genGLTex3d(bd::Texture::Format::RED,
        bd::Texture::Format::RED,
        b->voxel_extent().x,
        b->voxel_extent().y,
        b->voxel_extent().z,
        //IndexFileHeader::getType(m_indexFile->getHeader()),
        DataType::Float,
        tex);
  }
  std::cout << " ...done." << std::endl;

  delete[] buf;
  delete[] tex;

  return true;
}


template< typename Ty >
void
BlockCollection::fillBlockData
(
  Block const& b,
  std::istream& infile,
  Ty * blockBuffer
) const
{
//  const glm::u64vec3 &nb{ m_volume.lower().block_count() };
//  const glm::u64vec3 &bd{ m_volume.lower().block_dims() };
//  const glm::u64vec3 &vd{ m_volume.dims() };

  glm::u64vec3 const& bd{ b.voxel_extent() };
  glm::u64vec2 const& vd{ m_indexFile->getHeader().volume_extent[0],
      m_indexFile->getHeader().volume_extent[1] };

  // start element = block index w/in volume * block size
  const glm::u64vec3 start{ b.ijk() * bd };
  // block end element = block voxel start dims + block size
  const glm::u64vec3 end{ start + bd };
  // byte offset into file to read from
  size_t offset{ b.fileBlock().data_offset };

  // Loop through rows and slabs of volume reading rows of voxels into memory.
  const size_t blockRowLength{ bd.x };
  for (auto slab = start.z; slab < end.z; ++slab) {
    for (auto row = start.y; row < end.y; ++row) {

      // seek to start of row
      infile.seekg(offset, infile.beg);

      // read the bytes of current row
      infile.read(reinterpret_cast<char *>(blockBuffer), blockRowLength * sizeof(Ty));
      blockBuffer += blockRowLength;

      // offset of next row
      offset = bd::to1D(start.x, row + 1, slab, vd.x, vd.y);
      offset *= sizeof(Ty);
    }
  }
}


std::vector<Block *> const &
BlockCollection::blocks()
{
  return m_blocks;
}


std::vector<Block *> const &
BlockCollection::nonEmptyBlocks()
{
  return m_nonEmptyBlocks;
}




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
//BlockCollection::blockDims(const glm::u64vec3& dims)
//{
//  m_blockDims = dims;
//}
//
//
/////////////////////////////////////////////////////////////////////////////////
//glm::u64vec3
//BlockCollection::volDims()
//{
//  return m_volDims;
//}
//
//
/////////////////////////////////////////////////////////////////////////////////
//void
//BlockCollection::volDims(const glm::u64vec3& voldims)
//{
//  m_volDims = voldims;
//}


///////////////////////////////////////////////////////////////////////////////
// nb: number of blocks
// vd: volume voxel dimensions
// blocks: out parameter to be filled with blocks.
//void
//BlockCollection::initBlocks(glm::u64vec3 nb, glm::u64vec3 vd)
//{
//  m_blockDims = vd / nb;
//  m_volDims = vd;
//  m_numBlocks = nb;
//
//  // block world dims
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


