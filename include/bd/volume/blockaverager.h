#ifndef blockaverager_h__
#define blockaverager_h__

#include "fileblock.h"

#include <limits>
#include <algorithm>


struct BlockAverager
{

  void operator()(bd::FileBlock &b)
  {

  }

private:
  bd::FileBlock *m_b;

};


#endif // ! blockaverager_h__