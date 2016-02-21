#ifndef datatypes_h__
#define datatypes_h__

#include <map>
#include <string>

namespace bd
{
enum class DataType
{
  Integer,
  UnsignedInteger,

  Character,
  UnsignedCharacter,

  Short,
  UnsignedShort,

  Float
};

extern const std::map<std::string, DataType> DataTypesMap;

std::string
to_string(bd::DataType);

} /* namespace bd */

#endif /* ifndef datatypes_h__ */

