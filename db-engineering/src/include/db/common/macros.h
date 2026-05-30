#pragma once

#include <cassert>
#include <stdexcept>

// ============================================================
// 断言与编译期检查
// ============================================================

/// 运行时断言（Debug 模式启用，Release 模式编译消除）
#ifdef DB_DEBUG
#define DB_ASSERT(expr, msg) assert((expr) && (msg))
#else
#define DB_ASSERT(expr, msg) ((void)0)
#endif

/// 编译期静态断言（始终生效）
#define DB_STATIC_ASSERT(expr, msg) static_assert((expr), msg)

// ============================================================
// 标记与属性
// ============================================================

/// 标记当前代码路径暂未实现，调用会抛异常
#define DB_NOT_IMPLEMENTED()                                                    \
  throw std::runtime_error("Not implemented: " __FILE__ ":" +                  \
                           std::to_string(__LINE__))

/// 标记未使用的变量，抑制编译器警告
#define DB_UNUSED(var) ((void)(var))

/// 标记有意 fall-through 的 case（C++17 [[fallthrough]]）
#define DB_FALLTHROUGH [[fallthrough]]

// ============================================================
// 内存对齐
// ============================================================

/// 按页面大小对齐（用于 Direct I/O 或 mmap 场景）
#define DB_PAGE_ALIGNED alignas(db::PAGE_SIZE)

// ============================================================
// 防拷贝 / 防移动
// ============================================================

/// 禁用拷贝构造和拷贝赋值
#define DB_DISALLOW_COPY(class_name)                                            \
  class_name(const class_name &) = delete;                                     \
  auto operator=(const class_name &) -> class_name & = delete

/// 禁用移动构造和移动赋值
#define DB_DISALLOW_MOVE(class_name)                                            \
  class_name(class_name &&) = delete;                                          \
  auto operator=(class_name &&) -> class_name & = delete

/// 同时禁用拷贝和移动
#define DB_DISALLOW_COPY_AND_MOVE(class_name)                                  \
  DB_DISALLOW_COPY(class_name);                                                \
  DB_DISALLOW_MOVE(class_name)

// ============================================================
// 作用域退出钩子
// ============================================================

/// 在作用域退出时自动执行 lambda（类似 Go 的 defer）
#define DB_DEFER(...)                                                          \
  auto DB_CONCAT_IMPL(_db_defer_, __LINE__) = ScopeGuard([&] { __VA_ARGS__ })

#define DB_CONCAT_IMPL(a, b) a##b

namespace db {

/// RAII 作用域守卫
template <typename F>
class ScopeGuard {
 public:
  explicit ScopeGuard(F &&fn) : fn_(std::move(fn)) {}
  ~ScopeGuard() { fn_(); }

 private:
  F fn_;
};

}  // namespace db
