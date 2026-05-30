cmake_minimum_required(VERSION 3.20)
project(db-engineering VERSION 0.1.0 LANGUAGES CXX)

# ============================================================
# C++ 标准
# ============================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================
# 构建类型
# ============================================================
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# ============================================================
# 全局编译选项
# ============================================================
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDB_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")

if(MSVC)
  add_compile_options(/W4 /utf-8)
  add_compile_options("$<$<CONFIG:DEBUG>:/Od /Zi>")
  add_compile_options("$<$<CONFIG:RELEASE>:/O2>")
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ============================================================
# 测试选项
# ============================================================
option(DB_BUILD_TESTS "Build unit tests" ON)

# ============================================================
# 第三方依赖
# ============================================================
add_subdirectory(third_party)

# ============================================================
# 主库 & 可执行目标
# ============================================================
add_subdirectory(src)

# ============================================================
# 测试
# ============================================================
if(DB_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()
