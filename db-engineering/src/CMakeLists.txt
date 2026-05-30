# ============================================================
# db-engine 库
# ============================================================
file(GLOB_RECURSE DB_SRC
  "common/*.cpp"
  "type/*.cc"
  "storage/disk/*.cpp"
  "storage/buffer/*.cpp"
  "storage/record/*.cpp"
  "index/*.cpp"
  "parser/*.cpp"
  "execution/*.cpp"
  "transaction/*.cpp"
)

add_library(db_engine STATIC ${DB_SRC})

target_include_directories(db_engine
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(db_engine
  PUBLIC
    fmt::fmt
)

# ============================================================
# 可选的命令行入口（后续使用）
# ============================================================
# add_executable(db_shell shell.cpp)
# target_link_libraries(db_shell PRIVATE db_engine)
