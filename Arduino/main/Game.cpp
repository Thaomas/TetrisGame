#include "Game.h"
#include "shapes.h"
#include "Control.h"

bool Game::checkCollision(int shape, int rotation, int x, int y) {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (!tetrominoes[shape][rotation][i][j]) {
        continue;
      }

      int gridX = x + j;
      int gridY = y + i;

      if (gridX < 0 || gridX >= 10 || gridY < 0 || gridY >= 22) {
        return true;
      }
      if (grid[gridY][gridX]) {
        return true;
      }
    }
  }

  return false;
}

void Game::init() {
  for (int i = 0; i < GRID_HEIGHT; i++) {
    for (int j = 0; j < GRID_WIDTH; j++) {
      grid[i][j] = 0;
    }
  }
  currentTetromino = random(0, 7);
  currentRotation = 0;
  currentX = 3;
  currentY = 0;
}

void Game::moveDown() {
  if (!checkCollision(currentTetromino, currentRotation, currentX, currentY + 1)) {
    currentY++;
    return;
  }

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (tetrominoes[currentTetromino][currentRotation][i][j]) {
        int gridX = currentX + j;
        int gridY = currentY + i;
        grid[gridY][gridX] = currentTetromino + 1;
      }
    }
  }

  currentX = 3;
  currentY = 0;
  currentTetromino = random(0, 7);
  currentRotation = 0;
}

void Game::moveLeft() {
  if (!checkCollision(currentTetromino, currentRotation, currentX - 1, currentY)) {
    currentX--;
  }
}

void Game::moveRight() {
  if (!checkCollision(currentTetromino, currentRotation, currentX + 1, currentY)) {
    currentX++;
  }
}

bool Game::tryApplyRotation(int targetRotation) {
  if (!checkCollision(currentTetromino, targetRotation, currentX, currentY)) {
    currentRotation = targetRotation;
    return true;
  }

  int kicks[4] = { -1, 1, -2, 2 };
  for (int k = 0; k < 4; k++) {
    int dx = kicks[k];
    if (!checkCollision(currentTetromino, targetRotation, currentX + dx, currentY)) {
      currentX += dx;
      currentRotation = targetRotation;
      return true;
    }
  }
  return false;
}

void Game::rotateLeft() {
  int target = currentRotation - 1;
  if (target < 0) {
    target = 3;
  }
  tryApplyRotation(target);
}

void Game::rotateRight() {
  int target = currentRotation + 1;
  if (target > 3) {
    target = 0;
  }
  tryApplyRotation(target);
}

void Game::checkLines() {
  for (int i = 0; i < GRID_HEIGHT; i++) {
    bool full = true;
    for (int j = 0; j < GRID_WIDTH; j++) {
      if (grid[i][j] == 0) {
        full = false;
        break;
      }
    }
    
    if (full) {
      for (int r = i; r >= 0; r--) {
        for (int c = 0; c < GRID_WIDTH; c++) {
          grid[r][c] = (r == 0) ? 0 : grid[r - 1][c];
        }
      }
    }
  }
}

void Game::printGrid() {
  byte tempGrid[GRID_HEIGHT][GRID_WIDTH];
  for (int i = 0; i < GRID_HEIGHT; i++) {
    for (int j = 0; j < GRID_WIDTH; j++) {
      tempGrid[i][j] = grid[i][j];
    }
  }

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (!tetrominoes[currentTetromino][currentRotation][i][j]) continue;
      int gridX = currentX + j;
      int gridY = currentY + i;
      if (gridX < 0 || gridX >= GRID_WIDTH || gridY < 0 || gridY >= GRID_HEIGHT) continue;
      tempGrid[gridY][gridX] = currentTetromino + 1;
    }
  }

  Serial.print('+');
  for (int j = 0; j < GRID_WIDTH; j++) Serial.print('-');
  Serial.println('+');
  for (int i = 0; i < GRID_HEIGHT; i++) {
    Serial.print('|');
    for (int j = 0; j < GRID_WIDTH; j++) Serial.print(tempGrid[i][j]);
    Serial.println('|');
  }
  Serial.print('+');
  for (int j = 0; j < GRID_WIDTH; j++) Serial.print('-');
  Serial.println('+');
  Serial.println();
}


void Game::Tick(Controller controller) {
// Check for left press
if (controller.isLeftPressed()) {
  Serial.println("Rotate left");
  this->rotateLeft();
}

// Check for right press
if (controller.isRightPressed()) {
  Serial.println("Rotate right");
  this->rotateRight();
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
    this->moveDown();
    break;
  case LEFT:
    this->moveLeft();
    break;
  case RIGHT:
    this->moveRight();
    break;
  default:
    break;
}

// Ticks per drop
if (this->gameTickTime >= 4) {
  this->moveDown();
  gameTickTime = 0;
} else {
  if(direction != DOWN) {
    this->gameTickTime++;
  }
}
this->checkLines();
}

