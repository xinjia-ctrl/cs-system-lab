#include "HttpRequest.h"
#include <cstdlib>
#include <algorithm>
#include <cctype>

// 单请求最大解析字节数（防止恶意请求耗尽内存）
static const size_t kMaxRequestSize = 64 * 1024;   // 64KB
static const size_t kMaxHeaderCount = 64;

HttpRequest::HttpRequest()
    : state_(kExpectRequestLine), contentLength_(0) {}

void HttpRequest::reset() {
    state_ = kExpectRequestLine;
    buf_.clear();
    method_.clear();
    path_.clear();
    version_.clear();
    headers_.clear();
    body_.clear();
    contentLength_ = 0;
}

// 从网络数据中增量解析 HTTP 请求。可能一次 read 只收到半包，
// 多次调用 parse 直到 gotAll() 返回 true 才算完整。
// return: true = 正常（可能尚未解析完）, false = 协议错误
bool HttpRequest::parse(const char* data, size_t len) {
    // 拒绝超长请求
    if (buf_.size() + len > kMaxRequestSize) {
        return false;
    }
    buf_.append(data, len);

    while (true) {
        switch (state_) {
        case kExpectRequestLine: {
            // 查找一行结尾 "\r\n"
            size_t pos = buf_.find("\r\n");
            if (pos == std::string::npos) return true;  // 等待更多数据

            std::string line = buf_.substr(0, pos);
            buf_.erase(0, pos + 2);

            if (!parseRequestLine(line)) return false;
            state_ = kExpectHeaders;
            break;
        }
        case kExpectHeaders: {
            size_t pos = buf_.find("\r\n");
            if (pos == std::string::npos) return true;

            std::string line = buf_.substr(0, pos);
            buf_.erase(0, pos + 2);

            if (line.empty()) {
                // 空行 = header 结束
                state_ = (contentLength_ > 0) ? kExpectBody : kGotAll;
            } else {
                if (!parseHeader(line)) return false;
                // 限制 header 数量，防止攻击
                if (headers_.size() > kMaxHeaderCount) return false;
            }
            break;
        }
        case kExpectBody: {
            if (buf_.size() >= static_cast<size_t>(contentLength_)) {
                body_ = buf_.substr(0, contentLength_);
                buf_.erase(0, contentLength_);
                state_ = kGotAll;
            }
            return true;
        }
        case kGotAll:
            return true;
        }
    }
}

// 解析请求行 "GET /path HTTP/1.1"
bool HttpRequest::parseRequestLine(const std::string& line) {
    // HTTP 方法
    size_t pos1 = line.find(' ');
    if (pos1 == std::string::npos) return false;

    // HTTP 版本（从末尾往前找空格）
    size_t pos2 = line.rfind(' ');
    if (pos2 == pos1 || pos2 == std::string::npos) return false;

    method_.assign(line, 0, pos1);
    path_.assign(line, pos1 + 1, pos2 - pos1 - 1);
    version_.assign(line, pos2 + 1, std::string::npos);

    // 确保 path 以 '/' 开头
    if (path_.empty()) {
        path_ = "/";
    } else if (path_[0] != '/') {
        path_ = "/" + path_;
    }

    // 检查 HTTP 版本号
    if (version_.find("HTTP/") != 0) return false;

    return true;
}

// 解析单个请求头行 "Key: Value"
bool HttpRequest::parseHeader(const std::string& line) {
    size_t colonPos = line.find(':');
    if (colonPos == std::string::npos) return false;

    // key: 冒号前全部内容，转为小写
    std::string key(line, 0, colonPos);
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);

    // value: 冒号后内容，去除前导空白
    std::string value(line, colonPos + 1);
    size_t start = value.find_first_not_of(" \t");
    if (start != std::string::npos) {
        value = value.substr(start);
    }

    headers_[key] = value;

    if (key == "content-length") {
        char* end = nullptr;
        long len = std::strtol(value.c_str(), &end, 10);
        if (end == value.c_str() || len < 0) {
            return false;  // 非法的 Content-Length
        }
        contentLength_ = len;
    }

    return true;
}

// 获取请求头（不区分大小写）
std::string HttpRequest::getHeader(const std::string& key) const {
    std::string lowerKey(key);
    std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);
    auto it = headers_.find(lowerKey);
    return (it != headers_.end()) ? it->second : "";
}

// 判断是否应保持长连接
bool HttpRequest::keepAlive() const {
    std::string conn = getHeader("connection");
    // HTTP/1.1 默认 keep-alive，除非显式声明 close
    if (version_ == "HTTP/1.1") {
        return conn != "close";
    }
    // HTTP/1.0 默认关闭，除非显式声明 keep-alive
    return conn == "keep-alive";
}
