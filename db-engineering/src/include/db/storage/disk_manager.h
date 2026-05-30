#pragma once

#include <string>

#include "db/common/config.h"
#include "db/common/error.h"

namespace db {

/// DiskManager — 管理数据库文件在磁盘上的读写操作。
///
/// 职责：
///   1. 分配/回收页号
///   2. 从磁盘指定偏移读取页面到内存
///   3. 将脏页写回磁盘
///   4. 管理文件的增长与收缩
class DiskManager {
 public:
  explicit DiskManager(const std::string &db_file);
  ~DiskManager();

  DB_DISALLOW_COPY_AND_MOVE(DiskManager);

  /// 读取指定页号的数据到内存缓冲区
  void ReadPage(page_id_t page_id, char *page_data);

  /// 将内存缓冲区的数据写入磁盘指定页号
  void WritePage(page_id_t page_id, const char *page_data);

  /// 分配一个新的页号（追加文件）
  auto AllocatePage() -> page_id_t;

  /// 回收指定页号
  void DeallocatePage(page_id_t page_id);

  /// 当前文件已分配的页数
  [[nodiscard]] auto GetNumPages() const -> size_t;

  /// 文件总大小（字节）
  [[nodiscard]] auto GetFileSize() const -> size_t;

  /// 关闭并清理
  void Close();

  /// 判断文件是否已打开
  [[nodiscard]] auto IsOpen() const -> bool { return fd_ != -1; }

 private:
  /// 扩展文件到能容纳指定页号的位置
  void GrowFile(page_id_t page_id);

  std::string db_file_;  // 数据库文件路径
  int fd_ = -1;          // 文件描述符
  size_t num_pages_{0};  // 当前文件中的页数
};

}  // namespace db
