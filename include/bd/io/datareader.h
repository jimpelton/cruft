#ifndef cruft_datareader_h__
#define cruft_datareader_h__


#include <bd/log/logger.h>

#include <limits>
#include <fstream>
#include <memory>
#include <cmath>
#include <cstdint>
#include <algorithm>

namespace bd
{
/**
 * \brief Read data and convert from ExternTy to InternTy.
 * Optionally, normalize to [0..1] range.
 * \tparam ExternTy Type read from io.
 * \tparam InternTy Type data is converted to.
 */
template <typename ExternTy, typename InternTy>
class DataReader
{
public:

  DataReader()
    : m_data(nullptr)
      , m_max(std::numeric_limits<InternTy>::lowest()) // Man, I hope InternTy has a numeric_limit!
      , m_min(std::numeric_limits<InternTy>::max())
      , haveOwnership(true)
  {
  }

  ~DataReader()
  {
    if (m_data != nullptr && haveOwnership)
      delete [] m_data;
  }

  /**
  * \brief Load the raw io at path \c imagepath. The external representation
  * in the io is expected to be ExternTy. The data is converted to InternTy.
  *
  *  Note: constraint classes do not exist to prevent incompatible types so it
  *  is assumed that a natural conversion already exists between
  *  ExternalTy and InternalTy.
  *
  * \param imagepath The path to the raw io to open.
  * \param width The width in voxels of the volume.
  * \param height The height in voxels of the volume.
  * \param depth The depth in voxels of the volume.
  * \param normalize If true, normalize floating point type between [0..1]. Default is true.
  *
  * \return The name of the 3D texture as created by glTexImage3D().
  *
  * \throws some exception on failure!
  */
  size_t loadRaw3d(const std::string& imagepath,
                   size_t width, size_t height, size_t depth, bool normalize = true);

  /**
          * \return true if this datareader has ownership of the data else false.
          */
  bool
  hasOwnership() const
  {
    return haveOwnership;
  }

  /**
  * \brief The caller takes ownership of the data.
  *
  * The caller takes ownership of the data in the sense that when this DataReader
  * is deleted, the data will not be dealocated at that time, and the new owner takes
  * responsibility for that (messed up, I know). This can only be called once, after the
  * initial call it returns nullptr.
  *
  * If this datareader does not have ownership, or no data has been read, returns a nullptr,
  *
  * \return a pointer to the data, or nullptr
  */
  InternTy*
  takeOwnership()
  {
    if (haveOwnership) {
      haveOwnership = false;
      return m_data;
    } else {
      return nullptr;
    }
  }

  /**
  * \brief Get pointer to the array of converted data or nullptr.
  *
  * Returns nullptr if no ownership, or if no data has been read yet.
  *
  * \return A pointer to the data, or nullptr.
  */
  InternTy*
  data_ptr() const
  {
    if (haveOwnership)
      return m_data;
    else
      return nullptr;
  }

  /** \brief Returns the min value of the converted data. */
  InternTy
  min() const
  {
    return m_min;
  }

  /** \brief Returns the max value of the converted data. */
  InternTy
  max() const
  {
    return m_max;
  }

  /** \brief Returns fileSize/(width*height*depth). */
  size_t
  numVoxels() const
  {
    return m_numVoxels;
  }

private:
  /** \brief calculate and return io size in bytes */
  size_t volsize(std::filebuf* pbuf);

  /** \brief compute and return fabs(m_min) */
  InternTy shiftAmt();

  /** \brief normalize \c image to between 0..1 */
  void normalize_copy(const ExternTy* image, InternTy* internal);

  /** \brief find min and max values in image */
  void minMax(const ExternTy* image);

private:
  InternTy* m_data;
  InternTy m_max;
  InternTy m_min;

  size_t m_numVoxels;

