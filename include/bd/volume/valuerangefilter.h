//
// Created by jim on 5/31/16.
//

#ifndef CRUFTERLY_VALUERANGEFILTER_H
#define CRUFTERLY_VALUERANGEFILTER_H

/// \brief Test if a value is within given range.
template<typename Ty>
class ValueRangeFilter
{
public:
  ValueRangeFilter(Ty min, Ty max)
      : m_min{ min }
      , m_max{ max }
  { }

  ~ValueRangeFilter() {}

  /// \brief return true if val between m_min and m_max.
  bool operator()(Ty val) {
    return val >= m_min && val <= m_max;
  }

private:
  Ty m_min, m_max;
};



#endif //CRUFTERLY_VALUERANGEFILTER_H
