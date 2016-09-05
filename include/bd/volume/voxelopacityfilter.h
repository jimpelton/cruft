//
// Created by Jim Pelton on 8/31/16.
//

#ifndef voxelopacityfilter_h__
#define voxelopacityfilter_h__

#include <bd/volume/transferfunction.h>

#include <limits>
#include <vector>

namespace bd
{

template<typename Ty>
class VoxelOpacityFilter
{

public:
  /// \brief Create a filter based on the given opacity function.
  /// \note Scalars in OpacityKnots should be normalized data values.
  VoxelOpacityFilter(const std::vector<OpacityKnot> &function,
                     double knotMin,
                     double knotMax,
                     const Ty &dataMin,
                     const Ty &dataMax)
      : m_func{ function }
      , m_min{ knotMin }
      , m_max{ knotMax }
      , m_dataMin{ dataMin }
      , m_diff{ static_cast<Ty>(dataMax - dataMin) }
  {
  }

  ~VoxelOpacityFilter()
  {
  }

  bool operator()(Ty const &val) const
  {
    double val_norm{ (val - m_dataMin) / static_cast<double>(m_diff) };
    double a{ alpha(val_norm) };
    return a >= m_min && a < m_max;

  }

  double alpha(double v) const
  {
    OpacityKnot b{ 0, 0 };
    OpacityKnot a{ m_func[0] };
    double alpha{ 0 };
    for(size_t i = 1; i < m_func.size(); ++i) {
      b = m_func[i];

      //TODO: fix unsafe comparison 
      if (v == b.scalar) {
        return b.alpha;
      } else if (v < b.scalar) {
        return a.alpha * (1.0 - v) + b.alpha * v;
      }

      a = m_func[i];
    }

    return alpha;

  }


private:
  const std::vector<OpacityKnot> m_func;
  const double m_min;
  const double m_max;
  const Ty m_dataMin;
  const Ty m_diff;


}; // class VoxelOpacityFilter

} // namespace bd

#endif // ! voxelopacityfilter_h__
