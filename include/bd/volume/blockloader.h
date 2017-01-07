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


private:
  bd::BlockingQueue<Block *> m_loadQueue;
  std::atomic_bool m_stopThread;


}; // class BlockLoader

}
#endif //! bd_blockloader_h
