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




ColorTransferFunction::ColorTransferFunction()
  : TransferFunction{ }
{
}

//ColorTransferFunction::ColorTransferFunction(std::string const &filePath)
//{
//  load(filePath);
//}

bool
ColorTransferFunction::load(std::string const &filename)
{
  bd::Dbg() << "Reading CTF: " << filename;
  bool success{ false };
  _knots.clear();

  size_t lineNum{ 0 };
  size_t numKnots{ 0 };
  try {
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    file.open(filename.c_str(), std::ifstream::in);
    if (!file.is_open()) {
      bd::Err() << "Couldn't open CTF file: " << filename;
      return false;
    }

    // number of entries/lines in the scalar opacity file.
    file >> numKnots;
    Dbg() << "CTF File says it has " << numKnots << " knots.";
    lineNum++;

    if (numKnots > 8192) {
      bd::Warn() << "CTF has > 8192 lines.";
    }

    // read rest of file consisting of scalar-opacity pairs
    double s, r, g, b;
    while (lineNum <= numKnots && file >> s >> r >> g >> b) {
      _knots.push_back(ColorKnot{ s, r, g, b });
      lineNum++;
    }

    if (lineNum < numKnots) {
      throw std::runtime_error("Not enough knots");
    } else {
      bd::Info() << "Read " << numKnots << " scalar-color knots.";
    }

    file.close();
    success = true;
  }
  catch (std::ios_base::failure &e) {
    bd::Err() << "Problem reading CTF file: " << e.what();
  }
  catch (std::runtime_error &e) {
    Err() << "Malformed CTF file: " << e.what();
  }
  return success;
}


Color
ColorTransferFunction::interpolate(double v) const
{
  if (_knots.size() == 0){
    return {0, 0, 0};
  }
//  assert(v <= 1.0);
  ColorKnot prev{ _knots[0] };

  if (v == prev.s) {
    return prev.c;
  }

  ColorKnot next{ _knots[1] };
  int i = 1;
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
    return next.c;
  }

  return { prev.c.r * ( 1.0 - v ) + next.c.r * v,
           prev.c.g * ( 1.0 - v ) + prev.c.r * v,
           prev.c.b * ( 1.0 - v ) + prev.c.b * v };

}

std::ostream&
operator<<(std::ostream &os, bd::ColorTransferFunction const &ctf)
{
  return os << ctf.to_string();
}

std::ostream&
operator<<(std::ostream &os, bd::ColorKnot const &ctf)
{
  return os << ctf.to_string();
}

} // namespace bd
