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

  Float,

  Unknown
};

//extern const std::map<std::string, DataType> DataTypesMap;

DataType
to_dataType(const std::string &typeStr);

std::string
to_string(DataType);

} /* namespace bd */

#endif /* ifndef datatypes_h__ */

