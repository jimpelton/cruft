//
// Created by Jim Pelton on 8/31/16.
//

#ifndef CRUFTERLY_PARALLELVOXELCLASSIFIER_H
#define CRUFTERLY_PARALLELVOXELCLASSIFIER_H

#include <tbb/tbb.h>

namespace bd
{

template<typename Ty>
class ParallelVoxelClassifier
{
  ParallelVoxelClassifier()
  {

  }

  ParallelVoxelClassifier(ParallelVoxelClassifer const &rhs, tbb::split)
  {

  }

  ~ParallelVoxelClassifier()
  {

  }




}; // class ParallelVoxelClassifier

} // namespace bd

#endif // ! CRUFTERLY_PARALLELVOXELCLASSIFIER_H
