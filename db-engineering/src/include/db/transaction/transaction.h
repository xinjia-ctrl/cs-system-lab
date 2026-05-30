#pragma once

#include <set>

#include "db/common/config.h"

namespace db {

/// 事务隔离级别
enum class IsolationLevel {
  READ_UNCOMMITTED,
  READ_COMMITTED,
  REPEATABLE_READ,
  SERIALIZABLE,
};

/// 事务状态
enum class TransactionState {
  RUNNING,
  COMMITTED,
  ABORTED,
};

/// 锁类型
enum class LockType : uint8_t {
  INVALID = 0,
  SHARED,   // S 锁
  EXCLUSIVE, // X 锁
};

/// 事务 — 封装事务的所有状态和资源。
///
/// 职责：
///   1. 跟踪事务状态
///   2. 管理事务持有锁的集合
///   3. 记录事务的读写集
class Transaction {
 public:
  explicit Transaction(txn_id_t txn_id,
                       IsolationLevel level = IsolationLevel::REPEATABLE_READ);
  ~Transaction() = default;

  DB_DISALLOW_COPY_AND_MOVE(Transaction);

  [[nodiscard]] auto GetTransactionId() const -> txn_id_t { return txn_id_; }
  [[nodiscard]] auto GetState() const -> TransactionState { return state_; }
  [[nodiscard]] auto GetIsolationLevel() const -> IsolationLevel { return level_; }

  void SetState(TransactionState state) { state_ = state; }

  /// 已加锁的页面集合
  [[nodiscard]] auto GetLockSet() const -> const std::set<page_id_t> & {
    return lock_set_;
  }

  /// 写入集（当前事务修改的页面）
  [[nodiscard]] auto GetWriteSet() const -> const std::set<page_id_t> & {
    return write_set_;
  }

  // TODO(Stage 7): 完善锁管理、2PL、MVCC 接口

 private:
  txn_id_t txn_id_;
  TransactionState state_{TransactionState::RUNNING};
  IsolationLevel level_;
  std::set<page_id_t> lock_set_;    // 当前持有的锁
  std::set<page_id_t> write_set_;   // 写入集

  // TODO(Stage 7): undo log, read timestamp, etc.
};

/// 锁管理器 — 管理事务间的锁冲突与死锁检测
class LockManager {
 public:
  LockManager() = default;
  ~LockManager() = default;

  DB_DISALLOW_COPY_AND_MOVE(LockManager);

  bool LockShared(Transaction *txn, page_id_t page_id);
  bool LockExclusive(Transaction *txn, page_id_t page_id);
  bool Unlock(Transaction *txn, page_id_t page_id);

  // TODO(Stage 7): 2PL 协议、死锁检测、锁升级
};

}  // namespace db
