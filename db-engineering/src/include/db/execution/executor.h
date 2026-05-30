#pragma once

#include <memory>
#include <vector>

#include "db/common/config.h"
#include "db/parser/sql_parser.h"

namespace db {

class BufferPoolManager;
class DiskManager;

/// 执行上下文
struct ExecutorContext {
  DiskManager *disk_manager{nullptr};
  BufferPoolManager *buffer_pool{nullptr};
  // TODO(Stage 7): 添加事务上下文
};

/// 执行结果
struct ExecResult {
  bool success{false};
  std::string message;
  size_t affected_rows{0};
  std::vector<std::vector<std::string>> rows; // 列式结果（SELECT）
};

/// 执行器 — 将解析后的 AST 转换为实际的数据操作。
///
/// 职责：
///   1. SELECT 语句：遍历表、过滤、投影
///   2. INSERT/DELETE/UPDATE：修改数据
///   3. CREATE/DROP TABLE：DDL 操作
class Executor {
 public:
  explicit Executor(ExecutorContext *context);
  ~Executor() = default;

  DB_DISALLOW_COPY_AND_MOVE(Executor);

  /// 执行单条 AST
  auto Execute(ASTNode *ast) -> ExecResult;

  /// 执行多条 AST
  auto ExecuteMultiple(const std::vector<std::unique_ptr<ASTNode>> &asts)
      -> std::vector<ExecResult>;

 private:
  auto ExecuteSelect(SelectStmt *stmt) -> ExecResult;
  auto ExecuteInsert(InsertStmt *stmt) -> ExecResult;
  auto ExecuteCreateTable(CreateTableStmt *stmt) -> ExecResult;

  ExecutorContext *context_;

  // TODO(Stage 6): 实现具体执行逻辑
};

}  // namespace db
