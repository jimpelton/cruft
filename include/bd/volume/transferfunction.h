//
// Created by Jim Pelton on 9/4/16.
//

#ifndef bd_transferfunction_h
#define bd_transferfunction_h

#include <string>
#include <memory>
#include <vector>

namespace bd
{

/// \brief A knot in an opacity transfer function
struct OpacityKnot
{
  double scalar;
  double alpha;
};

std::unique_ptr<std::vector<OpacityKnot>>
load1dtScalar(std::string const &filename);

} // namespace bd

#endif // ! bd_transferfunction_h
