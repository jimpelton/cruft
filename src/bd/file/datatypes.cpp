#include <bd/file/datatypes.h>

namespace bd
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

  { "float", DataType::Float }
};

std::string
to_string(bd::DataType type)
{

  // convert data type to string.
  auto cit = std::find_if(DataTypesMap.begin(), DataTypesMap.end(),
      [type](std::pair<std::string, DataType> p)
      {
        return p.second == type;
      });

  if (cit != DataTypesMap.end()) {
    return (cit->first).c_str();
  } else {
    return "unknown";
  }

}

} /* namespace bd */


