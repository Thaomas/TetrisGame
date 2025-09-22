#include "ServerConnector.h"
#include <WiFi.h>
#include "types.h"

ServerConnector::ServerConnector(const char* wifiSSID, const char* wifiPassword, const char* host, uint16_t port, const char* path, void (*callback)(WStype_t type, uint8_t* payload, size_t length))
    : wifiSSID(wifiSSID), wifiPassword(wifiPassword), host(host), port(port), path(path), callback(callback) {}

void ServerConnector::init() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSSID, wifiPassword);
    Serial.print("Connecting WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
        Serial.print(".");
    }
    Serial.print("\nWiFi connected, IP: ");
    Serial.println(WiFi.localIP());

    webSocket.begin(host, port, path);
    webSocket.onEvent(callback);

    webSocket.setReconnectInterval(5000);    // auto-reconnect every 5s
    webSocket.enableHeartbeat(15000, 3000, 2); // ping every 15s

}

void ServerConnector::sendData(const uint8_t* data, size_t length) {
    webSocket.sendBIN(data, length);
}

void ServerConnector::sendPacket(PacketType type, const uint8_t* data) {
    size_t length = sizeof(data);
    uint8_t packet[length + 1];
    packet[0] = (uint8_t)type;
    memcpy(packet + 1, data, length);
    webSocket.sendBIN(packet, length + 1);
}
