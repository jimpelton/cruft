//
// Created by jim on 5/31/16.
//

#ifndef bd_valuerangefilter_h
#define bd_valuerangefilter_h

/// \brief Test if a value is within given range.
template<typename Ty>
class ValueRangeFunction
{
public:
  ValueRangeFunction(Ty min, Ty max)
      : m_min{ min }
      , m_max{ max }
  { }

  ~ValueRangeFunction() {}

  /// \brief return true if val between m_min and m_max.
  bool operator()(Ty val) {
    return val >= m_min && val <= m_max;
  }

private:
  Ty m_min, m_max;
};



#endif //bd_valuerangefilter_h
