#include "Control.h"
#include "shapes.h"
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

byte grid[22][10] = {};

int current_tetromino = 0;
int current_rotation = 0;
int current_x = 3;
int current_y = 0;

// Function to check collision for a tetromino at given position
bool checkCollision(int shape, int rotation, int x, int y) {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (!tetrominoes[shape][rotation][i][j]) {
        continue;
      }

      int grid_x = x + j;
      int grid_y = y + i;

      // Check bounds
      if (grid_x < 0 || grid_x >= 10 || grid_y < 0 || grid_y >= 22) {
        Serial.println("Collision detected! Out of bounds");
        Serial.println(grid_x);
        Serial.println(grid_y);
        return true;
      }
      if (grid[grid_y][grid_x]) {
        Serial.println("Collision detected! Grid cell occupied");
        Serial.println(grid_x);
        Serial.println(grid_y);
        return true;
      }
    }
  }

  return false;
}

void setup() {
  // Initialize serial for debugging/monitoring
  Serial.begin(115200);
  while (!Serial) {
    ;  // Wait for serial port to connect (useful on native USB boards)
  }
  // Fill the grid with zeros
  for (int i = 0; i < 22; i++) {
    for (int j = 0; j < 10; j++) {
      grid[i][j] = 0;
    }
  }

  controller.begin();
}

void printGrid() {
  // Create a temporary grid and copy the current grid into it
  byte tempGrid[22][10];
  for (int i = 0; i < 22; i++) {
    for (int j = 0; j < 10; j++) {
      tempGrid[i][j] = grid[i][j];
    }
  }

  // Overlay the current tetromino onto the temp grid
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (!tetrominoes[current_tetromino][current_rotation][i][j]) {
        continue;
      }
      int grid_x = current_x + j;
      int grid_y = current_y + i;
      if (grid_x < 0 || grid_x >= 10 || grid_y < 0 || grid_y >= 22) {
        continue;
      }
      tempGrid[grid_y][grid_x] = current_tetromino + 1;
    }
  }

  // Print the temp grid
  // Print top border
  Serial.print('+');
  for (int j = 0; j < 10; j++) {
    Serial.print('-');
  }
  Serial.println('+');
  // Print grid with side borders
  for (int i = 0; i < 22; i++) {
    Serial.print('|');
    for (int j = 0; j < 10; j++) {
      Serial.print(tempGrid[i][j]);
      // if (tempGrid[i][j] == 0) {
      //   Serial.print('-');
      // } else {
      //   Serial.print('0');
      // }
    }
    Serial.println('|');
  }
  // Print bottom border
  Serial.print('+');
  for (int j = 0; j < 10; j++) {
    Serial.print('-');
  }
  Serial.println('+');
  Serial.println();
}

int gameTickTime = 3;

// Dedicated movement functions per direction
void moveDown() {
  if (!checkCollision(current_tetromino, current_rotation, current_x, current_y + 1)) {
    current_y++;
    return;
  }


  // Add current tetromino to grid and reset
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (tetrominoes[current_tetromino][current_rotation][i][j]) {
        int gridX = current_x + j;
        int gridY = current_y + i;
        grid[gridY][gridX] = current_tetromino + 1;
      }
    }
  }

  // Reset tetromino
  current_x = 3;
  current_y = 0;
  current_tetromino = random(0, 7);
  current_rotation = 0;
}

void moveLeft() {
  if (!checkCollision(current_tetromino, current_rotation, current_x - 1, current_y)) {
    current_x--;
  }
}

void moveRight() {
  if (!checkCollision(current_tetromino, current_rotation, current_x + 1, current_y)) {
    current_x++;
  }
}

// Attempt to apply a rotation with simple wall kicks (nudges left/right)
bool tryApplyRotation(int targetRotation) {
  // Try in place
  if (!checkCollision(current_tetromino, targetRotation, current_x, current_y)) {
    current_rotation = targetRotation;
    return true;
  }

  // Try simple wall kicks: left and right up to 2 cells
  int kicks[4] = { -1, 1, -2, 2 };
  for (int k = 0; k < 4; k++) {
    int dx = kicks[k];
    if (!checkCollision(current_tetromino, targetRotation, current_x + dx, current_y)) {
      current_x += dx;
      current_rotation = targetRotation;
      return true;
    }
  }
  return false;
}

void rotateLeft() {
  int target = current_rotation - 1;
  if (target < 0) {
    target = 3;
  }
  tryApplyRotation(target);
}

void rotateRight() {
  int target = current_rotation + 1;
  if (target > 3) {
    target = 0;
  }
  tryApplyRotation(target);
}

void gameTick() {
  // Check for left press
  if (controller.isLeftPressed()) {
    Serial.println("Rotate left");
    rotateLeft();
  }

  // Check for right press
  if (controller.isRightPressed()) {
    Serial.println("Rotate right");
    rotateRight();
  }

  // Check for stick press
  if (controller.isStickPressed()) {
    Serial.println("Stick pressed");
  }

  // Get direction
  StickDirection direction = controller.getDirection();
  if (direction != CENTER) {
    Serial.println(StickDirectionStr[direction]);
  }

  // Move tetromino
  switch (direction) {
    case DOWN:
      moveDown();
      break;
    case LEFT:
      moveLeft();
      break;
    case RIGHT:
      moveRight();
      break;
    default:
      break;
  }

  // Ticks per drop
  if (gameTickTime >= 4) {
    moveDown();
    gameTickTime = 0;
  } else {
  gameTickTime++;
  } 
  checkLines();
}

void checkLines() {
  for (int i = 0; i < 22; i++) {
    for (int j = 0; j < 10; j++) {
      if (grid[i][j] == 0) {
        break;
      }
      if (j == 10) {
        removeLine(i);
      }
    }
  }
}

void removeLine(int line) {
  for (int i = line; i >= 0; i--) {
    for (int j = 0; j < 10; j++) {
      if (i == 0) {
        grid[i][j] = 0;
      } else {
        grid[i][j] = grid[i - 1][j];
      }
    }
  }
}

void loop() {
  controller.update();
  gameTick();


  // Check for collision
  if (checkCollision(current_tetromino, current_rotation, current_x, current_y)) {
    Serial.println("Collision detected!");
  }
  printGrid();
  delay(250);  // Small delay to reduce spam; adjust as needed
}
