//
// Created by Jim Pelton on 8/24/16.
//

#ifndef parallelhistogram_h__
#define parallelhistogram_h__

#include <bd/io/buffer.h>
#include <tbb/tbb.h>

namespace bd
{
namespace
{

const size_t NUM_BUCKETS{ 1536 };
const size_t MAX_IDX{ NUM_BUCKETS - 1 };

}

template<typename Ty>
class ParallelHistogram
{
public:
  ParallelHistogram(Buffer<Ty> *b, Ty rawmin, Ty rawmax)
      : m_data{ b->ptr() }
        , m_rawmin{ rawmin }
        , m_rawmax{ rawmax }
  {
    m_buckets = new unsigned int[ NUM_BUCKETS ];
  }

  ParallelHistogram(ParallelHistogram<Ty> &o, tbb::split)
      : m_data{ o.m_data }
        , m_rawmin{ o.m_rawmin }
        , m_rawmax{ o.m_rawmax }
  {
    m_buckets = new unsigned int[ NUM_BUCKETS ];
  }

  ~ParallelHistogram() { }

  void
  operator()(tbb::blocked_range<size_t> const &r)
  {
    Ty const rawmin{ m_rawmin };
    Ty const rawmax{ m_rawmax };
    unsigned int * const buckets{ m_buckets };
//    Ty histmin{ m_histMin };
//    Ty histmax{ m_histMax };

    for(size_t i{ r.begin() }; i != r.end(); ++i) {
      Ty val{ m_data[i] };

      // Compute the bucket index.
      long long idx{
          static_cast<long long>((val - rawmin)/(rawmax - rawmin) * MAX_IDX + Ty(0.5)) };

      // Reset idx if it is out of range.
      if (idx < 0)
        idx = 0;
      if (idx > MAX_IDX)
        idx = MAX_IDX;

      // Accumulate frequency.
      buckets[idx] += 1;
    }
  }

  void
  join(ParallelHistogram<Ty> const &rhs)
  {
    // Accumulate histogram frequences from the joinee.
    for(size_t i{ 0 }; i < NUM_BUCKETS; ++i) {
      m_buckets[i] += rhs.m_buckets[i];
    }
  }

  unsigned int
  get(size_t i)
  {
    return m_buckets[i];
  }

private:
  unsigned int *m_buckets; // [NUM_BUCKETS];
  Ty const * const m_data;
  Ty const m_rawmin;
  Ty const m_rawmax;
//  Ty m_histMin;
//  Ty m_histMax;
};



} // namespace bd

#endif // ! parallelhistogram_h__
