#ifndef minmax_h__
#define minmax_h__

#include <bd/io/buffer.h>
#include <bd/io/indexfile.h>
#include <bd/volume/volume.h>

#include <tbb/parallel_reduce.h>
#include <tbb/blocked_range.h>

#include <limits>
#include <vector>
#include <functional>

namespace bd
{

/// \brief Simply compute the min/max of the given blocked_range.
/// \note Use this with TBB's parallel_reduce.
template<typename Ty>
class ParallelMinMax
{
private:
  const Ty* const data;

//  std::function<bool(Ty)> isRelevant;

public:
  Ty min_value;
  Ty max_value;
//  uint64_t empty_voxels;

  void operator()(const tbb::blocked_range<size_t>& r)
  {
    const Ty* const a{ data };

    for (size_t i{ r.begin() }; i!=r.end(); ++i) {

      Ty val{ a[i] };
      
      if (val<min_value) { min_value = val; }
      if (val>max_value) { max_value = val; }

//      if (isRelevant(val)) {
//        empty_voxels += 1;
//      }

    }
  }


  ParallelMinMax(ParallelMinMax& x, tbb::split)
      : data{ x.data }
//      , isRelevant{ x.isRelevant }
      , min_value{ std::numeric_limits<Ty>::max() }
      , max_value{ std::numeric_limits<Ty>::lowest() }
//      , empty_voxels{ 0 }
  {
  }


  void join(const ParallelMinMax& y)
  {
    // Reduce to a global minimum and maximum for the volume.
    if (y.min_value<min_value) {
      min_value = y.min_value;
    }
    if (y.max_value>max_value) {
      max_value = y.max_value;
    }

//    empty_voxels += y.empty_voxels;
  }


  ParallelMinMax(const Buffer<Ty>* b /*, const std::function<bool(Ty)> &isRelevant*/)
      : data{ b->ptr() }
//    , isRelevant{ isRelevant }
      , min_value{ std::numeric_limits<Ty>::max() }
      , max_value{ std::numeric_limits<Ty>::lowest() }
//      , empty_voxels{ 0 }
  {
  }

}; // class ParallelMinMax

} // namespace preproc

#endif // ! minmax_h__
