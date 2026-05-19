#include "HttpServer.h"
#include <cstdio>

int main() {
    HttpServer server(8080, 4);

    server.setCallback([](const HttpRequest& req, HttpResponse* resp) {
        printf("%s %s\n", req.method().c_str(), req.path().c_str());

        if (req.path() == "/") {
            resp->setStatusCode(200);
            resp->setContentType("text/plain");
            resp->setBody("Hello, World!\n");
        } else if (req.path() == "/hello") {
            resp->setStatusCode(200);
            resp->setContentType("text/html");
            resp->setBody("<h1>Hello!</h1><p>from epoll HTTP server</p>\n");
        } else {
            resp->setStatusCode(404);
            resp->setContentType("text/plain");
            resp->setBody("404 Not Found\n");
        }
    });

    if (!server.start()) {
        return 1;
    }

    printf("Stage 4 — HTTP server running on http://localhost:8080\n");
    server.run();
    return 0;
}
