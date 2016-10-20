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

  void
  initBlocksFromIndexFile(std::shared_ptr<IndexFile const> index);


  /// \brief Initializes \c blocks from the provided vector of FileBlock.
  /// \note Blocks are sized to fit it within the world-extent of the volume data.
  /// \param fileBlocks[in] The FileBlocks generated from the IndexFile.
  /// \param numBlocks[in]  The number of blocks to generate in each dimension.
  void
  initBlocksFromFileBlocks(std::vector<FileBlock> const &fileBlocks,
                           glm::u64vec3 const &numblocks);


  void
  filterBlocks(std::function<bool(Block const &)> filter);


  bool
  initBlockTextures(std::string const &rawFile);


  std::vector<Block *> const &
  blocks() const;


  std::vector<Block *> &
  blocks();


  std::vector<Block *> const &
  nonEmptyBlocks() const;


  std::vector<Block *> &
  nonEmptyBlocks();


  IndexFile const &
  indexFile() const;


  /// Find the largest non-empty block and return the number of voxels.
  /// \return size_t that is the number of voxels in the largest block.
  size_t
  largestNonEmptyBlock();


private:

  /// \brief initialize block texture from the raw data at \c file.
  template<class Ty>
  bool
  do_initBlockTextures(std::string const &file);


  /// \brief Fills \c blockBuffer with the data from \c infile corresponding to block (i,j,k).
  /// \param b[in] The block for which data should be read.
  /// \param infile[in] Source data
  /// \param blockBuffer[out] Destination space for data.
  template<class Ty>
  void
  fillBlockData(Block const &b, std::istream &infile, Ty *blockBuffer) const;


  std::vector<Block *> m_blocks;

  std::vector<Block *> m_nonEmptyBlocks;

  std::shared_ptr<IndexFile const> m_indexFile;

  //TODO: volume member in BlockCollection.

};


template<typename Ty>
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
    Err() << "There are no non-empty blocks to initialize texture for.";
    return false;
  }

  // init buffers for the raw values and a
  // texture buffer that contains normalized values.
  size_t buf_size{ largestNonEmptyBlock() };
  Ty *buf{ new Ty[buf_size] };
  float *tex{ new float[buf_size] };


  int i{ 0 };
  double diff{ m_indexFile->getHeader().vol_max -
                   m_indexFile->getHeader().vol_min };

  // Generate textures for each non-empty block
  std::cout << std::endl;
  for (Block *b : m_nonEmptyBlocks) {

    std::cout << "\rInitializing texture block "
              << ++i << "/" << m_nonEmptyBlocks.size() << std::flush;


    // Read data for this block from the disk.
    fillBlockData<Ty>(*b, is, buf);


    // Normalize the data prior to generating the texture.
    for (size_t idx{ 0 }; idx < buf_size; ++idx) {
      tex[idx] = buf[idx] / diff;
    }

    //TODO: try not normalizing the texture data and see what happens... :)
    b->texture().genGLTex3d(bd::Texture::Format::RED,
                            bd::Texture::Format::RED,
                            b->voxel_extent().x,
                            b->voxel_extent().y,
                            b->voxel_extent().z,
                            DataType::Float,
                            buf);
  } //for
  std::cout << " ...done." << std::endl;

  delete[] buf;
  delete[] tex;

  return true;
}


template<typename Ty>
void
BlockCollection::fillBlockData(Block const &b, std::istream &infile,
                               Ty *blockBuffer) const
{
//  const glm::u64vec3 &nb{ m_volume.lower().block_count() };
//  const glm::u64vec3 &bd{ m_volume.lower().block_dims() };
//  const glm::u64vec3 &vd{ m_volume.dims() };

  glm::u64vec3 const &bd{ b.voxel_extent() };
  glm::u64vec2 const &vd{ m_indexFile->getHeader().volume_extent[0],
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


