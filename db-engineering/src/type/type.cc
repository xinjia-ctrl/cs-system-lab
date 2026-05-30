#include "db/type/type.h"

namespace db {

auto TypeName(TypeId id) -> std::string_view {
  switch (id) {
    case TypeId::BOOLEAN:
      return "BOOLEAN";
    case TypeId::TINYINT:
      return "TINYINT";
    case TypeId::SMALLINT:
      return "SMALLINT";
    case TypeId::INTEGER:
      return "INTEGER";
    case TypeId::BIGINT:
      return "BIGINT";
    case TypeId::FLOAT:
      return "FLOAT";
    case TypeId::DOUBLE:
      return "DOUBLE";
    case TypeId::VARCHAR:
      return "VARCHAR";
    case TypeId::TIMESTAMP:
      return "TIMESTAMP";
    default:
      return "INVALID";
  }
}

auto TypeSize(TypeId id) -> size_t {
  switch (id) {
    case TypeId::BOOLEAN:
      return 1;
    case TypeId::TINYINT:
      return 1;
    case TypeId::SMALLINT:
      return 2;
    case TypeId::INTEGER:
      return 4;
    case TypeId::BIGINT:
      return 8;
    case TypeId::FLOAT:
      return 4;
    case TypeId::DOUBLE:
      return 8;
    case TypeId::TIMESTAMP:
      return 8;
    case TypeId::VARCHAR:
      return 0; // 变长，需单独存储长度
    default:
      return 0;
  }
}

auto IsNumeric(TypeId id) -> bool {
  return id >= TypeId::TINYINT && id <= TypeId::DOUBLE;
}

auto IsInteger(TypeId id) -> bool {
  return id >= TypeId::TINYINT && id <= TypeId::BIGINT;
}

}  // namespace db
