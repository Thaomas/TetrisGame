#include "Control.h"
#include "Game.h"
// Pin assignments: adjust as needed
const int LEFT_BUTTON_PIN = 23;   // Left button
const int RIGHT_BUTTON_PIN = 21;  // Right button
const int STICK_BUTTON_PIN = 25;  // Stick click down

const int STICK_X_PIN = 26;  // L/R = x
const int STICK_Y_PIN = 27;  // U/D = y

Controller controller(
  LEFT_BUTTON_PIN,
  RIGHT_BUTTON_PIN,
  STICK_BUTTON_PIN,
  STICK_X_PIN,
  STICK_Y_PIN,
  4095.0f,
  0.15f);

static Game game;


void setup() {
  // Initialize serial for debugging/monitoring
  Serial.begin(115200);
  while (!Serial) {
    ;  // Wait for serial port to connect (useful on native USB boards)
  }

  controller.begin();
  game.init();
}

void loop() {
  controller.update();
  game.Tick(controller);

  game.printGrid();
  delay(250);  // Small delay to reduce spam; adjust as needed
}
