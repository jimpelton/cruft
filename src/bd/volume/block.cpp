#include <bd/volume/block.h>

#include <bd/log/gl_log.h>
#include <bd/util/util.h>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <sstream>
#include <glm/gtc/matrix_transform.hpp>


namespace bd
{



///////////////////////////////////////////////////////////////////////////////
Block::Block(const glm::u64vec3& ijk, const glm::vec3 &dims, const FileBlock &fb)
  : m_fb{ fb }
  , m_ijk{ ijk }
  , m_transform{ 1.0f }  // identity matrix
  , m_tex{ bd::Texture::Target::Tex3D }
{
  glm::vec3 wld_pos{ fb.world_oigin[0], fb.world_oigin[1], fb.world_oigin[2] };

  glm::mat4 s{ glm::scale(glm::mat4{ 1.0f }, dims) };
  glm::mat4 t{ glm::translate(glm::mat4{ 1.0f }, wld_pos) };

  m_transform = t * s;

}


///////////////////////////////////////////////////////////////////////////////
Block::~Block()
{
}


///////////////////////////////////////////////////////////////////////////////
const FileBlock&
Block::fileBlock() const
{
  return m_fb;
}


///////////////////////////////////////////////////////////////////////////////
const glm::u64vec3&
Block::ijk() const
{
  return m_ijk;
}


///////////////////////////////////////////////////////////////////////////////
void
Block::ijk(const glm::u64vec3& ijk)
{
  m_ijk = ijk;
}


///////////////////////////////////////////////////////////////////////////////
bool
Block::empty() const
{
  return m_empty;
}


///////////////////////////////////////////////////////////////////////////////
void
Block::empty(bool b)
{
  m_empty = b;
}


///////////////////////////////////////////////////////////////////////////////
glm::vec3
Block::origin() const
{
  return { m_fb.world_oigin[0], m_fb.world_oigin[1], m_fb.world_oigin[2] };
}


///////////////////////////////////////////////////////////////////////////////
double
Block::avg() const
{
  return m_fb.avg_val;
}


///////////////////////////////////////////////////////////////////////////////
bd::Texture&
Block::texture()
{
  return m_tex;
}


///////////////////////////////////////////////////////////////////////////////
glm::mat4&
Block::transform()
{
  return m_transform;
}



///////////////////////////////////////////////////////////////////////////////
std::string
Block::to_string() const
{
  std::stringstream ss;
  ss << "{ ijk: ("
      << m_ijk.x << ',' << m_ijk.y << ',' << m_ijk.z << "),\n"
      "Origin: ("
          << m_fb.world_oigin[0]
          << ',' << m_fb.world_oigin[1]
          << ',' << m_fb.world_oigin[2] << "),\n"
      "Empty: " << (empty() ? "True" : "False") << "\n"
      "Texture: " << m_tex << " }";

  return ss.str();
}

std::ostream&
operator<<(std::ostream& os, const Block& b)
{
  return os << b.to_string();
}

glm::u64vec3
Block::voxel_extent() const
{
  return { m_fb.voxel_dims[0], m_fb.voxel_dims[1], m_fb.voxel_dims[2] };
}
} // namespace bd


