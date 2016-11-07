
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

  if (idxfile->getHeader().version != VERSION) {
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
    , m_fileBlocks( )
{
}




///////////////////////////////////////////////////////////////////////////////
IndexFile::~IndexFile()
{
}


///////////////////////////////////////////////////////////////////////////////
const IndexFileHeader &
IndexFile::getHeader() const
{
  return m_header;
}


///////////////////////////////////////////////////////////////////////////////
//FileBlockCollectionWrapper_Base *
//IndexFile::instantiate_wrapper(DataType type,
//                               uint64_t const *num_vox,
//                               uint64_t const *numblocks)
//{
//  FileBlockCollectionWrapper_Base *col{ nullptr };
//
//  switch (type) {
//
//    case bd::DataType::UnsignedCharacter:
//      col = new FileBlockCollectionWrapper<unsigned char>{
//          { num_vox[0],   num_vox[1],   num_vox[2] },
//          { numblocks[0], numblocks[1], numblocks[2] }};
//      break;
//
//    case bd::DataType::Character:
//      col = new FileBlockCollectionWrapper<char>
//          {{ num_vox[0],   num_vox[1],   num_vox[2] },
//           { numblocks[0], numblocks[1], numblocks[2] }};
//      break;
//
//    case bd::DataType::UnsignedShort:
//      col = new FileBlockCollectionWrapper<unsigned short>
//          {{ num_vox[0],   num_vox[1],   num_vox[2] },
//           { numblocks[0], numblocks[1], numblocks[2] }};
//      break;
//
//    case bd::DataType::Short:
//      col = new FileBlockCollectionWrapper<short>
//          {{ num_vox[0],   num_vox[1],   num_vox[2] },
//           { numblocks[0], numblocks[1], numblocks[2] }};
//      break;
//
//    case bd::DataType::Float:
//      col = new FileBlockCollectionWrapper<float>
//          {{ num_vox[0],   num_vox[1],   num_vox[2] },
//           { numblocks[0], numblocks[1], numblocks[2] }};
//      break;
//
//    default:
//      std::cerr << "Unsupported/unknown datatype: " << bd::to_string(type) << ".\n";
//      break;
//  }
//
//  return col;
//}


///////////////////////////////////////////////////////////////////////////////
void
IndexFile::writeBinaryIndexFile(std::ostream &os) const
{
  // write header to stream.
  IndexFileHeader::writeToStream(os, m_header);
  // read all the blocks
  for (auto &b : m_fileBlocks) {
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

  for (size_t i{ 0 }; i<m_fileBlocks.size()-1; ++i) {
    os << m_fileBlocks[i] << ",\n";
  }
  os << m_fileBlocks[m_fileBlocks.size()-1] << "\n";

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
  return m_fileBlocks;
}

bd::Volume 
IndexFile::getVolume() const
{
  return m_volume;
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

  m_volume.min(ifh.vol_min);
  m_volume.max(ifh.vol_max);
  m_volume.avg(ifh.vol_avg);
  m_volume.block_count({ ifh.numblocks[0], ifh.numblocks[1], ifh.numblocks[2] });
  m_volume.voxelDims({ ifh.volume_extent[0], ifh.volume_extent[1], ifh.volume_extent[2] });

  initFileBlocks(IndexFileHeader::getType(ifh));

  // Read all the things!
  FileBlock fb;
  for (size_t i = 0; i < m_volume.total_block_count(); ++i) {
    is.read(reinterpret_cast<char *>(&fb), sizeof(FileBlock));
    m_fileBlocks.push_back(fb);
  }

  return true;
}

void IndexFile::initFileBlocks(bd::DataType type)
{
  size_t tySize{ bd::to_sizeType(type) };
  
  // bc: number of blocks
  // vd: volume voxel dimensions
  // bd: block dimensions
  glm::u64vec3 bc{ m_volume.block_count() };
  glm::u64vec3 vd{ m_volume.voxelDims() };
  glm::u64vec3 bd{ m_volume.block_dims() };

  // block world voxelDims 1.0 = volume world dimensions on each side.
  glm::vec3 wld_dims{ m_volume.worldDims() / glm::vec3(bc) };

  Dbg() << "Starting FileBlock creation: "
    " # blocks: "
    << bc.x << ", " << bc.y << ", " << bc.z <<
    " Vol dim: "
    << vd.x << ", " << vd.y << ", " << vd.z <<
    " Block dim: "
    << ", " << wld_dims.x << ", " << wld_dims.y << ", " << wld_dims.z;


  // Loop through all our blocks (identified by <bxi,byj,bzk>) and populate block fields.
  for (auto bzk = 0ull; bzk < bc.z; ++bzk) {
    for (auto byj = 0ull; byj < bc.y; ++byj) {
      for (auto bxi = 0ull; bxi < bc.x; ++bxi) {
        // i,j,k block identifier
        glm::u64vec3 const blkId{ bxi, byj, bzk };

        // block center in world coordinates
         glm::vec3 const blkOrigin{ wld_dims * glm::vec3(blkId) + wld_dims / 2.0f };
        
        // voxel start of block within volume
         glm::u64vec3 const startVoxel{ blkId * bd };

        FileBlock blk;
        blk.block_index = bd::to1D(bxi, byj, bzk, bc.x, bc.y);
        blk.data_offset =
          tySize * bd::to1D(startVoxel.x, startVoxel.y, startVoxel.z, vd.x, vd.y);

        blk.voxel_dims[0] = static_cast<decltype(blk.voxel_dims[0])>(bd.x);
        blk.voxel_dims[1] = static_cast<decltype(blk.voxel_dims[1])>(bd.y);
        blk.voxel_dims[2] = static_cast<decltype(blk.voxel_dims[2])>(bd.z);

        blk.world_oigin[0] = blkOrigin.x;
        blk.world_oigin[1] = blkOrigin.y;
        blk.world_oigin[2] = blkOrigin.z;

        blk.world_dims[0] = wld_dims.x;
        blk.world_dims[1] = wld_dims.y;
        blk.world_dims[2] = wld_dims.z;

        m_fileBlocks.push_back(blk);
      }
    }
  }

  Dbg() << "Total blocks is: " << m_fileBlocks.size();
}


// FileBlock operator<<
std::ostream &
operator<<(std::ostream &os, bd::FileBlock const &block)
{
  return os << block.to_string();
}



} // namepsace bd
