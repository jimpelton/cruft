#ifndef indexfile_h__
#define indexfile_h__

#include "fileblock.h"
#include "blockcollection2.h"

#include <iostream>
#include <cstdint>
#include <bd/file/datatypes.h>

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
///   Volume average val  | 4 bytes float
///   Volume min value    | 4 bytes float
///   Volume max value    | 4 bytes float
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

  float vol_avg;
  float vol_min;
  float vol_max;

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
/// \brief Generate an index file from the provided BlockCollection2. The
///        IndexFile can be written to disk in either ASCII or binary format.
///////////////////////////////////////////////////////////////////////////////
class IndexFile
{

  enum class Mode
  {
    Read, Write
  };

public:
  IndexFile();

  IndexFile(const std::string& fileName);

  ~IndexFile();


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Read binary index file from \c is and populate \c collection with
  ///        blocks.
  ///////////////////////////////////////////////////////////////////////////////
  bool readBinary();

  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Write binary index file to ostream \c os.
  ///////////////////////////////////////////////////////////////////////////////
  void writeBinary(std::ostream& os);

  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Write ascii index file to ostream \c os.
  ///////////////////////////////////////////////////////////////////////////////
  void writeAscii(std::ostream& os);


//    static IndexFileHeader makeHeaderFromCollection(
//        const BlockCollection2<Ty>& collection);


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Get the IndexFileHeader for the index file.
  ///////////////////////////////////////////////////////////////////////////////
  const IndexFileHeader& getHeader() const;

private:

  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Write single block binary to \c os.
  ///////////////////////////////////////////////////////////////////////////////
  void writeSingleBlockHeaderBinary(std::ostream& os, const FileBlock& block);

  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Write the binary header for index file.
  ///////////////////////////////////////////////////////////////////////////////
  void writeIndexFileHeaderBinary(std::ostream& os);

  void openRead();

  void openWrite();

  class base_collection_wrapper {
  public:
    virtual ~base_collection_wrapper() { }

    virtual void addBlock(const FileBlock&) = 0;
  };

  template<typename Ty>
  class collection_wrapper : public base_collection_wrapper
  {
  public:
    collection_wrapper(glm::u64vec3 volDims, glm::u64vec3 numBlocks)
        : c{ volDims, numBlocks } { }

    BlockCollection2<Ty> c;

    void addBlock(const FileBlock &b) override {
      c.addBlock(b);
    }

  };

  template<Ty>

  IndexFileHeader m_header;
  std::string m_fileName;
  base_collection_wrapper *m_col;

};

} // namespace bd


#endif // !indexfile_h__
