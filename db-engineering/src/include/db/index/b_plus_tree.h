#pragma once

#include <memory>

#include "db/common/config.h"
#include "db/storage/page.h"

namespace db {

/// BPlusTree — 单线程 B+ 树索引。
///
/// 支持的操作：
///   1. 插入键值对
///   2. 基于键的查询（精确查找）
///   3. 基于键的范围扫描
///   4. 删除键值对
template <typename KeyType, typename ValueType>
class BPlusTree {
 public:
  explicit BPlusTree(page_id_t root_page_id = INVALID_PAGE_ID);
  ~BPlusTree() = default;

  DB_DISALLOW_COPY_AND_MOVE(BPlusTree);

  /// 插入键值对
  void Insert(const KeyType &key, const ValueType &value);

  /// 精确查找键对应的值
  auto GetValue(const KeyType &key) -> std::optional<ValueType>;

  /// 删除键对应的记录
  void Delete(const KeyType &key);

  /// 范围扫描：[left_key, right_key] 之间的所有值
  auto RangeScan(const KeyType &left_key, const KeyType &right_key)
      -> std::vector<ValueType>;

  /// 树的高度
  [[nodiscard]] auto GetHeight() const -> size_t;

  /// 树中的键数量
  [[nodiscard]] auto GetSize() const -> size_t;

 private:
  page_id_t root_page_id_;
  size_t size_{0};

  // TODO(Stage 4): 实现内部节点和叶子节点的结构和操作
};

}  // namespace db
