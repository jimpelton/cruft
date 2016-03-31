#ifndef block_collection2_h__
#define block_collection2_h__

#include "fileblock.h"

#include <bd/util/util.h>
#include <bd/log/gl_log.h>

#include <glm/glm.hpp>

#include <istream>
#include <iostream>
#include <vector>
#include <algorithm>

namespace bd
{

//////////////////////////////////////////////////////////////////////////////
/// \brief Creates a list of blocks from a large binary raw file. The data type
///        in the binary raw file is supplied by the template parameter.
///
/// \param Ty Data type in the istream this BlockCollection will be
///            generated from.
//////////////////////////////////////////////////////////////////////////////
template<typename Ty>
class BlockCollection2
{

public:

  BlockCollection2();


  BlockCollection2(glm::u64vec3 volDims, glm::u64vec3 numBlocks);


  ~BlockCollection2();


  //////////////////////////////////////////////////////////////////////////////
  /// \brief Marks blocks as empty and uploads GL textures if average is outside of [tmin..tmax].
  /// \param rawFile[in] Volume data set
  /// \param tmin[in] min average block value to filter against.
  /// \param tmax[in] max average block value to filter against.
  ///////////////////////////////////////////////////////////////////////////////
  //TODO: filterblocks takes Functor for thresholding.
  void filterBlocks(std::istream& rawFile, float tmin = 0.0f, float tmax = 1.0f,
    bool normalize = true);


  //////////////////////////////////////////////////////////////////////////////
  /// \brief Read a single block into buffer \c out.
  /// \param b[in]      The FileBlock for the block that will be read.
  /// \param index[in]  i,j,k coords of the block whos data to get.
  /// \param infile[in] The raw data file.
  /// \param out[out]   Destination space for block data.
  //////////////////////////////////////////////////////////////////////////////
  void fillBlockData(const FileBlock& b, std::istream& infile, Ty* out);


  //////////////////////////////////////////////////////////////////////////////
  /// \brief Add a pre-initialized block to this BlockCollection2.
  /// \note  Adds block to non-empty list if block is not empty.
  /// \param b The block to add.
  //////////////////////////////////////////////////////////////////////////////
  void addBlock(const FileBlock& b);


  /////////////////////////////////////////////////////////////////////////////////
  /// \brief Set dimensinos of blocks in voxels
  /////////////////////////////////////////////////////////////////////////////////
  void blockDims(const glm::u64vec3& dims);


  /////////////////////////////////////////////////////////////////////////////////
  /// \brief Get dimensinos of blocks in voxels
  /////////////////////////////////////////////////////////////////////////////////
  glm::u64vec3 blockDims() const;


  /////////////////////////////////////////////////////////////////////////////////
  /// \brief Get the volume's dimensions in voxels
  /////////////////////////////////////////////////////////////////////////////////
  glm::u64vec3 volDims() const;


  /////////////////////////////////////////////////////////////////////////////////
  /// \brief Set the volume's dimensions in voxels
  /////////////////////////////////////////////////////////////////////////////////
  void volDims(const glm::u64vec3& voldims);


  //////////////////////////////////////////////////////////////////////////////
  /// \brief Get the number of blocks along each axis.
  //////////////////////////////////////////////////////////////////////////////
  glm::u64vec3 numBlocks() const;


  //////////////////////////////////////////////////////////////////////////////
  double volMin() const { return m_volMin; }


  //////////////////////////////////////////////////////////////////////////////
  double volMax() const { return m_volMax; }


  //////////////////////////////////////////////////////////////////////////////
  double volAvg() const { return m_volAvg; }

  //////////////////////////////////////////////////////////////////////////////
  const std::vector<std::shared_ptr<FileBlock>>& 
  blocks() const
  {
    return m_blocks;
  }


  //////////////////////////////////////////////////////////////////////////////
  const std::vector<std::weak_ptr<FileBlock>>& 
  nonEmptyBlocks() const
  {
    return m_nonEmptyBlocks;
  }

private:

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Initializes \c nb blocks so that they fit within the extent of \c vd.
  //////////////////////////////////////////////////////////////////////////////
  void initBlocks();


  //////////////////////////////////////////////////////////////////////////////
  /// \brief Compute and save a few stats from provided raw file.
  //////////////////////////////////////////////////////////////////////////////
  void computeVolumeStatistics(std::istream& raw);



  glm::u64vec3 m_blockDims; ///< Dimensions of a block in voxels.
  glm::u64vec3 m_volDims;   ///< Volume dimensions in voxels.
  glm::u64vec3 m_numBlocks; ///< Number of blocks volume is divided into.

  double m_volMax; ///< Max value found in volume.
  double m_volMin; ///< Min value found in volume.
  double m_volAvg; ///< Avg value found in volume.

  std::vector<std::shared_ptr<FileBlock>> m_blocks;
  std::vector<std::weak_ptr<FileBlock>> m_nonEmptyBlocks;

  template<typename RTy=Ty>
  class Reader
  {
  public:

    ///////////////////////////////////////////////////////////////////////////////
    /// \brief Init buffer, trying to target \c target number of blocks.
    ///////////////////////////////////////////////////////////////////////////////
    void initBuffers(int target);

  private:
    RTy *m_buffer;

    BlockCollection2<RTy> *m_bc2;

  };

}; // class BlockCollection2


#include "blockcollection2_impl.hpp"


} // namespace bd

#endif // !block_collection2_h__


