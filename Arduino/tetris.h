#ifndef TETRIS_H
#define TETRIS_H

class Tetris {
public:
    virtual ~Tetris() {}

    // Initialize the game
    virtual void init() = 0;

    // Update the game state
    virtual void update() = 0;

    // Handle user input
    virtual void handleInput(int key) = 0;

    // Get the game state
    virtual int getGameState() = 0;
};

#endif // TETRIS_H
