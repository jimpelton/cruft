//
// Created by jim on 9/17/16.
//

#include <bd/datastructure/octree.h>

#include <catch.hpp>

TEST_CASE("testit")
{
  bd::Octree<char, glm::u32vec3> *tree{
      new bd::Octree<char, glm::u64vec3>{ {0,0,0}, {50,50,50}, 10, 0};
}