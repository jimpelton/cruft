#include <bd/volume/block.h>

#include <bd/log/gl_log.h>
#include <bd/util/util.h>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <sstream>


namespace bd
{



///////////////////////////////////////////////////////////////////////////////
Block::Block(const glm::u64vec3& ijk, const FileBlock &fb)
  : m_fb{ fb }
  , m_ijk{ ijk }
  , m_origin{ fb.world_oigin[0], fb.world_oigin[1], fb.world_oigin[2] }
  , m_transform{ 1.0f }  // identity matrix
  , m_tex{ nullptr }
{

  glm::vec3 wld_dims{ fb.world_dims[0], fb.world_dims[1], fb.world_dims[2] };

  glm::mat4 s{ glm::scale(glm::mat4{ 1.0f }, wld_dims) };

  glm::mat4 t{ glm::translate(glm::mat4{ 1.0f }, m_origin) };

  m_transform = t * s;

}


///////////////////////////////////////////////////////////////////////////////
Block::~Block()
{
}


void
Block::evictYourself()
{

}


void
Block::uploadYourself(Texture::Format format, DataType type, void const *voxels)
{
  m_tex->subImage3D(0, 0, 0,
                    (int)m_fb.voxel_dims[0],
                    (int)m_fb.voxel_dims[1],
                    (int)m_fb.voxel_dims[2],
                    format, type, voxels);

  m_status = m_status | GPU_RES;
  m_status = m_status | CPU_RES;

}


///////////////////////////////////////////////////////////////////////////////
bool
Block::visible() const
{
  return m_isVisible;
}


///////////////////////////////////////////////////////////////////////////////
void
Block::visible(bool v)
{
  m_isVisible = v;
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
  return m_fb.is_empty == 1;
}


///////////////////////////////////////////////////////////////////////////////
glm::vec3 const &
Block::origin() const
{
  return m_origin;
}


///////////////////////////////////////////////////////////////////////////////
double
Block::avg() const
{
  return m_fb.avg_val;
}


///////////////////////////////////////////////////////////////////////////////
bd::Texture const &
Block::texture() const
{
  return *m_tex;
}


///////////////////////////////////////////////////////////////////////////////
void
Block::texture(Texture * tex)
{
  m_tex = tex;
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


///////////////////////////////////////////////////////////////////////////////
size_t 
Block::byteSize() const
{
  return m_fb.data_bytes;
}


///////////////////////////////////////////////////////////////////////////////
int
Block::status() const
{
  return m_status;
}


///////////////////////////////////////////////////////////////////////////////
std::ostream&
operator<<(std::ostream& os, const Block& b)
{
  return os << b.to_string();
}


///////////////////////////////////////////////////////////////////////////////
glm::u64vec3
Block::voxel_extent() const
{
  return { m_fb.voxel_dims[0], m_fb.voxel_dims[1], m_fb.voxel_dims[2] };
}
} // namespace bd


