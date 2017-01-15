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

Block *
removeLastInvisibleBlock(std::list<Block *> &list)
{
  auto rend = list.rend();
  std::list<Block *>::reverse_iterator not_vis =
      std::find_if(list.rbegin(), rend,
                   [](Block *be) -> bool {
                     return ( be->status() & Block::VISIBLE ) == 0;
                   });

  if (not_vis == rend) {
    // no non-visible blocks in the cpu queue
    // (only happens if size gpu == size cpu)
    return nullptr;
  }

  auto byebye = list.erase(( not_vis.base()-- ));
  return *byebye;
}


bool
isInList(Block *b, std::list<Block*> &list) {
  auto end = list.end();
  auto found = std::find(list.begin(), end, b);
  return found != end;
}

int
BlockLoader::operator()(BLThreadData const &data)
{
  std::ifstream raw{ data.filename, std::ios::binary };
  if (!raw.is_open()) {
    return -1;
  }

  // GPU blocks have valid pixelData ptrs and valid texture ptrs.
  std::list<Block *> gpu;
  // CPU blocks have valid pixelData ptrs, and may have valid texture ptrs.
  // (if they are visible).
  std::list<Block *> cpu;
  std::vector<Texture *> texs;
  std::vector<char *> buffers;

  while (!m_stopThread) {

    // get a block marked as visible
    Block *b{ waitPopLoadQueue() };

    if (!b) {
      continue;
    }

    if (! isInList(b, gpu)) {
      // Not in GPU, check the CPU cache
      if (! isInList(b, cpu)) {
        // Not in CPU, or GPU
        // b must be loaded to CPU.
        char *pixData{ nullptr };
        if (cpu.size() == data.maxCpuBlocks) {
          // Cpu full, evict a non-visible block
          Block *notvis{ removeLastInvisibleBlock(cpu) };
          if (notvis) {
            pixData = notvis->pixelData();
            notvis->pixelData(nullptr);
          }
        } else {
          pixData = buffers.back();
          buffers.pop_back();
        }

        b->pixelData(pixData);
        fillBlockData(b, &raw, data.slabDims[0], data.slabDims[1]);
        cpu.push_front(b);
        // put back to load queue so it can be processed for GPU.
        pushLoadQueue(b);

      } // ! cpu list
      else
      {
        // Is in cpu, but not in GPU
        // Give b a texture so it will be uploaded to GPU
        Texture *tex{ nullptr };
        if (gpu.size() == data.maxGpuBlocks) {

          // no textures, evict non-vis from gpu
          Block *notvis{ removeLastInvisibleBlock(gpu) };
          if (notvis) {
            // nonvis block found, give it a texture
            tex = notvis->texture();
            notvis->texture(nullptr);
          }

        } else {

          tex = texs.back();
          texs.pop_back();

        }

        b->texture(tex);
        gpu.push_front(b);
        //TODO: submitForGpuLoad(b);
      }
    } // ! gpu list
    else
    {

    }
  } // while

  raw.close();
  bd::Dbg() << "Exiting block loader thread.";
  return 0;

} // operator()

bd::Block* 
BlockLoader::waitPopLoadQueue()
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