#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <string>
#include <unordered_map>

class HttpResponse {
public:
    HttpResponse();

    void setStatusCode(int code);
    void setStatusMessage(const std::string& msg);
    void setContentType(const std::string& type);
    void setBody(const std::string& body);
    void setKeepAlive(bool on);
    void setHeader(const std::string& key, const std::string& value);

    std::string toMessage() const;

private:
    int statusCode_;
    std::string statusMessage_;
    std::unordered_map<std::string, std::string> headers_;
    std::string body_;
    bool keepAlive_;
};

#endif // HTTPRESPONSE_H
