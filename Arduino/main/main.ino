#include "Control.h"
#include "shapes.h"
// Pin assignments: adjust as needed
const int LEFT_BUTTON_PIN = 22;  // Left button
const int RIGHT_BUTTON_PIN = 19;  // Right button
const int STICK_BUTTON_PIN = 25;  // Stick click down

const int STICK_X_PIN = 26; // L/R = x
const int STICK_Y_PIN = 27; // U/D = y

Controller controller(
  LEFT_BUTTON_PIN,
  RIGHT_BUTTON_PIN,
  STICK_BUTTON_PIN,
  STICK_X_PIN,
  STICK_Y_PIN,
  4095.0f,
  0.15f
);

byte grid[22][10] = {};

int current_tetromino = 0;
int current_rotation = 0;
int current_x = 0;
int current_y = 0;

// Function to check collision for a tetromino at given position
bool checkCollision(int shape, int rotation, int x, int y) {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (tetrominoes[shape][rotation][i][j]) {
        int grid_x = x + j;
        int grid_y = y + i;
        // Check bounds
        if (grid_x < 0 || grid_x >= 10 || grid_y < 0 || grid_y >= 20) {
          return true;
        } else if (grid[grid_y][grid_x]) {
          return true;
        }
      }
    }
  }
  return false;
}

void setup() {
  // Initialize serial for debugging/monitoring
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for serial port to connect (useful on native USB boards)
  }
  // Fill the grid with zeros
  for (int i = 0; i < 22; i++) {
    for (int j = 0; j < 10; j++) {
      grid[i][j] = 0;
    }
  }

  controller.begin();
}

void loop() {
  controller.update();
  bool button_left = controller.isLeftPressed();
  bool button_right = controller.isRightPressed();
  bool button_stick = controller.isStickPressed();
  int stick_x = controller.getStickX();
  int stick_y = controller.getStickY();

  if (button_left){
    Serial.println("Rotate left");
    current_rotation--;
    if(current_rotation < 0){
      current_rotation = 3;
    }
  }

  if(button_right){
    Serial.println("Rotate right");
    current_rotation++;
    if(current_rotation > 3){
      current_rotation = 0;
    }
  }

  if(button_stick){
    Serial.println("Stick pressed");
  }

  StickDirection direction = controller.getDirection();
  if(direction != CENTER){
    Serial.println(StickDirectionStr[direction]);   
  }

  switch(direction){
    case UP:
    if(current_y > 0){
    current_y--;

      break;
    case DOWN:
      if(current_y < 20){
      current_y++;
      break;
    case LEFT:
      current_x--;
      break;
    case RIGHT:
      current_x++;
      break;
  }
  
  // Check for collision
  if (checkCollision(current_tetromino, current_rotation, current_x, current_y)) {
    Serial.println("Collision detected!");
  }

  delay(200); // Small delay to reduce spam; adjust as needed
}



