#ifndef indexfile_h__
#define indexfile_h__

#include <bd/io/fileblock.h>
#include <bd/volume/fileblockcollection.h>
#include <bd/io/datatypes.h>

#include <iostream>
#include <cstdint>
#include <fstream>

namespace bd
{

///////////////////////////////////////////////////////////////////////////////
///   \brief The header for the index file.
struct IndexFileHeader
{
public:
  // IndexFileHeader Data
  uint16_t magic_number;
  uint16_t version;
  uint32_t header_length;

  // Block metadata

  uint64_t numblocks[3];        ///< Num blocks along each coordinate axis.

  uint32_t
      dataType;            ///< Int representing the data type of elements (char, short, etc).
  uint64_t volume_extent[3];    ///< Dimensions of volume in voxels.
  uint64_t blocks_extent[3
  ];    ///< The voxel extent of the blocks (may not equal the volume voxel extent).
  ///  Equates to the extent of voxels processed.
  // Volume statistics
//  uint64_t vol_empty_voxels;    ///< Num voxels determined empty (ie, irrelevent voxels).
  double vol_avg;
  double vol_min;
  double vol_max;

public:
  /// \brief Generate an IndexFileHeader from an input stream of binary data.
  static IndexFileHeader
  fromStream(std::istream &);


  /// \brief Write an existing IndexFileHeader to a binary ostream.
  static void
  writeToStream(std::ostream &, const IndexFileHeader &);


  /// \brief Convert the dataType value to a bd::DataType.
  static DataType
  getType(const IndexFileHeader &);


  /// \brief Convert the given bd::DataType to the integer rep used in IFH's.
  static uint32_t
  getTypeInt(DataType);


}; // struct IndexFileHeader


namespace
{
const uint16_t MAGIC{ 7376 }; ///< Magic number for the file (ascii 'SV')
const uint16_t VERSION{ 2 };
const uint32_t HEAD_LEN{ sizeof(IndexFileHeader) };
}  // namespace


///////////////////////////////////////////////////////////////////////////////
/// \brief Allows using the FileBlockCollection template without
///        exposing the templated-ness of FileBlockCollection, since we don't
///        know what type of BC2 we need until runtime.
class FileBlockCollectionWrapper_Base
{
public:
  virtual ~FileBlockCollectionWrapper_Base()
  {
  }


  virtual void
  addBlock(FileBlock const &) = 0;


  virtual std::vector<FileBlock*> const &
  getBlocks() = 0;


  virtual std::vector<FileBlock*> const &
  getNonEmptyBlocks() = 0;


  /// \brief Create the blocks using Function as the relevance test.
  virtual void
  generateBlocksFromRawFile(std::string const &rawFile, size_t buffSize) = 0;


  virtual const Volume &
  getVolume() = 0;

};

/// \sa FileBlockCollectionWrapper_Base
template<typename DataType>
class FileBlockCollectionWrapper
    : public FileBlockCollectionWrapper_Base
{
public:

  FileBlockCollectionWrapper(glm::u64vec3 volDims, glm::u64vec3 numBlocks);


  void
  generateBlocksFromRawFile(std::string const &rawFile, size_t buffSize) override;


  void
  addBlock(FileBlock const &b) override;


  Volume const &
  getVolume() override;


  std::vector<FileBlock*> const &
  getBlocks() override;


  std::vector<FileBlock*> const &
  getNonEmptyBlocks() override;


private:
  FileBlockCollection<DataType> c;

};

/// \brief Generate an index file from the provided FileBlockCollection. The
///        IndexFile can be written to disk in either ASCII or binary format.
class IndexFile
{
public:

  /// \brief Create IndexFile from an existing raw file.
  /// \param path The path to the raw file.
  /// \param dataType The data type in the file (\sa bd::DataType)
  /// \param numVox number of voxels along x, y, and z axis.
  /// \param numBlks number of blocks along x, y, and z axis.
  /// \param nummax The min and max block averages to use for threshold values 
  ///               when filtering blocks.
  /// \param function The voxel relevance function.
  /// \returns A unique_ptr to the IndexFile generated.
  template<typename DataType>
  static std::unique_ptr<IndexFile>
  fromRawFile(std::string const &path,
              size_t bufsz,
              bd::DataType dataType,
              uint64_t const numVox[3],
              uint64_t const numBlks[3],
              std::function<bool(DataType)> function);


