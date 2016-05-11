#ifndef block_h__
#define block_h__

#include <bd/graphics/texture.h>
#include <bd/io/fileblock.h>
//#include <bd/scene/transformable.h>

#include <glm/glm.hpp>

#include <string>
#include <vector>

#include <algorithm>

namespace bd
{

class ShaderProgram;


//////////////////////////////////////////////////////////////////////////
/// \brief Associates volumetric data with a location in world coordinates.
/// A blocks transform can be used to position a set of proxy geometry in
/// 3D world space. The block's texture contains the GL id/name of the 3D
/// texture that should be sampled by the proxy geometry.
//////////////////////////////////////////////////////////////////////////
class Block : public bd::BDObj
{
public:


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Create block (i,j,k) with specified dims and world origin.
  /// \param[in] ijk The block ID (and position with in the grid of blocks).
  /// \param[in] dims The dimensions in data points of this block.
  /// \param[in] origin Center of this block in world coords
  ///            (or whatever coords you want...maybe...sigh...whatevs).
  ///////////////////////////////////////////////////////////////////////////////
  Block(const glm::u64vec3& ijk, const FileBlock &fb);

  virtual ~Block();


  /// \brief Get the ijk location of this block.
  const glm::u64vec3 & ijk() const;
  /// \brief Set the ijk location of this block.
  void ijk(const glm::u64vec3& ijk);


  /// \brief Set/get if this block is marked empty.
//  void empty(bool);
  /// \brief Set/get if this block is marked empty.
  bool empty() const;


  /// \brief Set/get the average value of this here block.
//  void avg(float);
  /// \brief Set/get the average value of this here block.
  double avg() const;

  /// \brief Get the center coordinates of this block.
  glm::vec3 origin() const;

  /// \brief Get the texture assoc'd with this block.
  bd::Texture& texture();


  /// \brief String rep. of this blockeroo.
  virtual std::string to_string() const override;

private:
  FileBlock m_fb;
  glm::u64vec3 m_ijk;   ///< Block's location in block coordinates.
  Texture m_tex;        ///< Texture data assoc'd with this block.
//  glm::vec3 m_origin;   ///< Center of block in world coordinates.
//  bool m_empty;       ///< True if this block was determined empty.
//  float m_avg;          ///< Avg. val. of this block.

}; // class Block

std::ostream& operator<<(std::ostream& os, const Block& b);
} // namespace bd

#endif // !block_h__


