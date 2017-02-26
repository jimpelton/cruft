//
// Created by jim on 2/11/17.
//


#include <bd/io/fileblock.h>
#include <bd/io/indexfile.h>

#include <catch.hpp>


TEST_CASE("I don't know yet")
{
  bd::IndexFile index_file;
  bd::Volume *v{ &index_file.getVolume() };
  v->block_count({3, 3, 3});
  v->voxelDims({256, 256, 256});
  index_file.init(bd::DataType::Character);

  std::vector<bd::FileBlock> const &blocks{ index_file.getFileBlocks() };

  REQUIRE(blocks.size() == 3*3*3);

  bd::FileBlock const *block = &blocks[0];
  auto *thing = block->ijk_index;
  REQUIRE(thing[0] == 0);
  REQUIRE(thing[1] == 0);
  REQUIRE(thing[2] == 0);

  block = &blocks[2];
  thing = block->ijk_index;
  REQUIRE(thing[0] == 2);
  REQUIRE(thing[1] == 0);
  REQUIRE(thing[2] == 0);

}
