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


void
OpacityTransferFunction::load(std::string const &filename)
{
  bd::Dbg() << "Reading OTF: " << filename;

  _knots.clear();

  size_t lineNum{ 0 };
  size_t numKnots{ 0 };
  try {
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    file.open(filename.c_str(), std::ifstream::in);
    if (!file.is_open()) {
      bd::Err() << "Couldn't open OTF file: " << filename;
    }

    // number of entries/lines in the scalar opacity file.
    file >> numKnots;
    Dbg() << "OTF file says it has " << numKnots << " knots.";
    lineNum++;

    if (numKnots > 8192) {
      bd::Err() << "OTF has > 8192 lines.";
//    throw std::runtime_error("1D transfer function has > 8192 lines.");
    }

    // read rest of file consisting of scalar-opacity pairs
    double s, a;
    while (lineNum <= numKnots && file >> s >> a) {
      _knots.push_back(OpacityKnot{ s, a });
      lineNum++;
    }

    if (lineNum < numKnots) {
      Err() << "Malformed OTF file (not enough knots?).";
//      throw std::runtime_error("Malformed 1D transfer function file (not enough lines?).");
    } else {
      bd::Info() << "Read " << numKnots << " scalar-opacity knots.";
    }

    file.close();

  } catch (std::exception e) {
    bd::Err() << "Problem reading 1D transfer function file: " << e.what();
  }

}


double
OpacityTransferFunction::interpolate(double v) const
{
//  assert(v <= 1.0);
  OpacityKnot prev{ _knots[0] };

  if (v == prev.s) {
    return prev.alpha;
  }

  OpacityKnot next{ _knots[1] };
  size_t i = 1;
  while (i < _knots.size() - 1) {
    if (v > next.s) {
      prev = next;
      i += 1;
      next = _knots[i];
    } else {
      break;
    }
  }

  if (v == next.s) {
    return next.alpha;
  }

  return prev.alpha * ( 1.0 - v ) + next.alpha * v;

}

std::ostream&
operator<<(std::ostream &os, bd::OpacityTransferFunction const &otf)
{
  return os << otf.to_string();
}

std::ostream&
operator<<(std::ostream &os, bd::OpacityKnot const &otf)
{
  return os << otf.to_string();
}

} // namespace bd
