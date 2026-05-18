#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <string>
#include <unordered_map>

class HttpRequest {
public:
    HttpRequest();

    bool parse(const char* data, size_t len);
    bool gotAll() const { return state_ == kGotAll; }
    void reset();

    const std::string& method() const { return method_; }
    const std::string& path() const { return path_; }
    const std::string& version() const { return version_; }
    const std::string& body() const { return body_; }

    std::string getHeader(const std::string& key) const;
    bool keepAlive() const;

private:
    enum State {
        kExpectRequestLine,
        kExpectHeaders,
        kExpectBody,
        kGotAll,
    };

    bool parseRequestLine(const std::string& line);
    bool parseHeader(const std::string& line);

    State state_;
    std::string buf_;
    std::string method_;
    std::string path_;
    std::string version_;
    std::unordered_map<std::string, std::string> headers_;
    std::string body_;
    int contentLength_;
};

#endif // HTTPREQUEST_H
