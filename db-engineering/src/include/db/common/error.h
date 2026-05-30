#pragma once

#include <stdexcept>
#include <string>

namespace db {

// ============================================================
// 数据库异常体系
// ============================================================

/// 基础数据库异常
class DbException : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

/// 磁盘 I/O 错误（文件读写失败等）
class IOException : public DbException {
 public:
  using DbException::DbException;
};

/// 页面相关错误（无效页号、页损坏等）
class PageException : public DbException {
 public:
  using DbException::DbException;
};

/// 索引相关错误
class IndexException : public DbException {
 public:
  using DbException::DbException;
};

/// 类型系统错误
class TypeException : public DbException {
 public:
  using DbException::DbException;
};

/// 事务相关错误
class TransactionException : public DbException {
 public:
  using DbException::DbException;
};

/// 序列化/反序列化错误
class SerializationException : public DbException {
 public:
  using DbException::DbException;
};

}  // namespace db
