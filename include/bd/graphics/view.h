#ifndef view_h__
#define view_h__


#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace bd
{
class View
{
public:

  View();

  ~View();

  void setGLViewport(int x, int y, int w, int h);

  void setPosition(const glm::vec3& pos);

  const glm::vec3& getPosition() const;

  void translate(const glm::vec3& delta);

  void setRotation(const glm::quat& rot);

  const glm::quat& getRotation() const;

  void rotateBy(const glm::quat& delta);

  void rotateTo(const glm::vec3& dest);

  void setPerspectiveProjectionMatrix(float fov_rads, float aspect_rat,
                                      float z_near, float z_far);

  const glm::mat4& getProjectionMatrix() const;

  const glm::mat4& getViewMatrix() const;

  void updateViewMatrix();

private:

  glm::vec3 m_position;

  glm::uvec4 m_viewport;

  glm::quat m_rotation;

  glm::mat4 m_proj;

  glm::mat4 m_view;

  bool m_viewDirty;
};
} // namespace bd

#endif // !view_h__


