//
// Created by Jim Pelton on 4/26/16.
//

#include <bd/volume/volume.h>

namespace bd
{

namespace
{
  glm::vec3 
  calculateWorldDims(glm::u64vec3 voxelDims)
  {
    glm::vec3 worldDims{ 0 };

    glm::u64 longest{ voxelDims.x };
    if (longest < voxelDims.y) {
      longest = voxelDims.y;
    }
    if (longest < voxelDims.z) {
      longest = voxelDims.z;
    }

    worldDims.x = voxelDims.x / glm::f32(longest);
    worldDims.y = voxelDims.y / glm::f32(longest);
    worldDims.z = voxelDims.z / glm::f32(longest);
    
    return worldDims;
  }
}

///////////////////////////////////////////////////////////////////////////////
Volume::Volume()
    : Volume({ }, { 1, 1, 1 })
{
}


///////////////////////////////////////////////////////////////////////////////
Volume::Volume(glm::u64vec3 const &voxelDims, glm::u64vec3 const &numBlocks)
    : m_blockDims{ }
    , m_blockCount{ }
    , m_voxelDims{ voxelDims }
    , m_worldDims{ 0, 0, 0 }
    , m_volEmptyVoxels{ 0 }
    , m_volMax{ std::numeric_limits<double>::lowest() }
    , m_volMin{ std::numeric_limits<double>::max() }
    , m_volAvg{ 0.0 }
    , m_volTot{ 0.0 }
{

  glm::u64vec3 nb{ numBlocks };
  if (numBlocks.x == 0 || numBlocks.y == 0 || numBlocks.z == 0) {
    nb.x = nb.y = nb.z = 1;
  }

  m_blockDims = voxelDims / nb;
  m_blockCount = nb;

  m_worldDims = calculateWorldDims(voxelDims);

}


///////////////////////////////////////////////////////////////////////////////
Volume::Volume(Volume const &other)
    : m_blockDims{ other.m_blockDims }
    , m_blockCount{ other.m_blockCount }
    , m_voxelDims{ other.m_voxelDims }
    , m_worldDims{ other.m_worldDims }
    , m_volEmptyVoxels{ other.m_volEmptyVoxels }
    , m_volMax{ other.m_volMax }
    , m_volMin{ other.m_volMin }
    , m_volAvg{ other.m_volAvg }
    , m_volTot{ other.m_volTot }
{
}


///////////////////////////////////////////////////////////////////////////////
Volume::~Volume()
{
}


///////////////////////////////////////////////////////////////////////////////
const glm::u64vec3 &
Volume::block_dims() const
{
  return m_blockDims;
}


///////////////////////////////////////////////////////////////////////////////
//glm::u64vec3 &
//Volume::block_dims()
//{
//  return m_blockDims;
//}


///////////////////////////////////////////////////////////////////////////////
//void
//Volume::block_dims(const glm::u64vec3 &dims)
//{
//  m_blockDims = dims;
//}


///////////////////////////////////////////////////////////////////////////////
const glm::u64vec3 &
Volume::block_count() const
{
  return m_blockCount;
}


///////////////////////////////////////////////////////////////////////////////
void
Volume::block_count(const glm::u64vec3 &nb)
{
  m_blockDims = m_voxelDims / nb;
  m_blockCount = nb;
}


uint64_t
Volume::total_block_count() const
{
  return m_blockCount.x * m_blockCount.y * m_blockCount.z;
}


///////////////////////////////////////////////////////////////////////////////
const glm::u64vec3 &
Volume::voxelDims() const
{
  return m_voxelDims;
}


///////////////////////////////////////////////////////////////////////////////
void
Volume::voxelDims(const glm::u64vec3 &voxdims)
{
  m_worldDims = calculateWorldDims(voxdims);
  m_blockDims = voxdims / m_blockCount;
  m_voxelDims = voxdims;
}


///////////////////////////////////////////////////////////////////////////////
glm::u64vec3 const &
Volume::voxelDims()
{
  return m_voxelDims;
}


void
Volume::worldDims(glm::f32vec3 const &wd)
{
  m_worldDims = wd;
}


glm::f32vec3 const &
Volume::worldDims() const
{
  return m_worldDims;
}


///////////////////////////////////////////////////////////////////////////////
double
Volume::min() const
{
  return m_volMin;
}


///////////////////////////////////////////////////////////////////////////////
void
Volume::min(double min)
{
  m_volMin = min;
}


///////////////////////////////////////////////////////////////////////////////
double
Volume::max() const
{
  return m_volMax;
}


///////////////////////////////////////////////////////////////////////////////
void
Volume::max(double max)
{
  m_volMax = max;
}


///////////////////////////////////////////////////////////////////////////////
double
Volume::avg() const
{
  return m_volAvg;
}


///////////////////////////////////////////////////////////////////////////////
void
Volume::avg(double avg)
{
  m_volAvg = avg;
}


///////////////////////////////////////////////////////////////////////////////
double
Volume::total() const
{
  return m_volTot;
}


///////////////////////////////////////////////////////////////////////////////
void
Volume::total(double t)
{
  m_volTot = t;
}


///////////////////////////////////////////////////////////////////////////////
void
Volume::numEmptyVoxels(uint64_t empty)
{
  m_volEmptyVoxels = empty;
}


///////////////////////////////////////////////////////////////////////////////
uint64_t
Volume::numEmptyVoxels() const
{
  return m_volEmptyVoxels;
}


///////////////////////////////////////////////////////////////////////////////
glm::u64vec3
Volume::blocksExtent() const
{
  return m_blockCount * m_blockDims;  // component-wise multiply
}


} // namespace bd
