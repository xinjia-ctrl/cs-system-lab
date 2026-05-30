#pragma once

#include <cstddef>
#include <cstdint>

namespace db {

// ============================================================
// 数据库全局配置常量
// ============================================================

/// 页面大小：4KB（磁盘与内存的 I/O 基本单位）
static constexpr size_t PAGE_SIZE = 4096;

/// 缓冲池默认帧数
static constexpr size_t BUFFER_POOL_SIZE = 128;

/// 磁盘管理器文件名的最大长度
static constexpr size_t MAX_FILE_NAME_LEN = 128;

/// 表空间文件扩展名
static constexpr const char *DB_FILE_EXT = ".db";

/// 日志文件扩展名
static constexpr const char *LOG_FILE_EXT = ".log";

/// 逻辑页号偏移的位宽（32 位可寻址约 16TB 空间）
using page_id_t = int32_t;

/// 物理帧号
using frame_id_t = int32_t;

/// 事务 ID
using txn_id_t = int64_t;

/// LSN（日志序列号）
using lsn_t = int64_t;

/// 无效的页号标识
static constexpr page_id_t INVALID_PAGE_ID = -1;

/// 无效的帧号标识
static constexpr frame_id_t INVALID_FRAME_ID = -1;

/// 无效的事务 ID
static constexpr txn_id_t INVALID_TXN_ID = -1;

}  // namespace db
