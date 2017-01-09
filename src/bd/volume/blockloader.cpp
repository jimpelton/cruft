//
// Created by jim on 1/6/17.
//

#include <bd/volume/blockloader.h>
#include <bd/volume/block.h>
#include <bd/graphics/texture.h>
#include <bd/log/logger.h>
#include <bd/util/util.h>

#include <list>
#include <vector>
#include <fstream>
#include <queue>

namespace bd
{

int
BlockLoader::operator()(BLThreadData const &data)
{
  std::ifstream raw{ data.filename, std::ios::binary };
  if (!raw.is_open()) {
    return -1;
  }

  std::list<Block *> gpu;
  std::list<Block *> cpu;
  std::vector<Texture *> texs;

  while (!m_stopThread) {

    Block *b{ waitNextBlock() };

    if (!b) {
      continue;
    }

    auto found = std::find(gpu.begin(), gpu.end(), b);
    if (found == gpu.end()) {

      // not in GPU, check the CPU cache
      found = std::find(cpu.begin(), cpu.end(), b);
      if (found == cpu.end()) {

        // not in the cpu cache, find a free buffer and load it.
        // if no space in cpu cache, remove the last non-visible Block
        if (cpu.size() == data.maxCpuBlocks) {

          std::list<Block*>::reverse_iterator not_vis = std::find_if(cpu.rbegin(), cpu.rend(),
            [](Block *be) -> bool { return (be->status() & Block::VISIBLE) == 0; });

          if (not_vis == cpu.rend()) {
            // no non-visible blocks in the cpu queue (only happens if size gpu == size cpu)
            continue;
          }

          cpu.erase(not_vis);

        }
      }
      b->pixelData();
      fillBlockData(b, &raw, data.slabDims[0], data.slabDims[1]);

    }
    else {
      // in cpu memory -- just push to loadToGpuQueue
      if (gpu.size() == data.maxGpuBlocks) {
        // remove a non-gpu resident block from the end of the gpu list
        // if no non-visible gpu resident blocks exist, then continue
      }
      //Texture *t{ popTexture(texs) };

      //b->texture(t);
      //        pushGPUReadyBlock(b);

    }


    //fillBlockData(b, &raw);
  }
  //    pushGPUReadyBlock(b);

  raw.close();
  bd::Dbg() << "Exiting block loader thread.";
  return 0;

} // operator()

bd::Block* 
BlockLoader::waitNextBlock()
{
  std::unique_lock<std::mutex> lock(m_mutex);
  while (m_loadQueue.size() == 0 && !m_stopThread) {
    m_wait.wait(m_mutex);
  }
  if (m_stopThread) {
    return nullptr;
  }

  Block * b{ m_loadQueue.front() };
  m_loadQueue.pop();

  return b;
}


void
BlockLoader::queueBlock(Block *b) 
{
  std::unique_lock<std::mutex> lock(m_mutex);
  m_loadQueue.push(b);
}


void
BlockLoader::fillBlockData(Block *b, std::istream *infile, size_t vX, size_t vY) const
{
  char * blockBuffer{ b->pixelData() };

  // block's dimensions in voxels
  glm::u64vec3 const be{ b->voxel_extent() };
  // start element = block index w/in volume * block size
  glm::u64vec3 const start{ b->ijk() * be };
  // block end element = block voxel start voxelDims + block size
  glm::u64vec3 const end{ start + be };

  size_t const blockRowLength{ be.x };
  size_t const sizeType{ to_sizeType(b->texture()->dataType()) };

  // byte offset into file to read from
  size_t offset{ b->fileBlock().data_offset };

  // Loop through rows and slabs of volume reading rows of voxels into memory.
  for (auto slab = start.z; slab < end.z; ++slab) {
    for (auto row = start.y; row < end.y; ++row) {

      // seek to start of row
      infile->seekg(offset);

      // read the bytes of current row
      infile->read(blockBuffer, blockRowLength * sizeType);
      blockBuffer += blockRowLength;

      // offset of next row
      offset = bd::to1D(start.x, row + 1, slab, vX, vY);
      offset *= sizeType;
    }
  }
}

} // namespace bd