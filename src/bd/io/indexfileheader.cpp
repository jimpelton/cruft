//
// Created by jim on 10/3/16.
//
#include <bd/io/indexfileheader.h>

namespace bd
{

/*****************************************************************************
 * I n d e x F i l e H e a d e r                                             *
*****************************************************************************/


///////////////////////////////////////////////////////////////////////////////
IndexFileHeader
IndexFileHeader::fromStream(std::istream &is)
{
  IndexFileHeader ifh;
  is.seekg(0, std::ios::beg);

  is.read(reinterpret_cast<char *>(&ifh), sizeof(IndexFileHeader));

  return ifh;
}


///////////////////////////////////////////////////////////////////////////////
void
IndexFileHeader::writeToStream(std::ostream &os, IndexFileHeader const &ifh)
{
  os.write(reinterpret_cast<const char *>(&ifh), sizeof(IndexFileHeader));
}


///////////////////////////////////////////////////////////////////////////////
DataType
IndexFileHeader::getType(IndexFileHeader const &ifh)
{
  uint32_t type{ ifh.dataType };
  switch (type) {
    case 0x0:
      return DataType::Character;
    case 0x1:
      return DataType::Short;
    case 0x2:
      return DataType::Integer;
    case 0x3:
      return DataType::UnsignedCharacter;
    case 0x4:
      return DataType::UnsignedShort;
    case 0x5:
      return DataType::UnsignedInteger;
    case 0x6:
    default:
      return DataType::Float;
  }
}


///////////////////////////////////////////////////////////////////////////////
uint32_t
IndexFileHeader::getTypeInt(DataType ty)
{
  switch (ty) {
    case DataType::Character:
      return 0x0;
    case DataType::Short:
      return 0x1;
    case DataType::Integer:
      return 0x2;
    case DataType::UnsignedCharacter:
      return 0x3;
    case DataType::UnsignedShort:
      return 0x4;
    case DataType::UnsignedInteger:
      return 0x5;
    case DataType::Float:
    default:
      return 0x6;
  }
}



// IndexFileHeader operator<<
std::ostream &
operator<<(std::ostream &os, bd::IndexFileHeader const &h)
{
  os <<
     "\"header\": {\n"
         "  \"magic\": " << h.magic_number << ",\n"
         "  \"version\": " << h.version << ",\n"
         "  \"header_length\": " << h.header_length << ",\n"
         "  \"num_blocks\": [" << h.numblocks[0] << ", " << h.numblocks[1] << ", "
     << h.numblocks[2] << "],\n"
         "  \"data_type\": \"" << bd::to_string(IndexFileHeader::getType(h)) << "\",\n"
         "  \"volume_extent\": [" << h.volume_extent[0] << ", " << h.volume_extent[1]
     << ", " << h.volume_extent[2] << "],\n"
         "  \"blocks_extent\": [" << h.blocks_extent[0] << ", " << h.blocks_extent[1]
     << ", " << h.blocks_extent[2] << "],\n"
         "  \"vol_empty_voxels\": " << h.vol_empty_voxels << ",\n"
         "  \"volume_world_dims\": [" << h.volume_world_dims[0] << ", "
     << h.volume_world_dims[1] << ", " << h.volume_world_dims[2] << "],\n"
         "  \"vol_min\": " << std::fixed << h.vol_min << ",\n"
         "  \"vol_max\": " << std::fixed << h.vol_max << ",\n"
         "  \"vol_avg\": " << std::fixed << h.vol_avg << ",\n"
         "  \"vol_total\": " << std::fixed << h.vol_total << "\n"
         "}";

  return os;
}

} // namespace bd
