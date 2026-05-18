#include "HttpRequest.h"
#include <cstdlib>
#include <algorithm>

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

bool HttpRequest::parse(const char* data, size_t len) {
    buf_.append(data, len);

    while (true) {
        switch (state_) {
        case kExpectRequestLine: {
            size_t pos = buf_.find("\r\n");
            if (pos == std::string::npos) return true;

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
                state_ = (contentLength_ > 0) ? kExpectBody : kGotAll;
            } else {
                if (!parseHeader(line)) return false;
            }
            break;
        }
        case kExpectBody: {
            if (buf_.size() >= (size_t)contentLength_) {
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

bool HttpRequest::parseRequestLine(const std::string& line) {
    // "GET /path HTTP/1.1"
    size_t pos1 = line.find(' ');
    if (pos1 == std::string::npos) return false;

    size_t pos2 = line.rfind(' ');
    if (pos2 == pos1 || pos2 == std::string::npos) return false;

    method_ = line.substr(0, pos1);
    path_ = line.substr(pos1 + 1, pos2 - pos1 - 1);
    version_ = line.substr(pos2 + 1);

    // Normalize path
    if (path_.empty() || path_[0] != '/') {
        path_ = "/" + path_;
    }

    return true;
}

bool HttpRequest::parseHeader(const std::string& line) {
    size_t pos = line.find(": ");
    if (pos == std::string::npos) {
        // Some clients send "key:value" without space
        pos = line.find(':');
        if (pos == std::string::npos) return false;
    }

    std::string key = line.substr(0, pos);
    std::string value = line.substr(pos + 2);

    // Lowercase key for case-insensitive lookup
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);

    headers_[key] = value;

    if (key == "content-length") {
        contentLength_ = std::atoi(value.c_str());
    }

    return true;
}

std::string HttpRequest::getHeader(const std::string& key) const {
    std::string lower = key;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    auto it = headers_.find(lower);
    return (it != headers_.end()) ? it->second : "";
}

bool HttpRequest::keepAlive() const {
    std::string conn = getHeader("connection");
    if (version_ == "HTTP/1.1") {
        return conn != "close";
    }
    return conn == "keep-alive";
}
