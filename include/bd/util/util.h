#ifndef util_h__
#define util_h__

#include <glm/fwd.hpp>

#include <memory>
#include <string>

namespace bd
{
///////////////////////////////////////////////////////////////////////////////
/// \brief Convert hue, saturation and value to red, green, blue.
/// \param h Hue value
/// \param s Saturation value
/// \param v Value value ;)
/// \param[out] Returns the corresponding rgb value for h,s,v.
///////////////////////////////////////////////////////////////////////////////
void hsvToRgb(float h, float s, float v, glm::vec3& rgb);


///////////////////////////////////////////////////////////////////////////////
/// \brief Convert \c x,y,z to 1D coordinate.
///////////////////////////////////////////////////////////////////////////////
size_t to1D(size_t col, size_t row, size_t slab, size_t maxCols, size_t maxRows);
//size_t to1D(size_t x, size_t y, size_t z, size_t maxX, size_t maxY);


///////////////////////////////////////////////////////////////////////////////
/// \brief Compute product of components of \c v.
///////////////////////////////////////////////////////////////////////////////
unsigned long long vecCompMult(glm::u64vec3 v);


///////////////////////////////////////////////////////////////////////////////
/// \brief A convenience method for reading an entire data file into memory.
/// \param[in] dtype The data type as a string
/// \param[in] path The file path
/// \param[in] volx Number of samples in the x dim
/// \param[in] voly Number of samples in the y dim
/// \param[in] volz Number of samples in the z dim
/// \returns An std::unique_ptr to an array of floats.
///////////////////////////////////////////////////////////////////////////////
std::unique_ptr<float []> readVolumeData(const std::string& dtype,
    const std::string& fpath, size_t volx, size_t voly, size_t volz);

std::unique_ptr<float []>
readVolumeData(const std::string& datFilePath, const std::string &filePath);

} // namespace bd

#endif // !util_h__


