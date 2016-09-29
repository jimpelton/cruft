//
// Created by Jim Pelton on 9/8/16.
//

#include <bd/volume/transferfunction.h>
#include <bd/log/logger.h>

#include <vector>
#include <memory>
#include <fstream>
#include <cassert>

namespace bd
{

OpacityTransferFunction::OpacityTransferFunction()
  : TransferFunction{ }
{
}

OpacityTransferFunction::OpacityTransferFunction(std::string const &filePath)
{
  load(filePath);
}

void
OpacityTransferFunction::load(std::string const &filename)
{
  bd::Dbg() << "Reading 1dt formatted scalar transfer function: " << filename;

  _knots.clear();

  size_t lineNum{ 0 };
  size_t numKnots{ 0 };
  try {
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    file.open(filename.c_str(), std::ifstream::in);
    if (!file.is_open()) {
      bd::Err() << "Couldn't open transfer function file: " << filename;
    }

    // number of entries/lines in the scalar opacity file.
    file >> numKnots;
    Dbg() << "File says it has " << numKnots << std::endl;
    lineNum++;

    if (numKnots > 8192) {
      bd::Err() << "1D transfer function has > 8192 lines.";
//    throw std::runtime_error("1D transfer function has > 8192 lines.");
    }

    // read rest of file consisting of scalar-opacity pairs
    double s, a;
    while (lineNum <= numKnots && file >> s >> a) {
      _knots.push_back(OpacityKnot{ s, a });
      lineNum++;
    }

    if (lineNum < numKnots) {
      Err() << "Malformed 1D transfer function file (not enough lines?).";
      throw std::runtime_error("Malformed 1D transfer function file (not enough lines?).");
    } else {
      bd::Info() << "Read " << numKnots << " scalar-opacity knots.";
    }

    file.close();

  } catch (std::ifstream::failure e) {
    bd::Err() << "Problem reading 1D transfer function file: " << e.what();
//    throw e;
  }

}


double
OpacityTransferFunction::interpolate(double v) const
{
//  assert(v <= 1.0);
  OpacityKnot a{ _knots[0] };
  OpacityKnot b{ _knots[1] };

  int i = 1;
  while(i < _knots.size() - 1 && v > b.scalar){
    a = _knots[i];
    ++i;
    b = _knots[i];
  }

  if (b.scalar == v) {
    return b.alpha;
  }

  return a.alpha * (1.0 - v) + b.alpha * v;


//  for (size_t i = 1; i<_knots.size(); ++i) {
//    b = _knots[i];
//
//    if (v==b.scalar) {
//      return b.alpha;
//    } else if (v<b.scalar) {
//       v is between a.scalar and b.scalar, so lerp the alpha value.
//      return a.alpha*( 1.0-v )+b.alpha*v;
//    }
//
//    a = _knots[i];
//  }

//  return 0;
}


} // namespace bd
