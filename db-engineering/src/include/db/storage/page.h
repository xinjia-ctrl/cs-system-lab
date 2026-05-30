#pragma once

#include <cstring>
#include <string_view>

#include "db/common/config.h"
#include "db/common/macros.h"

namespace db {

// ============================================================
// Page — 磁盘与内存间传输的基本数据单元
// ============================================================

/// 页类型标识
enum class PageType : uint8_t {
  INVALID = 0,
  TABLE_PAGE,    // 表数据页
  INDEX_PAGE,    // 索引页
  HEADER_PAGE,   // 文件头页
  FREE_LIST_PAGE // 空闲页链表页
};

/// 页面头部（固定在每页开头）
struct PageHeader {
  page_id_t page_id{INVALID_PAGE_ID};
  PageType page_type{PageType::INVALID};
  uint32_t free_space_offset{PAGE_SIZE}; // 空闲空间起始偏移
  uint32_t record_count{0};              // 记录数
  page_id_t next_page_id{INVALID_PAGE_ID};
  page_id_t prev_page_id{INVALID_PAGE_ID};
};

class Page {
 public:
  /// 构造时自动初始化页面头部
  Page() { Reset(); }

  ~Page() = default;

  Page(const Page &) = delete;
  auto operator=(const Page &) -> Page & = delete;

  Page(Page &&) = delete;
  auto operator=(Page &&) -> Page & = delete;

  /// 获取页面原始数据指针
  [[nodiscard]] auto GetData() -> char * { return data_; }
  [[nodiscard]] auto GetData() const -> const char * { return data_; }

  /// 获取页面头部
  [[nodiscard]] auto GetHeader() -> PageHeader * {
    return reinterpret_cast<PageHeader *>(data_);
  }
  [[nodiscard]] auto GetHeader() const -> const PageHeader * {
    return reinterpret_cast<const PageHeader *>(data_);
  }

  /// 获取有效数据起始位置（跳过头部）
  [[nodiscard]] auto GetBodyStart() const -> uint32_t {
    return sizeof(PageHeader);
  }

  /// 获取页面大小
  [[nodiscard]] auto GetSize() const -> size_t { return PAGE_SIZE; }

  /// 重置页面 — 清空数据并重新初始化头部
  void Reset() {
    std::memset(data_, 0, PAGE_SIZE);
    auto *hdr = GetHeader();
    hdr->page_id = INVALID_PAGE_ID;
    hdr->page_type = PageType::INVALID;
    hdr->free_space_offset = static_cast<uint32_t>(PAGE_SIZE);
    hdr->record_count = 0;
    hdr->next_page_id = INVALID_PAGE_ID;
    hdr->prev_page_id = INVALID_PAGE_ID;
  }

  /// 获取页号
  [[nodiscard]] auto GetPageId() const -> page_id_t {
    return GetHeader()->page_id;
  }
  void SetPageId(page_id_t pid) { GetHeader()->page_id = pid; }

  /// 获取页类型
  [[nodiscard]] auto GetPageType() const -> PageType {
    return GetHeader()->page_type;
  }
  void SetPageType(PageType type) { GetHeader()->page_type = type; }

 private:
  /// 原始页面数据缓冲区
  char data_[PAGE_SIZE]{};
};

// PageHeader 必须能放进一页
DB_STATIC_ASSERT(sizeof(PageHeader) < PAGE_SIZE,
                 "PageHeader exceeds page size");

}  // namespace db
