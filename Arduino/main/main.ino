#include "Control.h"  
#include "Game.h"
#include "ServerConnector.h"

// Pin assignments: adjust as needed
const int LEFT_BUTTON_PIN = 23;   // Left button
const int RIGHT_BUTTON_PIN = 21;  // Right button
const int STICK_BUTTON_PIN = 25;  // Stick click down

const int STICK_X_PIN = 33;  // L/R = x
const int STICK_Y_PIN = 32;  // U/D = y

// Wi-Fi credentials (set these!)
const char* WIFI_SSID = "MSI 6387";
const char* WIFI_PASSWORD = "53y17C0!";

// WebSocket server settings (match Webserver/server.js)
// Example: ws://192.168.1.100:8080/?code=12345
const char* WS_HOST = "192.168.137.1";  // replace with your PC/server LAN IP
const uint16_t WS_PORT = 8080;
const char* ROOM_CODE = "12345";        // set your room code
const char* wsPath = String("/?code=") + ROOM_CODE;

ServerConnector serverConnector(
  WIFI_SSID, WIFI_PASSWORD, 
  WS_HOST, WS_PORT, wsPath.c_str(), 
  webSocketEvent);

Controller controller(
  LEFT_BUTTON_PIN,
  RIGHT_BUTTON_PIN,
  STICK_BUTTON_PIN,
  STICK_X_PIN,
  STICK_Y_PIN,
  4095.0f,
  0.15f);

static Game game;

static void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.printf("WS connected: %s\n", (const char*)payload);
      break;
    case WStype_TEXT:
      Serial.printf("WS text: %s\n", (const char*)payload);
      // TODO: parse incoming text to control game if desired
      break;
    case WStype_BIN:
      Serial.printf("WS binary: %s\n", (const char*)payload);
      // TODO: parse incoming binary to control game if desired
      break;
    case WStype_DISCONNECTED:
      Serial.println("WS disconnected");
      break;
    default:
      break;
  }
}

void setup() {
  // Initialize serial for debugging/monitoring
  Serial.begin(115200);
  while (!Serial) {
    ;  // Wait for serial port to connect (useful on native USB boards)
  }

  // Build path with required room code and begin WebSocket (ws://)
  serverConnector.init();

  controller.begin();
  game.init();
}

void loop() {
  static unsigned long lastGameUpdate = 0;
  static byte out[GRID_HEIGHT][GRID_WIDTH];

  // Keep WebSocket alive
  serverConnector.loop();
  controller.update();

  // Only update game every 250ms
  unsigned long now = millis();
  if (now - lastGameUpdate >= 100) {

    game.Tick(controller);
    game.printGrid();

    // Get current grid and send it
    game.getGrid(out);
    serverConnector.sendPacket(PacketType.GAME_STATE_UPDATE, (uint8_t*)out);
    
    if (game.scoreChanged) {
      serverConnector.sendPacket(PacketType.GAME_SCORE_UPDATE, (uint8_t*)game.getScore());
    }

    if (game.bufferChanged) {
      serverConnector.sendPacket(PacketType.GAME_BUFFER_UPDATE, (uint8_t*)game.getBuffer());
    }
    lastGameUpdate = now;
  }
}
