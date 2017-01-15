//
// Created by jim on 1/6/17.
//

#ifndef bd_blockloader_h
#define bd_blockloader_h

#include <bd/volume/block.h>
#include <bd/datastructure/blockingqueue.h>

#include <string>
#include <atomic>

namespace bd
{

struct BLThreadData
{
  size_t maxGpuBlocks;
  size_t maxCpuBlocks;
  // x, y dims of volume slab
  size_t slabDims[2];
  std::string filename;
};

/// Threaded load block data from disk. Blocks to load are put into a queue by
/// a thread. 
class BlockLoader
{
public:

  BlockLoader();


  ~BlockLoader();


  int
  operator()(BLThreadData const &params);


  void
  pushLoadQueue(Block *b);


  void
  stop();


  void
  queueBlock(Block *block);


private:

  Block* 
  waitPopLoadQueue();

  void
  fillBlockData(Block *b, std::istream *infile, size_t vX, size_t vY) const;

  std::queue<Block *> m_loadQueue;
  std::shared_ptr<Block*> m_gpuLoadQueue;
  std::atomic_bool m_stopThread;
  std::mutex m_mutex;
  std::condition_variable_any m_wait;

}; // class BlockLoader

}
#endif //! bd_blockloader_h