  /// \brief Create IndexFile from an existing binary index file.
  /// \returns A unique_ptr to the IndexFile created or nullptr on failure.
  static std::unique_ptr<IndexFile>
  fromBinaryIndexFile(std::string const &path);


  /// \brief Create an empty IndexFile with empty header.
  IndexFile();
  ~IndexFile();


  /// \brief Write binary index file to ostream \c os.
  void
  writeBinaryIndexFile(std::ostream &os) const;


  /// \brief Write binary index file to the file at \c outpath.
  void
  writeBinaryIndexFile(std::string const &outpath) const;


  /// \brief Write ascii index file to ostream \c os.
  void
  writeAsciiIndexFile(std::ostream &os) const;


  /// \brief Write ascii index file to the file at \c outpath.
  void
  writeAsciiIndexFile(std::string const &outpath) const;


  /// \brief Get the IndexFileHeader for the index file.
  IndexFileHeader const &
  getHeader() const;


  std::vector<FileBlock*> const &
  getBlocks() const;


private:

  /// \brief Instantiate a FileBlockCollection<Ty> with Ty determined by
  ///        the \c type parameter.
  static FileBlockCollectionWrapper_Base*
  instantiate_wrapper(DataType type, const uint64_t* num_vox, const uint64_t* numblocks);


  /// \brief Read binary index file and populate \c a collection with blocks
  bool
  readBinaryIndexFile();


  IndexFileHeader m_header;
  std::string m_fileName;
  FileBlockCollectionWrapper_Base* m_col;
};  // class IndexFile


/// \brief IndexFileHeader output stream operator.
std::ostream &
operator<<(std::ostream &os, IndexFileHeader const &h);


// static
template<typename Ty>
std::unique_ptr<IndexFile>
IndexFile::fromRawFile(std::string const &path,
                       size_t bufsz,
                       bd::DataType dataType,
                       uint64_t const num_vox[3],
                       uint64_t const numblocks[3],
                       std::function<bool(Ty)> function)
{
  std::unique_ptr<IndexFile> idxfile{ std::unique_ptr<IndexFile>{ new IndexFile{ }}};
  idxfile->m_fileName = path;

  // instantiate FileBlockCollection object.
  idxfile->m_col =
      new FileBlockCollectionWrapper<Ty>{{ num_vox[0],   num_vox[1],   num_vox[2] },
                                         { numblocks[0], numblocks[1], numblocks[2] }};

  // build the block collection
  idxfile->m_col->generateBlocksFromRawFile(idxfile->m_fileName, bufsz);

  idxfile->m_header.magic_number = MAGIC;
  idxfile->m_header.version = VERSION;
  idxfile->m_header.header_length = HEAD_LEN;

  //TODO: add upper and lower volume boundaries.
  idxfile->m_header.numblocks[0] =
      idxfile->m_col->getVolume().lower().block_count()
             .x; // + idxfile->m_col->volume().upper().block_count().x;
  idxfile->m_header.numblocks[1] =
      idxfile->m_col->getVolume().lower().block_count()
             .y; // + idxfile->m_col->getVolume().upper().block_count().y;
  idxfile->m_header.numblocks[2] =
      idxfile->m_col->getVolume().lower().block_count()
             .z; // + idxfile->m_col->getVolume().upper().block_count().z;


  idxfile->m_header.dataType = IndexFileHeader::getTypeInt(dataType);

  idxfile->m_header.volume_extent[0] = idxfile->m_col->getVolume().dims().x;
  idxfile->m_header.volume_extent[1] = idxfile->m_col->getVolume().dims().y;
  idxfile->m_header.volume_extent[2] = idxfile->m_col->getVolume().dims().z;

  glm::u64vec3 blkExt = idxfile->m_col->getVolume().lower().extent();
  idxfile->m_header.blocks_extent[0] = blkExt.x;
  idxfile->m_header.blocks_extent[1] = blkExt.y;
  idxfile->m_header.blocks_extent[2] = blkExt.z;

  idxfile->m_header.vol_avg = idxfile->m_col->getVolume().avg();
  idxfile->m_header.vol_max = idxfile->m_col->getVolume().max();
  idxfile->m_header.vol_min = idxfile->m_col->getVolume().min();

  return idxfile;

}


} // namespace bd


#endif // !indexfile_h__
