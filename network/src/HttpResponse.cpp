#include "HttpResponse.h"
#include <sstream>

HttpResponse::HttpResponse()
    : statusCode_(200), statusMessage_("OK"), keepAlive_(false) {}

void HttpResponse::setStatusCode(int code) {
    statusCode_ = code;
    switch (code) {
    case 200: statusMessage_ = "OK"; break;
    case 404: statusMessage_ = "Not Found"; break;
    case 400: statusMessage_ = "Bad Request"; break;
    case 500: statusMessage_ = "Internal Server Error"; break;
    default: statusMessage_ = "Unknown"; break;
    }
}

void HttpResponse::setStatusMessage(const std::string& msg) {
    statusMessage_ = msg;
}

void HttpResponse::setContentType(const std::string& type) {
    headers_["Content-Type"] = type;
}

void HttpResponse::setBody(const std::string& body) {
    body_ = body;
}

void HttpResponse::setKeepAlive(bool on) {
    keepAlive_ = on;
}

void HttpResponse::setHeader(const std::string& key, const std::string& value) {
    headers_[key] = value;
}

std::string HttpResponse::toMessage() const {
    std::ostringstream oss;

    // Status line
    oss << "HTTP/1.1 " << statusCode_ << " " << statusMessage_ << "\r\n";

    // Headers
    for (const auto& h : headers_) {
        oss << h.first << ": " << h.second << "\r\n";
    }

    oss << "Content-Length: " << body_.size() << "\r\n";
    oss << "Connection: " << (keepAlive_ ? "keep-alive" : "close") << "\r\n";

    // Header terminator
    oss << "\r\n";

    // Body
    oss << body_;

    return oss.str();
}
