#include <gtest/gtest.h>

#include "db/common/config.h"
#include "db/common/error.h"
#include "db/common/macros.h"
#include "db/type/type.h"

namespace db {

// ============================================================
// 基础配置测试
// ============================================================

TEST(ConfigTest, PageSize) { ASSERT_EQ(PAGE_SIZE, 4096); }

TEST(ConfigTest, InvalidPageId) { ASSERT_EQ(INVALID_PAGE_ID, -1); }

// ============================================================
// 异常体系测试
// ============================================================

TEST(ErrorTest, DbException) {
  DbException e("test error");
  ASSERT_EQ(std::string(e.what()), "test error");
}

TEST(ErrorTest, IOException) {
  IOException e("io error");
  ASSERT_EQ(std::string(e.what()), "io error");
}

TEST(ErrorTest, PageException) {
  PageException e("page error");
  ASSERT_EQ(std::string(e.what()), "page error");
}

// ============================================================
// 类型系统测试
// ============================================================

TEST(TypeTest, TypeName) {
  ASSERT_EQ(TypeName(TypeId::INTEGER), "INTEGER");
  ASSERT_EQ(TypeName(TypeId::VARCHAR), "VARCHAR");
  ASSERT_EQ(TypeName(TypeId::INVALID), "INVALID");
}

TEST(TypeTest, TypeSize) {
  ASSERT_EQ(TypeSize(TypeId::BOOLEAN), 1);
  ASSERT_EQ(TypeSize(TypeId::INTEGER), 4);
  ASSERT_EQ(TypeSize(TypeId::BIGINT), 8);
  ASSERT_EQ(TypeSize(TypeId::DOUBLE), 8);
  ASSERT_EQ(TypeSize(TypeId::VARCHAR), 0);
}

TEST(TypeTest, IsNumeric) {
  ASSERT_TRUE(IsNumeric(TypeId::INTEGER));
  ASSERT_TRUE(IsNumeric(TypeId::DOUBLE));
  ASSERT_FALSE(IsNumeric(TypeId::BOOLEAN));
  ASSERT_FALSE(IsNumeric(TypeId::VARCHAR));
}

TEST(TypeTest, IsInteger) {
  ASSERT_TRUE(IsInteger(TypeId::INTEGER));
  ASSERT_TRUE(IsInteger(TypeId::BIGINT));
  ASSERT_FALSE(IsInteger(TypeId::FLOAT));
  ASSERT_FALSE(IsInteger(TypeId::VARCHAR));
}

}  // namespace db