  bool haveOwnership;
};

template <typename ExternTy, typename InternTy>
size_t
DataReader<ExternTy, InternTy>::loadRaw3d
(
  const std::string& imagepath,
  size_t width,
  size_t height,
  size_t depth,
  bool normalize
)
{
  using std::ifstream;
  using std::filebuf;

  //gl_log("Opening %s, normalize=%d", imagepath.c_str(), normalize);
  Info() << "Opening " << imagepath.c_str() << ", normalize=" << normalize;

  ifstream rawfile;
  rawfile.exceptions(ifstream::failbit | ifstream::badbit);
  try {
    rawfile.open(imagepath, ifstream::in | ifstream::binary);
  } catch (std::exception& err) {
    Err() << "Could not open file " << imagepath << "Message was: " << err.what();
    return 0;
  }

  filebuf* pbuf = rawfile.rdbuf();
  size_t szbytes = volsize(pbuf);
  m_numVoxels = szbytes / sizeof(ExternTy);
  if (m_numVoxels < width * height * depth) {
    Err() << "File size does not jive with the given dimensions and/or data type.\n"
      "\tActual Size: " << szbytes << " bytes (= " << m_numVoxels << " voxels)\n"
      "\tYou gave dimensions (WxHxD): " << width << "x" << height << "x" << depth;
    return 0;
  }

  if (m_numVoxels > width * height * depth) {
    Info() << "File size is larger than given dimensions. Reading anyway.";
  }

  Info() << "Reading " << szbytes << " bytes (=" << m_numVoxels << " voxels) WxHxD: " <<
      width << "x" << height << "x" << depth;

  // read io buffer
  char* raw = new char[szbytes];
  pbuf->sgetn(raw, szbytes);
  rawfile.close();

  // begin pre-processing data.
  ExternTy* image = (ExternTy *)raw;
  minMax(image);
  if (normalize) {
    InternTy* internal = new InternTy[m_numVoxels];
    normalize_copy(image, internal);

    m_data = internal;

    delete [] raw;
    raw = nullptr;
  } else {
    InternTy* internal = new InternTy[m_numVoxels];
    for(size_t i{ 0 }; i < m_numVoxels; ++i) {
      internal[i] = static_cast<InternTy>(raw[i]);
    }

    m_data = internal;
    delete [] raw;
    raw = nullptr;
  }

  return m_numVoxels;
}

template <typename ExternTy, typename InternTy>
size_t
DataReader<ExternTy, InternTy>::volsize
(
  std::filebuf* pbuf
)
{
  pbuf->pubseekpos(0);
  size_t szbytes = pbuf->pubseekoff(0, std::istream::end, std::istream::in);
  pbuf->pubseekpos(0);
  return szbytes;
}

template <typename ExternTy, typename InternTy>
InternTy
DataReader<ExternTy, InternTy>::shiftAmt
(
)
{
  InternTy amt = InternTy();
  if (m_min < 0.0f) {
    amt = std::fabs(m_min);
    m_max += amt;
    m_min = 0.0f;
  }
  return amt;
}

template <typename ExternTy, typename InternTy>
void
DataReader<ExternTy, InternTy>::minMax
(
  const ExternTy* image
)
{
  Info() << "Calculating min and max.";

  double avg{ 0.0 };
  for (size_t idx = 0; idx < m_numVoxels; ++idx) {
    InternTy d{ static_cast<InternTy>(image[idx]) };
    m_max = std::max<InternTy>(m_max, d);
    m_min = std::min<InternTy>(m_min, d);
    avg += d;
  }
  avg /= m_numVoxels;

  Info() << "Max: " << m_max << " Min: " << m_min << "Mean: " << avg;
}

template <typename ExternTy, typename InternTy>
void
DataReader<ExternTy, InternTy>::normalize_copy
(
  const ExternTy* image,
  InternTy* internal
)
{
  Info() << "Normalizing data";

  double min = std::numeric_limits<InternTy>::max();
  double max = std::numeric_limits<InternTy>::lowest();
  
  double avg{ 0.0 };
  const double diff{ m_max - m_min };

  for (size_t idx{ 0 }; idx < m_numVoxels; ++idx) {
    InternTy val = static_cast<InternTy>(image[idx]);
    double d{ (val - m_min) / diff };
    max = std::max<InternTy>(max, d);
    min = std::min<InternTy>(min, d);
    internal[idx] = d;
    avg += d;
  }

  m_max = max;
  m_min = min;
  avg /= m_numVoxels;

  Info() << "Max: "<< m_max << " Min: " << m_min << "Mean: " << avg;
}
} // namespace bd

#endif // cruft_datareader_h__


