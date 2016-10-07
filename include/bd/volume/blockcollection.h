#ifndef block_collection_h__
#define block_collection_h__


#include <bd/volume/block.h>
#include <bd/io/indexfile.h>

#include <functional>

namespace bd
{
class BlockCollection
{
public:
  BlockCollection();
  ~BlockCollection();
  BlockCollection(BlockCollection const &) = delete;
  BlockCollection(BlockCollection const &&) = delete;

//  void initBlocksFromIndexFile(const std::string &fileName);
//  void initBlocksFromIndexFile(std::unique_ptr<bd::IndexFile> index);

  void initBlocksFromIndexFile(std::shared_ptr<IndexFile const> index);

  /////////////////////////////////////////////////////////////////////////////////
  /// \brief Initializes \c blocks from the provided vector of FileBlock.
  /// \note Blocks are sized to fit it within the world-extent of the volume data.
  /// \param fileBlocks[in] The FileBlocks generated from the IndexFile.
  /// \param numBlocks[in]  The number of blocks to generate in each dimension.
  void initBlocksFromFileBlocks(std::vector<FileBlock> const &fileBlocks,
                                glm::u64vec3 const &numblocks);

  void filterBlocks(std::function<bool(Block const *)> isEmpty);

  bool initBlockTextures(std::string const &rawFile);

  std::vector<Block *> const &blocks() const;
  std::vector<Block *>& blocks();

  std::vector<Block *> const& nonEmptyBlocks() const;
  std::vector<Block *>& nonEmptyBlocks();

  IndexFile const& indexFile() const { return *m_indexFile; }

private:

  /// \brief initialize block texture from the raw data at \c file.
  template<class Ty>
  bool do_initBlockTextures(std::string const &file);

  /// \brief Fills \c blockBuffer with the data from \c infile corresponding to block (i,j,k).
  /// \param b[in] The block for which data should be read.
  /// \param infile[in] Source data
  /// \param blockBuffer[out] Destination space for data.
  template<class Ty>
  void fillBlockData( Block const &b, std::istream& infile, Ty* blockBuffer) const;

  std::vector<Block *> m_blocks;
  std::vector<Block *> m_nonEmptyBlocks;
  std::shared_ptr<IndexFile const> m_indexFile;

  //TODO: volume member in BlockCollection.

};


template< typename Ty >
bool
BlockCollection::do_initBlockTextures(std::string const &file)
{
  Dbg() << "Initializing block textures";
  std::ifstream is(file, std::ios::binary);
  if (!is.is_open()) {
    Err() << "Could not open rawfile: " << file << ".";
    return false;
  }

  //TODO: Handle multiple block sizes: find largest block and use that for buf_size.
  // Since all the blocks are the same size, just use the first block's size
  // to get the voxel dimensions for all the blocks. These dimensions are the
  // size for the buffer used for handing the texture data to OpenGL.
  size_t buf_size{ m_blocks[0]->voxel_extent().x *
                       m_blocks[0]->voxel_extent().y *
                       m_blocks[0]->voxel_extent().z };

  Ty *buf{ new Ty[buf_size] };
  float *tex{ new float[buf_size] };

  std::cout << std::endl;
  int i{ 0 };
  for (auto *b : m_nonEmptyBlocks) {
    std::cout << "\rInitializing texture block " << ++i << "/" << m_nonEmptyBlocks.size();

    // Read data for this block from the disk.
    fillBlockData< Ty >(*b, is, buf);

    // Normalize the data prior to generating the texture.
    for (size_t idx{ 0 }; idx < buf_size; ++idx) {
      tex[idx] = buf[idx] / static_cast<float>(m_indexFile->getHeader().vol_max);
    }

    b->texture().genGLTex3d(bd::Texture::Format::RED,
                            bd::Texture::Format::RED,
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


template< typename Ty >
void
BlockCollection::fillBlockData(Block const& b, std::istream& infile,
                               Ty * blockBuffer) const
{
//  const glm::u64vec3 &nb{ m_volume.lower().block_count() };
//  const glm::u64vec3 &bd{ m_volume.lower().block_dims() };
//  const glm::u64vec3 &vd{ m_volume.dims() };

  glm::u64vec3 const & bd{ b.voxel_extent() };
  glm::u64vec2 const & vd{ m_indexFile->getHeader().volume_extent[0],
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




} // namespace bd

#endif // !block_collection_h__


