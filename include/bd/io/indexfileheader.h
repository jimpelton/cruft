//
// Created by jim on 10/3/16.
//

#ifndef bd_INDEXFILEHEADER_H
#define bd_INDEXFILEHEADER_H

#include <bd/io/datatypes.h>

#include <cstdint>
#include <fstream>

namespace bd
{

///////////////////////////////////////////////////////////////////////////////
///   \brief The header for the index file.
struct IndexFileHeader
{

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


  /// \brief Number describing filetype.
  uint16_t magic_number;
  /// \brief IndexFile revision number
  uint16_t version;
  /// \brief Length in bytes of the header
  uint32_t header_length;
  /// \brief Num blocks along each coordinate axis.
  uint64_t numblocks[3];
  /// \brief Int representing the data type of elements (char, short, etc).
  uint32_t dataType;
  /// \brief Dimensions of volume in voxels
  uint64_t volume_extent[3];
  /// \brief The voxel extent of the blocks (may not equal the volume voxel extent).
  uint64_t blocks_extent[3];
  /// \brief Num voxels determined irrelevent.
  uint64_t vol_empty_voxels;

  double vol_avg;
  double vol_min;
  double vol_max;

}; // struct IndexFileHeader

std::ostream &
operator<<(std::ostream &os, bd::IndexFileHeader const &h);

} // namespace bd

#endif //bd_INDEXFILEHEADER_H