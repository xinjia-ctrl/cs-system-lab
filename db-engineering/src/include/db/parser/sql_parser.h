#pragma once

#include <memory>
#include <string>
#include <vector>

#include "db/common/config.h"

namespace db {

/// SQL 语句类型
enum class StatementType {
  INVALID = 0,
  SELECT,
  INSERT,
  UPDATE,
  DELETE,
  CREATE_TABLE,
  DROP_TABLE,
};

/// 表达式类型
enum class ExpressionType {
  INVALID = 0,
  CONSTANT,
  COLUMN_REF,
  COMPARISON,
  LOGICAL,
};

/// 值类型
struct Value {
  ExpressionType type{ExpressionType::INVALID};
  std::string str_val;

  // TODO(Stage 5): 添加更完善的类型支持
};

/// 抽象语法树节点基类
struct ASTNode {
  virtual ~ASTNode() = default;
  StatementType stmt_type{StatementType::INVALID};
};

/// SELECT 语句 AST
struct SelectStmt : ASTNode {
  std::vector<std::string> columns;
  std::string table_name;
  // TODO(Stage 5): 添加 WHERE、JOIN、ORDER BY 等
};

/// INSERT 语句 AST
struct InsertStmt : ASTNode {
  std::string table_name;
  std::vector<std::vector<Value>> values;
};

/// CREATE TABLE 语句 AST
struct CreateTableStmt : ASTNode {
  std::string table_name;
  std::vector<std::pair<std::string, std::string>> columns; // name, type
};

/// SQL 解析器 — 将 SQL 文本解析为 AST
class SqlParser {
 public:
  SqlParser() = default;
  ~SqlParser() = default;

  DB_DISALLOW_COPY_AND_MOVE(SqlParser);

  /// 解析单条 SQL 语句
  auto Parse(const std::string &sql) -> std::unique_ptr<ASTNode>;

  /// 解析多条 SQL 语句
  auto ParseMultiple(const std::string &sql)
      -> std::vector<std::unique_ptr<ASTNode>>;

 private:
  size_t pos_{0};
  std::string input_;

  // TODO(Stage 5): 实现词法分析和语法分析
};

}  // namespace db
