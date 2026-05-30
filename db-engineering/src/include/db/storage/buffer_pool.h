#pragma once

#include <list>
#include <unordered_map>

#include "db/common/config.h"
#include "db/storage/page.h"

namespace db {

class DiskManager;

/// BufferPoolManager — 管理内存中的页面缓冲池。
///
/// 职责：
///   1. 从缓冲池中获取页面（如果不在内存中则从磁盘加载）
///   2. 跟踪脏页并在驱逐时写回磁盘
///   3. 使用 LRU 策略驱逐页面
///   4. 钉住页面以防止被驱逐
class BufferPoolManager {
 public:
  explicit BufferPoolManager(size_t pool_size, DiskManager *disk_manager);
  ~BufferPoolManager();

  DB_DISALLOW_COPY_AND_MOVE(BufferPoolManager);

  /// 获取指定页号的页面（若未在内存则从磁盘加载）
  /// @param pin 是否钉住页面
  auto FetchPage(page_id_t page_id, bool pin = true) -> Page *;

  /// 标记页面为脏页（需要写回磁盘）
  void MarkDirty(page_id_t page_id);

  /// 取消钉住页面（减少钉住计数）
  bool UnpinPage(page_id_t page_id, bool is_dirty = false);

  /// 强制将指定页面写回磁盘
  bool FlushPage(page_id_t page_id);

  /// 将所有脏页写回磁盘
  void FlushAllPages();

  /// 分配一个新的页面
  auto NewPage(page_id_t *page_id) -> Page *;

  /// 删除指定页面（回收页号和帧）
  bool DeletePage(page_id_t page_id);

  /// 获取缓冲池大小
  [[nodiscard]] auto GetPoolSize() const -> size_t { return pool_size_; }

 private:
  /// LRU 替换器 — 跟踪页面使用顺序
  struct LRUReplacer {
    auto Victim() -> frame_id_t;
    void Pin(frame_id_t frame_id);
    void Unpin(frame_id_t frame_id);

    std::list<frame_id_t> lru_list_;
    std::unordered_map<frame_id_t, std::list<frame_id_t>::iterator> map_;
  };

  size_t pool_size_;                       // 缓冲池大小（帧数）
  DiskManager *disk_manager_;              // 磁盘管理器（不拥有）
  std::unordered_map<page_id_t, frame_id_t> page_table_; // 页号 → 帧号映射
  Page *pages_{nullptr};                   // 页帧数组
  bool *dirty_flags_{nullptr};             // 脏页标记数组
  int32_t *pin_counts_{nullptr};           // 钉住计数数组
  LRUReplacer replacer_;                   // LRU 替换策略
  frame_id_t next_free_frame_{0};          // 下一个空闲帧
};

}  // namespace db
