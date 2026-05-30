#pragma once

#include "db/common/config.h"

namespace db {

/// WAL 日志记录类型
enum class LogType : uint8_t {
  INVALID = 0,
  INSERT,
  DELETE,
  UPDATE,
  BEGIN_TXN,
  COMMIT_TXN,
  ABORT_TXN,
};

/// 日志记录头部
struct LogHeader {
  lsn_t lsn{0};
  lsn_t prev_lsn{0};
  txn_id_t txn_id{INVALID_TXN_ID};
  LogType log_type{LogType::INVALID};
  uint32_t record_size{0};
};

/// WAL 管理器 — 预写式日志。
///
/// 职责：
///   1. 追加日志记录（Append）
///   2. 日志刷盘（Flush）
///   3. 崩溃恢复（Redo / Undo）
///   4. 检查点（Checkpoint）
class WalManager {
 public:
  WalManager() = default;
  ~WalManager() = default;

  DB_DISALLOW_COPY_AND_MOVE(WalManager);

  // TODO(Stage 7): 实现 WAL 日志接口

 private:
  // TODO(Stage 7): 日志缓冲区、文件管理、恢复算法
};

}  // namespace db
