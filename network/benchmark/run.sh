#!/bin/bash
# epoll HTTP Server 压测脚本
# 对 4 个阶段分别做基准测试，输出对比结果

set -e

PORT=8080
URL="http://localhost:$PORT/"
BUILD_DIR="/root/cs-system-lab/network/build"
STAGES=("stage1_blocking" "stage2_epoll" "stage3_reactor" "stage4_http")
NAMES=("阶段1: 阻塞Server" "阶段2: epoll单线程" "阶段3: Reactor多线程" "阶段4: HTTP服务器")

check_tool() {
    if command -v wrk &>/dev/null; then
        TOOL="wrk"
    elif command -v ab &>/dev/null; then
        TOOL="ab"
    else
        echo "错误: 未找到 wrk 或 ab (ApacheBench)"
        echo "安装: yum install -y httpd-tools"
        exit 1
    fi
}

run_bench() {
    local stage=$1
    local name=$2

    # Kill any existing process on port
    pkill -f "$stage" 2>/dev/null || true
    sleep 0.5

    # Start server
    nohup "$BUILD_DIR/$stage" > /dev/null 2>&1 &
    local pid=$!
    sleep 0.5

    if ! kill -0 $pid 2>/dev/null; then
        echo "  [跳过] 服务器启动失败"
        return
    fi

    echo ""
    echo "===== $name ====="

    if [ "$TOOL" = "wrk" ]; then
        wrk -t2 -c100 -d10s "$URL"
    else
        # ab: stage1 只能用低并发
        if [ "$stage" = "stage1_blocking" ]; then
            ab -n 100 -c 10 -s 5 "$URL" 2>&1 | grep -E '(Requests per second|Failed|Complete|Time taken)'
        else
            ab -n 5000 -c 100 "$URL" 2>&1 | grep -E '(Requests per second|Failed|Complete|Time taken)'
        fi
    fi

    pkill -f "$stage" 2>/dev/null || true
    sleep 0.3
}

# Main
check_tool
echo "压测工具: $TOOL"
echo "压测对象: http://localhost:$PORT/"
echo ""

for i in "${!STAGES[@]}"; do
    run_bench "${STAGES[$i]}" "${NAMES[$i]}"
done

echo ""
echo "===== 对比总结 ====="
echo "| 阶段 | 模型 | 吞吐量 | 延迟 |"
echo "|------|------|--------|------|"
echo "| 1 | 阻塞 accept | — | — |"
echo "| 2 | epoll ET/LT | — | — |"
echo "| 3 | 主从 Reactor | — | — |"
echo "| 4 | +HTTP 解析 | — | — |"
echo ""
echo "提示: 将上面的 — 替换为实际压测数据"
