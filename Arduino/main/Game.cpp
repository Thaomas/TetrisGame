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

      if (gridX < 0 || gridX >= GRID_WIDTH || gridY < 0 || gridY >= GRID_HEIGHT) {
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
  this->reset();
  this->gameOver = false;
}

void Game::reset() {
  for (int i = 0; i < GRID_HEIGHT; i++) {
    for (int j = 0; j < GRID_WIDTH; j++) {
      grid[i][j] = 0;
    }
  }
  
  currentTetromino = random(0, 7);
  currentRotation = 0;
  currentX = 3;
  currentY = 0;
  
  updateScore(0);

  buffer = 255;
  bufferChanged = true;
  gameOver = false;
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

  if(checkCollision(currentTetromino, currentRotation, currentX, currentY)) {
    this->gameOver = true;
  }
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
  int linesCleared = 0;
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
      linesCleared++;
    }
  }
  if(linesCleared){
    this->updateScore(this->score + linesCleared);
  }
}

void Game::printGrid() {

  // Header block with score and buffered tetromino preview
  Serial.print('+');
  for (int j = 0; j < GRID_WIDTH; j++) Serial.print('-');
  Serial.println('+');

  // Line with score and buffer label
  Serial.print('|');
  String header = String("Score: ") + String(this->score) + String("  Buf:");
  for (int j = 0; j < GRID_WIDTH; j++) {
    if (j < header.length()) {
      Serial.print(header[j]);
    } else {
      Serial.print(' ');
    }
  }
  Serial.println('|');

  // 4x4 preview for buffered tetromino at rotation 0
  for (int i = 0; i < 4; i++) {
    Serial.print('|');
    for (int j = 0; j < GRID_WIDTH; j++) {
      char ch = ' ';
      if (j < 4) {
        if (this->buffer != -1 && tetrominoes[this->buffer][0][i][j]) {
          ch = '#';
        }
      }
      Serial.print(ch);
    }
    Serial.println('|');
  }

  Serial.print('+');
  for (int j = 0; j < GRID_WIDTH; j++) Serial.print('-');
  Serial.println('+');

  // Actual grid below the header block
  byte tempGrid[GRID_HEIGHT][GRID_WIDTH];
  this->getGrid(tempGrid);

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

void Game::getGrid(byte outGrid[GRID_HEIGHT][GRID_WIDTH]) const {
  for (int i = 0; i < GRID_HEIGHT; i++) {
    for (int j = 0; j < GRID_WIDTH; j++) {
      outGrid[i][j] = grid[i][j];
    }
  }

  // Overlay active piece
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (!tetrominoes[currentTetromino][currentRotation][i][j]) continue;
      int gridX = currentX + j;
      int gridY = currentY + i;
      if (gridX < 0 || gridX >= GRID_WIDTH || gridY < 0 || gridY >= GRID_HEIGHT) continue;
      outGrid[gridY][gridX] = currentTetromino + 1;
    }
  }
}

void Game::getCompressedGrid(byte outGrid[GRID_HEIGHT][GRID_WIDTH/2]) const {
  byte tmpGrid[GRID_HEIGHT][GRID_WIDTH];
  this->getGrid(tmpGrid);
  for (int i = 0; i < GRID_HEIGHT; i++) {
    for (int j = 0; j < GRID_WIDTH; j+=2) {
      outGrid[i][j/2] = tmpGrid[i][j+1] << 4 | tmpGrid[i][j];
    }
  }
}


void Game::Tick(Controller& controller) {
  Serial.println(gameTickTime);
  Serial.println(gameTickTime % 2);
  if (this->gameOver) {
    if (controller.isStickPressed() || controller.isLeftPressed() || controller.isRightPressed()) {
      this->reset();
    }
    return;
  }
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
  this->swapBuffer();
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
if (this->gameTickTime >= 3) {
  this->moveDown();
  gameTickTime = 0;
} else {
  if(direction != DOWN) {
    this->gameTickTime++;
  }
}
this->checkLines();
}

void Game::updateScore(int newScore) {
  this->score = newScore;
  this->scoreChanged = true;
}

void Game::swapBuffer() {
  // Only allow swapping near the top to avoid invalid placements
  if (this->currentY > 2)
    return;

  if (this->buffer == -1) {
    this->buffer = random(0, 7);
  }
  // Swap current tetromino with buffer
  int oldTetromino = this->currentTetromino;
  this->currentTetromino = this->buffer;
  this->buffer = oldTetromino;

  // If the swap causes a collision, revert it
  if (checkCollision(this->currentTetromino, this->currentRotation, this->currentX, this->currentY)) {
    int tmp = this->currentTetromino;
    this->currentTetromino = this->buffer;
    this->buffer = tmp;
    return;
  }

  this->bufferChanged = true;
}