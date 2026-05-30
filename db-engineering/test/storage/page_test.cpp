#include <gtest/gtest.h>

#include <cstring>

#include "db/storage/page.h"

namespace db {

// ============================================================
// Page 基础功能测试
// ============================================================

TEST(PageTest, DefaultState) {
  Page page;

  // 默认页号应为 INVALID
  ASSERT_EQ(page.GetPageId(), INVALID_PAGE_ID);
  ASSERT_EQ(page.GetPageType(), PageType::INVALID);
  ASSERT_EQ(page.GetSize(), PAGE_SIZE);
}

TEST(PageTest, SetPageId) {
  Page page;
  page.SetPageId(42);
  ASSERT_EQ(page.GetPageId(), 42);
}

TEST(PageTest, SetPageType) {
  Page page;
  page.SetPageType(PageType::TABLE_PAGE);
  ASSERT_EQ(page.GetPageType(), PageType::TABLE_PAGE);
}

TEST(PageTest, ResetClearsData) {
  Page page;

  // 先设置一些值再重置
  page.SetPageId(42);
  page.SetPageType(PageType::TABLE_PAGE);
  ASSERT_EQ(page.GetPageId(), 42);

  // 重置
  page.Reset();

  // 验证头部字段被重新初始化
  ASSERT_EQ(page.GetPageId(), INVALID_PAGE_ID);
  ASSERT_EQ(page.GetPageType(), PageType::INVALID);
  ASSERT_EQ(page.GetHeader()->free_space_offset, PAGE_SIZE);
  ASSERT_EQ(page.GetHeader()->record_count, 0);
  ASSERT_EQ(page.GetHeader()->next_page_id, INVALID_PAGE_ID);
  ASSERT_EQ(page.GetHeader()->prev_page_id, INVALID_PAGE_ID);

  // 验证数据体部分（头部之后）全部归零
  auto *data = page.GetData();
  for (size_t i = page.GetBodyStart(); i < PAGE_SIZE; ++i) {
    ASSERT_EQ(data[i], 0) << "offset " << i << " not zero";
  }
}

TEST(PageTest, PageHeaderSize) {
  // PageHeader 必须能放进一页
  ASSERT_LT(sizeof(PageHeader), PAGE_SIZE);
}

TEST(PageTest, BodyStart) {
  Page page;
  // 正文起始位置 == sizeof(PageHeader)
  ASSERT_EQ(page.GetBodyStart(), sizeof(PageHeader));
}

}  // namespace db
