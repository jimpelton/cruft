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

/// \brief Parses a 1d scalar transfer function in a text file
/// into a vector of OpacityKnot.
/// \param filename The path to the text file that has the scalar transfer function.
/// \return A vector with the knots from the function
/// \throws std::runtime_error If the file could not be parsed.
/// \throws std::ifstream::failure If there was a problem reading the file.
std::vector<OpacityKnot>
load1dtScalar(std::string const &filename);

} // namespace bd

#endif // ! bd_transferfunction_h
