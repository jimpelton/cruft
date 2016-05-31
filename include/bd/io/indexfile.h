#ifndef indexfile_h__
#define indexfile_h__

#include <bd/io/fileblock.h>
#include <bd/volume/blockcollection2.h>
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

  uint32_t dataType;            ///< Int representing the data type of elements (char, short, etc).
  uint64_t volume_extent[3];          ///< Dimensions of volume in voxels.
  uint64_t blocks_extent[3];

  // Volume statistics
  uint64_t vol_empty_voxels;    ///< Num voxels determined empty (ie, irrelevent voxels).
  double vol_avg;
  double vol_min;
  double vol_max;

public:
  /// \brief Generate an IndexFileHeader from an input stream of binary data.
  static IndexFileHeader fromStream(std::istream&);


  /// \brief Write an existing IndexFileHeader to a binary ostream.
  static void writeToStream(std::ostream&, const IndexFileHeader&);


  /// \brief Convert the dataType value to a bd::DataType.
  static DataType getType(const IndexFileHeader&);


  /// \brief Convert the given bd::DataType to the integer rep used in IFH's.
  static uint32_t getTypeInt(DataType);


}; // struct IndexFileHeader


namespace
{
const uint16_t MAGIC{ 7376 }; ///< Magic number for the file (ascii 'SV')
const uint16_t VERSION{ 2 };
const uint32_t HEAD_LEN{ sizeof(IndexFileHeader) };
}  // namespace


///////////////////////////////////////////////////////////////////////////////
/// \brief Allows using the BlockCollection2 template without
///        exposing the templated-ness of BlockCollection2, since we don't
///        know what type of BC2 we need until runtime.
class collection_wrapper_base
{
public:
  virtual ~collection_wrapper_base()
  {
  }


  virtual void addBlock(const FileBlock&) = 0;


  virtual const std::vector<FileBlock*>& blocks() = 0;


  virtual const std::vector<FileBlock*>& nonEmptyBlocks() = 0;


  /// \brief Create the BlockCollection using values between minmax to do its thang.
  virtual void create(const std::string& rawFile, size_t buffSize, const float minmax[2]) = 0;


  virtual const Volume& volume() = 0;

};

///////////////////////////////////////////////////////////////////////////////
/// \sa base_collection_wrapper
///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
class collection_wrapper : public collection_wrapper_base
{
public:

  collection_wrapper(glm::u64vec3 volDims, glm::u64vec3 numBlocks)
      : c{ volDims, numBlocks }
  {
  }


  void create(const std::string& rawFile, size_t buffSize, const float minmax[2]) override
  {
    ValueRangeFilter<Ty> isRelevant(minmax[0], minmax[1]);
    c.createFromRawFile(rawFile, buffSize, isRelevant);
  }


  void addBlock(const FileBlock& b) override
  {
    c.addBlock(b);
  }


  const Volume& volume() override
  {
    return c.volume();
  }


  const std::vector<FileBlock*>& blocks() override
  {
    return c.blocks();
  }


  const std::vector<FileBlock*>& nonEmptyBlocks() override
  {
    return c.nonEmptyBlocks();
  }


private:
  BlockCollection2<Ty> c;

};

///////////////////////////////////////////////////////////////////////////////
/// \brief Generate an index file from the provided BlockCollection2. The
///        IndexFile can be written to disk in either ASCII or binary format.
class IndexFile
{
public:

  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Create IndexFile from an existing raw file.
  /// \param path The path to the raw file.
  /// \param type The data type in the file (\sa bd::DataType)
  /// \param numVox number of voxels along x, y, and z axis.
  /// \param numBlks number of blocks along x, y, and z axis.
  /// \param nummax The min and max block averages to use for threshold values 
  ///               when filtering blocks.
  static IndexFile* fromRawFile(const std::string& path,
                                size_t bufsz,
                                DataType type,
                                const uint64_t numVox[3],
                                const uint64_t numBlks[3],
                                const float minmax[2]);


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Create IndexFile from an existing binary index file.
  static IndexFile* fromBinaryIndexFile(const std::string& path);


  IndexFile();


  ~IndexFile();


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Write binary index file to ostream \c os.
  void writeBinaryIndexFile(std::ostream& os);


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Write binary index file to the file at \c outpath.
  void writeBinaryIndexFile(const std::string& outpath);


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Write ascii index file to ostream \c os.
  void writeAsciiIndexFile(std::ostream& os);


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Write ascii index file to the file at \c outpath.
  void writeAsciiIndexFile(const std::string& outpath);


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Get the IndexFileHeader for the index file.
  const IndexFileHeader& getHeader() const;


  const std::vector<FileBlock*>& blocks() const;


  static collection_wrapper_base* make_wrapper(DataType type,
                                               const uint64_t num_vox[3],
                                               const uint64_t numblocks[3]);


private:
  /// \brief Read binary index file and populate \c a collection with blocks
  bool readBinaryIndexFile();

private:
  IndexFileHeader m_header;
  std::string m_fileName;
  collection_wrapper_base* m_col;

};

///////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, const IndexFileHeader& h);

} // namespace bd


#endif // !indexfile_h__
