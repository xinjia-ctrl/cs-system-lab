#pragma once

#include <string>
#include <string_view>

#include "db/common/config.h"

namespace db {

/// 数据库支持的数据类型枚举
enum class TypeId : uint8_t {
  INVALID = 0,
  BOOLEAN,
  TINYINT,
  SMALLINT,
  INTEGER,
  BIGINT,
  FLOAT,
  DOUBLE,
  VARCHAR,
  TIMESTAMP,
};

/// 获取类型名称（用于调试和元数据）
auto TypeName(TypeId id) -> std::string_view;

/// 获取类型的大小（字节）
auto TypeSize(TypeId id) -> size_t;

/// 判断是否为数字类型
auto IsNumeric(TypeId id) -> bool;

/// 判断是否为整数类型
auto IsInteger(TypeId id) -> bool;

}  // namespace db
