#ifndef SERVERCONNECTOR_H
#define SERVERCONNECTOR_H

#include <Arduino.h>
#include <WebSocketsClient.h>

class ServerConnector {
private:
    WebSocketsClient webSocket;
    const char* wifiSSID;
    const char* wifiPassword;
    const char* host;
    uint16_t port;
    const char* path;
    void (*callback)(WStype_t type, uint8_t* payload, size_t length);
public:
    ServerConnector(const char* host, uint16_t port, const char* path);
    void init();
    void loop();
    void connect();
    void disconnect();
    void sendData(const uint8_t* data, size_t length);
    void sendPacket(PacketType type, const char* data);
    void receiveData();
};

#endif