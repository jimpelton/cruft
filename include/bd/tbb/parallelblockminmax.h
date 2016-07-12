//
// Created by jim on 6/15/16.
//




#ifndef CRUFTERLY_PARALLELBLOCKMINMAX_H_H
#define CRUFTERLY_PARALLELBLOCKMINMAX_H_H

#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

/// \brief Compute the min and max values for each block
///        associated with values in the BlockedRange.
/// \note  For use with parallel_for() in Intel TBB.
template<typename Ty>
class ParallelBlockMinMax
{
public:
  ParallelBlockMinMax(){ }
  ParallelBlockMinMax(const ParallelBlockMinMax &o) { }



private:

};


#endif //CRUFTERLY_PARALLELBLOCKMINMAX_H_H
