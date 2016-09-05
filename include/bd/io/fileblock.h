//
// Created by Jim Pelton on 3/5/16.
//

#ifndef crufterly_fileblock_h
#define crufterly_fileblock_h

#include <cstdint>
#include <ostream>
#include <sstream>
#include <limits>
//#include <atomic>


namespace bd
{

///////////////////////////////////////////////////////////////////////////////
///   \brief Describes a block in the IndexFile.
/// 
///   All values in the FileBlock struct are initialized to 0 by the c'tor.
/// 
///////////////////////////////////////////////////////////////////////////////
struct FileBlock
{

  FileBlock()
      : block_index{ 0 }
      , data_offset{ 0 }
      , voxel_dims{ 0 }
      , world_oigin{ 0 }
      , empty_voxels{ 0 }
      , min_val{ std::numeric_limits< decltype(min_val) >::max() }
      , max_val{ std::numeric_limits< decltype(max_val) >::lowest() }
      , avg_val{ 0.0 }
      , total_val{ 0.0 }
      , is_empty{ 0 }
  { }


  FileBlock(const FileBlock& other) = default;
  std::string to_string() const;


  uint64_t block_index;    ///< The 1D idx of this block (derived from the i,j,k block-grid coordinates).
  uint64_t data_offset;    ///< Offset into the raw file that the block data starts.
  uint64_t voxel_dims[3];  ///< Dimensions of this block in voxels.
  double world_oigin[3];   ///< Center coordinates within canonical cube.
  uint64_t empty_voxels;   ///< Number of empty (ie, irrelevent) voxels in this block.
  double min_val;          ///< The min value found in this block.
  double max_val;          ///< The largest value found in this block.
  double avg_val;          ///< Average value within this block.
  double total_val;        ///< The total value of the block voxels.
  uint32_t is_empty;       ///< If this block is empty or not.

}; // struct FileBlock

std::ostream& operator<<(std::ostream&, const FileBlock&);

} // namespace bd

#endif //! crufterly_fileblock_h
