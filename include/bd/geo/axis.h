#ifndef bd_axis_h__
#define bd_axis_h__


#include <bd/scene/transformable.h>
#include <bd/geo/drawable.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>

namespace bd
{
class CoordinateAxis : public Transformable, public IDrawable
{
public:
  static const std::array<glm::vec4, 6> verts;
  static const std::array<glm::vec3, 6> colors;
  static const std::array<unsigned short, 6> elements;
  static const unsigned int vert_element_size = 4;

  CoordinateAxis();

  //    CoordinateAxis(const glm::vec3 &center, const glm::vec3 &dims);
  ~CoordinateAxis()
  {
  }

  virtual void draw() override;
};
} // namespace bd

#endif // !bd_axis_h__


