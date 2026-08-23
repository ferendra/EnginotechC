// EnginotechC++ — MQQT IoT Package
// MQTT client for ESP32 — lightweight pub/sub messaging

#ifndef ENG_MQTT_H
#define ENG_MQTT_H

#include <cstdint>
#include <string>
#include <functional>

namespace eng {
namespace mqtt {

struct Client {
    std::string broker;
    std::string client_id;
    uint16_t port;
    bool connected;
    int mqtt_sock;
};

using MessageCallback = std::function<void(const char* topic, const char* payload)>;

// Connect to MQTT broker
Client connect(const char* broker_url, const char* client_id = nullptr,
               uint16_t port = 1883);

// Disconnect
void disconnect(Client& client);

// Publish message
bool publish(Client& client, const char* topic, const char* payload,
             bool retained = false);

// Subscribe to topic
bool subscribe(Client& client, const char* topic,
               int qos = 0);

// Unsubscribe
bool unsubscribe(Client& client, const char* topic);

// Set message callback
void onMessage(Client& client, MessageCallback cb);

// Call this periodically to handle incoming messages
void loop(Client& client);

// Check connection status
bool isConnected(Client& client);

} // namespace mqtt
} // namespace eng

#endif // ENG_MQTT_H
