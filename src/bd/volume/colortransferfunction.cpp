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

int
ColorTransferFunction::load(std::string const &filename)
{
  bd::Dbg() << "Reading CTF: " << filename;
//  bool success{ false };
  _knots.clear();

  size_t lineNum{ 0 };
  size_t numKnots{ 0 };
  try {
    std::ifstream file;
//    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    file.open(filename.c_str(), std::ifstream::in);
    if (!file.is_open()) {
      bd::Err() << "Couldn't open CTF file: " << filename;
      return -1;
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
      bd::Err() << "Not enough color transfer knots!";
      return -1;
//      throw std::runtime_error("Not enough knots");
    }

    file.close();
//    success = true;
  }
  catch (std::ios_base::failure &e) {
    bd::Err() << "Problem reading CTF file: " << e.what();
  }
  catch (std::runtime_error &e) {
    Err() << "Malformed CTF file: " << e.what();
  }
  catch(std::exception &e) {
    Err() << "Exception caught in " << __func__ << ": " << e.what();
  }
  return numKnots;
}


Color
ColorTransferFunction::interpolate(double v_norm) const
{

  if (v_norm < 0 || v_norm > 1.0) {
    throw std::runtime_error("v_norm out of range.");
  }

  if (_knots.size() == 0){
    return {0, 0, 0};
  }

//  ColorKnot prev{ _knots[0] };

//  if (v == prev.s) {
//    return prev.c;
//  }



  if (v_norm <= _knots[0].s) {
    return _knots[0].c;
  }
  if (v_norm >= _knots.back().s) {
    return _knots.back().c;
  }


  size_t const max_idx{ _knots.size() - 1 };
  size_t idx{ static_cast<size_t>((v_norm * max_idx) + 0.5) };
  if (idx > max_idx) {
    idx = max_idx;
  } else if (idx == 0) {
    //TODO: This maybe is not correct (rewrite your tests, man!)
    idx = 1;
  }

  if (v_norm == _knots[idx].s) {
    return _knots[idx].c;
  }

  auto prev = _knots[idx-1];
  auto next = _knots[idx];

  return { prev.c.r * ( 1.0 - v_norm ) + next.c.r * v_norm,
           prev.c.g * ( 1.0 - v_norm ) + next.c.r * v_norm,
           prev.c.b * ( 1.0 - v_norm ) + next.c.b * v_norm };


//  ColorKnot next{ _knots[1] };
//  size_t i = 1;
//  while (i < _knots.size() - 1) {
//    if (v > next.s) {
//      prev = next;
//      i += 1;
//      next = _knots[i];
//    } else {
//      break;
//    }
//  }
//
//  if (v == next.s) {
//    return next.c;
//  }

//  return { prev.c.r * ( 1.0 - v ) + next.c.r * v,
//           prev.c.g * ( 1.0 - v ) + prev.c.r * v,
//           prev.c.b * ( 1.0 - v ) + prev.c.b * v };

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
