// EnginotechC++ — Web Server Implementation
#include "webserver.h"
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <memory>

namespace eng {
namespace webserver {

WebServer::WebServer() : port_(80), running_(false) {}
WebServer::~WebServer() { end(); }

bool WebServer::begin(uint16_t port) {
    port_ = port;
    running_ = true;
    printf("[WebServer] Started on port %d\n", port);
    return true;
}

void WebServer::end() {
    if (running_) {
        printf("[WebServer] Stopped\n");
        running_ = false;
    }
}

bool WebServer::isRunning() const { return running_; }

void WebServer::on(const char* path, Handler handler) {
    routes_[path] = handler;
}

void WebServer::get(const char* path, Handler handler) {
    routes_[std::string(path) + " GET"] = handler;
}

void WebServer::post(const char* path, Handler handler) {
    routes_[std::string(path) + " POST"] = handler;
}

void WebServer::onNotFound(Handler handler) {
    not_found_handler_ = handler;
}

void WebServer::serveStatic(const char* uri, const char* filepath) {
    static_routes_.push_back({uri, filepath});
}

Response WebServer::ok(const char* body, const char* ct) {
    Response r{200, ct, body ? body : ""};
    return r;
}

Response WebServer::json(const char* body) {
    Response r{200, "application/json", body ? body : "{}"};
    return r;
}

Response WebServer::notFound(const char* msg) {
    return {404, "text/html", msg ? msg : "<h1>404 Not Found</h1>"};
}

Response WebServer::serverError(const char* msg) {
    return {500, "text/html", msg ? msg : "<h1>500 Internal Server Error</h1>"};
}

// Global server instance (singleton pattern)
static WebServer* g_server = nullptr;

WebServer* server(uint16_t port) {
    if (!g_server) {
        g_server = new WebServer();
        g_server->begin(port);
    }
    return g_server;
}

} // namespace webserver
} // namespace eng
