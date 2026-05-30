# db-engineering

一个从零实现的轻量级关系型数据库引擎，参考 CMU 15-445/645 课程设计思路。

## 项目路线图

| 阶段 | 模块 | 核心内容 |
|------|------|---------|
| Stage 0 | 项目骨架 | 构建系统、目录结构、基础类型和异常体系 |
| Stage 1 | Page + DiskManager | 页布局、磁盘 I/O、页分配回收 |
| Stage 2 | BufferPool + LRU | 缓冲池管理、LRU 替换策略、钉住机制 |
| Stage 3 | Record 存储 | 表页、元组序列化、记录管理 |
| Stage 4 | 单线程 B+Tree | 索引的增删查和范围扫描 |
| Stage 5 | SQL Parser | 词法分析、语法分析、AST 构建 |
| Stage 6 | Executor | 查询计划执行、CRUD 操作 |
| Stage 7 | WAL / Transaction / MVCC | 预写日志、事务管理、多版本并发控制 |

## 技术栈

- **语言**: C++20
- **构建**: CMake 3.20+ / Ninja
- **测试**: Google Test
- **格式化**: fmtlib
- **平台**: Windows / Linux / macOS

## 快速开始

### 环境要求

- CMake >= 3.20
- 支持 C++20 的编译器（MSVC 2022+ / GCC 11+ / Clang 14+）
- Ninja（可选，推荐）

### 构建

```bash
# Debug 模式
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Release 模式
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### 运行测试

```bash
cd build
ctest --output-on-failure
```

## 项目结构

```
db-engineering/
├── CMakeLists.txt           # 顶层构建
├── src/                     # 源代码
│   ├── include/db/          # 公共头文件
│   │   ├── common/          # 配置、宏、异常
│   │   ├── storage/         # 存储层接口
│   │   ├── index/           # 索引接口
│   │   ├── parser/          # SQL 解析接口
│   │   ├── execution/       # 执行器接口
│   │   └── transaction/     # 事务接口
│   ├── storage/             # 存储层实现
│   ├── index/               # 索引实现
│   ├── parser/              # 解析器实现
│   ├── execution/           # 执行器实现
│   └── transaction/         # 事务实现
├── test/                    # 单元测试
└── third_party/             # 第三方依赖
```

## 许可证

MIT
