#ifndef HTTPCONTEXT_H
#define HTTPCONTEXT_H

#include "HttpRequest.h"

// Per-connection HTTP parsing context.
// Handles partial data and maintains parse state across multiple read() calls.
class HttpContext {
public:
    HttpContext() = default;

    bool parse(const char* data, size_t len) {
        return request_.parse(data, len);
    }

    bool gotAll() const {
        return request_.gotAll();
    }

    HttpRequest& request() {
        return request_;
    }

    void reset() {
        request_.reset();
    }

private:
    HttpRequest request_;
};

#endif // HTTPCONTEXT_H
