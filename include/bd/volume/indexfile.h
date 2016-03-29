#ifndef indexfile_h__
#define indexfile_h__

#include "fileblock.h"
#include "blockcollection2.h"

#include <iostream>
#include <cstdint>
#include <bd/file/datatypes.h>
#include <fstream>

namespace bd
{

///////////////////////////////////////////////////////////////////////////////
///   \brief The header for the index file.
///
///   Index file header layout
///   -----------------------------------------
///   magic number        | 2 bytes unsigned (7376 --> characters "sv")
///   version             | 2 bytes unsigned
///   header length       | 4 bytes unsigned
///   -----------------------------------------
///   Block metadata
///   -----------------------------------------
///   Num blocks X        | 8 bytes unsigned
///   Num blocks Y        | 8 bytes unsigned
///   Num blocks Z        | 8 bytes unsigned
///   -----------------------------------------
///   Volume statistics   
///   -----------------------------------------
///   volume data type    | 4 bytes unsigned (0x0, 0x1, 0x2, 0x3, 0x4, 0x5)
///   Num Vox X           | 8 bytes unsigned
///   Num Vox Y           | 8 bytes unsigned
///   Num Vox Z           | 8 bytes unsigned
///   Volume average val  | 8 bytes float
///   Volume min value    | 8 bytes float
///   Volume max value    | 8 bytes float
///////////////////////////////////////////////////////////////////////////////
struct IndexFileHeader
{
  uint16_t magic_number;
  uint16_t version;
  uint32_t header_length;

  // Block metadata
  uint64_t numblocks[3];

  // Volume statistics
  uint32_t dataType;
  uint64_t num_vox[3];                      ///< Dimensions of volume in voxels.

  double vol_avg;
  double vol_min;
  double vol_max;

///////////////////////////////////////////////////////////////////////////////
  static IndexFileHeader fromStream(std::istream&);
  static void writeToStream(std::ostream&, const IndexFileHeader &);
  static DataType getType(const IndexFileHeader &);
  static uint32_t getTypeInt(DataType);
};

namespace
{
const uint16_t MAGIC{ 7376 }; ///< Magic number for the file (ascii 'SV')
const uint16_t VERSION{ 1 };
const uint32_t HEAD_LEN{ sizeof(IndexFileHeader) };
}  // namespace

///////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, const IndexFileHeader& h);

///////////////////////////////////////////////////////////////////////////////
/// \brief This rediculousness allows using templated BlockCollection2 without
///        exposing the templated-ness of BlockCollection2, since we don't
///        know what type of BC2 we need until runtime.
///////////////////////////////////////////////////////////////////////////////
class base_collection_wrapper {
public:
  virtual ~base_collection_wrapper() { }

  virtual void addBlock(const FileBlock&) = 0;

  virtual const std::vector<std::shared_ptr<FileBlock>>& blocks() = 0;

  virtual const std::vector<std::weak_ptr<FileBlock>>& nonEmptyBlocks() = 0;

  virtual void filterBlocks(std::ifstream &, float min, float max) = 0;
  virtual glm::u64vec3 numBlocks() = 0;
  virtual glm::u64vec3 volDims() = 0;
  virtual glm::u64vec3 blockDims() = 0;
  virtual double volMin() = 0;
  virtual double volMax() = 0;
  virtual double volAvg() = 0;

};

///////////////////////////////////////////////////////////////////////////////
/// \sa base_collection_wrapper
///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
class collection_wrapper : public base_collection_wrapper
{
public:

  collection_wrapper(glm::u64vec3 volDims, glm::u64vec3 numBlocks)
      : c{ volDims, numBlocks }
  {
  }


  void addBlock(const FileBlock &b) override
  {
    c.addBlock(b);
  }

  const std::vector<std::shared_ptr<FileBlock>>& blocks() override
  {
    return c.blocks();
  }


  const std::vector<std::weak_ptr<FileBlock>>& nonEmptyBlocks() override
  {
    return c.nonEmptyBlocks();
  }


  void filterBlocks(std::ifstream &rawFile, float min, float max) override
  {
    c.filterBlocks(rawFile, min, max);
  }

  glm::u64vec3 numBlocks() override
  {
    return c.numBlocks();
  }

  glm::u64vec3 volDims() override
  {
    return c.volDims();
  }

  glm::u64vec3 blockDims() override
  {
    return c.blockDims();
  }

  double volMin() override
  {
    return c.volMin();
  }

  double volMax() override
  {
    return c.volMax();
  }

  double volAvg() override
  {
    return c.volAvg();
  }

private:
  BlockCollection2<Ty> c;

};

///////////////////////////////////////////////////////////////////////////////
/// \brief Generate an index file from the provided BlockCollection2. The
///        IndexFile can be written to disk in either ASCII or binary format.
///////////////////////////////////////////////////////////////////////////////
class IndexFile
{


public:

  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Create IndexFile from an existing raw file.
  ///////////////////////////////////////////////////////////////////////////////
  static std::shared_ptr<IndexFile> fromRawFile(
      const std::string &path,
      DataType type,
      const unsigned long long numVox[3],
      const unsigned long long numBlks[3],
      const float minmax[2]);


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Create IndexFile from an existing binary index file.
  ///////////////////////////////////////////////////////////////////////////////
  static std::shared_ptr<IndexFile> fromBinaryIndexFile(const std::string &path);


  IndexFile();
  ~IndexFile();


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Write binary index file to ostream \c os.
  ///////////////////////////////////////////////////////////////////////////////
  void writeBinaryIndexFile(std::ostream& os);
  void writeBinaryIndexFile(const std::string& outpath);

  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Write ascii index file to ostream \c os.
  ///////////////////////////////////////////////////////////////////////////////
  void writeAsciiIndexFile(std::ostream& os);
  void writeAsciiIndexFile(const std::string& outpath);


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Get the IndexFileHeader for the index file.
  ///////////////////////////////////////////////////////////////////////////////
  const IndexFileHeader& getHeader() const;

  const std::vector<std::shared_ptr<FileBlock>>& blocks();

  static base_collection_wrapper* make_wrapper(DataType type,
      const unsigned long long num_vox[3], const unsigned long long numblocks[3]);


private:

  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Write single block binary to \c os.
  ///////////////////////////////////////////////////////////////////////////////
//  void writeBinarySingleBlockHeader(std::ostream& os, const FileBlock& block);

  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Write the binary header for index file.
  ///////////////////////////////////////////////////////////////////////////////
//  void writeBinaryIndexFileHeader(std::ostream& os);


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Read binary index file from \c is and populate \c collection with
  ///        blocks.
  ///////////////////////////////////////////////////////////////////////////////
  bool readBinaryIndexFile();


private:
  IndexFileHeader m_header;
  std::string m_fileName;
  base_collection_wrapper *m_col;

};

} // namespace bd


#endif // !indexfile_h__
