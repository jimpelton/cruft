//
// Created by Jim Pelton on 9/8/16.
//

#include <bd/volume/transferfunction.h>
#include <bd/log/logger.h>

#include <vector>
#include <memory>
#include <fstream>

namespace bd
{

std::vector<OpacityKnot>
load1dtScalar(std::string const &filename)
{
  bd::Dbg() << "Reading 1dt formatted scalar transfer function.";

  size_t lineNum{ 0 };
  size_t numKnots{ 0 };
  std::vector<OpacityKnot> knots;

  std::ifstream file;
  file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  try {
    file.open(filename.c_str(), std::ifstream::in);

    // number of entries/lines in the scalar opacity file.
    file >> numKnots;
    lineNum++;

    if (numKnots>8192) {
      bd::Err() << "1D transfer function has > 8192 lines.";
      throw std::runtime_error("1D transfer function has > 8192 lines.");
    }

    // read rest of file consisting of rgba colors
    float s, a;
    while (lineNum <= numKnots && file >> s >> a) {
      knots.push_back(OpacityKnot{ s, a });
      lineNum++;
    }
    file.close();

  } catch (std::ifstream::failure e) {
    bd::Err() << "Problem reading 1D transfer function file: " << e.what();
    throw e;
  }

  return knots;
}


} // namespace bd
