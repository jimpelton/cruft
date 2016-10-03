//
// Created by Jim Pelton on 4/26/16.
//

#ifndef preproc_volume_h__
#define preproc_volume_h__

#include <glm/glm.hpp>
#include <bd/log/logger.h>

namespace bd
{


class Volume
{

public:
  Volume();


  Volume(glm::u64vec3 const &volDims, glm::u64vec3 const &numBlocks);


  Volume(Volume const &other);


  ~Volume();


  /// \brief Get/Set dimensions of the blocks in this region.
  void
  block_dims(glm::u64vec3 const &dims);


  const glm::u64vec3 &
  block_dims() const;


  glm::u64vec3 &
  block_dims();


  /// \brief Get/Set the number of blocks along each axis.
  glm::u64vec3 const &
  block_count() const;


  void
  block_count(glm::u64vec3 const &);


  uint64_t
  total_block_count() const;


  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get/Set the volume's dimensions in voxels
  const glm::u64vec3 &
  dims() const;


  void
  dims(glm::u64vec3 const &voldims);


  double
  min() const;


  void
  min(double);


  double
  max() const;


  void
  max(double);


  double
  avg() const;


  void
  avg(double);


  double
  total() const;


  void
  total(double);


  uint64_t
  numEmptyVoxels() const;


  void
  numEmptyVoxels(uint64_t);


  glm::u64vec3
  extent() const;

private:

  glm::u64vec3 m_blockDims;  ///< Dimensions of a block in this region, in voxels.
  glm::u64vec3 m_blockCount; ///< Number of equal sized blocks Region is divided into.
  glm::u64vec3 m_volDims;    ///< Volume dimensions in voxels.
  uint64_t m_volEmptyVoxels; ///< Number of non-relevant voxels.
  double m_volMax;           ///< Max value found in volume.
  double m_volMin;           ///< Min value found in volume.
  double m_volAvg;           ///< Avg value found in volume.
  double m_volTot;           ///< Total of all values in volume.


};

} // namespace bd

#endif // ! preproc_volume_h__
