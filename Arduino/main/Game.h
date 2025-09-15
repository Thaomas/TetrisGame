#ifndef GAME_H
#define GAME_H

#include <Arduino.h>
#include "Control.h"

class Game {
public:
  // Lifecycle
  void init();

  // Debug/diagnostic printing to Serial
  void printGrid();

  void Tick(Controller controller);

private:
  // Movement controls (internal)
  void moveDown();
  void moveLeft();
  void moveRight();
  void rotateLeft();
  void rotateRight();

  // Step housekeeping (line clears, etc.)
  void checkLines();

  bool checkCollision(int shape, int rotation, int x, int y);
  bool tryApplyRotation(int targetRotation);

  byte grid[22][10] = {};
  int currentTetromino = 0;
  int currentRotation = 0;
  int currentX = 3;
  int currentY = 0;
  int gameTickTime = 0;
};

#endif // GAME_H


