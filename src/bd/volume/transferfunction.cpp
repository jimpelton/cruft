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

std::unique_ptr<std::vector<OpacityKnot>>
load1dtScalar(std::string const &filename)
{
  bd::Dbg() << "Reading 1dt formatted scalar transfer function.";

  size_t lineNum{ 0 };
  size_t numKnots{ 0 };
  std::unique_ptr<std::vector<OpacityKnot>>
      knots{ new std::vector<OpacityKnot>() };

  std::ifstream file;
  file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  try {
    file.open(filename.c_str(), std::ifstream::in);
    if (!file.is_open()) {
      bd::Err() << "Can't open tfunc file: " << filename;
    }

    // number of entries/lines in the scalar opacity file.
    file >> numKnots;
    lineNum++;

    if (numKnots>8192) {
      bd::Err() << "The 1dt transfer function has " << numKnots
                << " knots but max allowed is 8192)."
                    "Skipping loading the transfer function file.";
      return nullptr;
    }

    // read rest of file consisting of rgba colors
    float s, a;
    while (lineNum<numKnots && file >> s >> a) {
      knots->push_back(OpacityKnot{ s, a });
      lineNum++;
    }
    file.close();

  }
  catch (std::ifstream::failure e) {
//    bd::Err() << "Can't open tfunc file: " << filename
//              << "\n\t Reason: " << e.what();
    throw e;
  }

  return knots;
}


} // namespace bd
