#ifndef indexfile_h__
#define indexfile_h__

#include <bd/io/fileblock.h>
#include <bd/volume/fileblockcollection.h>
//#include <bd/io/datatypes.h>
#include <bd/io/indexfileheader.h>
#include <iostream>
#include <cstdint>
#include <fstream>

namespace bd
{



namespace
{
///< Magic number for the file (ascii 'SV')
uint16_t const MAGIC{ 7376 };
/// \brief The version of the IndexFile
uint16_t const VERSION{ 3 };
/// \brief Length of the IndexFileHeader in bytes.
uint32_t const HEAD_LEN{ sizeof(IndexFileHeader) };
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


  virtual std::vector<FileBlock> const &
  getBlocks() = 0;


  virtual std::vector<FileBlock*> const &
  getNonEmptyBlocks() = 0;


  virtual const Volume &
  getVolume() = 0;

};

/*****************************************************************************
 * FileBlockCollectionWrapper                                              *
*****************************************************************************/

/// \sa FileBlockCollectionWrapper_Base
template<class Ty>
class FileBlockCollectionWrapper : public FileBlockCollectionWrapper_Base
{
public:

  /// \brief Create a wrapper around a FileBlockCollection<Ty>
  FileBlockCollectionWrapper(glm::u64vec3 volDims, glm::u64vec3 numBlocks);

  /// \brief Create a wrapper around the provided FileBlockCollection<Ty>.
  FileBlockCollectionWrapper(FileBlockCollection<Ty> const &col);

  void
  addBlock(FileBlock const &b) override;


  Volume const &
  getVolume() override;


  std::vector<FileBlock> const &
  getBlocks() override;


  std::vector<FileBlock*> const &
  getNonEmptyBlocks() override;


private:
  FileBlockCollection<Ty> c;

};

/*****************************************************************************
 * FileBlockCollectionWrapper  Impl                                        *
*****************************************************************************/

template<class Ty>
FileBlockCollectionWrapper<Ty>::FileBlockCollectionWrapper(glm::u64vec3 volDims,
                                                           glm::u64vec3 numBlocks)
    : c{ volDims, numBlocks }
{
}

template<class Ty>
FileBlockCollectionWrapper<Ty>::FileBlockCollectionWrapper(
    FileBlockCollection<Ty> const &col)
    : c{ col }
{
}

template<class Ty>
void
FileBlockCollectionWrapper<Ty>::addBlock(FileBlock const &b)
{
  c.addBlock(b);
}


template<class Ty>
Volume const &
FileBlockCollectionWrapper<Ty>::getVolume()
{
  return c.volume();
}


template<class Ty>
std::vector<FileBlock> const &
FileBlockCollectionWrapper<Ty>::getBlocks()
{
  return c.blocks();
}


//template<class Ty>
//std::vector<FileBlock *> const &
//FileBlockCollectionWrapper<Ty>::getNonEmptyBlocks()
//{
//  return c.nonEmptyBlocks();
//}

/// \brief Generate an index file from the provided FileBlockCollection. The
///        IndexFile can be written to disk in either ASCII or binary format.
class IndexFile
{
public:

  template<class Ty>
  static std::unique_ptr<IndexFile>
  fromBlockCollection(std::string const &path,
                      FileBlockCollection<Ty> &col,
                      bd::DataType dt);


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


  std::vector<FileBlock> const &
  getFileBlocks() const;


private:

  /// \brief Instantiate a FileBlockCollection<Ty> with \c Ty determined by
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


/*
/// \brief IndexFileHeader output stream operator.
std::ostream &
operator<<(std::ostream &os, IndexFileHeader const &h);
*/


//static
template<class Ty>
std::unique_ptr<bd::IndexFile>
IndexFile::fromBlockCollection(std::string const &path,
                               bd::FileBlockCollection<Ty> &col,
                               bd::DataType dt)
{
  std::unique_ptr<IndexFile> idxfile{ std::unique_ptr<IndexFile>{ new IndexFile{ }}};
  idxfile->m_fileName = path;

  idxfile->m_col = new FileBlockCollectionWrapper<Ty>(col);

  idxfile->m_header.magic_number = MAGIC;
  idxfile->m_header.version = VERSION;
  idxfile->m_header.header_length = HEAD_LEN;

  //TODO: add upper and lower volume boundaries.
  idxfile->m_header.numblocks[0] =
      idxfile->m_col->getVolume().block_count().x;
  idxfile->m_header.numblocks[1] =
      idxfile->m_col->getVolume().block_count().y;
  idxfile->m_header.numblocks[2] =
      idxfile->m_col->getVolume().block_count().z; // + idxfile->m_col->getVolume().upper().block_count().z;


  idxfile->m_header.dataType = IndexFileHeader::getTypeInt(dt);

  idxfile->m_header.volume_extent[0] = idxfile->m_col->getVolume().dims().x;
  idxfile->m_header.volume_extent[1] = idxfile->m_col->getVolume().dims().y;
  idxfile->m_header.volume_extent[2] = idxfile->m_col->getVolume().dims().z;

  glm::u64vec3 blkExt = idxfile->m_col->getVolume().extent();
  idxfile->m_header.blocks_extent[0] = blkExt.x;
  idxfile->m_header.blocks_extent[1] = blkExt.y;
  idxfile->m_header.blocks_extent[2] = blkExt.z;

  idxfile->m_header.vol_empty_voxels = idxfile->m_col->getVolume().numEmptyVoxels();
  idxfile->m_header.vol_avg = idxfile->m_col->getVolume().avg();
  idxfile->m_header.vol_max = idxfile->m_col->getVolume().max();
  idxfile->m_header.vol_min = idxfile->m_col->getVolume().min();

  return idxfile;
}

} // namespace bd


#endif // !indexfile_h__
