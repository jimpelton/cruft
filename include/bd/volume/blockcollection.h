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

  void initBlocksFromIndexFile(const std::string &fileName);

  void initBlocksFromIndexFile(std::shared_ptr<IndexFile> index);

  /////////////////////////////////////////////////////////////////////////////////
  /// \brief Initializes \c blocks from the provided vector of FileBlock.
  /// \note Blocks are sized to fit it within the world-extent of the volume data.
  /// \param fileBlocks[in] The FileBlocks generated from the IndexFile.
  /// \param numBlocks[in]  The number of blocks to generate in each dimension.
  /// \param blocks[out] Vector that new blocks are pushed onto.
  void initBlocksFromFileBlocks(const std::vector<FileBlock*> fileBlocks, glm::u64vec3 numblocks);

  void filterBlocks(std::function<bool(const Block*)> isEmpty);

  bool initBlockTextures(const std::string &rawFile);

  std::vector<Block *> const& blocks() const;
  std::vector<Block *>& blocks();

  std::vector<Block *> const& nonEmptyBlocks() const;
  std::vector<Block *>& nonEmptyBlocks();

  IndexFile const& indexFile() const { return *m_indexFile; }

private:

  template<typename Ty>
  bool
  do_initBlockTextures(const std::string &file);

  /////////////////////////////////////////////////////////////////////////////////
  /// \brief Fills \c out_blockData with part of \c in_data corresponding to block (i,j,k).
  /// \param ijk[in]     ijk coords of the block whos data to get.
  /// \param bsz[in]     The size of the block data.
  /// \param volsz[in]   The size of the volume data s.t.
  ///                    volsz.x*volsz.y*volsz.z == length(in_data).
  /// \param in_data[in] Source data
  /// \param out_blockData[out] Destination space for data.

  template<typename Ty>
  void
  fillBlockData( const Block& b, std::istream& infile, Ty* blockBuffer) const;

//  static glm::u64vec3 m_blockDims; ///< Dimensions of a block in something.
//  static glm::u64vec3 m_volDims; ///< Volume dimensions (# data points).
//  static glm::u64vec3 m_numBlocks; ///< Number of blocks volume is divided into.

  std::vector<Block *> m_blocks;
  std::vector<Block *> m_nonEmptyBlocks;

  std::shared_ptr<IndexFile> m_indexFile;

  //TODO: volume member in BlockCollection.

};
} // namespace bd

#endif // !block_collection_h__


