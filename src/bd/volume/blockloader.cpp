//
// Created by jim on 1/6/17.
//

#include <bd/volume/blockloader.h>
#include <bd/volume/block.h>
#include <bd/graphics/texture.h>

#include <list>
#include <vector>
#include <fstream>

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

    Block *b{ m_loadQueue.pop() };

    if (!b) {
      continue;
    }

    auto found = std::find(gpu.begin(), gpu.end(), b);
    if (found == gpu.end()) {
      // not in GPU, check the CPU cache
      found = std::find(cpu.begin(), cpu.end(), b);
      if (found == cpu.end()) {
//        loadFromDisk(b);
//        pushToGpuQueue();
        // not in cpu memory -- load from disk
        // push to loadToGpuQueue.

      } else {
        // in cpu memory -- just push to loadToGpuQueue
        if (gpu.size() == data.maxGpuBlocks) {
          // remove a non-gpu resident block from the end of the gpu list
          // if no non-visible gpu resident blocks exist, then continue
        }
        Texture *t{ popTexture(texs) };

        b->texture(t);
//        pushGPUReadyBlock(b);

      }
    } // else { in gpu memory already, do nothing. }

    char const *data{ b->pixelData() };

    fillBlockData(b, &raw);

//    pushGPUReadyBlock(b);
  }

  raw.close();

  return 0;

} // operator()



} // namespace bd