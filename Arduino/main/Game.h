#ifndef GAME_H
#define GAME_H

#define GRID_WIDTH 10
#define GRID_HEIGHT 22

#include <Arduino.h>
#include "Control.h"

class Game {
public:
  // Lifecycle
  void init();
  bool gameOver = false;

  // Debug/diagnostic printing to Serial
  void printGrid();

  void Tick(Controller& controller);
  void reset();

  // Copy current grid (including active piece) into outGrid
  void getGrid(byte outGrid[GRID_HEIGHT][GRID_WIDTH]) const;
  void getCompressedGrid(byte outGrid[GRID_HEIGHT][GRID_WIDTH/2]) const ;

  bool scoreChanged = true;
  int getScore(){
    scoreChanged = false; 
    return score;
  };

  bool bufferChanged = true;
  int getBuffer() {
    bufferChanged = false; 
    return buffer;
  };

private:
  // Piece movement and rotation
  void moveDown();
  void moveLeft();
  void moveRight();
  void rotateLeft();
  void rotateRight();

  // Game state and logic
  void updateScore(int newScore);
  void swapBuffer();
  void checkLines();
  
  // Collision and rotation helpers
  bool checkCollision(int shape, int rotation, int x, int y);
  bool tryApplyRotation(int targetRotation);

  // Game data
  byte grid[GRID_HEIGHT][GRID_WIDTH] = {};
  int currentTetromino = 0;
  int currentRotation = 0;
  int currentX = 3;
  int currentY = 0;
  int gameTickTime = 0;
  int score = 0;
  int buffer = -1;
};

#endif // GAME_H


