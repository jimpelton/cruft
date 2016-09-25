#ifndef block_h__
#define block_h__

#include <bd/graphics/texture.h>
#include <bd/io/fileblock.h>

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <algorithm>

namespace bd
{



//////////////////////////////////////////////////////////////////////////
/// \brief Associates volumetric data with a location in world coordinates.
/// A blocks transform can be used to position a set of proxy geometry in
/// 3D world space. The block's texture contains the GL id/name of the 3D
/// texture that should be sampled by the proxy geometry.
//////////////////////////////////////////////////////////////////////////
class Block //: public bd::BDObj
{
public:


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Create block (i,j,k) with specified dims and world origin.
  /// \param[in] ijk The block ID (and position with in the grid of blocks).
  /// \param[in] dims The dimensions in data points of this block.
  /// \param[in] origin Center of this block in world coords
  ///            (or whatever coords you want...maybe...sigh...whatevs).
  Block(glm::u64vec3 const &ijk, glm::vec3 const &dims, FileBlock const &fb);

  virtual ~Block();

  /// \brief Get the FileBlock for this block.
  const FileBlock& fileBlock() const;


  /// \brief Get the ijk location of this block.
  const glm::u64vec3 & ijk() const;
  /// \brief Set the ijk location of this block.
  void ijk(glm::u64vec3 const & ijk);


  /// \brief Set if this block is marked empty.
  void empty(bool);
  /// \brief Get if this block is marked empty.
  bool empty() const;


  /// \brief Get the average value of this here block.
  double avg() const;


  /// \brief Get the center coordinates of this block.
  glm::vec3 const & origin() const;


  /// \brief Get the texture assoc'd with this block.
  bd::Texture& texture();


  /// \brief Get a reference to this blocks model-to-world transform matrix.
  glm::mat4& transform();


  /// \brief Get dimensions of this block in voxels.
  glm::u64vec3 voxel_extent() const;


  /// \brief String rep. of this blockeroo.
  std::string to_string() const;

private:
  FileBlock m_fb;
  glm::u64vec3 m_ijk;    ///< Block's location in block coordinates.
  glm::vec3 m_origin;    ///< This blocks center in world coordinates.
  glm::mat4 m_transform; ///< Block's model-to-world transform matrix.
  Texture m_tex;         ///< Texture data assoc'd with this block.
  bool m_empty;          ///< True if this block was determined empty.

}; // class Block

/// \brief Output a string representation of given Block.
std::ostream& operator<<(std::ostream& os, const Block& b);

} // namespace bd

#endif // !block_h__


