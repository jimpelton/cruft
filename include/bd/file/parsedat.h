#ifndef parsedat_h__
#define parsedat_h__

#include <bd/file/datatypes.h>

#include <string>
#include <fstream>

namespace bd
{

struct DatFileData
{
  DatFileData() :
      rX{ 0 }, rY{ 0 }, rZ{ 0 }, volumeFileName(""), dataType{ bd::DataType::Float }
  {
  }

  // resolution
  unsigned int rX;
  unsigned int rY;
  unsigned int rZ;

  // file name of raw file
  std::string volumeFileName;

  // data type of raw file
  bd::DataType dataType;
};

std::ostream& operator<<(std::ostream&, const DatFileData&);

bool
parseDat(const std::string& datfile, DatFileData& data);

} // namespace bd


#endif // !parsedat_h__


