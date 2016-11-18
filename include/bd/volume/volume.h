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


  Volume(glm::u64vec3 const &voxelDims, glm::u64vec3 const &numBlocks);


  Volume(Volume const &other);


  ~Volume();


  /// \brief Get/set the number of voxels in each block.
//  void
//  block_dims(glm::u64vec3 const &dims);


  /// \brief Get/set the number of voxels in each block.
  const glm::u64vec3 &
  block_dims() const;


  /// \brief Get/set the number of voxels in each block.
  glm::u64vec3 &
  block_dims();


  /// \brief Get/Set the number of blocks along each axis.
  glm::u64vec3 const &
  block_count() const;


  /// \brief Get/Set the number of blocks along each axis.
  void
  block_count(glm::u64vec3 const &);


  uint64_t
  total_block_count() const;


  /// \brief Get/Set the volume's dimensions in voxels
  const glm::u64vec3 &
  voxelDims() const;


  /// \brief Set the voxel dimensions of this volume.
  void
  voxelDims(glm::u64vec3 const &voxdims);


  /// \brief Get the voxel dimensions of this volume.
  glm::u64vec3 const &
  voxelDims();


  /// \brief Get/set the volume world dimensions.
  glm::f32vec3 const &
  worldDims() const;

  void
  worldDims(glm::f32vec3 const &);

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


  /// \brief Return the voxel extent covered by the blocks (the blocks may not run from edge to
  /// edge of the volume).
  /// \return x, y, z dims of the voxel extent.
  glm::u64vec3
  blocksExtent() const;

private:

  glm::u64vec3 m_blockDims;  ///< Dimensions of a block in this volume, in voxels.
  glm::u64vec3 m_blockCount; ///< Number of equal sized blocks the volume is divided into.
  glm::u64vec3 m_voxelDims;    ///< Volume dimensions in voxels.
  glm::f32vec3 m_worldDims; ///< Volume dimensions in world coords.
  uint64_t m_volEmptyVoxels; ///< Number of non-relevant voxels.
  double m_volMax;           ///< Max value found in volume.
  double m_volMin;           ///< Min value found in volume.
  double m_volAvg;           ///< Avg value found in volume.
  double m_volTot;           ///< Total of all values in volume.


};

} // namespace bd

#endif // ! preproc_volume_h__
