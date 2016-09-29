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


  bool
  operator==(OpacityKnot const &rhs) const
  {
    return scalar == rhs.scalar &&
        alpha == rhs.alpha;
  }


  bool
  operator!=(OpacityKnot const &rhs) const
  {
    return !( rhs == *this );
  }

}; // struct OpacityKnot

struct Color
{
  double r, g, b;

  bool
  operator==(Color const &rhs) const
  {
    return r == rhs.r &&
        g == rhs.g &&
        b == rhs.b;
  }

  bool
  operator!=(Color const &rhs) const
  {
    return !( rhs == *this );
  }

}; // struct Color


struct ColorKnot
{
  double s;
  Color c;


  bool
  operator==(ColorKnot const &rhs) const
  {
    return s == rhs.s
        && c == rhs.c;
  }


  bool
  operator!=(ColorKnot const &rhs) const
  {
    return !( rhs == *this );
  }

}; // struct ColorKnot


template<class Knot, class Value>
class TransferFunction
{
public:
  TransferFunction(){ }
  ~TransferFunction() { }
  TransferFunction(TransferFunction const &) = default;
  TransferFunction(TransferFunction &&) = default;

  /// \brief Parses a 1d scalar transfer function in a text file
  /// into a vector of OpacityKnot.
  ///
  /// \note The file can use double precision floating pt values.
  ///
  /// \param filename The path to the text file that has the scalar transfer function.
  /// \return A vector with the knots from the function
  /// \throws std::runtime_error If the file could not be parsed.
  /// \throws std::ifstream::failure If there was a problem reading the file.
  virtual void
  load(std::string const &filename) = 0;

  virtual Value
  interpolate(double scalar) const = 0;

  size_t
  getNumKnots() { return _knots.size(); }


  std::vector<Knot> const &
  getKnotsVector() const { return _knots; }

protected:
  std::vector<Knot> _knots;

};

/// \brief Represents an opacity transfer function
///
/// The file should have the number of knots on the first line
/// with the following lines each containing one knot. The knots
/// are pairs of double precision floating point values. The
/// first value is the knot scalar value (normalized!) and
/// the second value is the knot alpha value.
class OpacityTransferFunction
    : public TransferFunction<OpacityKnot, double>
{
public:

  OpacityTransferFunction();
  OpacityTransferFunction(OpacityTransferFunction const &o) = default;
  OpacityTransferFunction(OpacityTransferFunction &&o) = default;

  /// \brief Load a transfer function from the file at \c tfFilePath.
  /// \param tfFilePath  The text file containing the transfer function values.
  OpacityTransferFunction(std::string const &filePath);

  void
  load(std::string const &filename) override;

  double
  interpolate(double v) const override;

}; // class OpacityTransferFunction


/// \brief Represents an color transfer function (color map)
///
/// The file should have the number of knots on the first line
/// with the following lines each containing one knot. The knots
/// are quad tuples of double precision floating point values.
/// Each value is separated by a space. The
/// first value is the knot scalar value (normalized!) and
/// the following three values are the knot red, gree, blue values.
/// The r, g, b values range from 0.0 to 1.0.
class ColorTransferFunction
    : public TransferFunction<ColorKnot, Color>
{
public:
  void load(std::string const & filename) override { }
  Color interpolate(double scalar) const override { return {0,0,0}; }


}; // class ColorTransferFunction

} // namespace bd

#endif // ! bd_transferfunction_h
