//
// Created by Jim Pelton on 9/4/16.
//

#ifndef CRUFTERLY_TRANSFERFUNCTION_H
#define CRUFTERLY_TRANSFERFUNCTION_H

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
#endif // ! CRUFTERLY_TRANSFERFUNCTION_H
