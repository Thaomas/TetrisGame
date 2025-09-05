
#include "tetris.h"

class BaseTetris : public Tetris {
public:
    virtual ~BaseTetris() {}

    void init() override {
        // Base implementation (can be empty or provide default behavior)
    }

    void update() override {
        // Base implementation (can be empty or provide default behavior)
    }

    void handleInput(int key) override {
        // Base implementation (can be empty or provide default behavior)
    }

    int getGameState() override {
        // Base implementation (can be empty or provide default behavior)
        return 0;
    }
};

