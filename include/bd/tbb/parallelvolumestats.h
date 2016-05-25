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

template<typename Ty>
class ParallelVolumeStats
{
private:
  const Ty* const data;

  std::function<bool(Ty)> isEmpty;

public:
  Ty min_value;
  Ty max_value;
  uint64_t empty_voxels;

  void operator()(const tbb::blocked_range<size_t>& r)
  {
    const Ty* const a{ data };

    for (size_t i{ r.begin() }; i!=r.end(); ++i) {

      Ty val{ a[i] };
      
      if (val<min_value) { min_value = val; }
      if (val>max_value) { max_value = val; }

      if (isEmpty(val)) {
        empty_voxels += 1;
      }

    }
  }


  ParallelVolumeStats(ParallelVolumeStats& x, tbb::split)
      : data{ x.data }
      , isEmpty{ x.isEmpty }
      , min_value{ std::numeric_limits<Ty>::max() }
      , max_value{ std::numeric_limits<Ty>::min() }
      , empty_voxels{ 0 }
  {
  }


  void join(const ParallelVolumeStats& y)
  {
    // Reduce to a global minimum and maximum for the volume.
    if (y.min_value<min_value) {
      min_value = y.min_value;
    }
    if (y.max_value>max_value) {
      max_value = y.max_value;
    }

    empty_voxels += y.empty_voxels;
  }


  ParallelVolumeStats(const Buffer<Ty>* b, const std::function<bool(Ty)> &isEmpty)
      : data{ b->ptr() }
      , isEmpty{ isEmpty }
      , min_value{ std::numeric_limits<Ty>::max() }
      , max_value{ std::numeric_limits<Ty>::min() }
      , empty_voxels{ 0 }
  {
  }

}; // class ParallelVolumeStats

} // namespace preproc

#endif // ! minmax_h__
