#ifndef transform_h__ 
#define transform_h__

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace bd {

class Transform {
public:
    Transform();
    
    ~Transform();


    ///////////////////////////////////////////////////////////////////////////////
    // \brief Set the position to \c pos.
    ///////////////////////////////////////////////////////////////////////////////
    void origin(glm::vec3 pos);
    

    ///////////////////////////////////////////////////////////////////////////////
    // \brief Adjust position by \c amt
    ///////////////////////////////////////////////////////////////////////////////
    void translate(glm::vec3 amt);


    ///////////////////////////////////////////////////////////////////////////////
    // \brief Set the orientation to \c rot.
    ///////////////////////////////////////////////////////////////////////////////
    void orientation(glm::quat orient);


    ///////////////////////////////////////////////////////////////////////////////
    // \brief Rotate \c amt radians around \c axis.
    ///////////////////////////////////////////////////////////////////////////////
    void rotate(float amt, glm::vec3 axis);


    ///////////////////////////////////////////////////////////////////////////////
    // \brief Accumulates the orientation provided by \c amt 
    ///////////////////////////////////////////////////////////////////////////////
    void rotate(glm::quat amt);


    ///////////////////////////////////////////////////////////////////////////////
    // \brief Scale by vector \c by.
    ///////////////////////////////////////////////////////////////////////////////
    void scale(glm::vec3 by);


    ///////////////////////////////////////////////////////////////////////////////
    // \brief Rotate around vector \c up to look at \c point.
    // \param up Vector to rotate around.
    // \param point Point to look at.
    ///////////////////////////////////////////////////////////////////////////////
    void lookat(glm::vec3 up, glm::vec3 point);


    ///////////////////////////////////////////////////////////////////////////////
    // \brief Update the view if it has changed since last update.
    ///////////////////////////////////////////////////////////////////////////////
    glm::mat4 update(const glm::mat4 &parent);


    ///////////////////////////////////////////////////////////////////////////////
    // \brief Return the 4x4 matrix of this Transform.
    ///////////////////////////////////////////////////////////////////////////////
    const glm::mat4& matrix() const;

    const glm::vec3& origin() const;
    const glm::vec3& scale() const;
    const glm::quat& rotation() const;

private:
    glm::vec3 m_pos;
    glm::quat m_rot;
    glm::vec3 m_scale;

    glm::mat4 m_transform;
    
    bool m_isDirty;
};

} // namespace bd
#endif // !transform_h__
