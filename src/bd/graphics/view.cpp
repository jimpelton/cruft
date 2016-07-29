#include <bd/graphics/view.h>

#include <GL/glew.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace bd
{
;

View::View()
  : m_position{ 0.0f }
    , m_viewport{ 0 }
    , m_rotation{ }
    , m_proj{ 1.0f }
    , m_view{ 1.0f }
    , m_viewDirty{ true }
{
}


///////////////////////////////////////////////////////////////////////////////
View::~View()
{
}

//void View::setMouseRotation(const glm::vec2& cpos)
//{
//    glm::vec2 dr{ m_cursorPos - cpos };
//
//    glm::quat rotX{ glm::angleAxis<float>(
//        glm::radians(dr.y) * m_mouseSpeed,
//        glm::vec3(1, 0, 0)
//        ) };
//
//    glm::quat rotY{ glm::angleAxis<float>(
//        glm::radians(-dr.x) * m_mouseSpeed,
//        glm::vec3(0, 1, 0)
//        ) };
//
//    m_rotation = (rotX * rotY) * m_rotation;
//
//    m_viewDirty = true;
//}
//
//void View::setCursorPos(const glm::vec2& cpos)
//{
//    m_cursorPos = cpos;
//}


///////////////////////////////////////////////////////////////////////////////
void
View::setGLViewport(int x, int y, int w, int h)
{
  m_viewport = glm::uvec4(x, y, w, h);
  glViewport(x, y, w, h);

  //m_viewDirty = true;
}


///////////////////////////////////////////////////////////////////////////////
void
View::setPosition(const glm::vec3& pos)
{
  m_position = pos;
}


///////////////////////////////////////////////////////////////////////////////
const glm::vec3&
View::getPosition() const
{
  return m_position;
}


///////////////////////////////////////////////////////////////////////////////
void
View::translate(const glm::vec3& delta)
{
  m_position += delta;
  m_viewDirty = true;
}


///////////////////////////////////////////////////////////////////////////////
void
View::setRotation(const glm::quat& rot)
{
  m_rotation = rot;
  m_viewDirty = true;
}


///////////////////////////////////////////////////////////////////////////////
const glm::quat&
View::getRotation() const
{
  return m_rotation;
}


///////////////////////////////////////////////////////////////////////////////
void
View::rotateBy(const glm::quat& delta)
{
  m_rotation = m_rotation * delta;
  m_viewDirty = true;
}

///////////////////////////////////////////////////////////////////////////////
void
View::rotateTo(const glm::vec3& dest)
{
  glm::quat rot = glm::rotation(glm::normalize(m_position), dest);
  m_rotation = m_rotation * rot;
  m_viewDirty = true;
}


///////////////////////////////////////////////////////////////////////////////
void View::setPerspectiveProjectionMatrix(float fov, float aspect_rat,
                                          float z_near, float z_far)
{
  m_proj = glm::perspective(fov, aspect_rat, z_near, z_far);
}


///////////////////////////////////////////////////////////////////////////////
glm::mat4 const&
View::getProjectionMatrix() const
{
  return m_proj;
}


///////////////////////////////////////////////////////////////////////////////
glm::mat4 const &
View::getViewMatrix()
{
  updateViewMatrix();
  return m_view;
}


///////////////////////////////////////////////////////////////////////////////
void
View::updateViewMatrix()
{
  if (m_viewDirty) {
    glm::mat4 tr = glm::translate(glm::mat4(1.0), -(m_position));
    glm::mat4 rot = glm::transpose(glm::toMat4(m_rotation));
    glm::mat4 tr_inv = glm::translate(tr, m_position);

    m_view = tr_inv * rot * tr;

    m_viewDirty = false;
  }
}
} // namespace bd


