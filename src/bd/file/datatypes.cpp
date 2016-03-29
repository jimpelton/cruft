#include <bd/file/datatypes.h>
#include <algorithm>
#include <iostream>
#include <bd/log/gl_log.h>

namespace bd
{

namespace
{
  const std::map<std::string, DataType> DataTypesMap
  {
    { "int", DataType::Integer },
    { "uint", DataType::UnsignedInteger },
    { "unsigned integer", DataType::UnsignedInteger },

    { "char", DataType::Character },
    { "uchar", DataType::UnsignedCharacter },
    { "unsigned char", DataType::UnsignedCharacter },

    { "short", DataType::Short },
    { "ushort", DataType::UnsignedShort },
    { "unsigned short", DataType::UnsignedShort },

    { "float", DataType::Float },

    { "unknown", DataType::Unknown }
  };
} // namespace


bd::DataType
to_dataType(const std::string &typeStr)
{
 
  try {
    return DataTypesMap.at(typeStr);
  } catch (std::out_of_range &e) {
    std::cerr << e.what() << std::endl;
    gl_log_err("%s is unknown datatype!", typeStr.c_str());
    return DataType::Unknown;
  }
}

std::string
to_string(bd::DataType type)
{
  try {
    // convert data type to string.
    auto cit = std::find_if(DataTypesMap.begin(), DataTypesMap.end(),
      [type](std::pair<std::string, DataType> p)
        {
          return p.second == type;
        });

    if (cit != DataTypesMap.end()) {
      return (cit->first).c_str();
    } 
    
    return "unknown";
    

  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return "unknown";
  }
}
} /* namespace bd */


