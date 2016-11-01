//
// Created by jim on 10/3/16.
//

#include <bd/io/fileblock.h>

#include <iomanip>
#include <sstream>

namespace bd
{
///////////////////////////////////////////////////////////////////////////////
std::string
FileBlock::to_string() const
{
  std::stringstream ss;
  ss <<
     "   \"block_" << block_index << "\": {\n"
      "      \"index\":" << block_index << ",\n"
      "      \"data_offset\": " << data_offset << ",\n"
      "      \"voxel_dims\": [" << voxel_dims[0] << ", " << voxel_dims[1] << ", "
     << voxel_dims[2] << "],\n"
      "      \"world_dims\": [" << world_dims[0] << ", " << world_dims[1] << ", "
     << world_dims[2] << "],\n"
      "      \"world_oigin\": [" << world_oigin[0] << ", " << world_oigin[1] << ", "
     << world_oigin[2] << "],\n"
      "      \"min_val\": " << min_val << ",\n"
      "      \"max_val\": " << max_val << ",\n"
      "      \"avg_val\": " << avg_val << ",\n"
      "      \"total_val\": " << total_val << ",\n"
      "      \"rov\": " << std::setprecision(9) << rov << ",\n"
      "      \"empty_voxels\": " << empty_voxels << ",\n"
      "      \"empty\": " << std::boolalpha << is_empty << "\n"
      "   }";

  return ss.str();
}

} //namespace bd
