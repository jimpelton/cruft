#ifndef parsedat_h__
#define parsedat_h__

#include <bd/io/datatypes.h>
#include <bd/util/ordinal.h>

#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>

namespace bd
{

struct DatFileData
{
  DatFileData() 
    : rX{ 0 }
    , rY{ 0 }
    , rZ{ 0 }
    , volumeFileName("")
    , dataType{ bd::DataType::Float }
  {
  }

  // resolution
  unsigned int rX;
  unsigned int rY;
  unsigned int rZ;

  // io name of raw io
  std::string volumeFileName;

  // data type of raw io
  bd::DataType dataType;

  std::string 
  to_string() const
  {
    std::stringstream ss;
 
    ss << "{ \"res\":[" << rX << "," << rY << "," << rZ << "],\n"
      "\"filename\":" << volumeFileName << ",\n"
      "\"dataType\":\"" << bd::to_string(this->dataType) << "\"\n}";
    
    return ss.str();
  }
};

std::ostream& 
operator<<(std::ostream&, const DatFileData&);

bool
parseDat(const std::string& datfile, DatFileData& data);

} // namespace bd


#endif // !parsedat_h__


