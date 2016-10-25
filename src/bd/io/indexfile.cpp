
#include <bd/io/indexfile.h>
#include <bd/filter/blockaveragefilter.h>

#include <glm/glm.hpp>

#include <istream>
#include <ostream>
#include <bd/filter/voxelopacityfilter.h>

namespace bd
{

/*****************************************************************************
 * IndexFile                                                                 *
*****************************************************************************/


///////////////////////////////////////////////////////////////////////////////
std::unique_ptr<IndexFile>
IndexFile::fromBinaryIndexFile(std::string const &path)
{
  std::unique_ptr<IndexFile> idxfile{ new IndexFile() };
  idxfile->m_fileName = path;
  bool success = idxfile->readBinaryIndexFile();
  if (!success) {
    //delete idxfile;
    return nullptr;
  }

  if (idxfile->getHeader().version != VERSION){
    Err() << "The index file provided is the wrong version! You should regenerate the"
        "index file.";
    return nullptr;
  }

  return idxfile;
}


///////////////////////////////////////////////////////////////////////////////
IndexFile::IndexFile()
    : m_header{ }
    , m_fileName{ }
    , m_col{ nullptr }
{
}


///////////////////////////////////////////////////////////////////////////////
IndexFile::~IndexFile()
{
  if (m_col) {
    delete m_col;
  }
}


///////////////////////////////////////////////////////////////////////////////
const IndexFileHeader &
IndexFile::getHeader() const
{
  return m_header;
}


///////////////////////////////////////////////////////////////////////////////
FileBlockCollectionWrapper_Base *
IndexFile::instantiate_wrapper(DataType type,
                               uint64_t const *num_vox,
                               uint64_t const *numblocks)
{
  FileBlockCollectionWrapper_Base *col{ nullptr };

  switch (type) {

    case bd::DataType::UnsignedCharacter:
      col = new FileBlockCollectionWrapper<unsigned char>{
          { num_vox[0],   num_vox[1],   num_vox[2] },
          { numblocks[0], numblocks[1], numblocks[2] }};
      break;

    case bd::DataType::Character:
      col = new FileBlockCollectionWrapper<char>
          {{ num_vox[0],   num_vox[1],   num_vox[2] },
           { numblocks[0], numblocks[1], numblocks[2] }};
      break;

    case bd::DataType::UnsignedShort:
      col = new FileBlockCollectionWrapper<unsigned short>
          {{ num_vox[0],   num_vox[1],   num_vox[2] },
           { numblocks[0], numblocks[1], numblocks[2] }};
      break;

    case bd::DataType::Short:
      col = new FileBlockCollectionWrapper<short>
          {{ num_vox[0],   num_vox[1],   num_vox[2] },
           { numblocks[0], numblocks[1], numblocks[2] }};
      break;

    case bd::DataType::Float:
      col = new FileBlockCollectionWrapper<float>
          {{ num_vox[0],   num_vox[1],   num_vox[2] },
           { numblocks[0], numblocks[1], numblocks[2] }};
      break;

    default:
      std::cerr << "Unsupported/unknown datatype: " << bd::to_string(type) << ".\n";
      break;
  }

  return col;
}


///////////////////////////////////////////////////////////////////////////////
void
IndexFile::writeBinaryIndexFile(std::ostream &os) const
{
  // write header to stream.
  IndexFileHeader::writeToStream(os, m_header);
  // read all the blocks
  for (auto &b : m_col->getBlocks()) {
    os.write(reinterpret_cast<const char *>(&b), sizeof(FileBlock));
  }
}


///////////////////////////////////////////////////////////////////////////////
void
IndexFile::writeBinaryIndexFile(std::string const &outpath) const
{
  std::ofstream os;
  os.open(outpath, std::ios::binary);
  if (!os.is_open()) {
    std::cerr << outpath << " could not be opened." << std::endl;
    return;
  }

  writeBinaryIndexFile(os);
  os.flush();
  os.close();

}


///////////////////////////////////////////////////////////////////////////////
void
IndexFile::writeAsciiIndexFile(std::ostream &os) const
{
  // os << "\"index\": {\n";
  //
  // Open outer JSON object
  os << "{\n";
  os << m_header << ",\n\"blocks\": { \n";

  std::vector<FileBlock> const &blocks = m_col->getBlocks();
  for (size_t i{ 0 }; i<blocks.size()-1; ++i) {
    os << blocks[i] << ",\n";
  }
  os << blocks[blocks.size()-1] << "\n";

  os << "}}\n";
}


///////////////////////////////////////////////////////////////////////////////
void
IndexFile::writeAsciiIndexFile(std::string const &outpath) const
{
  std::ofstream os;
  os.open(outpath);
  if (!os.is_open()) {
    Err() << outpath << " could not be opened!";
    return;
  }

  writeAsciiIndexFile(os);

  os.flush();
  os.close();
}


///////////////////////////////////////////////////////////////////////////////
std::vector<FileBlock> const &
IndexFile::getFileBlocks() const
{
  return m_col->getBlocks();
}


///////////////////////////////////////////////////////////////////////////////
bool
IndexFile::readBinaryIndexFile()
{
  // open index file (binary)
  std::ifstream is{ m_fileName, std::ios::binary };
  if (!is.is_open()) {
    Err() << "The file " << m_fileName << " could not be opened.";
    return false;
  }

  // read header
  IndexFileHeader ifh;
  is.seekg(0, std::ios::beg);
  is.read(reinterpret_cast<char *>(&ifh), sizeof(IndexFileHeader));
  m_header = ifh;

  m_col = IndexFile::instantiate_wrapper(IndexFileHeader::getType(ifh),
                                         ifh.volume_extent,
                                         ifh.numblocks);

  size_t numBlocks{ ifh.numblocks[0]*ifh.numblocks[1]*ifh.numblocks[2] };

  // read many blocks!
  FileBlock fb;
  for (size_t i = 0; i<numBlocks; ++i) {
    is.read(reinterpret_cast<char *>(&fb), sizeof(FileBlock));
    m_col->addBlock(fb);
  }

  return true;
}



// FileBlock operator<<
std::ostream &
operator<<(std::ostream &os, bd::FileBlock const &block)
{
  return os << block.to_string();
}



} // namepsace bd
