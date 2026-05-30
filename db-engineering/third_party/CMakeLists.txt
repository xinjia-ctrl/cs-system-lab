# ============================================================
# fmtlib — 类型安全的格式化
# ============================================================
include(FetchContent)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG        11.0.2
  GIT_SHALLOW    TRUE
)
set(FMT_DOC OFF CACHE BOOL "" FORCE)
set(FMT_TEST  OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(fmt)

# ============================================================
# Google Test（仅测试模式）
# ============================================================
if(DB_BUILD_TESTS)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2
    GIT_SHALLOW    TRUE
  )
  set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
endif()
