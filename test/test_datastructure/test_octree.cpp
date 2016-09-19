//
// Created by jim on 9/17/16.
//

#include <bd/datastructure/octree.h>
#include <glm/glm.hpp>
#include <catch.hpp>

namespace
{
  std::array<char,



}



TEST_CASE("testit")
{
  bd::Octree<glm::u32vec3, char> *tree{
      new bd::Octree<glm::u32vec3, char>{ glm::u32vec3{0,0,0}, glm::u32vec3{50,50,50}, 10, 0} };




}