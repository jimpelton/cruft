#ifndef parsedat_h__
#define parsedat_h__

#include <bd/file/datatypes.h>
#include <bd/util/ordinal.h>

#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>

namespace bd
{

struct DatFileData
{
  DatFileData() :
    rX{ 0 }, rY{ 0 }, rZ{ 0 }, 
    volumeFileName(""), dataType{ bd::DataType::Float }
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

  std::string 
  to_string() const
  {
    std::stringstream ss;
    
    // convert data type to string.
    auto cit = std::find_if(DataTypesMap.begin(), DataTypesMap.end(),
      [this](std::pair<std::string, DataType> p)
    {
      return p.second == this->dataType;
    });
    
    const char *dataTypeStr = "unknown";
    if (cit != DataTypesMap.end()) {  
      dataTypeStr = (cit->first).c_str();
    }

    ss << "{ Res: " << rX << "x" << rY << "x" << rZ << ", "
      " Filename: " << volumeFileName << ", dataType: " << dataTypeStr << " }";
    
    return ss.str();
  }
};

std::ostream& 
operator<<(std::ostream&, const DatFileData&);

bool
parseDat(const std::string& datfile, DatFileData& data);

} // namespace bd


#endif // !parsedat_h__


