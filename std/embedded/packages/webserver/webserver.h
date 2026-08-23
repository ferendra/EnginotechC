// EnginotechC++ — Web Server Package
// HTTP server for ESP32 — routes, GET/POST, static files

#ifndef ENG_WEBSERVER_H
#define ENG_WEBSERVER_H

#include <cstdint>
#include <string>
#include <functional>
#include <map>

namespace eng {
namespace webserver {

struct Request {
    std::string method;   // "GET", "POST"
    std::string path;     // "/api/temperature"
    std::map<std::string, std::string> headers;
    std::string body;
    std::string remoteIP;
};

struct Response {
    int status_code;
    std::string content_type;
    std::string body;
};

using Handler = std::function<Response(Request)>;

class WebServer {
public:
    WebServer();
    ~WebServer();

    bool begin(uint16_t port = 80);
    void end();
    bool isRunning() const;

    // Route registration
    void on(const char* path, Handler handler);
    void get(const char* path, Handler handler);
    void post(const char* path, Handler handler);
    void onNotFound(Handler handler);

    // Static file serving
    void serveStatic(const char* uri, const char* filepath);

    // Send helper functions
    static Response ok(const char* body, const char* ct = "text/html");
    static Response json(const char* body);
    static Response notFound(const char* msg = "Not Found");
    static Response serverError(const char* msg = "Internal Server Error");

private:
    uint16_t port_;
    bool running_;
    std::map<std::string, Handler> routes_;
    Handler not_found_handler_;
    struct StaticRoute { std::string uri; std::string path; };
    std::vector<StaticRoute> static_routes_;
};

// Convenience function
WebServer* server(uint16_t port = 80);

} // namespace webserver
} // namespace eng

#endif // ENG_WEBSERVER_H
